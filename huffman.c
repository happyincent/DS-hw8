#include "huffman.h"

int main(int argc, char *argv[])
{
    if (  argc != 6 || 
         (strcmp(argv[1], "-c") && strcmp(argv[1] ,"-u")) || 
          strcmp(argv[2], "-i") || strcmp(argv[4], "-o")  ) {
        DIE("Usage: ./huffman [-c|-u] [-i] FileIN [-o] FileOUT\n\n");
    }
    
    FILE *fin = fopen(argv[3], "rb");
    FILE *fout = fopen(argv[5], "wb");
    if (fin == NULL || fout == NULL)
        DIE("Fail to open file.\n\n");
    
    int original_bytes = 0, archive_bytes = 0;
    
    if ( strcmp(argv[1], "-c") == 0 )
        compress(fin, fout, &original_bytes, &archive_bytes);
    else if ( strcmp(argv[1], "-u") == 0 )
        decompress(fin, fout, &original_bytes, &archive_bytes);
    
    printf("Original Byte(s): %d\n", original_bytes);
    printf("Archive Byte(s): %d\n", archive_bytes);
    printf("Compression ratio: %g%%\n", (100*(double)archive_bytes/(double)original_bytes));
    
    fclose(fin);
    fclose(fout);
    
    return 0;
}

void compress(FILE *fin, FILE *fout, int *original_bytes, int *archive_bytes)
{
    fseek(fin, 0, SEEK_END);
    *original_bytes = ftell(fin);
    rewind(fin);
    
    if (*original_bytes == 0) return;
    
    int ch;
    int frequencies[CHAR_SIZE] = {0};
    while ( (ch = fgetc(fin)) != EOF && ++frequencies[ch] );
    rewind(fin);
    
    Stacknode *top = newStack();
    huffmantree(frequencies, top);
    
    Huffnode* root = pop(top);
    delStack(top);
    
    char s[CHAR_SIZE] = "";
    char code[CHAR_SIZE][CHAR_SIZE];
    memset(code, '\0', CHAR_SIZE*CHAR_SIZE);
    
    if (root->left==NULL && root->right==NULL)  s[0]='0';
    huffmancode(root, s, code);
    
    char tmpCode[CHAR_BIT+1] = "";
    
    for (int i = 0; i < CHAR_SIZE; ++i) {
        if (strcmp(code[i], "")) {
            printf("%c = %s\n", (char)i, code[i]);
            
            fputc(i, fout);
            fputc(strlen(code[i]), fout);
            
            for (int j=0; j<strlen(code[i]); j+=CHAR_BIT) {
                memcpy( tmpCode, code[i]+j, 
                        (strlen(code[i])-j < CHAR_BIT) ? strlen(code[i])-j : CHAR_BIT );
                fputc(strtol(tmpCode, NULL, 2), fout);
                memset(tmpCode, '\0', CHAR_BIT+1);
            }
        }
    }
    
    
    int length = 0, extra_zero = 0;
    
    while ( (ch = fgetc(fin)) != EOF ) {
        length += strlen(code[ch]);
    }
    
    extra_zero = CHAR_BIT - (length % CHAR_BIT);
    fputc(extra_zero+'0', fout);
    fputc(0, fout);
    rewind(fin);
    

    int count, index;
    memset(tmpCode, '\0', CHAR_BIT+1);
    
    while ( (ch = fgetc(fin)) != EOF ) {
        
        count = strlen(tmpCode) + strlen(code[ch]);
        index=0;
        
        for (int i=0, j=0; i<count/CHAR_BIT; ++i) {

            j = CHAR_BIT-strlen(tmpCode);
            memcpy(tmpCode+strlen(tmpCode), code[ch]+index, j);
            index += j;
            
            fputc(strtol(tmpCode, NULL, 2), fout);
            memset(tmpCode, '\0', CHAR_BIT+1);
        }
        
        memcpy(tmpCode+strlen(tmpCode), code[ch]+index, strlen(code[ch])-index);
    }
    
    if (extra_zero > 0) {
        memset(tmpCode+strlen(tmpCode), '0', extra_zero);
        fputc(strtol(tmpCode, NULL, 2), fout);
    }
    
    *archive_bytes = ftell(fout);
}

