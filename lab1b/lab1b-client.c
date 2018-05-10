/*NAME: Aashita Patwari
EMAIL: harshupatwari@gmail.com */

#include <getopt.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>
#include "zlib.h"


struct termios savedTermAttributes;
struct termios termAttributes;
int debug;
int logFd;

void printAttributes (struct termios *state)
{
	printf("%d \n", state -> c_iflag);
	printf("%d\n", state -> c_oflag);
	printf("%d\n", state-> c_lflag);
}

void doWrite(int writeTo, char* buffer, int numberBytes)
{
	int i;
	for (i = 0; i < numberBytes; i++) {
		switch(buffer[i])
		{
		case '\r':
		case '\n':
			if(writeTo == STDOUT_FILENO) { //echoing
				char x[2];
				x[0] = '\r';
				x[1] = '\n';
				if(debug)
				{
					printf("cr/lf detected\n");
				}
				if(write(writeTo, x, 2) < 0)
				{
					fprintf(stderr, "error with write function. errno: %d, message: %s \n", errno, strerror(errno));
					exit(1);
				}
			}
			else //write to socket as it is
			{
				if(write(writeTo, buffer + i, 1) < 0)
				{
					fprintf(stderr, "error with write function. errno: %d, message: %s \n", errno, strerror(errno));
					exit(1);
				}
			}	
			break;
		default:
			if (write(writeTo, buffer + i, 1) < 0)
			{
				fprintf(stderr, "error with write function. errno: %d, message: %s \n", errno, strerror(errno));
				exit(1);
			}
		}
	}
}

void writeLog(char * buf, bool toServer, int numberBytes)
{
	char x = '\n'; 
	if(toServer)
	{
		if(dprintf(logFd, "SENT %d bytes: ", numberBytes) < 0) //write opening string
		{
			fprintf(stderr, "Error with write to log file. errno: %d, error message: %s \n", errno, strerror(errno));
		}

		if(write(logFd, buf, numberBytes) < 0) //write the bytes
		{
			fprintf(stderr, "Error with write to log file. errno: %d, error message: %s \n", errno, strerror(errno));
		}

		if(write(logFd, &x, 1) < 0) //write the newline character
		{
			fprintf(stderr, "Error with write to log file. errno: %d, error message: %s \n", errno, strerror(errno));
		}
	}

	else
	{
		if(dprintf(logFd, "RECEIVED %d bytes: ", numberBytes) < 0) //write opening string
		{
			fprintf(stderr, "Error with write to log file. errno: %d, error message: %s \n", errno, strerror(errno));
		}

		if(write(logFd, buf, numberBytes) < 0) //write the bytes
		{
			fprintf(stderr, "Error with write to log file. errno: %d, error message: %s \n", errno, strerror(errno));
		}

		if(write(logFd, &x, 1) < 0) //write the newline character
		{
			fprintf(stderr, "Error with write to log file. errno: %d, error message: %s \n", errno, strerror(errno));
		}
	}
}


void resetInputMode (void)
{
	if(tcsetattr(STDIN_FILENO, TCSANOW, &savedTermAttributes) < 0)
	{
		fprintf(stderr, "Error setting terminal attributes. errno: %d, message: %s \n", errno, strerror(errno));
		exit(1);
	}

	if (debug)
	{
		struct termios checkTermAttributes;
		if (tcgetattr(STDIN_FILENO, &checkTermAttributes) < 0)
		{
			printf("error in getting terminal attributes to check");
		}
		printAttributes(&checkTermAttributes); //should be resetted input
	}

	exit(0);

}

