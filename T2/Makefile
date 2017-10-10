# Para usar este Makefile es necesario definir la variable
# de ambiente NSYSTEM con el directorio en donde se encuentra
# la raiz de nSystem.
# En csh esto se hace con:  setenv NSYSTEM .../nsystem64-beta
# En sh: NSYSTEM= .../nsystem64-beta ; export NSYSTEM
#
# Para compilar ingrese make
#

LIBNSYS= $(NSYSTEM)/lib/libnSys.a

CFLAGS= -ggdb -I$(NSYSTEM)/include -I$(NSYSTEM)/src
LFLAGS= -ggdb

all: testlrlock

.SUFFIXES:
.SUFFIXES: .o .c .s

.c.o .s.o:
	gcc -c $(CFLAGS) $<

testlrlock: testlrlock.o $(LIBNSYS)
	gcc $(LFLAGS) $@.o -o $@ $(LIBNSYS)

clean:
	rm -f *.o *~ testlrlock
