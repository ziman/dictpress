#ifndef FGK_H
#define FGK_H

#include <stdio.h>
#include "bitio.h"

#define COREDUMP *((char *) 0) = 0

struct Huffman;

struct Huffman * hufInit();

void hufPut(struct Huffman * huf, BitIO * bio, unsigned char byte);
unsigned char hufGet(struct Huffman * huf, BitIO * bio);

#ifdef DEBUG
void hufDump(struct Huffman * huf, FILE * f);
int hufValid(struct Huffman * huf);
#endif

void hufFree(struct Huffman * huf);

#endif