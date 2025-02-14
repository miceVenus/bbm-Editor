#ifndef APPEND_BUFFER_H
#define APPEND_BUFFER_H

#include <stdlib.h>

typedef struct appendBuffer{
    char *buffer;
    int len;
}appendBuffer; 

void ABufferSetup(appendBuffer *ab);
void ABufferAppend(appendBuffer *ab, const char *s, int len);
void ABufferFree(appendBuffer *ab);

#endif