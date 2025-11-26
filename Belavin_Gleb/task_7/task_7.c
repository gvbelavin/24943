#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/wait.h>

#define MAX_LINES   1000

typedef struct {
    off_t  offset;   // позиция начала строки в файле
    size_t length;   // длина строки (без '\n')
} line_info_t;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("fstat");
        close(fd);
        return 1;
    }

    if (st.st_size == 0) {
        printf("File is empty\n");
        close(fd);
        return 0;
    }

    // Отображаем файл в память
    char *data = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    line_info_t lines[MAX_LINES];
    int   line_count  = 0;
    off_t line_start  = 0;

    // Строим таблицу строк, проходя по отображённому файлу
    for (off_t i = 0; i < st.st_size; i++) {
        if (data[i] == '\n') {
            if (line_count < MAX_LINES) {
                lines[line_count].offset = line_start;
                lines[line_count].length = i - line_start;
                line_count++;
            }
            line_start = i + 1;
        }
    }

    // Последняя строка без '\n'
    if (line_start < st.st_size && line_count < MAX_LINES) {
        lines[line_count].offset = line_start;
        lines[line_count].length = st.st_size - line_start;
        line_count++;
    }

    // Таблица строк (для отладки)
    printf("\nLine table for file '%s':\n", argv[1]);
    printf("Line#\tOffset\tLength\n");
    printf("-----\t------\t------\n");
    for (int i = 0; i < line_count; i++) {
        printf("%d\t%ld\t%zu\n",
               i + 1,
               (long)lines[i].offset,
               lines[i].length);
    }
    printf("Total lines: %d\n\n", line_count);

    // Основной цикл запросов
    while (1) {
        char input[64];
        int  line_number;

        printf("Enter line number (0 to exit) [5 seconds timeout]: ");
        fflush(stdout);

        // Ожидание ввода с таймаутом 5 секунд
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds);

        struct timeval tv;
        tv.tv_sec  = 5;
        tv.tv_usec = 0;

        int ret = select(STDIN_FILENO + 1, &rfds, NULL, NULL, &tv);
        if (ret == -1) {
            perror("select");
            break;
        } else if (ret == 0) {
            // Таймаут — запускаем cat как дочерний процесс и выходим
            printf("\nTimeout (5 seconds). Running cat and exiting.\n\n");

            pid_t pid = fork();
            if (pid == -1) {
                perror("fork");
                break;
            } else if (pid == 0) {
                execlp("cat", "cat", argv[1], (char *)NULL);
                perror("execlp");
                _exit(1);
            } else {
                int status;
                waitpid(pid, &status, 0);
                break;
            }
        }

        // Ввод успели сделать — продолжаем как в предыдущей задаче
        if (!fgets(input, sizeof(input), stdin)) {
            printf("\nProcess_end (EOF)\n");
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        char *endptr;
        line_number = (int)strtol(input, &endptr, 10);

        if (input[0] == '\0' || endptr == input || *endptr != '\0') {
            printf("Invalid input. Please enter a number.\n");
            continue;
        }

        if (line_number == 0) {
            printf("Process_end\n");
            break;
        }

        if (line_number < 1 || line_number > line_count) {
            printf("Line number must be between 1 and %d\n", line_count);
            continue;
        }

        line_info_t *line = &lines[line_number - 1];

        // Печатаем строку напрямую из отображённой памяти
        printf("Line %d: %.*s\n",
               line_number,
               (int)line->length,
               data + line->offset);
    }

    munmap(data, st.st_size);
    close(fd);
    return 0;
}