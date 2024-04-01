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

// int main(int argc, char *argv[]) {
//     // Determine if running in interactive mode
//     int interactive = isatty(STDIN_FILENO);

//     // Print welcome message in interactive mode
//     if (interactive)
//         printf("Welcome to my shell!\n");

//     // Main loop to read and execute commands
//     while (1) {
//         char *command = NULL;
//         size_t command_size = 0;
//         ssize_t bytes_read;

//         // Read command from stdin or file
//         if (interactive)
//             printf("mysh> ");
//         bytes_read = getline(&command, &command_size, stdin);
//         if (bytes_read == -1) {
//             free(command);
//             break; // End of input
//         }

//         // Remove trailing newline character
//         if (command[bytes_read - 1] == '\n')
//             command[bytes_read - 1] = '\0';

//         // Execute command
//         execute_command(command);

//         free(command);
//     }

//     // Print goodbye message in interactive mode
//     if (interactive)
//         printf("Exiting my shell.\n");

//     return 0;
// }

int main(int argc, char *argv[]) {
    int interactive = isatty(STDIN_FILENO);

    if (interactive)
        printf("Welcome to my shell!\n");

    if (argc == 1) { // Interactive mode
        while (1) {
            char *command = NULL;
            size_t command_size = 0;
            ssize_t bytes_read;

            if (interactive)
                printf("mysh> ");
            bytes_read = getline(&command, &command_size, stdin);
            if (bytes_read == -1) {
                free(command);
                break;
            }

            if (command[bytes_read - 1] == '\n')
                command[bytes_read - 1] = '\0';

            // if (strcmp(command, "exit") == 0) {
            //     printf("mysh: exiting\n");
            //     free(command);
            //     break;
            // }

            execute_command(command);

            free(command);
        }
    } else if (argc == 2) { // Batch mode
        int interactive = 0; //batch mode is not interactive

        FILE *batch_file = fopen(argv[1], "r");
        if (!batch_file) {
            fprintf(stderr, "Error opening file: %s\n", argv[1]);
            exit(EXIT_FAILURE);
        }

        char *line = NULL;
        size_t line_size = 0;
        ssize_t read;

        while ((read = getline(&line, &line_size, batch_file)) != -1) {
            if (line[read - 1] == '\n')
                line[read - 1] = '\0';

            execute_command(line);
        }

        free(line);
        fclose(batch_file);
    } else {
        fprintf(stderr, "Usage: %s [batch_file]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (interactive)
        printf("Exiting my shell.\n");

    return 0;
}
