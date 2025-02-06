#include "../include/main.h"
#include "../include/terminal.h"
#include "../include/editor.h"
#include "../include/input.h"
#include "../include/file.h"


int main(int argc, char **argv){
    EnableRawMode();
    InitEditor();
    if(argc >= 2){
        fileOpen(argv[1]);
    }
    while(1){
        RefreshScreen();
        ProcessKeypress();
    }

    return 0;
}
