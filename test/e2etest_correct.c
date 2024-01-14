#include "../src/mylloc.h"

int main(void){

    int len = 10;

    char* string = mylloc(sizeof(char) * len);
    for(int i = 0; i < len; i++){
        string[i] = 'a';
    }
    printf("%s\n", string);
    myfree(string);

    return 0;
}

