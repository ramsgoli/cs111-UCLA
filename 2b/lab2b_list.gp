#! /usr/local/cs/bin/gnuplot
#
# purpose:
#	 generate data reduction graphs for the multi-threaded list project
#
# input: lab2b_list.csv
#	1. test name
#	2. # threads
#	3. # iterations per thread
#	4. # lists
#	5. # operations performed (threads x iterations x (ins + lookup + delete))
#	6. run time (ns)
#	7. run time per operation (ns)
#
# output:
#	lab2_list-1.png ... cost per operation vs threads and iterations
#	lab2_list-2.png ... threads and iterations that run (un-protected) w/o failure
#	lab2_list-3.png ... threads and iterations that run (protected) w/o failure
#	lab2_list-4.png ... cost per operation vs number of threads
#
# Note:
#	Managing data is simplified by keeping all of the results in a single
#	file.  But this means that the individual graphing commands have to
#	grep to select only the data they want.
#
#	Early in your implementation, you will not have data for all of the
#	tests, and the later sections may generate errors for missing data.
#

# general plot parameters
set terminal png
set datafile separator ","

# how many threads/iterations we can run without failure (w/o yielding)
set title "List-1: Operations per second vs number of threads"
set xlabel "Threads"
set logscale x 2
set ylabel "Operations per second (ns)"
set logscale y 10
set output 'lab2b_1.png'

# grep out only single threaded, un-protected, non-yield results
system("grep -E 'list-none-[ms],[0-9][0-9]?,1000,1,' lab2b_list.csv >> temp.csv")
plot \
     "< grep 'list-none-m,' temp.csv" using ($2):(1000000000)/($7) \
	title 'mutex' with linespoints lc rgb 'red', \
     "< grep 'list-none-s,' temp.csv" using ($2):(1000000000)/($7) \
	title 'spin-lock' with linespoints lc rgb 'green'

system("rm -f temp.csv")

# Wait for lock time and average time per operation against number of threads
set title "Time per operation and average wait-for-mutex time"
set xlabel "Threads"
set logscale x 2
set ylabel "Time (ns)"
set logscale y 10
set output "lab2b_2.png"

# grep out only mutex locked outputs
system("grep -E 'list-none-m,[0-9][0-9]?,1000,1,' lab2b_list.csv >> temp.csv")

plot \
    "temp.csv" using ($2):($8) \
    title 'wait-for-mutex' with linespoints lc rgb 'red', \
    "temp.csv" using ($2):($7) \
    title 'average-time-per-operation' with linespoints lc rgb 'green'

system("rm -f temp.csv")

set title "Protected Iterations that run without failure"
unset logscale x
set logscale x 2
set xlabel "Threads"
set ylabel "Successful iterations"
set logscale y 10
set output 'lab2b_3.png'

system ("grep -E 'list-id-[ms],[0-9][0-9]?,[0-9][0-9]?,4,' lab2b_list.csv >> temp.csv")
system ("grep -E 'list-id-none,[0-9][0-9]?,[0-9][0-9]?,4,' lab2b_list.csv >> temp.csv")
plot \
    "< grep 'none' temp.csv" using ($2):($3) \
	with points lc rgb "red" title "unprotected", \
    "< grep 'id-s' temp.csv" using ($2):($3) \
	with points lc rgb "green" title "spin lock", \
    "< grep 'id-m' temp.csv" using ($2):($3) \
	with points lc rgb "blue" title "mutex lock", \


# Performance of partitioned lists 
system("grep -E 'list-none-[ms],[0-9][0-9]?,1000,' lab2b_list.csv >> temp.csv")

set title "Throughput vs number of threads - mutex lock"
set xlabel "Threads"
set logscale x 2
set ylabel "Total operations per second"
set logscale y 10
set output "lab2b_4.png"

plot \
    "< grep -E 'list-none-m,[0-9][0-9]?,1000,1,' temp.csv" using ($2):(1000000000*($5)/($6)) \
    title '1 list' with linespoints lc rgb 'red', \
    "< grep -E 'list-none-m,[0-9][0-9]?,1000,4,' temp.csv" using ($2):(1000000000*($5)/($6)) \
    title '4 lists' with linespoints lc rgb 'green', \
    "< grep -E 'list-none-m,[0-9][0-9]?,1000,8,' temp.csv" using ($2):(1000000000*($5)/($6)) \
    title '8 lists' with linespoints lc rgb 'blue', \
    "< grep -E 'list-none-m,[0-9][0-9]?,1000,16,' temp.csv" using ($2):(1000000000*($5)/($6)) \
    title '16 lists' with linespoints lc rgb 'black', \

set title "Throughput vs number of threads - spin lock"
set xlabel "Threads"
set logscale x 2
set ylabel "Total operations per second"
set logscale y 10
set output "lab2b_5.png"

plot \
    "< grep -E 'list-none-s,[0-9][0-9]?,1000,1,' temp.csv" using ($2):(1000000000*($5)/($6)) \
    title '1 list' with linespoints lc rgb 'red', \
    "< grep -E 'list-none-s,[0-9][0-9]?,1000,4,' temp.csv" using ($2):(1000000000*($5)/($6)) \
    title '4 lists' with linespoints lc rgb 'green', \
    "< grep -E 'list-none-s,[0-9][0-9]?,1000,8,' temp.csv" using ($2):(1000000000*($5)/($6)) \
    title '8 lists' with linespoints lc rgb 'blue', \
    "< grep -E 'list-none-s,[0-9][0-9]?,1000,16,' temp.csv" using ($2):(1000000000*($5)/($6)) \
    title '16 lists' with linespoints lc rgb 'black', \

# remove our temp file
system("rm -f temp.csv")