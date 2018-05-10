#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

void segfault_handler(int signum){
	if(signum == SIGSEGV) {
		fprintf(stderr, "segmentation fault: error code = %d error message = %s \n", errno, strerror(errno));
		exit(4);
	}
}

int main(int argc, char **argv)
{
	int c;
	char cur;
	int errno;
	int ifd;
	int ofd;

	static struct option long_options[] = {
		{"input", required_argument, 0, 'i'}, 
		{"output", required_argument, 0, 'o'},
		{"segfault", no_argument, 0, 's'},
		{"catch", no_argument, 0, 'c'},
		{0, 0, 0, 0}
	};

	while( (c = getopt_long(argc, argv, "i:o:sc", long_options, NULL)) != -1) {
		switch(c) {
			case 'i':
				ifd = open(optarg, O_RDONLY);
				if(ifd == -1)
				{
					fprintf(stderr, "unable to open input file: error code = %d error message = %s \n", errno, strerror(errno));
					exit(2);
				}
				if (ifd >= 0) {
					close(0);
					dup(ifd);
					close(ifd);
				}
				break;
			case 'o':
				ofd = creat(optarg, 0666);
				if(ofd == -1)
				{
					fprintf(stderr, "unable to create output file : error code = %d error message = %s \n", errno, strerror(errno));
					exit(3);
				}
				if (ofd >= 0) {
					close(1);
					dup(ofd);
					close(ofd);
				}
				break;
			case 'c':
				signal(SIGSEGV, segfault_handler);
				break;
			case 's':
				;
				char* x = NULL;
				*x = 'A';
			break;
			default:
				fprintf(stderr, "error: wrong argument \n");
				exit(1);
		}
	}
	while(read(0, &cur, 1) > 0)
	{
		if(write(1, &cur, 1) < 1)
		{
			fprintf(stderr, "error in writing: error code = %d error message = %s \n", errno, strerror(errno));
			exit(3);
		}
	}
	exit(0);
}
	
