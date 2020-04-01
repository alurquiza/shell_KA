#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#define Rojo "\x1b[31m"
#define Yellow "\x1b[33m"
#define Blue "\x1b[34m"
#define Cyan "\x1b[36m"
#define RESET "\x1b[0m"

int main(){

    char *directory = malloc(sizeof(char) * 100);
    directory = getcwd(NULL, 0);
    while(1){
        printf(Yellow "my-prompt $ " RESET);
        char *rd = malloc(sizeof(char) * 100);
        char *command = malloc(sizeof(char) * 100);
        char *rest = malloc(sizeof(char) * 100);
        fgets(rd, 100, stdin);
        int size_command = 0, size_rest = 0;
        int t = 0;
        for(int i = 0; i < 100; i++){
            if(rd[i] == ' ' || rd[i] == '\n' || rd[i] == '#'){
                t = i + 1;   
                break;         
            }
            command[i] = rd[i];
            size_command++;
        }
        if(rd[t - 1] == ' '){
            for(int i = t; i < 100; i++){
                if(rd[i] == '\n' || rd[i] == '#') break;
                rest[i - t] = rd[i];
                size_rest++;
            }
        }
        if(strcmp(command, "exit") == 0)
            return 0;
        if(strcmp(command, "ls") == 0){
            pid_t pid = fork();
            if(pid == 0){
                char *args[] = {"ls", NULL};
                execvp("ls", args);
            }
            else{
                wait();
            }
            /* Otra forma de hacer ls
            /media/ubuntu-mate/Universidad/2do Year/2do semestre/Sistema Operativo
            DIR *dir;
            dir = opendir(directory);
            struct dirent *ent;
            while((ent = readdir(dir)) != NULL){
                if(strcmp(ent->d_name, ".") && strcmp(ent->d_name, ".."))
                    printf(Blue "%s " RESET, ent->d_name);
            }
            printf("\n");
            */
        }
        else if(strcmp(command, "pwd") == 0){
            directory = getcwd(NULL, 0);
            printf(Cyan "%s\n" RESET, directory);
        }
        else if(strcmp(command, "cd") == 0){
            char *path = malloc(100);
            int tam = 0;
            for(int i = 0; i < size_rest; i++){
                if(rest[i] == '"') continue;
                path[tam] = rest[i];
                tam++;
            }
            chdir(path);
        }
        else if(size_command > 0){
            printf(Rojo "Command \'%s\' not found\n" RESET, command);    
        }
    }
    return 0;
}
