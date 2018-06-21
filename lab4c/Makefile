#NAME: Aashita Patwari
#EMAIL: harshupatwari@gmail.com

default:
	gcc -Wextra -Wall -g -lmraa lab4c_tcp.c -o lab4c_tcp -lm
	gcc -Wextra -Wall -g -lmraa -lssl -lcrypto lab4c_tls.c -o lab4c_tls -lm
clean:
	rm -rf lab4c_tls lab4c_tcp lab4c-004810708.tar.gz
dist:
	tar -czvf lab4c-004810708.tar.gz lab4c_tls.c lab4c_tcp.c README Makefile
