#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wordexp.h>

void open_test()
{
    char fullpath[200];
    strcpy(fullpath, getenv("HOME"));
    strcat(fullpath, "/restr");
    printf("%s\n", fullpath);
    FILE *restr_f = fopen(fullpath, "r");
    if (restr_f == NULL)
    {
        perror("Couldn't open file");
    }
    else
    {
        printf("Successfull open.\n");
        fclose(restr_f);
    }
}

int main(int argc, char const *argv[])
{
    printf("UID: %d, EUID: %d\n", getuid(), geteuid());
    open_test();
    setuid(getuid());
    printf("UID: %d, EUID: %d\n", getuid(), geteuid());
    open_test();
    
    return 0;
}
