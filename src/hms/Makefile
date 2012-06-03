# 
# Hermes
# ------
# by Gokul Soundararajan
#
# Makefile for Hermes C edition
#

CC=gcc
CFLAGS=-Wall -g -O4 -fPIC
CLIBS=-lpthread
INCLUDE_DIR="../include"

all: clean hermes.o hms_parser.o hms_msg.o hms_util.o

hermes.o: hermes.c
	${CC} ${CFLAGS} -I${INCLUDE_DIR} -c hermes.c -o hermes.o
hms_util.o: hms_util.c
	${CC} ${CFLAGS} -I${INCLUDE_DIR} -c hms_util.c -o hms_util.o
hms_msg.o: hms_msg.c
	${CC} ${CFLAGS} -I${INCLUDE_DIR} -c hms_msg.c -o hms_msg.o
hms_parser.o: hms_parser.c
	${CC} ${CFLAGS} -I${INCLUDE_DIR} -c hms_parser.c -o hms_parser.o
clean:
	rm -f *.o *.exe *.a *.so *.*~ *~

