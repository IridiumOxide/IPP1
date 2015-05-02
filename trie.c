#include <stdlib.h>
#include <string.h>
#include "trie.h"

#define ALPHABET_SIZE 26  // all small english letters
#define MAX_WORDS 220000  // based on maximum test file size
#define MAX_WORD_LENGTH 100500

typedef struct Node Node;


/* NODE - represents a node in the tree.
     label - label of an edge leading from this node's parent to itself
 -1 if the node does not represent a full word
     parent - target of an edge leading upwards, to reconstruct a word based on its ID.
     id - id given to the full word represented by this node;
     path[x] - target of an edge with label that begins on ('a' + x)
*/
struct Node{
    char* label;
    Node* parent;
    int id;
    Node* path[ALPHABET_SIZE];
};



/* ******************
 * GLOBAL VARIABLES *
 * ******************/


 
// Global trie to use in this task
Node* tree = NULL;

// total number of nodes in the global tree
int node_count = 0;

// id to be given to the next inserted node, managed by next_id() function
int current_id = 0;

// full_word[x] is a pointer to a node representing a full word with id = x
Node* full_word[MAX_WORDS];

char buffer[MAX_WORD_LENGTH];



/* *********************
 * AUXILIARY FUNCTIONS *
 * *********************/



// Create an empty node and return a pointer to it
Node* node_construct(char* parent_label, Node* parent, int id){
    Node* node = malloc(sizeof(*node));
    if (parent != NULL){
        node->label = strdup(parent_label);
    }
    else{
        node->label = NULL;
    }
    node->parent = parent;

    node->id = id;

    for (int i = 0; i < ALPHABET_SIZE; ++i){
        node->path[i] = NULL;
    }
    node_count++;
    return node;
}


// Free all memory used by a node
void node_destruct(Node* node){
    if (node != NULL){
        free(node->label);
        free(node);
    }
    node_count--;
}


// initialize the global tree if it has no root
void init(){
    if (tree == NULL){
        tree = node_construct(NULL, NULL, -1);
        node_count = 1;
    }
}


// get next id; used when adding a new node to the tree. 
int next_id(){
    return current_id++;
}


// add and edge from parent to child.
void add_edge(Node* parent, Node* child){
    char first_letter = child->label[0];
    int letter_number = first_letter - 'a';
    parent->path[letter_number] = child;
}


// remove an edge whose label begins with first_letter from parent
void remove_edge(Node* parent, char first_letter){
    int letter_number = first_letter - 'a';
    parent->path[letter_number] = NULL;
}


// change the label and parent of a given node without modifying its other properties
void change_parent_edge(Node* node, char* new_label, Node* parent){
    free(node->label);
    node->label = strdup(new_label);
    node->parent = parent;
}


// return the number of a given node's children
int child_count(Node* node){
    int result = 0;
    for (int i = 0; i < ALPHABET_SIZE; ++i){
        if (node->path[i] != NULL){
            ++result;
        }
    }
    return result;
}


// recursively clear a tree represented by a given node.
void clear_node(Node* node){
    if (node != NULL){
        for (int i = 0; i < ALPHABET_SIZE; ++i){
            clear_node(node->path[i]);
        }
        node_destruct(node);
    }
}


// reverse a string
void reverse_string(char* a_string){
    int ibegin = 0;
    int iend = strlen(a_string) - 1;
    while (ibegin < iend){
        // swap letters
        a_string[ibegin] ^= a_string[iend];
        a_string[iend] ^= a_string[ibegin];
        a_string[ibegin] ^= a_string[iend];
        ++ibegin;
        --iend;
    }
}

/* **********************
 * MAIN FUNCTIONS BELOW *
 * **********************/


// insert a word into the tree. Returns -1 on fail, id otherwise
int insert(char* word){
    // if the tree is empty, insert will succeed, so we can use init()
    init();
    int index = 0;
    int word_l = strlen(word);
    Node* current_node = tree;
    char first_edge_letter;
    int letter_number;
    char* label;

    while (1){
        // we will eventually exit the loop through one of the return statements.
        if (index == word_l){
            // current node represents the word we're trying to insert
            if (current_node->id == -1){
                // the word has not yet been inserted
                current_node->id = next_id();
                full_word[current_node->id] = current_node;
                return current_node->id;
            }
            else{
                // the word has already been inserted
                return -1;
            }
        }
        else{
            // edge section
            first_edge_letter = word[index];
            letter_number = first_edge_letter - 'a';
            if(current_node->path[letter_number] != NULL){
                label = current_node->path[letter_number]->label;
            }else{
                label = NULL;
            }
            if (label == NULL){
                // 1--w--2     ->       1--w--2
                //                       \-v--3
                
                Node* new_node = node_construct(word + index, current_node, next_id());
                add_edge(current_node, new_node);
                full_word[new_node->id] = new_node;
                return new_node->id;
            }
            else{
                for (int i = 0; i < strlen(label); ++i){
                    // either the word ends on this edge or we have nowhere to go
                    if (index == word_l){
                        // end of the word, create a node here
                        // 1--wv--2      ->   1--w--3--v--2
                        
                        char* label_a = calloc(i + 1, sizeof(char));
                        strncpy(label_a, label, i);
                        char* label_b = calloc(strlen(label) - i + 1, sizeof(char));
                        strcpy(label_b, label + i);
                        
                        Node* next_node = current_node->path[letter_number];
                        Node* new_node = node_construct(label_a, current_node, next_id());
                        full_word[new_node->id] = new_node;
                        change_parent_edge(next_node, label_b, new_node);
                        add_edge(new_node, next_node);
                        current_node->path[letter_number] = new_node;

                        free(label_a);
                        free(label_b);

                        return new_node->id;
                    }
                    else if (word[index] != label[i]){
                        // nowhere to go; create new node and edge
                        // 1--ab--2       ->     1--a--3--b--2
                        //                              \-c--4
                        
                        char* label_a = calloc(i + 1, sizeof(char));
                        strncpy(label_a, label, i);
                        char* label_b = calloc(strlen(label) - i + 1, sizeof(char));
                        strcpy(label_b, label + i);

                        Node* next_node = current_node->path[letter_number];
                        Node* transition_node = node_construct(label_a, current_node, -1);
                        change_parent_edge(next_node, label_b, transition_node);
                        add_edge(transition_node, next_node);
                        current_node->path[letter_number] = transition_node;

                        Node* new_node = node_construct(word + index, transition_node, next_id());
                        full_word[new_node->id] = new_node;
                        add_edge(transition_node, new_node);

                        free(label_a);
                        free(label_b);

                        return new_node->id;
                    }
                    else{
                        // just follow the edge
                        index++;
                    }
                }
            }
        }
        current_node = current_node->path[letter_number];
    }
}


