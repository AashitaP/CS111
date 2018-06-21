/*NAME: Aashita Patwari
EMAIL: harshupatwari@gmail.com */

#include <stdio.h>
#include <stdlib.h>
#include <time.h> 
#include <getopt.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include "SortedList.h"

int numberThreads = 1;
int numberIter = 1;
SortedList_t *listHead;
SortedListElement_t *elements;
char* yieldOption;
char syncOption;
int opt_yield;
pthread_mutex_t lock;
volatile int lockTest = 0;
#define BILLION 1E9

void signalHandler(int signal)
{
	if(signal == SIGSEGV)
	{
		fprintf(stderr, "Segmentation fault");
		exit(2);
	}
} 

void * insertAndDelete(void* element)
{
	SortedListElement_t * startingElement = (SortedListElement_t *) element;
//	int threadNumber = *((int *)i); //if e.g. 10 iterations, 3rd thread
//	printf("threadNumber: %d \n", threadNumber);
//	int index = threadNumber * numberIter;  //position 20 is where we resume adding elements
//	printf("index: %d \n", index);
	//printf("%d \n", sizeof(startingElement));
	//printf("inserting elements");
	for(int j = 0; j < numberIter; j++) //insert elements
	{
		if(syncOption == 'm')
		{
			pthread_mutex_lock(&lock);
			SortedList_insert(listHead, startingElement + j);
			//index++;
			pthread_mutex_unlock(&lock);
		}

		else if(syncOption == 's')
		{
			while(__sync_lock_test_and_set(&lockTest, 1) == 1)
				;
			SortedList_insert(listHead, startingElement + j);
			__sync_lock_release(&lockTest);
		}

		else
		{
			SortedList_insert(listHead, startingElement + j);
			//index++;
		}
	}
	int length; //lookup length

	if(syncOption == 'm')
	{
		pthread_mutex_lock(&lock);
		length = SortedList_length(listHead);
		if(length == -1)
		{
			fprintf(stderr, "Error getting list length, pointers do not align\n");
			exit(2);
		}
		pthread_mutex_unlock(&lock);
	}

	else if(syncOption == 's')
	{
		while(__sync_lock_test_and_set(&lockTest, 1))
			;
		length = SortedList_length(listHead);
		if(length == -1)
		{
			fprintf(stderr, "Error getting list length, pointers do not align\n");
			exit(2);
		}
		__sync_lock_release(&lockTest);
	}

	else
	{
		length = SortedList_length(listHead);
		if(length == -1)
		{
			fprintf(stderr, "Error getting list length, pointers do not align\n");
			exit(2);
		}
	}

//	index = threadNumber * numberIter; //reset index

	//printf("deleting elements");

	for(int j = 0; j < numberIter; j++) 		//lookup element and delete
	{
		if(syncOption == 'm')
		{
			pthread_mutex_lock(&lock);
			SortedListElement_t* toDelete = SortedList_lookup(listHead, startingElement[j].key);
			if(toDelete == NULL)
			{
				fprintf(stderr, "Error finding element \n");
				exit(2);
			}
			if(SortedList_delete(toDelete) == 1)
			{
				fprintf(stderr, "Error deleting elements from list, pointers do not align \n");
				exit(2);
			}
			//index++;
			pthread_mutex_unlock(&lock);
		}

		else if(syncOption == 's')
		{
			while(__sync_lock_test_and_set(&lockTest, 1))
				;
			SortedListElement_t* toDelete = SortedList_lookup(listHead, startingElement[j].key);
			if(toDelete == NULL)
			{
				fprintf(stderr, "Error finding element \n");
				exit(2);
			}
			if(SortedList_delete(toDelete) == 1)
			{
				fprintf(stderr, "Error deleting elements from list, pointers do not align \n");
				exit(2);
			}

			__sync_lock_release(&lockTest);
		}

		else
		{
			SortedListElement_t* toDelete = SortedList_lookup(listHead, startingElement[j].key);
			if(toDelete == NULL)
			{
				fprintf(stderr, "Error finding element \n");
				exit(2);
			}
			if(SortedList_delete(toDelete) == 1)
			{
				fprintf(stderr, "Error deleting elements from list, pointers do not align \n");
				exit(2);
			}
			//index++;
		}
	}
	pthread_exit(NULL);
	return NULL;
}

void generateRandomKeys(int count)
{
	elements = (SortedListElement_t *) malloc(count * sizeof(SortedListElement_t));
	//printf("%d\n", count);
	if(elements == NULL)
	{
		fprintf(stderr, "Error allocating memory for elements \n");
		exit(1);
	}
	char letters[26] = "abcdefghijklmnopqrstuvwxyz";
	int choose;
	int keyLength = 15;
	for(int i = 0; i < count; i++)
	{
		//srand(time(NULL)); 
		char* newKey = (char *) malloc(sizeof(char) * (keyLength+1));
		if(newKey == NULL)
		{
			fprintf(stderr, "Error allocating memory for keys \n");
			exit(1);
		}
		for(int j = 0; j < keyLength; j++)
		{
			choose = rand() % 26;
			newKey[j] = letters[choose];
		}
		newKey[keyLength] = 0;
		elements[i].key = newKey;
	}

	/*for(int i = 0; i < count; i++)
	{
		printf("%s \n", elements[i].key);
	} */
}

