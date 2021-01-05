CC=gcc
FLAG=-g
OBJS=ftp.c command.c
SERVER=myFtpServer.c

all:
	$(CC) $(FLAG) -c $(OBJS) 
	
	$(CC) $(FLAG) $(SERVER) -lpthread ftp.o command.o -o server

clean:
	rm -f ftp.o 
	rm -f command.o
	rm -f server
