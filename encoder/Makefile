.PHONY: all
all: encoder

encoder: encoder.c
	gcc -Wall -lsndfile -o encoder encoder.c

.PHONY: clean
clean:
	rm encoder
