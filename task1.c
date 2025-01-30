#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MY_MAX_INPUT 1024

void changeDirectory(char *path) {
    char cwd[PATH_MAX];

    if (path == NULL) {
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("Current directory: %s\n", cwd);
        } else {
            perror("getcwd");
        }
        return;
    }

    if (chdir(path) != 0) {
        perror("chdir");
        return;
    }

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        setenv("PWD", cwd, 1);
        printf("Changed to directory: %s\n", cwd);
    } else {
        perror("getcwd");
    }
}

void listDirectory(char *path) {
    struct dirent *entry;
    DIR *dir = opendir(path ? path : ".");

    if (dir == NULL) {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        printf("%s\n", entry->d_name);
    }

    closedir(dir);
}

void listEnviron() {
    extern char **environ;
    for (char **env = environ; *env; env++) {
        printf("%s\n", *env);
    }
}

void setVariable(char *var, char *value) {
    if (var == NULL || value == NULL) {
        printf("Usage: set VARIABLE VALUE\n");
        return;
    }
    setenv(var, value, 1);
    printf("Set %s=%s\n", var, value);
}


void echoMessage(char *message) {
    if (message != NULL) {
        printf("%s\n", message);
    }
}

void showHelp() {
    printf("Simple Shell Commands:\n");
    printf("  cd DIRECTORY    - Change current directory\n");
    printf("  dir DIRECTORY   - List contents of a directory\n");
    printf("  environ         - List environment variables\n");
    printf("  set VAR VALUE   - Set an environment variable\n");
    printf("  echo MESSAGE    - Print a message\n");
    printf("  help            - Show this help manual\n");
    printf("  pause           - Pause until Enter is pressed\n");
    printf("  quit            - Exit the shell\n");
}

void pauseShell() {
    printf("Press Enter to continue...");
    while (getchar() != '\n');
}

void executeCommand(char *input) {
    char *args[128];
    char *inputFile = NULL;
    char *outputFile = NULL;
    int background = 0;
    int i = 0;

    char *token = strtok(input, " ");
    while (token != NULL) {
        if (strcmp(token, "<") == 0) {
            inputFile = strtok(NULL, " ");
        } else if (strcmp(token, ">") == 0) {
            outputFile = strtok(NULL, " ");
        } else if (strcmp(token, ">>") == 0) {
            outputFile = strtok(NULL, " ");
        } else if (strcmp(token, "&") == 0) {
            background = 1;
        } else {
            args[i++] = token;
        }
        token = strtok(NULL, " ");
    }
    args[i] = NULL;

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        if (inputFile) {
            int inFd = open(inputFile, O_RDONLY);
            if (inFd < 0) {
                perror("open input file");
                exit(EXIT_FAILURE);
            }
            dup2(inFd, STDIN_FILENO);
            close(inFd);
        }

        if (outputFile) {
            int outFd;
            if (strstr(outputFile, ">>")) {
                outFd = open(outputFile, O_WRONLY | O_CREAT | O_APPEND, 0644);
            } else {
                outFd = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            }
            if (outFd < 0) {
                perror("open output file");
                exit(EXIT_FAILURE);
            }
            dup2(outFd, STDOUT_FILENO);
            close(outFd);
        }

        if (execvp(args[0], args) == -1) {
            perror("execvp");
        }
        exit(EXIT_FAILURE);
    } else {
        if (!background) {
            waitpid(pid, NULL, 0);
        } else {
            printf("Process running in background: %d\n", pid);
        }
    }
}

void processBatchFile(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("fopen");
        return;
    }

    char input[MY_MAX_INPUT];
    while (fgets(input, MY_MAX_INPUT, file) != NULL) {
        input[strcspn(input, "\n")] = 0;
        executeCommand(input);
    }

    fclose(file);
}

void getCurrentDirectory(char *buffer, size_t size) {
    if (getcwd(buffer, size) == NULL) {
        perror("getcwd");
    }
}

int main(int argc, char *argv[]) {
    char input[MY_MAX_INPUT];
    char cwd[PATH_MAX];

    if (argc > 1) {
        processBatchFile(argv[1]);
        return 0;
    }

    while (1) {
        getCurrentDirectory(cwd, sizeof(cwd));
        printf("%s> ", cwd);
        if (fgets(input, MY_MAX_INPUT, stdin) == NULL) break;
        input[strcspn(input, "\n")] = 0;
        executeCommand(input);
    }

    return 0;
}