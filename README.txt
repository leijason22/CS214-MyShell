Systems Programming - Project 3: My Shell
Nelson Li, njl117
Jason Lei, jjl330

Program Overview

This program implements a simple shell in C that parses command-line input to handle wildcard expansion, input/output redirection, 
piping between commands, and execution of both built-in (like cd, pwd, exit) and external commands.

There are two main source files that manage different aspects of the project:
    1. mysh.c - the main driver file that executes a simple shell that can operate in an interactive mode, displaying a prompt and reading commands from the user, 
    or a batch mode, executing commands from a given file.
    2. commands.c - library source file that provides functions to parse commands, handle wildcards, execute commands with piping and/or redirection.

Important Notes and Contents of the Program
• Posix (unbuffered) stream IO
• Reading and changing the working directory
• Spawning child processes and obtaining their exit status
• Use of dup2() and pipe()
• Reading the contents of a directory

isatty(), mysh will determine whether to run in interactive mode or batch mode.

The command itself is made of tokens. For our purposes, a token is a sequence of non-whitespace characters, except that >, <, and | are always a token on their own. 
Thus, a string foo bar<baz consists of four tokens: “foo”, “bar”, “<”, and “baz”.

Wildcards
A single asterisk (*) represents a set of files whose names match a pattern.
Patterns contain only one asterisk, which is always in the final path segment.
Wildcards do not match names beginning with a period (hidden files).


Redirection
< and > are used to specify files for standard input and output, respectively.
Tokens following < or > are treated as file paths and not included in the argument list.


Pipes
A single pipe | connects two processes, allowing data to flow from one to the other.
dup2() is used to set standard output of the first process to the write end of the pipe and standard input of the second process to the read end.
Built-in Commands
cd: Change the working directory.
pwd: Print the current working directory.
which: Print the path that mysh would use if asked to start a program.
exit: Terminate mysh.


Test cases:
Built-In Cmds
cd: after using cd to go into other directories, printed pwd to make sure it was in right directory
cd subdir
(while in subdir) cd subdir3


pwd: printed pwd in different directories, like subdir, to see if it worked
pwd


which:
Which takes in a command like ls, and print out the path to the cmd
which ls


exit: leaves the shell , printing "exiting my shell"

running ./mysh myscript.sh:
    - tests batch mode, as well as the built-in commands developed

running external commands
ls: prints all files in current working directory
cat myscript.sh: prints out contents of myscript.sh

Wildcards
ran ls and commands like cat using wildcards as argument
made multiple txt files and then did *.txt with ls or cat to make sure it prints out all of them
ls *.txt


Conditionals
Implemented by idetifying the first code if "then" or "else". Running cmd if then , and flag works
examples:
ls
then ls (works!)
else ls (fails)

randomInvalidCommand
then pwd (fails)
else ls (works!)

Redirection
< uses the next argument as the input to the file 
> outputs the program to that file

wc -w < myscript.sh
(should print out the number of words in myscript.sh)

cat myscript.sh > out.sh
(should print all the contents of myscript.sh in out.sh, and create out.sh if not already present)

sort < myscript.sh > sorted.sh
(should sort everything in myscript.sh alphabetically and the output should be in sorted.sh (created if not already present))

Pipes
ls | grep *.sh
(will display all files that end in .sh in the current working directory)

Limitations: 
A few kinks with extreme test cases where it ran successfully, but had a memory leak somewhere

