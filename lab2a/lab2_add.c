/*NAME: Aashita Patwari
EMAIL: harshupatwari@gmail.com */

#include <stdio.h>
#include <stdlib.h>
#include <time.h> 
#include <getopt.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

int numberThreads = 1;
int numberIter = 1;
int opt_yield;
char syncOption;
pthread_mutex_t lock;
volatile int lockTest = 0;
#define BILLION 1E9

void add(long long *pointer, long long value) {
    long long sum = *pointer + value;
    if (opt_yield)
    	sched_yield();
    *pointer = sum;
}

/*
void add(long long *pointer, long long value) {
                long long sum = *pointer + value;
                *pointer = sum;
        }
*/
void *addIter(void* pointer)
{
	int i;
	long long * counter = (long long *)pointer;
	for(i = 0; i < numberIter; i++)
	{
		if(syncOption == 'm')
		{
			pthread_mutex_lock(&lock);
			add(counter, 1);
			pthread_mutex_unlock(&lock);
		}
		else if(syncOption == 's')
		{
			while(__sync_lock_test_and_set(&lockTest, 1))
				;
			add(counter, 1);
			__sync_lock_release(&lockTest);
		}


		else if(syncOption == 'c')
		{
			long long addedValue, oldValue;
			do 
			{
				oldValue = *counter;
				addedValue = oldValue + 1;
				if(opt_yield)
				{
					sched_yield();
				}
			}
			while (oldValue != __sync_val_compare_and_swap(counter, oldValue, addedValue));
		}

		else
		{
			add(counter, 1);
		}
	}

	//printf("%lld \n", counter);

	for(i = 0; i < numberIter; i++)
	{
		if(syncOption == 'm')
		{
			pthread_mutex_lock(&lock);
			add(counter, -1);
			pthread_mutex_unlock(&lock);
		}

		else if(syncOption == 's')
		{
			while(__sync_lock_test_and_set(&lockTest, 1))
				;
			add(counter, -1);
			__sync_lock_release(&lockTest);
		}

		else if(syncOption == 'c')
		{
			long long addedValue, oldValue;
			do 
			{
				oldValue = *counter;
				addedValue = oldValue - 1;
				if(opt_yield)
				{
					sched_yield();
				}
			}
			while (oldValue != __sync_val_compare_and_swap(counter, oldValue, addedValue));
		}

		else
		{
			add(counter, -1);
		}
	}

	//printf("%lld \n", counter);

	pthread_exit(NULL);
	return NULL;

}

int main(int argc, char ** argv)
{
	//printf("starting program \n");
	int c;
	struct timespec start,end;
	long long counter = 0;
	int i;
	opt_yield = 0;

	static struct option long_options[] = {
		{"threads", required_argument, 0, 't'}, 
		{"iterations", required_argument, 0, 'i'},
		{"yield", no_argument, 0, 'y'},
		{"sync", required_argument, 0, 's'},
		{0, 0, 0, 0}
	};

	while((c = getopt_long(argc, argv, "sd", long_options, NULL)) != -1) {
		switch(c) {
			case 't':
				numberThreads = atoi(optarg);
			break;
			case 'i':
				numberIter = atoi(optarg);
			break;	
			case 'y':	
				opt_yield = 1;
			break;	
			case 's':
				syncOption = optarg[0];
				if(syncOption != 'm' && syncOption != 'c' && syncOption != 's')
				{
					fprintf(stderr, "Usage: [--threads=#] [--iterations=#] [--yield=[idl]] [--sync=[ms]] \n");
					exit(2);
				}
			break;
			default: //some other option passed in
				fprintf(stderr, "Usage: [--threads=#] [--iterations=#] [--yield] [--sync=[ms]] \n");
				exit(1);
		}
	}

	if(syncOption == 'm')
	{
		int error = pthread_mutex_init(&lock, NULL);
		if(error)
		{
			fprintf(stderr, "Error initializing mutex. Errno: %d, error message: %s", error, strerror(error));
			exit(1);
		}
	}

	if(clock_gettime(CLOCK_REALTIME, &start) ==  -1)
	{
		fprintf(stderr, "Failed to get clock time. Errno: %d, error message: %s \n", errno, strerror(errno));
		exit(1);
	}

	//printf("finished getting time \n");

	pthread_t thread[numberThreads];

	//printf("declared threads \n");

	for(i = 0; i < numberThreads; i++)
	{

		int error = pthread_create(&thread[i], NULL, addIter, &counter);
		if(error) {
			fprintf(stderr, "Error creating thread. Errno: %d, error message: %s \n", error, strerror(error));
			exit(1);
		}
		//printf("Created thread %d \n", i);
	}

	//printf("created all threads \n");

	for(i = 0; i < numberThreads; i++)
	{

		int error = pthread_join(thread[i], NULL);
		if(error) {
			fprintf(stderr, "Error joining thread. Errno: %d, error message: %s \n", error, strerror(error));
			exit(1);
		}
		//printf("Joined thread %d \n", i);
	}

	//printf("joined all threads \n");

	//printf("Main: completed \n");

	if(clock_gettime(CLOCK_REALTIME, &end) == -1)
	{
		fprintf(stderr, "Failed to get clock time. Errno: %d, error message: %s \n", errno, strerror(errno));
		exit(1);
	}


	int numberOps = numberThreads * numberIter * 2;
	long long runtime = (end.tv_sec - start.tv_sec) * BILLION + (end.tv_nsec - start.tv_nsec);
	long long timePerOp = runtime/numberOps;

	if(!opt_yield)
	{
		if(syncOption == 'm')
		{
			printf("add-m,%d,%d,%d,%lld,%lld,%lld\n", numberThreads, numberIter, numberOps, runtime, timePerOp, counter);
		}
		else if(syncOption == 's')
		{
			printf("add-s,%d,%d,%d,%lld,%lld,%lld\n", numberThreads, numberIter, numberOps, runtime, timePerOp, counter);
		}
		else if(syncOption == 'c')
		{
			printf("add-c,%d,%d,%d,%lld,%lld,%lld\n", numberThreads, numberIter, numberOps, runtime, timePerOp, counter);
		}
		else
		{
			printf("add-none,%d,%d,%d,%lld,%lld,%lld\n", numberThreads, numberIter, numberOps, runtime, timePerOp, counter);
		}
	}
	else
	{
		if(syncOption == 'm')
		{
			printf("add-yield-m,%d,%d,%d,%lld,%lld,%lld\n", numberThreads, numberIter, numberOps, runtime, timePerOp, counter);
		}
		else if(syncOption == 's')
		{
			printf("add-yield-s,%d,%d,%d,%lld,%lld,%lld\n", numberThreads, numberIter, numberOps, runtime, timePerOp, counter);
		}
		else if(syncOption == 'c')
		{
			printf("add-yield-c,%d,%d,%d,%lld,%lld,%lld\n", numberThreads, numberIter, numberOps, runtime, timePerOp, counter);
		}
		else
		{
			printf("add-yield-none,%d,%d,%d,%lld,%lld,%lld\n", numberThreads, numberIter, numberOps, runtime, timePerOp, counter);
		}
	}
	exit(0);
}