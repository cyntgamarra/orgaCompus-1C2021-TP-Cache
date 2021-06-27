PROGS = cache
CC = gcc
CFLAGS = -Wall -std=c99 -g

all: clean c

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<
	
clean:
	rm -f $(PROGS) *.so *.o *.a *.core

c: 
	$(CC) $(CFLAGS) $(PROGS).c -o $(PROGS)