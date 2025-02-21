#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/editorFind.h"
#include "../include/file.h"
#include "../include/editor.h"
#include "../include/syntaxHighlight.h"

void editorFind(){
    int savedXcurs = E.xcursPosition;
    int savedYcurs = E.ycursPosition;
    int savedRowoff = E.rowoff;
    int savedColoff = E.coloff;

    char *queryString = editorPrompt("Search : %s (USE ARROW|ENTER|ESC|CTRL-Q)", editorFindCallback, NULL);
    if(queryString) free(queryString);
    else{
        E.xcursPosition = savedXcurs;
        E.ycursPosition = savedYcurs;
        E.rowoff = savedRowoff;
        E.coloff = savedColoff;
    }
}
void editorFindCallback(const char *queryString, int key){
    static int lastMatch = NO_LAST_MATCH;
    static int direction = FORWARD_SEARCH;
    static int savedHighlightLine;
    static unsigned char *savedHighlight = NULL;
    if(savedHighlight){
        editorRow *row = &E.row[savedHighlightLine];
        memcpy(row->hl, savedHighlight, row->rsize);
        free(savedHighlight);
        savedHighlight = NULL;
    }
    if(key == '\r' || key == '\x1b'){
        lastMatch = NO_LAST_MATCH;
        direction = FORWARD_SEARCH;
        return;
    }else if(key == ARROW_RIGHT || key == ARROW_DOWN){
        direction = FORWARD_SEARCH;
    }else if(key == ARROW_LEFT || key == ARROW_UP){
        direction = BACKWARD_SEARCH;
    }else{
        lastMatch = NO_LAST_MATCH;
        direction = FORWARD_SEARCH;
    }
    if(lastMatch == NO_LAST_MATCH) direction = FORWARD_SEARCH;

    int current = lastMatch;
    char *targetString = NULL;


    if(lastMatch == NO_LAST_MATCH) current = 0;
    else if(direction == BACKWARD_SEARCH) current = lastMatch - 1;
    else if(direction == FORWARD_SEARCH) current = lastMatch + 1;

    for(int i = 0;i < E.numrows; i++){
        if(current == -1) current = E.numrows - 1;
        else if(current == E.numrows) current = 0;

        targetString = strstr(E.row[current].render, queryString);
        if(targetString != NULL){
            editorRow *row = &E.row[current];
            lastMatch = current;
            E.ycursPosition = current;
            E.xcursPosition = Rxcurs2xcurs(row , targetString - row->render);
            E.rowoff = E.numrows;
            savedHighlightLine = current;
            savedHighlight = (unsigned char*)malloc(row->rsize);
            memcpy(savedHighlight, row->hl, row->rsize);
            highlightMatch(E.row[current].hl + E.rxcursPosition, strlen(queryString));
            break;
        }

        if(direction == FORWARD_SEARCH) current++;
        else current--;
        
    }
    return;
}
