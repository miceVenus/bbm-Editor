/*** includes ***/

#include<unistd.h>
#include<errno.h>
#include<stdlib.h>
#include<stdio.h>
#include<sys/ioctl.h>
#include<ctype.h>
#include "bbmEditor.h"
#include<string.h>

#define CTRL_KEY(k) ((k) & 0x1f)

#define ABUF_INIT {NULL, 0}
#define KILO_VERSION "0.0.1"
/*** includes ***/

/*** data ***/

enum editorKey{
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

struct editorConfig E; //尚未被赋值的全局变量会被初始化为0

/*** data ***/

/*** terminal ***/
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
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
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
/*** terminal ***/

/*** input ***/

/* 有一点bug 快速按esc + [ + ctrl + c 会导致cursor移动*/

int ReadKeypress(){

    char key = '\0';
    CheckedRead(STDIN_FILENO, &key, 1);
    if(key == '\x1b'){
        char seq[3];
        if(read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
        if(read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';

        if(seq[1] <= '9' && seq[1] >= '0'){
            if(read(STDIN_FILENO, &seq[2], 1) != 1) return '\x1b';
            if(seq[2] == '~'){
                switch(seq[1]){
                    case '5': return PAGE_UP;
                    case '6': return PAGE_DOWN;
                    case '3': return DELETE;
                }
            }
        }
        // if(JudgePageUp(seq)) return PAGE_UP;
        // if(JudgePageDown(seq)) return PAGE_DOWN;

        if(seq[0] == '[')
            switch(seq[1]){
                case 'A': return ARROW_UP;
                case 'B': return ARROW_DOWN;
                case 'C': return ARROW_RIGHT;
                case 'D': return ARROW_LEFT;
                case 'H': return HOME;
                case 'F': return END;
            }
        return key;
    }else{
        return key;
    }
}

void ProcessKeypress(){
    int key = ReadKeypress();
    switch(key){
        case CTRL_KEY('q'):
            FlushTerminalAndSetCursorToLT();
            exit(0);
            break;
        case ARROW_UP:
        case ARROW_DOWN:
        case ARROW_LEFT:
        case ARROW_RIGHT:
            MoveCursor(key);
            break;
        case PAGE_UP:
        case PAGE_DOWN:{
            int times = E.WindowsRow;
            while(times--)
                MoveCursor(key == PAGE_UP ? ARROW_UP : ARROW_DOWN);
        }
            break;
        
        case HOME:{
            E.xcursPosition = 0;
            break;
        }
        case END:{
            E.xcursPosition = E.WindowsCol - 1;
            break;
        }
        case DELETE:{
            MoveCursor(DELETE);
            break;
        }
    }
}

void MoveCursor(int key){
  switch (key) {
    case ARROW_LEFT:
      if (E.xcursPosition != 0) {
        E.xcursPosition--;
      }
      break;
    case ARROW_RIGHT:
      if (E.xcursPosition != E.WindowsCol - 1) {
        E.xcursPosition++;
      }
      break;
    case ARROW_UP:
      if (E.ycursPosition != 0) {
        E.ycursPosition--;
      }
      break;
    case ARROW_DOWN:
      if (E.ycursPosition != E.WindowsRow - 1) {
        E.ycursPosition++;
      }
      break;
    case DELETE:
      if (E.xcursPosition != 0) {
        E.xcursPosition--;
      }
      break;
  }
}
/*** output ***/

void DrawRows(struct appendBuffer *ab){
    for(int i = 0; i < E.WindowsRow; i++){

        if(i<E.WindowsRow-1) {
            ABufferAppend(ab, "\x1b[K", 3);
            ABufferAppend(ab, "~", 1);
            ABufferAppend(ab, "\r\n", 2);
        }

        if(i == E.WindowsRow - 1){
            char welCome[80];
            char cup[10];

            int welComeLen = snprintf(welCome, sizeof(welCome), "Welcome to Kilo editor -- version %s -- by miceVenus",KILO_VERSION);
            // printf("%d", welComeLen); 在启动原始模式后就不能用printf打印了 
            if(welComeLen > E.WindowsCol) welComeLen = E.WindowsCol;

            int cupLen = snprintf(cup, sizeof(cup), "\x1b[%d;%dH", E.WindowsRow, E.WindowsCol - welComeLen + 1);

            ABufferAppend(ab, cup, cupLen);
            ABufferAppend(ab, welCome, welComeLen);
        }
    }
}

void RefreshScreen(){
    struct appendBuffer ab = ABUF_INIT;

    ABufferAppend(&ab, "\x1b[?25l", 6);
    ABufferAppend(&ab, "\x1b[H", 3);

    DrawRows(&ab);

    char buf[32];
    int bufLen = snprintf(buf, sizeof(buf), "\x1b[%d;%dH", E.ycursPosition + 1, E.xcursPosition + 1);
    ABufferAppend(&ab, buf, bufLen);
    ABufferAppend(&ab, "\x1b[?25h", 6);
    write(STDOUT_FILENO, ab.buffer, ab.len);

    ABufferFree(&ab);
}

/*** appendBuffer ***/

// 有分配失败的风险
void ABufferAppend(struct appendBuffer *ab, const char *s, int len){
    char *newString = realloc(ab->buffer, ab->len + len);

    memcpy(&newString[ab->len], s, len);

    ab->buffer = newString;
    ab->len += len;
}

void ABufferFree(struct appendBuffer *ab){
    free(ab->buffer);
}

/*** init ***/
void InitEditor(){
    E.xcursPosition = 0;
    E.ycursPosition = 0;
    if(GetWindowRowCol(&E.WindowsRow, &E.WindowsCol) == -1)
        Die("GetWindowSize");
}


int main(){
    EnableRawMode();
    InitEditor();

    while(1){
        RefreshScreen();
        ProcessKeypress();
    }

    return 0;
}

/*** init ***/