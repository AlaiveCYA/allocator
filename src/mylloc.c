#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "mylloc.h"
#include <stdint.h>


/**
 * @file mylloc.c
 *
 * @brief Implementation of the allocator
 * 
 * @author Szymon Rzewuski
 * 
 * @details
 * My implementation of the allocator.
 * The allocator uses First Fit algorithm and header|block structure.
 * Allocation is done using sbrk.
 * All functions are thread safe.
 * The implementation has following functions:
 * initializeAllocator
 * enableOutput
 * disableOutput
 * mylloc
 * myfree
 * dumpMemory
 * getStats
 * 
 *  
*/



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
#define MIN_ALLOC_SIZE 64UL
#define MAGIC_NUMBER 0x272341
#define CONST_8 8UL

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static bool isInitialized = false;
static struct Header *firstHeader = NULL;
static size_t currentAllocatedMemory = 0;

static void programExit(void);

/**
 * @brief Initializes the allocator
 * 
 * @return int 0 on success
 * 
 * @details
 * Function to initialize the allocator.
 * The function allocates the first header of memory using sbrk
 * and then registers the programExit function to be called at the end of the program.
 * This function should be called once at the beginning of the program.
 * If gcc or clang is used, this function is automatically called before main.
 * If another compiler is used, this function must be called manually.
 * If the allocator is already initialized, the function does nothing
 * and if the allocator cannot be initialized, the function aborts the program with an error message.
 * 
 */
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

/**
 * @brief Enables output at the end of the program
 * 
 * @details
 * Function to print collected statistics at the end of the program to stderr.
*/
void enableOutput(void){
    if(!isInitialized){
        (void)fprintf(stderr,"Error: Allocator not initialized\n");  
        return;
    }
    stats.outputEnabled = true;
}

/**
 * @brief Disables output at the end of the program
 * 
 * @details
 * Function to disable printing collected statistics at the end of the program.
*/
void disableOutput(void){
    if(!isInitialized){
        (void)fprintf(stderr,"Error: Allocator not initialized\n");  
        return;
    }
    stats.outputEnabled = false;
}


/**
 * @brief Allocates memory
 * 
 * @param size_to_alloc The size of the memory to allocate
 * @param file The file where the allocation is made
 * @param line The line where the allocation is made
 * 
 * @return void* Pointer to the allocated memory
 * 
 * @details
 * Funtion to allocate memory of size size_to_alloc using sbrk, First Fit algorithm and header|block structure.
 * Function returns a pointer to the allocated memory if successful, otherwise
 * it aborts the program with an error message.
 * Memory allocated is rounded up to the nearest multiple of 8.
 * If the block of memory is large enough, it is split into two blocks when memory is allocated.
 * The first block is returned to the user and the second block is added to the free list.
 * Whole function works in a thread safe manner.
 * If the allocator is not initialized, the function aborts the program with an error message.
 * If the block is broken, the function aborts the program with an error message.
*/
void* mylloc_full(size_t size_to_alloc, const char* file, int line){

    size_t temp = size_to_alloc/CONST_8;
    size_to_alloc = (temp + 1) * CONST_8;

    if (!isInitialized)
    {
        (void)fprintf(stderr, "Error: Allocator not initialized\n");
        abort();
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

/**
 * @brief Frees memory
 * 
 * @param block Pointer to the memory to free
 * 
 * @details
 * Function to free memory allocated by mylloc_full.
 * If the freed block of memory is adjacent to another free block, the two blocks are merged into one.
 * Whole function works in a thread safe manner.
 * If the block is already free, the function does nothing.
 * If the block is NULL, the function does nothing.
 * If the block is broken, the function aborts the program with an error message.
 * If the allocator is not initialized, the function aborts the program with an error message.
 * 
*/
void myfree(void* block){

    if(!isInitialized){

        (void)fprintf(stderr,"Error: Allocator not initialized\n");  
        abort();
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

/**
 * @brief Prints statistics at the end of the program
 * 
 * @details
 * Function to print collected statistics at the end of the program to stderr.
 * If the allocator is not initialized, the function aborts the program with an error message.
 * If the block is broken, the function aborts the program with an error message.
 * Funtion prints the following statistics:
 * Total number of allocations
 * Total number of sbrk calls
 * Total number of not freed blocks
 * Total number of bytes allocated
 * Average number of bytes allocated
 * Peak memory usage in bytes
*/
static void programExit(void){

    if(!isInitialized){
        (void)fprintf(stderr,"Error: Allocator not initialized\n");  
        abort();
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

/**
 * @brief Dumps whole memory
 * 
 * @details
 * Function to dump whole memory to stderr.
 * If the allocator is not initialized, the function aborts the program with an error message.
 * If the block is broken, the function aborts the program with an error message.
 * Function prints the following information about each block:
 * Start address
 * End address
 * Size
 * If the block is free
 * File where the block was allocated
 * Line where the block was allocated
 * 
*/
void dumpMemory(void) {

    if(!isInitialized){
        (void)fprintf(stderr,"Error: Allocator not initialized\n");  
        abort();
    }

    struct Header *currentHeader = firstHeader;

    pthread_mutex_lock(&mutex);
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
    pthread_mutex_unlock(&mutex);
}

/**
 * @brief Gets statistics
 * 
 * @param stats_out Pointer to the struct where the statistics are stored
 * 
 * @details
 * Function to get statistics about the allocator.
 * Function stores the statistics in the struct pointed to by stats_out.
 * Stats struct contains the following statistics:
 * Total number of allocations
 * Total number of sbrk calls
 * Total number of not freed blocks
 * Total number of bytes allocated
 * Average number of bytes allocated
 * Peak memory usage in bytes
 * If the allocator is not initialized, the function aborts the program with an error message.
*/
void getStats(struct Stats* stats_out) {
    if(!isInitialized){
        (void)fprintf(stderr,"Error: Allocator not initialized\n");  
        abort();
    }
    *stats_out = stats;
}
