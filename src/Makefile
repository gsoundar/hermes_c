# 
# Hermes
# ------
# by Gokul Soundararajan
#
# Makefile for Hermes C edition
#

all: clean libhermes.a test

libhermes.a: hermes.o tpool.o hashtab.o md5.o
	ar rcs libhermes.a *.o

hermes.o:
	cd hms; make
	cp hms/*.o .
tpool.o:
	cd tpool; make
	cp tpool/*.o .
hashtab.o:
	cd hashtab; make
	cp hashtab/*.o .
md5.o:
	cd md5; make
	cp md5/*.o .

test:
	cd tests; make

clean:
	rm -f *.o *.exe *.a *.so *.*~ *~ include/*.*~ include/*~
	cd hms; make clean
	cd tpool; make clean
	cd hashtab; make clean
	cd tests; make clean