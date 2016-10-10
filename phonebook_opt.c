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
            pHead->lastName[len] = '\0';
            if(pHead->dtl == NULL)
                pHead->dtl = (pdetail) malloc(sizeof(detail));
            return pHead;
        }
        DEBUG_PRINT("find string = %s\n", pHead->lastName);
        pHead = pHead->pNext;
    }
    return NULL;
}
/* ThrdInitial: Store value for each thread*/
/* StartAds: The starting address of lastName,
             where the space was allocated by mmap.
   EndAdrs: The end of the address lastName can use.
   tid: Id of the threads
   nthrd : The number of the thread.
   pptr: The pointer of entry pool.
*/
ThrdArg *ThrdInitial(char *StartAdrs, char *EndAdrs, int tid, int nthrd,
                     entry *pptr)
{
    ThrdArg *thrdArg = (ThrdArg *) malloc(sizeof(ThrdArg));

    thrdArg->StartAdrs = StartAdrs;
    thrdArg->EndAdrs = EndAdrs;
    thrdArg->tid = tid;
    thrdArg->nthread = nthrd;
    thrdArg->PoolPtr = pptr;

    thrdArg->pHead = (thrdArg->pTail = thrdArg->PoolPtr);
    return thrdArg;
}

void append(void *arg)
{
    struct timespec start, end;
    double cpu_time;
    int count = 0;

    clock_gettime(CLOCK_REALTIME, &start);

    ThrdArg *thrdArg = (ThrdArg *) arg;

    entry *curEntry = thrdArg->PoolPtr;
    for (char *curData = thrdArg->StartAdrs; curData < thrdArg->EndAdrs;
            curData += MAX_LAST_NAME_SIZE, curEntry ++, count++) {
        thrdArg->pTail->pNext = curEntry;
        thrdArg->pTail = thrdArg->pTail->pNext;

        thrdArg->pTail->lastName = curData;
        DEBUG_PRINT("thread %d append string = %s\n",
                    thrdArg->tid, thrdArg->pTail->lastName);
        thrdArg->pTail->pNext = NULL;
    }
    clock_gettime(CLOCK_REALTIME, &end);
    cpu_time = diff_in_second(start, end);

    DEBUG_PRINT("thread take %lf sec, count %d\n", cpu_time, count);

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
