set terminal pngcairo enhanced rounded color size 800,500
set output "profile_firefox_cpu+mem.png"
#set terminal svg enhanced rounded size 1600,1000 background rgb "white"
#set output "profile_firefox_cpu+mem.svg"

set key left top box

set border 3 back ls -1
set tics nomirror
set grid front lt 0 lw 1

set style fill transparent solid 0.5 border

set xlabel "run time [s]"
set xrange[0:]
set xtics 0.5

set ylabel "CPU [%]"
set ytics
set yrange[0:]

set y2label "memory [MB]"
set y2tics
set y2range[0:]

set datafile separator ","

plot "profile_firefox.txt" using 19:($20/1024) axes x1y2 title "VirtMem" with filledcurves x1 linecolor 5,\
     "profile_firefox.txt" using 19:($23/1024) axes x1y2 title "VMemRSS" with filledcurves x1 linecolor 3,\
     "profile_firefox.txt" using 19:7  axes x1y1 title "Average CPU" smooth csplines linewidth 1.5 linecolor 8, \
     "profile_firefox.txt" using 19:8  axes x1y1 title "Current CPU" smooth csplines linewidth 1.5 linecolor 1

