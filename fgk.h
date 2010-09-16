#ifndef FGK_H
#define FGK_H

#include <stdio.h>
#include "bitio.h"

struct Huffman;

struct Huffman * hufInit();

void hufPut(struct Huffman * huf, BitIO * bio, unsigned char byte);
unsigned char hufGet(struct Huffman * huf, BitIO * bio);

void hufDump(struct Huffman * huf, FILE * f);

void hufFree(struct Huffman * huf);

#endif