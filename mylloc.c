#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "mylloc.h"

struct Header
{
    struct Header* previousHeader;
    struct Header* nextHeader;
    unsigned int magicNumber;
    size_t size;
    bool isFree;
    const char* file;
    int line;
};

static struct Stats stats = {0, 0, 0, 0, 0, 0};

#define HEADER_SIZE sizeof(struct Header)
#define MIN_ALLOC_SIZE 64
#define MAGIC_NUMBER 0x272341

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static bool isInitialized = false;
static struct Header *firstHeader = NULL;

static void programExit(void);


#ifdef __GNUC__
__attribute__((constructor))
#endif
int initializeAllocator(void){

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
    firstHeader->magicNumber = MAGIC_NUMBER;
    atexit(programExit);

    return 0;
}



void* mylloc_full(size_t size_to_alloc, const char* file, int line){

    if (!isInitialized)
    {
        fprintf(stderr, "Error: Allocator not initialized\n");
        exit(EXIT_FAILURE);
    }
    else
    {

        stats.allocCalls++;

        struct Header *currentHeader = firstHeader;
        struct Header *previousHeader = currentHeader->previousHeader;

        pthread_mutex_lock(&mutex);

        

        while (currentHeader != NULL)
        {
            if (currentHeader->magicNumber != MAGIC_NUMBER)
            {
                fprintf(stderr, "Error: Broken block\n");
                fprintf(stderr, "File: %s\n", currentHeader->file);
                fprintf(stderr, "Line: %d\n", currentHeader->line);
                fprintf(stderr, "Size: %zu\n", currentHeader->size);
                exit(EXIT_FAILURE);
            }
            if (currentHeader->isFree && currentHeader->size >= size_to_alloc)
            {
                
                if (currentHeader->size >= size_to_alloc + HEADER_SIZE + MIN_ALLOC_SIZE)
                {
                    struct Header *newHeader = (struct Header*)(currentHeader + HEADER_SIZE + size_to_alloc);

                    newHeader->previousHeader = currentHeader;
                    newHeader->nextHeader = currentHeader->nextHeader;
                    newHeader->size = currentHeader->size - size_to_alloc - HEADER_SIZE;
                    newHeader->isFree = true;
                    newHeader->file = strdup(file);
                    newHeader->line = line;
                    newHeader->magicNumber = MAGIC_NUMBER;

                    currentHeader->nextHeader = newHeader;
                    currentHeader->size = size_to_alloc;
                }
                else{
                    currentHeader->isFree = false;
                }
                pthread_mutex_unlock(&mutex);

                stats.totalAllocatedBytes += size_to_alloc;
                stats.averageAllocatedBytes = stats.totalAllocatedBytes / stats.allocCalls;
                stats.peakMemory = stats.peakMemory < stats.totalAllocatedBytes ? stats.totalAllocatedBytes : stats.peakMemory;

                return (void*)(currentHeader + 1);
                
            }

            previousHeader = currentHeader;
            currentHeader = currentHeader->nextHeader;
        }

        void* newBrk = sbrk(HEADER_SIZE + size_to_alloc);
        stats.sbrkCalls++;

        if (newBrk == (void*)-1)
        {
            pthread_mutex_unlock(&mutex);

            fprintf(stderr, "Error: Could not allocate memory\n");
            exit(EXIT_FAILURE);
        }

        struct Header *newHeader = (struct Header*)newBrk;

        newHeader->previousHeader = previousHeader;
        newHeader->nextHeader = NULL;
        newHeader->size = size_to_alloc;
        newHeader->isFree = false;
        newHeader->file = strdup(file);
        newHeader->line = line;
        newHeader->magicNumber = MAGIC_NUMBER;

        if (previousHeader != NULL)
        {
            previousHeader->nextHeader = newHeader;
        }

        pthread_mutex_unlock(&mutex);

        stats.totalAllocatedBytes += size_to_alloc;
        stats.peakMemory = stats.peakMemory < stats.totalAllocatedBytes ? stats.totalAllocatedBytes : stats.peakMemory;
        stats.averageAllocatedBytes = stats.totalAllocatedBytes / stats.allocCalls;

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

    if (header->magicNumber != MAGIC_NUMBER)
    {
        abort();
    }

    if (header->isFree)
    {
        fprintf(stderr, "Error: Block already freed\n");
        exit(EXIT_FAILURE);
    }
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

static void programExit(void){

    if(!isInitialized){
        return;
    }

    struct Header *currentHeader = firstHeader;


    while (currentHeader != NULL)
    {
        if (!currentHeader->isFree)
        {
            stats.notFreedBlocks++;
        }
        currentHeader = currentHeader->nextHeader;
    }

    fprintf(stderr, "Total number of allocations: %zu\n", stats.allocCalls);
    fprintf(stderr, "Total number of sbrk calls: %zu\n", stats.sbrkCalls);
    fprintf(stderr, "Total number of not freed blocks: %zu\n", stats.notFreedBlocks);
    fprintf(stderr, "Total number of bytes allocated: %zu\n", stats.totalAllocatedBytes);
    fprintf(stderr, "Peak memory usage: %zu\n", stats.peakMemory);
    fprintf(stderr, "Average memory usage: %zu\n", stats.averageAllocatedBytes);
    return;
}

void dumpMemory(void) {
    struct Header *currentHeader = firstHeader;

    while (currentHeader != NULL)
    {
        if (currentHeader->magicNumber != MAGIC_NUMBER)
        {
            fprintf(stderr, "Error: Broken block\n");
            fprintf(stderr, "File: %s\n", currentHeader->file);
            fprintf(stderr, "Line: %d\n", currentHeader->line);
            fprintf(stderr, "Size: %zu\n", currentHeader->size);
            exit(EXIT_FAILURE);
        }
        if(currentHeader == firstHeader){
            fprintf(stderr, "{BLOCK start %p end %p, size %zu, allocated %s, %d}\n",
                (void*)currentHeader,
                (void*)((char*)currentHeader + currentHeader->size),
                currentHeader->size,
                currentHeader->file,
                currentHeader->line);
        }
        currentHeader = currentHeader->nextHeader;
    }
}

void getStats(struct Stats* stats_out) {
    *stats_out = stats;
}