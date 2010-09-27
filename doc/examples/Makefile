LIBS=-lrtfilter

all: butterworth

butterworth: butterworth.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

clean: 
	$(RM) butterworth
