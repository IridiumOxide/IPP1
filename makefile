CC=gcc
CFLAGS=-Wall -O2 -std=c11

all: dictionary

debug: dictionary.dbg
	
dictionary: dictionary.o parse.o trie.o
	$(CC) -o dictionary dictionary.o parse.o trie.o $(CFLAGS)

dictionary.o: dictionary.c
	$(CC) -c dictionary.c $(CFLAGS)

parse.o: parse.c parse.h
	$(CC) -c parse.c $(CFLAGS)

trie.o: trie.c trie.h
	$(CC) -c trie.c $(CFLAGS)

dictionary.dbg: dictionary.o parse.o trie.o
	$(CC) -g -o dictionary  dictionary.o parse.o trie.o $(CFLAGS)
	
.PHONY: clean

clean:
		rm *.o dictionary dictionary.dbg