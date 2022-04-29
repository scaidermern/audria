# audria - A Utility for Detailed Resource Inspection of Applications

*audria* is a little tool designed to read various program statistics from the */proc* filesystem, including:
  * current and average CPU usage
  * virtual memory usage (peak, resident set, swap etc.)
  * IO load (current/total read/writes with and without buffers)
  * time spent in user/system mode
  * page faults
  * and other information

*audria* can be used to monitor a single process, a group of processes or all currently running processes.
Furthermore it is possible to specify a command to execute in order to watch a process directly from startup.
In contrast to some other tools, *audria* is designed to run in batch mode at very short intervals.
This allows very detailed inspections of the resource usage of a process.

# Usage

*audria* has several command line arguments:

    PID(s)    PID(s) to monitor
    -a        monitor all processes
    -d delay  delay in seconds between intervals (default: 0.5), specify '-1' to use
              2 * kernel clock tick rate (2 * 1/100 on most systems), note: values equal or below
              the kernel clock tick rate will lead to bogus values for the 'CurCPUPerc' field
    -e cmd    program to execute and watch, all remaining arguments will be forwarded
    -f fields names of fields to show, separated by comma (default: all)
    -k        show kernel threads (default: false)
    -n num    number of iterations before quitting (default: unlimited)
    -o file   file to write output to instead of stdout, will append to existing files,
              if file is '-' then output will be written to stdout (default)
    -r        acquire real-time priority (lowest niceness, highest scheduling priority),
              usually requires root privileges or the CAP_SYS_NICE capability
    -s        include self in list of processes to monitor
    -h        print this help and exit

## Example Usage

Usually it is sufficient to just pass the *PID* you want to monitor and optionally the name of the output file:
`audria $(pidof myProgram) -o data.txt`

For deeper inspections you might want to watch the program directly from startup and at a higher interval:
`audria -d -1 -o data.txt -e myProgram -myProgramArgument`

You can also specify which fields to show:
`audria -f Name,CurCPUPerc,Threads,VmSizekB,CurReadBytesPerSec,CurWrittenBytesPerSec -a`

## Plotting

audria generates a CSV-like output which is suitable for plotting.
For the [gnuplot](http://gnuplot.sourceforge.net/) software you can use the example file `example.plot` as a start.
It will plot several graphs based on the generated data, assuming this data has been stored as `data.txt`.
Just install gnuplot and run `gnuplot example.plot` to generate the graphs.

### Example Plots
These are some example plots created with gnuplot from [audria's output](https://raw.github.com/scaidermern/audria/master/plots/profile_firefox.txt).
A *firefox* process was monitored right from the start:

The first plot shows the *CPU* (current + average) and *memory* (allocated + actually used) usage.
Seems like IO is the bottleneck at startup as the CPU is only occasionally active:
![firefox CPU + memory usage](https://raw.github.com/scaidermern/audria/master/plots/profile_firefox_cpu+mem.png)

The second plot visuializes the *IO load*, especially the current number of read/written bytes/s and read/write calls:
![firefox IO usage](https://raw.github.com/scaidermern/audria/master/plots/profile_firefox_io.png)

In the last plot we can see the relative time the process spend in *user* and *kernel mode*, as well as the current number of *threads*:
![firefox user/system time + threads](https://raw.github.com/scaidermern/audria/master/plots/profile_firefox_utime+stime.png)

# License
[GPL v3](http://www.gnu.org/licenses/gpl.html)
(c) [Alexander Heinlein](http://choerbaert.org)
