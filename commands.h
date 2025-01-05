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

void wildcards(char ***tokens, int *token_count, int *size);
char **parse_command(char *command);
char *find_command_path(const char *command);
void execute_piped_commands(char **first_command, char **second_command);
void execute_command(char *command, int *flag);