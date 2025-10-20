#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

enum { MAX_LINES = 500, BUFSZ = 256 };

int main(int argc, char **argv) {
    off_t off[MAX_LINES];     // смещения начала строк
    int   len[MAX_LINES];     // длины строк (в байтах)
    int fd, i = 0, cur = 0, ask;
    char ch, buf[BUFSZ];

    if (argc < 2) { fprintf(stderr, "Usage: %s <file>\n", argv[0]); return 1; }
    fd = open(argv[1], O_RDONLY);
    if (fd == -1) { perror(argv[1]); return 1; }

    off[0] = 0;
    while (read(fd, &ch, 1) == 1) {
        if (ch == '\n') {
            if (i < MAX_LINES) {
                len[i++] = cur + 1;                         // включая '\n'
                if (i < MAX_LINES) off[i] = lseek(fd, 0L, 1); // начало след. строки
            }
            cur = 0;
        } else {
            ++cur;
        }
    }
    if (cur > 0 && i < MAX_LINES) len[i++] = cur;           // последняя строка без '\n'

    for (;;) {
        printf("Line number : ");
        if (scanf("%d", &ask) != 1 || ask <= 0) break;
        if (ask > i) { fprintf(stderr, "Bad Line Number\n"); continue; }

        if (lseek(fd, off[ask - 1], 0) == (off_t)-1) { perror("lseek"); continue; }
        int left = len[ask - 1];
        while (left > 0) {
            int chunk = left < BUFSZ ? left : BUFSZ;
            ssize_t r = read(fd, buf, chunk);
            if (r <= 0) { fprintf(stderr, "Bad Line Number\n"); break; }
            (void)write(1, buf, r);
            left -= (int)r;
        }
    }

    close(fd);
    return 0;
}
