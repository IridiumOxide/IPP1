#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "trie.h"

#define ALPHABET_SIZE 26   // all small english letters
#define MAX_WORDS 262144  // 2^18, based on maximum test file size
#define MAX_WORD_LENGTH 100500

typedef struct Edge Edge;
typedef struct Node Node;


/* EDGE - represents an edge in the tree
     label - character string represented by the edge
     target - pointer to the child node the edge leads to 
*/
struct Edge{
    char* label;
    Node* target;
};


/* NODE - represents a node in the tree.
     child_count - number of children of the node.
     id - id given to the full word represented by this node;
 -1 if the node does not represent a full word
     path[x] - edge with label that begins on ('a' + x)
     parent - edge leading upwards, to reconstruct a word based on its ID 
*/
struct Node{
    int child_count;
    int id;
    Edge path[ALPHABET_SIZE];
    Edge parentE;
};


// Global trie to use in this task
Node* tree = NULL;

int node_count = 0;
int current_id = 0;

// full_word[x] is a pointer to a node representing a full word with id = x
Node* full_word[MAX_WORDS];
char buffer[MAX_WORD_LENGTH];

// returns a copy of an edge with a given label and target
Edge get_edge(char* label, Node* target){
    Edge e;
    if (label == NULL){
        e.label = NULL;
    }
    else{
        e.label = calloc(strlen(label) + 1, sizeof(char));
        strcpy(e.label, label);
    }
    e.target = target;
    return e;
}


// Create an empty node and return a pointer to it
Node* node_construct(char* parent_label, Node* parent, int id){
    Node* node = malloc(sizeof(*node));
    node->child_count = 0;
    node->id = id;
    node->parentE = get_edge(parent_label, parent);
    for (int i = 0; i < ALPHABET_SIZE; ++i){
        node->path[i] = get_edge(NULL, NULL);
    }
    node_count++;
    return node;
}


// Free all memory used by a node
void node_destruct(Node* node){
    if (node != NULL){
        free(node->parentE.label);
        for (int i = 0; i < ALPHABET_SIZE; ++i){
            free(node->path[i].label);
        }
        free(node);
    }
    node_count--;
}


Node* get_parent(Node* node){
    return node->parentE.target;
}


// initialize the global tree if it has no root (number of nodes == 0)
void init(){
    if (tree == NULL){
        tree = node_construct(NULL, NULL, -1);
        node_count = 1;
    }
}


int next_id(){
    return current_id++;
}


void add_edge(Node* parent, char* label, Node* child){
    char first_letter = label[0];
    int letter_number = first_letter - 'a';
    parent->path[letter_number].label = calloc(strlen(label) + 1, sizeof(char));
    strcpy(parent->path[letter_number].label, label);
    parent->path[letter_number].target = child;
    parent->child_count++;
}


void remove_edge(Node* parent, char first_letter){
    int letter_number = first_letter - 'a';
    free(parent->path[letter_number].label);
    parent->path[letter_number].label = NULL;
    parent->path[letter_number].target = NULL;
    parent->child_count--;
}


