// This code is not complete and has been manipulated to align to Purdue University's Academic Integrity Policy. Any use or distribution of this code is considered
   and should be reported as academic dishonesty.

all: shell

command.o: command.cc command.hh
        g++ -g -std=c++17 -Wall -Wextra -pedantic -c command.cc

simpleCommand.o: simpleCommand.cc simpleCommand.hh
        g++ -g -std=c++17 -Wall -Wextra -pedantic -c simpleCommand.cc

shell.o: shell.cc shell.hh
        g++ -g -std=c++17 -Wall -Wextra -pedantic -c shell.cc

shell: shell.o command.o simpleCommand.o read-line.o
                g++ -g -std=c++17 -Wall -Wextra -pedantic -o shell shell.o command.o simpleCommand.o $(EDIT_MODE_OBJECTS)

read-line.o: read-line.c
        g++ -g -std=c++17 -Wall -Wextra -pedantic -c read-line.c

clean:
        rm -f test-shell/out test-shell/out2
        rm -f test-shell/sh-in test-shell/sh-out
        rm -f test-shell/shell-in test-shell/shell-out
        rm -f test-shell/err1 test-shell/file-list
