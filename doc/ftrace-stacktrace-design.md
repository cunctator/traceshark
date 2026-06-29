# Design: ftrace stack-trace support

Status: implemented
Scope: display `kernel_stack` / `user_stack` backtraces from ftrace traces in
`EventInfoDialog`, the same way perf backtraces are already shown.

## 1. Background: how perf stack traces work today

In a perf trace the stack frames are physically contiguous in the file,
immediately after the event line:

```
migration/0  18 [000] 12942.039622: sched:sched_switch: prev_comm=...
	ffffffffb7ad6b30 __schedule+0x8e0 ([kernel.kallsyms])
	ffffffffb7ad7303 schedule+0x23 ([kernel.kallsyms])
	...
```

`TraceParser::parseLinePerf()` (`parser/traceparser.h`) captures this with a
"trailing bytes" trick:

- Lines that do **not** parse as an event are "garbage". The first garbage line
  after an event records `perfLineData.infoBegin = line.begin`.
- When the next real event is parsed, a `Chunk{offset, len}` covering
  `[infoBegin, nextLine.begin)` is stored in the **previous** event's
  `postEventInfo`.

`Chunk` (`misc/chunk.h`) is just a file `{offset, len}`. `EventInfoDialog::show()`
(`ui/eventinfodialog.cpp`) reads those raw bytes zero-copy from the mmap via
`TraceFile::getChunkArray()` and displays them verbatim. `fixLastEvent()`
(`parser/traceparser.cpp`) handles the final event at EOF (no "next event" to
trigger finalization).

`parseLineFtrace()` currently hard-sets `event.postEventInfo = nullptr`.

## 2. Why ftrace is different

ftrace emits the stack as **separate events**, not trailing bytes, and splits it
into a kernel part and a user part:

```
trace-cmd-17890 [015] d..2. 18481.505414: sched_switch: trace-cmd:17890 ...   <- origin (CPU 15)
   <idle>-0     [004] d..1. 18481.505417: cpu_idle: ...                        <- other CPU interleaved
trace-cmd-17890 [015] d..2. 18481.505419: kernel_stack: <stack trace >        <- belongs to origin
=> trace_event_raw_event_sched_switch (ffffffffb4d4fc8e)
=> __schedule (ffffffffb58d6b30)
   ...
trace-cmd-17890 [015] d..2. 18481.505419: user_stack:  => 0x7fd77245b687       <- belongs to origin
	=> 0x0
	...
```

Two structural consequences:

1. **The stack lives in distinct events**, displaced from the origin event.
2. **They are the next events on the same CPU**, with other CPUs' events
   interleaved in between. The rule: a `kernel_stack` / `user_stack` belongs to
   the most recent non-stack event on the same CPU.

There is one useful symmetry: the `=> ...` frame lines that follow a
`kernel_stack` / `user_stack` line fail `FtraceGrammar::parseLine()` (they have no
`task-pid [cpu] time:` prefix), so they are already "garbage" — capturable with
the same trailing-bytes idea perf uses.

## 3. Chosen design

Two decisions were made up front:

- **Consume + discard** the `kernel_stack` / `user_stack` pseudo-events: detect
  them in the parser, attach their text to the origin event, and never commit
  them to the event list. No event-filter is needed.
- **Capture from line begin**: each captured `Chunk` spans from the pseudo-event
  line's own `line.begin` through its trailing frame lines. This needs no grammar
  changes and includes the `user_stack` inline first frame. The captured text
  therefore still contains the `kernel_stack:` / `user_stack:` header lines; those
  are removed cheaply at consume time (see §3.5), keeping the parser simple and the
  zero-copy file-chunk model intact.

The display path is **unified** between perf and ftrace by letting `postEventInfo`
describe more than one file range.

### 3.1 Data-structure changes

**`misc/types.h`** — add the two event types to `TRACEEVENTS_DEFS_` so they get
stable `event_t` ids and are seeded into the grammar's event tree by
`FtraceGrammar::setupEventTree()`. This is how the parser recognizes them
(`event.type == KERNEL_STACK || event.type == USER_STACK`):

```c
TSHARK_ITEM_(KERNEL_STACK,     "kernel_stack"),
TSHARK_ITEM_(USER_STACK,       "user_stack"),
```

(They never reach `processTrace()` since they are discarded, and no dispatch case
references them, so adding them as "known" events is inert beyond detection.)

**`misc/chunk.h`** — turn `Chunk` into a singly linked list so one event can carry
several non-adjacent file ranges:

```c
class Chunk {
public:
	int64_t offset;
	int32_t len;
	Chunk  *next;   // new; nullptr for the single-chunk (perf) case
};
```

- perf: one `Chunk`, `next == nullptr` — behavior unchanged.
- ftrace origin: a chain of up to two chunks (kernel chunk, then user chunk).

**`parser/traceparser.h`** — add ftrace-only bookkeeping as `TraceParser` members
(this state is only meaningful while parsing ftrace, so it does not belong in the
per-format `TraceLineData`, which is instantiated for both perf and ftrace):

