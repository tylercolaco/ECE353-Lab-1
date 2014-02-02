/*
 * main.c: A simple shell
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#define BUF_SIZE 512
#define PROMPT "ece353sh$"
#define PROMPT_SIZE ARGUMENTS*512
#define ARGUMENTS 20
#define MAX_HISTORY_SIZE 100
// Look up Access*()
//realpath()

extern int errno;

int bound_check;
int history_size = 0;
int path_size = 0;

struct node {
    char * data;        // data held by node
    struct node * next; // pointer to the next node
};

struct node * path;     // pointer to the path
struct node * history;

void pathadd(char ** args);
void pathremove(char ** args);
void pathprint(char ** args);

void createhistory(char * buf){
    
    if (history == NULL) {
        history = malloc(sizeof(struct node));
        history->data = malloc(strlen(buf) + 1);
        strcpy(history->data, buf);
        history->next = NULL;
        history_size++;
    }
    else {
        struct node * new_entry;
        new_entry = malloc(sizeof(struct node));
        new_entry->data = malloc(strlen(buf) + 1);
        strcpy(new_entry->data, buf);
        new_entry->next = NULL;
        struct node * current = history;
        
        while (current->next != NULL) current = current->next;
        current->next = new_entry;
        history_size++;
        
        if (history_size > MAX_HISTORY_SIZE) {
            struct node * temp = history;
            history = history->next;
            free(temp);
        }
    }
}

void pathadd(char ** args) {
    if (path == NULL) {
        path = malloc(sizeof(struct node));
        path->data = malloc(strlen(args[2]) + 1);
        strcpy(path->data, args[2]);
        path->next = NULL;
        path_size++;
    }
    else {
        struct node * new_path;
        new_path = malloc(sizeof(struct node));
        new_path->data = malloc(strlen(args[2]) + 1);
        strcpy(new_path->data, args[2]);
        new_path->next = NULL;
        
        struct node * current_path = path;
        
        while (current_path->next != NULL) current_path = current_path->next;
        current_path->next = new_path;
        path_size++;
    }
}

void pathremove(char ** args){
    
    if (path != NULL) {
        struct node * previous_node;
        struct node * current_node = path;
        if (strcmp(args[2], current_node->data) == 0) {
            path = current_node->next;
            free(current_node);
        }
        else {
            while ((current_node != NULL) && (strcmp(args[2], current_node->data) != 0)) {
                previous_node = current_node;
                current_node = current_node->next;
                if (strcmp(args[2], current_node->data) == 0) {
                    previous_node->next = current_node->next;
                    free(current_node);
                    break;
                }
            }
        }
    }
}

void pathprint(char ** args){
    struct node * temp = path;
    while (temp != NULL) {
        printf("%s", temp->data);
        if (temp->next != NULL) {
            printf(":");
            temp = temp->next;
        }
        else break;
    }
    printf("\n");
}

char *getinput(char * buf, int len) {
    
    printf("%s ", PROMPT);
    return fgets(buf, len, stdin);
}

int parseinput(const char * buf, char ** args) {
    
    bound_check = 1;
    if (strlen(buf) > PROMPT_SIZE) printf("Prompt too long!\n");
    else if (strcmp(buf, "exit\n") == 0) return 0;
    else {
        char * input = strtok ((char *) buf, "\n");
        if (strncmp(input, "!xxx", 1) == 0) {
            char histcommand[strlen(input)];
            strcpy(histcommand, &input[1]);
            int number = strtol (histcommand, NULL, 10);
            struct node * temp;
            temp = history;
            if (number > history_size || number > MAX_HISTORY_SIZE){
                bound_check = 0;
                printf("Command not found.\n");
            }
            else {
                while (number != 1) {
                    temp = temp->next;
                    number--;
                }
                strcpy(input,temp->data);
            }
        }
        if (bound_check) createhistory(input);
        char * token;
        if (args == NULL) {
            printf("Failed to allocate memory!");
            free(args);
            return 1;
        }
        int i = 0;
        token = strtok (input, " ");
        while (token != NULL) {
            if (i < (ARGUMENTS + 1)) {
                args[i] = token;
                token = strtok (NULL, " ");
                i++;
            }
            else break;
        }
        args[i] = NULL;
    }
    return 1;
}

int executecommand(char ** args) {
    
    if (strcmp(args[0], "cd") == 0) {
        if (args[1] != NULL) {
            chdir (args[1]);
            if (errno != 0) {
                printf("error: %s\n",strerror(errno));
                errno = 0;
            }
        }
        return 1;
    }
    
    if (strcmp(args[0], "history") == 0){
        int i = 1;
        struct node * temp = history;
        while (temp != NULL) {
            printf("[%d] %s\n", i, temp->data);
            temp = temp->next;
            i++;
        }
        return 1;
    }
    
    if (strcmp(args[0], "path") == 0) {
        
        if (args[1] == NULL) pathprint(args);
        else if (strcmp(args[1], "+") == 0) pathadd(args);
        else if (strcmp(args[1], "-") == 0) pathremove(args);
        else pathprint(args);
        return 1;
    }
    
    int pid = fork();
    
    if (pid == 0) execvp(args[0], args);
    
    else {
        int status;
        waitpid(pid, &status, 0);
    }
    return 1;
}

int main(int argc, char *argv[]) {
    
    char buf[BUF_SIZE];
    char ** args;
    
    args = malloc((ARGUMENTS + 2) * sizeof(char));
    
    getinput(buf, sizeof(buf));
    
    while (parseinput(buf, args) == 1) {
        executecommand(args);
        if ((errno != 0) && (bound_check == 1)) {
            printf("error: %s\n",strerror(errno));
            errno = 0;
            break;
        }
        getinput(buf, sizeof(buf));
    }
    
    free(args);
    return 0;
}