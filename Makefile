PROGS = tp2
CC = gcc
CFLAGS = -Wall -std=gnu99 -g

all: clean c run

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<
	
clean:
	rm -f $(PROGS) *.so *.o *.a *.core

c: 
	$(CC) $(CFLAGS) $(PROGS).c -o $(PROGS)
	
run:
	valgrind ./$(PROGS) -w 4 -c 8 -b 16 pruebas/prueba1.mem
