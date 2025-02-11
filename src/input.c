#include<unistd.h>
#include<stdlib.h>
#include<errno.h>

#include "../include/input.h"
#include "../include/editor.h"
#include "../include/terminal.h"
#include "../include/file.h"
#include "../include/editorFind.h"

int ReadKeypress(){
    int nread;
    char key = '\0';
    while((nread = read(STDERR_FILENO, &key, 1)) != 1){
        if(nread == -1 && errno != EAGAIN) Die("read");
    }
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
    static int quitTime = EIDTOR_CTRLQ_TIME;

    int key = ReadKeypress();
    switch(key){
        case '\r':
            HandleEnter();
            break;
        case CTRL_KEY('q'):
            if(E.dirty != 0 && quitTime > 0) WaringExitWithoutSave(quitTime--);
            else{
                FlushTerminalAndSetCursorToLT();
                exit(0);
                break;
            }
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
        case BACKSPACE:
        case CTRL_KEY('h'):
            EditorDelChar();
            break;
        case DELETE:
            if(E.xcursPosition < E.row[E.ycursPosition].size){
                MoveCursor(ARROW_RIGHT);
                EditorDelChar();
                break;
            }
            else
                break;
        case CTRL_KEY('l'):
        case '\x1b':
            break;

        case CTRL_KEY('s'):
            fileSave();
            break;
        
        case CTRL_KEY('f'):
            editorFind();
            break;

        default:
            EditorInsertChar(key);
            break;
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
}

void WaringExitWithoutSave(int quitTime){
    editorSetStatusMessage("Use  CTRL-S To Save Or Repeat CTRL-Q To Exit Times : %d", quitTime);
}

void HandleEnter(){
    if(E.xcursPosition == 0){
        editorInsertRow(E.ycursPosition, "", 0);
        if (E.ycursPosition < E.numrows) {
            E.ycursPosition++;
            E.xcursPosition = 0;
        }
    }else{
        editorInsertRow(E.ycursPosition + 1, &E.row[E.ycursPosition].chars[E.xcursPosition], E.row[E.ycursPosition].size - E.xcursPosition);
        editorRow *row = &E.row[E.ycursPosition];
        row->size = E.xcursPosition;
        row->chars[row->size] = '\0';

        eidtorUpdateRow(row);
        if (E.ycursPosition < E.numrows) {
            E.ycursPosition++;
            E.xcursPosition = 0;
        }
    }
}