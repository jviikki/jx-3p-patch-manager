CFLAGS=-Wall
LIBS=-lsndfile
OBJS=encoder.o audio.o

.PHONY: all
all: encoder

audio.o: audio.c
	gcc $(CFLAGS) -c audio.c

encoder.o: encoder.c audio.h
	gcc $(CFLAGS) -c encoder.c

encoder: $(OBJS)
	gcc $(CFLAGS) $(LIBS) -o encoder $(OBJS)

.PHONY: clean
clean:
	rm -f encoder
	rm -f *.o