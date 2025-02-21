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
#include "../include/syntaxHighlight.h"

void InitFileNameAndSyntax(char *fileName);
char *editorPromptForSave();

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
    char *filename = editorPromptForSave();
    if(filename == NULL || filename[0] == '\0'){
        editorSetStatusMessage("Save aborted");
        free(buf);
        E.dirty = 0;
        return;
    }
    FILE *fp = fopen(filename, "w");
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
        InitFileNameAndSyntax(filename);
        fclose(fp);
        free(buf);
        free(filename);
        E.dirty = 0;
    }
}

void fileOpen(char *filename){
    free(E.filename);
    free(E.fileExtension);
    InitFileNameAndSyntax(filename);
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

void InitFileNameAndSyntax(char *fileName){
    InitEditorFileName(fileName);
    InitEditorSyntax();
}

void InitEditorFileName(char *fileName){
    if(E.filename){
        free(E.filename);
        E.filename = NULL;
    }
    if(E.fileExtension){
        free(E.fileExtension);
        E.fileExtension = NULL;
    }
    char *dot = strrchr(fileName, '.');
    
    if(!dot || dot == fileName) dot = NULL;
    E.fileExtension  = strdup(dot == NULL ? "" : dot);

    if(dot == NULL) E.filename = strdup(fileName);
    else{
        size_t fileNameLen = dot - fileName;
        E.filename = (char*)malloc(fileNameLen + 1);
        memmove(E.filename, fileName, fileNameLen);
        E.filename[fileNameLen] = '\0';
    }
}

void InitEditorSyntax(){
    if(E.fileExtension == NULL || !strcmp(E.fileExtension, "")) return;
    for(int i = 0; i < getHLDBEntries(); i++){
        editorSyntax *entry = &HLDB[i];
        int j = 0;
        while(entry->fileMatch[j]){
            if(strcmp(entry->fileMatch[j], E.fileExtension) == 0){
                E.syntax = entry;
                for(int k = 0; k < E.numrows; k++){
                    updateSyntaxHighlight(&E.row[k]);
                }
                return;
            }
            j++;
        }
    }
}
char *editorPromptForSave(){
    if(E.filename == NULL){
        return editorPrompt("Save as: %s (CTRL-Q to quit)", NULL, NULL);
    }
    if(E.fileExtension == NULL){
        return editorPrompt("Save as: %s (CTRL-Q to quit)", NULL, E.filename);
    }
    return editorPrompt("Save as: %s (CTRL-Q to quit)", NULL, strcat(E.filename, E.fileExtension));
}