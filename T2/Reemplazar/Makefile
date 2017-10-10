# Para Linux/amd64
SYSDEP= nStack-amd64.o
MAKELIB= ar-ranlib
DEFINES=

# Para Linux/i386
# SYSDEP= nStack-i386.o
# MAKELIB= ar-ranlib
# DEFINES=

#------ fin parte parte dependiente -----

OBJS= nProcess.o nLRLock.o nTime.o nMsg.o nSem.o nMonitor.o nIO.o nDep.o \
         nMain.o nQueue.o nOther.o fifoqueues.o $(SYSDEP)
LIBNSYS= libnSys.a

CFLAGS= -ggdb -Wall -pedantic -I../include $(DEFINES)
LFLAGS= -ggdb 
#CFLAGS= -O2 -I../include $(DEFINES)
#LFLAGS=

# Para Linux x86 en maquinas de 64 bits (amd64)
#CFLAGS= -ggdb -m32 -I../include $(DEFINES)
#LFLAGS= -ggdb -m32

all: $(LIBNSYS)

.SUFFIXES:
.SUFFIXES: .o .c .s

.c.o .s.o:
	gcc -c $(CFLAGS) $<

$(OBJS): ../include/nSystem.h nSysimp.h

$(LIBNSYS): $(OBJS) $(MAKELIB)
	rm -rf $(LIBNSYS)
	sh $(MAKELIB) $@ $(OBJS)

clean:
	rm -f *.o *~
