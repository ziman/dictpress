#include "fgk.h"
#include <stdlib.h>

typedef struct Node
{
	int number;
	unsigned char value;
	unsigned int weight;
	struct Node * child[2];
	struct Node * parent;
} Node;

struct Huffman
{
	Node * root;
	Node * codes[257];
	Node * numbers[513];
	int nextNumber;
};

#define NYT 256
#define ROOT 512

static Node * newNode(struct Huffman * huf, Node * parent, unsigned char value)
{
	Node * result = (Node *) malloc(sizeof(Node));

	result->number = huf->nextNumber--;
	huf->numbers[result->number] = result;
	
	result->weight = 0;
	result->child[0] = 0;
	result->child[1] = 0;
	result->parent = parent;
	result->value = value;

	return result;
}

struct Huffman * hufInit()
{
	struct Huffman * huf = (struct Huffman *) malloc(sizeof(struct Huffman));
	if (!huf) return 0;
	
	// initialize nextNumber
	huf->nextNumber = 512;
	
	// create NYT
	huf->root = newNode(huf, 0, 0xFF);

	// initialize codes
	int i;
	for (i = 0; i < 256; ++i)
		huf->codes[i] = 0;
	huf->codes[NYT] = huf->root;

	// initialize numbers
	for (i = 0; i < 512; ++i)
		huf->numbers[i] = 0;
	huf->numbers[ROOT] = huf->root;

	return huf;
}

static Node * findLeader(struct Huffman * huf, Node * node, int leafNeeded)
{
	int leader = node->number;
	unsigned int weight = node->weight;

	int i;
	for (i = node->number+1; i < ROOT; ++i)
		if (huf->numbers[i]->weight == weight && (!leafNeeded || huf->numbers[i]->child[0] == 0))
			leader = i;
		else
			break;

	return huf->numbers[leader];
}

static void emitCode(BitIO * bio, Node * node)
{
	if (node->parent)
	{
		if (node->parent == node)
			((void(*)())0)();
		emitCode(bio, node->parent);
		
		if (node == node->parent->child[0])
			putBit(bio, 0);
		else
			putBit(bio, 1);
	}
}

#ifdef DEBUG
static void getCode(FILE * f, Node * node)
{
	if (node->parent)
	{
		getCode(f, node->parent);
		
		if (node == node->parent->child[0])
			fputc('0', f);
		else if (node == node->parent->child[1])
			fputc('1', f);
		else
			fputc('?', f);
	}
}

int hufValid(struct Huffman * huf)
{
	int i;
	for (i = 0; i < 256; ++i)
	{
		Node * node = huf->codes[i];
		if (node)
		{
			if (node == node->parent->child[0])
				continue;
			else if (node == node->parent->child[1])
				continue;
			else
				return 0;
		}
	}

	return 1;
}

void hufDump(struct Huffman * huf, FILE * f, char * name)
{
	fprintf(f, "-- [%s] --\n", name);
	int i;
	for (i = 0; i < 256; ++i)
	{
		Node * node = huf->codes[i];
		
		if (node)
		{
			fprintf(f, "%02x '%c' (%p): ", (unsigned char) i, i > 31 ? (char) i : ' ', node);
			getCode(f, node);
			fprintf(f, "\n");
		}
	}
}
#endif

#define SWAP(a,b) { Node * p = a; a = b; b = p; }
static void incrementWeight(struct Huffman * huf, Node * node)
{
	if (!node->parent)
	{
		++node->weight;
		return;
	}
	
	Node * leader = findLeader(huf, node, node->parent == huf->codes[NYT]->parent);
	if (leader != node)
	{
		if (node->parent == leader->parent)
		{
			SWAP(node->parent->child[0], node->parent->child[1]);
		}
		else if (node->parent == leader)
		{
			Node * parent = leader->parent;
			parent->child[leader == parent->child[0] ? 0 : 1] = node;

			int ix = node == leader->child[0] ? 0 : 1;
			leader->child[ix] = node->child[ix];
			node->child[ix] = leader;
		}
		else
		{
			Node * p = node->parent;
			p->child[node == p->child[0] ? 0 : 1] = leader;

			// root will never be selected as a leader
			p = leader->parent;
			p->child[leader == p->child[0] ? 0 : 1] = node;

			// swap parents
			SWAP(node->parent, leader->parent)
		}

		// swap numbers
		int nr = node->number;
		node->number = leader->number;
		leader->number = nr;

		// swap the numbers within huf
		huf->numbers[node->number] = node;
		huf->numbers[leader->number] = leader;
	}

	++node->weight;
	incrementWeight(huf, node->parent);
}

static Node * createNode(struct Huffman * huf, unsigned char byte)
{
	// create two children of NYT
	Node * nyt = huf->codes[NYT];
	Node * node = newNode(huf, nyt, byte);
	Node * newNyt = newNode(huf, nyt, 0xFF);
	nyt->child[0] = newNyt;
	nyt->child[1] = node;

	// update backreferences
	huf->codes[NYT] = newNyt;
	huf->codes[byte] = node;

	return node;
}

void hufPut(struct Huffman * huf, BitIO * bio, unsigned char byte)
{
	Node * node = huf->codes[byte];
	
	if (node)
	{
		emitCode(bio, node);
	}
	else
	{
		// emit NYT+literal code into the bitstream
		emitCode(bio, huf->codes[NYT]);
		putByte(bio, byte);

		node = createNode(huf, byte);
	}

	incrementWeight(huf, node);
}

unsigned char hufGet(struct Huffman * huf, BitIO * bio)
{
	Node * node = huf->root;
	while (node->child[0])
		node = node->child[getBit(bio)];

	if (node == huf->codes[NYT])
	{
		unsigned char byte = getByte(bio);
		node = createNode(huf, byte);
	}

	incrementWeight(huf, node);

	return node->value;
}

static void freeNode(Node * node)
{
	if (!node) return;
	
	free(node->child[0]);
	free(node->child[1]);
	
	free(node);
}

void hufFree(struct Huffman * huf)
{
	freeNode(huf->root);
	free(huf);
}
