/*** includes ***/

#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include<unistd.h>
#include<errno.h>
#include<stdlib.h>
#include<stdio.h>
#include<sys/ioctl.h>
#include<sys/types.h>
#include<ctype.h>
#include "bbmEditor.h"
#include<string.h>


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
            if(key == PAGE_UP){
                E.ycursPosition = E.rowoff;
            }else if(key == PAGE_DOWN){
                E.ycursPosition = E.rowoff + E.WindowsRow - 1;
                if(E.ycursPosition > E.numrows) E.ycursPosition = E.numrows;
            }
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
            if(E.ycursPosition < E.numrows) E.xcursPosition = E.row[E.ycursPosition].rsize;
            break;
        }
        case DELETE:{
            MoveCursor(DELETE);
            break;
        }
    }
}

void MoveCursor(int key){
    // editorRow *row = (E.ycursPosition >= E.numrows) ? NULL : &E.row[E.ycursPosition];
    switch (key) {
        case ARROW_LEFT:
            if(E.ycursPosition < E.numrows){
                if (E.xcursPosition != 0) {
                E.xcursPosition--;
                } else if(E.ycursPosition > 0){
                E.ycursPosition--;
                E.xcursPosition = E.row[E.ycursPosition].size;
                }
            }
            else E.xcursPosition = 0;

            break;
        case ARROW_RIGHT:
            if(E.ycursPosition < E.numrows){
                if (E.xcursPosition < E.row[E.ycursPosition].size) {
                E.xcursPosition++;
                } else if(E.ycursPosition < E.numrows){
                E.ycursPosition++;
                E.xcursPosition = 0;
                }
            }
            else E.xcursPosition = 0;

            break;
        case ARROW_UP:
            if (E.ycursPosition != 0) {
            E.ycursPosition--;
            }
            if(E.ycursPosition == E.numrows) E.xcursPosition = 0;
            else E.xcursPosition = E.row[E.ycursPosition].size;
            break;
        case ARROW_DOWN:
            if (E.ycursPosition < E.numrows) {
            E.ycursPosition++;
            }
            if(E.ycursPosition == E.numrows) E.xcursPosition = 0;
            else E.xcursPosition = E.row[E.ycursPosition].size;
            break;
        case DELETE:
            if (E.xcursPosition != 0) {
            E.xcursPosition--;
            }
            break;
    }
    // if(row != NULL && E.xcursPosition > row->size){
    //     E.xcursPosition = row->size;
    // }
}
/*** output ***/

void PrintWelcome(struct appendBuffer *ab){
    char welCome[80];
    char cup[10];

    int welComeLen = snprintf(welCome, sizeof(welCome), "Welcome to Kilo editor -- version %s -- by miceVenus",KILO_VERSION);
    if(welComeLen > E.WindowsCol) welComeLen = E.WindowsCol;

    int cupLen = snprintf(cup, sizeof(cup), "\x1b[%d;%dH", E.WindowsRow, E.WindowsCol - welComeLen + 1);

    ABufferAppend(ab, cup, cupLen);
    ABufferAppend(ab, welCome, welComeLen);
}

void DrawStatusBar(struct appendBuffer *ab){
    ABufferAppend(ab, "\x1b[7m", 4);
    char status[80];

    int len = snprintf(status, sizeof(status), "%.20s - %d lines - %d current lines",
    E.filename ? E.filename : "[No Name]", 
    E.numrows, 
    E.ycursPosition);

    if (len > E.WindowsCol) len = E.WindowsCol;
    for(int i = 0; i < E.WindowsCol - len; i++){
        ABufferAppend(ab, " ", 1);
    }
    ABufferAppend(ab, status, len);
    ABufferAppend(ab, "\x1b[m", 3);
    ABufferAppend(ab, "\r\n", 2);
}
void DrawStatusMessageBar(struct appendBuffer *ab){
    ABufferAppend(ab, "\x1b[K", 3);
    int msgLen = strlen(E.statusmsg);
    if(msgLen > E.WindowsCol) msgLen = E.WindowsCol;
    if(msgLen && (time(NULL) - E.statusmsg_time < 5)){
        ABufferAppend(ab, E.statusmsg, msgLen);
    }
}

