#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#define Rojo "\x1b[31m"
#define Yellow "\x1b[33m"
#define Blue "\x1b[34m"
#define Cyan "\x1b[36m"
#define RESET "\x1b[0m"

typedef struct Commands{
    char *args[100];
    int count_arg;
    int mod1, mod2, mod3; //mod1 = 1 cuando existe >, mod2 = 1 para >>, mod3 = 1 para <
    char *input, *output;
}Commands;

void initial(Commands *command){
    command->count_arg = 0;
    command->mod1 = command->mod2 = command->mod3 = 0;
    command->input = malloc(100);
    command->output = malloc(100);
}

void Parse_to_command(char **tokens, int count_tokens, Commands *command){
    for(int i = 0; i < count_tokens; i++){
        if(strcmp(tokens[i], ">") == 0){
            command->mod1 = 1;
            command->output = tokens[i + 1];
            i++;
        }
        else if(strcmp(tokens[i], ">>") == 0){
            command->mod2 = 1;
            command->output = tokens[i + 1];
            i++;
        }
        else if(strcmp(tokens[i], "<") == 0){
            command->mod3 = 1;
            command->input = tokens[i + 1];
            i++;
        }
        else{
            command->args[command->count_arg] = tokens[i];
            command->count_arg++;
        }
    }
    command->args[command->count_arg] = NULL;
    command->count_arg++;
}

void save_history(char **line){
    FILE *file_h = fopen("file_h", "a+");
    int lines = 0;
    char **hist = malloc(12);
    int cnt = 0;
    char *lh = malloc(100);
    while(1){
        char *x = fgets(lh, 100, file_h);
        if(x == NULL) break;
        hist[lines] = malloc(10);
        /*hist[lines][99] = 0;
        for(int i = 0; i < 100; i++){
            if(lh[i] == '\n') break;
            hist[lines][i] = lh[i];    
        }*/
        lines++;
    }
    fclose(file_h);
    /*file_h = fopen("file_h", "wt");
    int i = 0;
    if(lines == 10) i++;
    for(; i < lines; i++){
        char *cad = malloc(100);
        for(int j = 0; j < 100; j++){
            if(hist[i][j] == '\n'){
                cad[j] = '\n';
            }
            cad[j] = hist[i][j];
        }
        printf("%s", cad);
        fputs(cad, file_h);

    }*/
    //fputs(*line, file_h);
    //fclose(file_h);
}

int main(){
    int u = 0;
    while(1){
        printf(Yellow "my-prompt " RESET "$ ");
        char *rd = malloc(sizeof(char) * 100);
        char *command = malloc(sizeof(char) * 100);
        char *rest = malloc(sizeof(char) * 100);
        char *save_h = malloc(sizeof(char) * 100);
        fgets(rd, 100, stdin);
        for(int i = 0; i < 100; i++){
            if(rd[i] == '\n'){
                save_h[i] = '\n';
                break;    
            }
            save_h[i] = rd[i];
        }
        //save_history(&save_h);
        int size_command = 0, size_rest = 0;
        int t = 0;
        int spaces_ = 0;
        while(spaces_ < 100 && rd[spaces_] == ' ')
            spaces_++;
        for(int i = spaces_; i < 100; i++){
            if(rd[i] == ' ' || rd[i] == '\n' || rd[i] == '#'){
                t = i + 1;
                command[size_command] = 0;
                break;         
            }
            command[size_command] = rd[i];
            size_command++;
        }
        if(rd[t - 1] == ' '){
            for(int i = t; i < 100; i++){
                if(rd[i] == '\n' || rd[i] == '#'){
                    rest[i - t] = 0;
                    break;
                }
                rest[i - t] = rd[i];
                size_rest++;
            }
        }
        if(strcmp(command, "history") == 0){
            
        }
        if(strcmp(command, "exit") == 0)
            return 0;
        //Captar la entrada luego de separar en comando y rest que deberian ser los parametros y obviando lo que este despues de '#'
        int count_comm = -1;
        Commands array_comm[100]; // Array de diferentes comandos
        char **tokens = malloc(100);
        int count_tokens = 0;
        char *cad_aux = malloc(100);
        int cnt = 0;
        int comillas = 0;
        tokens[0] = command;
        count_tokens++;
        for(int i = 0; i < size_rest; i++){
            if(rest[i] == '|' && comillas == 0){
                count_comm++;
                tokens[count_tokens] = NULL;
                initial(&array_comm[count_comm]);
                Parse_to_command(tokens, count_tokens, &array_comm[count_comm]);
            }
            if(rest[i] == ' ' && comillas == 0){
                if(cnt){
                    tokens[count_tokens] = cad_aux;
                    count_tokens++;
                    cnt = 0;
                    cad_aux = malloc(100);
                }
                continue;
            }
            if(rest[i] == '"'){
                if(comillas == 1){
                    tokens[count_tokens] = cad_aux;
                    count_tokens++;
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
            tokens[count_tokens] = cad_aux;
            count_tokens++;
            cnt = 0;
        }
        count_comm++;
        initial(&array_comm[count_comm]);
        Parse_to_command(tokens, count_tokens, &array_comm[count_comm]);
        if(strcmp(command, "cd") == 0){
            chdir(array_comm[0].args[1]);
        }
        else if(size_command > 0){
            pid_t pid = fork();
            if(pid == 0){
                if(array_comm[0].mod1 == 1){
                    int fd = creat(array_comm[0].output, 0644);
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                }
                if(array_comm[0].mod2 == 1){
                    int fd = open(array_comm[0].output, O_CREAT | O_WRONLY | O_APPEND, 777);
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                }
                if(array_comm[0].mod3 == 1){
                    int fd = open(array_comm[0].input, O_RDONLY);
                    dup2(fd, STDIN_FILENO);
                    close(fd);
                }
                int cap = execvp(array_comm[0].args[0], array_comm[0].args);
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
