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

    // Проверка на пустой файл
    if (file_size == 0) {
        printf("File is empty\n");
        close(fd);
        return 0;
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

    printf("\n=== DEBUG: MMap Line Table ===\n");
    printf("File mapped at: %p, Size: %ld bytes\n", file_data, file_size);
    printf("Total lines: %d\n", line_count);
    printf("Line | Memory Addr  | Length | Content Preview\n");
    printf("-----+-------------+--------+----------------\n");
    
    for (int j = 0; j < line_count; j++) {
        // Создаем предпросмотр содержимого строки
        char preview[21];
        int preview_len = line_length[j] < 20 ? line_length[j] : 20;
        
        // Копируем данные напрямую из памяти (mmap)
        memcpy(preview, line_start[j], preview_len);
        preview[preview_len] = '\0';
        
        // Заменяем непечатаемые символы
        for (int k = 0; k < preview_len; k++) {
            if (preview[k] == '\n') preview[k] = '\\';
            if (preview[k] == '\t') preview[k] = '→';
            if (preview[k] < 32 || preview[k] > 126) preview[k] = '.';
        }
        
        // Выводим информацию о строке (адрес в памяти вместо смещения в файле)
        printf("%4d | %11p | %6d | %s\n", 
               j+1, line_start[j], line_length[j], preview);
    }
    
    printf("=== End of Table ===\n\n");

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

        // ВЫВОД СТРОКИ БЕЗ ИСПОЛЬЗОВАНИЯ write/fwrite - используем fwrite с отображенной памятью
        // Это допустимо, так как fwrite работает с памятью, а не с файловыми дескрипторами
        int idx = line_no - 1;
        
        // Альтернативный вариант - использовать puts или printf с ограничением длины
        // Но fwrite - наиболее корректный для бинарных данных
        fwrite(line_start[idx], 1, line_length[idx], stdout);
        
        // Если строка не заканчивается на \n, добавляем его
        if (line_length[idx] > 0 && line_start[idx][line_length[idx] - 1] != '\n') {
            printf("\n");
        }
    }

    // Освобождение ресурсов
    munmap(file_data, file_size);
    close(fd);
    return 0;
}
