#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/editorFind.h"
#include "../include/file.h"
#include "../include/editor.h"

void editorFind(){
    char *queryString = editorPrompt("Search : %s");
    if(queryString == NULL) return;
    
    int Buflen;
    char *targetString = NULL;
    for(int i = 0;i < E.numrows; i++){
        targetString = strstr(E.row[i].chars, queryString);
        if(targetString != NULL){
            E.ycursPosition = i;
            E.xcursPosition = targetString - E.row[i].chars;
            E.rowoff = E.numrows;
            break;
        }
    }
    free(queryString);
}