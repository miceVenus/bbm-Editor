#include "../include/syntaxHighlight.h"
#include "../include/editor.h"

void updateSyntaxHighlight(editorRow *row){
    row->hl = realloc(row->hl, row->rsize);
    memset(row->hl, HL_NORMAL, row->rsize);
    if(E.syntax == NULL) return;

    int preCharisSeperator = 1;
    int i = 0;
    while(i < row->rsize){
        unsigned char preCharHl = (i > 0)? row->hl[i - 1] : HL_NORMAL;
        char c = row->render[i];
        if(E.syntax->flags & HL_HIGHLIGHT_NUMBERS){
            if(isNumber(c, preCharHl, preCharisSeperator)){
                row->hl[i] = HL_NUMBER;
                i++;
                preCharisSeperator = 0;
                continue;
            }
            preCharisSeperator = isSeperator(c);
        }
        if(E.syntax->flags & HL_HIGHLIGHT_STRINGS){
            if(c == '"'){
                row->hl[i] = HL_STRING;
                int j = i+1;
                while(j<row->rsize && row->render[j] != '"'){
                    row->hl[j] = HL_STRING;
                    j++;
                }
                if(j < row->rsize) row->hl[j] = HL_STRING;
                i = j+1;
                continue;
            }
        }
        if(E.syntax->flags & HL_HIGHLIGHT_WORDS){
            if(isSeperator(c)){
                i++;
                continue;
            }
            if(isspace(c)){
                i++;
                continue;
            }

            if(isprint(c)){
                row->hl[i] = HL_WORD;
                i++;
                continue;
            }
        }
        i++;
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
    
    case HL_WORD:
        return 36;

    case HL_STRING:
        return 33;

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