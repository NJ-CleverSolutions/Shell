#ifndef command_hh
#define command_hh
#include "simpleCommand.hh"
#include <string>
#include <vector>
extern int errorCode;
extern int lastID;
extern std::vector<std::vector<std::string>> historyTable;
struct Command {
        std::vector<SimpleCommand *> simpleCommands;
        std::string * outFile;
        std::string * inFile;
        std::string * errFile;
        bool append;
        bool background;
        int doubleGreaterThan;
        char ** path;

        Command();
        void insertSimpleCommand( SimpleCommand * simpleCommand );

        void clear();
        void print();
        void execute();

        static SimpleCommand *currentSimpleCommand;
};
