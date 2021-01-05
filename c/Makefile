CC=gcc
FLAG=-g
OBJS=ftp.c command.c
CLIENT=Client.c

all:
	$(CC) $(FLAG) -c $(OBJS) 
	
	$(CC) $(FLAG) $(CLIENT) -lpthread ftp.o command.o -o client

clean:
	rm -f ftp.o 
	rm -f command.o
	rm -f clientt
