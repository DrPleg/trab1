CC = gcc

CFLAGS = -Wall -lcurses -lm
DBGFLAGS = -g -DDEBUG
#LDLIBS

objects = messenger.o crc.o
exec = messenger

all: messenger

debug: messengerdebug

messenger: messenger.o crc.o
	$(CC) messenger.o crc.o -o messenger $(CFLAGS)
# gcc messenger.o -o messenger

messengerdebug: messenger.odebug
	$(CC) messenger.o -o messenger $(CFLAGS) $(DBGFLAGS)

messenger.o: messenger.c
	$(CC) -c messenger.c -o messenger.o $(CFLAGS)

messenger.odebug: messenger.c
	$(CC) -c messenger.c -o messenger.o $(CFLAGS) $(DBGFLAGS)
# gcc -c messenger.c -o messenger.o

crc.o: crc.c 
	$(CC) -c crc.c -o crc.o $(CFLAGS)

crc.odebug: crc.c 
	$(CC) -c crc.c -o crc.o $(CFLAGS) $(DBGFLAGS)

clean:
	-rm -f $(objects)
# -rm -f *.o

purge: clean
	-rm -f $(exec)
# -rm -f messenger