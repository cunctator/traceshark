traceshark
==========

This is an early version of an Ftrace visualizer that visualizes the following
events:
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

You can get a trace to view by doing the following:

trace-cmd record -e cpu_frequency -e cpu_idle -e sched_kthread_stop -e sched_kthread_stop_ret -e sched_migrate_task -e sched_move_numa -e sched_pi_setprio -e sched_process_exec -e sched_process_exit -e sched_process_fork -e sched_process_free -e sched_process_hang -e sched_process_wait -e sched_stick_numa -e sched_swap_numa -e sched_switch -e sched_wait_task -e sched_wake_idle_without_ipi -e sched_wakeup -e sched_wakeup_new

trace-cmd report trace.dat > file_to_open_with_traceshark.asc
