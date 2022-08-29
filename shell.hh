#ifndef shell_hh
#define shell_hh
#include "command.hh"

struct Shell {
        std::vector<SimpleCommand *> historyTable = std::vector<SimpleCommand *>();
        static void prompt();
        static Command currentCommand;
};
