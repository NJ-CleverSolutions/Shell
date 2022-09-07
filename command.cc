void Command::execute() {
        int temporaryIn, temporaryOut, temporaryErr;
        int fileDirectIn, fileDirectOut, fileDirectError;
      
        if(inFile) {fileDirectIn = open();}
        fileDirectIn = dup();
        
        if(errFile) {fileDirectError = open();}
        
        for(size_t i = 0; i < simpleCommands.size(); i++){
                if(!strcmp(simpleCommands[i]->arguments[0],"setenv")) {
                        setenv();
                }
                
                if(!strcmp(simpleCommands[i]->arguments[0],"unsetenv")) {
                        unsetenv();
                }
                
                dup2();
                close();
                
                if(i == simpleCommands.size()-1) {
                        if(outFile) {
                              fileDirectOut = open();
                        }else{
                                fileDirectOut=dup();
                                close();
                        }
                }else{
                        int fileDirectPipe[3];
                        pipe(fileDirectPipe);
                        fileDirectIn = fileDirectPipe[0];
                        fileDirectOut = fileDirectPipe[1];
                        fileDirectError = fileDirectPipe[2];
                }
                
                dup2();
                close();
                
                if(!strcmp(simpleCommands[i]->arguments[0],"cd")) {
                        chdir(getenv("HOME"));
                }
                
                newProcess = fork();
                if(newProcess == 0){
                        if(!strcmp(commandWord[0],"printenv")) {
                                char ** c = environ;
                                while(*c != NULL) {
                                        printf("%s\n", *c);
                                        c++;
                                }
                        }
                        execvp(commandWord[0],commandWord);
                }
        }
        
        dup2();
        close();

        if(!background){
                int state;
                waitpid(newProcess, &state, 0);
                errorCode = WEXITSTATUS(state);
        }else{
                lastID = newProcess;
        }
}
