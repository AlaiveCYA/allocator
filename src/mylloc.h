#ifndef MYLLOC_H
#define MYLLOC_H

#include <stdlib.h>
#include <stdbool.h>

#define mylloc(size_to_alloc) mylloc_full(size_to_alloc, __FILE__, __LINE__)

struct Stats
{
    size_t allocCalls;
    size_t totalAllocatedBytes;
    size_t peakMemory;
    size_t averageAllocatedBytes;
    size_t sbrkCalls;
    size_t notFreedBlocks;
    bool outputEnabled;
};

void* mylloc_full(size_t size_to_alloc, const char* file, int line);
void enableOutput(void);
void disableOutput(void);
void myfree(void *block);
void dumpMemory(void);
void getStats(struct Stats* stats);
int initializeAllocator(void);

#endif // MYLLOC_H