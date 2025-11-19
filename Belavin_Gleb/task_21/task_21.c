#include <stdio.h>
#include <signal.h>
#include <unistd.h>

volatile sig_atomic_t count = 0;

void sigint_handler(int s) {
    count++;
    write(1, "\a", 1);
}

void sigquit_handler(int s) {
    printf("\nСигналов: %d\n", count);
    _exit(0);
}

int main() {
    signal(SIGINT, sigint_handler);
    signal(SIGQUIT, sigquit_handler);
    printf("CTRL-C: beep, CTRL-\\: exit\n");
    while(1) pause();
}
