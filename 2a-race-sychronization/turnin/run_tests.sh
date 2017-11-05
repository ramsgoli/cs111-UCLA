#!/bin/bash

# without yield
for ITERATION in 10 20 40 80 100 1000 100000; do
for THREAD in 1 2 4 8 12; do
./lab2_add --threads=$THREAD --iterations=$ITERATION >> lab2_add.csv
done
done

# with yield
for ITERATION in 10 20 40 80 100 1000 10000 100000; do
for THREAD in 1 2 4 8 12; do
./lab2_add --yield --threads=$THREAD --iterations=$ITERATION >> lab2_add.csv
done
done

# with the different kinds of syncs
for ITERATION in 10 100 1000 10000; do
for THREAD in 1 2 4 8 12; do
./lab2_add --sync=m --yield --threads=$THREAD --iterations=$ITERATION >> lab2_add.csv
./lab2_add --sync=c --yield --threads=$THREAD --iterations=$ITERATION >> lab2_add.csv
./lab2_add --sync=s --yield --threads=$THREAD --iterations=$ITERATION >> lab2_add.csv
done
done

# with different kinds of syncs but no yields
for ITERATION in 10 20 40 80 100 1000 10000; do
for THREAD in 1 2 4 8 12; do
./lab2_add --sync=m --threads=$THREAD --iterations=$ITERATION >> lab2_add.csv
./lab2_add --sync=c --threads=$THREAD --iterations=$ITERATION >> lab2_add.csv
./lab2_add --sync=s --threads=$THREAD --iterations=$ITERATION >> lab2_add.csv
./lab2_add --threads=$THREAD --iterations=$ITERATION >> lab2_add.csv
done
done

# single thread with increasing ITERATION
for ITERATION in 10 100 1000 10000 20000; do
./lab2_list --threads=1 --iterations=$ITERATION >> lab2_list.csv
done

#lab2_list-2.png
for ITERATION in 1 2 4 8 16 32; do
for THREAD in 2 4 8 12; do
./lab2_list --yield=i --threads=$THREAD --iterations=$ITERATION >> lab2_list.csv
./lab2_list --yield=d --threads=$THREAD --iterations=$ITERATION >> lab2_list.csv
./lab2_list --yield=il --threads=$THREAD --iterations=$ITERATION >> lab2_list.csv
./lab2_list --yield=dl --threads=$THREAD --iterations=$ITERATION >> lab2_list.csv
done
done

#lab2_list-3.png
for ITERATION in 1 2 4 8 16 32; do
for THREAD in 2 4 8 12; do
./lab2_list --sync=s --yield=i --threads=$THREAD --iterations=$ITERATION >> lab2_list.csv
./lab2_list --sync=s --yield=d --threads=$THREAD --iterations=$ITERATION >> lab2_list.csv
./lab2_list --sync=s --yield=dl --threads=$THREAD --iterations=$ITERATION >> lab2_list.csv
./lab2_list --sync=s --yield=il --threads=$THREAD --iterations=$ITERATION >> lab2_list.csv

./lab2_list --sync=m --yield=i --threads=$THREAD --iterations=$ITERATION >> lab2_list.csv
./lab2_list --sync=m --yield=d --threads=$THREAD --iterations=$ITERATION >> lab2_list.csv
./lab2_list --sync=m --yield=il --threads=$THREAD --iterations=$ITERATION >> lab2_list.csv
./lab2_list --sync=m --yield=dl --threads=$THREAD --iterations=$ITERATION >> lab2_list.csv
done
done


#lab2_list-4.png
for ITERATION in 1000; do
for THREAD in 1 2 4 8 12 16 24; do
./lab2_list --sync=s --threads=$THREAD --iterations=$ITERATION >> lab2_list.csv
./lab2_list --sync=m --threads=$THREAD --iterations=$ITERATION >> lab2_list.csv
done
done