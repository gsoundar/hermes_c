#
# THREAD POOL
# -----------
# Makefile
#

CC=gcc
CFLAGS=-g -Wall -O3 -fPIC
CLIBS=-lpthread -lm -lc
INCLUDE_DIR="../include"

all: clean md5.o

md5.o: md5.c
	${CC} ${CFLAGS} -I${INCLUDE_DIR} -c md5.c -o md5.o
clean:
	rm -f *.o *.a *.so *.*~ *~
