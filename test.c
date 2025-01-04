#include<stdio.h>
#include<unistd.h>
#include<ctype.h>

int main(){
    char c = 0;
    while (1)
    {
        read(STDIN_FILENO, &c, 1);
        if(iscntrl(c))
            printf("%hd\n", c);
        else
            printf("%hd ('%c')\n", c, c);
    }

    return 0;
}