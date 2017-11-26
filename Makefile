CC = gcc
CXX = mpicxx
LDFLAGS = -fopenmp -pthread -lm
CFLAGS = -O3 -std=gnu99 -pthread -lm
CXXFLAGS = -O3 -std=gnu++11
TARGETS = APSP_MPI_sync APSP_MPI_async
PTHREAD = APSP_Pthread

.PHONY: all
all: $(TARGETS) $(PTHREAD)

.PHONY: clean
clean:
	rm -f $(TARGETS) $(TARGETS:=.o) $(PTHREAD)
