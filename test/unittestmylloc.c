#include <assert.h>
#include "../src/mylloc.h"
#include <stdio.h>



void test_mylloc_and_myfree(void){

    void *block = mylloc(20);
    assert(block != NULL);
    myfree(block);

}

void test_myfree_null(void){

    myfree(NULL);

}

void test_mylloc_same_block(void){

    void *block = mylloc(20);
    assert(block != NULL);

    myfree(block);

    void *block2 = mylloc(20);
    assert(block2 != NULL);

    assert(block == block2);

}

void test_mylloc_divide_block(void){

    void *block = mylloc(200);
    assert(block != NULL);

    myfree(block);

    void *block2 = mylloc(20);
    assert(block2 != NULL);

}

void test_myfree_merging(void){

    void* block = mylloc(20);
    assert(block != NULL);

    void* block2 = mylloc(20);
    assert(block2 != NULL);

    myfree(block2);
    myfree(block);
}

void test_getStats(void){

    struct Stats stats;
    getStats(&stats);

    assert(stats.allocCalls == 7);
    assert(stats.averageAllocatedBytes == 50);
    assert(stats.peakMemory == 288);
    assert(stats.totalAllocatedBytes == 352);
    assert(stats.sbrkCalls == 4);

}

int main() {

    int status = initializeAllocator();
    if(status == -1){
        fprintf(stderr,"Error initializing allocator\n"); //NOLINT
        abort();
    }
    test_mylloc_and_myfree();
    test_myfree_null();
    test_mylloc_same_block();
    test_myfree_merging();
    
    test_mylloc_divide_block();

    dumpMemory();

    test_getStats();

    return 0;
}


