#NAME: Aashita Patwari
#EMAIL: harshupatwari@gmail.com

default: lab4b.c
	gcc -g -Wall -Wextra -lm -lmraa lab4b.c -o lab4b

dist: lab4b-004810708.tar.gz

lab4b-004810708.tar.gz: lab4b.c
	tar -czf $@ lab4b.c Makefile README

check: lab4b
	echo | ./lab4b --invalid &> /dev/null; \
	if [[ $? -ne 1 ]]; then \
	echo "Test failed"; \
	fi
	echo | ./lab4b --period=2 --scale=C --log="LOGFILE" &> /dev/null; \
	if [[ $? -ne 0 ]]; then \
	echo "Test failed"; \
	fi

clean:
	rm -f *.tar.gz *.txt lab4b LOGFILE