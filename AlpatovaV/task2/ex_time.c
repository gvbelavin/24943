#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>

int main(void){
    time_t now;
    struct tm *sp;
    
    (void)time(&now);


    putenv("TZ=PST8");
    tzset();


    sp = localtime(&now);
    

    sp->tm_isdst = 0;
    
    printf("California Time (PST): ");
    printf("%d/%d/%d %02d:%02d %s\n",
           sp->tm_mon + 1,  
           sp->tm_mday,        
           sp->tm_year + 1900, 
           sp->tm_hour,     
           sp->tm_min, 
           tzname[0]);   
    
    return 0;
}
