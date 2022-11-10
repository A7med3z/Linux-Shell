#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

int num_vars = 0;
struct variable {
    char* name;
    char* value;
};

struct variable variables[20];

FILE *file;
void child_terminated(){
	int wstat;
	pid_t pid;
	while (1) {
		pid = wait3 (&wstat, WNOHANG, (struct rusage *)NULL);
		if(pid==0||pid==-1)
			break;
	}
	file = fopen("log_file", "a");
	fputs("The child terminated successfully\n", file);
	fclose(file);
}

char* get_command_type(char *command) {
    if (strcmp(command, "cd") == 0 || strcmp(command, "echo") == 0 || strcmp(command, "exit") == 0) {
        return "builtin";
    } else
        return "others";
}

static void excute_shell_builtin(char *command, char *parameters[]) {
    if(strcmp(command, "cd") == 0) {
        chdir(parameters[1]);
        printf("> %s\n", getcwd(command, 100));
    } else if (strcmp(command, "echo") == 0) {
        int i = 1,j = 0;
        char par[50];
        while (parameters[i] != NULL) {
            int k = 0;
            while (parameters[i][k] != NULL) {
                par[j] = parameters[i][k];
                j++;
                k++;
            }
            par[j] = ' ';
            j++;
            i++;
        }
        par[j] = '\0';
        char* p = strtok(par, "\"");
        printf("%s\n", p);
    } else {
        return;
    }
}

void excute_commands(char *command, char *parameters[]) {
    pid_t pid = fork();
    int status;
    int background = 0;
    int i = 0;
    while (parameters[i] != NULL){
        if (strcmp("&", parameters[i]) == 0) {
            background = 1;
        }
        parameters[i] = parameters[i + background];
        i++;
    }
    if (pid < 0) {
        printf("Error\n");
        exit(1);
    } else if (pid == 0){
        execvp(command, parameters);
        printf("Error\n");
        exit(1);
    } else {
        if (background == 0) {
            if ((pid = waitpid(pid, &status, WUNTRACED)) == -1)
                perror("wait() error");
        }
    }
}

void setup_environment() {
    printf("       Welcome to my shell         \n");
    sleep(1);
}

void printDir() {
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf(">>> %s: ", cwd);
}

int main(int argc, char* argv[]) {
    setup_environment();
    signal(SIGCHLD, child_terminated);
    char* array[30];
    char* command = "";
    char* parameters[20];
    while (strcmp(command, "exit") != 0) {
        printDir();
        int background = 0;
        char line[1000] = "";
        char readed[1000] = "";
        gets(readed/*, 1000, stdin*/);
        int k = 0;
        int d = 0;
        while (readed[k] != '\n' && readed[k] != NULL) {
            if (readed[k] == '$') {
                if (k != 0) {
                    if (readed[k - 1] != ' ') {
                       continue;
                    }
                }
                while (readed[k - d] != ' ' && readed[k - d] != '\n') {
                    d --;
                }
                char v_name[-1 * d - 1];
                int d2;
                for (d2 = 0; d2 < -1 * d - 1; d2++) {
                    v_name[d2] = readed[k + d2 + 1];
                }
                char* var_name = v_name;
                char var_value[200];
                char* value;
                int i = 0;
                for (i = 0; i < num_vars; i++) {
                    if (strcmp(var_name, variables[i].name) == 0) {
                        value = variables[i].value;
                    }
                }
                strcpy(var_value, value);
                i = 0;
                while (var_value[i] != NULL) {
                    line[k + i] = var_value[i];
                    i++;
                }
                k = k - d;
                d = i + d;
            } else {
                line[k + d] = readed[k];
            }
            k++;
        }
        int i = 0;
        char* token = strtok(line, " ");
        array[0] = token;
        i++;
        while(token != NULL) {
            token = strtok(NULL, " ");
            array[i] = token;
            i++;
        }
        int j;
        command = array[0];
        for (j = 0;j < i;j++){
            parameters[j] = array[j];
            parameters[i] = NULL;
        }
        if (strcmp(command, "export") == 0) {
        int i = 1, j = 0;
        char par[20];
        while (parameters[i] != NULL) {
            int k = 0;
            while (parameters[i][k] != NULL) {
                par[j] = parameters[i][k];
                j++;
                k++;
            }
            par[j] = ' ';
            j++;
            i++;
        }
        par[j - 1] = '\n';
        char* var_name = strtok(par, "=");
        char* var_value = strtok(NULL, "=");
        char s[20];
        strcpy(s, var_value);
        if (s[0] == '\"') {
            var_value = strtok(s, "\"");
        }
        char name[20];
        char value[20];
        strcpy(name, var_name);
        strcpy(value, var_value);
        struct variable v1 = {name, value};
        variables[num_vars] = v1;
        num_vars ++;
        continue;
    }
        if(strcmp(get_command_type(command), "builtin") == 0) {
            excute_shell_builtin(command, parameters);
        } else
            excute_commands(command, parameters);
    }
return 0;
}
