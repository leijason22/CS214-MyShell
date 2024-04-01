#include "commands.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <limits.h>

/* parsing functions
- functions to parse commands, tokenization, handling redirection, pipes, conditionals
*/

/* execution functions
- functions to execute parsesd commands, handling build-in commands
ex: cd, pwd, which, exit, and EXTERNAL commands
*/

/* I/O functions
- functions to handle file IO, including redirection of standard input/output
*/

// Function to parse a command and return an array of tokens
char** parse_command(char *command) {
    const char delimiters[] = " \t\n"; // Tokens are separated by whitespace
    int token_count = 0;
    char *token;
    char **tokens = malloc(sizeof(char*));

    if (!tokens) {
        fprintf(stderr, "Memory allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(command, delimiters);
    while (token != NULL) {
        tokens[token_count] = strdup(token);
        if (!tokens[token_count]) {
            fprintf(stderr, "Memory allocation error\n");
            exit(EXIT_FAILURE);
        }
        token_count++;
        tokens = realloc(tokens, (token_count + 1) * sizeof(char*));
        if (!tokens) {
            fprintf(stderr, "Memory allocation error\n");
            exit(EXIT_FAILURE);
        }
        token = strtok(NULL, delimiters);
    }
    tokens[token_count] = NULL;
    return tokens;
}

// Function to execute a parsed command
void execute_command(char *command) {
    char **tokens = parse_command(command);

    // Example: handle built-in commands
    //cd: change the working directory
    //expects one argument, which is a path to a directory
    //use chdir() to change its own directory
    //cd should print an error message and fail if it is given the wrong number of arguments
    //or if chdir() fails
    if (strcmp(tokens[0], "cd") == 0) {
        // Example: chdir(tokens[1]);
        if (tokens[1] == NULL) {
            fprintf(stderr, "cd: missing argument\n");
        } else {
            if (chdir(tokens[1]) != 0) {
                fprintf(stderr, "cd: %s\n", strerror(errno));
            }
        }
    } else if (strcmp(tokens[0], "pwd") == 0) {
        //pwd: prints current working directory to std output
        //use getcwd()
        // Example: system("pwd");
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
        } else {
            fprintf(stderr, "pwd: %s\n", strerror(errno));
        }
    } else if (strcmp(tokens[0], "which") == 0) {
        //which: takes a single argument (name of a program), prints path
        //that mysh would use if asked to start that program (result of search for bare names)
        //print nothing and fails if it is given the wrong number of arguments, or the name of a built-in, or if the program
        //is not found
        if (tokens[1] == NULL) {
            fprintf(stderr, "which: missing argument\n");
        } else {
            char *path = getenv("PATH");
            if (path != NULL) {
                char *path_copy = strdup(path);
                char *dir = strtok(path_copy, ":");
                while (dir != NULL) {
                    char command_path[PATH_MAX];
                    snprintf(command_path, sizeof(command_path), "%s/%s", dir, tokens[1]);
                    if (access(command_path, F_OK | X_OK) == 0) {
                        printf("%s\n", command_path);
                        break;
                    }
                    dir = strtok(NULL, ":");
                }
                free(path_copy);
            }
        }
    } else if (strcmp(tokens[0], "exit") == 0) {
        //exit: indicates that mysh should cease reading commands and terminate
        // Free memory allocated for tokens
        for (int i = 0; tokens[i] != NULL; i++) {
            free(tokens[i]);
        }
        free(tokens);
        printf("Exiting my shell.\n");
        exit(EXIT_SUCCESS);
    } else {
        // Execute external commands
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            execvp(tokens[0], tokens);
            // execvp returns only if an error occurs
            fprintf(stderr, "Error executing command %s\n", tokens[0]);
            exit(EXIT_FAILURE);
        } else if (pid < 0) {
            // Fork failed
            fprintf(stderr, "Fork failed\n");
        } else {
            // Parent process
            int status;
            waitpid(pid, &status, 0);
            // Handle exit status if needed
        }
    }

    // Free memory allocated for tokens
    for (int i = 0; tokens[i] != NULL; i++) {
        free(tokens[i]);
    }
    free(tokens);
}