void setInputMode (void)
{

	if (!isatty(STDIN_FILENO))
	{
		fprintf(stderr, "Not a terminal. errno: %d, message: %s \n", errno, strerror(errno));
		exit(1);
	}

	if (tcgetattr(STDIN_FILENO, &savedTermAttributes) < 0)
	{
		fprintf(stderr, "Error getting normal terminal attributes. errno: %d, message: %s \n", errno, strerror(errno));
		exit(1);
	} //save attributes to reset
	if(debug)
	{
		printAttributes(&savedTermAttributes);
	}

	if (tcgetattr(STDIN_FILENO, &termAttributes) < 0)
	{
		fprintf(stderr, "Error getting terminal attributes.errno: %d, message: %s \n", errno, strerror(errno));
		exit(1);
	} //get attributes again to change
	termAttributes.c_iflag = ISTRIP;
	termAttributes.c_oflag = 0;
	termAttributes.c_lflag = 0;
	if (tcsetattr(STDIN_FILENO, TCSANOW, &termAttributes) < 0)
	{
		fprintf(stderr, "Error setting changed terminal attributes. errno: %d, message: %s \n", errno, strerror(errno));
		exit(1);
	}
}

int main(int argc, char **argv) {
	debug = 0;
	int c;
	int ret;
	int portNumber;
	int compress = 0;
	int port = 0;
	char* fileName;
	int log = 0;
	ssize_t nBytes;

	static struct option long_options[] = {
		{"port", required_argument,0, 'p'}, 
		{"log", required_argument, 0, 'l'}, 
		{"debug", no_argument, 0, 'd'},
		{"compress", no_argument, 0, 'c'},
		{0, 0, 0, 0}
	};

	while((c = getopt_long(argc, argv, "sd", long_options, NULL)) != -1) {
		switch(c) {
			case 'p':
				port = 1;
				portNumber = atoi(optarg);
			break;
			case 'l':
				log = 1;
				fileName = optarg;
				break;		
			case 'd':
				debug = 1;
				break;	
			case 'c':
				compress = 1;
				break;	
			default: //some other option passed in
				fprintf(stderr, "Usage: --port=port# [--log=filename] [--compress] \n");
				exit(1);
		}
	}

	if(!port)
	{
		fprintf(stderr, "--port=port# is a mandatory option");
		exit(1);
	}

	setInputMode(); //saves and changes attributes
	atexit(resetInputMode);
	if (debug)
	{
		printAttributes(&termAttributes); //should be new attributes
	}


	int sockfd = 0;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		fprintf(stderr, "Error creating socket. Error code: %d, error message: %s \n", errno, strerror(errno));
		exit(1);
	}

	server = gethostbyname("localhost");
	 if (server == NULL) {
        fprintf(stderr,"ERROR, no such host. Error code: %d, error message: %s \n", errno, strerror(errno));
        exit(1);
    }

    memset(&serv_addr, '0', sizeof(serv_addr)); //check if it works, otherwise (char *), 0, 


	serv_addr.sin_family = AF_INET;
	memcpy((char *) &serv_addr.sin_addr.s_addr, (char *) server -> h_addr, server -> h_length);
	serv_addr.sin_port = htons(portNumber);
	if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		fprintf(stderr, "Connection failed. Error code: %d, error message: %s \n", errno, strerror(errno));
		exit(1);
	}

