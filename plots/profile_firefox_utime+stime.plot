set terminal pngcairo enhanced rounded color size 800,500
set output "profile_firefox_utime+stime.png"
#set terminal svg enhanced rounded size 1600,1000 background rgb "white"
#set output "profile_firefox_utime+stime.svg"

set key left top box

set border 3 back ls -1
set tics nomirror
set grid front lt 0 lw 1

set style fill transparent solid 0.5 border

set xlabel "run time [s]"
set xrange[0:]
set xtics 0.5

set ylabel "time [%]"
set ytics
set yrange[0:]

set y2label "threads"
set y2tics
set y2range[0:]

set datafile separator ","
plot "profile_firefox.txt" using 19:13 axes x1y1 title "user mode" with filledcurves x1 linecolor 2,\
     "profile_firefox.txt" using 19:14 axes x1y1 title "kernel mode" with filledcurves x1 linecolor 1,\
     "profile_firefox.txt" using 19:17 axes x1y2 title "threads" with lines linecolor 3
