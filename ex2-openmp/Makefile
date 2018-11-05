EXE:=$(basename $(wildcard omp-*.c))
CFLAGS+=-std=c99 -Wall -Wpedantic -fopenmp

ALL: $(EXE)

.PHONY: clean

clean:
	\rm -f $(EXE) *.o *~
