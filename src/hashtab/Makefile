#
# HASHTABLE
# -----------
# Makefile
#

CC=gcc
CFLAGS=-g -Wall -O3 -fPIC
CLIBS=-lpthread -lm -lc
INCLUDE_DIR="../include"

all: clean hashtab.o

hashtab.o: hashtable.c
	${CC} ${CFLAGS} -I${INCLUDE_DIR} -c hashtable.c -o hashtab.o
clean:
	rm -f *.o *.a *.so *.*~ *~
