#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv) {
    off_t off[501];
    int len[501];
    char buf[257], ch;
    int fd, i = 0, n = 0, ask;

    if (argc < 2) { fprintf(stderr,"Usage: %s <file>\n", argv[0]); return 1; }
    if ((fd = open(argv[1], O_RDONLY)) == -1) { perror(argv[1]); return 1; }

    off[0] = 0;
    while (read(fd, &ch, 1) == 1) {
        if (ch == '\n') {
            len[i++] = n + 1;
            off[i] = lseek(fd, 0, SEEK_CUR);
            n = 0;
            if (i == 500) break;
        } else {
            n++;
        }
    }
    if (n > 0 && i < 500) len[i++] = n;

    int max = i;
    while (printf("Line number : "), scanf("%d", &ask) == 1) {
        if (ask <= 0) break;
        if (ask > max) { fprintf(stderr, "Bad Line Number\n"); continue; }
        if (lseek(fd, off[ask - 1], SEEK_SET) < 0) { perror("lseek"); continue; }
        int want = len[ask - 1]; if (want > 256) want = 256;
        ssize_t r = read(fd, buf, want);
        if (r > 0) write(1, buf, r); else fprintf(stderr, "Bad Line Number\n");
    }

    close(fd);
    return 0;
}