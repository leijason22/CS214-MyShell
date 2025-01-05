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

#define MAX_COMMAND_LENGTH 1024
int flag = 0;

int main(int argc, char *argv[])
{
    int interactive = isatty(STDIN_FILENO);

    if (argc == 1)
    {
        if (interactive)
            printf("Welcome to my shell!\n");

        // interactive mode
        if (interactive)
        {
            char command[MAX_COMMAND_LENGTH];
            ssize_t bytes_read;

            while (interactive)
            {
                write(STDOUT_FILENO, "mysh> ", strlen("mysh> "));
                bytes_read = read(STDIN_FILENO, command, MAX_COMMAND_LENGTH - 1);
                if (bytes_read == -1)
                {
                    perror("read");
                    exit(EXIT_FAILURE);
                }

                // end of file
                if (bytes_read == 0)
                    break;

                // null terminate command
                command[bytes_read] = '\0';

                // remove new line character
                if (command[bytes_read - 1] == '\n')
                    command[bytes_read - 1] = '\0';

                execute_command(command, &flag);
            }
        }
    }
    else if (argc > 1)
    { // batch mode
        int fd = open(argv[1], O_RDONLY);
        if (fd == -1)
        {
            fprintf(stderr, "Error opening file: %s\n", argv[1]);
            exit(EXIT_FAILURE);
        }

        char command[MAX_COMMAND_LENGTH];
        ssize_t bytes_read;

        // reads and executes each command one by one until the end of the file
        while ((bytes_read = read(fd, command, MAX_COMMAND_LENGTH - 1)) > 0)
        {
            // null terminates command string
            command[bytes_read] = '\0';

            char *next_command = command;
            while (next_command)
            {
                char *newline = strchr(next_command, '\n');
                if (newline)
                    *newline = '\0'; // terminates current command after finished

                execute_command(next_command, &flag);

                if (newline)
                    next_command = newline + 1;
                else
                    break;
            }
        }
        if (bytes_read == -1)
        {
            perror("read");
            close(fd);
            exit(EXIT_FAILURE);
        }

        close(fd);
    }
    else
    {
        fprintf(stderr, "Usage: %s [batch_file]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    return 0;
}
