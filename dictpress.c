/*
 * dictpress.c
 * Compress alphabetically sorted wordlists.
 *
 * Copyright (c) 2010, Matus Tejiscak <functor.sk@ziman>
 * Released under the BSD license.
 * http://creativecommons.org/licenses/BSD/
 *
 * See README for more information.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fgk.h"

// Print an error message and die.
void die(const char * msg)
{
	fprintf(stderr, "%s\n", msg);
	exit(1);
}

// Determine the trim length and the suffix from two words.
void compare(char * lastWord, char * word, char ** suffix, int * trim)
{
	while (1)
	{
		if (*lastWord == '\0')
		{
			// Second word longer.
			*trim = 0;
			break;
		}

		if (*word == '\0')
		{
			// First word longer.
			*trim = strlen(lastWord);
			break;
		}

		if (*lastWord != *word)
		{
			// First mismatch.
			*trim = strlen(lastWord);
			break;
		}

		// Match, move on.
		++lastWord;
		++word;
	}

	*suffix = word;
}

// Encode the dictionary.
void encode(FILE * input, FILE * output)
{
	// The buffers.
	char _word[256], _lastWord[256];
	char * lastWord = _lastWord, *word = _word;
	*lastWord = 0;

	// Create Huffman contexts
	struct Huffman * hufChars = hufInit();
	struct Huffman * hufTrims = hufInit();

	// Create bitIO
	BitIO * bio = (BitIO *) malloc(sizeof(BitIO));
	bitUseOutputStream(bio, output);
	
	while (1)
	{
		// Get the line.
		fgets(word, 256, input);
		if (feof(stdin)) break;

		// Remove CRLF.
		char * p;
		for (p = word; *p; ++p)
			if (*p == '\r' || *p == '\n')
			{
				*p = '\0';
				break;
			}

		// Determine suffix and trim length.
		char * suffix;
		int trim;
		compare(lastWord, word, &suffix, &trim);

		// Print the trim length
		hufPut(hufTrims, bio, (unsigned char) trim);
		// Print the characters
		while (*suffix)
			hufPut(hufChars, bio, *suffix++);
		// Print zero
		hufPut(hufChars, bio, 0);

		// Swap the two buffers.
		char * tmp = lastWord;
		lastWord = word;
		word = tmp;
	}

	free(bio);

	hufDump(hufTrims, stderr);

	hufFree(hufTrims);
	free(hufTrims);
	
	hufFree(hufChars);
	free(hufChars);
}

// Decode the dictionary.
void decode(FILE * input, FILE * output)
{
	char word[256];
	*word = 0;
	char * p = word;

	while (1)
	{
		// Get the trim length.
		int trim = getc(input);
		if (trim == EOF) break;

		// Decrease the write pointer.
		if (trim < 0 || trim > p - word)
			die("Bad input format.");
		p -= trim;

		// Append the new suffix.
		while (1)
		{
			// Get the character.
			int ch = getc(input);
			if (ch == EOF) break;
			if (ch == 0) break;
			if (p - word >= (int) sizeof(word))
				die("Buffers too small.");

			// Write it to the buffer.
			*p++ = (char) ch;
		}

		*p = '\0';

		fprintf(output, "%s\n", word);
	}
}

int main(int argc, char * argv[])
{
	if (argc == 2 && strcmp(argv[1], "-d") == 0)
		decode(stdin, stdout);
	else
		encode(stdin, stdout);

	return 0;
}
