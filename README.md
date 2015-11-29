traceshark
==========

This is an early version of a visualizer for Ftrace and Perf events that can
be captured by the Linux kernel. It visualizes the following events:
cpu_frequency
sched_migrate_task
sched_process_exit
sched_process_fork
sched_switch
sched_wakeup
sched_wakeup_new

Over time, I hope to add more events.

The program can be built by doing something like this, assuming that you have
Qt4 development packages installed:

qmake-qt4

make -j5

You can get an Ftrace trace to view by doing the following:

trace-cmd record -e cpu_frequency -e cpu_idle -e sched_kthread_stop -e sched_kthread_stop_ret -e sched_migrate_task -e sched_move_numa -e sched_pi_setprio -e sched_process_exec -e sched_process_exit -e sched_process_fork -e sched_process_free -e sched_process_hang -e sched_process_wait -e sched_stick_numa -e sched_swap_numa -e sched_switch -e sched_wait_task -e sched_wake_idle_without_ipi -e sched_wakeup -e sched_wakeup_new

trace-cmd report trace.dat > file_to_open_with_traceshark.asc

If you prefer Perf, the trace can be obtained by doing something like this:

perf record -e power:cpu_frequency -e power:cpu_idle -e sched:sched_kthread_stop -e sched:sched_kthread_stop_ret -e sched:sched_migrate_task -e sched:sched_move_numa -e sched:sched_pi_setprio -e sched:sched_process_exec -e sched:sched_process_exit -e sched:sched_process_fork -e sched:sched_process_free -e sched:sched_process_hang -e sched:sched_process_wait -e sched:sched_stick_numa -e sched:sched_swap_numa -e sched:sched_switch -e sched:sched_wait_task -e sched:sched_wake_idle_without_ipi -e sched:sched_wakeup -e sched:sched_wakeup_new -a

Append the '-g' flag to the end of previous command, if you want to get stack
traces for your events. The stack trace of an event will be displayed by
traceshark if you double click on the event's info field in the events view.

perf script -f > file_to_open_with_traceshark.asc

NB: Your perf program need to be recent enough to work with traceshark, that is
compiled from a the kernel sources of a recent kernel, rather than the one
supplied with your Linux distro.