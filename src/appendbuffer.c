#include<string.h>

#include "../include/appendbuffer.h"

void ABufferSetup(appendBuffer *ab){
    ab->buffer = NULL;
    ab->len = 0;
}

void ABufferAppend(struct appendBuffer *ab, const char *s, int len){
    char *newString = realloc(ab->buffer, ab->len + len);

    memcpy(&newString[ab->len], s, len);

    ab->buffer = newString;
    ab->len += len;
}

void ABufferFree(struct appendBuffer *ab){
    free(ab->buffer);
}
