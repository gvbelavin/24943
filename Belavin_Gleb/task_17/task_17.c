#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>

#define MAX_COLS 40

static struct termios original_settings;

/* Восстановить оригинальные настройки терминала при выходе */
void restore_terminal(void)
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_settings);
}

int main(void)
{
    struct termios new_settings;
    int ch;
    char line[MAX_COLS + 1];
    int len = 0;       /* количество символов в буфере */
    int col = 0;       /* текущая колонка (0..39) */

    /* Получить текущие настройки терминала */
    tcgetattr(STDIN_FILENO, &original_settings);
    new_settings = original_settings;

    /* Сохранить восстановление при выходе */
    atexit(restore_terminal);

    /* Установить режим: без эхо, неканонический */
    new_settings.c_lflag &= ~(ECHO | ICANON);
    new_settings.c_cc[VMIN] = 1;   /* читать по одному символу */
    new_settings.c_cc[VTIME] = 0;  /* без таймаута */

    /* Получить управляющие коды из оригинальных настроек */
    int erase_char = original_settings.c_cc[VERASE];  /* Backspace, обычно 127 */
    int kill_char = original_settings.c_cc[VKILL];    /* Ctrl-U, обычно 21 */
    int eof_char = original_settings.c_cc[VEOF];      /* Ctrl-D, обычно 4 */

    int ctrl_w = 23;    /* Ctrl-W (не стандартный, определяем сами) */
    int bell = 7;       /* BEL - звуковой сигнал */

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &new_settings);

    memset(line, 0, sizeof(line));

    while (1) {
        unsigned char c;
        if (read(STDIN_FILENO, &c, 1) != 1)
            break;

        ch = (int)c;

        switch (ch) {
            case 4:  /* Ctrl-D (EOF) */
                if (len == 0) {
                    write(STDOUT_FILENO, "\n", 1);
                    goto end;  /* Выход из цикла */
                }
                break;

            case 21:  /* Ctrl-U (KILL) */
                while (len > 0) {
                    write(STDOUT_FILENO, "\b \b", 3);
                    len--;
                    if (col > 0) col--;
                }
                break;

            case 8:   /* BS (Backspace) */
            case 127: /* DEL */
                if (len > 0) {
                    write(STDOUT_FILENO, "\b \b", 3);
                    len--;
                    if (col > 0) col--;
                } else {
                    write(STDOUT_FILENO, (char[]){bell}, 1);
                }
                break;

            case 23:  /* Ctrl-W (удалить слово) */
                if (len == 0) {
                    write(STDOUT_FILENO, (char[]){bell}, 1);
                } else {
                    /* Удаляем пробелы в конце */
                    while (len > 0 && isspace((unsigned char)line[len - 1])) {
                        write(STDOUT_FILENO, "\b \b", 3);
                        len--;
                        if (col > 0) col--;
                    }
                    /* Удаляем символы слова */
                    while (len > 0 && !isspace((unsigned char)line[len - 1])) {
                        write(STDOUT_FILENO, "\b \b", 3);
                        len--;
                        if (col > 0) col--;
                    }
                }
                break;

            default:
                /* Печатаемые символы */
                if (isprint(ch) || ch == ' ' || ch == '\t') {
                    /* Ограничение 40 столбцов */
                    if (col >= MAX_COLS) {
                        write(STDOUT_FILENO, "\n", 1);
                        col = 0;
                        len = 0;
                        memset(line, 0, sizeof(line));
                    }

                    if (len < MAX_COLS) {
                        line[len++] = (char)ch;
                    }

                    write(STDOUT_FILENO, (char[]){(char)ch}, 1);
                    col++;
                } else {
                    /* Непечатаемые (не наши управляющие) -> звуковой сигнал */
                    write(STDOUT_FILENO, (char[]){bell}, 1);
                }
                break;
        }
    }

end:
    return 0;
}