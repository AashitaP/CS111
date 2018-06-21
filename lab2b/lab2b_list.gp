#! /usr/bin/gnuplot
#
# purpose:
#	 generate data reduction graphs for the multi-threaded list project
#

# general plot parameters
set terminal png
set datafile separator ","

# unset the kinky x axis
unset xtics
set xtics


set title "List-1: Throughput vs. number of threads for mutex and spin-lock synchronized list operations"
set xlabel "Number of threads"
set logscale x 2
unset xrange
set xrange [0.75:]
set ylabel "Throughput"
set logscale y
set output 'lab2b_1.png'
set key left top


plot \
    "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'list w/mutex' with linespoints lc rgb 'blue', \
     "< grep -e 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'list w/spin-lock' with linespoints lc rgb 'green'


set title "List-2: Mean time per mutex wait and mean time per operation for mutex-synchronized list operations"
set xlabel "Number of threads"
set logscale x 2
unset xrange
set xrange [0.75:]
set ylabel "Time per operation (ns)"
set logscale y
set output 'lab2b_2.png'
set key left top

plot \
    "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($8) \
	title 'Average wait-for-lock time' with linespoints lc rgb 'blue', \
     "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($7) \
	title 'Average time per operation' with linespoints lc rgb 'green'


set title "List-3: Successful iterations vs. threads for each synchronization method"
set xlabel "Number of threads"
set logscale x 2
unset xrange
set xrange [0.75:]
set ylabel "Successful Iterations"
set logscale y 10
set output 'lab2b_3.png'

plot \
     "< grep list-id-none,[0-9]*,[0-9]*,4 lab2b_list.csv" using ($2):($3) \
	title 'w/o synchronization' with points lc rgb 'red', \
     "< grep list-id-m,[0-9]*,[0-9]*,4 lab2b_list.csv" using ($2):($3) \
	title 'w/ mutex synchronization' with points lc rgb 'violet', \
     "< grep list-id-s,[0-9]*,[0-9]*,4 lab2b_list.csv" using ($2):($3) \
	title 'w/ spin locks synchronization' with points lc rgb 'orange'

set title "List-4: Throughput vs. number of threads for mutex synchronized partitioned list"
set xlabel "Number of threads"
set logscale x 2
unset xrange
set xrange [0.75:]
set ylabel "Throughput"
set logscale y
set output 'lab2b_4.png'
set key left top

plot \
    "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '1 list' with linespoints lc rgb 'blue', \
     "< grep -e 'list-none-m,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '4 lists' with linespoints lc rgb 'blue', \
     "< grep -e 'list-none-m,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '8 lists' with linespoints lc rgb 'blue', \
     "< grep -e 'list-none-m,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '16 lists' with linespoints lc rgb 'blue'


set title "List-5: Throughput vs. number of threads for spin-lock-synchronized partitioned lists"
set xlabel "Number of threads"
set logscale x 2
unset xrange
set xrange [0.75:]
set ylabel "Throughput"
set logscale y
set output 'lab2b_5.png'
set key left top


plot \
    "< grep -e 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '1 list' with linespoints lc rgb 'blue', \
     "< grep -e 'list-none-s,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '4 lists' with linespoints lc rgb 'blue', \
     "< grep -e 'list-none-s,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '8 lists' with linespoints lc rgb 'blue', \
     "< grep -e 'list-none-s,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '16 lists' with linespoints lc rgb 'blue'