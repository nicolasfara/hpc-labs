EXE_SIMD:=$(basename $(wildcard simd-*.c))
EXE:=$(EXE_SIMD)
CFLAGS+=-std=c99 -Wall -Wpedantic -march=native -g -ggdb -O2 -D_XOPEN_SOURCE=600

ALL: $(EXE_SIMD) $(EXE_SERIAL)

simd-dot: LDLIBS+=-lm

clean:
	\rm -f $(EXE) *.o *~ *.pbm *.s
