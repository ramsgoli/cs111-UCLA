# NAME: Ram Goli
# ID: 604751659
# EMAIL: ramsgoli@gmail.com

build: list

list: lab2_list.c SortedList.c
	gcc -L`echo ~/`lib -lprofiler -Wall -Wextra -pthread -g lab2_list.c SortedList.c -o lab2_list

clean:
	rm -rf lab2_list
	rm -rf lab2b-604751659.tar.gz

graphs: tests
	./lab2b_list.gp

tests: build run_tests.sh
	./run_tests.sh
	
profile: build 
	CPUPROFILE=profile.out ./lab2_list --threads=10 --sync=s --iterations=1000

dist: build graphs profile
	tar -cvzf lab2b-604751659.tar.gz README Makefile SortedList.h SortedList.c lab2_list.c lab2b_list.gp lab2b_list.csv *.png profile.out run_tests.sh