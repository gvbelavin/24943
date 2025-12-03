#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <termios.h>
#include <unistd.h>

#define MAX_COLS 40

static struct termios original_settings;

void restore_terminal(void)
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_settings);
}

int main(void)
{
    struct termios new_settings;
    int ch;
    char line[MAX_COLS + 1];
    int len = 0;
    int col = 0;

    if (tcgetattr(STDIN_FILENO, &original_settings) == -1)
        return 1;

    new_settings = original_settings;

    if (atexit(restore_terminal) != 0)
        return 1;

    new_settings.c_lflag &= ~(ECHO | ICANON);
    new_settings.c_cc[VMIN]  = 1;
    new_settings.c_cc[VTIME] = 0;

    int bell = 7;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &new_settings) == -1)
        return 1;

    memset(line, 0, sizeof(line));

    while (1) {
        unsigned char c;

        if (read(STDIN_FILENO, &c, 1) != 1)
            break;

        ch = (int)c;

        if (ch == 4) {
            if (len == 0) {
                write(STDOUT_FILENO, "\n", 1);
                break;
            }
        } else if (ch == 21) {
            while (len > 0) {
                write(STDOUT_FILENO, "\b \b", 3);
                len--;
                if (col > 0)
                    col--;
            }
        } else if (ch == 8 || ch == 127) {
            if (len > 0) {
                write(STDOUT_FILENO, "\b \b", 3);
                len--;
                if (col > 0)
                    col--;
            } else {
                write(STDOUT_FILENO, (char[]){bell}, 1);
            }
        } else if (ch == 23) {
            if (len == 0) {
                write(STDOUT_FILENO, (char[]){bell}, 1);
            } else {
                while (len > 0 && isspace((unsigned char)line[len - 1])) {
                    write(STDOUT_FILENO, "\b \b", 3);
                    len--;
                    if (col > 0)
                        col--;
                }
                while (len > 0 && !isspace((unsigned char)line[len - 1])) {
                    write(STDOUT_FILENO, "\b \b", 3);
                    len--;
                    if (col > 0)
                        col--;
                }
            }
        } else {
            if (isprint(ch) || ch == ' ' || ch == '\t') {
                if (col >= MAX_COLS) {
                    write(STDOUT_FILENO, "\n", 1);
                    col = 0;
                    len = 0;
                    memset(line, 0, sizeof(line));
                }

                if (len < MAX_COLS)
                    line[len++] = (char)ch;

                write(STDOUT_FILENO, (char[]){(char)ch}, 1);
                col++;
            } else {
                write(STDOUT_FILENO, (char[]){bell}, 1);
            }
        }
    }

    return 0;
}