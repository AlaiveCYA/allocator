#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include "mylloc.h"

struct Header
{
    struct Header *previousHeader;
    struct Header *nextHeader;
    size_t size;
    bool isFree;
};

#define HEADER_SIZE sizeof(struct Header)
#define MIN_ALLOC_SIZE 1024

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static bool isInitialized = false;
static struct Header *firstHeader = NULL;


#ifdef __GNUC__
__attribute__((constructor)) static
#endif
int initializeAllocator(){

    if(isInitialized){
        return 0;
    }

    firstHeader = (struct Header*) sbrk(HEADER_SIZE);
    if(firstHeader == (void*)-1){
        return -1;
    }
    firstHeader->previousHeader = NULL;
    firstHeader->nextHeader = NULL;
    firstHeader->size = 0;
    firstHeader->isFree = true;
    isInitialized = true;
    return 0;
}



void* mylloc(size_t size_to_alloc){

    if (!isInitialized)
    {
        fprintf(stderr, "Error: Allocator not initialized\n");
        exit(EXIT_FAILURE);
    }
    {
        struct Header *currentHeader = firstHeader;
        pthread_mutex_lock(&mutex);
        while (currentHeader != NULL)
        {
            if (currentHeader->isFree && currentHeader->size >= size_to_alloc)
            {
                if (currentHeader->size >= size_to_alloc + HEADER_SIZE + MIN_ALLOC_SIZE)
                {
                    struct Header *newHeader = (struct Header*)(currentHeader + HEADER_SIZE + size_to_alloc);
                    newHeader->previousHeader = currentHeader;
                    newHeader->nextHeader = currentHeader->nextHeader;
                    newHeader->size = currentHeader->size - size_to_alloc - HEADER_SIZE;
                    newHeader->isFree = true;
                    currentHeader->nextHeader = newHeader;
                    currentHeader->size = size_to_alloc;
                }
                else{
                    currentHeader->isFree = false;
                }
                pthread_mutex_unlock(&mutex);
                return (void*)(currentHeader + 1);
                
            }
            currentHeader = currentHeader->nextHeader;
        }
        if (size_to_alloc < MIN_ALLOC_SIZE)
        {
            size_to_alloc = MIN_ALLOC_SIZE;
        }
        void* newBrk = sbrk(HEADER_SIZE + size_to_alloc);
        if (newBrk == (void*)-1)
        {
            pthread_mutex_unlock(&mutex);

            fprintf(stderr, "Error: Could not allocate memory\n");
            exit(EXIT_FAILURE);
        }
        struct Header *newHeader = (struct Header*)newBrk;
        newHeader->previousHeader = currentHeader;
        newHeader->nextHeader = NULL;
        newHeader->size = size_to_alloc;
        newHeader->isFree = false;
        if (currentHeader != NULL)
        {
            currentHeader->nextHeader = newHeader;
        }
        pthread_mutex_unlock(&mutex);
        return (void*)(newHeader + 1);
        
    }
    
}


void myfree(void* block){

    if(!isInitialized){
        fprintf(stderr, "Error: Allocator not initialized\n");
        exit(EXIT_FAILURE);
    }

    if (block == NULL)
    {
        return;
    }
    struct Header *header = (struct Header*)block - 1;
    pthread_mutex_lock(&mutex);
    header->isFree = true;
    if (header->previousHeader != NULL && header->previousHeader->isFree)
    {
        header->previousHeader->nextHeader = header->nextHeader;
        header->previousHeader->size += header->size + HEADER_SIZE;
        if (header->nextHeader != NULL)
        {
            header->nextHeader->previousHeader = header->previousHeader;
        }
        header = header->previousHeader;
    }
    if (header->nextHeader != NULL && header->nextHeader->isFree)
    {
        header->size += header->nextHeader->size + HEADER_SIZE;
        header->nextHeader = header->nextHeader->nextHeader;
        if (header->nextHeader != NULL)
        {
            header->nextHeader->previousHeader = header;
        }
    }
    pthread_mutex_unlock(&mutex);
    return;
}

int main(int argc, char const *argv[])
{
    int *a = (int*)mylloc(sizeof(int));
    *a = 5;
    printf("%d\n", *a);
    myfree(a);
    return 0;
}
