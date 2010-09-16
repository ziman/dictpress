#include "vitter.h"

typedef struct Node
{
	int nyt;
	unsigned int weight;
	Node * child[2];
} Node;

struct Huffman
{
	Node * root;
	Node * codes[257];
};

#define NYT 256

Node * newNode()
{
	Node * result = (Node *) malloc(sizeof(Node));

	result->nyt = 0;
	result->weight = 0;
	result->child[0] = 0;
	result->child[1] = 0;

	return result;
}

void hufInit(struct Huffman * huf)
{
	// create NYT
	Node * nyt = newNode();
	nyt->nyt = 1;
	huf->root = nyt;

	// initialize codes
	int i;
	for (i = 0; i < 256; ++i)
		huf->codes[i] = 0;
	huf->codes[NYT] = nyt;
}

void hufPut(struct Huffman * huf, BitIO * bio, unsigned char byte)
{
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
}
