# NAME: Divij Ohri, Aashita Patwari
# EMAIL: divijohri28@gmail.com, harshupatwari@gmail.com
# ID: 404821856, 004810708

CC=gcc
CFLAGS=-g -Wall -Wextra -std=gnu99

# Build the executable
build: lab3a.c
	$(CC) $(CFLAGS) lab3a.c -o lab3a

# Deletes all makefile-created files
clean:
	rm -f lab3a *.o *.tar.gz *.txt *~
	

# Build the distribution tarball
dist: 
	tar -cvzf lab3a-004810708.tar.gz lab3a.c Makefile README ext2_fs.h
