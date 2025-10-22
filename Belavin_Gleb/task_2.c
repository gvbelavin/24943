#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>  // для setenv() (если используется)

extern char *tzname[];

int main()
{
    time_t now;
    struct tm *sp;

    // Устанавливаем часовой пояс на тихоокеанское время (Калифорния)
    setenv("TZ", "America/Los_Angeles", 1);
    tzset();  // применяем новое значение TZ

    (void) time(&now);

    printf("%s", ctime(&now));

    sp = localtime(&now);
    printf("%d/%d/%02d %d:%02d %s\n",
        sp->tm_mon + 1, sp->tm_mday,
        sp->tm_year % 100,  // чтобы выводить 23 вместо 123
        sp->tm_hour,
        sp->tm_min,
        tzname[sp->tm_isdst]);

    exit(0);
}