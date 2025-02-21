#include "../include/syntaxHighlight.h"
#include "../include/editor.h"
int isKeywordPos(editorRow *row, int pos, int len);

void updateSyntaxHighlight(editorRow *row){
    row->hl = realloc(row->hl, row->rsize);
    memset(row->hl, HL_NORMAL, row->rsize);
    if(E.syntax == NULL) return;
    char **keywords = E.syntax->fileKeywords;
    char *scs = E.syntax->singleLineCommentStart;
    char *mcs = E.syntax->multiLineCommentStart;
    char *mce = E.syntax->multiLineCommentEnd;

    int mcs_len = mcs ? strlen(mcs) : 0;
    int mce_len = mce ? strlen(mce) : 0;

    int i = 0;
    int inComment = row->index > 0 && E.row[row->index - 1].isInComment;
    while(i < row->rsize){
        // char postChar = (i < row->rsize - 1)? row->render[i+1] : '\0';
        unsigned char preCharHl = (i > 0)? row->hl[i - 1] : HL_NORMAL;
        char curChar = row->render[i];
        if(E.syntax->flags & HL_HIGHLIGHT_COMMENT){
            char *slCommentStartNeedle = strstr(row->render, scs);
            if(slCommentStartNeedle != NULL){
                int slCommentStartPos = slCommentStartNeedle - row->render;
                memset(row->hl + slCommentStartPos, HL_COMMENT, row->rsize - slCommentStartPos);
                i += row->rsize - slCommentStartPos;
                continue;
            }

            if(inComment) {
                row->hl[i] = HL_COMMENT;
                if (!strncmp(&row->render[i], mce, mce_len)) {
                    memset(&row->hl[i], HL_COMMENT, mce_len);
                    i += mce_len;
                    inComment = 0;
                    continue;
                }else {
                    i++;
                    continue;
                }
            }else if(!strncmp(&row->render[i], mcs, mcs_len)) {
                memset(&row->hl[i], HL_COMMENT, mcs_len);
                i += mcs_len;
                inComment = 1;
                continue;
            }
        }
        if(E.syntax->flags & HL_HIGHLIGHT_KEYWORD){
            int j = 0;
            while(keywords[j] != NULL){
                int keyword2 = 0;
                char *keyword = keywords[j];
                int keywordLen = strlen(keyword);
                if(keyword[keywordLen - 1] == '|'){
                    keywordLen--;
                    keyword2 = 1;
                }
                if(!strncmp(&row->render[i], keyword, keywordLen) && isKeywordPos(row, i, keywordLen)){
                    memset(row->hl + i, keyword2? HL_KEYWORD2 : HL_KEYWORD1, keywordLen);
                    i += keywordLen;
                    break;
                }
                j++;
            }
            if(keywords[j] != NULL) continue;
        }
        if(E.syntax->flags & HL_HIGHLIGHT_NUMBERS){
            int preCharisSeperator = 1;
            if(isNumber(curChar, preCharHl, preCharisSeperator)){
                row->hl[i] = HL_NUMBER;
                i++;
                preCharisSeperator = 0;
                continue;
            }
            preCharisSeperator = isSeperator(curChar);
        }
        if(E.syntax->flags & HL_HIGHLIGHT_STRINGS){
            if(curChar == '\'' || curChar == '"'){
                row->hl[i] = HL_STRING;
                int j = i+1;
                while(j<row->rsize && row->render[j] != curChar){
                    row->hl[j] = HL_STRING;
                    j++;
                }
                if(j < row->rsize) row->hl[j] = HL_STRING;
                i = j+1;
                continue;
            }
        }
        if(E.syntax->flags & HL_HIGHLIGHT_WORDS){
            if(isSeperator(curChar)){
                i++;
                continue;
            }
            if(isspace(curChar)){
                i++;
                continue;
            }

            if(isprint(curChar)){
                row->hl[i] = HL_WORD;
                i++;
                continue;
            }
        }
        i++;
    }
    int changed = (row->isInComment != inComment);
    row->isInComment = inComment;
    if (changed && row->index + 1 < E.numrows)
      updateSyntaxHighlight(&E.row[row->index + 1]);
}

int highlight2Color(int hl){

    switch (hl)
    {
    case HL_NUMBER:
        return 31;
    
    case HL_NORMAL:
        return 39;

    case HL_MATCH:
        return 37;
    
    case HL_WORD:
        return 96;

    case HL_STRING:
        return 33;
    
    case HL_COMMENT:
        return 32;
    
    case HL_KEYWORD1:
        return 94;
    
    case HL_KEYWORD2:
        return 95;

    default:
        return 39;
    }

}
void highlightMatch(unsigned char *pos, int matchLen){
    memset(pos, HL_MATCH, matchLen);
}
int isKeywordPos(editorRow *row, int pos, int len){
    if(pos + len >= row->rsize) return 0;
    return (pos == 0? 1 : isSeperator(row->render[pos - 1])) && isSeperator(row->render[pos + len]);
}
int isSeperator(char c){
    return isspace(c) || c == '\0' || strchr(",.()+-/*=~%<>[];", c) != NULL;
}
int isNumber(char c, int preCharHl, int preCharisSeperator){
    return (isdigit(c) && (preCharisSeperator || preCharHl == HL_NUMBER)) ||(c == '.' && preCharHl == HL_NUMBER);
}