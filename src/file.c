#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
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
void fileSave(){
    int len;
    char *buf = Erow2String(&len);
    char promptBuf[80];
    snprintf(promptBuf, sizeof(promptBuf), "Saved as: %s", E.filename == NULL ? "" : E.filename);
    char *filename = editorPrompt(strcat(promptBuf, "%s (CTRL-Q to quit)"));
    if(filename == NULL){
        editorSetStatusMessage("Save aborted");
        free(buf);
        E.dirty = 0;
        return;
    }
    FILE *fp = fopen(strcat(filename, E.fileExtension), "w");
    if(!fp) {
        editorSetStatusMessage("File error : %s when saving",strerror(errno));
        return;
    }else{
        int writeLen = fwrite(buf, sizeof(char), len, fp);
        if(writeLen != len){
            editorSetStatusMessage("File error : %s when writing",strerror(errno));
            fclose(fp);
            return;
        }
        editorSetStatusMessage("%d Bytes has been saved",len);
        fclose(fp);
        free(buf);
        free(filename);
        E.dirty = 0;
    }
}

void fileOpen(char *filename){
    free(E.filename);
    free(E.fileExtension);
    InitEditorFileName(filename);
    FILE *fp = fopen(filename, "r");
    if(!fp) {
        Die("fopen");
    }else{
        E.dirty = 0;
        char *line = NULL;
        size_t linecap = 0;
        ssize_t linelen = getline(&line, &linecap, fp);

        while(linelen != -1){
            while(linelen > 0 && (line[linelen - 1] == '\n' || line[linelen - 1] == '\r')){
                linelen--;
            }
            editorAppendRow(line, linelen);
            linelen = getline(&line, &linecap, fp);
        }
        free(line);
        fclose(fp);
        E.dirty = 0;
    }
}

void InitEditorFileName(char *fileName){
    char *dot = strrchr(fileName, '.');
    if(!dot || dot == fileName) dot = NULL;
    E.fileExtension  = strdup(dot == NULL ? ".txt" : dot);
    size_t fileNameLen = dot - fileName;
    E.filename = (char*)malloc(fileNameLen + 1);
    memmove(E.filename, fileName, fileNameLen);
    E.filename[fileNameLen] = '\0';
}