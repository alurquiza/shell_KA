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

/*void execute(){
    pid_t pid = fork();
    if(pid == 0){
        int cap = execvp(args_aux[0], args_aux);
        if(cap < 0) printf(Rojo "Error \'%s\'\n" RESET, command);
    }
    else{
        wait(NULL);
    }
}*/

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
        else if(strcmp(command, "pwd") == 0){
            directory = getcwd(NULL, 0);
            printf(Cyan "%s\n" RESET, directory);
        }
        //Captar la entrada luego de separar en comando y rest que deberian ser los parametros y obviando lo que este despues de '#'
        char *args_aux[100];
        int tam = 1;
        char *cad_aux = malloc(100);
        int cnt = 0;
        args_aux[0] = command;
        int comillas = 0;
        for(int i = 0; i < size_rest; i++){
            if(rest[i] == ' ' && comillas == 0){
                if(cnt){
                    args_aux[tam] = cad_aux;
                    tam++;
                    cnt = 0;
                    cad_aux = malloc(100);
                }
                continue;
            }
            if(rest[i] == '"'){
                if(comillas == 1){
                    args_aux[tam] = cad_aux;
                    tam++;
                    cnt = 0;
                    cad_aux = malloc(100);
                    comillas = 0;
                }
                else comillas = 1;
                continue;                
            }
            cad_aux[cnt] = rest[i];
            cnt++;           
        }
        if(cnt){
            args_aux[tam] = cad_aux;
            tam++;
            cnt = 0;
            cad_aux = malloc(100);
        }
        args_aux[tam] = NULL;
        tam++;
        //Falta Revisar si tiene '|,>, <, >>'
        
        if(strcmp(command, "cd") == 0){
            chdir(args_aux[1]);
        }
        else if(size_command > 0){
            pid_t pid = fork();
            if(pid == 0){
                int cap = execvp(command, args_aux);
                if(cap < 0) printf(Rojo "Error to execute \'%s\'\n" RESET, command);
                return 0;
            }
            else{
                wait(NULL);
            }
        }
        
    }
    return 0;
}
