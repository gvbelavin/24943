#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Node {
    char *str;
    struct Node *next;
};

// Удаляем ESC-последовательности БЕЗ оставления пробелов
void remove_escape_sequences(char *str) 
{
    int i = 0, j = 0;
    while (str[i] != '\0') 
    {
        // Если нашли ESC символ (\x1b или 27)
        if ((unsigned char)str[i] == '\x1B') 
        {
            // Пропускаем ESC и всю последовательность
            i++; // пропускаем ESC
            // Пропускаем все символы до буквы (A,B,C,D для стрелок)
            while (str[i] != '\0' && str[i] != 'A' && str[i] != 'B' && 
                   str[i] != 'C' && str[i] != 'D' && str[i] != '~') 
                   {
                    i++;
                   }
            if (str[i] != '\0') i++; // пропускаем последний символ (A,B,C,D,~)
        } 
        else 
        {
            // Копируем обычный символ
            str[j] = str[i];
            j++;
            i++;
        }
    }
    str[j] = '\0';
}

int main() {
    struct Node *head = NULL;
    struct Node *current = NULL;
    char buffer[256];
    
    printf("Вводи строки (точка для завершения):\n");
    
    while (1) {
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) break;
    
        if (buffer[0] == '.') break;
        
        // Удаляем ESC-последовательности вместо простой проверки
        remove_escape_sequences(buffer);
        
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len-1] == '\n') {
            buffer[len-1] = '\0';
            len--;
        }
        
        // Пропускаем пустые строки
        if (len == 0) continue;
        
        char *str = malloc(len + 1);
        strcpy(str, buffer);
  
        struct Node *new_node = malloc(sizeof(struct Node));
        new_node->str = str;
        new_node->next = NULL;

        if (head == NULL) {
            head = new_node;
            current = new_node;
        } else {
            current->next = new_node;
            current = new_node;
        }
    }
    
    printf("\nСписок строк:\n");
    current = head;
    while (current != NULL) {
        printf("%s\n", current->str);
        current = current->next;
    }
    
    current = head;
    while (current != NULL) {
        struct Node *next = current->next;
        free(current->str);
        free(current);
        current = next;
    }
    
    return 0;
}