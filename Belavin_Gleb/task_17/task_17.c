#include <ncurses.h>
#include <ctype.h>
#include <string.h>

#define MAX_COLS 40

int main(void)
{
    int ch;
    char line[MAX_COLS + 1];
    int len = 0;
    int col = 0;

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);   // ВАЖНО: чтобы getch() возвращал KEY_BACKSPACE

    int ctrl_d   = 4;
    int ctrl_w   = 23;
    int bell     = 7;
    int erase1   = 127;     // DEL
    int erase2   = 8;       // BS
    int kill_char = 21;     // Ctrl-U

    memset(line, 0, sizeof(line));

    while (1) {
        ch = getch();

        if (ch == ctrl_d && len == 0) {
            addch('\n');
            refresh();
            break;
        }

        /* KILL (^U) */
        if (ch == kill_char) {
            while (len > 0) {
                move(getcury(stdscr), getcurx(stdscr) - 1);
                delch();
                len--;
                if (col > 0) col--;
            }
            refresh();
            continue;
        }

        /* ERASE: Backspace/Delete/KEY_BACKSPACE */
        if (ch == erase1 || ch == erase2 || ch == KEY_BACKSPACE) {
            if (len > 0) {
                move(getcury(stdscr), getcurx(stdscr) - 1);
                delch();
                len--;
                if (col > 0) col--;
            } else {
                addch(bell);
            }
            refresh();
            continue;
        }

        /* CTRL-W */
        if (ch == ctrl_w) {
            if (len == 0) {
                addch(bell);
                refresh();
                continue;
            }

            while (len > 0 && isspace((unsigned char)line[len - 1])) {
                move(getcury(stdscr), getcurx(stdscr) - 1);
                delch();
                len--;
                if (col > 0) col--;
            }

            while (len > 0 && !isspace((unsigned char)line[len - 1])) {
                move(getcury(stdscr), getcurx(stdscr) - 1);
                delch();
                len--;
                if (col > 0) col--;
            }

            refresh();
            continue;
        }

        /* Печатаемые */
        if (isprint(ch) || ch == ' ' || ch == '\t') {
            if (col >= MAX_COLS) {
                addch('\n');
                col = 0;
                len = 0;
                memset(line, 0, sizeof(line));
            }

            if (len < MAX_COLS) {
                line[len++] = (char)ch;
            }
            addch(ch);
            col++;
            refresh();
            continue;
        }

        /* Остальные непечатаемые -> BEL */
        if (ch != ctrl_d && ch != ctrl_w && ch != kill_char) {
            addch(bell);
            refresh();
        }
    }

    endwin();
    return 0;
}