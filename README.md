# 1. Introduction to Traceshark

This is a graphical viewer for the Ftrace and Perf events that can be captured by the Linux kernel. It visualizes the following events:

```
cpu_frequency
cpu_idle
sched_migrate_task
sched_process_exit
sched_process_fork
sched_switch
sched_wakeup
sched_wakeup_new
```

![traceshark screenshot](https://raw.githubusercontent.com/cunctator/traceshark/6c82bceacf272617de2692b5e2efd4ec1b0ecdde/doc/ts-screenshot1.png)

Above is a screenshot of traceshark. The four uppermost graphs are for displaying CPU idle and frequency states. They are four because the measurment was made on a system with four virtual CPUs. The green graphs with red circles ![idle graph](https://raw.githubusercontent.com/cunctator/traceshark/eb46ef87526687ec139be16d444f1379f9d5c8a0/doc/idle-graph.png) show the CPU idle states while the thicker blue graphs ![idle graph](https://raw.githubusercontent.com/cunctator/traceshark/eb46ef87526687ec139be16d444f1379f9d5c8a0/doc/freq-graph.png) show the CPU frequency changes.

Below these four graphs are the per CPU scheduling graphs, the different colors of these graphs are for different tasks. The small vertical bars that are shown just above the per CPU graphs indicates the waiting time between wakeup and being scheduled, the highest height is equal to 20 ms, i.e. a full lenght means that the waiting time was at least 20 ms, possibly more.

Below the scheduling graphs are the migration graphs. Task migrations between CPUs are shown with arrows. Fork/exit is shown with an arrow from/to `fork/exit`.

Below the migration arrows are the unified task graphs, where tasks are shown without caring about which CPU it is running on. Here the time between wakeup and being scheduled in shown by horizontal bars.

These graphs will only be shown if requested by the user. It is necessary to select a task and click the ![Add task graph button](https://raw.githubusercontent.com/cunctator/traceshark/808c9a1ed38acfd01e4a2d985b25c98867168f71/images/addtask30x30.png) button or the `Add a unified graph` button in the task select dialog.

The task select dialog can be shown by clicking `View -> Show task list`, or by clicking the dedicated ![show task dialog button](https://raw.githubusercontent.com/cunctator/traceshark/808c9a1ed38acfd01e4a2d985b25c98867168f71/images/taskselector30x30.png) button for it on the left panel.

## 1.1 Brief summary of the functionality of the GUI

### 1.1.1 Functionality of the buttons

There are a number of buttons in the GUI, here is a description of the buttons in the left panel:

* ![open button](https://raw.githubusercontent.com/cunctator/traceshark/808c9a1ed38acfd01e4a2d985b25c98867168f71/images/open30x30.png) This button is used to open a trace file.
* ![close button](https://raw.githubusercontent.com/cunctator/traceshark/808c9a1ed38acfd01e4a2d985b25c98867168f71/images/close30x30.png) Closes the currently open trace.
* ![screenshot button](https://raw.githubusercontent.com/cunctator/traceshark/808c9a1ed38acfd01e4a2d985b25c98867168f71/images/screenshot30x30.png) Take a screenshot of the plot and save it to a file.
* ![task select button](https://raw.githubusercontent.com/cunctator/traceshark/808c9a1ed38acfd01e4a2d985b25c98867168f71/images/taskselector30x30.png) Show the task select dialog. This makes it possible to filter the events view by task, to show a task in the legend, or to show a unified graph. This button is very useful when the user knows the name of a task of interest but cannot find it easily among the scheduling graphs.
* ![Event list](https://raw.githubusercontent.com/cunctator/traceshark/808c9a1ed38acfd01e4a2d985b25c98867168f71/images/eventfilter30x30.png) Show a list of the different event types and it's possible to filter the events view by event type.
* ![Time filter](https://raw.githubusercontent.com/cunctator/traceshark/808c9a1ed38acfd01e4a2d985b25c98867168f71/images/timefilter30x30.png) This will filter the events view so that only events in the interval between the cursors are displayed.
* ![Reset all filters](https://raw.githubusercontent.com/cunctator/traceshark/808c9a1ed38acfd01e4a2d985b25c98867168f71/images/resetfilters30x30.png) This button resets all filters.
* ![Export filtered events](https://raw.githubusercontent.com/cunctator/traceshark/808c9a1ed38acfd01e4a2d985b25c98867168f71/images/exportevents30x30.png) Opens a dialog that allows the filtered events to be saved to a file. The output format is more or less the same as from perf, so that if one has filtered on the `cycles` events, it's possible to generate a CPU Flame Graph with the tools [here](https://github.com/brendangregg/FlameGraph), or [here](https://github.com/cunctator/FlameGraph). A typical way to use this would be something like this:
  1. Move the blue and red cursors so that they define a time interval that is of interest.
  2. Click on the ![Time filter](https://raw.githubusercontent.com/cunctator/traceshark/808c9a1ed38acfd01e4a2d985b25c98867168f71/images/timefilter30x30.png) button to limit the events to this interval.
  3. Click on the ![Event list](https://raw.githubusercontent.com/cunctator/traceshark/808c9a1ed38acfd01e4a2d985b25c98867168f71/images/eventfilter30x30.png) button. Select the `cycles` event and click on the `Create events filter` button.
  4. Click on the ![task select button](https://raw.githubusercontent.com/cunctator/traceshark/808c9a1ed38acfd01e4a2d985b25c98867168f71/images/taskselector30x30.png) button and select a task or a set of tasks that is of interest. Unselect the `Include wakeup/fork/switch from other PIDs` checkbox. Click on the `Create events filter` button.
  5. Click on the ![Export filtered events](https://raw.githubusercontent.com/cunctator/traceshark/808c9a1ed38acfd01e4a2d985b25c98867168f71/images/exportevents30x30.png) button and type in the filename `filtered.asc`. Click on the `Save` button.
  6. Download the flamegraph software by giving one of the following commands in your home directory:
  ```
  git clone https://github.com/cunctator/FlameGraph.git
  ````
  or
  ```
  git clone https://github.com/brendangregg/FlameGraph.git
  ```
  7. Create the flamegraph of the filtered events:
  ```
  ~/FlameGraph/stackcollapse-perf.pl --kernel  filtered.asc > filtered.folded
  ~/FlameGraph/flamegraph.pl --hash --color=java filtered.folded > filtered.svg
  ```
  8. If all went well, your file `filtered.svg` should now contain an image, that can be displayed by a web browser such as Chromium or Firefox:
![Flamegraph](https://raw.githubusercontent.com/cunctator/traceshark/ce24ad54552cc45ab952c6fc961274fae50105b9/doc/filtered.svg)

The top widget has some buttons as well:

* ![Move the red cursor](https://raw.githubusercontent.com/cunctator/traceshark/808c9a1ed38acfd01e4a2d985b25c98867168f71/images/movered30x30.png) Move the red cursor to the time specified in the time text box.
* ![Add the blue cursor](https://raw.githubusercontent.com/cunctator/traceshark/808c9a1ed38acfd01e4a2d985b25c98867168f71/images/moveblue30x30.png) Move the blue cursor to the time specified in the time text box.
* ![Add to legend](https://raw.githubusercontent.com/cunctator/traceshark/808c9a1ed38acfd01e4a2d985b25c98867168f71/images/addtolegend30x30.png) Adds the currently selected task, from the scheduling graphs, to the legend.
* ![Clear legend](https://raw.githubusercontent.com/cunctator/traceshark/808c9a1ed38acfd01e4a2d985b25c98867168f71/images/clearlegend30x30.png) Removes all tasks from the legend.
* ![Wakeup](https://raw.githubusercontent.com/cunctator/traceshark/808c9a1ed38acfd01e4a2d985b25c98867168f71/images/wakeup30x30.png) This moves the non-active cursor to the immediately preceeding scheduling of the currently selected task. The active cursor is moved to the wakeup event. The selected task is unselected and instead the task of the wakeup event is selected. The events view is scrolled to the wakeup event. This makes it convenient to easily check which task is waiting for which. It is a good idea to double click on the info field of the wakeup event, in order to see the backtrace so that one can determine whether the wakeup was caused by an interrupt or not. If all IRQs are traced it may also be possible to form a conclusion by looking at the surrounding IRQ related events. A typical way to use this button would be something like this:
  1. Choose a task whose wakeup sequence is of interest
  2. Double click to move the cursor to just after the scheduling of interest
  3. Make sure that the task is selected.
  4. Click on the ![Wakeup](https://raw.githubusercontent.com/cunctator/traceshark/808c9a1ed38acfd01e4a2d985b25c98867168f71/images/wakeup30x30.png) button.
  5. Locate the wakup event in the events view. It should be selected.
  6. Double click on the `Info` field to display the backtrace.
  7. If the backtrace leads to an interrupt (including software interrupts), then the wakeup source has been found.
  8. If the backtrace does not lead to an interrupt, then go back to IV.
* ![Add unified task graph](https://raw.githubusercontent.com/cunctator/traceshark/808c9a1ed38acfd01e4a2d985b25c98867168f71/images/addtask30x30.png) Adds a unified scheduling graph for the currently selected task.
* ![Remove unified task graph](https://raw.githubusercontent.com/cunctator/traceshark/808c9a1ed38acfd01e4a2d985b25c98867168f71/images/removetask30x30.png) Removes the currently selected unified graph. 

### 1.1.2 The Events view

At the bottom of the screen is the events view. The events view will be automatically scrolled when a cursor is moved. It is also possible to move the currently active cursor by clicking on a time in the events view. Another very important feature is that by double clicking on the info field, a dialog will open that displays the backtrace of that particular event.

# 2. Building traceshark

The program can be built by doing something like this, assuming that you have
Qt5 development packages installed:

```
qmake-qt5 (or just qmake)
make -j5
```

It is not necessary but you can customize your build by editing traceshark.pro.

For example, you can uncomment the following if you want the try to detect and
optmize for your build machine:

```
# MARCH_FLAG = -march=native
# MTUNE_FLAG = -mtune=native
```

, or you can uncomment some specific flags to build for a certain machine, e.g.
uncomment this to build for Broadwell:

```
# MARCH_FLAG = -march=broadwell
# MTUNE_FLAG = -mtune=broadwell
```

The recommended default compiler is g++ but you can compile with clang if you
like, by uncommenting the following in traceshark.pro:

```
# QMAKE_CXX=clang-6.0
```

If you want to build a debug build, uncomment the following two lines:

```
# DEBUG_FLAG = -g
...
# CONFIG += DEBUG
```

Please note that the software will compile for Qt 4 but that it has not been as
tested with Qt 4. For that reason you might want to build with Qt 5, unless
you happen to prefer Qt 4.

# 3. Capturing a trace

You can get an Ftrace trace to view by doing the following:

```
trace-cmd record -e cpu_frequency -e cpu_idle -e sched_kthread_stop -e sched_kthread_stop_ret -e sched_migrate_task -e sched_move_numa -e sched_pi_setprio -e sched_process_exec -e sched_process_exit -e sched_process_fork -e sched_process_free -e sched_process_wait -e sched_stick_numa -e sched_swap_numa -e sched_switch -e sched_wait_task -e sched_wake_idle_without_ipi -e sched_wakeup -e sched_wakeup_new
```
In order to open it with traceshark, it must first be converted to ASCII:

```
trace-cmd report trace.dat > file_to_open_with_traceshark.asc
```

If you prefer Perf, the trace can be obtained by doing something like this:

```
perf record -e power:cpu_frequency -e power:cpu_idle -e sched:sched_kthread_stop -e sched:sched_kthread_stop_ret -e sched:sched_migrate_task -e sched:sched_move_numa -e sched:sched_pi_setprio -e sched:sched_process_exec -e sched:sched_process_exit -e sched:sched_process_fork -e sched:sched_process_free -e sched:sched_process_wait -e sched:sched_stick_numa -e sched:sched_swap_numa -e sched:sched_switch -e sched:sched_wait_task -e sched:sched_wake_idle_without_ipi -e sched:sched_wakeup -e sched:sched_wakeup_new -a --call-graph=dwarf,65528 -m 128M
```

The `--call-graph=dwarf,65528` option is needed, if you want to get stack traces
for your events. I believe that you can use the `-g` option instead if your
software is compiled with frame pointer. The option `-m 128M` is needed to
increase the memory used by perf. The stack trace of an event will be displayed
by traceshark if you double click on the event's info field in the events view.

In order to get an ASCII representation that can be parsed by traceshark:

```
perf script -f > file_to_open_with_traceshark.asc
```

NB: Your perf program need to be recent enough to work with traceshark, it may
mean that you need to compile perf from the kernel sources of a recent kernel,
rather than the perf that is supplied with your Linux distro.

It seems that some distros provide a perf program that is older than the kernel
in the distro, or somehow a modified perf. This results in a trace being
captured with some events in a different format than expected by traceshark, so
that for example scheduling is not correctly shown.

One approach if the perf provided by your distro doesn't work with traceshark,
is to check the kernel version with "uname -r", then go to kernel.org and
donwload the corresponding mainline kernel and compile perf from the tools/perf
directory in that kernel source tree. If it still doesn't work and you have a
very old kernel, it might work to use perf from a newer kernel, although you
would probably be better off if you upgraded both the kernel and perf but
upgrading the kernel isn't always possible.

I am not exactly sure how recent perf/kernel is necessary but basically I
believe that late 3.X and all 4.X kernels to date should work as long as the
perf program has not been patched.

If you use the '-g' flag, you might also want to compile your own perf because
in some distros perf is compiled without support for backtraces and it starts
working when you compile perf with those bits enabled.

When you compile perf, you get a report like this:

```
Auto-detecting system features:
...                         dwarf: [ on  ]
...            dwarf_getlocations: [ on  ]
...                         glibc: [ on  ]
...                          gtk2: [ OFF ]
...                      libaudit: [ on  ]
...                        libbfd: [ on  ]
...                        libelf: [ on  ]
...                       libnuma: [ on  ]
...        numa_num_possible_cpus: [ on  ]
...                       libperl: [ on  ]
...                     libpython: [ on  ]
...                      libslang: [ on  ]
...                     libcrypto: [ on  ]
...                     libunwind: [ OFF ]
...            libdw-dwarf-unwind: [ on  ]
...                          zlib: [ on  ]
...                          lzma: [ on  ]
...                     get_cpuid: [ on  ]
...                           bpf: [ on  ]
```

I believe that for backtraces to work, it's desirable that as many as possible
of those dwarf, bfd, elf, and unwind related options are enabled. They tend to
get automatically enabled if you have the necessary development packages
installed on your machine. Here is a list of development packages that you can
try to install on Debian Stretch:

```
binutils-dev
binutils-multiarch-dev
bison
elfutils
flex
libaudit-dev
libbfd-dev
libdw-dev
libelf-dev
libelf1
libgtk2.0-dev
libiberty-dev
liblzma-dev
libnuma-dev
libnuma-dev
libperl-dev
libslang-dev
libslang2
libunwind*
libunwind8
python-dev
```