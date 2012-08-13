# audria - A Utility for Detailed Ressource Inspection of Applications

*audria* is a little tool designed to read various program statistics from the */proc* filesystem, including:
  * current and average CPU usage
  * virtual memory usage (peak, resident set, swap etc.)
  * IO load (current/total read/writes with and without buffers)
  * time spent in user/system mode
  * page faults
  * and other information

In contrast to additional tools, *audria* is designed to run in batch mode at very short intervals.
This allows very detailed inspections of the ressource usage of a process.

# Usage

*audria* has several command line arguments:

    PID(s)   PID(s) to monitor
    -a       monitor all processes
    -d       delay in seconds between intervals (default: 0.5)
    -e       program to execute and watch, all remaining arguments will be forwarded
    -s       include self in list of processes to monitor
    -n       number of iterations before quitting (default: unlimited)
    -r       acquire real-time priority (lowest niceness, highest scheduling priority),
             usually requires root privileges or the CAP_SYS_NICE capability
    -h       print this help and exit

Usually it is sufficient to just pass the *PID* to monitor:  
`audria $(pidof myProgram)`

For deeper inspections you might want to watch the program directly from startup and at a higher interval:  
`audria -d 0.01 -e myProgram -myProgramArgument`

# Example Plots
These are some example plots created with [gnuplot](http://gnuplot.sourceforge.net/) from [audria's output](https://raw.github.com/scaidermern/audria/master/plots/profile_firefox.txt).
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
