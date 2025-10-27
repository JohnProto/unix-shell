//CSD4999 John Protopsaltis
#include<stdio.h>
#include<string.h>
#include<stdbool.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<fcntl.h>
#include<ctype.h>

char* space_trim(char* str){
    char* end;

    while(isspace((unsigned char)* str)){
        str++;
    }
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)* end)){
        end--;
    }
    *(end + 1) = '\0';
    return (str);
}

int redirections(char* command){
    char* str1;
    char* str2;
    int fd;

    str1 = strtok(command, "<");
    str2 = strtok(NULL, "<");
    if(str2){
        if(strstr(str2, ">")){
            if(strstr(str2, ">>")){
                str1 = strtok(str2, ">>");
                str2 = strtok(NULL, ">>");
                str1 = space_trim(str1);
                str2 = space_trim(str2);
                fd = open(str1, O_RDONLY);
                dup2(fd, 0);
                fd = open(str2, O_WRONLY | O_CREAT | O_APPEND, 0644);
                dup2(fd, 1);
            }
            else{
                str1 = strtok(str2, ">");
                str2 = strtok(NULL, ">");
                str1 = space_trim(str1);
                str2 = space_trim(str2);
                fd = open(str1, O_RDONLY);
                dup2(fd, 0);
                fd = open(str2, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                dup2(fd, 1);
            }
            return 1;
        }
        else if(strstr(str1, ">")){
            str2 = space_trim(str2);
            fd = open(str2, O_RDONLY);
            dup2(fd, 0);
            if(strstr(str1, ">>")){
                strtok(str1, ">>");
                str1 = strtok(NULL, ">>");
                str1 = space_trim(str1);
                fd = open(str1, O_WRONLY | O_CREAT | O_APPEND, 0644);
                dup2(fd, 1);
            }
            else{
                strtok(str1, ">");
                str1 = strtok(NULL, ">");
                str1 = space_trim(str1);
                fd = open(str1, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                dup2(fd, 1);
            }
            return 1;
        }
        else{
            str2 = space_trim(str2);
            fd = open(str2, O_RDONLY);
            dup2(fd, 0);
            return 2;
        }
    }
    else if(strstr(str1, ">")){
        if(strstr(str1, ">>")){
            strtok(str1, ">>");
            str1 = strtok(NULL, ">>");
            str1 = space_trim(str1);
            fd = open(str1, O_WRONLY | O_CREAT | O_APPEND, 0644);
            dup2(fd, 1);
        }
        else{
            strtok(str1, ">");
            str1 = strtok(NULL, ">");
            str1 = space_trim(str1);
            fd = open(str1, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            dup2(fd, 1);
        }
        return 3;
    }
    return 0;
}

bool specialCommands(char input[]){
    if(strstr(input, "quit")){
        exit(0);
    }
    if(strstr(input, "chdir")){
        strtok(input, " ");
        chdir(strtok(NULL, " "));
        return true;
    }
    return false;
}

void pipes(char* piped_commands[]){
    int i = 0, PId, pos, status, fd[2], prev, state;
    char* token;
    char* arguments[500];

    while(piped_commands[i]){
        if(specialCommands(piped_commands[i])){
            i++;
            continue;
        }
        if(pipe(fd)){
            perror("pipe initialization failure");
            break;
        }
        PId = fork();
        if(!PId){
            state = redirections(piped_commands[i]);
            if(piped_commands[i+1] && state != 1 && state != 3){
                dup2(fd[1], 1);
            }
            if(i && state != 1 && state != 2){
                dup2(prev, 0);
            }
            pos = 0;
            token = strtok(piped_commands[i], " ");
            while(token){
                arguments[pos++] = token;
                token = strtok(NULL, " ");
            }
            arguments[pos] = NULL;
            close(fd[1]);
            close(prev);
            execvp(arguments[0], arguments);
            perror("Wrong input\n");
            exit(1);
        }
        else if(PId > 0){
            close(fd[1]);
            close(prev);
            waitpid(-1, &status, 0);
            prev = fd[0];
        }
        else{
            printf("Something went wrong, oopsies (̶◉‿◉̶)\n");
        }
        i++;
    }
}

void normal(char* commands){
    int PId, status, pos;
    char* token;
    char* arguments[500];

    if(specialCommands(commands)){
        return;
    }
    PId = fork();
    if(!PId){
        pos = 0;
        redirections(commands);
        token = strtok(commands, " ");
        while(token){
            arguments[pos++] = token;
            token = strtok(NULL, " ");
        }
        arguments[pos] = NULL;
        execvp(arguments[0], arguments);
        printf("Wrong input\n");
        exit(1);
    }
    else if(PId > 0){
        waitpid(-1, &status, 0);
    }
    else{
        printf("Something went wrong, oopsies (̶◉‿◉̶)\n");
    }
}

void seperate(char *test){
    char* commands[500];
    char* piped_commands[500];
    char* token = strtok(test, ";");
    int pos = 0, i;

    while(token){
        commands[pos++] = token;
        token = strtok(NULL, ";"); 
    }
    commands[pos] = NULL;
    for(i = 0; commands[i]; i++){
        if(strstr(commands[i], "|")){
            pos = 0;
            token = strtok(commands[i], "|");
            while(token){
                piped_commands[pos++] = token;
                token = strtok(NULL, "|"); 
            }
            piped_commands[pos] = NULL;
            pipes(piped_commands);
        }
        else{
            normal(commands[i]);
        }
    }
}

int main(void){
    char currDir[1024];
    char nigput[500];

    while(true){
        getcwd(currDir, 1024);
        if(currDir){
            printf("CSD4999-hy345sh@%s:%s ", getlogin(), currDir);
        }
        else{
            printf("Couldn't get current directory\n");
        }
        fgets(nigput, 500, stdin);
        strtok(nigput, "\n");
        seperate(nigput);
    }
    return 0;
}