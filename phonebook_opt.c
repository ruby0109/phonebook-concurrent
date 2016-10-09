#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "phonebook_opt.h"
#include "debug.h"

entry *findName(char lastname[], entry *pHead)
{
    size_t len = strlen(lastname);
    while (pHead != NULL) {
        if (strncasecmp(lastname, pHead->lastName, len) == 0
                && (pHead->lastName[len] == '\n' ||
                    pHead->lastName[len] == '\0')) {
            pHead->lastName = (char *) malloc(sizeof(char) *
                                              MAX_LAST_NAME_SIZE);
            memset(pHead->lastName, '\0', MAX_LAST_NAME_SIZE);
            strcpy(pHead->lastName, lastname);
            pHead->dtl = (pdetail) malloc(sizeof(detail));
            return pHead;
        }
        dprintf("find string = %s\n", pHead->lastName);
        pHead = pHead->pNext;
    }
    return NULL;
}
/* ThrdInitial: Store value for each thread*/
/* StartAds: The starting address of lastName, where the space was allocated by mmap.
   EndAdrs: The end of the address lastName can use.
   tid: Id of the threads
   nthrd : The number of the thread.
   pptr: The pointer of entry pool.   
*/ 
ThrdStack *ThrdInitial(char *StartAdrs, char *EndAdrs, int tid, int nthrd,
                       entry *pptr)
{
    ThrdStack *stack = (ThrdStack *) malloc(sizeof(ThrdStack));

    stack->StartAdrs = StartAdrs;
    stack->EndAdrs = EndAdrs;
    stack->tid = tid;
    stack->nthread = nthrd;
    stack->PoolPtr = pptr;

    stack->pHead = (stack->pTail = stack->PoolPtr);
    return stack;
}

void append(void *arg)
{
    struct timespec start, end;
    double cpu_time;

    clock_gettime(CLOCK_REALTIME, &start);

    ThrdStack *stack = (ThrdStack *) arg;

    int count = 0;
    entry *j = stack->PoolPtr;
    for (char *i = stack->StartAdrs; i < stack->EndAdrs;
            i += MAX_LAST_NAME_SIZE * stack->nthread,
            j += stack->nthread,count++) {
        stack->pTail->pNext = j;
        stack->pTail = stack->pTail->pNext;

        stack->pTail->lastName = i;
        dprintf("thread %d append string = %s\n",
                stack->tid, stack->pTail->lastName);
        stack->pTail->pNext = NULL;
    }
    clock_gettime(CLOCK_REALTIME, &end);
    cpu_time = diff_in_second(start, end);

    dprintf("thread take %lf sec, count %d\n", cpu_time, count);

    pthread_exit(NULL);
}

void show_entry(entry *pHead)
{
    while (pHead != NULL) {
        printf("lastName = %s\n", pHead->lastName);
        pHead = pHead->pNext;
    }
}

static double diff_in_second(struct timespec t1, struct timespec t2)
{
    struct timespec diff;
    if (t2.tv_nsec-t1.tv_nsec < 0) {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec - 1;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec + 1000000000;
    } else {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec;
    }
    return (diff.tv_sec + diff.tv_nsec / 1000000000.0);
}
