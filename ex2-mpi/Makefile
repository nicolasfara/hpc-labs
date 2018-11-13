EXE_MPI:=$(basename $(wildcard mpi-*.c))
EXE:=$(EXE_MPI) bbox-gen

ALL: $(EXE)

.PHONY: clean

CFLAGS+=-std=c99 -Wall -Wpedantic

$(EXE_MPI): CC=mpicc

mpi-bbox: LDLIBS+=-lm

clean:
	\rm -f *~ $(EXE) mandelbrot.ppm
