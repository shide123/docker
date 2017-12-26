/*
 * trie_tree.cpp
 *
 *  Created on: 2015年12月21日
 *      Author: shuisheng
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "trie_tree.h"

struct trie_node *create_trie_node(int value)
{
	struct trie_node * node = (struct trie_node*)calloc(1, sizeof(struct trie_node));
	node->value = value;
	return node;
}

int tire_word_is_exist(struct trie_node *root, const unsigned char *word)
{
	struct trie_node *n = NULL;
	const unsigned char *p = NULL;

	if (root == NULL) {
		return 0;
	}

	while (*word != 0) {
		p = word++;
		n = root;
		while (*p != 0) {
			n = n->node[*p];
			if (n == NULL) {
				break;
			}
			else if (n->exist == 1) {
				return 1;
			}
			p++;
		}
	}

	return 0;
}

void trie_tree_insert_word(struct trie_node *root, unsigned char *word)
{
	struct trie_node *n;
	while (*word != 0) {
		n = root->node[*word];
		if (n == NULL) {
			n = create_trie_node(*word);
			root->node[*word] = n;
		}
		root = n;
		word++;
	}
	root->exist = 1;
}

void destroy_trie_tree(struct trie_node *root)
{
	int i;
	if (root == NULL) {
		return;
	}
	for (i = 0; i < WORD_NUM; i++) {
		destroy_trie_tree(root->node[i]);
	}
	free(root);
}


void update_trie_tree(struct trie_node **root, const char *filename)
{
	char word[1024];
	FILE *fp;
	char *p;

	if (*root != NULL) {
		destroy_trie_tree(*root);
	}

	*root = (struct trie_node *)calloc(sizeof(**root),1);

	fp = fopen(filename, "r");
	if (fp == NULL) {
		printf("file can't open %s\n", filename);
		return;
	}

	while (fgets(word, sizeof(word), fp)) {
		p = word;

		while (*p != 0) {
			if (*p == '\r' || *p == '\n' || *p == ' ') {
				*p = 0;
				break;
			}
			p++;
		}
		trie_tree_insert_word(*root, (unsigned char *)word);
	}
}





