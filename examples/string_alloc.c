#include <stdio.h>
#include <mylloc.h>

int main(void){
    char *string = (char *)mylloc(sizeof(char*));
    *string = "Ala ma kota";
    printf("%s\n", *string);
    myfree(string);
    return 0;
}