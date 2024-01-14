#include "../src/mylloc.h"
#include <stdio.h>

int main(void){
    
    initializeAllocator();
    enableOutput();
    
    int len = 10;

    char* string = mylloc(sizeof(char) * len);
    for(int i = 0; i < len; i++){
        string[i] = 'a';
    }
    printf("%s\n", string);
    myfree(string+3);

    return 0;
}