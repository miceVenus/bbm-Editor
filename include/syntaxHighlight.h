#ifndef SYNTAXHIGHLIGHT_H
#define SYNTAXHIGHLIGHT_H

struct editorRow;

void updateSyntaxHighlight(struct editorRow *row);
int highlight2Color(int hl);
void highlightMatch(unsigned char *pos, int matchLen);
int isSeperator(char c);
int isNumber(char c, int preCharHl, int preCharisSeperator);
#endif