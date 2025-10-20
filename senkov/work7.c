#include <sys/types.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <signal.h>

#define BUFSIZE 10
#define MAX_LINES 500

static char *displ[MAX_LINES]; // Массив для хранения указателей на начало строк
static int line_ln[MAX_LINES]; // Массив для хранения длин строк

void timeout_handler(int sig) {
    write(1, displ[0], line_ln[0]);  // Можно также использовать entire file
    exit(0);
}

int main(int argc, char *argv[]) {
    int fd, line_no, i = 1, j = 0, fd1;
    char c, buf[BUFSIZE];
    off_t size;
    char *p;
    static char err_msg[32] = "Input file - ";

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file_name>\n", argv[0]);
        exit(1);
    }

    if ((fd = open(argv[1], O_RDONLY)) == -1) {
        perror(strcat(err_msg, argv[1]));
        exit(1);
    }

    size = lseek(fd, 0, SEEK_END);
    p = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED) {
        perror("mmap failed");
        exit(1);
    }
    
    displ[0] = p;
    for (int count = 0; count < size; count++) {
        if (*(p + count) == '\n') {
            line_ln[i++] = j;              // Длина строки
            displ[i] = p + count + 1;      // Смещение для следующей строки
            j = 0;
        } else {
            j++;
        }
    }

    signal(SIGALRM, timeout_handler); // Устанавливаем обработчик сигнала
    alarm(5); // 5 секунд на ввод номера строки

    while (1) {
        printf("Line number: ");
        if (scanf("%d", &line_no) != 1 || line_no <= 0) {
            break;  // Завершаем программу при вводе 0 или ошибке
        }

        if (line_no > 0 && line_no < MAX_LINES && displ[line_no] != NULL) {
            write(1, displ[line_no], line_ln[line_no]);
        } else {
            fprintf(stderr, "Bad Line Number\n");
        }
    }

    close(fd);
    return 0;
}