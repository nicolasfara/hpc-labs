EXE_CUDA:=$(basename $(wildcard cuda-*.cu))
EXE:=$(EXE_CUDA)
NVCC:=nvcc
NVCFLAGS:=-Wno-deprecated-gpu-targets

ALL: $(EXE)

cuda-matsum: NVCFLAGS+=-lm

% : %.cu
	$(NVCC) $(NVCFLAGS) $< -o $@

.PHONY: clean

clean:
	\rm -f *~ $(EXE) rule30.pbm
