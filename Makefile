# Makefile for Huffman

TGT := huffman
SRC := huffman.c
OBJ := $(SRC:.c=.o)

CC := gcc
CFLAGS := -g -Wall -o3 -std=c99

all: $(TGT) run

$(TGT): $(OBJ)

dep:
	$(CC) -M *.c > depend

clean:
	@rm -f $(TGT) *.o depend B.txt
    
run:
	./huffman -c -i A.txt -o t.ddl #> /dev/null
	./huffman -u -i t.ddl -o B.txt #> /dev/null
	@diff -s A.txt B.txt && rm -f t.ddl