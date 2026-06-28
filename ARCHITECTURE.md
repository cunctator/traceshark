# Traceshark Architecture Overview

Traceshark is a C++/Qt5/6 application for visualizing Linux kernel ftrace and perf traces. The build system is qmake (`traceshark.pro`).

---

## Directory Layout

| Directory | Purpose |
|-----------|---------|
| `ui/` | All Qt widgets, dialogs, models, and plot classes |
| `analyzer/` | Domain model: `TraceAnalyzer` orchestrator and task/CPU/frequency/latency/migration data classes |
| `parser/` | File I/O and grammar-based parsing for ftrace and perf formats |
| `parser/ftrace/` | ftrace-specific grammar and params |
| `parser/perf/` | perf-specific grammar and params |
| `threads/` | `LoadThread` (reader), `WorkThread` (parser), `ThreadBuffer`, `IndexWatcher`, `WorkQueue` |
| `mm/` | Custom allocators: `MemPool`, `StringPool`, `StringTree` |
| `vtl/` | Viktor's Template Library: `TList<T>`, `AVLTree<K,V>`, `BitVector`, `Time`, heapsort |
| `qcustomplot/` | Locally modified QCustomPlot with OpenGL and large-dataset patches |
| `misc/` | `main.cpp`, global constants (`traceshark.h`), settings, state file, string/type helpers |

---

## Key Classes

### Application Shell
- **`MainWindow`** (`ui/mainwindow.h`) — Top-level window; owns the analyzer, plot, and all UI objects. Entry point for user actions (open file, filter, reset, etc.).

### Domain / Analysis
- **`TraceAnalyzer`** (`analyzer/traceanalyzer.h`) — Central mediator. Owns the parser and all data models. Drives the processing pipeline and exposes results to the UI.
- **`AbstractTask`** / **`Task`** / **`CPUTask`** (`analyzer/`) — Hierarchical time-series data for drawing task execution lanes. `CPUTask` holds per-CPU run/sleep vectors; `Task` aggregates across CPUs.
- **`CpuFreq`** / **`CpuIdle`** (`analyzer/`) — Per-CPU frequency and idle-state time series.
- **`Migration`** / **`Latency`** (`analyzer/`) — Scheduling migration arrows and latency (sched + wakeup) records.

### Parsing
- **`TraceParser`** (`parser/traceparser.h`) — File I/O plus grammar dispatch. Spawns two background threads (reader + parser) and auto-detects ftrace vs. perf format.
- **`FtraceGrammar`** / **`PerfGrammar`** (`parser/ftrace/`, `parser/perf/`) — Line-level parsers that produce `TraceEvent` objects.
- **`TraceEvent`** (`parser/traceevent.h`) — Atomic unit of parsed data. Fields: `pid`, `cpu`, `time`, `type`, `argv`.
- **`TraceFile`** (`parser/tracefile.h`) — mmap abstraction; `TString` objects are zero-copy pointers into the mapping.

### Threading
- **`LoadThread`** / **`LoadBuffer`** (`threads/`) — Reads the file in 2 MB chunks into a `ThreadBuffer<TraceLine>` ring (4 slots).
- **`WorkThread`** / **`WorkQueue`** (`threads/`) — Parser thread; consumes `TraceLine` slots and produces `TraceEvent` into a `TList<TraceEvent>`.
- **`IndexWatcher`** (`threads/indexwatcher.h`) — Synchronization primitive; lets the main thread block until the next batch of events is ready.

### Rendering
- **`TracePlot`** (`ui/traceplot.h`) — `QCustomPlot` subclass; the main 2D canvas.
- **`TaskGraph`** (`ui/taskgraph.h`) — Wraps `QCPGraph`; maintains a static map `QCPGraph* → TaskGraph*` for hit-testing.
- **`YAxisTicker`** (`ui/yaxisticker.h`) — Custom Y-axis ticker that maps CPU/task lane positions to human-readable labels.
- **`MigrationArrow`** / **`MigrationLine`** (`ui/`) — Custom `QCPAbstractItem` subclasses for drawing task migrations.
- **`Cursor`** (`ui/cursor.h`) — Vertical time cursor drawn on the plot.

