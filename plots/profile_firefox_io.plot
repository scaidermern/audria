set terminal pngcairo enhanced rounded color size 800,500
set output "profile_firefox_io.png"
#set terminal svg enhanced rounded size 1600,1000 background rgb "white"
#set output "profile_firefox_io.svg"

set key left top box

set border 3 back ls -1
set tics nomirror
set grid front lt 0 lw 1

set style fill transparent solid 0.5 border

set xlabel "run time [s]"
set xrange[0:]
set xtics 0.5

set ylabel "read/write [MB/s]"
set ytics
set yrange[0:]

set datafile separator ","
plot "profile_firefox.txt" using 18:($28/1024/1024) axes x1y1 title "read" with filledcurves x1 linecolor 1,\
     "profile_firefox.txt" using 18:($32/1024/1024) axes x1y1 title "write" with filledcurves x1 linecolor 3,\
     "profile_firefox.txt" using 18:34 axes x1y2 title "read calls" with filledcurves x1 linecolor 4,\
     "profile_firefox.txt" using 18:36 axes x1y2 title "write calls" with filledcurves x1 linecolor 5