void DrawRows(struct appendBuffer *ab){
    for(int i = 0; i < E.WindowsRow; i++){
        int fileRow = i + E.rowoff;
        
        if(i == E.WindowsRow - 1 && E.numrows == 0) PrintWelcome(ab);

        if(fileRow >= E.numrows){
            if(E.numrows == 0 && i < E.WindowsRow - 1) ABufferAppend(ab, "~", 1);
            else
            if(i < E.WindowsRow && E.numrows != 0) ABufferAppend(ab, "~", 1);
        }else{
            int len = E.row[fileRow].rsize - E.coloff;
            if(len < 0) len = 0;
            if(len > E.WindowsCol) len = E.WindowsCol;
            ABufferAppend(ab, &E.row[fileRow].render[E.coloff], len);
        }

        ABufferAppend(ab, "\x1b[K", 3);
        ABufferAppend(ab, "\r\n", 2);
    }
}
int xcursToRxcurs(editorRow *row, int xcurs){
    int rxcurs = 0;
    for(int i = 0; i < xcurs; i++){
        if(row->chars[i] == '\t'){
            rxcurs += (KILO_TAB_STOP - 1) - (rxcurs % KILO_TAB_STOP);
        }
        rxcurs++;
    }
    return rxcurs;
}
void editorSetStatusMessage(const char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(E.statusmsg, sizeof(E.statusmsg), fmt, ap);
    va_end(ap);
    E.statusmsg_time = time(NULL);
}

void editorScroll(){
    E.rxcursPosition = 0;
    if(E.ycursPosition < E.numrows){
        E.rxcursPosition = xcursToRxcurs(&E.row[E.ycursPosition], E.xcursPosition);
    }
    if(E.ycursPosition < E.rowoff){
        E.rowoff = E.ycursPosition;
    }
    if(E.ycursPosition >= E.rowoff + E.WindowsRow){
        E.rowoff = E.ycursPosition - E.WindowsRow + 1;
    }
    if(E.rxcursPosition < E.coloff){
        E.coloff = E.rxcursPosition;
    }
    if(E.rxcursPosition >= E.WindowsCol + E.coloff){
        E.coloff = E.rxcursPosition - E.WindowsCol + 1;
    }
}
void RefreshScreen(){
    struct appendBuffer ab = ABUF_INIT;
    editorScroll();

    ABufferAppend(&ab, "\x1b[?25l", 6);
    ABufferAppend(&ab, "\x1b[H", 3);

    DrawRows(&ab);
    DrawStatusBar(&ab);
    DrawStatusMessageBar(&ab);

    char buf[32];
    int bufLen = snprintf(buf, sizeof(buf), "\x1b[%d;%dH", E.ycursPosition - E.rowoff + 1, E.rxcursPosition - E.coloff + 1);
    ABufferAppend(&ab, buf, bufLen);
    ABufferAppend(&ab, "\x1b[?25h", 6);
    write(STDOUT_FILENO, ab.buffer, ab.len);

    ABufferFree(&ab);
}
/*** file IO ***/
void editorAppendNewLine(char *s, size_t len){
    E.row = realloc(E.row, sizeof(editorRow) * (E.numrows + 1));

    E.row[E.numrows].size = len;
    E.row[E.numrows].chars = malloc(len + 1);
    memcpy(E.row[E.numrows].chars, s, len);
    E.row[E.numrows].chars[len] = '\0';

    E.row[E.numrows].rsize = 0;
    E.row[E.numrows].render = NULL;

    eidtorUpdateRow(&E.row[E.numrows]);

    E.numrows++;
}

void eidtorUpdateRow(editorRow *row){
    int tabs = 0;
    for(int i = 0; i < row->size; i++) if(row->chars[i] == '\t') tabs++;
    free(row->render);
    row->render = malloc(row->size + 1 + tabs * (KILO_TAB_STOP - 1));

    int rsize = 0;
    for(int i = 0; i < row->size; i++){
        if(row->chars[i] == '\t'){
            row->render[rsize++] = ' ';
            while(rsize % KILO_TAB_STOP != 0)  row->render[rsize++] = ' ';
        }else{
            row->render[rsize++] = row->chars[i];
        }
    }
    row->render[rsize] = '\0';
    row->rsize = rsize;
}
void editorOpen(char *filename){
    free(E.filename);
    E.filename = strdup(filename);
    FILE *fp = fopen(filename, "r");
    if(!fp) Die("fopen");

    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen = getline(&line, &linecap, fp);

    while(linelen != -1){
        while(linelen > 0 && (line[linelen - 1] == '\n' || line[linelen - 1] == '\r')){
            linelen--;
        }
        editorAppendNewLine(line, linelen);
        linelen = getline(&line, &linecap, fp);
    }
    free(line);
    fclose(fp);
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
    E.numrows = 0;
    E.row = NULL;
    E.rowoff = 0;
    E.coloff = 0;
    E.filename = NULL;
    E.rxcursPosition = 0;
    E.statusmsg[0] = '\0';
    E.statusmsg_time = 0;
    if(GetWindowRowCol(&E.WindowsRow, &E.WindowsCol) == -1)
        Die("GetWindowSize");
    E.WindowsRow -= 2;
}


int main(int argc, char **argv){
    EnableRawMode();
    InitEditor();
    if(argc == 2){
        editorOpen(argv[1]);
    }
    editorSetStatusMessage("HELP: Ctrl-Q = quit");
    while(1){
        ProcessKeypress();
        RefreshScreen();
    }

    return 0;
}

/*** init ***/