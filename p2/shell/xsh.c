#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

int numArgs = 0;
char **path;
int numPath = 1;
char error_message[30] = "An error has occurred\n";

int run_cd(char **args) {
    if (numArgs == 1) {
        char* dir = getenv("HOME");
        if (chdir(dir) != 0) {
            write(STDERR_FILENO, error_message, strlen(error_message));
            return 1;
        }
    } else {
        if (chdir(args[1]) != 0) {
            write(STDERR_FILENO, error_message, strlen(error_message));
            return 1;
        }
    }
    return 0;
}

int run_out(char **args, int *in, int *out) {
    for (int i = 0; i < numArgs; i++) {
        if (strcmp(args[i], ">") == 0) {
            if (i == (numArgs-1)) {
                return -1;
            } else if (numArgs -i -1 == 1) {
                *out = 1;
                return i;
            } else if (numArgs -i -1 == 3) {
                if (strcmp(args[numArgs -2], "<<<") == 0) {
//                    printf("out found > and <<<\n");  ///////
                    *in = 1;
                    *out = 1;
                    return i;
                } else {
//                    printf("excess arg after >\n");  //////
                    return -1;
                }
            } else {
//                printf("other situation\n");   /////////////
                return -1;
            }
        } else if (strcmp(args[i], "<<<") == 0) {
                if (i == (numArgs-1) || (numArgs -1 - i) > 1) {
                    return -1;
                } else {
///                    printf("out found <<<\n");   ///////////
                    *in = 1;
                    return i;
                }
            }
    }
        return 0;
}

int run_child(char **args) {
    pid_t pid;
    int *in = malloc(sizeof(int));
    *in = 0;
    int *out = malloc(sizeof(int));
    *out = 0;
    int status = 0;
    int rm = run_out(args, in, out);
//    printf("in:%d out:%d\n",*in, *out);  /////////////
    char *dir[numPath];
    struct stat *buf;
    buf = malloc(sizeof(struct stat));
    int p[2];
    if (pipe(p) == -1) {
        write(STDERR_FILENO, error_message, strlen(error_message));
        return 1;
    }
    pid = fork();
    if (pid < 0) {
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    } else if (pid == 0) {
///        printf("pid: %d rm: %d\n", pid, rm);  /////////
        if (rm == -1) {
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(1);
        } else if (rm == 0) {
        } else if (rm > 0 && *out == 1) {
            int len = strlen(args[rm + 1]) + strlen(".out") + 1;
            char *output = malloc(sizeof(char) * len);
//            char *output = strdup(args[rm + 1]);
            strcpy(output, args[rm + 1]);
            strcat(output, ".out");
            int fd_out = open(output ,
               O_WRONLY|O_CREAT|O_TRUNC , S_IRWXU);
            if (fd_out == -1) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(1);
            }
            if (dup2(fd_out, STDOUT_FILENO) == -1) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(1);
            }
            char *outerr = malloc(sizeof(char) * len);
            strcpy(outerr, args[rm + 1]);
//           char *outerr = strdup(args[rm + 1]);
            strcat(outerr, ".err");
            int fd_err = open(outerr ,
               O_WRONLY|O_CREAT|O_TRUNC , S_IRWXU);
            if (fd_err == -1) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(1);
            }
            if (dup2(fd_err, STDERR_FILENO) == -1) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(1);
            }
            free(output);
            free(outerr);
            args[rm] = NULL;
        }
//        printf("child: in:%d out:%d\n",*in, *out);  //////////
        if (*in == 1) {
//            printf(" child found <<<\n");  ////////////
            close(p[1]);
            if (dup2(p[0], STDIN_FILENO) == -1) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(1);
            }
            close(p[0]);
            args[rm] = NULL;
        } else {
            close(p[0]);
            close(p[1]);
        }
        for (int i = 0; i< numPath; i++) {
            int len = strlen(args[0]) + strlen(path[i]) + 2;
            dir[i] = malloc(sizeof(char) * len);
            strcpy(dir[i], path[i]);
//            dir[i] = strdup(path[i]);
            strcat(dir[i], "/");
            strcat(dir[i], args[0]);
            }
         for (int i = 0; i < numPath; i++) {
            if (stat(dir[i], buf) == 0) {
                args[0] = dir[i];
                status = execv(dir[i], args);
                if (status == -1) {
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    exit(1);
                }
            }
         }
         for (int i = 0; i < numPath; i++) {
            free(dir[i]);
         }
         write(STDERR_FILENO, error_message, strlen(error_message));
         exit(1);
    } else {
//         printf("pid: %d\n", pid);  /////////
//        pid_t r = wait(&status);
        if (*in == 1) {
//            printf("parent found <<<\n");   ///////////////
            write(p[1], args[numArgs - 1], strlen(args[numArgs - 1]));
            write(p[1], "\n", 1);
        }
        close(p[0]);
        close(p[1]);
        pid_t r = wait(&status);
        if (r == -1) {
            write(STDERR_FILENO, error_message, strlen(error_message));
            return 1;
        }
    }
    free(buf);
    free(in);
    free(out);
    buf = NULL;
    out = NULL;
    in = NULL;
    return WEXITSTATUS(status);
}

