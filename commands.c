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
#include <glob.h>
#include <ctype.h>
#include <sys/stat.h>

#define MAX_ARGS 64
#define PATH_MAX 4096
#define SUCCESS 0
#define FAILURE 1

void wildcards(char ***tokens, int *token_count, int *size)
{
    char **expanded_tokens = malloc(MAX_ARGS * sizeof(char *));
    int new_token_count = 0;

    if (!expanded_tokens)
    {
        perror("Memory allocation error");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < *token_count; i++)
    {
        if (strchr((*tokens)[i], '*'))
        {
            glob_t glob_result;
            if (glob((*tokens)[i], 0, NULL, &glob_result) == 0)
            {
                for (size_t j = 0; j < glob_result.gl_pathc; j++)
                {
                    expanded_tokens[new_token_count++] = strdup(glob_result.gl_pathv[j]);
                    if (!expanded_tokens[new_token_count - 1])
                    {
                        perror("Memory allocation error");
                        exit(EXIT_FAILURE);
                    }
                    if (new_token_count >= MAX_ARGS - 1)
                    {
                        fprintf(stderr, "Too many arguments\n");
                        exit(EXIT_FAILURE);
                    }
                }
                globfree(&glob_result);
            }
        }
        else
        {
            expanded_tokens[new_token_count++] = strdup((*tokens)[i]);
            if (!expanded_tokens[new_token_count - 1])
            {
                perror("Memory allocation error");
                exit(EXIT_FAILURE);
            }
            if (new_token_count >= MAX_ARGS - 1)
            {
                fprintf(stderr, "Too many arguments\n");
                exit(EXIT_FAILURE);
            }
        }
    }

    // null terminates expanded_tokens
    expanded_tokens[new_token_count] = NULL;

    // free original tokens
    for (int i = 0; i < *token_count; i++)
    {
        free((*tokens)[i]);
    }
    free(*tokens);

    // update tokens with expanded wildcard tokens
    *tokens = expanded_tokens;
    *token_count = new_token_count;
    *size = new_token_count;
}

// parses command line into array of tokens, handles whitespace, encounters with redirection symbols < >
char **parse_command(char *command)
{
    int token_count = 0;
    char **tokens = malloc(MAX_ARGS * sizeof(char *));
    char *next_token = command;
    char *end_token;

    if (!tokens)
    {
        fprintf(stderr, "Memory allocation error\n");
        exit(EXIT_FAILURE);
    }

    // handle tokens that may have redirection symbols attached
    while (*next_token != '\0')
    {
        while (isspace(*next_token))
            next_token++;
        if (*next_token == '\0')
            break;

        if (*next_token == '>' || *next_token == '<')
        {
            tokens[token_count++] = strndup(next_token, 1);
            next_token++;
            // assuming there may not be spaces between the redirection symbol and the file name
            if (!isspace(*next_token))
            {
                end_token = next_token;
                while (!isspace(*end_token) && *end_token != '\0')
                    end_token++;
                tokens[token_count++] = strndup(next_token, end_token - next_token);
                next_token = end_token;
            }
        }
        else
        {
            end_token = next_token;
            while (!isspace(*end_token) && *end_token != '\0')
                end_token++;
            tokens[token_count++] = strndup(next_token, end_token - next_token);
            next_token = end_token;
        }
    }

    // null terminate the tokens array
    tokens[token_count] = NULL;

    // handle wildcards in args
    int size = token_count;
    wildcards(&tokens, &token_count, &size);

    return tokens;
}

// finds full path of a command (use for execv())
char *find_command_path(const char *command)
{
    // get PATH environment
    char *path_env = getenv("PATH");
    if (path_env == NULL)
    {
        perror("PATH");
        return NULL;
    }

    // dup PATH to avoid modifying the environment variable directly
    char *paths = strdup(path_env);
    if (paths == NULL)
    {
        perror("strdup");
        exit(EXIT_FAILURE);
    }

    // search each directory in PATH to find the command
    char *full_path = malloc(FILENAME_MAX);
    char *token = strtok(paths, ":");
    int command_found = 0;

    while (token != NULL)
    {
        snprintf(full_path, FILENAME_MAX, "%s/%s", token, command);
        if (access(full_path, X_OK) == 0)
        {
            command_found = 1; //command is found
            break;
            // free(paths);
            // return full_path; // command found, return full path
        }
        token = strtok(NULL, ":");
    }

    free(paths);

    if (command_found) {
        return full_path;
    } else {
        free(full_path);
        return NULL;
    }

    // free(full_path);
    return NULL; // command not found
}

