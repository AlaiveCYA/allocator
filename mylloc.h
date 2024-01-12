#ifndef MYLLOC_H
#define MYLLOC_H

#include <stdlib.h>


void *mylloc(size_t size);
void myfree(void *ptr);

#endif // MYLLOC_H