#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <limits.h>
#include <string.h>

extern char **environ;

int main(int argc, char *argv[]) {
    char options[] = ":ispuU:cC:dvV:";  /* valid options */
    int c;
    
    // Обрабатываем опции в обычном порядке (слева направо)
    while ((c = getopt(argc, argv, options)) != EOF) {
        switch (c) {
            // Печатает реальные и эффективные идентификаторы пользователя и группы
            case 'i': {
                printf("Real UID: %d, Effective UID: %d\n", getuid(), geteuid());
                printf("Real GID: %d, Effective GID: %d\n", getgid(), getegid());
                break;
            }

            // Процесс становится лидером группы
            case 's': {
                if (setpgid(0, 0) == -1) {
                    perror("setpgid failed");
                } else {
                    printf("Process became group leader\n");
                }
                break;
            }

            // Печатает идентификаторы процесса, процесса-родителя и группы процессов
            case 'p': {
                printf("Process ID: %d\n", getpid());
                printf("Parent Process ID: %d\n", getppid());
                printf("Process Group ID: %d\n", getpgrp());
                break;
            }

            // Печатает значение ulimit
            case 'u': {
                struct rlimit rlp;
                if (getrlimit(RLIMIT_FSIZE, &rlp) == -1) {
                    perror("getrlimit failed");
                } else {
                    printf("ulimit: %llu\n", (unsigned long long)rlp.rlim_cur);
                }
                break;
            }

            // Изменяет значение ulimit
            case 'U': {
                char *endptr;
                long new_ulimit = strtol(optarg, &endptr, 10);
                
                if (endptr == optarg || *endptr != '\0' || new_ulimit < 0) {
                    fprintf(stderr, "Invalid value for -U: %s\n", optarg);
                    break;
                }
                
                struct rlimit rlp;
                rlp.rlim_cur = new_ulimit;
                rlp.rlim_max = new_ulimit;
                
                if (setrlimit(RLIMIT_FSIZE, &rlp) == -1) {
                    perror("setrlimit failed");
                } else {
                    printf("ulimit set to: %ld\n", new_ulimit);
                }
                break;
            }

            // Печатает размер в байтах core-файла, который может быть создан
            case 'c': {
                struct rlimit rlp;
                if (getrlimit(RLIMIT_CORE, &rlp) == -1) {
                    perror("getrlimit failed");
                } else {
                    printf("Core file size limit: %llu bytes\n", 
                           (unsigned long long)rlp.rlim_cur);
                }
                break;
            }

            // Изменяет размер core-файла
            case 'C': {
                char *endptr;
                long new_core_limit = strtol(optarg, &endptr, 10);
                
                if (endptr == optarg || *endptr != '\0' || new_core_limit < 0) {
                    fprintf(stderr, "Invalid value for -C: %s\n", optarg);
                    break;
                }
                
                struct rlimit rlp;
                rlp.rlim_cur = new_core_limit;
                rlp.rlim_max = new_core_limit;
                
                if (setrlimit(RLIMIT_CORE, &rlp) == -1) {
                    perror("setrlimit failed");
                } else {
                    printf("Core file size limit set to: %ld bytes\n", new_core_limit);
                }
                break;
            }

            // Печатает текущую рабочую директорию
            case 'd': {
                char cwd[PATH_MAX];
                if (getcwd(cwd, sizeof(cwd)) != NULL) {
                    printf("Current directory: %s\n", cwd);
                } else {
                    perror("getcwd failed");
                }
                break;
            }

            // Распечатывает переменные среды и их значения
            case 'v': {
                for (char **env = environ; *env != NULL; env++) {
                    printf("%s\n", *env);
                }
                break;
            }

            // Вносит новую переменную в среду или изменяет значение существующей переменной
            case 'V': {
                char *name = optarg;
                char *value = strchr(optarg, '=');
                
                if (value == NULL || value == optarg) {
                    fprintf(stderr, "Invalid format for -V: %s (should be name=value)\n", optarg);
                    break;
                }
                
                *value = '\0';
                value++;
                
                if (setenv(name, value, 1) == -1) {
                    perror("setenv failed");
                } else {
                    printf("Environment variable set: %s=%s\n", name, value);
                }
                break;
            }

            case ':': {
                fprintf(stderr, "Option -%c requires an argument\n", optopt);
                break;
            }

            case '?': {
                fprintf(stderr, "Invalid option: -%c\n", optopt);
                break;
            }
        }
    }

    return 0;
}
