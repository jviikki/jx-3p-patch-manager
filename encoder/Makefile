CFLAGS=-Wall
LIBS=-lsndfile
READER_OBJS=reader.o
WRITER_OBJS=writer.o audio.o

.PHONY: all
all: reader writer

audio.o: audio.c
	gcc $(CFLAGS) -c audio.c

reader.o: reader.c
	gcc $(CFLAGS) -c reader.c

writer.o: writer.c audio.h
	gcc $(CFLAGS) -c writer.c

reader: $(READER_OBJS)
	gcc $(CFLAGS) $(LIBS) -o reader $(READER_OBJS)

writer: $(WRITER_OBJS)
	gcc $(CFLAGS) $(LIBS) -o writer $(WRITER_OBJS)

.PHONY: clean
clean:
	rm -f reader writer
	rm -f *.o