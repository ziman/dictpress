#include "bitio.h"

void bitUseInputStream(BitIO * bio, FILE * stream)
{
	bio->byte = 0;
	bio->shift = -1;
	bio->stream = stream;
}

void bitUseOutputStream(BitIO * bio, FILE * stream)
{
	bio->byte = 0;
	bio->shift = 7;
	bio->stream = stream;
}

void bitFlush(BitIO * bio)
{
	putc(bio->byte, bio->stream);
}

int getElias(BitIO * bio)
{
	int n = 0;
	while (getBit(bio) == 0) ++n;

	int x = 1;
	while (n--)
	{
		x <<= 1;
		x += getBit(bio);
	}

	return x - 1;
}

void putElias(BitIO * bio, int x)
{
	++x; // to allow zeros
	
	int y, n;
	for (y = x, n = 0; y > 1; y >>= 2, ++n)
		putBit(bio, 0);

	while (n >= 0)
	{
		putBit(bio, (x >> n--) & 0x1);
	}
}

void putByte(BitIO * bio, unsigned char x)
{
	int i;
	for (int i = 7; i >= 0; --i)
		putBit(bio, (x >> i) & 0x1);
}



