#include "../include/syntaxHighlight.h"
#include "../include/editor.h"

void updateSyntaxHighlight(editorRow *row){
    row->hl = realloc(row->hl, row->rsize);
    memset(row->hl, HL_NORMAL, row->rsize);

    int preCharisSeperator = 1;
    int i = 0;
    while(i < row->rsize){
        unsigned char preCharHl = (i > 0)? row->hl[i - 1] : HL_NORMAL;
        char c = row->render[i];
        if(isNumber(c, preCharHl, preCharisSeperator)){
            row->hl[i] = HL_NUMBER;
            preCharisSeperator = 0;
            i++;
            continue;
        }
        i++;
        preCharisSeperator = isSeperator(c);
    }
}

int highlight2Color(int hl){

    switch (hl)
    {
    case HL_NUMBER:
        return 31;
    
    case HL_NORMAL:
        return 39;

    case HL_MATCH:
        return 34;

    default:
        return 37;
    }

}
void highlightMatch(unsigned char *pos, int matchLen){
    memset(pos, HL_MATCH, matchLen);
}

int isSeperator(char c){
    return isspace(c) || c == '\0' || strchr(",.()+-/*=~%<>[];", c) != NULL;
}
int isNumber(char c, int preCharHl, int preCharisSeperator){
    return (isdigit(c) && (preCharisSeperator || preCharHl == HL_NUMBER)) ||(c == '.' && preCharHl == HL_NUMBER);
}