//  recursively clear a tree represented by a given node.
void clear_node(Node* node){
    if (node != NULL){
        for (int i = 0; i < ALPHABET_SIZE; ++i){
            clear_node(node->path[i].target);
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
            label = current_node->path[letter_number].label;
            if (label == NULL){
                Node* new_node = node_construct(word + index, current_node, next_id());
                add_edge(current_node, word + index, new_node);
                full_word[new_node->id] = new_node;
                return new_node->id;
            }
            else{
                for (int i = 0; i < strlen(label); ++i){
                    // either the word ends on this edge or we have nowhere to go
                    if (index == word_l){
                        // end of the word, create a node here
                        char* label_a = calloc(i + 1, sizeof(char));
                        strncpy(label_a, label, i);
                        char* label_b = calloc(strlen(label) - i + 1, sizeof(char));
                        strcpy(label_b, label + i);
                        Node* next_node = current_node->path[letter_number].target;

                        Node* new_node = node_construct(label_a, current_node, next_id());
                        full_word[new_node->id] = new_node;
                        add_edge(new_node, label_b, next_node);
                        free(next_node->parentE.label);
                        next_node->parentE = get_edge(label_b, new_node);
                        free(current_node->path[letter_number].label);
                        current_node->path[letter_number] = get_edge(label_a, new_node);

                        free(label_a);
                        free(label_b);

                        return new_node->id;
                    }
                    else if (word[index] != label[i]){
                        // nowhere to go; create new node and edge
                        char* label_a = calloc(i + 1, sizeof(char));
                        strncpy(label_a, label, i);
                        char* label_b = calloc(strlen(label) - i + 1, sizeof(char));
                        strcpy(label_b, label + i);

                        Node* next_node = current_node->path[letter_number].target;

                        Node* transition_node = node_construct(label_a, current_node, -1);
                        add_edge(transition_node, label_b, next_node);
                        free(next_node->parentE.label);
                        next_node->parentE = get_edge(label_b, transition_node);
                        free(current_node->path[letter_number].label);
                        current_node->path[letter_number] = get_edge(label_a, transition_node);

                        Node* new_node = node_construct(word + index, transition_node, next_id());
                        full_word[new_node->id] = new_node;

                        add_edge(transition_node, word + index, new_node);

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
        current_node = current_node->path[letter_number].target;
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
        clear_node(tree);
        tree = NULL;
        for (int i = 0; i < MAX_WORDS; i++){
            full_word[i] = NULL;
        }
        return id;
    }

    Node* node = full_word[id];
    Node* parent = node->parentE.target;
    Edge child_edge = get_edge(NULL, NULL);
    char first_letter = node->parentE.label[0]; // to delete parent's edge
    full_word[id] = NULL;
    node->id = -1;

    if (node->child_count == 0){
        // just delete the node and an edge from parent
        remove_edge(parent, first_letter);
        node_destruct(node);
    }
    else if (node->child_count == 1){
        // unify the node with its parent
        for (int i = 0; i < ALPHABET_SIZE; ++i){
            if (node->path[i].target != NULL){
                child_edge = node->path[i];
                break;
            }
        }
        strcpy(buffer, node->parentE.label);
        strcat(buffer, child_edge.label);
        remove_edge(parent, first_letter);
        add_edge(parent, buffer, child_edge.target);
        remove_edge(node, child_edge.label[0]);
        node_destruct(node);
        free(child_edge.target->parentE.label);
        child_edge.target->parentE = get_edge(buffer, parent);
    }

    if (parent->id == -1 && parent->child_count < 2 && parent->parentE.target != NULL){
        // we need to unify the parent with its own parent
        Node* grandparent = parent->parentE.target;
        first_letter = parent->parentE.label[0];
        if (parent->child_count == 0){
            remove_edge(grandparent, first_letter);
            node_destruct(parent);
            return id;
        }

        for (int i = 0; i < ALPHABET_SIZE; ++i){
            if (parent->path[i].target != NULL){
                child_edge = parent->path[i];
                break;
            }
        }
        strcpy(buffer, parent->parentE.label);
        strcat(buffer, child_edge.label);
        remove_edge(grandparent, first_letter);
        add_edge(grandparent, buffer, child_edge.target);
        remove_edge(parent, child_edge.label[0]);
        node_destruct(parent);
        free(child_edge.target->parentE.label);
        child_edge.target->parentE = get_edge(buffer, grandparent);
    }
    // if child_count > 1 we just leave it as a transition node
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
        for (int i = strlen(node->parentE.label) - 1; i >= 0; --i){
            buffer[index] = node->parentE.label[i];
            ++index;
        }
        node = node->parentE.target;
        if (node->parentE.target == NULL){
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
        char* label = node->path[letter_number].label;
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
        node = node->path[letter_number].target;
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
