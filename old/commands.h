#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <limits.h>

// Function to parse a command and return an array of tokens
char** parse_command(char *command);

// Function to execute a parsed command
void execute_command(char *command);