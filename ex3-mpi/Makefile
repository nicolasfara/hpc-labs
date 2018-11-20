EXE_MPI:=$(basename $(wildcard mpi-*.c))
EXE:=$(EXE_MPI)

ALL: $(EXE)

.PHONY: clean

CFLAGS+=-std=c99 -Wall -Wpedantic

$(EXE_MPI): CC=mpicc

clean:
	\rm -f *~ $(EXE) rule30.pbm
