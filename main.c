#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>




struct Header
{
    struct Header *previousHeader;
    struct Header *nextHeader;
    int size;
    bool isFree;
};

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

#define HEADER_SIZE sizeof(struct Header)

static bool isInitialized = false;

static struct Header *firstHeader = NULL;





static int initializeAllocator(){

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






void* my_alloc(size_t size_to_alloc){

    if (!isInitialized)
    {
        if (initializeAllocator() == -1)
        {
            return NULL;
        }
    }
    {
        struct Header *currentHeader = firstHeader;
        pthread_mutex_lock(&mutex);
        while (currentHeader != NULL)
        {
            if (currentHeader->isFree && currentHeader->size >= size_to_alloc)
            {
                currentHeader->isFree = false;
                void* currentBrk = sbrk(HEADER_SIZE);
                if (currentBrk == (void*)-1)
                {
                    return NULL;
                }
            }
            currentHeader = currentHeader->nextHeader;
        }
    }
    
}

