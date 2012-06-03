#
# HERMES - TESTS
# by Gokul Soundararajan
#
# Makefile
#

CC=gcc
CFLAGS=-Wall -g -O3 
CLIBS=-lpthread -lhermes
INCLUDE_DIR="../include"
LIBDIR="../"

all: clean tests

tests: test.exe msg_test1.exe parser_test1.exe copy_test

test.exe: hermes_test.c
	${CC} ${CFLAGS} -I${INCLUDE_DIR} hermes_test.c -L${LIBDIR} -lhermes -o test.exe ${CLIBS}
msg_test1.exe: hms_msg_test1.c
	${CC} ${CFLAGS} -I${INCLUDE_DIR} hms_msg_test1.c -L${LIBDIR} -lhermes -o msg_test1.exe ${CLIBS}
parser_test1.exe: hms_parser_test1.c
	${CC} ${CFLAGS} -I${INCLUDE_DIR} hms_parser_test1.c -L${LIBDIR} -lhermes -o parser_test1.exe ${CLIBS}

# copy program
copy_test: copy_client.exe copy_server.exe
copy_server.exe: copy_server.c
	${CC} ${CFLAGS} -I${INCLUDE_DIR} copy_server.c -L${LIBDIR} -lhermes -o copy_server.exe ${CLIBS}
copy_client.exe: copy_client.c
	${CC} ${CFLAGS} -I${INCLUDE_DIR} copy_client.c -L${LIBDIR} -lhermes -o copy_client.exe ${CLIBS}
clean:
	rm -f *.o *.exe *.a *.so *.*~ *~

