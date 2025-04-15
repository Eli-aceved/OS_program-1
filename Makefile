
CC = gcc
CFLAGS = -std=c99 -D_XOPEN_SOURCE=700 -Wall -Wextra -pedantic -O2
LIBS = 

OBJS = 

all: slosh

slosh: SLOsh.c 
	$(CC) $(CFLAGS) -o SLOsh SLOsh.c $(OBJS) $(LIBS)

clean: 
	rm -f SLOsh *.o