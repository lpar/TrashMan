CC=gcc
CFLAGS=-I. -std=c99
DEPS=debug.h
OBJ=trashman.o

%.o: %.c $(DEPS)
		$(CC) -c -o $@ $< $(CFLAGS)

trashman: $(OBJ)
		gcc -o $@ $^ $(CFLAGS)
