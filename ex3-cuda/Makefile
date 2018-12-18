EXE_CUDA:=$(basename $(wildcard cuda-*.cu))
EXE:=$(EXE_CUDA) knapsack-gen
NVCC:=nvcc
NVCFLAGS+=-Wno-deprecated-gpu-targets

ALL: $(EXE)

cuda-knapsack: NVCFLAGS+=-lm

% : %.cu
	$(NVCC) $(NVCFLAGS) $< -o $@

.PHONY: clean

clean:
	\rm -f *~ $(EXE) anneal-*.pbm
