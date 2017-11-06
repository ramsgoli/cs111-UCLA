#! /bin/bash

OUTPUT_FILE=lab2b_list.csv
EXECUTABLE=lab2_list

rm -f $OUTPUT_FILE

# mutex lock
for THREAD in 1, 2, 4, 8, 12, 16, 24; do
    ./$EXECUTABLE --sync=m --iterations=1000 --threads=$THREAD >> $OUTPUT_FILE
done

# spin lock
for THREAD in 1, 2, 4, 8, 12, 16, 24; do
    ./$EXECUTABLE --sync=s --iterations=1000 --threads=$THREAD >> $OUTPUT_FILE
done

# lab2b_3.png 
# no synchronization, so some tests should fail
for THREAD in 1, 4, 8, 12, 16; do
    for ITERATION in 1, 2, 4, 8, 16; do
        ./$EXECUTABLE --yield=id --lists=4 --threads=$THREAD --iterations=$ITERATION >> $OUTPUT_FILE
    done
done

# with synchronization, so all tests should correctly produce output
for THREAD in 1, 4, 8, 12, 16; do
    for ITERATION in 10, 20, 40, 80; do
        ./$EXECUTABLE --yield=id --lists=4 --threads=$THREAD --iterations=$ITERATION --sync=s >> $OUTPUT_FILE
        ./$EXECUTABLE --yield=id --lists=4 --threads=$THREAD --iterations=$ITERATION --sync=m >> $OUTPUT_FILE
    done
done

# lab2b_4.png and lab2b_5.png (sync=m, sync=s)
for THREAD in 1, 2, 4, 8, 12; do
    for LIST in 1, 4, 8, 16; do
        ./$EXECUTABLE --iterations=1000 --threads=$THREAD --sync=m --lists=$LIST >> $OUTPUT_FILE
        ./$EXECUTABLE --iterations=1000 --threads=$THREAD --sync=s --lists=$LIST >> $OUTPUT_FILE
    done
done