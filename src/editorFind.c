#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/editorFind.h"
#include "../include/file.h"
#include "../include/editor.h"

void editorFind(){
    int savedXcurs = E.xcursPosition;
    int savedYcurs = E.ycursPosition;
    int savedRowoff = E.rowoff;
    int savedColoff = E.coloff;

    char *queryString = editorPrompt("Search : %s (USE ARROW|ENTER|ESC|CTRL-Q)", editorFindCallback);
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
            lastMatch = current;
            E.ycursPosition = current;
            E.xcursPosition = Rxcurs2xcurs(&E.row[current], targetString - E.row[current].render);
            E.rowoff = E.numrows;
            break;
        }

        if(direction == FORWARD_SEARCH) current++;
        else current--;
        
    }
    return;
}