#include "vitter.h"

typedef struct Node
{
	int nyt;
	int order;
	unsigned char value;
	unsigned int weight;
	Node * child[2];
	Node * parent;
} Node;

typedef struct Nodes
{
	Node * node;
	Nodes * next;
} Nodes;

typedef struct Block
{
	Nodes * nodes;
	Leader * next;
} Block;

struct Huffman
{
	Node * root;
	Node * codes[257];
	Block * blocks;
};

#define NYT 256

#if 0
static Node * determinePrevious(Node * node)
{
	if (!node->parent) return 0;

	// right child -> easy-peasy
	if (node == node->parent->child[1])
		return node->parent->child[0];

	// left child
	int levels = 0;
	Node * n = node;
	while (n != n->parent->child[1])
	{
		n = n->parent;
		++levels;

		if (!n->parent) return nodeAcross(node); // leftmost node in the tree -> NYT
	}

	// jump around
	n = n->parent->child[0];

	// descend
	while (levels--)
	{
		if (!n->child[1])
			return nodeAcross(node);
		
		n = n->child[1];
	}

	return n;
}
#endif

static Node * newNode(Node * parent)
{
	Node * result = (Node *) malloc(sizeof(Node));

	result->nyt = 0;
	result->weight = 0;
	result->child[0] = 0;
	result->child[1] = 0;
	result->parent = parent;

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

static void freeNodes(Nodes * nodes)
{
	while (nodes)
	{
		Nodes * next = nodes->next;
		free(nodes);
		nodes = next;
	}
}

static void freeBlocks(Block * block)
{
	while (block)
	{
		Block * next = block->next;
		freeNodes(block->nodes);
		free(block);
		block = next;
	}
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
	freeBlocks(huf->blocks);
}