// delete the word with given id. Returns -1 on fail, id otherwise
int delete(int id){
    if (id >= MAX_WORDS || full_word[id] == NULL){
        // word with this id does not exist
        return -1;
    }
    if (node_count == 2){
        // we must delete the root, as the tree becomes empty.
        // detele root's child from id table:
        for (int i = 0; i < ALPHABET_SIZE; i++){
            if(tree->path[i] != NULL){
                full_word[tree->path[i]->id] = NULL;
            }
        }
        clear_node(tree);
        tree = NULL;
        return id;
    }

    Node* node = full_word[id];
    Node* parent = node->parent;
    Node* child = NULL;
    char first_letter = node->label[0]; // to delete parent's edge
    full_word[id] = NULL;
    node->id = -1;

    if (child_count(node) == 0){
        // just delete the node and an edge from parent
        remove_edge(parent, first_letter);
        node_destruct(node);
    }
    else if (child_count(node) == 1){
        // unify the node with its parent
        for (int i = 0; i < ALPHABET_SIZE; ++i){
            if (node->path[i] != NULL){
                child = node->path[i];
                break;
            }
        }
        strcpy(buffer, node->label);
        strcat(buffer, child->label);
        remove_edge(parent, first_letter);
        remove_edge(node, child->label[0]);
        node_destruct(node);
        change_parent_edge(child, buffer, parent);
        add_edge(parent, child);
    }
    // if child_count(node) > 1 we just leave it as a transition node

    if (parent->id == -1 && child_count(parent) < 2 && parent->parent != NULL){
        // we need to unify the parent with its own parent
        Node* grandparent = parent->parent;
        first_letter = parent->label[0];
        if (child_count(parent) == 0){
            remove_edge(grandparent, first_letter);
            node_destruct(parent);
            return id;
        }

        for (int i = 0; i < ALPHABET_SIZE; ++i){
            if (parent->path[i] != NULL){
                child = parent->path[i];
                break;
            }
        }
        strcpy(buffer, parent->label);
        strcat(buffer, child->label);
        remove_edge(grandparent, first_letter);
        remove_edge(parent, child->label[0]);
        node_destruct(parent);
        change_parent_edge(child, buffer, grandparent);
        add_edge(grandparent, child);
    }
    return id;
}


// instert a chosen fragment of word with a given id. Returns -1 if
//  that word with this id does not exist or if we can't insert the fragment
int prev(int id, int start, int end){
    if (full_word[id] == NULL || start > end){
        return -1;
    }
    int index = 0;
    Node* node = full_word[id];
    // we will transfer the reversed word to the buffer, letter by letter.
    while (1){
        // we will break the loop after getting to the root
        for (int i = strlen(node->label) - 1; i >= 0; --i){
            buffer[index] = node->label[i];
            ++index;
        }
        node = node->parent;
        if (node->parent == NULL){
            buffer[index] = 0;
            break;
        }
    }
    reverse_string(buffer);
    if (end >= strlen(buffer)){
        return -1;
    }
    char* new_word = calloc(end - start + 2, sizeof(char));
    strncpy(new_word, buffer + start, end - start + 1);
    // we have recreated the whole word, now we can insert it
    int result = insert(new_word);
    free(new_word);
    return result;
}


// check if a pattern belongs to the tree. Returns 1 if it does, -1 otherwise
int find(char* pattern){
    int index = 0;
    Node* node = tree;
    int pattern_l = strlen(pattern);
    while (1){
        // we will break the null upon finding the pattern / reaching NULL
        if (node == NULL){
            return -1;
        }

        int first_letter = pattern[index];
        int letter_number = first_letter - 'a';
        char* label = NULL;
        if(node->path[letter_number] != NULL){
            label = node->path[letter_number]->label;
        }
        if (label == NULL){
            return -1;
        }

        for (int i = 0; i < strlen(label); ++i){
            if (pattern[index] != label[i]){
                return -1;
            }
            ++index;
            if (index == pattern_l){
                return 1;
            }
        }
        node = node->path[letter_number];
    }
}


// clear the whole tree
void clear(){
    clear_node(tree);
    tree = NULL;
    current_id = 0;
    for (int i = 0; i < MAX_WORDS; i++){
        full_word[i] = NULL;
    }
}


// returns the number of nodes
int get_node_count(){
    return node_count;
}
