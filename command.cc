#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <sys/wait.h>
#include "command.hh"
#include "shell.hh"
int newProcess;
int hist = 0;
int errorCode = 0;
int lastID = 0;
std::vector<std::vector<std::string>> historyTable = std::vector<std::vector<std::string>>();
Command::Command() {
        simpleCommands = std::vector<SimpleCommand *>();
        append = NULL;
        outFile = NULL;
        inFile = NULL;
        errFile = NULL;
        background = false;
        doubleGreaterThan = 0;
}
void Command::insertSimpleCommand( SimpleCommand * simpleCommand ) {
        simpleCommands.push_back(simpleCommand);
}
void Command::clear() {
        for (auto simpleCommand : simpleCommands) {
                delete simpleCommand;
        }
        simpleCommands.clear();
        if ( inFile ) {
                delete inFile;
        }
        inFile = NULL;

        if(outFile == errFile) {
                delete errFile;
        }else{
                if(outFile) {
                        delete outFile;
                }
                if(errFile) {
                        delete errFile;
                }
        }
        outFile = NULL;
        errFile = NULL;
        append = false;
        background = false;
        doubleGreaterThan = 0;
}
void Command::print() {
        /*if(isatty(0)){
                printf("\n\n");
                printf("              COMMAND TABLE                \n");
                printf("\n");
                printf("  #   Simple Commands\n");
                printf("  --- ----------------------------------------------------------\n");
                int i = 0;
                for ( auto & simpleCommand : simpleCommands ) {
                        printf("  %-3d ", i++ );
                        simpleCommand->print();
                }
                printf( "\n\n" );
                printf( "  Output       Input        Error        Background\n" );
                printf( "  ------------ ------------ ------------ ------------\n" );
                printf( "  %-12s %-12s %-12s %-12s\n",
                        outFile?outFile->c_str():"default",
                        inFile?inFile->c_str():"default",
                        errFile?errFile->c_str():"default",
                        background?"YES":"NO");
                printf( "\n\n" );
        }*/
}
void Command::execute() {
        if (simpleCommands.size() == 0) {
                Shell::prompt();
                return;
        }
        print();
        int temporaryIn = dup(0);
        int temporaryOut = dup(1);
        int temporaryErr = dup(2);
        int fileDirectIn;
        int fileDirectOut;
        int fileDirectError;
        if(inFile) {
                fileDirectIn = open(inFile->c_str(),O_RDWR);
        }else{
                fileDirectIn=dup(temporaryIn);
        }
        if(errFile) {
                if(append) {
                        fileDirectError = open(errFile->c_str(),O_WRONLY|O_APPEND|O_CREAT,0600);
                }else{
                        fileDirectError = open(errFile->c_str(),O_WRONLY|O_CREAT|O_TRUNC,0600);
                }
        }
        for(size_t i = 0; i < simpleCommands.size(); i++){
                if(!strcmp(simpleCommands[i]->arguments[0]->c_str(),"setenv")) {
                        setenv(simpleCommands[i]->arguments[1]->c_str(),simpleCommands[i]->arguments[2]->c_str(),1);
                        break;
                }
                if(!strcmp(simpleCommands[i]->arguments[0]->c_str(),"unsetenv")) {
                        unsetenv(simpleCommands[i]->arguments[1]->c_str());
                        break;
                }
                dup2(fileDirectIn,0);
                close(fileDirectIn);
                if(i == simpleCommands.size()-1) {
                        if(outFile) {
                                if(append) {
                                        fileDirectOut = open(outFile->c_str(),O_WRONLY|O_APPEND|O_CREAT,0600);
                                }else{
                                        fileDirectOut = open(outFile->c_str(),O_WRONLY|O_CREAT|O_TRUNC,0600);
                                }
                        }else{
                                fileDirectOut=dup(temporaryOut);
                                close(temporaryOut);
                        }
                }else{
                        int fileDirectPipe[3];
                        pipe(fileDirectPipe);
                        fileDirectIn = fileDirectPipe[0];
                        fileDirectOut = fileDirectPipe[1];
                        fileDirectError = fileDirectPipe[2];
                }
                dup2(fileDirectOut,1);
                close(fileDirectOut);
                dup2(fileDirectError,2);
                close(fileDirectError);
                if(!strcmp(simpleCommands[i]->arguments[0]->c_str(),"source")) {
                        break;
                }
                if(!strcmp(simpleCommands[i]->arguments[0]->c_str(),"cd")) {
                        if(simpleCommands[i]->arguments.size() == 1){
                                chdir(getenv("HOME"));
                        }else if(simpleCommands[i]->arguments[1]->c_str()[0] == '$') {
                                chdir(getenv("HOME"));
                        }else{
                                int error = chdir(simpleCommands[i]->arguments[1]->c_str());
                                if(error == -1){
                                        fprintf(stderr, "cd: can't cd to %s\n",simpleCommands[i]->arguments[1]->c_str());
                                }
                        }
                        break;
                }
                newProcess = fork();
                if(newProcess == 0){
                        std::vector<char*> newVector(simpleCommands[i]->arguments.size());
                        for(size_t j = 0; j < newVector.size(); j++){
                                newVector[j] = const_cast <char*> (simpleCommands[i]->arguments[j]->c_str());
                        }
                        newVector.push_back(NULL);
                        char **commandWord = &newVector[0];
                        if(!strcmp(commandWord[0],"printenv")) {
                                char ** c = environ;
                                while(*c != NULL) {
                                        printf("%s\n", *c);
                                        c++;
                                }
                                exit(0);
                        }
                        execvp(commandWord[0],commandWord);
                        perror("execvp");
                        exit(1);
                }else if(newProcess < 0){
                        perror("fork");
                        return;
                }
        }
        dup2(temporaryIn,0);
        dup2(temporaryOut,1);
        dup2(temporaryErr,2);
        close(temporaryIn);
        close(temporaryOut);
        close(temporaryErr);

        if(!background){
                int state;
                waitpid(newProcess, &state, 0);
                errorCode = WEXITSTATUS(state);
                historyTable.resize(historyTable.size()+1);
                for(size_t i = 0; i < simpleCommands[0]->arguments.size(); i++) {
                        historyTable[hist].push_back(simpleCommands[0]->arguments[i]->c_str());
                }
                hist++;
        }else{
                lastID = newProcess;
        }
        clear();
        Shell::prompt();
}
SimpleCommand * Command::currentSimpleCommand;
