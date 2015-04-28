#pragma once

// insert a word into the global tree
int insert(char* word);

// insert a subword of a word from the tree with given id
int prev(int id, int start, int end);

// delete a word from the tree
int delete(int id);

// check if any wordin the tree has got a given prefix
int find(char* pattern);

// clear the tree
void clear();

// get the number of nodes in the tree
int get_node_count();