```c
TraceEvent *ftraceLastEventByCPU[NR_CPUS_ALLOWED]; // last non-stack event per CPU
// pending stack capture awaiting finalization on the next event line:
bool        ftraceStackPending;
int64_t     ftraceStackInfoBegin;
TraceEvent *ftraceStackOrigin;
```

`nrCPUs` is not known at parse time (the analyzer derives it later), so
`ftraceLastEventByCPU` is a fixed array sized by the existing `NR_CPUS_ALLOWED`
cap, indexed through the `ftraceLastEventForCPU()` / `ftraceSetLastEventForCPU()`
helpers which gate every access with the existing `isValidCPU()` bounds check. It
is zeroed in `clearFtraceStackData()` (called from `prepareParse()`) via
`tshark_bzero()` — consistent with how the analyzer allocates its per-CPU arrays.
Because `TList` never relocates elements, the stored origin pointer stays valid for
the life of the trace.

### 3.2 Parser algorithm (ftrace)

Modeled on the perf trailing-chunk mechanism, but the chunk is redirected onto the
origin event instead of the pseudo-event, and the pseudo-event is not committed.

On each parsed ftrace event line (inside / called from `parseLineFtrace()`):

```
1. If stackPending:
       finalize chunk = { offset: stackInfoBegin,
                          len:    line.begin - stackInfoBegin,
                          next:   nullptr }
       append chunk to stackOrigin->postEventInfo chain   // tail append, max 2
       stackPending = false

2. If event.type is KERNEL_STACK or USER_STACK:        // a stack pseudo-event
       origin = lastEventByCPU[event.cpu]              // may be null at trace start
       if origin != null:
           stackPending    = true
           stackInfoBegin  = line.begin                // "capture from line begin"
           stackOrigin     = origin
       do NOT commit; return false                     // discard, reuse preAlloc slot
                                                        // do NOT update lastEventByCPU

3. Else (ordinary event):
       commit as today
       lastEventByCPU[event.cpu] = &committed event
       return true
```

Notes:

- The trailing `=> ...` frame lines are non-events; they need no per-line handling
  because `stackInfoBegin` was already set at the pseudo-event line. The chunk
  simply extends until the next event line of any kind.
- A `kernel_stack` block is normally followed immediately (same CPU) by the
  `user_stack` event. That `user_stack` line is itself an event line, so step 1
  finalizes the kernel chunk onto the origin, then step 2 starts the user chunk
  with the **same** origin. The next ordinary event finalizes the user chunk.
- Returning `false` for a discarded stack event makes `parseBuffer_()` reuse the
  pre-allocated `TraceEvent` slot and argv (no commit) — exactly the existing
  behavior for lines that fail to parse. The grammar's pool allocations for the
  discarded line (argv/taskName) are not rolled back; this is minor, bounded
  waste, freed in bulk on `clear()`.
- `ftraceStackPending` / `ftraceLastEventByCPU` are `TraceParser` members that
  persist across buffer boundaries, so a stack block split across two read buffers
  is handled. They are reset per file by `clearFtraceStackData()`.

### 3.3 EOF handling

`fixLastEvent()` (`parser/traceparser.cpp`) currently early-returns for non-perf
traces. Extend the ftrace branch: if `ftraceStackPending` is set at EOF, finalize
the last chunk with `len = traceFile->getFileSize() - ftraceStackInfoBegin` and
append it to `ftraceStackOrigin`'s chain. This covers a trace that ends in a stack
block.

### 3.4 Display (`ui/eventinfodialog.cpp`)

`EventInfoDialog::show()` takes the trace type (`tracetype_t ttype`, passed by
`MainWindow` from `analyzer->getTraceType()`) and walks the `Chunk` chain instead
of reading a single chunk, concatenating each range. The header stripping (§3.5)
is then applied **only for ftrace traces**; perf backtraces are displayed
directly:

```cpp
QByteArray array;
for (const Chunk *c = event.postEventInfo; c != nullptr; c = c->next) {
	if (c->len <= 0)
		continue;
	array += file.getChunkArray(c, &ts_errno);
	if (ts_errno != 0)
		break;
}
// ... then:
if (ttype == TRACE_TYPE_FTRACE) {
	QByteArray cleaned;
	cleaned.resize(array.size());
	int len = TShark::stripStackTraceHeaders(array.constData(), array.size(),
						 cleaned.data());
	cleaned.resize(len);
	text = QString(cleaned);
} else {
	text = QString(array);
}
textEdit->setPlainText(text);
```

The chunk-walk itself serves both formats: perf yields a single-element chain,
ftrace a one- or two-element chain. Only the header stripping is ftrace-specific,
so perf does no unnecessary work. The existing `isIntact()` guard is unaffected
because every range is still a genuine file offset.

### 3.5 Header stripping accessor (`misc/traceshark.{h,cpp}`)

`TShark::stripStackTraceHeaders(const char *in, int inlen, char *out)` removes the
`kernel_stack:` / `user_stack:` header lines from the captured text and returns the
cleaned length. It is deliberately a plain-`char*`, allocation-free, single linear
pass (no Qt string operations) so it can be reused on the hot path of future
consumers such as flame graphs. The output is never longer than the input, so the
caller sizes `out` to `inlen`.

