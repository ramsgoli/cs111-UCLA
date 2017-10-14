# NAME: Ram Goli
# EMAIL: ramsgoli@gmail.com
# ID: 604751659

.SILENT:

default: lab0

lab0: lab0.c
	gcc -o lab0 -Wall -Wextra lab0.c

clean:
	rm -rf lab0-604751659.tar.gz lab0

dist:
	tar -czvf lab0-604751659.tar.gz lab0.c README Makefile breakpoint.png backtrace.png 

check: default
	./lab0 --input=lab0.c --output=output.txt ; \
	cmp -s lab0.c output.txt
	if [ $$? -eq 0 ] ; then \
		echo "Checking correct use of --input and --output flag... OK"; \
	else \
		echo "Checking correct use of --input and --output flag... FAILURE"; \
	fi
	rm output.txt

	./lab0 --input=unknown.txt ; \
	if [ $$? -eq 2 ] ; then \
		echo "Checking correct error code due to invalid input file... OK" ; \
	else \
		echo "Checking correct error code due to invalid input file... FAILURE" ; \
	fi
	
	./lab0 --unknown ; \
	if [ $$? -eq 1 ] ; then \
		echo "Checking correct error code due to unrecognized option... OK" ; \
	else \
		echo "Checking correct error code due to unrecognized option... FAILURE" ; \
	fi

	./lab0 --catch --segfault ; \
	if [ $$? -eq 4 ] ; then \
		echo "Checking correct error code due to caught SIGSEGV... OK" ; \
	else \
		echo "Checking correct error code due to caught SIGSEGV... FAILURE" ; \
	fi

	touch BAD_FILE ; \
	chmod 444 BAD_FILE ; \
	./lab0 --output=BAD_FILE ; \
	if [ $$? -eq 3 ] ; then \
		echo "Checking correct error code due to output file that can't be opened... OK" ; \
	else \
		echo "Checking correct error code due to output file that can't be opened... FAILURE" ; \
	fi
	rm -f BAD_FILE


