CFLAGS = gcc -Wall -g -std=c11 -pthread

all: checker

checker: Spellchecker.c 
	$(CFLAGS) Spellchecker.c -o checker
val: checker
	valgrind --leak-check=full --track-origins=yes ./checker
clean:
	rm -f checker
