all:
	make rshell;
rshell:
	mkdir bin; g++ src/shell.c -Wall -Werror -ansi -pedantic -o bin/rshell 
