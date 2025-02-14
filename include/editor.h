#ifndef EDITOR_H
#define EDITOR_H


#include <termios.h>
#include <time.h>
#include<ctype.h>
#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdarg.h>

#define BBM_EDITOR_VERSION "0.0.1"
#define EDITOR_TAB_STOP 4
#define EIDTOR_CTRLQ_TIME 2

#define CTRL_KEY(k) ((k) & 0x1f)

#define HL_HIGHLIGHT_NUMBERS (1<<0)

enum editorKey{
    BACKSPACE = 127,
    ARROW_LEFT = 777,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    PAGE_UP,
    PAGE_DOWN,
    HOME,
    END,
    DELETE
};

enum editorHighLight{
    HL_NORMAL = 0,
    HL_NUMBER,
    HL_MATCH,
};

typedef struct editorRow{
    int size;
    char *chars;
    char *render;
    int rsize;
    unsigned char *hl;
}editorRow;

typedef struct editorSyntax{
    char **fileMatch;
    char *fileExtension;
    int flags;
}editorSyntax;

char *C_HL_extensions[] = {".c", ".cpp", ".h", NULL};
editorSyntax HLDB[] = {
    {   "c",
        C_HL_extensions,
        HL_HIGHLIGHT_NUMBERS
    }
};
#define HLDB_ENTRIES (sizeof(HLDB) / sizeof(HLDB[0]))

struct editorConfig{
    struct termios originTermios;
    struct editorSyntax *syntax;
    int WindowsRow;
    int WindowsCol;
    int xcursPosition;
    int ycursPosition;
    int rxcursPosition;
    int numrows;
    int coloff;
    int rowoff;
    int dirty;
    editorRow *row;
    char *filename;
    char statusmsg[80];
    time_t statusmsg_time;
    char *fileExtension;
};


struct appendBuffer;
struct editorConfig E; //尚未被赋值的全局变量会被初始化为0

void EditorInsertChar(int key);
void RowInsertChar(editorRow *row, int pos, int c);
void editorAppendRow(char *s, size_t len);
void eidtorUpdateRow(editorRow *row);
void InitEditor();
void RefreshScreen();
void editorSetStatusMessage(const char *fmt, ...);
void editorScroll();
void DrawRows(struct appendBuffer *ab);
void DrawStatusBar(struct appendBuffer *ab);
void DrawStatusMessageBar(struct appendBuffer *ab);
void PrintWelcome(struct appendBuffer *ab);
void EditorDelChar();
void RowDelChar(editorRow *row, int pos);
int xcurs2Rxcurs(editorRow *row, int xcurs);
int Rxcurs2xcurs(editorRow *row, int rxcurs);
void editorInsertRow(int pos, char *s, size_t len);
char *editorPrompt(char *prompt, void (*callback) (const char *, int));

#endif