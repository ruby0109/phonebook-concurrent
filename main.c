#define  _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include IMPL

#if defined(OPT)
#include <pthread.h>
#include <sys/mman.h>
#include <unistd.h>
#include "file.c"
#include "debug.h"
#include <fcntl.h>
#define ALIGN_FILE "align.txt"

#ifndef THREAD_NUM
#define THREAD_NUM 4
#endif
#endif

#define DICT_FILE "./dictionary/words.txt"

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

int main(int argc, char *argv[])
{
#ifndef OPT
    FILE *fp;
    int i = 0;
    char line[MAX_LAST_NAME_SIZE];
#else
    struct timespec mid;
#endif
    struct timespec start, end;
    double cpu_time1, cpu_time2;

#ifndef OPT
    /*==========File Preprocessing==========*/

    /* check file opening */
    fp = fopen(DICT_FILE, "r");
    if (!fp) {
        printf("cannot open the file\n");
        return -1;
    }
#else
    /* Align data file to MAX_LAST_NAME_SIZE one line*/
    file_align(DICT_FILE, ALIGN_FILE, MAX_LAST_NAME_SIZE);
    int fd = open(ALIGN_FILE, O_RDONLY | O_NONBLOCK);
    off_t fs = fsize(ALIGN_FILE);
#endif

    /*===========Build The Entry==========*/
    entry *pHead, *e;
    pHead = (entry *) malloc(sizeof(entry));
    printf("size of entry : %lu bytes\n", sizeof(entry));
    e = pHead;
    e->pNext = NULL;

#if defined(__GNUC__)
    __builtin___clear_cache((char *) pHead, (char *) pHead + sizeof(entry));
#endif
    /* Build The Entry With MMap And Pthread*/
#if defined(OPT)

    clock_gettime(CLOCK_REALTIME, &start);

    int numEntry;//Number of the entry of the data
    int entryPerThrd;//Number of the entry allocated to the threads
    pthread_t thread[THREAD_NUM];
    ThrdArg *thrdArg[THREAD_NUM];

    /* mmap for data file*/
    char *map = mmap(NULL, fs, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    assert(map && "mmap error");

    /* entry pool for allocate space for the whole entry*/
    numEntry = fs / MAX_LAST_NAME_SIZE;
    entry *entry_pool = (entry *) malloc(sizeof(entry) * numEntry);
    assert(entry_pool && "entry_pool error");

    /* For compatibility*/
    pthread_setconcurrency(THREAD_NUM + 1);

    /* Multi-Threading*/
    entryPerThrd = numEntry / THREAD_NUM + 1;
    DEBUG_PRINT("entryPerThrd: %d \n", entryPerThrd);
    DEBUG_PRINT("numEntry %d \n", numEntry);
    for (int i = 0; i < THREAD_NUM; i++) {
        char *startAdrs, *endAdrs;//start address and end address of data

        startAdrs = map + entryPerThrd * i * MAX_LAST_NAME_SIZE;
        /* allocate region for threads*/
        if(i == THREAD_NUM -1)
            endAdrs = map + fs;
        else
            endAdrs = map + entryPerThrd * (i+1) * MAX_LAST_NAME_SIZE;

        /* ThrdInitial: Store value for each thread*/
        thrdArg[i] = ThrdInitial(startAdrs, endAdrs, i, THREAD_NUM,
                                 entry_pool + entryPerThrd * i);
    }
    /* Thread append each list in way of row major*/
    clock_gettime(CLOCK_REALTIME, &mid);
    for (int i = 0; i < THREAD_NUM; i++)
        pthread_create( &thread[i], NULL, (void *) &append, (void *) thrdArg[i]);

    for (int i = 0; i < THREAD_NUM; i++)
        pthread_join(thread[i], NULL);

    /* connect each lists */
    for (int i = 0; i < THREAD_NUM; i++) {
        if (i == 0) {
            pHead = thrdArg[i]->pHead;
            DEBUG_PRINT("Connect %d head string %s %p\n", i,
                        pHead->lastName, thrdArg[i]->StartAdrs);
        } else {
            e->pNext = thrdArg[i]->pHead;
            DEBUG_PRINT("Connect %d head string %s %p\n", i,
                        e->pNext->lastName, thrdArg[i]->StartAdrs);
        }

        e = thrdArg[i]->pTail;
        DEBUG_PRINT("Connect %d tail string %s %p\n", i,
                    thrdArg[i]->pTail->lastName, thrdArg[i]->StartAdrs);
        DEBUG_PRINT("round %d\n", i);
    }

    clock_gettime(CLOCK_REALTIME, &end);
    cpu_time1 = diff_in_second(start, end);

// for testing whether program load data correctly
#if defined(TEST)
#include "entry_test.c"
    test(pHead);
#endif

#else /* ! OPT */
    clock_gettime(CLOCK_REALTIME, &start);
    while (fgets(line, sizeof(line), fp)) {
        while (line[i] != '\0')
            i++;
        line[i - 1] = '\0';
        i = 0;
        e = append(line, e);
    }

    clock_gettime(CLOCK_REALTIME, &end);
    cpu_time1 = diff_in_second(start, end);
    /* close file as soon as possible */
    fclose(fp);
#endif

    e = pHead;

    /* the givn last name to find */
    char input[MAX_LAST_NAME_SIZE] = "zyxel";

    /*===========Search For The Entry==========*/
    assert(findName(input, e) &&
           "Did you implement findName() in " IMPL "?");
    assert(0 == strcmp(findName(input, e)->lastName, "zyxel"));

#if defined(__GNUC__)
    __builtin___clear_cache((char *) pHead, (char *) pHead + sizeof(entry));
#endif
    /* compute the execution time */
    clock_gettime(CLOCK_REALTIME, &start);
    findName(input, e);
    clock_gettime(CLOCK_REALTIME, &end);
    cpu_time2 = diff_in_second(start, end);

    FILE *output;
#if defined(OPT)
    output = fopen("opt.txt", "a");
#else
    output = fopen("orig.txt", "a");
#endif
    fprintf(output, "append() findName() %lf %lf\n", cpu_time1, cpu_time2);
    fclose(output);

    printf("execution time of append() : %lf sec\n", cpu_time1);
    printf("execution time of findName() : %lf sec\n", cpu_time2);

    /*==========Release The Memory===========*/
#ifndef OPT
    while(pHead != NULL) {
        e = pHead;
        pHead = pHead->pNext;
        free(e);
    }
#else
    e = pHead;
    while(e != NULL) {
        free(e->dtl);
        e = e->pNext;
    }
    free(entry_pool);
    for (int i=0; i<THREAD_NUM; ++i)
        free(thrdArg[i]);
    munmap(map, fs);
#endif
    return 0;
}