Rules, applied per line:

- A line containing `kernel_stack:` is dropped entirely (it carries only the
  `<stack trace >` placeholder; its frames are on the following lines).
- On a line containing `user_stack:`, the prefix up to and including the label and
  the following blanks is removed, **preserving the first frame the kernel emits
  inline** on that line.
- Every other line is copied verbatim. Frame lines (which start with `=>` after
  optional whitespace) take a fast path that skips the label search entirely.

This is naturally perf-safe: perf backtraces contain neither label, so even if it
were called on one it would pass through byte-for-byte unchanged. Nevertheless
`EventInfoDialog::show()` invokes it only for ftrace traces (§3.4), so that
opening a perf trace does no unnecessary work for what is purely an ftrace
concern.

## 4. Files to change

| File | Change |
|------|--------|
| `misc/types.h` | Add `KERNEL_STACK`, `USER_STACK` to `TRACEEVENTS_DEFS_` |
| `misc/chunk.h` | Add `Chunk *next` |
| `misc/traceshark.{h,cpp}` | Add `TShark::stripStackTraceHeaders()` accessor (§3.5) |
| `parser/traceparser.h` | Rework `parseLineFtrace()` per §3.2; add ftrace-only stack state + `ftraceLastEventForCPU()`/`ftraceSetLastEventForCPU()`; ensure perf path sets `next = nullptr` |
| `parser/traceparser.cpp` | `clearFtraceStackData()` (called from `prepareParse()`); `attachStackChunk()`; extend `fixLastEvent()` ftrace branch (§3.3) |
| `ui/eventinfodialog.h` | Add `tracetype_t ttype` parameter to `show()` |
| `ui/eventinfodialog.cpp` | Walk the `Chunk` chain; strip headers only for ftrace (§3.4, §3.5) |
| `ui/mainwindow.cpp` | Pass `analyzer->getTraceType()` to `EventInfoDialog::show()` |

No event-filter changes are required (pseudo-events are discarded, not hidden).

`TraceAnalyzer::writePerfEvent()` (event export) also reads `postEventInfo`, but it
is reached only for perf traces (`if (!isPerf) goto skip_perf;`), where the chain
is always a single chunk, so it needs no change. Including ftrace stack traces in
the event-export output would be a separate enhancement.

## 5. Edge cases

- **Kernel-only or user-only stack** → chain length 1.
- **Stack event at a CPU's first appearance** with no prior non-stack event →
  `ftraceLastEventForCPU(cpu)` returns null → drop the stack (nothing to attach
  to).
- **Trace ends mid-stack** → handled by the `fixLastEvent()` extension (§3.3).
- **`<stack trace >` placeholder** and the `kernel_stack:` / `user_stack:` header
  lines are captured (a consequence of capture-from-line-begin) but removed at
  consume time by `stripStackTraceHeaders()` (§3.5).
- **Sparse / high CPU ids** → `ftraceLastEventForCPU()` /
  `ftraceSetLastEventForCPU()` gate every access with `isValidCPU()`, so a CPU id
  at or beyond `NR_CPUS_ALLOWED` is simply ignored (its stack is dropped) rather
  than indexing out of bounds.
- **perf unchanged** → its single-chunk path now just sets `next = nullptr`, and
  `EventInfoDialog::show()` displays perf backtraces directly, skipping
  `stripStackTraceHeaders()` entirely since it is an ftrace-only concern.

## 6. Testing

- `ftrace-stacktrace-format.txt` and `perf-stacktrace-format.txt` (in this `doc/`
  directory, alongside this document) are the reference fixtures. Verify:
  - Clicking the `sched_switch` on CPU 15 shows both its kernel and user frames.
  - Clicking the `cpu_idle` on CPU 4 shows its kernel and user frames.
  - The `kernel_stack` / `user_stack` lines no longer appear as their own rows in
    the events table.
  - perf traces still display their backtraces exactly as before.
- Check a trace with stacks enabled for only some events (origins without stacks
  show nothing, as today).
- Check a trace whose final event carries a stack (EOF path).

## 7. Alternatives considered

- **Keep pseudo-events + auto-filter** instead of discarding: reuses the
  `FILTER_EVENT` machinery and keeps them inspectable, but pollutes the event list
  and event counts and is more invasive to the commit pipeline. Rejected in favor
  of consume + discard.
- **Capture from just after the `kernel_stack:` / `user_stack:` label**: cleaner
  output (no header line) and still captures the `user_stack` inline first frame,
  but requires exposing the post-label byte offset from the grammar/`TraceLine`.
  Rejected for now in favor of the zero-grammar-change capture-from-line-begin.
- **Synthesize combined stack text in a `MemPool`** and point `postEventInfo` at
  it: breaks the zero-copy / `isIntact()` file-chunk model. Rejected.
- **Store links to the pseudo-events on the origin** instead of chunks: bloats
  every `TraceEvent` with extra pointers for a feature few events use. The
  `Chunk` chain keeps the cost on the (rare) events that actually have stacks.
