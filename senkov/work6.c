#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

enum { MAX_LINES = 500, BUFSZ = 256 };

int fd;  // Глобальная переменная для доступа из обработчика сигнала

// Обработчик сигнала SIGALRM - печатает весь файл
void timeout_handler(int sig) {
    printf("\nTime's up! Printing entire file...\n");
    
    char buf[BUFSZ];
    ssize_t bytes_read;
    
    // Перемещаемся к началу файла
    if (lseek(fd, 0, SEEK_SET) == (off_t)-1) {
        perror("lseek");
        close(fd);
        exit(1);
    }
    
    // Читаем и выводим весь файл
    while ((bytes_read = read(fd, buf, sizeof(buf))) > 0) {
        if (write(1, buf, bytes_read) != bytes_read) {
            perror("write");
            break;
        }
    }
    
    if (bytes_read == -1) {
        perror("read");
    }
    
    close(fd);
    exit(0);
}

int main(int argc, char **argv) {
    off_t off[MAX_LINES];
    int   len[MAX_LINES];
    int i = 0, cur = 0, ask;
    char ch, buf[BUFSZ];
    char input[BUFSZ];

    if (argc < 2) { 
        fprintf(stderr, "Usage: %s <file>\n", argv[0]); 
        return 1; 
    }
    
    fd = open(argv[1], O_RDONLY);
    if (fd == -1) { 
        perror(argv[1]); 
        return 1; 
    }

    // Установка обработчика сигнала SIGALRM
    if (signal(SIGALRM, timeout_handler) == SIG_ERR) {
        perror("signal");
        close(fd);
        return 1;
    }

    off[0] = 0;
    while (read(fd, &ch, 1) == 1) {
        if (i >= MAX_LINES) {
            fprintf(stderr, "Warning: too many lines, truncating to %d\n", MAX_LINES);
            break;
        }
        
        if (ch == '\n') {
            len[i++] = cur + 1;
            if (i < MAX_LINES) {
                off[i] = lseek(fd, 0L, SEEK_CUR);
            }
            cur = 0;
        } else {
            ++cur;
        }
    }
    
    if (cur > 0 && i < MAX_LINES) {
        len[i++] = cur;
    }

    int total_lines = i;

    for (;;) {
        printf("Line number: ");
        fflush(stdout);  // Сбросить буфер вывода перед установкой alarm
        
        // Установить таймер на 5 секунд
        alarm(5);
        
        if (fgets(input, sizeof(input), stdin) == NULL) {
            alarm(0);  // Отключить таймер при выходе
            break;
        }
        
        // Отключить таймер после успешного ввода
        alarm(0);
        
        input[strcspn(input, "\n")] = '\0';
        
        if (input[0] == '\0') {
            continue;
        }
        
        char *endptr;
        ask = (int)strtol(input, &endptr, 10);
        
        if (*endptr != '\0') {
            printf("Invalid input. Please enter a number.\n");
            continue;
        }
        
        if (ask == 0) {
            break;
        }
        
        if (ask < 1 || ask > total_lines) {
            fprintf(stderr, "Bad Line Number. Available lines: 1-%d\n", total_lines);
            continue;
        }

        if (lseek(fd, off[ask - 1], SEEK_SET) == (off_t)-1) {
            perror("lseek");
            continue;
        }
        
        int left = len[ask - 1];
        while (left > 0) {
            int chunk = left < BUFSZ ? left : BUFSZ;
            ssize_t r = read(fd, buf, chunk);
            if (r <= 0) {
                fprintf(stderr, "Error reading line\n");
                break;
            }
            if (write(1, buf, r) != r) {
                perror("write");
                break;
            }
            left -= (int)r;
        }
    }

    // Отключить таймер перед нормальным выходом
    alarm(0);
    close(fd);
    return 0;
}
