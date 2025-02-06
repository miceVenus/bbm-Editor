#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include<sys/ioctl.h>
#include<sys/types.h>

#include "../include/editor.h"
#include "../include/terminal.h"

void FlushTerminalAndSetCursorToLT(){
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
}

void Die(const char *s){

    FlushTerminalAndSetCursorToLT();
    perror(s);
    exit(1);
}

void CheckedTcSetAttr(int fd, int optionalAction, struct termios *t){
    if(tcsetattr(fd, optionalAction, t) == -1)
        Die("tcsetattr");
}

void CheckedTcGetAttr(int fd, struct termios *t){
    if(tcgetattr(fd, t) == -1)
        Die("tcgetattr");
}

void CheckedRead(int fd, void *buf, size_t count){
    if(read(fd, buf, count) == -1 && errno != EAGAIN)
        Die("read");
}

void DisabelRawMode(){
    CheckedTcSetAttr(STDIN_FILENO, TCSAFLUSH, &E.originTermios);
    printf("Disable Raw Mode\r\n");
}

void EnableRawMode(){
    
    CheckedTcGetAttr(STDIN_FILENO, &E.originTermios);
    atexit(DisabelRawMode);
    struct termios raw = E.originTermios;

    raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    raw.c_cflag |= (CS8);
    raw.c_oflag &= ~(OPOST);
    //set the minimum number of bytes of input needed before read() can return set 0 to return immediately non-block
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    CheckedTcSetAttr(STDIN_FILENO, TCSAFLUSH, &raw);
    
}

int GetWindowRowCol(int *row, int *col){
    struct winsize ws;
    if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0){
        if(write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12)return -1;
        return GetCursorPosition(row, col);
    }else{
        *row = ws.ws_row;
        *col = ws.ws_col;
        return 0;
    }
}

int GetCursorPosition(int *row, int *col){
    char buf[32];
    unsigned int i = 0;

    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;
    while (i < sizeof(buf) - 1) {
        if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
        if (buf[i] == 'R') break;
        i++;
    }

    buf[i] = '\0';

    if(buf[0] != '\x1b' || buf[1] != '[') return -1;
    if(sscanf(&buf[2], "%d;%d", row, col) !=2 ) return -1;

    return 0;
}
