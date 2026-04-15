CC     = gcc
CFLAGS = -Wall -Wextra -std=c11

OBJ = main.o cartas.o interface.o

simple_simon: $(OBJ)
	$(CC) $(CFLAGS) -o simple_simon $(OBJ)

main.o: main.c cartas.h interface.h
	$(CC) $(CFLAGS) -c main.c

cartas.o: cartas.c cartas.h
	$(CC) $(CFLAGS) -c cartas.c

interface.o: interface.c interface.h cartas.h
	$(CC) $(CFLAGS) -c interface.c

clean:
	rm -f *.o simple_simon