#ifndef EDITOR_FIND_H
#define EDITOR_FIND_H

#define BACKWARD_SEARCH -1
#define FORWARD_SEARCH 1
#define NO_LAST_MATCH -1

void editorFind();
void editorFindCallback(const char *queryString, int key);

#endif