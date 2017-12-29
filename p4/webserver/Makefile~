#
# To compile, type "make" or make "all"
# To remove files, type "make clean"
#
OBJS = server.o request.o cs537.o client.o server_none.o server_cond.o server_sema.o
TARGET = server

CC = gcc
CFLAGS = -g -Wall

LIBS = -lpthread 

.SUFFIXES: .c .o 

all: server client output.cgi

server: server_sema
	cp $< $@
server_none: server.o request.o cs537.o server_none.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
	cp $@ server
server_cond: server.o request.o cs537.o server_cond.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
	cp $@ server
server_sema: server.o request.o cs537.o server_sema.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
	cp $@ server

client: client.o cs537.o
	$(CC) $(CFLAGS) -o client client.o cs537.o

output.cgi: output.c
	$(CC) $(CFLAGS) -o output.cgi output.c

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	-rm -f $(OBJS) server client output.cgi server_none server_cond server_sema
