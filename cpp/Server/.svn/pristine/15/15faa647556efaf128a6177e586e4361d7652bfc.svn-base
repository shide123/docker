/*
 * trie_tree.h
 *
 *  Created on: 2015年12月21日
 *      Author: shuisheng
 */

#ifndef TRIE_TREE_H_
#define TRIE_TREE_H_

#define WORD_NUM     256
struct trie_node {
    struct trie_node *node[WORD_NUM];
    int value;
    int exist;
};


struct trie_node *create_trie_node(int value);
void trie_tree_insert_word(struct trie_node *root, unsigned char *word);
/* return 1 表示存在， return 0表示不存在 */
int tire_word_is_exist(struct trie_node *root, const unsigned char *word);
void destroy_trie_tree(struct trie_node *root);
void update_trie_tree(struct trie_node **root, const char *filename);


#endif /* TRIE_TREE_H_ */
