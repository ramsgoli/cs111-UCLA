default: lab1b-client lab1b-server

lab1b-client: lab1b-client.c
	gcc -o lab1b-client -Wall -Wextra lab1b-client.c -lmcrypt

lab1b-server: lab1b-server.c
	gcc -o lab1b-server -Wall -Wextra lab1b-server.c -lmcrypt

clean:
	rm -f lab1b-client lab1b-server lab1b-604751659.tar.gz

dist:
	tar -czvf lab1b-604751659.tar.gz my.key README lab1b-client.c lab1b-server.c Makefile
