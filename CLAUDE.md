# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

Traceshark is a C++/Qt (5 or 6) desktop GUI for visualizing Linux kernel **ftrace** and **perf** traces. It draws per-CPU idle/frequency graphs, per-CPU and unified scheduling graphs, migration arrows, and provides filterable event/latency/stats tables. Input is an ASCII trace file (`trace-cmd report ... > x.asc` or `perf script`-style output).

## Build, run, install

The build system is **qmake** (`traceshark.pro`); the checked-in `Makefile` is generated output — never edit it by hand, regenerate with qmake.

```sh
qmake          # use qmake6 or qmake-qt5 on distros where plain `qmake` is Qt4/ambiguous
make -j$(nproc)
sudo make install    # installs the `traceshark` binary; not supported on macOS
./traceshark [tracefile.asc]    # run directly; pass a trace file to auto-open it
```

After editing `traceshark.pro` (or adding/removing source files), re-run `qmake` before `make`. Generated artifacts (`moc_*`, `qrc_traceshark.cpp`, `obj/gitversion.h`, the `Makefile`, the `traceshark` binary) are git-ignored.

Toolchain: a C++ compiler (g++ default), `make`, and Qt5 dev packages (`qtbase5-dev` / `qt5-qtbase-devel`) or Qt6 (`brew install qt` on macOS).

## Build configuration

Build options are toggled by editing the variable assignments near the top of the config section of `traceshark.pro` (most are commented-out examples). Notable ones:

- `DISABLE_OPENGL = yes` — turn off OpenGL rendering (line-width selection only works with OpenGL; try this for rendering glitches/slowness).
- `USE_SYSTEM_QCUSTOMPLOT = yes` — link the system libqcustomplot instead of the bundled `qcustomplot/`. **Not recommended**: the bundled copy carries important performance/large-dataset patches not in upstream.
- `MARCH_FLAG` / `MTUNE_FLAG` — per-machine optimization flags (`-march=native` etc.).
- `USE_ALTERNATIVE_COMPILER = clang++-14` — build with a non-default compiler.
- `CONFIG += debug` / `QT_DEBUG_BUILD = yes` — debug builds.

## Testing

There is **no automated unit-test suite**. Verification is by running the GUI against real trace files:

- Sample traces live in a separate repo: `git clone https://github.com/cunctator/traceshark-resources.git`.
- `scripts/perf-record.sh` captures a perf trace with the right event set (sched + cpu_frequency/cpu_idle + callgraphs). Convert ftrace with `trace-cmd report trace.dat > x.asc`.
- `scripts/testrun.sh <binary> <tracefile> <iterations> [sleep]` — opens the trace N times (for crash/perf shakeout); pair with `scripts/stats.sh <logfile>` to summarize `processTrace`/`showTrace`/`tracePlot->show` timings the app prints.

When making changes, build cleanly and open at least one ftrace and one perf trace to confirm parsing and rendering still work.

## Architecture

See **ARCHITECTURE.md** for the full picture (directory layout, key classes, the file-open data flow, and the parser threading model). The essentials:

- `parser/` — file I/O + grammar parsing. `TraceParser` mmaps the file and runs a two-thread producer/consumer pipeline: a reader (`LoadThread`, 2 MB chunks) feeds a 4-slot `ThreadBuffer<TraceLine>` ring, and a parser thread (`WorkThread`) auto-detects ftrace vs perf and runs `FtraceGrammar`/`PerfGrammar` to emit `TraceEvent`s into a `TList<TraceEvent>`. `TString` fields are **zero-copy pointers into the mmap** — do not assume they outlive the file or are NUL-terminated like normal C strings.
- `analyzer/` — `TraceAnalyzer` is the central mediator; it owns the parser and all domain models (`Task`/`CPUTask`, `CpuFreq`, `CpuIdle`, `Migration`, `Latency`) and runs the per-event dispatch (`processSwitchEvent`, `processWakeupEvent`, `processCPUfreqEvent`, …) in a batch loop driven by `IndexWatcher`.
- `ui/` — all Qt widgets/dialogs/models plus the plotting layer. `MainWindow` is the shell and entry point for user actions; `TracePlot` (a `QCustomPlot` subclass) is the canvas; `TaskGraph` wraps `QCPGraph` for hit-testing.
- `mm/` — custom allocators (`MemPool` arena, `StringPool`/`StringTree` interning) the parser relies on for speed.
- `vtl/` — "Viktor's Template Library": `TList<T>` (cache-friendly segmented list, the primary event container), `AVLTree<K,V>` (used for the PID-keyed task map), `BitVector`, `Time`.

Performance is a primary design constraint throughout (large traces, tight parse loop, custom containers/allocators, mmap zero-copy). Prefer the existing `vtl`/`mm` primitives over STL/Qt containers in hot paths, and keep the parser allocation-free.

## Conventions

- **Keep lines within 80 columns.** This is a strong preference rather than an absolute rule; exceptions are acceptable for special cases, e.g. a string literal that would be harder to read and understand if split across lines.
- Every source file carries an SPDX header and is **dual licensed `GPL-2.0-or-later OR BSD-2-Clause`**. Copy the existing header (and copyright line format) when creating new files.
- Adding ftrace/perf event support means touching the grammar + params under `parser/ftrace/` or `parser/perf/`, then the matching `process*Event` handler in `analyzer/`.
- The app version string is injected at build time from git by `scripts/gitversion` into `obj/gitversion.h` (`-dirty` suffix when the tree has uncommitted changes).
