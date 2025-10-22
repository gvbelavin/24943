#include <stdio.h>
#include <string.h>
// #include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

long offsets[256];
int lnlens[256];
int lines = 0;
int buf;

int main(int argc, char *argv[])
{
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1)
    {
        perror("Error opening file.");
        exit(1);
    }
    offsets[0] = 0;

    int pos = 0;
    int lnlen = 0;
    while (read(fd, &buf, 1) > 0)
    {
        pos++;
        if (buf == '\n')
        {
            lnlens[lines] = lnlen;
            offsets[++lines] = pos;
            lnlen = 0;
        }
        else lnlen++;
    }
    lnlens[lines] = lnlen;
    lines++;

    /* Debug */
    // for (int i = 0; i < lines; i++)
    // {
    //     printf("%ld %d\n", offsets[i], lnlens[i]);
    // }
    
    int input;
    printf("Enter the line number [0-%d]: ", lines-1);
    scanf("%d", &input);

    if (input >= lines || input < 0)
    {
        fprintf(stderr, "Line index out of bounds.\n");
        return -1;
    }
    

    lseek(fd, offsets[input], SEEK_SET);

    char buff[256];
    int idx = 0;
    while (read(fd, &(buff[idx]), 1) > 0 && buff[idx] != '\n')
    {
        idx++;
    }
    buff[idx] = '\0';
    // for (int i = 0; read(fd, &(buff[i]), 1) > 0 && buff[i] != '\n'; i++);

    printf("%s\n", buff);
    
    return 0;
}
