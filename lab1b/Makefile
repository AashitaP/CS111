#NAME: Aashita Patwari
#EMAIL: harshupatwari@gmail.com

default: client server

client: lab1b-client.c
	gcc -g -Wall -Wextra -lz lab1b-client.c -o lab1b-client

server: lab1b-server.c
	gcc -g -Wall -Wextra -lz lab1b-server.c -o lab1b-server

dist: lab1b-004810708.tar.gz

lab1b-004810708.tar.gz: lab1b-client.c lab1b-server.c
	tar -czf $@ lab1b-client.c lab1b-server.c Makefile README

clean:
	rm -f *.tar.gz *.sh *.txt lab1b-client lab1b-server