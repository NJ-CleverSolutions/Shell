%code requires
{
#include <string>
#include <sys/types.h>
#include <algorithm>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <cstdio>
#include <unistd.h>
#include "shell.hh"
#if __cplusplus > 199711L
#define register
#endif
}

%union
{
  char        *string_val;
  std::string *cpp_string;
}

%token <cpp_string> WORD
%token GREATERTHAN PIPE LESSTHAN TWOGREATERTHAN GREATERAND GREATGREATERTHAN GREATGREATERAND AND NOTOKEN NEWLINE EXIT

%{
void wildCards(char * prefix, char * suffix);
std::vector<std::string> arguments = std::vector<std::string>();
#define MAXFILENAME 1024
void yyerror(const char * s);
int yylex();

%}

%%
goal: commandList;
commandList:
        commandLine
        | commandList commandLine
        ;
commandLine:
        pipeList ioModifierList backEndOption NEWLINE {
                if(isatty(0)) {
                        //printf("Yacc: Execute command\n");
                }
                Shell::currentCommand.execute();
        }
        | NEWLINE
        | error NEWLINE { yyerrok; }
        ;
pipeList:
        commandArguments
        | pipeList PIPE commandArguments
        ;
ioModifierList:
        ioModifierList ioModifier
        |
        ;
backEndOption:
        AND {
                if(isatty(0)) {
                        //printf("Yacc: insert background \"%s\"\n", "YES");
                }
                Shell::currentCommand.background = true;
        }
        |
        ;
ioModifier:
        GREATGREATERTHAN WORD {
                if(isatty(0)) {
                        //printf("Yacc: insert output \"%s\"\n", $2->c_str());
                }
                Shell::currentCommand.outFile = $2;
                Shell::currentCommand.append = $2;
        }
        ;
ioModifier:
        GREATGREATERAND WORD {
                if(isatty(0)) {
                        //printf("Yacc: insert output \"%s\"\n", $2->c_str());
                        //printf("Yacc: insert error \"%s\"\n", $2->c_str());
                }
                Shell::currentCommand.outFile = $2;
                Shell::currentCommand.errFile = $2;
                Shell::currentCommand.append = true;
        }
        ;
ioModifier:
        GREATERTHAN WORD {
                if(isatty(0)) {
                        //printf("Yacc: insert output \"%s\"\n", $2->c_str());
                }
                if(Shell::currentCommand.doubleGreaterThan > 0) {
                        printf("Ambiguous output redirect.\n");
                }else{
                        Shell::currentCommand.outFile = $2;
                        Shell::currentCommand.doubleGreaterThan++;
                }
        }
        ;
ioModifier:
        TWOGREATERTHAN WORD {
                if(isatty(0)) {
                        //printf("Yacc: insert error \"%s\"\n", $2->c_str());
                }
                Shell::currentCommand.errFile = $2;
        }
        ;
ioModifier:
        GREATERAND WORD {
                if(isatty(0)) {
                        //printf("Yacc: insert output \"%s\"\n", $2->c_str());
                        //printf("Yacc: insert error \"%s\"\n", $2->c_str());
                }
                Shell::currentCommand.outFile = $2;
                Shell::currentCommand.errFile = $2;
        }
        ;
ioModifier:
        LESSTHAN WORD {
                if(isatty(0)) {
                        //printf("Yacc: insert input \"%s\"\n", $2->c_str());
                }
                Shell::currentCommand.inFile = $2;
        }
        ;
commandArguments:
        commandWord argumentList {
                Shell::currentCommand.
                insertSimpleCommand(Command::currentSimpleCommand);
        }
        ;
commandWord:
        WORD {
                if(isatty(0)) {
                        //printf("Yacc: insert command \"%s\"\n", $1->c_str());
                }
                Command::currentSimpleCommand = new SimpleCommand();
                Command::currentSimpleCommand->insertArgument( $1 );
        }
        ;
argumentList:
        argumentList argumentWord
        |
        ;

argumentWord:
        WORD {
                if(isatty(0)) {
                        //printf("Yacc: insert argument \"%s\"\n", $1->c_str());
                }
                char * tester = (char*)$1->c_str();
                if(strchr(tester,'*') == NULL) {
                        Command::currentSimpleCommand->insertArgument($1);
                }else{
                        char * prefix = (char*)"";
                        wildCards(prefix, (char*)$1->c_str());
                        std::sort(arguments.begin(),arguments.end());
                        for (auto a: arguments) {
                                if(a.at(0) == '/' && a.at(1) == '/') {
                                        a.erase(0,1);
                                }
                                std::string * argToInsert = new std::string(a);
                                Command::currentSimpleCommand->insertArgument(argToInsert);
                        }
                        arguments.clear();
                }
        }
        ;
%%
void
yyerror(const char * s) {
        fprintf(stderr,"%s", s);
}
void wildCards(char * prefix, char * suffix) {
        if (suffix[0] == 0) {
                arguments.push_back(prefix);
                return;
        }
        char * s = strchr(suffix, '/');
        char component[MAXFILENAME];
        if (s != NULL) {
                strncpy(component, suffix, s-suffix);
                component[s-suffix] = 0;
                suffix = s + 1;
        }else {
                strcpy(component, suffix);
                suffix = suffix + strlen(suffix);
        }

        char newPrefix[MAXFILENAME*2];
        if (strchr(component,'?')==NULL & strchr(component,'*')==NULL) {
                sprintf(newPrefix, "%s/%s", prefix, component);
                wildCards(newPrefix, suffix);
                return;
        }

        char reg[MAXFILENAME]; char * a = component; char * r = reg;
        *r = '^'; r++;
        while (*a) {
                if (*a == '*') {*r='.'; r++; *r='*'; r++;}
                else if (*a == '?') {*r='.'; r++;}
                else if (*a == '.') {*r='\\'; r++; *r='.'; r++;}
                else {*r=*a; r++;}
                a++;
        }
        *r='$'; r++; *r=0;

        regex_t re; char * dir;
        int expbuf = regcomp(&re, reg, REG_EXTENDED|REG_NOSUB);
        if (strlen(prefix) == 0) {dir = (char*)".";}else{dir = prefix;}
        DIR * d = opendir(dir); if (d == NULL) {return;}

        struct dirent * ent;
        while ((ent = readdir(d)) != NULL) {
                if(regexec(&re, ent->d_name, 1, NULL, 0) == 0) {
                        if(ent->d_name[0] == '.') {
                                if(component[0] == '.') {
                                        if(strcmp(prefix,"")) {
                                                sprintf(newPrefix,"%s/%s",prefix,ent->d_name);
                                        }else{
                                                sprintf(newPrefix,"%s",ent->d_name);
                                        }
                                        wildCards(newPrefix,suffix);
                                }
                        }else{
                                if(!strcmp(prefix,"")){
                                  sprintf(newPrefix,"%s",ent->d_name);
                                }else if(!strcmp(prefix,"/")){
                                  sprintf(newPrefix,"/%s", ent->d_name);
                                }else{
                                  sprintf(newPrefix,"%s/%s",prefix,ent->d_name);
                                }
                                wildCards(newPrefix,suffix);
                        }
                }
        }
        closedir(d);
        regfree(&re);
}


bool sorting(std::string one, std::string two) {
        bool diff = (one.compare(two))<0;
        return diff;
}
#if 0
main() {
        yyparse();
}
#endif
