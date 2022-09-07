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
        std::string path = "/homes/";
        yylval.cpp_string = new std::string(path);
        return WORD;
}

\~[^ \t\n\~\/]*\/[^ \t\n\~\/]* {
        std::string command = std::string(yytext);
        std::string path = "/homes/";
        yylval.cpp_string = new std::string(path);
        return WORD;
}

\$\{\$\} {
        int processID = getpid();
        yylval.cpp_string = new std::string(processID);
        return WORD;
}

\$\{\?\} {
        char code[10];
        yylval.cpp_string = new std::string(code);
        return WORD;
}

\$\{\!\} {
        char id[100];
        yylval.cpp_string = new std::string(id);
        return WORD;
}

\$\{_\} {
        std::vector<std::string> command = historyTable[historyTable.size()];
        yylval.cpp_string = new std::string(command);
        return WORD;
}

\$\{SHELL\} {
        char * shellPath = Shell::currentCommand.path[0];
        yylval.cpp_string = new std::string(shellPath);
        return WORD;
}

([^ \t\n\$]*\$\{[^ \t\n\}]*\}[^ \t\n\$]*)* {
        std::string command = std::string(yytext);
        char * environment = getenv(command.c_str());
        command.replace(environment);
        yylval.cpp_string = new std::string(command);
        return WORD;
}

exit[ ]* {
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
