void read_line_print_usage() {
        std::string usage = "\n"
        " Left-Arrow-Key              Moves cursor to the left\n"
        " Right-Arrow-Key       Moves cursor to the right\n"
        " Delete-Key(ctrl-D)    Removes character at cursor\n"
        " Backspace-Key(ctrl-H) Removes the character at the position before the cursor\n"
        " Home-Key(ctrl-A)      Moves the cursor to the beginning of the line\n"
        " End-Key(ctrl-E)       Moves the cursor to the end of the line\n"
        " Up-Arrow-Key          Shows the previous command in the history list\n"
        " Down-Arrow-Key        Shows the next command in the history list\n";
        write(1, usage.c_str(), usage.size());
}

extern "C" char * read_line() {
        while(true) {
                read(0, &ch, 1);
                if(ch == 1) {
                        write(1,"\r",5);
                        write(1,"\033[8C",5);
                }else if(ch == 4) {
                        write(1,"\033[2K\r",5);
                        write(1,"myshell>",8);
                        line.erase(cursor,1);
                        write(1,(char*)line.c_str(),line.size());
                        while(cursor != tempLength) {
                                write(1,"\033[1D",5);
                                cursor--;
                        }
                }else if(ch == 5) {
                        write(1,"\r",5);
                        write(1,"\033[8C",5);
                        while(cursor < lineLength) {
                                write(1,"\033[1C",5);
                                cursor++;
                        }
                }else if (ch == 8 || ch == 127) {
                        write(1,"\033[2K\r",5);
                        write(1,"myshell>",8);
                        line.erase(cursor-1,1);
                        write(1,(char*)line.c_str(),line.size());
                        while(cursor != tempLength) {
                                write(1,"\033[1D",5);
                                cursor--;
                        }
                }else if(ch == 10) {
                        line.clear();
                        write(1,&ch,1);
                        write(1,"myshell>",8);
                }else if (ch==27) {
                        read(0, &ch1, 1); 
                        read(0, &ch2, 1);
                        if (ch1==91 && ch2==65) {
                                write(1,"\033[2K\r",5);
                                write(1,"myshell>",8);
                                line = history[index2];
                                write(1,(char*)line.c_str(),lineLength);
                        }else if(ch1 == 91 && ch2 == 66) {
                                write(1,"\033[2K\r",5);
                                write(1,"myshell>",8);
                                line = history[index2+1];
                                write(1,(char*)line.c_str(),lineLength);        
                        }else if((ch1 == 91 && ch2 == 68) && lineLength != 0) {
                                write(1,"\033[1D",5);
                        }else if((ch1 == 91 && ch2 == 67) && lineLength != 0) { // right
                                write(1,"\033[1C",5);
                        }
                }else if (ch == 31) {
                        read_line_print_usage();
                }else if(ch >= 32 && ch < 127) {
                        write(1,"\033[2k\r",5);
                        write(1,"myshell>",8);
                        while(i != cursor) {write(1,"\033[1D",5); i--;}
                }
        }
        return (char*)line.c_str();
}