int run_path(char **args) {
    if (numArgs == 1) {
        for (int i = 0; i < numPath; i++) {
           printf("%s\n", path[i]);
        }
    } else {
        for (int i = 0; i < numPath; i++) {
            free(path[i]);
        }
        path = realloc(path, sizeof(char*) * (numArgs -1));
        if (path == NULL) {
             write(STDERR_FILENO, error_message, strlen(error_message));
             return 1;
        }
        for (int i = 0; i < (numArgs -1); i++) {
            path[i] = strdup(args[i+1]);
        }
        numPath = (numArgs -1);
    }
    return 0;
}

int run_type(char **args) {
//    char *bt[4] = {"exit", "cd", "type", "path"};
    if (numArgs != 2) {
         write(STDERR_FILENO, error_message, strlen(error_message));
         return 1;
    }
    if ((strcmp(args[1], "cd") == 0) ||
           (strcmp(args[1], "path") == 0) ||
           (strcmp(args[1], "exit") == 0) ||
           (strcmp(args[1], "type") == 0)) {
            fprintf(stdout, "%s is a shell builtin\n", args[1]);
            return EXIT_SUCCESS;
    }

    char* dir[numPath];
    struct stat *buf;
    buf = malloc(sizeof(struct stat));
    for (int i = 0; i < numPath; i++) {
        int len = strlen(args[0]) + strlen(path[i]) + 2;
        dir[i] = malloc(sizeof(char) * len);
        strcpy(dir[i], path[i]);
        strcat(dir[i], "/");
        strcat(dir[i], args[1]);
    }
    for (int i=0; i < numPath; i++) {
        if (stat(dir[i], buf) == 0) {
            printf("%s is %s\n", args[1], dir[i]);
            free(buf);
            buf = NULL;
            for (int i = 0; i < numPath; i++) {
                free(dir[i]);
            }
            return 0;
        }
    }
    for (int i = 0; i < numPath; i++) {
        free(dir[i]);
    }

    free(buf);
    write(STDERR_FILENO, error_message, strlen(error_message));
    return 1;
}


int run_cmd(char* input, char **args) {
    if (numArgs == 0) {
        return 0;
    }
    int run_rc = 0;
    if (strcmp(args[0], "exit") == 0) {
        for (int i = 0; i < numPath; i++) {
            free(path[i]);
        }
        free(path);
        free(input);     //////
        free(args);
        exit(0);
    } else if (strcmp(args[0], "cd") == 0) {
//        printf("cding\n");
        run_rc = run_cd(args);
    } else if (strcmp(args[0], "path") == 0) {
        run_rc = run_path(args);
    } else if (strcmp(args[0], "type") == 0) {
        run_rc = run_type(args);
    } else {
        run_rc = run_child(args);
    }
    return run_rc;
}


void parse_cmd(char* input, char** args, int bufsize) {
    int position = 0;
    numArgs = 0;
    char *token;

    token = strtok(input, " \t\r\n\a");
    while (token != NULL) {
        args[position] = token;
        position++;
        numArgs++;
        if (position >= bufsize) {
            bufsize += 128;
            args = realloc(token, bufsize*sizeof(char));
            if (!args) {
                 write(STDERR_FILENO, error_message, strlen(error_message));
                 exit(1);
            }
        }
        token = strtok(NULL, " \n\t");
    }
    args[position] = NULL;
    free(token);
}

void loop() {
    int rc = 0;
    while (1) {
        char cwd[128];
        if (getcwd(cwd, 128) != NULL) {
            printf("[%s]\n%d> ", cwd, rc);
            fflush(stdout);
        } else {
            write(STDERR_FILENO, error_message, strlen(error_message));
            continue;
        }

        char *input = NULL;
        long unsigned int length = 0;
        getline(&input, &length, stdin);

        if (strlen(input) > 129) {
            write(STDERR_FILENO, error_message, strlen(error_message));
            free(input);
            rc = 1;
            continue;
        }
        int bufsize = 128;
        char **args = malloc(bufsize*sizeof(char*));

        if (!args) {
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(1);
        }
        parse_cmd(input, args, bufsize);

        rc = run_cmd(input, args);
        free(args);
        free(input);
        args = NULL;
        input = NULL;
    }
}
int main(int argc, char *argv[]) {
    if (argc != 1) {
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(1);
    }

    path = malloc(sizeof(char*));
    if (!path) {
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }
    path[numPath - 1] = strdup("/bin");
    loop();
//    free(path);
//    path = NULL;
    return EXIT_SUCCESS;
}
