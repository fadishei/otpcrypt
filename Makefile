CFLAGS=-Wall
CC=gcc

all: otpcrypt

install: all
	install -m 755 otpcrypt /usr/local/bin/
	
otpcrypt: otpcrypt.o
	$(CC) $(CFLAGS) otpcrypt.o -o otpcrypt

otpcrypt.o: otpcrypt.c dcp_bootstream_ioctl.h
	$(CC) $(CFLAGS) -c otpcrypt.c
	
clean:
	rm -f otpcrypt *.o
  