void execute_piped_commands(char **first_command, char **second_command)
{
    int pipe_fds[2];
    if (pipe(pipe_fds) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t first_pid = fork();
    if (first_pid == 0)
    {
        // first child process: executes first command
        close(pipe_fds[0]);               // close unused read end
        dup2(pipe_fds[1], STDOUT_FILENO); // redirects output to write pipe
        close(pipe_fds[1]);

        char *command_path = find_command_path(first_command[0]);
        if (command_path)
        {
            execv(command_path, first_command);
            free(command_path);
        }
        perror("execv first command");
        exit(EXIT_FAILURE);
    }

    pid_t second_pid = fork();
    if (second_pid == 0)
    {
        // second child process: execute second command
        close(pipe_fds[1]);              // close unused write end
        dup2(pipe_fds[0], STDIN_FILENO); // redirects input to read pipe
        close(pipe_fds[0]);

        char *command_path = find_command_path(second_command[0]);
        if (command_path)
        {
            execv(command_path, second_command);
            free(command_path);
        }
        perror("execv second command");
        exit(EXIT_FAILURE);
    }

    // in parent process, handle closure and waiting for child processes
    close(pipe_fds[0]);
    close(pipe_fds[1]);
    waitpid(first_pid, NULL, 0);
    waitpid(second_pid, NULL, 0);
}

void execute_command(char *command, int *flag)
{
    // calls function to parse command into tokens
    char **tokens = parse_command(command);
    // args new array for use in execv(), filters out redirection symbols and does not count them as arguments
    char **args = malloc(MAX_ARGS * sizeof(char *));
    int arg_count = 0;

    // checks for pipe symbol
    int pipe_index = -1;
    for (int i = 0; tokens[i] != NULL; i++)
    {
        if (strcmp(tokens[i], "|") == 0)
        {
            pipe_index = i;
            break;
        }
    }

    // if pipe is present, splits command into two commands
    if (pipe_index != -1)
    {
        tokens[pipe_index] = NULL;
        char **first_command = tokens;
        char **second_command = &tokens[pipe_index + 1];
        execute_piped_commands(first_command, second_command);
    }
    else
    { // if no pipe is present, execute command as normal
        int stdout_og = dup(STDOUT_FILENO);
        int stdin_og = dup(STDIN_FILENO);
        int fd_out = -1;
        int fd_in = -1; // file descriptors for redirection

        // separates tokens into arguments and handles redirection
        for (int i = 0; tokens[i] != NULL; i++)
        {
            if (strcmp(tokens[i], ">") == 0)
            {
                i++; // redirection symbol is not file name, so skip it
                fd_out = open(tokens[i], S_IRUSR | S_IWUSR | S_IRGRP, 0640);
                if (fd_out == -1)
                {
                    perror("failed to open file for output redirection");
                    exit(EXIT_FAILURE);
                }
            }
            else if (strcmp(tokens[i], "<") == 0)
            {
                i++; // skip the file name for redirection
                fd_in = open(tokens[i], O_RDONLY);
                if (fd_in == -1)
                {
                    perror("failed to open file for input redirection");
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                args[arg_count++] = tokens[i]; // collect arguments
            }
        }
        args[arg_count] = NULL; // null terminate the arguments array

        int size = 0;
        int args_size = 0;
        while (tokens[size] != NULL)
        {
            size++;
        }
        while (args[args_size] != NULL)
        {
            args_size++;
        }

        if (tokens[0] == NULL)
        {
            *flag = FAILURE;
        }
        else if ((((strcmp(tokens[0], "then") == 0) && *flag == SUCCESS) || ((strcmp(tokens[0], "else") == 0) && *flag == FAILURE)))
        {
            free(tokens[0]);
            // Shift all elements by one position to the left
            for (int i = 0; i < size - 1; i++)
            {
                tokens[i] = tokens[i + 1];

            }

            for (int i = 0; i < args_size - 1; i++)
            {
                args[i] = args[i + 1];

            }

            args[args_size - 1] = NULL;
            args_size--;
            tokens[size - 1] = NULL;
            size--;
            
        }
        else if ((((strcmp(tokens[0], "then") == 0) && *flag == FAILURE) || ((strcmp(tokens[0], "else") == 0) && *flag == SUCCESS)))
        {
            for (int i = 0; tokens[i] != NULL; i++)
            {
                free(tokens[i]);
            }
            free(tokens);
            free(args);
            *flag = FAILURE;
            printf("Cannot execute due to condition of previous command\n");
            return;
        }


        if (tokens[0] == NULL)        
        {
            *flag = FAILURE;
            fprintf(stderr, "Missing command\n");
        }
        // cd: change the working directory
        // expects one argument, which is a path to a directory, use chdir() to change its own directory
        // cd should print an error message and fail if it is given the wrong number of arguments
        //  printf("%d", *flag);
        else if (strcmp(tokens[0], "cd") == 0)
        {
            // Example: chdir(tokens[1]);
            if (tokens[1] == NULL)
            {
                *flag = FAILURE;
                fprintf(stderr, "cd: missing argument\n");
            }
            else
            {
                if (chdir(tokens[1]) != 0)
                {
                    *flag = FAILURE;
                    fprintf(stderr, "cd: %s\n", strerror(errno));
                }
            } 
        }
        else if (strcmp(tokens[0], "pwd") == 0)
        {
            char cwd[PATH_MAX];
            if (getcwd(cwd, sizeof(cwd)) != NULL)
            {
                if (fd_out != -1) // Check if there's a redirection for output
                {
                    // Temporarily redirect stdout to fd_out
                    dup2(fd_out, STDOUT_FILENO);
                }

                printf("%s\n", cwd); // This will go to fd_out if redirection is set

                if (fd_out != -1)
                {
                    // Restore stdout
                    dup2(stdout_og, STDOUT_FILENO);
                    close(fd_out);
                }
                *flag = SUCCESS;
            }
            else
            {
                *flag = FAILURE;
                fprintf(stderr, "pwd: %s\n", strerror(errno));
            }
        }
        else if (strcmp(tokens[0], "which") == 0)
        {
            // which: takes a single argument (name of a program), prints path
            // that mysh would use if asked to start that program (result of search for bare names)
            // print nothing and fails if it is given the wrong number of arguments, or the name of a built-in, or if the program
            // is not found
            if (tokens[1] == NULL)
            {
                *flag = FAILURE;
                fprintf(stderr, "which: missing argument\n");
            }
            else
            {
                char *command_path = find_command_path(tokens[1]);
                if (command_path != NULL)
                {
                    if (fd_out != -1) // If there's a redirection for output
                    {
                        // Redirect stdout to fd_out
                        dup2(fd_out, STDOUT_FILENO);
                    }

                    // Print command path, will be redirected if fd_out is set
                    printf("%s\n", command_path);
                    free(command_path);

                    if (fd_out != -1)
                    {
                        // Restore stdout
                        dup2(stdout_og, STDOUT_FILENO);
                        close(fd_out);
                    }
                    *flag = SUCCESS;
                }
                else
                {
                    *flag = FAILURE;
                    fprintf(stderr, "which: command not found: %s\n", tokens[1]);
                }
            }
        }
        else if (strcmp(tokens[0], "exit") == 0)
        {
            // exit: indicates that mysh should cease reading commands and terminate
            // free memory allocated for tokens
            for (int i = 0; tokens[i] != NULL; i++)
            {
                free(tokens[i]);
            }
            free(tokens);
            free(args);
            printf("Exiting my shell.\n");
            exit(EXIT_SUCCESS);
        }
        else
        {
            // fork and execute the external command
            pid_t pid = fork();
            if (pid == 0)
            { // child process
                // setup redirection in the child process
                if (fd_out != -1)
                {
                    dup2(fd_out, STDOUT_FILENO);
                    close(fd_out);
                }
                if (fd_in != -1)
                {
                    dup2(fd_in, STDIN_FILENO);
                    close(fd_in);
                }
                // execute command with args where redirection symbols and files have been removed
                char *command_path = find_command_path(args[0]);
                if (command_path)
                {
                    execv(command_path, args);
                    free(command_path); // execv only returns if there is an error
                }
                fprintf(stderr, "Command not found or execution failed\n");
                exit(EXIT_FAILURE);
            }
            else if (pid > 0)
            { // parent process, waiting for completion of child process
                int status;                   // parent process, waiting for completion of child process
                waitpid(pid, &status, 0); // wait for child process to finish
                if (WIFEXITED(status) && WEXITSTATUS(status) == 0){*flag = SUCCESS;}
                else{*flag = FAILURE;}
            }
            else
            {
                // fork fails
                fprintf(stderr, "Fork failed\n");
                *flag = FAILURE;
                exit(EXIT_FAILURE);
            }

            // reset file descriptors in the parent process and reset redirection
            if (fd_out != -1)
            {
                close(fd_out);
            }
            if (fd_in != -1)
            {
                close(fd_in);
            }
            dup2(stdout_og, STDOUT_FILENO);
            dup2(stdin_og, STDIN_FILENO);
            close(stdout_og);
            close(stdin_og);
        }

        // free memory allocated for arguments and tokens
        free(args);
        for (int i = 0; tokens[i] != NULL; i++)
        {
            free(tokens[i]);
        }
        free(tokens);

        // sets flag to indicate success or failure based on command execution
        *flag = SUCCESS; // or FAILURE based on the outcome of the command execution
    }
}