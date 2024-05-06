CFLAGS = gcc -Wall -g -std=c11 -pthread

all: A2checker

A2checker: A2checker.c 
	$(CFLAGS) A2checker.c -o A2checker
val: A2checker
	valgrind --leak-check=full --track-origins=yes ./A2checker
clean:
	rm -f A2checker