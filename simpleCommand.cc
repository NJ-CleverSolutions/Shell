#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unistd.h>

#include <iostream>

#include "simpleCommand.hh"

SimpleCommand::SimpleCommand() {
        arguments = std::vector<std::string *>();
}

SimpleCommand::~SimpleCommand() {
        for (auto & arg : arguments) {
                delete arg;
        }
}

void SimpleCommand::insertArgument(std::string * argument) {
        arguments.push_back(argument);
}

void SimpleCommand::print() {
        if(isatty(0)) {
                for (auto & arg : arguments) {
                        std::cout << "\"" << *arg << "\" \t";
                }
                // effectively the same as printf("\n\n");
                std::cout << std::endl;
        }
}
