CC = gcc
CFLAGS = -Wall

all: user

user: user.o
	$(CC) $(CFLAGS) user.c -o user

clean:
	rm -rf user *.o

run:
	./user
