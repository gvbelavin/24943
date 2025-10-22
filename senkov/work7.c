#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

enum { MAX_LINES = 500, BUFSZ = 256 };

int main(int argc, char **argv) {
    off_t file_size;
    char *file_data;
    char *line_start[MAX_LINES];  // Указатели на начало строк
    int line_len[MAX_LINES];      // Длины строк
    int fd, i = 0, cur_len = 0, ask;
    char input[BUFSZ];

    if (argc < 2) { 
        fprintf(stderr, "Usage: %s <file>\n", argv[0]); 
        return 1; 
    }
    
    // Открытие файла
    fd = open(argv[1], O_RDONLY);
    if (fd == -1) { 
        perror(argv[1]); 
        return 1; 
    }

    // Получение размера файла
    file_size = lseek(fd, 0, SEEK_END);
    if (file_size == (off_t)-1) {
        perror("lseek");
        close(fd);
        return 1;
    }

    // Отображение файла в память
    file_data = mmap(NULL, file_size, PROT_READ, MAP_SHARED, fd, 0);
    if (file_data == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    // Построение таблицы строк с использованием mmap
    line_start[0] = file_data;  // Первая строка начинается с начала файла
    
    for (off_t pos = 0; pos < file_size && i < MAX_LINES; pos++) {
        if (file_data[pos] == '\n') {
            // Сохраняем длину текущей строки (включая \n)
            line_len[i] = cur_len + 1;
            i++;
            
            // Устанавливаем начало следующей строки (если не конец файла)
            if (i < MAX_LINES && pos + 1 < file_size) {
                line_start[i] = &file_data[pos + 1];
            }
            cur_len = 0;
        } else {
            cur_len++;
        }
    }
    
    // Обработка последней строки (если файл не заканчивается на \n)
    if (cur_len > 0 && i < MAX_LINES) {
        line_len[i] = cur_len;
        i++;
    }

    int total_lines = i;

    // Интерактивный цикл
    for (;;) {
        printf("Line number: ");
        
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        
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

        // Вывод строки используя отображение в память
        // Вместо lseek + read + write используем прямое обращение к памяти
        int line_index = ask - 1;
        fwrite(line_start[line_index], 1, line_len[line_index], stdout);
    }

    // Освобождение ресурсов
    munmap(file_data, file_size);
    close(fd);
    return 0;
}
