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
#define Red "\x1b[31m"
#define Yellow "\x1b[33m"
#define Blue "\x1b[34m"
#define Cyan "\x1b[36m"
#define RESET "\x1b[0m"

char *path_initial;

typedef struct Commands{
    char *args[100];
    int count_arg;
    int mod1, mod2, mod3; //mod1 = 1 cuando existe >, mod2 = 1 para >>, mod3 = 1 para <
    char *input, *output;
}Commands;

typedef struct Parse{
    char *command, *rest;
    int size_command, size_rest;
}Parse;

void initial_Parse(Parse *parse){
    parse->command = malloc(100);
    parse->rest = malloc(100);
    parse->size_command = 0;
    parse->size_rest = 0;
}

void initial_Commands(Commands *command){
    command->count_arg = 0;
    command->mod1 = command->mod2 = command->mod3 = 0;
    command->input = malloc(100);
    command->output = malloc(100);
}

int min(int a, int b){
    if(a < b) return a;
    return b;
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

void save_history(char *line){
    FILE *file_h = fopen(path_initial, "a+");
    int lines = 0;
    char *hist = malloc(12 * 100);
    char *lh = malloc(100);
    int tot = 0;
    int cant;
    fscanf(file_h, "%d", &cant);
    fgets(lh, 100, file_h);
    for(int j = 0; j < cant; j++){
        fgets(lh, 100, file_h);
        if(cant == 10 && j == 0) continue;
        for(int i = 0; i < 100; i++){
            hist[tot] = lh[i];
            tot++;
            if(lh[i] == '\n') break;
        }
        lines++;
    }
    for(int i = 0; i < 100; i++){
        hist[tot] = line[i];
        tot++;
        if(line[i] == '\n') break;
    }
    fclose(file_h);
    file_h = fopen(path_initial, "w");
    fprintf(file_h, "%d\n", min(lines + 1, 10));
    fputs(hist, file_h);
    fclose(file_h);
}

void show_history(){
    FILE *file_h = fopen(path_initial, "r");
    char *lh = malloc(100);
    int cant;
    fscanf(file_h, "%d", &cant);
    fgets(lh, 100, file_h);
    for(int j = 0; j < cant; j++){
        fgets(lh, 100, file_h);
        printf("%d: ", j + 1);
        printf("%s", lh);
    }
    fclose(file_h);
}

void Parse_Line(Parse *line, char *rd){
    initial_Parse(line);
    int spaces_ = 0;
    while(spaces_ < 100 && rd[spaces_] == ' ')
        spaces_++;
    int t = 0;
    for(int i = spaces_; i < 100; i++){
        if(rd[i] == ' ' || rd[i] == '\n' || rd[i] == '#'){
            t = i + 1;
            line->command[line->size_command] = 0;
            break;         
        }
        line->command[line->size_command] = rd[i];
        line->size_command++;
    }
    if(rd[t - 1] == ' '){
        for(int i = t; i < 100; i++){
            if(rd[i] == '\n' || rd[i] == '#'){
                line->rest[i - t] = 0;
                break;
            }
            line->rest[i - t] = rd[i];
            line->size_rest++;
        }
    }
}

int Again_Command(Parse *line, char *rd){
	int num = 0;
    for(int i = 0; i < line->size_rest; i++)
        num = num * 10 + (line->rest[i] - '0');
    FILE *file_h = fopen(path_initial, "r");
    char *lh = malloc(100);
    int cant;
    fscanf(file_h, "%d", &cant);
    if(cant < num){
        printf(Red "Command not found\n" RESET);
        fclose(file_h);
        return 0;
    }
    fgets(lh, 100, file_h);
    for(int j = 0; j < cant; j++){
        fgets(lh, 100, file_h);
        if(j == num - 1){
            for(int i = 0; i < 100; i++)
                rd[i] = lh[i];
            break;
        }
    }
    fclose(file_h);
    Parse_Line(&(*line), rd);
   	return 1;
}

void execute_command(Commands *command){
	pid_t pid = fork();
    if(pid == 0){
        if(command->mod1 == 1){
            int fd = creat(command->output, 0644);
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
        if(command->mod2 == 1){
            int fd = open(command->output, O_CREAT | O_WRONLY | O_APPEND, 777);
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
        if(command->mod3 == 1){
            int fd = open(command->input, O_RDONLY);
            dup2(fd, STDIN_FILENO);
            close(fd);
        }
        int cap = execvp(command->args[0], command->args);
        if(cap < 0) printf(Red "Error to execute \'%s\'\n" RESET, command->args[0]);
        exit(0);
    }
    else{
        wait(NULL);
    }
}

void Parse_For_Pipes(Commands *array_comm, Parse *line){
	int count_comm = -1;
    char **tokens = malloc(100);
    int count_tokens = 0;
    char *cad_aux = malloc(100);
    int cnt = 0;
    int comillas = 0;
    tokens[0] = line->command;
    count_tokens++;
    for(int i = 0; i < line->size_rest; i++){
        if(line->rest[i] == '|' && comillas == 0){
        	if(cnt > 0){
	        	tokens[count_tokens] = cad_aux;
	            count_tokens++;
	            cnt = 0;
	            cad_aux = malloc(100);
        	}
            count_comm++;
            tokens[count_tokens] = NULL;
            initial_Commands(&array_comm[count_comm]);
            Parse_to_command(tokens, count_tokens, &array_comm[count_comm]);
        }
        if(line->rest[i] == ' ' && comillas == 0){
            if(cnt){
                tokens[count_tokens] = cad_aux;
                count_tokens++;
                cnt = 0;
                cad_aux = malloc(100);
            }
            continue;
        }
        if(line->rest[i] == '"'){
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
        cad_aux[cnt] = line->rest[i];
        cnt++;           
    }
    if(cnt){
        tokens[count_tokens] = cad_aux;
        count_tokens++;
        cnt = 0;
    }
    count_comm++;
    initial_Commands(&array_comm[count_comm]);
    Parse_to_command(tokens, count_tokens, &array_comm[count_comm]);
}

int main(){
	//Para crear el archivo file_h en caso de que no exista que va a contener el historial
    path_initial = malloc(100);
    getcwd(path_initial, 100);
    char *aux_s = "/file_h";
    for(int i = strlen(path_initial), c = 0; c < 7; c++, i++)
        path_initial[i] = aux_s[c];
    FILE *cf = fopen(path_initial, "r");
    if(cf == NULL){
        cf = fopen(path_initial, "w");
        fprintf(cf, "%d\n", 0);
    }
    fclose(cf);
    while(1){
        printf(Yellow "my-prompt " RESET "$ ");
        char *rd = malloc(sizeof(char) * 100);
        char *save_h = malloc(sizeof(char) * 100);
        fgets(rd, 100, stdin);
        if(rd[0] == '\n') continue;
        Parse line;
        Parse_Line(&line, rd);
        if(strcmp(line.command, "again") == 0){
            int proof = Again_Command(&line, rd);
           	if(proof == 0) continue;
        }
        int spaces_ = 0;
        while(spaces_ < 100 && rd[spaces_] == ' ')
            spaces_++;
        for(int i = 0; i < 100; i++){
            if(rd[i] == '\n'){
                save_h[i] = '\n';
                break;    
            }
            save_h[i] = rd[i];
        }
        if(spaces_ == 0) save_history(save_h);
        if(strcmp(line.command, "history") == 0){
            show_history();
            continue;
        }
        if(strcmp(line.command, "exit") == 0)
            return 0;
       	Commands *array_comm = malloc(100);
       	Parse_For_Pipes(array_comm, &line);
        if(strcmp(line.command, "cd") == 0){
            chdir(array_comm[0].args[1]);
        }
        else{
           	execute_command(&array_comm[0]);
        }
        
    }
    return 0;
}
