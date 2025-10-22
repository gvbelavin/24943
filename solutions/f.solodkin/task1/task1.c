#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>

// Глобальная переменная для хранения текущего ulimit
static long current_ulimit = -1;

int main(int argc, char *argv[]) {
    int opt;
    extern char *optarg;
    extern int optind, opterr, optopt;
    
    // Если нет аргументов
    if (argc == 1) {
        printf("No arguments provided. Use -h for help.\n");
        return 0;
    }
    
    // Обрабатываем опции
    while ((opt = getopt(argc, argv, "ispucdvU:C:V:h")) != -1) {
        switch (opt) {
            case 'i':
                printf("=== User and Group IDs ===\n");
                printf("Real UID: %d\n", getuid());
                printf("Effective UID: %d\n", geteuid());
                printf("Real GID: %d\n", getgid());
                printf("Effective GID: %d\n", getegid());
                break;
                
            case 's':
                printf("=== Process Group Leader ===\n");
                if (setpgid(0, 0) == 0) {
                    printf("Became process group leader\n");
                    printf("New process group: %d\n", getpgrp());
                } else {
                    perror("setpgid failed");
                }
                break;
                
            case 'p':
                printf("=== Process IDs ===\n");
                printf("PID: %d\n", getpid());
                printf("Parent PID: %d\n", getppid());
                printf("Process group: %d\n", getpgrp());
                break;
                
            case 'u': {
                printf("=== Ulimit ===\n");
                if (current_ulimit != -1) {
                    // Показываем значение, которое было установлено через -U
                    printf("Ulimit (file size): %ld\n", current_ulimit);
                } else {
                    // Или получаем текущее значение из системы
                    struct rlimit rlim;
                    if (getrlimit(RLIMIT_FSIZE, &rlim) == 0) {
                        printf("Ulimit (file size): %ld\n", (long)rlim.rlim_cur);
                        current_ulimit = (long)rlim.rlim_cur;
                    } else {
                        perror("getrlimit failed");
                    }
                }
                break;
            }
            
            case 'U': {
                printf("=== Change Ulimit ===\n");
                long new_limit = atol(optarg);
                
                if (new_limit < 0) {
                    printf("Error: Invalid ulimit value: %s\n", optarg);
                    break;
                }
                
                struct rlimit rlim;
                if (getrlimit(RLIMIT_FSIZE, &rlim) == 0) {
                    rlim.rlim_cur = (rlim_t)new_limit;
                    if (setrlimit(RLIMIT_FSIZE, &rlim) == 0) {
                        // Сохраняем новое значение в глобальной переменной
                        current_ulimit = new_limit;
                        printf("Ulimit changed to: %ld\n", new_limit);
                    } else {
                        perror("setrlimit failed");
                    }
                } else {
                    perror("getrlimit failed");
                }
                break;
            }
            
            case 'c': {
                printf("=== Core File Size ===\n");
                struct rlimit rlim;
                if (getrlimit(RLIMIT_CORE, &rlim) == 0) {
                    printf("Core file size: %ld bytes\n", (long)rlim.rlim_cur);
                } else {
                    perror("getrlimit core failed");
                }
                break;
            }
            
            case 'C': {
                printf("=== Change Core File Size ===\n");
                struct rlimit rlim;
                long new_size = atol(optarg);
                
                if (new_size < 0) {
                    printf("Error: Invalid core size: %s\n", optarg);
                    break;
                }
                
                if (getrlimit(RLIMIT_CORE, &rlim) == 0) {
                    rlim.rlim_cur = (rlim_t)new_size;
                    if (setrlimit(RLIMIT_CORE, &rlim) == 0) {
                        printf("Core file size changed to: %ld bytes\n", new_size);
                    } else {
                        perror("setrlimit core failed");
                    }
                } else {
                    perror("getrlimit core failed");
                }
                break;
            }
            
            case 'd': {
                printf("=== Current Directory ===\n");
                char cwd[1024];
                if (getcwd(cwd, sizeof(cwd))) {
                    printf("Current directory: %s\n", cwd);
                } else {
                    perror("getcwd failed");
                }
                break;
            }
            
            case 'v':
                printf("=== Environment Variables ===\n");
                extern char **environ;
                for (char **env = environ; *env; env++) {
                    printf("%s\n", *env);
                }
                break;
            
            case 'V':
                printf("=== Set Environment Variable ===\n");
                if (putenv(optarg) == 0) {
                    printf("Environment variable set: %s\n", optarg);
                } else {
                    perror("putenv failed");
                }
                break;
            
            case 'h':
                printf("=== Help ===\n");
                printf("Usage: %s [options]\n", argv[0]);
                printf("Options:\n");
                printf("  -i  Print user and group IDs\n");
                printf("  -s  Become process group leader\n");
                printf("  -p  Print process IDs\n");
                printf("  -u  Print ulimit\n");
                printf("  -U <value> Change ulimit\n");
                printf("  -c  Print core file size\n");
                printf("  -C <size> Change core file size\n");
                printf("  -d  Print current directory\n");
                printf("  -v  Print environment variables\n");
                printf("  -V <name=value> Set environment variable\n");
                printf("  -h  This help message\n");
                break;
            
            case '?':
                fprintf(stderr, "Error: Unknown option: -%c\n", optopt);
                fprintf(stderr, "Use -h for help\n");
                break;
        }
    }
    
    return 0;
}