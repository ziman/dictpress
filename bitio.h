#ifndef BITIO_H
#define BITIO_H

#include <stdio.h>

typedef struct BitIO
{
	FILE * stream;
	unsigned char byte;
	int shift;
} BitIO;

void bitUseInputStream(BitIO * bio, FILE * stream);
void bitUseOutputStream(BitIO * bio, FILE * stream);

static inline int getBit(BitIO * bio)
{
	if (bio->shift < 0)
	{
		bio->shift = 7;
		bio->byte = getc(bio->stream);
	}

	return (bio->byte >> bio->shift--) & 0x1;
}

static inline void putBit(BitIO * bio, int bit)
{
	if (bio->shift < 0)
	{
		putc(bio->byte, bio->stream);
		bio->shift = 7;
		bio->byte = 0;
	}

	bio->byte |= bit << bio->shift--;
}

int getElias(BitIO * bio);
void putElias(BitIO * bio, int x);

void putByte(BitIO * bio, unsigned char x);

void bitFlush(BitIO * bio);



#endif
