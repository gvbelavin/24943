#include <sys/types.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#define MAX_LINES 500

int main(int argc, char **argv) {
    int fd, line_count = 0;
    off_t file_size;
    char *file_data;
    char *line_start[MAX_LINES];  // Указатели на начало строк
    int line_length[MAX_LINES];   // Длины строк
    
    char input[256];
    int line_no;

    // Проверка аргументов
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

    // Определение размера файла
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

    // Построение таблицы строк
    line_start[0] = file_data;  // Первая строка начинается с начала файла
    line_length[0] = 0;
    line_count = 1;

    for (off_t i = 0; i < file_size && line_count < MAX_LINES; i++) {
        if (file_data[i] == '\n') {
            // Сохраняем длину текущей строки (включая \n)
            line_length[line_count - 1] = i - (line_start[line_count - 1] - file_data) + 1;
            
            // Начало следующей строки (если не конец файла)
            if (i + 1 < file_size && line_count < MAX_LINES) {
                line_start[line_count] = &file_data[i + 1];
                line_length[line_count] = 0;
                line_count++;
            }
        }
    }

    // Обработка последней строки (если файл не заканчивается на \n)
    if (line_count > 0 && line_length[line_count - 1] == 0) {
        line_length[line_count - 1] = file_size - (line_start[line_count - 1] - file_data);
    }

    printf("File mapped to memory. Total lines: %d\n", line_count);

    // Интерактивный цикл
    for (;;) {
        printf("Line number: ");
        
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        
        // Преобразование ввода в число
        char *endptr;
        line_no = (int)strtol(input, &endptr, 10);
        
        if (*endptr != '\0' && *endptr != '\n') {
            printf("Invalid input. Please enter a number.\n");
            continue;
        }
        
        if (line_no == 0) {
            break;
        }
        
        if (line_no < 1 || line_no > line_count) {
            fprintf(stderr, "Bad Line Number. Available lines: 1-%d\n", line_count);
            continue;
        }

        // Вывод строки используя отображение в память
        int idx = line_no - 1;
        fwrite(line_start[idx], 1, line_length[idx], stdout);
    }

    // Освобождение ресурсов
    munmap(file_data, file_size);
    close(fd);
    return 0;
}
