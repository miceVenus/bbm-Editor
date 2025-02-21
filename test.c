#include<stdio.h>
#include<stdlib.h>
int main(){
    char s[128];
    free(s);   
    if(s == NULL) printf("s is NULL\n");
    return 0;
}
