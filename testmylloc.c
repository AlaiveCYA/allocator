#include <assert.h>
#include "mylloc.h"
#include <stdio.h>

void test_mylloc_and_myfree() {

    int status = initializeAllocator();
    if(status == -1){
        fprintf(stderr,"Error initializing allocator\n");
        exit(EXIT_FAILURE);
    }

    void *block = mylloc(20);
    assert(block != NULL);

    void *block2 = mylloc(20);
    assert(block2 != NULL);
    
    void *block3 = mylloc(20);
    assert(block3 != NULL);
    myfree(block3);

    void *block4 = mylloc(20);
    assert(block4 != NULL);

    void *block5 = mylloc(20);
    assert(block5 != NULL);
    
    myfree(block2);
    myfree(block4);
    myfree(block5);
    myfree(block);

    struct Stats stats;
    getStats(&stats);

    dumpMemory();

    assert(stats.allocCalls == 5);
    assert(stats.totalAllocatedBytes == 100);
    assert(stats.peakMemory == 100);
    assert(stats.averageAllocatedBytes == 20);
    assert(stats.sbrkCalls == 4);
    assert(stats.notFreedBlocks == 0);
}

int main() {
    test_mylloc_and_myfree();
    return 0;
}