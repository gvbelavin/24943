#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>

int main(void){
    time_t now;
    struct tm *sp;
    
    // Получаем текущее время
    (void)time(&now);

    // Устанавливаем временную зону Калифорнии с принудительным летним временем
    putenv("TZ=PST8PDT");
    tzset();

    // Преобразуем время
    sp = localtime(&now);
    
    // Принудительно устанавливаем летнее время
    sp->tm_isdst = 1;
    
    printf("California Time (PDT): ");
    printf("%d/%d/%d %02d:%02d %s\n",
           sp->tm_mon + 1,      // месяц (1-12)
           sp->tm_mday,         // день месяца
           sp->tm_year + 1900,  // год
           sp->tm_hour,         // часы
           sp->tm_min,          // минуты
           tzname[1]);          // принудительно используем летнее время (PDT)
    
    return 0;
}
