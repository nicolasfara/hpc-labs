EXE:=$(basename $(wildcard omp-*.c))
CFLAGS+=-std=c99 -Wall -Wpedantic -fopenmp -I/usr/include/tirpc -ltirpc -lcrypt -lc

ALL: $(EXE)

omp-pi: LDLIBS+=-lm

.PHONY: clean

clean:
	\rm -f $(EXE) *.o *~
