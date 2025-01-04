#ifndef KILO_H
#define KILO_H
#include <unistd.h>
#include <termios.h>

struct editorConfig{
    struct termios originTermios;
    int WindowsRow;
    int WindowsCol;
    int xcursPosition;
    int ycursPosition;
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


#endif // KILO_H