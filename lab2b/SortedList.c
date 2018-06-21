/*NAME: Aashita Patwari
EMAIL: harshupatwari@gmail.com */

#include "SortedList.h"
#include <pthread.h>
#include <string.h>

void SortedList_insert(SortedList_t *list, SortedListElement_t *element)
{
	SortedListElement_t* nextElement = list -> next;
	while(nextElement -> key != NULL && strcmp(nextElement -> key, element -> key) < 0)
	{
		nextElement = nextElement -> next;
	} //will stop right after point of inserting


	nextElement -> prev -> next = element;
	element -> prev = nextElement -> prev;
	if(opt_yield & INSERT_YIELD)
	{
		sched_yield();
	}
	element -> next = nextElement;
	nextElement -> prev = element;
}

int SortedList_delete(SortedListElement_t *element)
{
	if(element == NULL || element -> prev -> next != element || element -> next -> prev != element)
	{
		return 1;
	}

	element -> prev -> next = element -> next;
	if(opt_yield & DELETE_YIELD)
	{
		sched_yield();
	}
	element -> next -> prev = element -> prev;
	return 0;
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key)
{
	SortedListElement_t *current = list -> next;
	if(opt_yield & LOOKUP_YIELD)
	{
		sched_yield();
	}
	while(current -> key != NULL)
	{
		if(strcmp(current-> key, key) == 0)
		{
			return current;
		}

		current = current -> next;
	}

	return NULL;
}

int SortedList_length(SortedList_t *list)
{
	SortedListElement_t *current = list -> next; 
	int count = 0;
	if(opt_yield & LOOKUP_YIELD)
	{
		sched_yield();
	}
	while(current -> key != NULL)
	{
		if(current -> next -> prev != current || current -> prev -> next != current)
		{
			return -1;
		}
		count++;
		current = current -> next;
	}
	return count;
}
