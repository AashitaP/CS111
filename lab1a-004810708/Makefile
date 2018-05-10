#NAME: Aashita Patwari
#EMAIL: harshupatwari@gmail.com
#ID: 004810708 

default: 
	gcc -g -Wall -Wextra lab1a.c -o lab1a


dist: lab1a-004810708.tar.gz

lab1a-004810708.tar.gz: lab1a.c
	tar -czf $@ lab1a.c Makefile README

clean:
	rm -f *.tar.gz *.sh lab1a 