void decompress(FILE *fin, FILE *fout, int *original_bytes, int *archive_bytes)
{
    fseek(fin, 0, SEEK_END);
    *archive_bytes = ftell(fin);
    rewind(fin);
    
    if (*archive_bytes == 0) return;
    
    int ch, ch1, len;
    char code[CHAR_SIZE][CHAR_SIZE];
    memset( code, '\0', CHAR_SIZE*(CHAR_SIZE) );
    
    while ( (ch = fgetc(fin)) != EOF ) {
        len = fgetc(fin);
        if (len == 0) break;
        
        for (int i=0, k=0, pass=0; i<len/CHAR_BIT+(len%CHAR_BIT!=0); ++i) {
            ch1 = fgetc(fin);
            
            pass = (CHAR_BIT - len%CHAR_BIT) * (i == len/CHAR_BIT);
            for (int j = CHAR_BIT-1-pass; j >= 0; --j)
                code[ch][k+j] = "01"[(ch1 >> (CHAR_BIT-1-j-pass)) & 1];
            k += CHAR_BIT-pass;
        }
    }
    int extra_zero = ch-'0';
    

    for (int i = 0; i < CHAR_SIZE; ++i) {
        if (strcmp(code[i], ""))
            printf("%c = %s\n", (char)i, code[i]);
    }
    
    char bset[CHAR_SIZE] = "";
    int if_stop = 0;
    
    for (int k=0, count=ftell(fin); (ch = fgetc(fin)) != EOF; ) {
        
        if_stop = (++count == *archive_bytes);
        
        for (int i=0; i < CHAR_BIT-extra_zero*if_stop; ++i) {
            bset[k++] = "01"[(ch >> (CHAR_BIT-1-i)) & 1];
            
            for (int j = 0; j < CHAR_SIZE; ++j) {
                if (strcmp(code[j], "") && strcmp(code[j], bset)==0 ) {
                    fputc(j, fout);
                    memset(bset, '\0', sizeof(bset));
                    k = 0;
                    break;
                }
            }
        }
    }
    
    *original_bytes = ftell(fout);
}

Huffnode *newnode(int f, int ch, Huffnode *L, Huffnode *R)
{
    Huffnode* tmp = (Huffnode *)malloc(sizeof(Huffnode));
    tmp->ch = ch;
    tmp->freq = f;
    tmp->left = L;
    tmp->right =R;
    return tmp;
}

void huffmancode(Huffnode *root, char *tmp, char code[][CHAR_SIZE])
{
    if(root==NULL)  return;
	if(root->left==NULL && root->right==NULL) {
        memcpy(code[root->ch], tmp, strlen(tmp));
    }
    
    char tmp2[CHAR_SIZE] = "";
    memcpy(tmp2, tmp, strlen(tmp));
    
	if(root->left) {
        tmp2[ strlen(tmp2) ] = '0';
        huffmancode(root->left, tmp2, code);
    }
    
    memset(tmp2, '\0', CHAR_SIZE);
    memcpy(tmp2, tmp, strlen(tmp));
	if(root->right) {
        tmp2[ strlen(tmp2) ] = '1';
        huffmancode(root->right, tmp2, code);
    }
}

void huffmantree(int *freq, Stacknode *top)
{
    int count = 0;
    
    for (int i = 0; i < CHAR_SIZE; ++i) {
        if (freq[i] != 0) {
            Huffnode* Hnode = newnode(freq[i], i, NULL, NULL);
            push(Hnode, top);
            ++count;
        }
    }

    while (count > 1)
    {
        Huffnode* tmpL = pop(top);
        Huffnode* tmpR = pop(top);
        Huffnode* Hnode = newnode(tmpL->freq+tmpR->freq, '\0', tmpL, tmpR);
        push(Hnode, top);
        --count;
    }
}

void push(Huffnode *input, Stacknode *top)
{
    Stacknode *node = (Stacknode *)malloc(sizeof(Stacknode));
    node->Hnode = input;
    
    Stacknode *tmp = top;    
    while (tmp->next!=NULL && tmp->next->Hnode->freq < input->freq)
        tmp = tmp->next;
    node-> next = tmp->next;
    tmp->next = node;
}

Huffnode *pop(Stacknode *top)
{
    if (top->next == NULL) return NULL;
    Stacknode *tmp = top->next;
    Huffnode *data = tmp->Hnode;
    top->next = top->next->next;
    free(tmp);
    return data;
}

Stacknode *newStack()
{
    Stacknode *top = (Stacknode *)malloc(sizeof(Stacknode));
    top->Hnode = NULL;
    top->next = NULL;
    return top;
}

void delStack(Stacknode *top)
{
    while (top->next != NULL)
    {
        Stacknode *tmp = top->next;
        top->next = top->next->next;
        free(tmp);
    }
    free(top);
}