int main(int argc, char ** argv)
{
	int c;
	struct timespec start,end;
	int error;
	opt_yield = 0;
	//int threadNumber = 0;

	static struct option long_options[] = {
		{"threads", required_argument, 0, 't'}, 
		{"iterations", required_argument, 0, 'i'},
		{"yield", required_argument, 0, 'y'},
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
				yieldOption = optarg;
				for(size_t i = 0; i < strlen(yieldOption); i++)
				{
					if(yieldOption[i] == 'i')
					{
						opt_yield = opt_yield | INSERT_YIELD; //bitwise operation
					}

					else if(yieldOption[i] == 'd')
					{
						opt_yield = opt_yield | DELETE_YIELD;
					}

					else if(yieldOption[i] == 'l')
					{
						opt_yield = opt_yield | LOOKUP_YIELD;
					}

					else
					{
						fprintf(stderr, "Usage: [--threads=#] [--iterations=#] [--yield=[idl]] [--sync=[ms]] \n");
						exit(2);
					}

				}
			break;	
			case 's':
				syncOption = optarg[0];
				if(syncOption != 'm' && syncOption != 's')
				{
					fprintf(stderr, "Usage: [--threads=#] [--iterations=#] [--yield=[idl]] [--sync=[ms]] \n");
					exit(2);
				}
			break;
			default: //some other option passed in
				fprintf(stderr, "Usage: [--threads=#] [--iterations=#] [--yield=[idl]] [--sync=[ms]] \n");
				exit(1);
		}
	}

	//initialize head + create elements

	listHead = (SortedList_t *) (malloc(sizeof(SortedList_t)));
	if(listHead == NULL)
	{
		fprintf(stderr, "Error allocating memory for list head\n");
		exit(1);
	}
	listHead -> prev = listHead;
	listHead -> next = listHead;
	listHead -> key = NULL;

	int numberElements = numberThreads * numberIter;
	generateRandomKeys(numberElements);

	//register handler

	if(signal(SIGSEGV, signalHandler) == SIG_ERR)
	{
		fprintf(stderr, "Signal registering failed. Errno: %d, error message: %s\n", errno, strerror(errno));
		exit(1);
	} 

	//initialize mutex

	if(syncOption == 'm')
	{
		int error = pthread_mutex_init(&lock, NULL);
		if(error)
		{
			fprintf(stderr, "Error initializing mutex. Errno: %d, error message: %s", error, strerror(error));
			exit(1);
		}
	}

	if(clock_gettime(CLOCK_REALTIME, &start) == -1)
	{
		fprintf(stderr, "Failed to get clock time. Errno: %d, error message: %s \n", errno, strerror(errno));
		exit(1);
	}

	pthread_t thread[numberThreads];

	//printf("declared threads \n");

	for(int i=0; i < numberThreads; i++)
	{

		error = pthread_create(&thread[i], NULL, insertAndDelete, elements + i*numberIter);
		if(error) {
			fprintf(stderr, "Error creating thread. Errno: %d, error message: %s \n", error, strerror(error));
			exit(1);
		}
		//printf("Created thread %d \n", i);
	}

	//printf("created all threads \n");

	for(int i = 0; i < numberThreads; i++)
	{

		error = pthread_join(thread[i], NULL);
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

	if(SortedList_length(listHead) != 0)
	{
		fprintf(stderr, "Final length is not 0 \n");
		exit(2);
	}

	int numberOps = numberThreads * numberIter * 3;
	long long runtime = (end.tv_sec - start.tv_sec) * BILLION + (end.tv_nsec - start.tv_nsec);
	long long timePerOp = runtime/numberOps;
	int numberLists = 1;

	char* yieldPrint;

	if(opt_yield & INSERT_YIELD)
	{
		if(opt_yield & DELETE_YIELD && opt_yield & LOOKUP_YIELD)
			yieldPrint = "idl";
		else if (opt_yield & DELETE_YIELD)
			yieldPrint = "id";
		else if (opt_yield & LOOKUP_YIELD)
			yieldPrint = "il";
		else
			yieldPrint = "i";
	}
	else if(opt_yield & DELETE_YIELD)
	{
		if(opt_yield & LOOKUP_YIELD)
			yieldPrint = "dl";
		else
			yieldPrint = "d";
	}
	else if(opt_yield & LOOKUP_YIELD)
	{
		yieldPrint = "l";
	}

	if(!opt_yield)
	{
		if(syncOption == 'm')
		{
			printf("list-none-m,%d,%d,%d,%d,%lld,%lld\n", numberThreads, numberIter, numberLists, numberOps, runtime, timePerOp);
		}
		else if(syncOption == 's')
		{
			printf("list-none-s,%d,%d,%d,%d,%lld,%lld\n", numberThreads, numberIter, numberLists, numberOps, runtime, timePerOp);
		}
		else
		{
			printf("list-none-none,%d,%d,%d,%d,%lld,%lld\n", numberThreads, numberIter, numberLists , numberOps, runtime, timePerOp);
		}
	}
	else
	{
		if(syncOption == 'm')
		{
			printf("list-%s-m,%d,%d,%d,%d,%lld,%lld\n", yieldPrint, numberThreads, numberIter, numberLists, numberOps, runtime, timePerOp);
		}
		else if(syncOption == 's')
		{
			printf("list-%s-s,%d,%d,%d,%d,%lld,%lld\n", yieldPrint, numberThreads, numberIter, numberLists, numberOps, runtime, timePerOp);
		}
		else
		{
			printf("list-%s-none,%d,%d,%d,%d,%lld,%lld\n", yieldPrint, numberThreads, numberIter, numberLists, numberOps, runtime, timePerOp);
		}
	}

	free(elements);
	free(listHead);
	exit(0);
}