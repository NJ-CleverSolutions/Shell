extern "C" void controlC(int num) {
        Shell::prompt();
}

void zombie(int num) {
        while(waitpid(-1,&state,WNOHANG)>0);
}

int main(int argc, char ** argv) {
        signal(SIGINT,controlC);
        signal(SIGCHLD,zombie);
        Shell::prompt();
}
