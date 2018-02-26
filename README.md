# Introduction to Traceshark

This is an early version of a graphical viewer for Ftrace and Perf events that
can be captured by the Linux kernel. It visualizes the following events:

```
cpu_frequency
sched_migrate_task
sched_process_exit
sched_process_fork
sched_switch
sched_wakeup
sched_wakeup_new
```

Over time, I hope to add more events.

# Building traceshark

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

# Capturing a trace

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