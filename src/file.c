#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


#include "../include/file.h"
#include "../include/editor.h"
#include "../include/terminal.h"

char *Erow2String(int *buflen){
    int len = 0;
    for(int i = 0; i < E.numrows; i++){
        len += E.row[i].size + 1;
    }
    *buflen = len;
    char *buf = (char*)malloc(sizeof(char) * len);
    char *p = buf;
    for(int i = 0; i < E.numrows; i++){
        memcpy(p, E.row[i].chars, E.row[i].size);
        p += E.row[i].size;
        *p = '\n';
        p++;
    }
    return buf;
}
void Save2Disk(){
    int len;
    char *buf = Erow2String(&len);
    char *filename = E.filename;
    if(filename == NULL){
        filename = "temp.txt";
    }

    FILE *fp = fopen(filename, "w");
    if(!fp) Die("fopen");
    fwrite(buf, sizeof(char), len, fp);
    fclose(fp);
    free(buf);
}

void editorOpen(char *filename){
    free(E.filename);
    E.filename = strdup(filename);
    FILE *fp = fopen(filename, "r");
    if(!fp) Die("fopen");

    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen = getline(&line, &linecap, fp);

    while(linelen != -1){
        while(linelen > 0 && (line[linelen - 1] == '\n' || line[linelen - 1] == '\r')){
            linelen--;
        }
        editorAppendNewLine(line, linelen);
        linelen = getline(&line, &linecap, fp);
    }
    free(line);
    fclose(fp);
}