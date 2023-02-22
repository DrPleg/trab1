CC = gcc

CFLAGS = -Wall
DBGFLAGS = -g -DDEBUG
#LDLIBS

objects = messenger.o
exec = messenger

all: messenger

debug: messengerdebug

messenger: messenger.o
	$(CC) messenger.o -o messenger $(CFLAGS)
# gcc messenger.o -o messenger

messengerdebug: messenger.odebug
	$(CC) messenger.o -o messenger $(CFLAGS) $(DBGFLAGS)

messenger.o: messenger.c
	$(CC) -c messenger.c -o messenger.o $(CFLAGS)

messenger.odebug: messenger.c
	$(CC) -c messenger.c -o messenger.o $(CFLAGS) $(DBGFLAGS)
# gcc -c messenger.c -o messenger.o

clean:
	-rm -f $(objects)
# -rm -f *.o

purge: clean
	-rm -f $(exec)
# -rm -f messenger