#
# THREAD POOL
# -----------
# Makefile
#

CC=gcc
CFLAGS=-g -Wall -O3 -fPIC
CLIBS=-lpthread -lm -lc
INCLUDE_DIR="../include"

all: tpool.o

tpool.o: tpool.c
	${CC} ${CFLAGS} -I${INCLUDE_DIR} -c tpool.c -o tpool.o
clean:
	rm -f *.o *.a *.so *.*~ *~
