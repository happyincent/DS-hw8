/*---------------------------------------------------------------
* Huffman by ddl
* Modified date: 2017/04/07
* To Compress: ./huffman -c -i A.txt -o t.ddl
* To Decompress: ./huffman -u -i t.ddl -o B.txt
*---------------------------------------------------------------
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#define DIE(...)  fprintf(stderr, __VA_ARGS__), exit(errno)

#define CHAR_SIZE 256
#define CHAR_BIT 8

/* Data Structure */
typedef struct node {
    int freq;
    int ch;
    struct node *left, *right;
} Huffnode;

typedef struct stacknode {
    Huffnode *Hnode;
    struct stacknode *next;
} Stacknode;

/* Huffman Function */
Huffnode *newnode(int f, int ch, Huffnode *L, Huffnode *R);
void huffmancode(Huffnode *root, char *tmp, char code[][CHAR_SIZE]);
void huffmantree(int *freq, Stacknode *top);

/* priority stack */
void push(Huffnode *Hnode, Stacknode *top);
Huffnode *pop(Stacknode *top);
Stacknode *newStack();
void delStack(Stacknode *top);

/* Main Function */
void compress(FILE *fin, FILE *fout, int *original_bytes, int *archive_bytes);
void decompress(FILE *fin, FILE *fout, int *original_bytes, int *archive_bytes);