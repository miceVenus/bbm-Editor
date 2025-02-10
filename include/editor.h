#ifndef EDITOR_H
#define EDITOR_H
#endif

#include <termios.h>
#include <time.h>
#include "appendbuffer.h"

#define BBM_EDITOR_VERSION "0.0.1"
#define EDITOR_TAB_STOP 4
#define EIDTOR_CTRLQ_TIME 2

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

typedef struct editorRow{
    int size;
    char *chars;
    char *render;
    int rsize;
}editorRow;

struct editorConfig{
    struct termios originTermios;
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
};

struct editorConfig E; //尚未被赋值的全局变量会被初始化为0


void EditorInsertChar(int key);
void RowInsertChar(editorRow *row, int pos, int c);
void editorAppendRow(char *s, size_t len);
void eidtorUpdateRow(editorRow *row);
void InitEditor();
void RefreshScreen();
void editorSetStatusMessage(const char *fmt, ...);
void editorScroll();
void DrawRows(appendBuffer *ab);
void DrawStatusBar(appendBuffer *ab);
void DrawStatusMessageBar(appendBuffer *ab);
void PrintWelcome(appendBuffer *ab);
void EditorDelChar();
void RowDelChar(editorRow *row, int pos);
int xcurs2Rxcurs(editorRow *row, int xcurs);
void editorInsertRow(int pos, char *s, size_t len);
