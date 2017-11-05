#!/bin/bash

echo "" > lab2_add.csv
echo "" > lab2_list.csv


# lab2_add-2.png
for ITERATIONS in 10 20 40 100 1000 100000; do
    for THREADS in 1 2 4 8 12; do
	./lab2_add --threads=$THREADS --iterations=$ITERATIONS >> lab2_add.csv
    done
done

# lab2_add-1.png
for ITERATIONS in 10 20 40 100 1000 10000 100000; do
    for THREADS in 1 2 4 8 12; do
	./lab2_add --yield --threads=$THREADS --iterations=$ITERATIONS >> lab2_add.csv
    done
done



# lab2_add-4.png
for ITERATIONS in 20 40 100 1000 10000; do
    for THREADS in 1 2 4 8 12; do
	./lab2_add --sync=m --yield --threads=$THREADS --iterations=$ITERATIONS >> lab2_add.csv
	./lab2_add --sync=s --yield --threads=$THREADS --iterations=$ITERATIONS >> lab2_add.csv
	./lab2_add --sync=c --yield --threads=$THREADS --iterations=$ITERATIONS >> lab2_add.csv
    done
done

# lab2_add-5.png
for ITERATIONS in 10 20 40 80 100 1000 10000; do
    for THREADS in 1 2 4 8 12; do
	./lab2_add --sync=m --threads=$THREADS --iterations=$ITERATIONS >> lab2_add.csv
	./lab2_add --sync=c --threads=$THREADS --iterations=$ITERATIONS >> lab2_add.csv
	./lab2_add --sync=s --threads=$THREADS --iterations=$ITERATIONS >> lab2_add.csv
	./lab2_add --threads=$THREADS --iterations=$ITERATIONS >> lab2_add.csv
    done
done

#lab2_list-1.png
for ITERATIONS in 10 100 1000 10000 20000; do
    for THREADS in 1; do
	./lab2_list --threads=$THREADS --iterations=$ITERATIONS >> lab2_list.csv
    done
done

#lab2_list-2.png
for ITERATIONS in 1 2 4 8 16 32; do
    for THREADS in 2 4 8 12; do
	./lab2_list --yield=i --threads=$THREADS --iterations=$ITERATIONS >> lab2_list.csv
	./lab2_list --yield=d --threads=$THREADS --iterations=$ITERATIONS >> lab2_list.csv
	./lab2_list --yield=il --threads=$THREADS --iterations=$ITERATIONS >> lab2_list.csv
	./lab2_list --yield=dl --threads=$THREADS --iterations=$ITERATIONS >> lab2_list.csv
    done
done

#lab2_list-3.png
for ITERATIONS in 1 2 4 8 16 32; do
    for THREADS in 2 4 8 12; do
	./lab2_list --sync=s --yield=i --threads=$THREADS --iterations=$ITERATIONS >> lab2_list.csv
	./lab2_list --sync=s --yield=d --threads=$THREADS --iterations=$ITERATIONS >> lab2_list.csv
	./lab2_list --sync=s --yield=il --threads=$THREADS --iterations=$ITERATIONS >> lab2_list.csv
	./lab2_list --sync=s --yield=dl --threads=$THREADS --iterations=$ITERATIONS >> lab2_list.csv

	./lab2_list --sync=m --yield=i --threads=$THREADS --iterations=$ITERATIONS >> lab2_list.csv
	./lab2_list --sync=m --yield=d --threads=$THREADS --iterations=$ITERATIONS >> lab2_list.csv
	./lab2_list --sync=m --yield=il --threads=$THREADS --iterations=$ITERATIONS >> lab2_list.csv
	./lab2_list --sync=m --yield=dl --threads=$THREADS --iterations=$ITERATIONS >> lab2_list.csv
    done
done


#lab2_list-4.png
for ITERATIONS in 1000; do
    for THREADS in 1 2 4 8 12 16 24; do
	./lab2_list --sync=s --threads=$THREADS --iterations=$ITERATIONS >> lab2_list.csv
	./lab2_list --sync=m --threads=$THREADS --iterations=$ITERATIONS >> lab2_list.csv
    done
done