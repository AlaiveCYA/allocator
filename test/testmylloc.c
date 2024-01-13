#include <assert.h>
#include "../src/mylloc.h"
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
    
    void *block3 = mylloc(220);
    assert(block3 != NULL);

    myfree(block3);

    void *block4 = mylloc(2);
    assert(block4 != NULL);

    void *block5 = mylloc(20);
    assert(block5 != NULL);

    myfree(block2);
    myfree(block4);
    myfree(block5);
    myfree(block);

    void *block6 = mylloc(20);
    assert(block6 != NULL);


    myfree(NULL);

    struct Stats stats;
    getStats(&stats);

    dumpMemory();

    assert(stats.allocCalls == 6);
    assert(stats.totalAllocatedBytes == 302);
    assert(stats.peakMemory == 260);
    assert(stats.averageAllocatedBytes == 50);
    assert(stats.sbrkCalls == 3);

    printf("Test mylloc and myfree passed\n");
}

int main() {
    test_mylloc_and_myfree();
    return 0;
}