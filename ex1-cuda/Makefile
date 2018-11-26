EXE_CUDA:=$(basename $(wildcard cuda-*.cu))
EXE:=$(EXE_CUDA)
NVCC=nvcc
NVCFLAGS=-Wno-deprecated-gpu-targets

ALL: $(EXE)

cuda-dot cuda-dot-devicemem cuda-dot-shared: NVLDLIBS+=-lm

% : %.cu
	$(NVCC) $(NVCFLAGS) $< $(NVLDLIBS) -o $@

.PHONY: clean

clean:
	\rm -f *~ $(EXE)
