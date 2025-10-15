#include <stdio.h>
#include <time.h>

int main(void) {
    time_t now;
    struct tm tm_pst;

    /* Получаем текущее календарное время (в секундах с epoch, обычно UTC) */
    time(&now);

    /* Отнимаем 8 часов (8 * 3600 секунд) для получения PST (UTC-8) */
    now -= 8 * 3600;

    /* Преобразуем в структурированное время по UTC — т.е. "фиксированное" время PST */
    /* Используем gmtime, потому что мы уже скорректировали значение вручную */
    if (gmtime_r(&now, &tm_pst) == NULL) {
        perror("gmtime_r");
        return 1;
    }

    printf("%04d-%02d-%02d %02d:%02d:%02d PST (UTC-8)\n",
           tm_pst.tm_year + 1900,
           tm_pst.tm_mon + 1,
           tm_pst.tm_mday,
           tm_pst.tm_hour,
           tm_pst.tm_min,
           tm_pst.tm_sec);

    return 0;
}
