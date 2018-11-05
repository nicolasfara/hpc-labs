EXE:=$(basename $(wildcard *.c))
CFLAGS+=-std=c99 -Wall -Wpedantic -fopenmp

ALL: $(EXE)

c-ray: LDLIBS+=-lm

.PHONY: clean

clean:
	\rm -f $(EXE) *.o *~
