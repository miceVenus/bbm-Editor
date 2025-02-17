#ifndef FILE_H
#define FILE_H

void fileSave();
void fileOpen(char *filename);
char *Erow2String(int *buflen);

void fileSave();
void fileOpen(char *filename);
void InitEditorFileName(char *filename);
void InitEditorSyntax();
#endif