set terminal pngcairo enhanced rounded color size 800,500
set key left top box

set border 3 back ls -1
set tics nomirror
set grid front lt 0 lw 1

set style fill transparent solid 0.5 border

set xlabel "run time [s]"
set xrange[0:]
#set xtics 0.5

set ytics
set yrange[0:]

set datafile separator ","


# plot I/O
set output "profile_io.png"
set ylabel "read/write [MB/s]"
plot "data.txt" using 19:($29/1024/1024) axes x1y1 title "read" with filledcurves x1 linecolor 1,\
     "data.txt" using 19:($33/1024/1024) axes x1y1 title "write" with filledcurves x1 linecolor 3,\
     "data.txt" using 19:35 axes x1y2 title "read calls" with filledcurves x1 linecolor 4,\
     "data.txt" using 19:37 axes x1y2 title "write calls" with filledcurves x1 linecolor 5


set y2tics
set y2range[0:]


# plot CPU and memory
set output "profile_cpu+mem.png"
set ylabel "CPU [%]"
set y2label "memory [MB]"
plot "data.txt" using 19:($20/1024) axes x1y2 title "VirtMem" with filledcurves x1 linecolor 5,\
     "data.txt" using 19:($23/1024) axes x1y2 title "VMemRSS" with filledcurves x1 linecolor 3,\
     "data.txt" using 19:7  axes x1y1 title "Average CPU" smooth csplines linewidth 1.5 linecolor 8, \
     "data.txt" using 19:8  axes x1y1 title "Current CPU" smooth csplines linewidth 1.5 linecolor 1


# plot user/kernel mode and thread count
set output "profile_utime+stime+threads.png"
set ylabel "time [%]"
set y2label "threads"
plot "data.txt" using 19:13 axes x1y1 title "user mode" with filledcurves x1 linecolor 2,\
     "data.txt" using 19:14 axes x1y1 title "kernel mode" with filledcurves x1 linecolor 1,\
     "data.txt" using 19:17 axes x1y2 title "threads" with lines linecolor 3
