#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Node {
    char *str;
    struct Node *next;
};

int main() {
    struct Node *head = NULL;
    struct Node *current = NULL;
    char buffer[256];
    
    printf("Вводи строки (точка для завершения):\n");
    
    while (1) {
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) break;
    
        if (buffer[0] == '.') break;
        
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len-1] == '\n') {
            buffer[len-1] = '\0';
            len--;
        }
        
        char *str = malloc(len + 1);
        strcpy(str, buffer);
  
        struct Node *new_node = malloc(sizeof(struct Node));
        new_node->str = str;
        new_node->next = NULL;

        if (head == NULL) {
            head = new_node;
        } else {
            current->next = new_node;
        }
        current = new_node;
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