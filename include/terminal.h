#ifndef TERMINAL_H
#define TERMINAL_H

#include <termios.h>
#include <unistd.h>

void FlushTerminalAndSetCursorToLT();
void Die(const char *s);
void CheckedTcSetAttr(int fd, int optionalAction, struct termios *t);
void CheckedTcGetAttr(int fd, struct termios *t);
void CheckedRead(int fd, void *buf, size_t count);
void DisabelRawMode();
void EnableRawMode();
int GetWindowRowCol(int *row, int *col);
int GetCursorPosition(int *row, int *col);

#endif