# NAME: Ram Goli
# ID: 604751659
# EMAIL: ramsgoli@gmail.com

build: lab4b

lab4b: lab4b.c
	gcc -Wall -Wextra -std=gnu11 -g -lmraa -lm -pthread -o lab4b lab4b.c

clean:
	rm -f lab4b-604751659.tar.gz
	rm -f lab4b

check: lab4b
	./tests.sh

dist: lab4b
	tar -zvcf lab4b-604751659.tar.gz lab4b.c Makefile README
