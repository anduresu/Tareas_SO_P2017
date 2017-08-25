# Descargue el nSystem desde:
# https://users.dcc.uchile.cl/~lmateu/CC4302/download/nsystem64-beta2.tgz
# Ud. debe descomprimir este archivo con:
# $ tar zxvf nsystem64-beta2.tgz
# Compile el nSystem con:
# $ cd nsystem64-beta2/src
# $ make

# Importante:
# Para compilar exitosamente su tarea necesita definir la variable de
# ambiente NSYSTEM con:
# $ export NSYSTEM=.../nsystem64-beta2
# (debe reemplazar ... por la ruta para llegar a nsystem64-beta2)

# Programe su tarea en el archivo batch.c y luego pruebela con:
# $ cd <el directorio en donde se encuentra este archivo>
# $ make test-batch
# $ ./test-batch

# Programe su test de prueba personal en test2-batch.c y pruebelo con:
# $ make test2-batch
# $ ./test2-batch2

LIBNSYS= $(NSYSTEM)/lib/libnSys.a

# Con opciones de debugging
CFLAGS= -Wall -pedantic -std=c99 -g -I$(NSYSTEM)/include -I$(NSYSTEM)/src
LFLAGS= -g

# Con opciones de optimizacion:
#CFLAGS= -O2 -I$(NSYSTEM)/include -I$(NSYSTEM)/src
#LFLAGS=

test-batch: test-batch.o batch.o
	gcc -g test-batch.o batch.o -o test-batch $(LIBNSYS)

test2-batch: test2-batch.o batch.o
	gcc -g test2-batch.o batch.o -o test2-batch $(LIBNSYS)

test-batch.o test2-batch.o batch.o: batch.h

clean:
	rm -f *.o test-batch test2-batch
