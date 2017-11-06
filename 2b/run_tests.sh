#! /bin/bash

# mutex lock
for THREAD in 1, 2, 4, 8, 12, 16, 24; do
    ./lab2_list --sync=m --iterations=1000 --threads=$THREAD >> lab2b_list.csv
done

# spin lock
for THREAD in 1, 2, 4, 8, 12, 16, 24; do
    ./lab2_list --sync=s --iterations=1000 --threads=$THREAD >> lab2b_list.csv
done

