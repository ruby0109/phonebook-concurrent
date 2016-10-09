#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "phonebook_opt.h"

#define MAX_LAST_NAME_SIZE 16
#define HASH_SIZE 42737
#define DICT_FILE "./dictionary/words.txt"

typedef struct __HASH_TABLE{
    entry **tail;
}HashTable;

HashTable *hash_ptr;

//djb2 Hash Function
unsigned int HashFunction(char *str)
{
    unsigned int hash = 5381;
    int c;

    while((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c 
    }
    return (hash & HASH_SIZE);
}

// findname for search each target
int findName_search(char lastname[], HashTable *hash_ptr)
{
    entry* list;
    int value;
    value = HashFunction(lastname);

    size_t len = strlen(lastname);
    for (list=hash_ptr->tail[value]; list != NULL ; list=list->pNext) {
        if (strncasecmp(lastname, list->lastName,len) == 0){
        return 1;
        }     
    }
    return 0;
}

// allocate memory for the new entry of the list goning to be tested
void append_search(char lastName[], HashTable *hash_ptr)
{
    entry *e;
    e = (entry *) malloc(sizeof(entry));
    int value;
    value = HashFunction(lastName);
    e->lastName = lastName;

    e->pNext=hash_ptr->tail[value];
    hash_ptr->tail[value]=e;

}


// allocate space for the Hash Table structure
HashTable *Initial_HashTable(void)
{

    hash_ptr = (HashTable* )malloc(sizeof(HashTable));
    hash_ptr->tail = malloc(HASH_SIZE*sizeof(entry*));
    int i;
    for(i=0; i<HASH_SIZE; i++) {
        hash_ptr->tail[i]=NULL;
    }
    return hash_ptr;
}

void test(entry* pHead)
{
    // use findname to test whether all elements in the list
    // open file for test and origin word.txt
    FILE *org,*test;

    hash_ptr = Initial_HashTable();
    entry *e;
    char line[MAX_LAST_NAME_SIZE];

    test = fopen("./dictionary/test.txt","a");
    
    org = fopen("./dictionary/words.txt", "r");
       if (!org) {
        printf("cannot open the file\n");
    }

    //put the list for testing into the hash table
    e = pHead;
    while(e){
        append_search(e->lastName,hash_ptr);
        fprintf(test, "apppend %s \n", e->lastName);
        e= e->pNext;
    }

    // use findname to test whether all elements in the list
    while(fgets(line, sizeof(line),org)){
        if(findName_search(line, hash_ptr) == 0 )  
            fprintf(test, "%s not found\n", line);
    }
    fclose(org);
}



