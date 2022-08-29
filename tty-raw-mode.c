#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <string.h>
void tty_raw_mode(void) {
        struct termios tty_attr;
        tcgetattr(0,&tty_attr);
        tty_attr.c_lflag &= (~(ICANON|ECHO));
        tty_attr.c_cc[VTIME] = 0;
        tty_attr.c_cc[VMIN] = 1;
        tcsetattr(0,TCSANOW,&tty_attr);
}