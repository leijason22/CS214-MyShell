#include "commands.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

// Function to determine if the shell is running in interactive mode
// int is_interactive_mode() {
//     return isatty(STDIN_FILENO);
// }

#define MAX_COMMAND_LENGTH 1024

int main(int argc, char *argv[]) {
    int interactive = isatty(STDIN_FILENO);

    if (interactive)
        printf("Welcome to my shell!\n");

    if (interactive) { // Interactive mode
        char command[MAX_COMMAND_LENGTH];
        ssize_t bytes_read;

        while (interactive) {
            // printf("mysh> ");
            //flushes output buffer, ensures mysh> is displayed/prompted immediately
            //CANT USE FFLUSH
            // fflush(stdout);
            write(STDOUT_FILENO, "mysh> ", strlen("mysh> "));
            bytes_read = read(STDIN_FILENO, command, MAX_COMMAND_LENGTH - 1);
            if (bytes_read == -1) {
                perror("read");
                exit(EXIT_FAILURE);
            }

            //end of file
            if (bytes_read == 0)
                break;

            //null terminate command
            command[bytes_read] = '\0';

            //remove new line character
            if (command[bytes_read - 1] == '\n')
                command[bytes_read - 1] = '\0';

            execute_command(command);
        }
    } else if (!interactive) { // Batch mode

        int fd = open(argv[1], O_RDONLY);
        if (fd == -1) {
            fprintf(stderr, "Error opening file: %s\n", argv[1]);
            exit(EXIT_FAILURE);
        }

        char command[MAX_COMMAND_LENGTH];
        ssize_t bytes_read = read(fd, command, MAX_COMMAND_LENGTH - 1);
        if (bytes_read == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        // Null-terminate the command string
        command[bytes_read] = '\0';

        // Execute the command
        execute_command(command);

        close(fd);
    } else {
        fprintf(stderr, "Usage: %s [batch_file]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (interactive)
        printf("Exiting my shell.\n");

    return 0;
}

// #define MAX_COMMAND_LENGTH 1024

// void interactive_mode();
// void batch_mode(const char *filename);

// int main(int argc, char *argv[]) {
//     int interactive = isatty(STDIN_FILENO);

//     if (interactive) {
//         printf("Welcome to my shell!\n");
//         interactive_mode();
//     } else if (!interactive){
//         batch_mode(argv[1]);
//     } else {
//         fprintf(stderr, "Usage: %s [batch_file]\n", argv[0]);
//         exit(EXIT_FAILURE);
//     }
//     return 0;
// }

// void interactive_mode() {
//     char command[MAX_COMMAND_LENGTH];
//     ssize_t bytes_read;

//     printf("mysh> ");

//     while ((bytes_read = read(STDIN_FILENO, command, MAX_COMMAND_LENGTH - 1)) > 0) {
//         command[bytes_read] = '\0'; // Null-terminate the command
//         if (bytes_read == 1 && command[0] == '\n') {
//             printf("mysh> ");
//             continue; // Skip execution if only a newline was entered
//         }
//         execute_command(command);
//         // printf("mysh> ");
//     }

    // printf("mysh: exiting\n");
    
    // char command[MAX_COMMAND_LENGTH];
    // ssize_t bytes_read;

    // printf("mysh> ");

    // while ((bytes_read = read(STDIN_FILENO, command, MAX_COMMAND_LENGTH)) > 0) {
    //     execute_command(command);
    //     printf("mysh> ");
    // }

    // printf("mysh: exiting\n");
// }

// void batch_mode(const char *filename) {
//     int fd = open(filename, O_RDONLY);
//     if (fd == -1) {
//         perror("Error opening file");
//         exit(EXIT_FAILURE);
//     }

//     char command[MAX_COMMAND_LENGTH];
//     ssize_t bytes_read;

//     while ((bytes_read = read(fd, command, MAX_COMMAND_LENGTH)) > 0) {
//         execute_command(command);
//     }

//     close(fd);
// }

// int main(int argc, char *argv[]) {
//     int interactive = isatty(STDIN_FILENO);

//     if (interactive)
//         printf("Welcome to my shell!\n");

//     if (interactive) { // Interactive mode
//         while (interactive) {
//             char *command = NULL;
//             size_t command_size = 0;
//             ssize_t bytes_read;

//             if (interactive)
//                 printf("mysh> ");
//             bytes_read = getline(&command, &command_size, stdin);
//             if (bytes_read == -1) {
//                 free(command);
//                 break;
//             }

//             if (command[bytes_read - 1] == '\n')
//                 command[bytes_read - 1] = '\0';

//             // if (strcmp(command, "exit") == 0) {
//             //     printf("mysh: exiting\n");
//             //     free(command);
//             //     break;
//             // }

//             execute_command(command);

//             free(command);
//         }
//     } else if (!interactive) { // Batch mode

//         FILE *batch_file = fopen(argv[1], "r");
//         if (!batch_file) {
//             fprintf(stderr, "Error opening file: %s\n", argv[1]);
//             exit(EXIT_FAILURE);
//         }

//         char *line = NULL;
//         size_t line_size = 0;
//         ssize_t read;

//         while ((read = getline(&line, &line_size, batch_file)) != -1) {
//             if (line[read - 1] == '\n')
//                 line[read - 1] = '\0';

//             execute_command(line);
//         }

//         free(line);
//         fclose(batch_file);
//     } else {
//         fprintf(stderr, "Usage: %s [batch_file]\n", argv[0]);
//         exit(EXIT_FAILURE);
//     }

//     if (interactive)
//         printf("Exiting my shell.\n");

//     return 0;
// }