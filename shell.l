%{
#include "y.tab.hh"
#include "shell.hh"
#include "command.hh"
#include <cstring>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <string>
#include <vector>

extern "C" char * read_line();

int mygetc(FILE * f) {
        static char *p;
        char ch;
        if(!isatty(0)) {
                return getc(f);
        }
        if(p == NULL || *p == 0) {
                char * s = read_line();
                p=s;
        }
        ch = *p;
        p++;
        return ch;
}
#undef getc
#define getc(f) mygetc(f)

extern int errorCode;
extern int lastID;
extern std::vector<std::vector<std::string>> historyTable;
static void yyunput (int c, char *buf_ptr);
void myunputc(int c) {
        unput(c);
}
%}
%option noyywrap
%%

">" {
        return GREATERTHAN;
}

"|" {
        return PIPE;
}

"<" {
        return LESSTHAN;
}

"2>" {
        return TWOGREATERTHAN;
}

">&" {
        return GREATERAND;
}

">>" {
        return GREATGREATERTHAN;
}

">>&" {
        return GREATGREATERAND;
}

"&" {
        return AND;
}

\~|\~\/ {
        yylval.cpp_string = new std::string(getenv("HOME"));
        return WORD;
}

\~[^ \t\n\~/]* {
        std::string command = std::string(yytext);
        command.erase(0,1);
        std::string path = "/homes/";
        path.append(command);
        yylval.cpp_string = new std::string(path);
        return WORD;
}

\~[^ \t\n\~\/]*\/[^ \t\n\~\/]* {
        std::string command = std::string(yytext);
        command.erase(0,1);
        std::string path = "/homes/";
        path.append(command);
        yylval.cpp_string = new std::string(path);
        return WORD;
}

\$\{\$\} {
        int processID = getpid();
        char id[100];
        sprintf(id, "%d", processID);
        yylval.cpp_string = new std::string(id);
        return WORD;
}

\$\{\?\} {
        char code[10];
        sprintf(code,"%d",errorCode);
        yylval.cpp_string = new std::string(code);
        return WORD;
}

\$\{\!\} {
        char id[100];
        sprintf(id, "%d", lastID);
        yylval.cpp_string = new std::string(id);
        return WORD;
}

\$\{_\} {
        std::vector<std::string> list = historyTable[historyTable.size()-1];
        std::string command = list[list.size()-1];
        yylval.cpp_string = new std::string(command);
        return WORD;
}

\$\{SHELL\} {
        char absolute[100];
        char * shellPath = realpath(Shell::currentCommand.path[0],absolute);
        yylval.cpp_string = new std::string(shellPath);
        return WORD;
}

([^ \t\n\$]*\$\{[^ \t\n\}]*\}[^ \t\n\$]*)* {
        std::string command = std::string(yytext);
        std::size_t firstDollar = command.find_first_of("$");
        std::size_t firstFBracket = command.find_first_of("{");
        std::size_t firstBBracket = command.find_first_of("}");
        std::string c1 = command.substr(firstFBracket+1,firstBBracket - firstFBracket - 1);
        char * environment = getenv(c1.c_str());
        command.replace(firstDollar,c1.size()+3,std::string(environment),c1.size()-c1.size());
        firstDollar = command.find_first_of("$");
        firstFBracket = command.find_first_of("{");
        firstBBracket = command.find_first_of("}");
        if(firstDollar != std::string::npos){
                std::string c2 = command.substr(firstFBracket+1,firstBBracket - firstFBracket - 1);
                char * environment = getenv(c2.c_str());
                command.replace(firstDollar,c2.size()+3,std::string(environment),c2.size()-c2.size());
        }
        yylval.cpp_string = new std::string(command);
        return WORD;
}

exit[ ]* {
        if(isatty(0)){
                printf("Good bye!!");
                printf("\n");
        }
        exit(1);
}

\n {
        return NEWLINE;
}

\"(\\.|[^\"])*\" {
        char command[strlen(yytext)];
        int i;
        for(i = 1; i < strlen(yytext); i++){
                if(yytext[i] == '\"') {
                        break;
                }else{
                        command[i-1] = yytext[i];
                }
        }
        command[i-1] = '\0';
        yylval.cpp_string = new std::string(command);
        return WORD;
}

\'[^\n\']*\'|\$\([^\n]*\) {
        std::string command = std::string(yytext);
        if(command.at(0) == '\'') {
                command = command.substr(1,command.size()-1);
                command.append("\nexit\n");
        }else if(command.at(0) == '$') {
                command = command.substr(2,command.size()-3);
                command.append("\nexit\n");
        }

        int inPipe[2];
        int outPipe[2];
        int inTemp = dup(0);
        int outTemp = dup(1);
        pipe(inPipe);
        pipe(outPipe);

        dup2(inPipe[0],0);
        dup2(outPipe[1],1);
        close(inPipe[0]);
        close(outPipe[1]);

        write(inPipe[1],command.c_str(),command.size());
        close(inPipe[1]);

        int newProcess = fork();
        if(newProcess == 0){
                char ** path = new char*[2];
                path[0] = (char*)"/proc/self/exe";
                path[1] = NULL;
                execvp("/proc/self/exe",path);
        }else{
                waitpid(newProcess,NULL,0);

                dup2(inTemp,0); close(inTemp);
                dup2(outTemp,1); close(outTemp);

                char * buf = new char[1024];
                int readInt = read(outPipe[0],buf,1024);
                close(outPipe[0]);

                for(int i = readInt - 1; i >= 0; --i){
                        if(buf[i] == '\n'){
                                buf[i] = '\t';
                        }
                        myunputc(buf[i]);
                }
                delete buf;
        }
}

source[ ][^ \t\n]* {
        std::string command = std::string(yytext);
        command.erase(0,7);
        yyin = fopen(command.c_str(),"r");
        if(!yyin){perror("yyin"); BEGIN(INITIAL);}
        yypush_buffer_state(yy_create_buffer(yyin, YY_BUF_SIZE));
        BEGIN(INITIAL);
}

<<EOF>> {
        yypop_buffer_state();
        if(!YY_CURRENT_BUFFER) {
                yyterminate();
        }
}

[^ \t\n|.]*\\[^ \t\n|.]* {
        char command[strlen(yytext)];
        int j = 0;
        for(int i = 0; i < strlen(yytext); i++){
                if(yytext[i] == '\\') {
                        command[j] = yytext[i+1];
                        j++;
                        i++;
                }else{
                        command[j] = yytext[i];
                        j++;
                }
        }
        command[j] = '\0';
        yylval.cpp_string = new std::string(command);
        return WORD;
}

[ \t] {

}

[^ \|>\t\n][^ \|>\t\n]*  {
        yylval.cpp_string = new std::string(yytext);
        return WORD;
}
%%
