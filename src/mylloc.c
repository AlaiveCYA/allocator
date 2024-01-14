#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "mylloc.h"
#include <stdint.h>



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

static struct Stats stats = {0, 0, 0, 0, 0, 0, false};

#define HEADER_SIZE sizeof(struct Header)
#define MIN_ALLOC_SIZE 64
#define MAGIC_NUMBER 0x272341
#define CONST_16 16UL

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static bool isInitialized = false;
static struct Header *firstHeader = NULL;
static size_t currentAllocatedMemory = 0;

static void programExit(void);


#ifdef __GNUC__
__attribute__((constructor))
#endif
int initializeAllocator(void){

    if(isInitialized){
        return 0;
    }

    void* brk = sbrk(HEADER_SIZE);
    if(brk == (void*)-1){  //NOLINT
        (void)fprintf(stderr,"Error: Could not initialize allocator\n");  
        abort();
    }

    firstHeader = (struct Header*) brk;

    firstHeader->previousHeader = NULL;
    firstHeader->nextHeader = NULL;
    firstHeader->size = 0;
    firstHeader->isFree = true;
    isInitialized = true;
    firstHeader->magicNumber = MAGIC_NUMBER;
    firstHeader->file = NULL;
    firstHeader->line = 0;

    int status = atexit(programExit);
    if(status != 0){
        (void)fprintf(stderr,"Error: Could not register exit handler\n");  
        abort();
    }

    return 0;
}

void enableOutput(void){
    if(!isInitialized){
        (void)fprintf(stderr,"Error: Allocator not initialized\n");  
        return;
    }
    stats.outputEnabled = true;
}

void disableOutput(void){
    if(!isInitialized){
        (void)fprintf(stderr,"Error: Allocator not initialized\n");  
        return;
    }
    stats.outputEnabled = false;
}

void* mylloc_full(size_t size_to_alloc, const char* file, int line){

    size_t temp = size_to_alloc/CONST_16;
    size_to_alloc = (temp + 1) * CONST_16;

    if (!isInitialized)
    {
        (void)fprintf(stderr, "Error: Allocator not initialized\n");
        exit(EXIT_FAILURE); //NOLINT 
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
                (void)fprintf(stderr, "Error: Broken block\n File: %s\n Line: %d\n Size: %zu\n", currentHeader->file, currentHeader->line, currentHeader->size);  
                abort();

            }
            if (currentHeader->isFree && currentHeader->size >= size_to_alloc)
            {
                if (currentHeader->size >= size_to_alloc + HEADER_SIZE + MIN_ALLOC_SIZE)
                {        

                    struct Header *newHeader = (struct Header*)((char*)currentHeader + HEADER_SIZE + size_to_alloc);

                    newHeader->previousHeader = currentHeader;
                    newHeader->nextHeader = currentHeader->nextHeader;
                    newHeader->size = currentHeader->size - size_to_alloc - HEADER_SIZE;
                    newHeader->isFree = true;
                    newHeader->file = file;
                    newHeader->line = line;
                    newHeader->magicNumber = MAGIC_NUMBER;

                    currentHeader->nextHeader = newHeader;
                    currentHeader->size = size_to_alloc;
                }
                currentHeader->isFree = false;
                pthread_mutex_unlock(&mutex);

                stats.totalAllocatedBytes += size_to_alloc;
                currentAllocatedMemory += currentHeader->size;
                stats.averageAllocatedBytes = stats.totalAllocatedBytes / stats.allocCalls;
                stats.peakMemory = stats.peakMemory < currentAllocatedMemory ? currentAllocatedMemory : stats.peakMemory;

                return (void*)(currentHeader + 1);
                
            }

            previousHeader = currentHeader;
            currentHeader = currentHeader->nextHeader;
        }

        void* newBrk = sbrk(HEADER_SIZE + size_to_alloc);
        stats.sbrkCalls++;

        if (newBrk == (void*)-1)  //NOLINT
        {
            pthread_mutex_unlock(&mutex);

            (void)fprintf(stderr, "Error: sbrk failed\n");   
            abort();
        }

        struct Header *newHeader = (struct Header*)newBrk;

        newHeader->previousHeader = previousHeader;
        newHeader->nextHeader = NULL;
        newHeader->size = size_to_alloc;
        newHeader->isFree = false;
        newHeader->file = file;
        newHeader->line = line;
        newHeader->magicNumber = MAGIC_NUMBER;

        if (previousHeader != NULL)
        {
            previousHeader->nextHeader = newHeader;
        }

        pthread_mutex_unlock(&mutex);

        stats.totalAllocatedBytes += size_to_alloc;
        currentAllocatedMemory += newHeader->size;
        stats.peakMemory = stats.peakMemory < currentAllocatedMemory ? currentAllocatedMemory : stats.peakMemory;
        stats.averageAllocatedBytes = stats.totalAllocatedBytes / stats.allocCalls;

        return (void*)(newHeader + 1);
        
    }
    
}


void myfree(void* block){

    if(!isInitialized){

        (void)fprintf(stderr,"Error: Allocator not initialized\n");  
        return;
    }

    if (block == NULL)
    {
        return;
    }
    struct Header *header = (struct Header*)block - 1;

    if (header->magicNumber != MAGIC_NUMBER)
    {
        (void)(void)fprintf(stderr, "Error: Broken block\n File: %s\n Line: %d\n Size: %zu\n", header->file, header->line, header->size);
        abort();
    }

    if (header->isFree)
    {
        return;
    }
    pthread_mutex_lock(&mutex);

    header->isFree = true;
    currentAllocatedMemory -= header->size;
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
}

static void programExit(void){

    if(!isInitialized){
        return;
    }


    pthread_mutex_lock(&mutex);
    struct Header *currentHeader = firstHeader;
    
    while (currentHeader != NULL)
    {
        if (!currentHeader->isFree)
        {
            stats.notFreedBlocks++;
        }
        currentHeader = currentHeader->nextHeader;
    }
    pthread_mutex_unlock(&mutex);
    if(stats.outputEnabled){
        (void)(void)fprintf(stderr, "Total number of allocations: %ld\nTotal number of sbrk calls: %ld\nTotal number of not freed blocks: %ld\nTotal number of bytes allocated: %ld\nAverage number of bytes allocated: %ld\nPeak memory usage: %ld\n",
                stats.allocCalls,
                stats.sbrkCalls,
                stats.notFreedBlocks,
                stats.totalAllocatedBytes,
                stats.averageAllocatedBytes,
                stats.peakMemory);
    }
    
    
}

void dumpMemory(void) {
    struct Header *currentHeader = firstHeader;

    while (currentHeader != NULL)
    {
        if (currentHeader->magicNumber != MAGIC_NUMBER)
        {
            (void)fprintf(stderr, "Error: Broken block\n File: %s\n Line: %d\n Size: %zu\n", currentHeader->file, currentHeader->line, currentHeader->size);  
            abort();
        }
        if(currentHeader != firstHeader){
            (void)fprintf(stderr, "{BLOCK start %p end %p, size %zu, free %d, allocated %s, %d}\n",  
                (void*)currentHeader,
                (void*)((char*)currentHeader + currentHeader->size),
                currentHeader->size,
                currentHeader->isFree,
                currentHeader->file,
                currentHeader->line);
        }
        currentHeader = currentHeader->nextHeader;
    }
}

void getStats(struct Stats* stats_out) {
    *stats_out = stats;
}
