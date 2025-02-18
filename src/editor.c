#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include "../include/editor.h"
#include "../include/terminal.h"
#include "../include/input.h"
#include "../include/file.h"
#include "../include/syntaxHighlight.h"
#include "../include/appendbuffer.h"

char *C_HL_extensions[] = {".c", ".cpp", ".h", NULL};
char *C_HL_keywords[] = {
    "switch", "if", "while", "for", "break", "continue", "return", "else",
    "struct", "union", "typedef", "static", "enum", "class", "case",
    
    "int|", "long|", "double|", "float|", "char|", "unsigned|", "signed|",
    "#define|", "#include|", "#if|", "#else|", "#endif|", "#ifdef|", "#ifndef|",
    "NULL|","void|", NULL
};
editorSyntax HLDB[] = {
    {
        "c", 
        C_HL_extensions, 
        C_HL_keywords,
        "//",
        "/*",
        "*/",
        HL_HIGHLIGHT_NUMBERS|HL_HIGHLIGHT_WORDS|HL_HIGHLIGHT_STRINGS|
        HL_HIGHLIGHT_COMMENT|HL_HIGHLIGHT_KEYWORD,
    },
};

void InitEditor(){
    E.dirty = 0;
    E.xcursPosition = 0;
    E.ycursPosition = 0;
    E.numrows = 0;
    E.row = NULL;
    E.rowoff = 0;
    E.coloff = 0;
    E.filename = NULL;
    E.fileExtension = NULL;
    E.rxcursPosition = 0;
    E.statusmsg[0] = '\0';
    E.statusmsg_time = 0;
    E.syntax = NULL;
    if(GetWindowRowCol(&E.WindowsRow, &E.WindowsCol) == -1)
        Die("GetWindowSize");
    E.WindowsRow -= 2;
    editorSetStatusMessage("HELP: Ctrl-Q = quit | Ctrl-S = save | Ctrl-F = find");
}

