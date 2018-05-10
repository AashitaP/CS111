/*NAME: Aashita Patwari
EMAIL: harshupatwari@gmail.com
ID: 004810708 */

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


struct termios savedTermAttributes;
struct termios termAttributes;
int shell;
int debug;
int toShell[2]; //read from terminal (parent) to the shell (child)
int fromShell[2]; //read from shell to terminal
pid_t childPid;
ssize_t nBytes;

void printAttributes (struct termios *state)
{
	printf("%d \n", state -> c_iflag);
	printf("%d\n", state -> c_oflag);
	printf("%d\n", state-> c_lflag);
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

void doWrite(int writeTo, char* buffer)
{
	int i;
	for (i = 0; i < nBytes; i++) {
		switch(buffer[i]) {
		case 0x04:
			//close pipe
			if(debug) {
				printf("control D detected, closing pipe");
			}
			if(shell) {
				if(close(toShell[1]) < 0)
				{
					fprintf(stderr, "error closing. errno: %d, message: %s \n", errno, strerror(errno));
					exit(1);
				}
			}
			else {
				exit(0);
			}
			break;
		case 0x03: 
			if(debug) {
				printf("control C detected, killing shell");
			}
			if(shell) {
				kill(childPid, SIGINT);
			}
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

	if(shell)
	{
		int status;
		if (waitpid(childPid, &status,0) == -1)
		{
			fprintf(stderr, "error with waitpid. errno: %d, message: %s \n", errno, strerror(errno));
			exit(1);
		}
		if(WIFEXITED(status))
		{
			fprintf(stderr, "SHELL EXIT SIGNAL = %d STATUS=%d \n", WTERMSIG(status), WEXITSTATUS(status));
			exit(0);
		}
	}

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

int main(int argc, char **argv)
{
	shell = 0;
	debug = 0;
	int c;
	int ret;

	static struct option long_options[] = {
		{"shell", no_argument, 0, 's'}, 
		{"debug", no_argument, 0, 'd'},
		{0, 0, 0, 0}
	};

	while((c = getopt_long(argc, argv, "sd", long_options, NULL)) != -1) {
		switch(c) {
			case 's':
				shell = 1;
				break;
			case 'd':
				debug = 1;
				break;				
			default: //some other option passed in
				fprintf(stderr, "error: wrong argument. \n");
				exit(1);
		}
	}

	setInputMode(); //saves and changes attributes
	atexit(resetInputMode);
	if (debug)
	{
		printAttributes(&termAttributes); //should be new attributes
	}

	if (shell)
	{
		signal(SIGPIPE, sig_handler);

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

				dup2(fromShell[1], 1); 
				dup2(fromShell[1], 2); 
				dup2(toShell[0], 0); 

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
			} else {
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

				fds[0].fd = STDIN_FILENO;
				fds[0].events = POLLIN;
				fds[1].fd = fromShell[0];
				fds[1].events = POLLIN | POLLHUP | POLLERR;

				while(1)
				{
					ret = poll(fds, 2, 0);
					if (ret == -1) {
						fprintf(stderr, "polling error. errno: %d, message: %s \n", errno, strerror(errno));
						exit(1);
					}

					if(fds[0].revents & POLLIN) //terminal to shell + echo
					{
						if (debug)
						{ printf("should read and write from terminal \n"); }
						char buffer1[256];
						nBytes = read(STDIN_FILENO, buffer1, 256);

						if(nBytes < 0)
						{
							fprintf(stderr, "error with read function. errno: %d, message: %s \n", errno, strerror(errno));
							exit(1);
						}

						doWrite(toShell[1], buffer1);
						doWrite(STDOUT_FILENO, buffer1);
					}

					if(fds[1].revents & POLLIN) //shell to stdout
					{
						if (debug)
						{ printf("should read and write from shell \n"); }
						char buffer2[256];
						nBytes = read(fromShell[0], buffer2, 256);

						if(nBytes < 0)
						{
							fprintf(stderr, "error with read function. errno: %d, message: %s \n", errno, strerror(errno));
							exit(1);
						}
						doWrite(STDOUT_FILENO, buffer2);
						//write to stdout
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

		else  // if no shell just read from input write to output
		{
			if(debug)
			{
				printf("without shell option\n");
			}

			char buffer3[256];

			while(1) {
				nBytes = read(STDIN_FILENO, buffer3, 256);

				if(nBytes < 0)
				{
					fprintf(stderr, "error with read function. errno: %d, message: %s \n", errno, strerror(errno));
					exit(1);
				}

				doWrite(STDOUT_FILENO, buffer3);
			}

		}
	}
