#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "command.hh"
#include "shell.hh"

int lineLength; int tempLength; char ch; char ch1; char ch2;
int cursor = 0; int index1 = 0; int index2 = 0; int tempCommand = 0;
std::vector<std::string> history = std::vector<std::string>();
std::string temp = std::string();
std::string line = std::string();

extern "C" void tty_raw_mode(void);

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
        tty_raw_mode();
        lineLength = 0;
        tempLength = 0;
        while(true) {
                read(0, &ch, 1);
                if(ch == 1) { //Home
                                write(1,"\r",5);
                                write(1,"\033[8C",5);
                                cursor = 0;
                }else if(ch == 4) { //Delete
                        if(lineLength > 0) {
                                write(1,"\033[2K\r",5);
                                write(1,"myshell>",8);
                                line.erase(cursor,1);
                                write(1,(char*)line.c_str(),line.size());
                                tempLength = cursor;
                                cursor = line.size();
                                while(cursor != tempLength) {
                                        write(1,"\033[1D",5);
                                        cursor--;
                                }
                                lineLength--;
                        }
                }else if(ch == 5) { //End of Line
                        write(1,"\r",5);
                        write(1,"\033[8C",5);
                        cursor = 0;
                        while(cursor < lineLength) {
                                write(1,"\033[1C",5);
                                cursor++;
                        }
                }else if (ch == 8 || ch == 127) { //Backspace
                        if(lineLength > 0) {
                                write(1,"\033[2K\r",5);
                                write(1,"myshell>",8);
                                line.erase(cursor-1,1);
                                write(1,(char*)line.c_str(),line.size());
                                tempLength = cursor-1;
                                cursor = line.size();
                                while(cursor != tempLength) {
                                        write(1,"\033[1D",5);
                                        cursor--;
                                }
                                lineLength--;
                        }
                }else if(ch == 10) { //Enter
                        if(lineLength != 0) {
                                history.push_back(line);
                                index2 = history.size()-1;
                                index1++;
                                tempCommand = 0; lineLength = 0; cursor = 0;
                                write(1,&ch,1);
                                break;
                        }else{
                                line.clear();
                                tempCommand = 0; lineLength = 0; cursor = 0;
                                write(1,&ch,1);
                                write(1,"myshell>",8);
                                break;
                        }
                }else if (ch==27) { // arrow keys
                        read(0, &ch1, 1); read(0, &ch2, 1);
                        if ((ch1==91 && ch2==65) && history.size() != 0) { // up arrow
                                write(1,"\033[2K\r",5);
                                write(1,"myshell>",8);
                                line.clear();
                                if(index2 < 0 ) {
                                        index2 = 0;
                                        line = history[index2];
                                        lineLength = line.size();
                                        write(1,(char*)line.c_str(),lineLength);
                                }else{
                                        line = history[index2];
                                        lineLength = line.size();
                                        write(1,(char*)line.c_str(),lineLength);
                                        index2--;
                                }
                                cursor = line.size();
                        }else if((ch1 == 91 && ch2 == 66) && history.size() != 0) { //down arrow
                                write(1,"\033[2K\r",5);
                                write(1,"myshell>",8);
                                line.clear();
                                if(index2+1 == index1) {
                                        lineLength = line.size();
                                        write(1,(char*)line.c_str(),lineLength);
                                }else{
                                        line = history[index2+1]; lineLength = line.size();
                                        write(1,(char*)line.c_str(),lineLength);
                                        index2++;
                                }
                                cursor = line.size();
                        }else if((ch1 == 91 && ch2 == 68) && lineLength != 0) { // left
                                if(cursor > 0){
                                        write(1,"\033[1D",5);
                                        cursor--;
                                }
                        }else if((ch1 == 91 && ch2 == 67) && lineLength != 0) { // right
                                if(cursor < lineLength) {
                                        write(1,"\033[1C",5);
                                        cursor++;
                                }
                        }
                }else if (ch == 31) { // ctrl-?
                        read_line_print_usage();
                        line.clear();
                        Shell::prompt();
                        break;
                }else if(ch >= 32 && ch < 127) { //regular characters
                        if(lineLength == 0) {line.clear();}
                        if(cursor == lineLength) {
                                line.push_back(ch);
                                lineLength++; cursor++;
                                write(1,&ch,1);
                        }else if(cursor > 0 && cursor < lineLength) {
                                temp.clear();
                                for(int i = 0; i < cursor; i++) {
                                        temp.push_back(line[i]);
                                }
                                temp.push_back(ch);
                                for(int i = cursor; i < lineLength; i++) {
                                        temp.push_back(line[i]);
                                }
                                line.clear(); line = temp; temp.clear();
                                lineLength++; cursor++;
                                write(1,"\033[2k\r",5);
                                write(1,"myshell>",8);
                                write(1,(char*)line.c_str(),line.size());
                                int i = lineLength; while(i != cursor) {write(1,"\033[1D",5); i--;}
                        }else{
                                temp.clear();
                                temp.push_back(ch);
                                for(int i = 0; i < lineLength; i++) {
                                        temp.push_back(line[i]);
                                }
                                line.clear(); line = temp; temp.clear();
                                lineLength++; cursor++;
                                write(1,"\033[2k\r",5);
                                write(1,"myshell>",8);
                                write(1,(char*)line.c_str(),line.size());
                                int i = lineLength; while(i != cursor) {write(1,"\033[1D",5); i--;}
                        }
                }
        }
        line.push_back(10);
        line.push_back(0);
        return (char*)line.c_str();
}