### UI / Dialogs / Models
- **`EventsWidget`** / **`EventsModel`** — Scrollable table of raw `TraceEvent` records.
- **`TaskSelectDialog`** / **`TaskModel`** — Dialog for choosing which tasks to display.
- **`CpuSelectDialog`** / **`CpuSelectModel`** — Dialog for choosing which CPUs to display.
- **`LatencyWidget`** / **`LatencyModel`** — Table of scheduling and wakeup latencies.
- **`StatsModel`** / **`StatsLimitedModel`** — Per-task statistics (runtime, switches, etc.).
- **`RegexDialog`** / **`RegexWidget`** — Regex-based event filtering UI.
- **`EventSelectDialog`** / **`EventSelectModel`** — Dialog for filtering by event type.
- **`GraphEnableDialog`** — Toggle visibility of individual task graphs.
- **`InfoWidget`** / **`CursorInfo`** — Display of time-at-cursor and event details.
- **`TaskToolBar`** — Toolbar for per-task actions (zoom, highlight, etc.).

### Support
- **`SettingStore`** / **`Setting`** (`misc/`) — Persistent application settings.
- **`StateFile`** (`misc/statefile.h`) — Saves and restores per-trace display state.
- **`MemPool`** (`mm/mempool.h`) — Arena allocator used by the parser for `TraceEvent` storage.
- **`StringPool`** / **`StringTree`** (`mm/`) — Interning structures for event-type strings.
- **`TList<T>`** (`vtl/tlist.h`) — Cache-friendly segmented list; primary container for `TraceEvent` arrays.
- **`AVLTree<K,V>`** (`vtl/avltree.h`) — Self-balancing BST used as `taskMap` (keyed by PID).

---

## File-Open Flow

```
openAction::triggered()
  └─ MainWindow::openTrace()          file dialog
       └─ openFile(name)
            ├─ loadTraceFile()
            │    └─ TraceAnalyzer::open()
            │         └─ TraceParser::open()
            │              ├─ mmap the file (TraceFile)
            │              ├─ spawn readerThread  ──→ fills ThreadBuffer<TraceLine>
            │              └─ spawn parserThread  ──→ drains TraceLine, produces TList<TraceEvent>
            │
            └─ processTrace()
                 └─ TraceAnalyzer::processTrace()   ← blocks on IndexWatcher per batch
                      ├─ processSwitchEvent()        → taskMap, cpuTaskMaps[]
                      ├─ processWakeupEvent()        → wakeLatencies
                      ├─ processCPUfreqEvent()       → cpuFreq[]
                      ├─ processCPUidleEvent()       → cpuIdle[]
                      └─ processMigrationEvent()     → migrations[]
```

After parsing completes:

1. **`computeLayout()`** — Assigns Y-coordinates to CPU/task lanes; populates ticks and tick labels for `YAxisTicker`.
2. **`eventsWidget->setEvents(...)`** — Feeds the raw event table.
3. **`taskSelectDialog->setTaskMap(...)`** — Feeds the task selector dialog.
4. **`rescaleTrace()`** → `TraceAnalyzer::doScale()` — Converts time vectors to plot coordinates.
5. **`computeStats()`** → `TraceAnalyzer::doStats()` — Computes per-task statistics.
6. **`showTrace()`** — Creates `QCPGraph` objects per `CPUTask` (`addSchedGraph`, `addWakeupGraph`, etc.) and calls `replot()`.

---

## Parser Threading Model

```
File on disk
    │
    ▼
LoadThread  ──2 MB chunks──►  ThreadBuffer<TraceLine>  (4-slot ring)
                                        │
                                        ▼
                              WorkThread / parserThread
                                  auto-detect ftrace vs perf
                                  FtraceGrammar or PerfGrammar
                                        │
                                        ▼
                              TList<TraceEvent>
                                        │
                              IndexWatcher notifies main thread
                                        │
                                        ▼
                              TraceAnalyzer::processGeneric()
                              (tight producer-consumer batch loop)
```

- The reader and parser threads are a producer-consumer pipeline sharing a 4-slot `ThreadBuffer` ring.
- `TString` fields inside `TraceEvent` are zero-copy pointers into the mmap'd file — no string allocation during parsing.
- The main thread waits on `IndexWatcher::waitForNextBatch()` and processes events in batches, keeping memory pressure low for large traces.

---

## Data Flow Summary

```
Raw file  →  [reader thread]  →  TraceLine ring
          →  [parser thread]  →  TList<TraceEvent>
          →  [main thread]    →  taskMap / cpuTaskMaps / cpuFreq / cpuIdle / migrations / latencies
          →  [main thread]    →  QCPGraph objects on TracePlot  →  screen
```
