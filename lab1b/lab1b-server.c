/*NAME: Aashita Patwari
EMAIL: harshupatwari@gmail.com */

#include <getopt.h>
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
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "zlib.h"


int debug;
pid_t childPid;
ssize_t nBytes;
int toShell[2]; //read from terminal (parent) to the shell (child)
int fromShell[2]; //read from shell to terminal

void logError(void)
{
	int status;
	if(childPid != -2) //shell called
	{
		if (waitpid(childPid, &status,0) == -1)
		{
			fprintf(stderr, "error with waitpid. errno: %d, message: %s \n", errno, strerror(errno));
			exit(1);
		}

		if(WIFEXITED(status))
		{
			fprintf(stderr, "SHELL EXIT SIGNAL = %d STATUS=%d \n", WTERMSIG(status), WEXITSTATUS(status));			
		}
	}
}


void sig_handler(int signo)
{
	if(signo == SIGPIPE)
	{
		if (debug)
		{
			printf("recieved SIGPIPE");
		}
		exit(0);
	}
}

void doWrite(int writeTo, char* buffer, int numberBytes)
{
	int i;
	for (i = 0; i < numberBytes; i++) {
		switch(buffer[i]) {
		case 0x04:
			//close pipe
			if(debug) {
				printf("control D detected, closing pipe");
			}
			if(close(toShell[1]) < 0)
			{
				fprintf(stderr, "error closing. errno: %d, message: %s \n", errno, strerror(errno));
				exit(1);
			}
			else {
				exit(0);
			}
			break;
		case 0x03: 
			if(debug) {
				printf("control C detected, killing shell");
			}
			kill(childPid, SIGINT);
			break;
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
			else //writing to shell 
			{
				char x;
				x = '\n';
				if(write(writeTo, &x, 1) < 0)
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

int main(int argc, char **argv)
{
	debug = 0;
	int c;
	int ret;
	int compress = 0;
	int port = 0;
	childPid = -2;

	static struct option long_options[] = {
		{"debug", no_argument, 0, 'd'},
		{"port", required_argument, 0, 'p'},
		{"compress", no_argument,0, 'c'},
		{0, 0, 0, 0}
	};

	int portNumber;

	while((c = getopt_long(argc, argv, "sd", long_options, NULL)) != -1) {
		switch(c) {
			case 'p':
				port = 1;
				portNumber = atoi(optarg);
			break;
			case 'd':
				debug = 1;
				break;	
			case 'c':
				compress = 1;
				break;				
			default: //some other option passed in
				fprintf(stderr, "Usage: --port=port# [--compress] \n");
				exit(1);
		}
	}

	if(!port)
	{
		fprintf(stderr, "--port=port# is a mandatory option");
		exit(1);
	}

	atexit(logError);

	int sockfd, newsockfd;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		fprintf(stderr, "error creating socket. errno: %d, message: %s \n", errno, strerror(errno));
		exit(1);
	}

	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portNumber);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		fprintf(stderr, "error binding socket. errno: %d, message: %s \n", errno, strerror(errno));
		exit(1);
	}

	listen(sockfd, 5);
	clilen = sizeof(cli_addr);

	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

	if (newsockfd < 0)
	{
		fprintf(stderr, "Error on accept\n");
		exit(1);
	}


//zlib set up

	z_stream socket_to_shell;
	z_stream shell_to_socket;

	shell_to_socket.zalloc = Z_NULL;
	shell_to_socket.zfree = Z_NULL;
	shell_to_socket.opaque = Z_NULL;

	deflateInit(&shell_to_socket, Z_DEFAULT_COMPRESSION);

	socket_to_shell.zalloc = Z_NULL;
	socket_to_shell.zfree = Z_NULL;
	socket_to_shell.opaque = Z_NULL;

	inflateInit(&socket_to_shell);

	signal(SIGPIPE, sig_handler); //sigpipe when shell exits

	//need two pipes, one for each direction

	if(pipe(toShell) == -1)
	{
		fprintf(stderr, "error creating pipe to shell. errno: %d, message: %s \n", errno, strerror(errno));
		exit(1);	
	}

	if(pipe(fromShell) == -1)	
	{
		fprintf(stderr, "error creating pipe from shell. errno: %d, message: %s\n", errno, strerror(errno));
		exit(1);
	}

	if(debug)
	{
		printf("created pipes \n");
	}

	childPid = fork();

	if(childPid == -1)
	{
		fprintf(stderr, "error forking. errno: %d, message: %s \n", errno, strerror(errno));
		exit(1);
	} else if (childPid == 0) {
		if(debug)
		{
			printf("in child process \n");
		}

		if(close(fromShell[0]) < 0)
		{
			fprintf(stderr, "error closing. errno: %d, message: %s \n", errno, strerror(errno));
			exit(1);
		} //shell writes to pipe
		if(close(toShell[1]) < 0)
		{
			fprintf(stderr, "error closing. errno: %d, message: %s \n", errno, strerror(errno));
			exit(1);
		} //shell reads from pipe

	/*if(close(STDIN_FILENO) < 0)
	{
		fprintf(stderr, "error closing. errno: %d, message: %s \n", errno, strerror(errno));
		exit(1);
	} //close stdin so that shell reads from socket */

		if(dup2(fromShell[1], 1) == -1)
		{
			fprintf(stderr, "error duplicating. errno: %d, message: %s \n", errno, strerror(errno));
			exit(1);
		}  
		if(dup2(fromShell[1], 2) == -1)
		{
			fprintf(stderr, "error duplicating. errno: %d, message: %s \n", errno, strerror(errno));
			exit(1);
		} 
		if(dup2(toShell[0], 0) == -1)
		{
			fprintf(stderr, "error duplicating. errno: %d, message: %s \n", errno, strerror(errno));
			exit(1);
		} 

		if(close(fromShell[1]) < 0)
		{
			fprintf(stderr, "error closing. errno: %d, message: %s \n", errno, strerror(errno));
			exit(1);
		}
		if(close(toShell[0]) < 0)
		{
			fprintf(stderr, "error closing. errno: %d, message: %s \n", errno, strerror(errno));
			exit(1);
		}
		char* argv[0];
		argv[0] = NULL;
		if (execvp("/bin/bash", argv) == -1)
		{
			fprintf(stderr, "error executing shell. errno: %d, message: %s \n", errno, strerror(errno));
			exit(1);
		}
	
		} 
	else {
		if (debug)
		{
			printf("in parent process \n");
		}
		if(close(fromShell[1]) < 0)
		{
			fprintf(stderr, "error closing. errno: %d, message: %s \n", errno, strerror(errno));
			exit(1);
		} //terminal reads from pipe
		if(close(toShell[0]) < 0)
		{
			fprintf(stderr, "error closing. errno: %d, message: %s \n", errno, strerror(errno));
			exit(1);
		} //terminal writes to pipe

		struct pollfd fds[2];

		fds[0].fd = newsockfd;
		fds[0].events = POLLIN | POLLHUP | POLLERR;
		fds[1].fd = fromShell[0];
		fds[1].events = POLLIN | POLLHUP | POLLERR;

		while(1)
		{
			ret = poll(fds, 2, 0);
			if (ret == -1) {
				fprintf(stderr, "polling error. errno: %d, message: %s \n", errno, strerror(errno));
				exit(1);
			}
			if(fds[0].revents & POLLIN) //socket to shell
			{
				if (debug)
				{ printf("should read and write from terminal \n"); }
				char buffer1[256];
				memset(buffer1, '0', 256);
				nBytes = read(newsockfd, buffer1, 256);

				if(nBytes < 0)
				{
					fprintf(stderr, "error with read function. errno: %d, message: %s \n", errno, strerror(errno));
					exit(1);
				}

				if(nBytes == 0) //EOF, done reading from socket to shell
				{
					break;
				}

				if(compress)
				{
					//decompress before sending to shell
					char decompressedBuffer[16384];
					socket_to_shell.avail_in = nBytes;
					socket_to_shell.next_in = (Bytef *) buffer1;
					socket_to_shell.avail_out = 16384;
					socket_to_shell.next_out = (Bytef *) decompressedBuffer;
					do {
						inflate(&socket_to_shell, Z_SYNC_FLUSH);
					} while (socket_to_shell.avail_in > 0);

					doWrite(toShell[1], decompressedBuffer, 16384 - socket_to_shell.avail_out);
				}

				else
				{
					doWrite(toShell[1], buffer1, nBytes);
				}

			}

			if(fds[0].revents & (POLLHUP | POLLERR)) //if nothing to read from socket
			{
				//close
				if(debug)
				{
					printf("error event at poll, closing pipe\n");
				}
				if(close(toShell[0]) < 0)
				{
					fprintf(stderr, "error closing. errno: %d, message: %s \n", errno, strerror(errno));
					exit(1);
				}
				break;
			}

			if(fds[1].revents & POLLIN) //shell to socket
			{
				if (debug)
				{ printf("should read and write from shell \n"); }
				char buffer2[256];
				memset(buffer2, '0', 256);
				nBytes = read(fromShell[0], buffer2, 256);

				if(nBytes < 0)
				{
					fprintf(stderr, "error with read function. errno: %d, message: %s \n", errno, strerror(errno));
					exit(1);
				}

				if(nBytes == 0)
				{
					break;
				}

				if(compress) 							//compress before writing
				{
					char compressedBuffer[16384];
					shell_to_socket.avail_in = nBytes;
					shell_to_socket.next_in = (Bytef *) buffer2;
					shell_to_socket.avail_out = 16384;
					shell_to_socket.next_out = (Bytef *) compressedBuffer;
					do {
						deflate(&shell_to_socket, Z_SYNC_FLUSH);
					} while (shell_to_socket.avail_in > 0);
					doWrite(newsockfd, compressedBuffer, 16384 - shell_to_socket.avail_out);
				}
				else
				{
					doWrite(newsockfd, buffer2, nBytes);
				}
				
			}
			if(fds[1].revents & (POLLHUP | POLLERR)) //terminate
			{
						//close
				if(debug)
				{
					printf("error event at poll, closing pipe\n");
				}
				if(close(fromShell[0]) < 0)
				{
					fprintf(stderr, "error closing. errno: %d, message: %s \n", errno, strerror(errno));
					exit(1);
				}
				break;
			}
		}
	}
}

