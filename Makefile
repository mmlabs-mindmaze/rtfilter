CC=gcc
LD=gcc
LDFLAGS=-lm -lrt $(PFLAGS)
CFLAGS=-march=native -msse2 -mfpmath=sse -O3 -I. -W -Wall -g3 $(PFLAGS)

all: filsse
	

filsse: filter-sse.o test_filter.o common-filters.o
	$(LD) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) *.o *.s *.i
	$(RM) filsse
