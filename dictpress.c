/*
 * dictpress.c
 * Encode/decode dictionaries before/after compression.
 *
 * Copyright (c) 2010, Matus Tejiscak <functor.sk@ziman>
 * Released under the BSD license.
 * http://creativecommons.org/licenses/BSD/
 *
 * Purpose:
 * Encode dictionaries in the form that is better compressible by
 * general-purpose compression algorithms, exploiting their structure.
 *
 * Usage:
 * $ cc -O2 dictpress.c -o dictpress
 * $ cat dictionary.txt | ./dictpress | bzip2 -9 > dictionary.dp.bz2
 * $ cat dictionary.dp.bz2 | bunzip2 | ./dictpress -d > dictionary-decompressed.txt
 *
 * Used best with bzip2 -9. (lzma and gzip perform worse).
 *
 * Approximate compression ratio:
 * My 96 megabyte dictionary compresses to
 *  -> 18M with bzip2
 *  -> 1M with dictpress+bzip2
 *
 * Prerequisites:
 * There's no point in running this on an unsorted dictionary.
 * Words must not contain binary zero.
 *
 * Algorithm used:
 * We exploit the fact that consecutive words differ only a little in the suffix.
 * Therefore, for each word we record a pair (n,s) saying "remove n chars from the
 * end of the last word and append the string s".
 *
 * Warning:
 * Does not preserve CRs (#13, '\r', ...) in the input.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
void encode(void)
{
	// The buffers.
	char _word[256], _lastWord[256];
	char * lastWord = _lastWord, *word = _word;
	*lastWord = 0;
	
	int delimiterNeeded = 0;

	while (1)
	{
		// Get the line.
		fgets(word, 256, stdin);
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

		// Print the data.
		if (delimiterNeeded) putchar(0);
		printf("%c%s", (char) trim, suffix);
		delimiterNeeded = 1;

		// Swap the two buffers.
		char * tmp = lastWord;
		lastWord = word;
		word = tmp;
	}
}

// Decode the dictionary.
void decode(void)
{
	char word[256];
	*word = 0;
	char * p = word;

	while (1)
	{
		// Get the trim length.
		int trim = getchar();
		if (trim == EOF) break;

		// Decrease the write pointer.
		if (trim < 0 || trim > p - word)
			die("Bad input format.");
		p -= trim;

		// Append the new suffix.
		while (1)
		{
			// Get the character.
			int ch = getchar();
			if (ch == EOF) break;
			if (ch == 0) break;
			if (p - word >= sizeof(word))
				die("Buffers too small.");

			// Write it to the buffer.
			*p++ = (char) ch;
		}

		*p = '\0';

		printf("%s\n", word);
	}
}

int main(int argc, char * argv[])
{
	if (argc == 2 && strcmp(argv[1], "-d") == 0)
		decode();
	else
		encode();

	return 0;
}
