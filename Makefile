CC=gcc
LD=gcc
LDFLAGS=-lm -lrt
CFLAGS=-march=native -msse2 -mfpmath=sse -O3 -I. -W -Wall -save-temps

all: filsse
	

filsse: filter-sse.o test_filter.o common-filters.o
	$(LD) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) *.o
	$(RM) filsse