//only need to initialize once, for compression/decompression
	z_stream stdin_to_socket;
	z_stream socket_to_stdout;

	stdin_to_socket.zalloc = Z_NULL;
	stdin_to_socket.zfree = Z_NULL;
	stdin_to_socket.opaque = Z_NULL;

	if(deflateInit(&stdin_to_socket, Z_DEFAULT_COMPRESSION) != Z_OK)
	{
		fprintf(stderr, "error with initialization for compression\n");
		deflateEnd(&stdin_to_socket);
		exit(1);
	}

	socket_to_stdout.zalloc = Z_NULL;
	socket_to_stdout.zfree = Z_NULL;
	socket_to_stdout.opaque = Z_NULL;

	if(inflateInit(&socket_to_stdout) != Z_OK)
	{
		fprintf(stderr, "error with initialization for decompression\n");
		inflateEnd(&socket_to_stdout);
		exit(1);
	}

	//polling setup
	struct pollfd fds[2];
	fds[0].fd = STDIN_FILENO;
	fds[0].events = POLLIN;
	fds[1].fd = sockfd;
	fds[1].events = POLLIN | POLLHUP | POLLERR;


    if (log){
        logFd = creat(fileName, S_IRWXU);
        if (logFd < 0){
            fprintf(stderr, "Error creating log file. Errno: %d, error message: %s \n", errno, strerror(errno));
            exit(1);
        }
    }

	while(1)
	{
		ret = poll(fds, 2, 0);
		if(ret == -1) {
			fprintf(stderr, "polling error. errno: %d, message: %s \n", errno, strerror(errno));
			exit(1);
		}

		if(fds[0].revents & POLLIN) //reading from input writing to server
		{
			if(debug)
			{
				printf("should read from terminal \n");
			}

			char buffer1[256];
			memset(buffer1, '0', 256);
			nBytes = read(STDIN_FILENO, buffer1, 256);

			if(nBytes < 0)
			{
				fprintf(stderr, "error with read function. errno: %d, message: %s \n", errno, strerror(errno));
				exit(1);
			}

			doWrite(STDOUT_FILENO, buffer1, nBytes); //echo

			if(compress) //compress before writing
			{
				char compressedBuffer[16384]; 
				stdin_to_socket.avail_in = nBytes;
				stdin_to_socket.next_in = (Bytef *) buffer1;
				stdin_to_socket.avail_out = 16384;
				stdin_to_socket.next_out = (Bytef *) compressedBuffer;
				do {
					deflate(&stdin_to_socket, Z_SYNC_FLUSH);
				} while (stdin_to_socket.avail_in > 0);

				if(log)
				{
					writeLog(compressedBuffer, true, 16384 - stdin_to_socket.avail_out);
				} //if log, record outgoing data post compression

				doWrite(sockfd, compressedBuffer, 16384 - stdin_to_socket.avail_out);
			}
			else
			{
				if(log)
				{
					writeLog(buffer1, true, nBytes);
				}
				//if log, just record

				doWrite(sockfd, buffer1, nBytes); //just write
			}
		}

		if(fds[1].revents & POLLIN) //socket to stdout
		{
			if (debug)
			{ 
				printf("should read and write from socket \n"); 
			}
			char buffer2[256];
			memset(buffer2, '0', 256);
			nBytes = read(sockfd, buffer2, 256);
			if(nBytes < 0)
			{
				fprintf(stderr, "error with read function. errno: %d, message: %s \n", errno, strerror(errno));
				exit(1);
			}

			if(nBytes==0)
			{
				if(close(sockfd) < 0)
				{
					fprintf(stderr, "Error closing socket. errno: %d, message: %s \n", errno, strerror(errno));
					exit(1);
				}
				exit(0);
			}

			//if log, record incoming data
			if(log)
			{
				writeLog(buffer2, false, nBytes);
			}


			if(compress) //decompress before writing
			{
				char decompressedBuffer[16384];
				socket_to_stdout.avail_in = nBytes;
				socket_to_stdout.next_in = (Bytef *) buffer2;
				socket_to_stdout.avail_out = 16384;
				socket_to_stdout.next_out = (Bytef *) decompressedBuffer;
				do {
					inflate(&socket_to_stdout, Z_SYNC_FLUSH);
				} while (socket_to_stdout.avail_in > 0);
				doWrite(STDOUT_FILENO, decompressedBuffer, 16384 - socket_to_stdout.avail_out);
			}
			else
			{
				doWrite(STDOUT_FILENO, buffer2, nBytes);
			}
		}


		if(fds[1].revents & (POLLHUP | POLLERR)) //terminate
		{
			//close
			if(debug)
			{
				printf("Done reading from socket, closing\n");
			}
			if(close(sockfd) < 0)
			{
				fprintf(stderr, "error closing. errno: %d, message: %s \n", errno, strerror(errno));
				exit(1);
			}
			break;
		}
	}

	inflateEnd(&socket_to_stdout);
	deflateEnd(&stdin_to_socket);
//will call reset atexit
}





