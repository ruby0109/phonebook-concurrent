#define  _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>

#include IMPL

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
    /* check file opening */
    fp = fopen(DICT_FILE, "r");
    if (!fp) {
        printf("cannot open the file\n");
        return -1;
    }
#else
/* Align data file to MAX_LAST_NAME_SIZE one line*/
#include "file.c"
#include "debug.h"
#include <fcntl.h>
#define ALIGN_FILE "align.txt"
    file_align(DICT_FILE, ALIGN_FILE, MAX_LAST_NAME_SIZE);
    int fd = open(ALIGN_FILE, O_RDONLY | O_NONBLOCK);
    off_t fs = fsize(ALIGN_FILE);
#endif

    /* build the entry */
    entry *pHead, *e;
    pHead = (entry *) malloc(sizeof(entry));
    printf("size of entry : %lu bytes\n", sizeof(entry));
    e = pHead;
    e->pNext = NULL;

#if defined(__GNUC__)
    __builtin___clear_cache((char *) pHead, (char *) pHead + sizeof(entry));
#endif

#if defined(OPT)

#ifndef THREAD_NUM
#define THREAD_NUM 4
#endif
    clock_gettime(CLOCK_REALTIME, &start);

    /* mmap for data file*/
    char *map = mmap(NULL, fs, PROT_READ, MAP_SHARED, fd, 0);
    assert(map && "mmap error");

    /* entry pool for allocate space for the whole entry*/
    entry *entry_pool = (entry *) malloc(sizeof(entry) *
                                         fs / MAX_LAST_NAME_SIZE);

    assert(entry_pool && "entry_pool error");
    /* no use in Linuxthread and for compatibility*/
    pthread_setconcurrency(THREAD_NUM + 1);
    
    pthread_t *tid = (pthread_t *) malloc(sizeof(pthread_t) * THREAD_NUM);
    ThrdStack **stack = (ThrdStack **) malloc(sizeof(ThrdStack *) * THREAD_NUM);
    for (int i = 0; i < THREAD_NUM; i++)
    /* ThrdInitial(char *StartAdrs, char *EndAdrs, int tid, int nthrd, entry *pptr) */
    /* ThrdInitial: Store value for each thread*/
        stack[i] = ThrdInitial(map + MAX_LAST_NAME_SIZE * i, map + fs, i,
                              THREAD_NUM, entry_pool + i);
    /* Thread append each list in way of column major*/ 
    clock_gettime(CLOCK_REALTIME, &mid);
    for (int i = 0; i < THREAD_NUM; i++)
        pthread_create( &tid[i], NULL, (void *) &append, (void *) stack[i]);

    for (int i = 0; i < THREAD_NUM; i++)
        pthread_join(tid[i], NULL);

    entry *etmp;
    pHead = pHead->pNext;
    for (int i = 0; i < THREAD_NUM; i++) {
        if (i == 0) {
            pHead = stack[i]->pHead->pNext;
            dprintf("Connect %d head string %s %p\n", i,
                    stack[i]->pHead->pNext->lastName, stack[i]->StartAdrs);
        } else {
            etmp->pNext = stack[i]->pHead->pNext;
            dprintf("Connect %d head string %s %p\n", i,
                    stack[i]->pHead->pNext->lastName, stack[i]->StartAdrs);
        }

        etmp = stack[i]->pTail;
        dprintf("Connect %d tail string %s %p\n", i,
                stack[i]->pTail->lastName, stack[i]->StartAdrs);
        dprintf("round %d\n", i);
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
#endif

#ifndef OPT
    /* close file as soon as possible */
    fclose(fp);
#endif

    e = pHead; 

    /* the givn last name to find */  
    char input[MAX_LAST_NAME_SIZE] = "amability";
    e = pHead;

    assert(findName(input, e) &&
           "Did you implement findName() in " IMPL "?");
    assert(0 == strcmp(findName(input, e)->lastName, "amability"));

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

#ifndef OPT
    if (pHead->pNext) free(pHead->pNext);
    free(pHead);
#else
    free(entry_pool);
    free(tid);
    free(stack);
    munmap(map, fs);
#endif
    return 0;
}
