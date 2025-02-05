#ifndef KILO_H
#define KILO_H
#define KILO_TAB_STOP 4
#define CTRL_KEY(k) ((k) & 0x1f)

#define ABUF_INIT {NULL, 0}
#define KILO_VERSION "0.0.1"
#include <unistd.h>
#include <termios.h>
#include <time.h>
#include <stdarg.h>
#include <fcntl.h>

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
    editorRow *row;
    char *filename;
    char statusmsg[80];
    time_t statusmsg_time;
};

struct appendBuffer{
    char *buffer;
    int len;
}; 

// Function prototypes
void Die(const char *s);
void CheckedTcSetAttr(int fd, int optionalAction, struct termios *t);
void CheckedTcGetAttr(int fd, struct termios *t);
void CheckedRead(int fd, void *buf, size_t count);
void DisabelRawMode();
void EnableRawMode();
int GetWindowRowCol(int *row, int *col);
int GetCursorPosition(int *row, int *col);
int ReadKeypress();
void ProcessKeypress();
void DrawRows();
void RefreshScreen();
void FlushTerminalAndSetCursorToLT();
void ABufferAppend(struct appendBuffer *ab, const char *s, int len);
void ABufferFree(struct appendBuffer *ab);
void InitEditor();
void MoveCursor(int c);
void eidtorUpdateRow(editorRow *row);
int xcursToRxcurs(editorRow *row, int xcurs);
void RowInsertChar(editorRow *row, int at, int c);
void EditorInsertChar(int c);  
void editorAppendNewLine(char *s, size_t len);
void Save2Disk();
char *Erow2String(int *buflen);

#endif // KILO_H