void RefreshScreen(){
    appendBuffer ab;
    ABufferSetup(&ab);
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

void DrawRows(struct appendBuffer *ab){
    for(int i = 0; i < E.WindowsRow; i++){
        int fileRow = i + E.rowoff;
        editorRow *row = &E.row[fileRow];
        
        if(i == E.WindowsRow - 1 && E.numrows == 0) PrintWelcome(ab);

        if(fileRow >= E.numrows){
            if(E.numrows == 0 && i < E.WindowsRow - 1) ABufferAppend(ab, "~", 1);
            else
            if(i < E.WindowsRow && E.numrows != 0) ABufferAppend(ab, "~", 1);
        }else{
            int len = E.row[fileRow].rsize - E.coloff;
            if(len < 0) len = 0;
            if(len > E.WindowsCol) len = E.WindowsCol;
            char *c = row->render;
            char escBuf[32];
            for(int j = E.coloff; j < E.coloff+len; j++){
                if(iscntrl(c[j])){
                    char sym = (c[j] <= 26) ? '@' : '?';
                    ABufferAppend(ab, "\x1b[7m", 4);
                    ABufferAppend(ab, &sym, 1);
                    ABufferAppend(ab, "\x1b[m", 3);
                }else if(row->hl[j] == HL_NORMAL){
                    ABufferAppend(ab, "\x1b[39m", 5);
                    ABufferAppend(ab, &c[j], 1);
                }else{
                    int hl = highlight2Color(row->hl[j]);
                    int len = sprintf(escBuf, "\x1b[%dm", hl);
                    ABufferAppend(ab, escBuf, len);
                    ABufferAppend(ab, &c[j], 1);
                }
                ABufferAppend(ab, "\x1b[39m", 5);
            }
        }

        ABufferAppend(ab, "\x1b[K", 3);
        ABufferAppend(ab, "\r\n", 2);
    }
}
void editorScroll(){
    E.rxcursPosition = 0;
    if(E.ycursPosition < E.numrows){
        E.rxcursPosition = xcurs2Rxcurs(&E.row[E.ycursPosition], E.xcursPosition);
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
char *editorPrompt(char *prompt, void (*callback) (const char *, int), char *defaultBuf){
    size_t bufsize = 128;
    size_t buflen = 0;
    char *buf;
    if(defaultBuf){
        buf = strdup(defaultBuf);
        buflen = strlen(defaultBuf);
    }
    else{
        buf = (char*)malloc(bufsize);
        buf[buflen] = '\0';
    }
    while(1){
        editorSetStatusMessage(prompt, buf);
        RefreshScreen();
        int key = ReadKeypress();
        if(key == ARROW_DOWN || key == ARROW_RIGHT){
            if(callback) callback(buf, key);
        }
        if(key == ARROW_UP || key == ARROW_LEFT){
            if(callback) callback(buf, key);
        }

        if(key == '\r' || key == '\x1b'){
            if(callback) callback(buf, key);
            return buf;
        }
        if(!iscntrl(key) && key < 128){
            if(buflen == bufsize - 1){
                bufsize *= 2;
                buf = realloc(buf, bufsize);
            }
            buf[buflen++] = key;
            buf[buflen] = '\0';
            if(callback) callback(buf, key);

        }else if(key == BACKSPACE || key == CTRL_KEY('h') || key == DELETE){
            if(buflen != 0) buflen--;
            buf[buflen] = '\0';
            if(callback) callback(buf, key);

        }else if(key == CTRL_KEY('q')){
            free(buf);
            return NULL;
        }
    }
    return buf;
}

void editorAppendRow(char *s, size_t len){
    editorInsertRow(E.numrows, s, len);
}

void editorFreeLine(editorRow *row){
    free(row->render);
    free(row->chars); 
    free(row->hl);
}

void editorDelRow(int pos){
    if(pos < 0 || pos >= E.numrows) return;
    editorFreeLine(&E.row[pos]);
    memmove(&E.row[pos], &E.row[pos + 1], sizeof(editorRow) * (E.numrows - pos - 1));
    E.numrows--;
    E.dirty++;
}

void editorInsertRow(int pos, char *s, size_t len){
    if(pos < 0 || pos > E.numrows) return;

    E.row = realloc(E.row, sizeof(editorRow) * (E.numrows + 1));

    memmove(&E.row[pos + 1], &E.row[pos], sizeof(editorRow) * (E.numrows - pos));

    editorRow *row = &E.row[pos];
    row->size = len;
    row->chars = malloc(len + 1);
    memcpy(row->chars, s, len);
    row->chars[len] = '\0';
    row->rsize = 0;
    row->render = NULL;
    row->hl = NULL;

    eidtorUpdateRow(row);
    E.numrows++;
    E.dirty++;
}

void eidtorUpdateRow(editorRow *row){
    int tabs = 0;
    for(int i = 0; i < row->size; i++) if(row->chars[i] == '\t') tabs++;
    free(row->render);
    row->render = malloc(row->size + 1 + tabs * (EDITOR_TAB_STOP - 1));

    int rsize = 0;
    for(int i = 0; i < row->size; i++){
        if(row->chars[i] == '\t'){
            row->render[rsize++] = ' ';
            while(rsize % EDITOR_TAB_STOP != 0)  row->render[rsize++] = ' ';
        }else{
            row->render[rsize++] = row->chars[i];
        }
    }
    row->render[rsize] = '\0';
    row->rsize = rsize;
    updateSyntaxHighlight(row);
}

void EditorInsertString(char *s, int pos, size_t len){
    if(pos < 0 || pos > E.numrows) return;
    editorRow *row = &E.row[pos];
    row->chars = realloc(row->chars, row->size + len + 1);
    memmove(&row->chars[row->size], s, len);
    row->size += len;
    row->chars[row->size] = '\0';
    eidtorUpdateRow(row);
    E.dirty++;
}
void EditorInsertChar(int c){
    if(E.ycursPosition == E.numrows){
        editorAppendRow("", 0);
    }
    RowInsertChar(&E.row[E.ycursPosition], E.xcursPosition, c);
    E.xcursPosition++;
}
void EditorDelChar(){
    if(E.ycursPosition == E.numrows) return;
    if(E.xcursPosition > 0){
        RowDelChar(&E.row[E.ycursPosition], E.xcursPosition - 1);
        E.xcursPosition--;
    }else if(E.xcursPosition == 0 && E.ycursPosition > 0){
        E.xcursPosition = E.row[E.ycursPosition - 1].size;
        EditorInsertString(E.row[E.ycursPosition].chars, E.ycursPosition - 1 , E.row[E.ycursPosition].size);
        editorDelRow(E.ycursPosition);
        E.ycursPosition--;
    }
}

void RowInsertChar(editorRow *row, int pos, int c){
    if(pos < 0 || pos > row->size) pos = row->size;
    if((iscntrl(c)&& c != '\n' && c != '\t') || c == 127) return;
    row->chars = realloc(row->chars, row->size + 2);
    row->size++;
    memmove(&row->chars[pos + 1], &row->chars[pos], row->size - pos);
    row->chars[pos] = c;
    eidtorUpdateRow(row);
    E.dirty++;
}

void RowDelChar(editorRow *row, int pos){
    if(pos < 0 || pos >= row->size) return;
    memmove(&row->chars[pos], &row->chars[pos + 1], row->size - pos);
    row->size--;
    eidtorUpdateRow(row);
    E.dirty++;
}

void DrawStatusBar(struct appendBuffer *ab){
    ABufferAppend(ab, "\x1b[7m", 4);
    char status[80];

    int len = snprintf(status, sizeof(status), "%.20s - %s - %d lines - %d current lines%s",
    E.filename ? E.filename : "[No Name]", 
    E.syntax ? E.syntax->fileExtension : "[No FileType]",
    E.numrows, 
    E.ycursPosition,
    E.dirty ? " (modified)" : "");

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

void editorSetStatusMessage(const char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(E.statusmsg, sizeof(E.statusmsg), fmt, ap);
    va_end(ap);
    E.statusmsg_time = time(NULL);
}

void PrintWelcome(struct appendBuffer *ab){
    char welCome[80];
    char cup[10];

    int welComeLen = snprintf(welCome, sizeof(welCome), "Welcome to Kilo editor -- version %s -- by miceVenus", BBM_EDITOR_VERSION);
    if(welComeLen > E.WindowsCol) welComeLen = E.WindowsCol;

    int cupLen = snprintf(cup, sizeof(cup), "\x1b[%d;%dH", E.WindowsRow, E.WindowsCol - welComeLen + 1);

    ABufferAppend(ab, cup, cupLen);
    ABufferAppend(ab, welCome, welComeLen);
}

int xcurs2Rxcurs(editorRow *row, int xcurs){
    int rxcurs = 0;
    for(int i = 0; i < xcurs; i++){
        if(row->chars[i] == '\t'){
            rxcurs += (EDITOR_TAB_STOP - 1) - (rxcurs % EDITOR_TAB_STOP);
        }
        rxcurs++;
    }
    return rxcurs;
}

int Rxcurs2xcurs(editorRow *row, int rxcurs){
    int xcurs = 0;
    for(xcurs = 0; xcurs < rxcurs; xcurs++){
        if(row->chars[xcurs] == '\t'){
            rxcurs -= (EDITOR_TAB_STOP - 1) - (rxcurs % EDITOR_TAB_STOP);
        }
        xcurs++;
    }
    return xcurs;
}
int getHLDBEntries(){
    return sizeof(HLDB) / sizeof(HLDB[0]);
}