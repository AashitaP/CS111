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
int numberLists = 1;
//SortedList_t *listHead;
SortedListElement_t *elements;
SortedList_t **	listHeads;
char* yieldOption;
char syncOption;
int opt_yield;
int list = 0;
pthread_mutex_t* lockArray;
int* lockTests; //does it need to be volatile
#define BILLION 1E9

unsigned long hash(const char *str) //hash value determines which sublist the inserted element belongs in   //http://www.cse.yorku.ca/~oz/hash.html
{
	unsigned long hash = 5381;
	int c;

	while((c = *str++))
		hash = ((hash<<5) + hash) + c;
	return hash % numberLists;
}


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
	struct timespec * threadTime = NULL;
	threadTime = malloc(sizeof(*threadTime));
	if(threadTime == NULL)
	{
		fprintf(stderr, "Error allocating memory \n");
		exit(1);
	}
	threadTime -> tv_sec = 0;
	threadTime -> tv_nsec = 0;
//	int threadNumber = *((int *)i); //if e.g. 10 iterations, 3rd thread
//	printf("threadNumber: %d \n", threadNumber);
//	int index = threadNumber * numberIter;  //position 20 is where we resume adding elements
//	printf("index: %d \n", index);
	//printf("%d \n", sizeof(startingElement));
	//printf("inserting elements");
	for(int j = 0; j < numberIter; j++) //insert elements
	{
		struct timespec startLock, endLock;
		int hashValue = hash(startingElement[j].key); //get hash value for each element
		if(syncOption == 'm') //need to get lock acquisition time
		{
			if(clock_gettime(CLOCK_MONOTONIC, &startLock) == -1)
			{
				fprintf(stderr, "Failed to get clock time. Errno: %d, error message: %s \n", errno, strerror(errno));
				exit(1);
			}
			pthread_mutex_lock(&lockArray[hashValue]); //same hash value can be used to index into array of synchronizing objects, a lock for each sub list
			if(clock_gettime(CLOCK_MONOTONIC, &endLock) == -1)
			{
				fprintf(stderr, "Failed to get clock time. Errno: %d, error message: %s \n", errno, strerror(errno));
				exit(1);
			}
			SortedList_insert(listHeads[hashValue], startingElement + j); //adding that particular element to sublist corresponding to hash value
			//index++;
			threadTime -> tv_sec += (endLock.tv_sec - startLock.tv_sec);
			threadTime -> tv_nsec += (endLock.tv_nsec - startLock.tv_nsec);
			//printf("%lld \n", *threadTime);
			pthread_mutex_unlock(&lockArray[hashValue]);
		}

		else if(syncOption == 's')
		{
			//lock get time 
			if(clock_gettime(CLOCK_MONOTONIC, &startLock) == -1)
			{
				fprintf(stderr, "Failed to get clock time. Errno: %d, error message: %s \n", errno, strerror(errno));
				exit(1);
			}
			while(__sync_lock_test_and_set(&lockTests[hashValue], 1) == 1)
				;
			//lock end time
			if(clock_gettime(CLOCK_MONOTONIC, &endLock) == -1)
			{
				fprintf(stderr, "Failed to get clock time. Errno: %d, error message: %s \n", errno, strerror(errno));
				exit(1);
			}
			SortedList_insert(listHeads[hashValue], startingElement + j);
			threadTime -> tv_sec += (endLock.tv_sec - startLock.tv_sec);
			threadTime -> tv_nsec += (endLock.tv_nsec - startLock.tv_nsec);
			//printf("%lld \n", *threadTime);
			__sync_lock_release(&lockTests[hashValue]);
		}

		else
		{
			SortedList_insert(listHeads[hashValue], startingElement + j);
		}
	}
	int length = 0; //lookup length
	int lengthSublist;

	for(int i = 0; i < numberLists; i++)
	{
		struct timespec startLock, endLock;
		if(syncOption == 'm')	
		{
			if(clock_gettime(CLOCK_MONOTONIC, &startLock) == -1)
			{
				fprintf(stderr, "Failed to get clock time. Errno: %d, error message: %s \n", errno, strerror(errno));
				exit(1);
			}
			pthread_mutex_lock(&lockArray[i]);
			if(clock_gettime(CLOCK_MONOTONIC, &endLock) == -1)
			{
				fprintf(stderr, "Failed to get clock time. Errno: %d, error message: %s \n", errno, strerror(errno));
				exit(1);
			}
			lengthSublist = SortedList_length(listHeads[i]);
			if(lengthSublist == -1)
			{
				fprintf(stderr, "Error getting list length, pointers do not align\n");
				exit(2);
			}
			length += lengthSublist;
			threadTime -> tv_sec += (endLock.tv_sec - startLock.tv_sec);
			threadTime -> tv_nsec += (endLock.tv_nsec - startLock.tv_nsec);
			//printf("%lld \n", *threadTime);
			pthread_mutex_unlock(&lockArray[i]);
		}

		else if(syncOption == 's')
		{
			if(clock_gettime(CLOCK_MONOTONIC, &startLock) == -1)
			{
				fprintf(stderr, "Failed to get clock time. Errno: %d, error message: %s \n", errno, strerror(errno));
				exit(1);
			}
			while(__sync_lock_test_and_set(&lockTests[i], 1))
			;
			if(clock_gettime(CLOCK_MONOTONIC, &endLock) == -1)
			{
				fprintf(stderr, "Failed to get clock time. Errno: %d, error message: %s \n", errno, strerror(errno));
				exit(1);
			}
			lengthSublist = SortedList_length(listHeads[i]);
			if(lengthSublist == -1)
			{
				fprintf(stderr, "Error getting list length, pointers do not align\n");
				exit(2);
			}
			length += lengthSublist;
			threadTime -> tv_sec += (endLock.tv_sec - startLock.tv_sec);
			threadTime -> tv_nsec += (endLock.tv_nsec - startLock.tv_nsec);
			//printf("%lld \n", *threadTime);
			__sync_lock_release(&lockTests[i]);
		}

		else
		{
			lengthSublist = SortedList_length(listHeads[i]);
			if(lengthSublist == -1)
			{
				fprintf(stderr, "Error getting list length, pointers do not align\n");
				exit(2);
			}
			length += lengthSublist;
		}
	}

