# NAME: Ram Goli
# ID: 604751659
# EMAIL: ramsgoli@gmail.com

build: add list

add: lab2_add.c
	gcc -Wall -Wextra -pthread lab2_add.c -o lab2_add	

list: lab2_list.c SortedList.c
	gcc -L`echo ~/`lib -lprofiler -Wall -Wextra -pthread -g lab2_list.c SortedList.c -o lab2_list

clean:
	rm -rf lab2_list
	rm -rf lab2_add
	rm -rf lab2a-604751659.tar.gz
	rm -rf *.png
	rm -rf *.csv

dist: build graphs
	tar -cvzf lab2a-604751659.tar.gz README Makefile lab2_add.c SortedList.h SortedList.c lab2_list.c lab2_add.csv lab2_list.csv *.png run_tests.sh

graphs: tests
	./lab2_add.gp
	./lab2_list.gp

tests: build run_tests.sh
	./run_tests.sh

spinlock_list_test:
	