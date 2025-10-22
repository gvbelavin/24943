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
    char *line_start[MAX_LINES];
    int line_length[MAX_LINES];
    
    char input[256];
    int line_no;

    if (argc < 2) { 
        fprintf(stderr, "Usage: %s <file>\n", argv[0]); 
        return 1; 
    }
    
    fd = open(argv[1], O_RDONLY);
    if (fd == -1) { 
        perror(argv[1]); 
        return 1; 
    }

    file_size = lseek(fd, 0, SEEK_END);
    if (file_size == (off_t)-1) {
        perror("lseek");
        close(fd);
        return 1;
    }

    file_data = mmap(NULL, file_size, PROT_READ, MAP_SHARED, fd, 0);
    if (file_data == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    // Build line table correctly
    line_start[0] = file_data;
    line_length[0] = 0;
    line_count = 1;

    for (off_t i = 0; i < file_size && line_count < MAX_LINES; i++) {
        line_length[line_count - 1]++;
        
        if (file_data[i] == '\n') {
            // Save current line length and setup next line
            if (i + 1 < file_size && line_count < MAX_LINES) {
                line_start[line_count] = &file_data[i + 1];
                line_length[line_count] = 0;
                line_count++;
            }
        }
    }

    printf("File mapped successfully. Total lines: %d\n", line_count);

    // Interactive loop
    for (;;) {
        printf("Line number: ");
        fflush(stdout);
        
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        
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
            fprintf(stderr, "Bad Line Number. Available: 1-%d\n", line_count);
            continue;
        }

        // Output the line with proper formatting
        int idx = line_no - 1;
        write(1, line_start[idx], line_length[idx]);
        printf("\n");  // Add newline after each output
    }

    munmap(file_data, file_size);
    close(fd);
    return 0;
}
