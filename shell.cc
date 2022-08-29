#include <cstdio>
#include <unistd.h>
#include "shell.hh"
#include "command.hh"
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
int yyparse(void);
char ** promptCheck;
void Shell::prompt() {
        if(isatty(0)) {
                printf("myshell>");
                fflush(stdout);
        }
}
extern "C" void controlC(int num) {
        if(num==SIGINT){printf("\n"); Shell::prompt();}
}
void zombie(int num) {
        if(num==0){}
        int state;
        while(waitpid(-1,&state,WNOHANG)>0);
        errorCode = WEXITSTATUS(state);
}
int main(int argc, char ** argv) {
        promptCheck = argv;
        if(argc==0){}
        Shell::currentCommand.path = argv;
        signal(SIGINT,controlC);
        signal(SIGCHLD,zombie);
        Shell::prompt();
        yyparse();
}
Command Shell::currentCommand;