//	index = threadNumber * numberIter; //reset index

	//printf("deleting elements");

	for(int j = 0; j < numberIter; j++) 		//lookup element and delete
	{
		struct timespec startLock, endLock;
		int hashValue = hash(startingElement[j].key);
		if(syncOption == 'm')
		{
			if(clock_gettime(CLOCK_MONOTONIC, &startLock) == -1)
			{
				fprintf(stderr, "Failed to get clock time. Errno: %d, error message: %s \n", errno, strerror(errno));
				exit(1);
			}
			pthread_mutex_lock(&lockArray[hashValue]);
			if(clock_gettime(CLOCK_MONOTONIC, &endLock) == -1)
			{
				fprintf(stderr, "Failed to get clock time. Errno: %d, error message: %s \n", errno, strerror(errno));
				exit(1);
			}
			threadTime -> tv_sec += (endLock.tv_sec - startLock.tv_sec);
			threadTime -> tv_nsec += (endLock.tv_nsec - startLock.tv_nsec);
			//printf("%lld \n", *threadTime);
			SortedListElement_t* toDelete = SortedList_lookup(listHeads[hashValue], startingElement[j].key);
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
			pthread_mutex_unlock(&lockArray[hashValue]);
		}

		else if(syncOption == 's')
		{
			if(clock_gettime(CLOCK_MONOTONIC, &startLock) == -1)
			{
				fprintf(stderr, "Failed to get clock time. Errno: %d, error message: %s \n", errno, strerror(errno));
				exit(1);
			}
			while(__sync_lock_test_and_set(&lockTests[hashValue], 1))
				;
			if(clock_gettime(CLOCK_MONOTONIC, &endLock) == -1)
			{
				fprintf(stderr, "Failed to get clock time. Errno: %d, error message: %s \n", errno, strerror(errno));
				exit(1);
			}
			threadTime -> tv_sec += (endLock.tv_sec - startLock.tv_sec);
			threadTime -> tv_nsec += (endLock.tv_nsec - startLock.tv_nsec);
			//printf("%lld \n", *threadTime);
			SortedListElement_t* toDelete = SortedList_lookup(listHeads[hashValue], startingElement[j].key);
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

			__sync_lock_release(&lockTests[hashValue]);
		}

		else
		{
			SortedListElement_t* toDelete = SortedList_lookup(listHeads[hashValue], startingElement[j].key);
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
	//printf("%lld \n", *threadTime);
	
	pthread_exit(threadTime);
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
		{"lists", required_argument, 0, 'l'},
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
			case 'l':
				list = 1;
				numberLists = atoi(optarg);
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
						fprintf(stderr, "Usage: [--threads=#] [--iterations=#] [--yield=[idl]] [--sync=[ms]] [--lists=#] \n");
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

	//listHead = (SortedList_t *) (malloc(sizeof(SortedList_t)));
	//if(listHead == NULL)
	/*{
		fprintf(stderr, "Error allocating memory for list head\n");
		exit(1);
	} */
	//listHead -> prev = listHead;
	//listHead -> next = listHead;
	//listHead -> key = NULL;

	listHeads = (SortedList_t **) (malloc(numberLists * sizeof(SortedList_t)));
	if(listHeads == NULL)
	{
		fprintf(stderr, "Error allocating memory \n");
		exit(1);
	}
	for(int i = 0; i < numberLists; i++)
	{
		listHeads[i] = (SortedList_t *) malloc(sizeof(SortedList_t));
		if(listHeads[i] == NULL)
		{
			fprintf(stderr, "Error allocating memory \n");
			exit(1);
		}
		listHeads[i] -> prev = listHeads[i];
		listHeads[i] -> next = listHeads[i];
		listHeads[i] -> key = NULL;
	}
	


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

		lockArray = (pthread_mutex_t *) (malloc(sizeof(pthread_mutex_t) * numberLists));
		if(lockArray == NULL)
		{
			fprintf(stderr, "Error allocating memory \n");
			exit(1);
		}
		for(int i = 0; i < numberLists; i++)
		{
			int error = pthread_mutex_init(&lockArray[i], NULL);
			if(error)
			{
				fprintf(stderr, "Error initializing mutex. Errno: %d, error message: %s", error, strerror(error));
				exit(1);
			}
		}
	}

	else if (syncOption == 's')
	{
		lockTests = (int *) (malloc(sizeof(int) * numberLists));
		if(lockTests == NULL)
		{
			fprintf(stderr, "Error allocating memory \n");
			exit(1);
		}
		memset(lockTests, 0, sizeof(int) * numberLists); //set all spin locks to 0
	}

	if(clock_gettime(CLOCK_MONOTONIC, &start) == -1)
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

	void * threadTime;
	struct timespec totalThreadTimes;
	totalThreadTimes.tv_nsec = 0;
	totalThreadTimes.tv_sec = 0;

	for(int i = 0; i < numberThreads; i++)
	{

		error = pthread_join(thread[i], &threadTime);
		if(error) {
			fprintf(stderr, "Error joining thread. Errno: %d, error message: %s \n", error, strerror(error));
			exit(1);
		}
		//printf("Joined thread %d \n", i);

		/*long long * returnValue = (long long *) (threadTime);
		printf("%lld \n", *(returnValue));
	
		totalThreadTimes += *(returnValue);
		printf("%lld \n", totalThreadTimes); */

		totalThreadTimes.tv_sec = ((struct timespec *)threadTime) -> tv_sec;
		totalThreadTimes.tv_nsec = ((struct timespec *) threadTime) -> tv_nsec;
		//printf("%lld", totalThreadTimes);

	}

	//printf("joined all threads \n");

	//printf("Main: completed \n");

	if(clock_gettime(CLOCK_MONOTONIC, &end) == -1)
	{
		fprintf(stderr, "Failed to get clock time. Errno: %d, error message: %s \n", errno, strerror(errno));
		exit(1);
	}

	if(SortedList_length(listHeads[0]) != 0)
	{
		fprintf(stderr, "Final length is not 0 \n");
		exit(2);
	}

	int numberOps = numberThreads * numberIter * 3; //numberIter * 2  + 1
	long long runtime = (end.tv_sec - start.tv_sec) * BILLION + (end.tv_nsec - start.tv_nsec);
	long long timePerOp = runtime/numberOps;
	long long totalThreadTime = totalThreadTimes.tv_sec * BILLION + totalThreadTimes.tv_nsec;
	long long avgWaitForLock = totalThreadTime/(numberThreads * (numberIter * 3 + 1)); //3 lock ops for every iteration + 1 length 

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
			printf("list-none-m,%d,%d,%d,%d,%lld,%lld,%lld\n", numberThreads, numberIter, numberLists, numberOps, runtime, timePerOp, avgWaitForLock);
		}
		else if(syncOption == 's')
		{
			printf("list-none-s,%d,%d,%d,%d,%lld,%lld,%lld\n", numberThreads, numberIter, numberLists, numberOps, runtime, timePerOp, avgWaitForLock);
		}
		else
		{
			printf("list-none-none,%d,%d,%d,%d,%lld,%lld,%lld\n", numberThreads, numberIter, numberLists , numberOps, runtime, timePerOp, avgWaitForLock);
		}
	}
	else
	{
		if(syncOption == 'm')
		{
			printf("list-%s-m,%d,%d,%d,%d,%lld,%lld,%lld\n", yieldPrint, numberThreads, numberIter, numberLists, numberOps, runtime, timePerOp, avgWaitForLock);
		}
		else if(syncOption == 's')
		{
			printf("list-%s-s,%d,%d,%d,%d,%lld,%lld,%lld\n", yieldPrint, numberThreads, numberIter, numberLists, numberOps, runtime, timePerOp, avgWaitForLock);
		}
		else
		{
			printf("list-%s-none,%d,%d,%d,%d,%lld,%lld,%lld\n", yieldPrint, numberThreads, numberIter, numberLists, numberOps, runtime, timePerOp, avgWaitForLock);
		}
	}

	free(elements);
	free(listHeads);
	exit(0);
}