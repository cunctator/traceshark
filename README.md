![traceshark logo](https://raw.githubusercontent.com/cunctator/traceshark/882584f48e9c03b85e6a5a6b96725040ef1739f5/images/shark-penguin.png)

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
sched_waking
```

The `sched_waking` events are not really visualized but there is a button to find the `sched_waking` event that has instigated a particular `sched_wakeup` event.

![traceshark screenshot](https://raw.githubusercontent.com/cunctator/traceshark/4d2083d6df569c3bd59a6b64ef210a5c61714cbe/doc/ts-screenshot5.png)

Above is a screenshot of traceshark. The eight uppermost graphs are for displaying CPU idle and frequency states. They are eight because the measurement was made on a system with eight virtual CPUs. The green graphs with red circles ![idle graph](https://raw.githubusercontent.com/cunctator/traceshark/608fdb55d78e7beebecf3a5e036cace07842f2c6/doc/idle-graph.png) show the CPU idle states while the thicker blue graphs ![idle graph](https://raw.githubusercontent.com/cunctator/traceshark/608fdb55d78e7beebecf3a5e036cace07842f2c6/doc/freq-graph.png) show the CPU frequency changes.

Below these eight graphs are the per CPU scheduling graphs, the different colors of these graphs are for different tasks. The small vertical bars that are shown just above the per CPU graphs indicates the waiting time between a task becoming runnable and being scheduled, the highest height is equal to 20 ms, i.e. a full length means that the waiting time was at least 20 ms, possibly more.

 Furthermore, in the scheduling graphs, there are the following subtle markers:
* ![still running example](https://raw.githubusercontent.com/cunctator/traceshark/b552614ebc40656a6cbaef77e21ee95eb4eb32cd/doc/running60x54.png) The blue triangle indicates that the task was still running when it was scheduled out.
* ![preempted example](https://raw.githubusercontent.com/cunctator/traceshark/b552614ebc40656a6cbaef77e21ee95eb4eb32cd/doc/preempted60x54.png) The red triangle indicates that the task was preempted when it was scheduled out.
* ![uninterruptible example](https://raw.githubusercontent.com/cunctator/traceshark/b552614ebc40656a6cbaef77e21ee95eb4eb32cd/doc/unint60x54.png) The magenta colored plus sign indicates that the task went into uninterruptible sleep when it was scheduled out. Note that in practice, only the lower line of the plus sign is visible.

Below the scheduling graphs are the migration graphs. Task migrations between CPUs are shown with arrows. Fork/exit is shown with an arrow from/to `fork/exit`.

Below the migration arrows are the unified task graphs, where tasks are shown without caring about which CPU it is running on. Here the time between becoming runnable and being scheduled in shown by horizontal bars.

These graphs will only be shown if requested by the user. It is necessary to select a task and click the ![Add task graph button](https://raw.githubusercontent.com/cunctator/traceshark/608fdb55d78e7beebecf3a5e036cace07842f2c6/images/addtask30x30.png) button or the `Add a unified graph` button in the task select dialog.

The task select dialog can be shown by clicking `View -> Show task list`, or by clicking the dedicated ![show task dialog button](https://raw.githubusercontent.com/cunctator/traceshark/608fdb55d78e7beebecf3a5e036cace07842f2c6/images/taskselector30x30.png) button for it on the left panel.

## 1.1 Brief summary of the functionality of the GUI

### 1.1.1 How to zoom and scroll vertically

The graphs are by default zoomed and scrolled horizontally, i.e. time wise. You can scroll by grabbing the graph with your mouse pointer and zoom with the mouse wheel.

If you instead want to zoom or scroll vertically, you need to toggle the ![Toggle vertical zoom](https://raw.githubusercontent.com/cunctator/traceshark/67651b9c424d938a1cef99e60e81e948486439fa/images/verticalzoom30x30.png) button.

Another option is to select the vertical axis by left clicking on it with your mouse pointer. N.B, you should click directly on the line representing the axis, not on the labels, such as "cpu0", "cpu1", etc. As long as the vertical axis is selected, all scrolling and zooming will be vertical. If you want to switch back to horizontal, then you just need to deselect the axis by clicking on it again. Vertical zooming and scrolling may be particularly useful if you are looking at a trace of a system with a large number of CPUs or if you are short of vertical screen space.

### 1.1.2 Functionality of the menus

The items in the menus are in general duplicated as buttons. However, there is one exception, those that are in the ```Event``` menu.

![traceshark screenshot](https://raw.githubusercontent.com/cunctator/traceshark/d31d91d0dcd39eea830de1fd9172bf9d309b2bc9/doc/menu_event.png)

Above is a screenshot of the ```Event``` menu. For these items, there are no push buttons in the GUI. However, these actions can also be triggered by double clicking on the corresponding column of the currently selected event in the events view. Below is a brief explanation of these menu items:

* ![backtrace button](https://raw.githubusercontent.com/cunctator/traceshark/d31d91d0dcd39eea830de1fd9172bf9d309b2bc9/images/eventbacktrace30x30.png)```Show backtrace```: Shows a dialog window containing the backtrace of the currently selected event. The backtrace dialog window will look somewhat like this:
![close button](https://raw.githubusercontent.com/cunctator/traceshark/d31d91d0dcd39eea830de1fd9172bf9d309b2bc9/doc/backtrace.png)
* ![moveblue button](https://raw.githubusercontent.com/cunctator/traceshark/d31d91d0dcd39eea830de1fd9172bf9d309b2bc9/images/eventmoveblue30x30.png)```Move blue cursor```: Move the blue cursor to the time of the selected event.
* ![movered button](https://raw.githubusercontent.com/cunctator/traceshark/d31d91d0dcd39eea830de1fd9172bf9d309b2bc9/images/eventmovered30x30.png)```Move red cursor```: Move the red cursor to the time of the selected event.
* ![filterpid button](https://raw.githubusercontent.com/cunctator/traceshark/d31d91d0dcd39eea830de1fd9172bf9d309b2bc9/images/eventfilterpid30x30.png)```Filter on event PID```: Filter the events view on the PID of the selected event.
* ![filtercpu button](https://raw.githubusercontent.com/cunctator/traceshark/d31d91d0dcd39eea830de1fd9172bf9d309b2bc9/images/eventfiltercpu30x30.png)```Filter on event CPU```: Filter the events view on the CPU of the selected event.
* ![filtertype button](https://raw.githubusercontent.com/cunctator/traceshark/d31d91d0dcd39eea830de1fd9172bf9d309b2bc9/images/eventfiltertype30x30.png)```Filter on event type```: Filter the events view on the type of the selected event.

### 1.1.3 Functionality of the buttons

There are a number of buttons in the GUI. These buttons are also duplicated in the menus.  Here is a description of the buttons in the left panel:

* ![open button](https://raw.githubusercontent.com/cunctator/traceshark/608fdb55d78e7beebecf3a5e036cace07842f2c6/images/open30x30.png) This button is used to open a trace file.
* ![close button](https://raw.githubusercontent.com/cunctator/traceshark/b2d0b868ea4947d89240dd627f05bb77bd51ce03/images/close30x30.png) Closes the currently open trace.
* ![screenshot button](https://raw.githubusercontent.com/cunctator/traceshark/608fdb55d78e7beebecf3a5e036cace07842f2c6/images/screenshot30x30.png) Take a screenshot of the plot and save it to a file.
* ![task select button](https://raw.githubusercontent.com/cunctator/traceshark/608fdb55d78e7beebecf3a5e036cace07842f2c6/images/taskselector30x30.png) Show the task select dialog. This makes it possible to filter the events view by task, to show a task in the legend, or to show a unified graph. This button is very useful when the user knows the name of a task of interest but cannot find it easily among the scheduling graphs.
* ![cpu filter button](https://raw.githubusercontent.com/cunctator/traceshark/5fbe59adad7cfa603337ef9ec1999e026851066f/images/cpufilter30x30.png) Show a list of CPUs and it's possible to filter the events view by CPU.
* ![Event list](https://raw.githubusercontent.com/cunctator/traceshark/608fdb55d78e7beebecf3a5e036cace07842f2c6/images/eventfilter30x30.png) Show a list of the different event types and it's possible to filter the events view by event type.
* ![Regex dialog](https://raw.githubusercontent.com/cunctator/traceshark/76fe91ec9155f5d67fb4d9663720a05c3a1f6e9b/images/argfilter30x30.png) Show a dialog that lets the user input one or several regular expressions and it is possible to filter the events view with these. The filtering only applies to what is in the Info field. The filtering is separately applied to the different words, which are delimited by spaces. The filtering will by default try with all words but it is also possible to select a certain position. The position may be from the start or from the match of the previous regular expression.
* ![Time filter](https://raw.githubusercontent.com/cunctator/traceshark/76fe91ec9155f5d67fb4d9663720a05c3a1f6e9b/images/timefilter30x30.png) This will filter the events view so that only events in the interval between the cursors are displayed.
* ![Reset all filters](https://raw.githubusercontent.com/cunctator/traceshark/76fe91ec9155f5d67fb4d9663720a05c3a1f6e9b/images/resetfilters30x30.png) This button resets all filters.
* ![Export filtered events](https://raw.githubusercontent.com/cunctator/traceshark/608fdb55d78e7beebecf3a5e036cace07842f2c6/images/exportevents30x30.png) Opens a dialog that allows the filtered events to be saved to a file. The output format is more or less the same as from perf, so that if one has filtered on the `cycles` events, it's possible to generate a CPU Flame Graph with the tools [here](https://github.com/brendangregg/FlameGraph), or [here](https://github.com/cunctator/FlameGraph). A typical way to use this would be something like this:
  1. Move the blue and red cursors so that they define a time interval that is of interest.
  2. Click on the ![Time filter](https://raw.githubusercontent.com/cunctator/traceshark/608fdb55d78e7beebecf3a5e036cace07842f2c6/images/timefilter30x30.png) button to limit the events to this interval.
  3. Click on the ![Event list](https://raw.githubusercontent.com/cunctator/traceshark/608fdb55d78e7beebecf3a5e036cace07842f2c6/images/eventfilter30x30.png) button. Select the `cycles` event and click on the `Create events filter` button.
  4. Click on the ![task select button](https://raw.githubusercontent.com/cunctator/traceshark/608fdb55d78e7beebecf3a5e036cace07842f2c6/images/taskselector30x30.png) button and select a task or a set of tasks that is of interest. Click on the `Create events filter` button.
  5. Click on the ![Export filtered events](https://raw.githubusercontent.com/cunctator/traceshark/608fdb55d78e7beebecf3a5e036cace07842f2c6/images/exportevents30x30.png) button and type in the filename `filtered.asc`. Click on the `Save` button.
  6. Download the flamegraph software by giving one of the following commands in your home directory:
  ```
  git clone https://github.com/cunctator/FlameGraph.git
  ```
  or
  ```
  git clone https://github.com/brendangregg/FlameGraph.git
  ```
  7. Create the flamegraph of the filtered events:
  ```
  ~/FlameGraph/stackcollapse-perf.pl --kernel  filtered.asc > filtered.folded
  ~/FlameGraph/flamegraph.pl --hash --color=java filtered.folded > filtered.svg
  ```
  8. If all went well, your file `filtered.svg` should now contain an image, that can be displayed by a web browser such as Chromium or Firefox. For an example, check the file doc/filtered.svg in this repository, it can be zoomed, unlike the png representation below:
  [![filtered.png](https://raw.githubusercontent.com/cunctator/traceshark/608fdb55d78e7beebecf3a5e036cace07842f2c6/doc/filtered.png)](https://raw.githubusercontent.com/cunctator/traceshark/608fdb55d78e7beebecf3a5e036cace07842f2c6/doc/filtered.png)
  You can read more about flame graphs [here](http://www.brendangregg.com/flamegraphs.html).
* ![Export filtered CPU events](https://raw.githubusercontent.com/cunctator/traceshark/c7168ab2ffffd65f3b87ae6b65c2f9c6a7daed6b/images/exportcpuevents30x30.png) This functions exactly as the previously mentioned ![Export filtered events](https://raw.githubusercontent.com/cunctator/traceshark/c7168ab2ffffd65f3b87ae6b65c2f9c6a7daed6b/images/exportevents30x30.png) button, except that it only exports cycles events. The benefit is that the user doesn't need to separately filter on cycles events. The downside is that if the events of interest have a slightly different name, nothing will be exported. This could be the case with certain kernel versions, particularly heavily patched vendor kernels.
* ![Cursor zoom](https://raw.githubusercontent.com/cunctator/traceshark/c7168ab2ffffd65f3b87ae6b65c2f9c6a7daed6b/images/cursorzoom30x30.png) Pressing this button will zoom the plot to the time interval that is defined by the cursors. This feature may be especially useful when a very large file has been opened and the response to mouse zooming is sluggish.
* ![Full zoom](https://raw.githubusercontent.com/cunctator/traceshark/d83b961645e038f55b39d78f2cc9768c8b6a0c2c/images/fullzoom30x30.png) Pressing this button will zoom the plot to the full time interval, that is from the beginning to the end of the trace. The height will also be adjusted so that everything is shown vertically too.
* ![Default zoom](https://raw.githubusercontent.com/cunctator/traceshark/d83b961645e038f55b39d78f2cc9768c8b6a0c2c/images/defaultzoom30x30.png) Pressing this button will zoom the plot to the default zoom. This is the same as the full zoom, except that if there is not enough vertical screen space for displaying the trace without cluttering, then vertically only a part  of the trace will be displayed and a vertical scroll bar will be displayed.
* ![Toggle vertical zoom](https://raw.githubusercontent.com/cunctator/traceshark/67651b9c424d938a1cef99e60e81e948486439fa/images/verticalzoom30x30.png) Pressing this button will toggle the vertical zooming and scrolling. The effect is the same as toggling the selection of the vertical axis.
* ![Show scheduling latencies](https://raw.githubusercontent.com/cunctator/traceshark/758009355ebe32897aa5b1854e5033da1dd3d27d/images/latency30x30.png) Pressing this button will show a list of scheduling latencies. If a latency in the list is double clicked, then the task in question will be selected and the cursors will be moved to show the latency. The latencies are sorted so that the biggest latencies are displayed first. A scheduling latency is the delay between a task becoming runnable and being scheduled.
* ![Show wakeup latencies](https://raw.githubusercontent.com/cunctator/traceshark/758009355ebe32897aa5b1854e5033da1dd3d27d/images/latency-wakeup30x30.png) Pressing this button will show a list of wakeup latencies. It works in the same way as the scheduling latencies button but shows wakeup latencies instead. Wakeup latencies only include the delay between a wakeup event and a task being scheduled. If a task is being scheduled out in a runnable state, then the delay until it being scheduled again is not included. If you want these latencies included, then you should use the previously mentioned ![Show scheduling latencies](https://raw.githubusercontent.com/cunctator/traceshark/758009355ebe32897aa5b1854e5033da1dd3d27d/images/latency30x30.png) button instead.
* ![Select which types of graphs should be enabled](https://raw.githubusercontent.com/cunctator/traceshark/608fdb55d78e7beebecf3a5e036cace07842f2c6/images/graphenabledialog30x30.png) Pressing this button will open a dialog that allows the user to select which types of graphs will be displayed. Here it is possible to disable certain graphs, for example CPU idle graphs that frequently may be of little interest. It is also possible to enable horizontal latency graphs for the per CPU task graphs that are disabled by default, because they will frequently overlap each other. If OpenGL is enabled at compile time, then it is possible to select the desired line width of the scheduling graphs. Otherwise, the line width will always be set to 1. The dialog has an `Apply & Save` button that allows the user to save the settings to `$HOME/.traceshark`, so that they will be remembered the next time traceshark is started.
* ![Show global statistics](https://github.com/cunctator/traceshark/raw/608fdb55d78e7beebecf3a5e036cace07842f2c6/images/getstats30x30.png) Pressing this button will show a dialog with global statistics on the left side of the plot. The statistics show how many % of the CPU time each task has consumed as well as the time consumed. The percentages are percentage of a core. This means that the maximum for a normal task is 100% and for the idle task, swapper with pid 0, the maximum is `N * 100`, where N is the number of CPUs. The tasks are sorted so that those tasks that consume more CPU time are shown earlier. The dialog has buttons for adding selected tasks to the legend, to add them as task graphs, to filter the events view on them, or to reset the filtering. There is even a button to close the dialog.
* ![Show statistics limited by cursors](https://github.com/cunctator/traceshark/raw/608fdb55d78e7beebecf3a5e036cace07842f2c6/images/getstatstimelimit30x30.png) Pressing this button will show a dialog with the same statistics as the previous button but the scope will not be the whole trace but the time between cursors and the statistics will be shown on the right side of the plot. If the cursors are moved while the dialog is shown, then the statistics will be updated accordingly.

The top widget has some buttons as well:

* ![Move the red cursor](https://raw.githubusercontent.com/cunctator/traceshark/608fdb55d78e7beebecf3a5e036cace07842f2c6/images/movered30x30.png) Move the red cursor to the time specified in the time text box.
* ![Add the blue cursor](https://raw.githubusercontent.com/cunctator/traceshark/608fdb55d78e7beebecf3a5e036cace07842f2c6/images/moveblue30x30.png) Move the blue cursor to the time specified in the time text box.
* ![Add to legend](https://raw.githubusercontent.com/cunctator/traceshark/608fdb55d78e7beebecf3a5e036cace07842f2c6/images/addtolegend30x30.png) Adds the currently selected task, from the scheduling graphs, to the legend. A task can be removed from the legend by double clicking on it in the legend.
* ![Clear legend](https://raw.githubusercontent.com/cunctator/traceshark/b2d0b868ea4947d89240dd627f05bb77bd51ce03/images/clearlegend30x30.png) Removes all tasks from the legend.
* ![Find Wakeup](https://raw.githubusercontent.com/cunctator/traceshark/608fdb55d78e7beebecf3a5e036cace07842f2c6/images/wakeup30x30.png) This moves the non-active cursor to the immediately preceding scheduling of the currently selected task. The active cursor is moved to the wakeup event. The selected task is unselected and instead the task of the wakeup event is selected. The events view is scrolled to the wakeup event. This makes it convenient to easily check which task is waiting for which. It is a good idea to double click on the info field of the wakeup event, in order to see the backtrace so that one can determine whether the wakeup was caused by an interrupt or not. If all IRQs are traced it may also be possible to form a conclusion by looking at the surrounding IRQ related events. A typical way to use this button would be something like this:
  1. Choose a task whose wakeup sequence is of interest
  2. Double click to move the cursor to just after the scheduling of interest
  3. Make sure that the task is selected.
  4. Click on the ![Wakeup](https://raw.githubusercontent.com/cunctator/traceshark/608fdb55d78e7beebecf3a5e036cace07842f2c6/images/wakeup30x30.png) button.
  5. Locate the wakeup event in the events view. It should be selected.
  6. Double click on the `Info` field to display the backtrace. Alternatively, you can use the ```Event->Show backtrace``` menu item.
  7. If the backtrace leads to an interrupt (including software interrupts), then the wakeup source has been found.
  8. If the `sched_wakeup` event is executed as pid 0, that is no new task has been auto-selected, and/or, the backtrace leads to something like this:
     ```
     2863f8 ttwu_do_wakeup
     286522 ttwu_do_activate
     2878a7 sched_ttwu_pending
     29fec6 do_idle
     2a00ff cpu_startup_entry
     237418 start_secondary
     2000d5 [unknown]
     ```
     Then it's necessary to find the preceding `sched_waking` event and study its backtrace instead. Click on the ![Find Waking](https://github.com/cunctator/traceshark/raw/608fdb55d78e7beebecf3a5e036cace07842f2c6/images/waking30x30.png) button and go back to VI.
  9. If none of the above is true, then go back to IV.
* ![Find Waking](https://github.com/cunctator/traceshark/raw/608fdb55d78e7beebecf3a5e036cace07842f2c6/images/waking30x30.png) This is used to find the `sched_waking` event that is associated with and precedes a particular `sched_wakeup` event. A `sched_wakeup` event must be selected in the events view for this button to be enabled.
* ![Find Waking Direct](https://raw.githubusercontent.com/cunctator/traceshark/608fdb55d78e7beebecf3a5e036cace07842f2c6/images/wakingdirect30x30.png) This is used to find the `sched_waking` event directly, without first finding the `sched_wakeup` event. It operates in the same way as the ![Find Wakeup](https://raw.githubusercontent.com/cunctator/traceshark/608fdb55d78e7beebecf3a5e036cace07842f2c6/images/wakeup30x30.png) button, based on the currently selected task and the position of the currently active cursor. If you use this button, you should remember that tasks might sometimes wake up without a `sched_waking` event. One example is when new tasks are woken up by the `sched_wakeup_new` event; in that case there is no corresponding `sched_waking` event.
* ![Find next sched_switch sleep event](https://raw.githubusercontent.com/cunctator/traceshark/608fdb55d78e7beebecf3a5e036cace07842f2c6/images/findsleep30x30.png) This is used to find the next sched_switch event that puts the currently selected task to sleep. It is sometimes desirable to see what kind of code makes a task go to sleep; this is particularly useful together with the feature that the backtrace can be displayed by double clicking on the event's info field in the events view.
* ![Add unified task graph](https://raw.githubusercontent.com/cunctator/traceshark/608fdb55d78e7beebecf3a5e036cace07842f2c6/images/addtask30x30.png) Adds a unified scheduling graph for the currently selected task.
* ![Remove unified task graph](https://raw.githubusercontent.com/cunctator/traceshark/608fdb55d78e7beebecf3a5e036cace07842f2c6/images/removetask30x30.png) Removes the currently selected unified graph.
* ![Remove all unified task graphs](https://raw.githubusercontent.com/cunctator/traceshark/dbf92937590e3987632d60c0ccc4119f7b06284a/images/cleartasksgraphs30x30.png) This button removes all unified graphs that have been added to the plot.
* ![Filter on the current task](https://raw.githubusercontent.com/cunctator/traceshark/608fdb55d78e7beebecf3a5e036cace07842f2c6/images/filtercurrent30x30.png) This will filter the events view so that only events involving the currently selected task will be displayed.
* ![Filter on the current task time limited](https://raw.githubusercontent.com/cunctator/traceshark/608fdb55d78e7beebecf3a5e036cace07842f2c6/images/filtercurrentlimited30x30.png) This will do the same as the previous, except that in addition it will exclude those events that are outside the interval defined by the positions of the cursors.

### 1.1.4 The Events view

At the bottom of the screen is the events view. The events view will be automatically scrolled when a cursor is moved. It is also possible to move the currently active cursor by double clicking on a time in the events view. Another very important feature is that by double clicking on the info field, a dialog will open that displays the backtrace of that particular event. In general, it is  possible to trigger the actions in the ```Event``` menu by double clicking on the corresponding column of the currently selected event.

# 2. Building traceshark

## 2.1 How to set up your build environment

In order to build traceshark, you will need three things:
* A C++ compiler
* make
* The development packages for Qt

On Ubuntu 16.04, Ubuntu 18.04, Ubuntu 20.04, Debian 9, and Debian 10, you can install these like this:

```
sudo apt-get install qt5-default g++ make
```

On Ubuntu 22.04 and Debian 11, you can install these like this:

```
sudo apt install qtbase5-dev g++ make
```

On Fedora (tested with Fedora 32), you can do the following:

```
sudo dnf install qt5-qtbase-devel g++ make
```

It is not recommended but if you plan to configure your build to use the QCustomPlot library on your distro instead of the patched built-in version, then you will need to install the relevant development package. On Ubuntu 20.04 and Debian 10:

```
sudo apt-get install libqcustomplot-dev
```

The QCustomPlot library on Debian 9 and Ubuntu 18.04 is too old for traceshark.

On Fedora 32:
```
sudo dnf install qcustomplot-qt5-devel
```

On macOS, you will need to:
* Install Xcode and macports as described at [macports.org](https://www.macports.org)
* Install the desired Qt version, for example Qt 5.13:
```
sudo port install qt513
```
* You need to find the directory where qmake is and add it to your path. Something like this:
````
echo 'export PATH=/opt/local/viktor/bin:$PATH:/opt/local/libexec/qt5/bin' >> $HOME/.zshrc
````

## 2.2 How to compile and install

The program can be compiled and installed by doing something like this:

```
qmake-qt5 (on some distros you should use just qmake instead of qmake-qt5)
make -j5
sudo make install
```

On macOS, the ```sudo make install``` doesn't work. You can find the executable in ```traceshark.app/Contents/MacOS/traceshark```. Running traceshark on macOS is currently in a quite experimental state.

## 2.3 How to configure your build

It is not necessary but you can tweak your build by editing traceshark.pro. One of the most important options is that you can disable OpenGL support. If and only if OpenGL support is enabled, then it is possible for the user to select the line width of the scheduling graphs, otherwise the line width will always be set to 1. OpenGL is enabled at compile time by default. If it has been enabled at compile time, then it will be enabled by default when running the application but only if the screen is deemed to be a high resolution screen.  The user can enable or disable OpenGL at runtime by opening the dialog with the ![Select which types of graphs should be enabled](https://raw.githubusercontent.com/cunctator/traceshark/608fdb55d78e7beebecf3a5e036cace07842f2c6/images/graphenabledialog30x30.png) button. If you run into rendering problems, including problems with very slow rendering, then disabling OpenGL might be worth trying. OpenGL can be disabled at compile time by uncommenting the following line in traceshark.pro:

```
# DISABLE_OPENGL = yes
```

You can uncomment the following if you want the try to detect and optimize for your build machine:

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

The recommended default compiler is g++ but you can compile with clang, or another version of g++, if you like, by uncommenting and possibly editing one of the following in traceshark.pro:

```
# USE_ALTERNATIVE_COMPILER = clang++-6.0
# USE_ALTERNATIVE_COMPILER = g++-8
```

If you want to build a debug build, uncomment one of the following two lines:

```
# Uncomment this for debug build:
# USE_DEBUG_FLAG = -g

# Uncomment this for debug build without optimization:
# USE_DEBUG_FLAG = -g -O0
```

This is not recommended because it will very likely result in worse performance but if you absolutely want to use the QCustomPlot library on your system, then you need to uncomment the following line:

```
# USE_SYSTEM_QCUSTOMPLOT = yes
```

Please note that the software will compile for Qt 4 but that it has not been as
tested with Qt 4. For that reason you might want to build with Qt 5, unless
you happen to prefer Qt 4.

# 3. Obtaining a trace

There are two ways to capture a trace: Ftrace and perf. Perf is the recommended method because it is able to generate backtraces that are understood by traceshark. However, Ftrace has the benefit that it almost always works right out of the box on many distros. Nowadays, perf usually works right out of the box too but it was not always the case in the past.

For both Ftrace and perf it is very desirable to avoid lost events because traceshark cannot visualize correctly with lost events, nor can it find a wakeup event that has been lost.

## 3.1 Sample traces

If you are not anxious to trace anything in particular but only want to play around with traceshark, then you can find sample traces [here](https://github.com/cunctator/traceshark-resources), or just clone the repo with the samples:
```
git clone https://github.com/cunctator/traceshark-resources.git
```

## 3.2 Capturing a trace with Ftrace

You can get an Ftrace trace to view by doing the following:

```
trace-cmd record -e cpu_frequency -e cpu_idle -e sched_kthread_stop -e sched_kthread_stop_ret -e sched_migrate_task -e sched_move_numa -e sched_pi_setprio -e sched_process_exec -e sched_process_exit -e sched_process_fork -e sched_process_free -e sched_process_wait -e sched_stick_numa -e sched_swap_numa -e sched_switch -e sched_wait_task -e sched_wake_idle_without_ipi -e sched_wakeup -e sched_wakeup_new -e sched_waking
```

If you get problem with lost events, then you may want to try the `-r` and `-b` options. For example:
```
trace-cmd record -e cpu_frequency -e cpu_idle -e sched_kthread_stop -e sched_kthread_stop_ret -e sched_migrate_task -e sched_move_numa -e sched_pi_setprio -e sched_process_exec -e sched_process_exit -e sched_process_fork -e sched_process_free -e sched_process_wait -e sched_stick_numa -e sched_swap_numa -e sched_switch -e sched_wait_task -e sched_wake_idle_without_ipi -e sched_wakeup -e sched_wakeup_new -e sched_waking -b 32768 -r 99
```

The above will use kernel buffers that are a whopping 32 MB per cpu and run the capture threads with a real time priority of 99. You may want to adjust these values to suit your system.

You may also want to add additional events to the above command that are of interest to your software. You can get a list of available events by running the following command as root:
```
trace-cmd list
```
In order to open the trace with traceshark, it must first be converted to ASCII:
```
trace-cmd report trace.dat > file_to_open_with_traceshark.asc
```

## 3.3 Capturing a trace with perf

With perf you may also want to consider additional events. A list of all events can be obtained by running the following command as root:
```
perf list
```

A perf trace can be obtained by doing something like this:

```
perf record -e power:cpu_frequency -e power:cpu_idle -e sched:sched_kthread_stop -e sched:sched_kthread_stop_ret -e sched:sched_migrate_task -e sched:sched_move_numa -e sched:sched_pi_setprio -e sched:sched_process_exec -e sched:sched_process_exit -e sched:sched_process_fork -e sched:sched_process_free -e sched:sched_process_wait -e sched:sched_stick_numa -e sched:sched_swap_numa -e sched:sched_switch -e sched:sched_wait_task -e sched:sched_wake_idle_without_ipi -e sched:sched_wakeup -e sched:sched_wakeup_new -e sched:sched_waking -e cycles -a --call-graph=dwarf,20480 -m 128M
```

The `--call-graph=dwarf,20480` option is needed, if you want to get stack traces for your events. You might need to adjust the size 20480, the maximum is 65528. The benefit with larger sizes is that you can capture bigger stacks, the downside is that the traces will be larger, tracing will have more overhead, and the probability that perf will lose some events is higher. I believe that you can use the `-g` option instead if your software is compiled with frame pointers.

The option `-m 128M` is needed to increase the memory used by perf for buffering in order to avoid lost events, especially when using the `--call-graph` option. This is necessary because traceshark doesn't cope well with lost events.

The stack trace of an event will be displayed by traceshark if you double click on the event's info field in the events view.

Typing the above commands every time may be error prone and tedious, for this reason, there is the [perf-record.sh](https://raw.githubusercontent.com/cunctator/traceshark/de71cc2f7982f3fe11f20da8c44c0ecaae16453f/scripts/perf-record.sh) script in the [scripts](https://github.com/cunctator/traceshark/tree/de71cc2f7982f3fe11f20da8c44c0ecaae16453f/scripts) directory.

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
download the corresponding mainline kernel and compile perf from the tools/perf
directory in that kernel source tree. If it still doesn't work and you have a
very old kernel, it might work to use perf from a newer kernel, although you
would probably be better off if you upgraded both the kernel and perf but
upgrading the kernel isn't always possible.

I am not exactly sure how recent perf/kernel is necessary but basically I
believe that late 3.X and all 4.X and 5.x kernels to date should work as long
as the perf program has not been patched.

If you use the '-g' flag, you might also want to compile your own perf because
in some distros perf is compiled without support for backtraces and it starts
working when you compile perf with those bits enabled. Fortunately, nowadays
it's common that the perf utility shipped with the distro support backtraces.

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
installed on your machine.

On Ubunut Bionic and Debian Buster/Bullseye the following might work:
```
sudo apt-get install binutils-dev binutils-multiarch-dev bison elfutils flex libaudit-dev libbfd-dev libdw-dev libelf-dev libelf1 libgtk2.0-dev libiberty-dev liblzma-dev libnuma-dev libperl-dev libslang2-dev libslang2 'libunwind*' libunwind8 python-dev libzstd-dev libcap-dev
```
