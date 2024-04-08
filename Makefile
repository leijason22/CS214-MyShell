CC = gcc
CFLAGS = -Wall -Wextra -g -fsanitize=address,undefined -DREALMALLOC -Wvla

mysh: mysh.o commands.o
	$(CC) $(CFLAGS) -o mysh mysh.o commands.o

mysh.o: mysh.c
	$(CC) $(CFLAGS) -c mysh.c

commands.o: commands.c
	$(CC) $(CFLAGS) -c commands.c

clean:
	rm -f mysh mysh.o commands.o