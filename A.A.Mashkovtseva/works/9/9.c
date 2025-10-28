#include <stdio.h>
#include <unistd.h>

int main() {
    if (fork() == 0) {
        execl("/bin/cat", "cat", "file.txt", NULL);
    }

    wait(NULL);
    printf("CAT finished!\n");
    return 0;
}