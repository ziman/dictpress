#ifndef FGK_H
#define FGK_H

#include "bitio.h"

struct Huffman;

void hufInit(struct Huffman * huf);

void hufPut(struct Huffman * huf, BitIO * bio, unsigned char byte);
unsigned char hufGet(struct Huffman * huf, BitIO * bio);

void hufFree(struct Huffman * huf);

#endif