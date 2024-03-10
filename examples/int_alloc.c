#include <stdio.h>
#include <mylloc.h>

int main(void){
    int *p = (int *)mylloc(sizeof(int));
    *p = 5;
    printf("%d\n", *p);
    myfree(p);
    return 0;
}