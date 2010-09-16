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

typedef struct Nodes
{
	Node * node;
	struct Nodes * next;
} Nodes;

typedef struct Block
{
	Nodes * nodes;
	struct Block * next;
} Block;

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
	huf->root = newNode(huf, 0, 0);

	// initialize codes
	int i;
	for (i = 0; i < 256; ++i)
		huf->codes[i] = 0;
	huf->codes[NYT] = huf->root;

	// initialize numbers
	for (i = 0; i < 512; ++i)
		huf->numbers[i] = 0;
	huf->numbers[ROOT] = huf->root;
}

static Node * findLeader(struct Huffman * huf, Node * node)
{
	int leader = node->number;
	unsigned int weight = node->weight;

	int i;
	for (i = node->number+1; i <= ROOT; ++i)
		if (huf->numbers[i]->weight == weight)
			leader = i;
		else
			break;

	return huf->numbers[leader];
}

static void emitCode(BitIO * bio, Node * node)
{
	if (node->parent)
	{
		emitCode(bio, node->parent);
		
		if (node == node->parent->child[0])
			putBit(bio, 0);
		else
			putBit(bio, 1);
	}
}

#define SWAP(a,b) { Node * p = a; a = b; b = p; }
static void incrementWeight(struct Huffman * huf, Node * node)
{
	if (!node->parent)
	{
		++node->weight;
		return;
	}
	
	Node * leader = findLeader(huf, node);
	if (leader != node)
	{
		if (node->parent == leader->parent)
		{
			SWAP(node->parent->child[0], node->parent->child[1]);
		}
		else
		{
			Node * p = node->parent;
			p->child[node == p->child[0] ? 0 : 1] = leader;

			// root will never be selected as a leader since it has strongly greater weight (forall n: root.w > n.w)
			p = leader->parent;
			p->child[leader == p->child[0] ? 0 : 1] = node;

			// swap parents
			SWAP(node->parent, leader->parent)
		}

		// swap numbers
		int nr = node->number;
		node->number = leader->number;
		leader->number = nr;
	}

	++node->weight;
	incrementWeight(huf, node->parent);
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
		Node * nyt = huf->codes[NYT];
		emitCode(bio, nyt);
		putByte(bio, byte);

		// create two children of NYT
		node = newNode(huf, nyt, 0);
		Node * newNyt = newNode(huf, nyt, byte);
		nyt->child[0] = newNyt;
		nyt->child[1] = node;

		// update backreferences
		huf->codes[NYT] = newNyt;
		huf->codes[byte] = node;
	}

	incrementWeight(huf, node);
}

unsigned char hufGet(struct Huffman * huf, BitIO * bio)
{
	Node * node = huf->root;
	while (node->child[0])
		node = node->child[getBit(bio)];

	incrementWeight(huf, node);

	return node->value;
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
}
