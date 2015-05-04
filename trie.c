#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "trie.h"

#define ALPHABET_SIZE 26  // all small english letters
#define MAX_WORDS 220000  // based on maximum test file size
#define STARTING_WORDS_CAPACITY 8

typedef struct Node Node;


/* NODE - represents a node in the tree.
     label_start - index in the words array where this node's label starts.
     label_end - index in the words array where this node's label ends.
     word_start - index in the words array where this node's whole word starts.
     parent - target of an edge leading upwards, to reconstruct a word based on its ID.
     id - id given to the full word represented by this node.
          -1 if it does not represent a full word.
     path[x] - target of an edge with label that begins on ('a' + x).
*/
struct Node{
    int label_start;
    int label_end;
    int word_start;
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

// Big char array used to keep all words added using the insert command,
//  used to optimize prev operation memory usage
char* all_words = NULL;

// currently used and max size of all_words array; Used to determine if we should realloc
int all_words_current = 0;
int all_words_max = 0;



/* *********************
 * AUXILIARY FUNCTIONS *
 * *********************/



// Create an empty node and return a pointer to it
Node* node_construct(int label_start, int label_end, int word_start, Node* parent, int id){
    Node* node = malloc(sizeof(*node));
    
    node->label_start = label_start;
    node->label_end = label_end;
    node->word_start = word_start;
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
        free(node);
        node_count--;
    }
}


// initialize the global tree if it has no root
void init(){
    if (tree == NULL){
        tree = node_construct(-1, -1, -1, NULL, -1);
        node_count = 1;
        all_words = calloc(STARTING_WORDS_CAPACITY + 1, sizeof(char));
        all_words_current = 0;
        all_words_max = STARTING_WORDS_CAPACITY;
    }
}


// get next id; used when adding a new node to the tree. 
int next_id(){
    return current_id++;
}


// add and edge from parent to child.
void add_edge(Node* parent, Node* child){
    char first_letter = all_words[child->label_start];
    int letter_number = first_letter - 'a';
    parent->path[letter_number] = child;
}


// remove an edge whose label begins with first_letter from parent
void remove_edge(Node* parent, char first_letter){
    int letter_number = first_letter - 'a';
    parent->path[letter_number] = NULL;
}


// change the label and parent of a given node without modifying its other properties
void change_parent_edge(Node* node, int n_start, Node* parent){
    node->label_start = n_start;
    node->parent = parent;
}


// add a node's label to its parent's label, delete the node
// 1--w--2--v--3  ->  1--wv--3
void union_with_parent(Node* node){
    // we assume here than node has exactly 1 child
    Node* parent = node->parent;
    Node* child = NULL;
    for (int i = 0; i < ALPHABET_SIZE; ++i){
            if (node->path[i] != NULL){
                child = node->path[i];
                break;
            }
    }
    int node_label_length = node->label_end - node->label_start + 1;
    char first_letter = all_words[node->label_start];
    
    remove_edge(parent, first_letter);
    remove_edge(node, all_words[child->label_start]);
    change_parent_edge(child, child->label_start - node_label_length, parent);
    node_destruct(node);
    add_edge(parent, child);
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


// add a word to words array; resize the array if needed. Returns the index
//  where the new word begins.
int add_word(char* word){
    int word_l = strlen(word);
    while (all_words_current + word_l > all_words_max){
        all_words = realloc(all_words, all_words_max * 2 + 1);
        all_words_max *= 2;
    }
    strcat(all_words, word);
    all_words_current += word_l;
    return all_words_current - word_l;
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



// insert a word into the tree. Returns -1 on fail, id otherwise;
//   l_end and word_start are set to -1 when inserting a new word
//   or to indices of all_words array when using the prev command.
int insert_word(char* word, int label_end, int word_start){
    init(); // if the tree is empty, insert will succeed, so we can use init()
    int index = 0; // which letter of the word we are currently on
    int word_l = strlen(word);
    Node* current_node = tree;
    char first_edge_letter;
    int letter_number;
    int label_start;

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
            if (current_node->path[letter_number] == NULL){
                // 1--w--2     ->       1--w--2
                //                       \-v--3
                
                if (word_start == -1){
                    word_start = add_word(word);
                    label_end = word_start + word_l - 1;
                }
                label_start = word_start + index;

                Node* new_node = node_construct(label_start, label_end, word_start,
                                                current_node, next_id());
                add_edge(current_node, new_node);
                full_word[new_node->id] = new_node;
                return new_node->id;
            }
            else{
                Node* next_node = current_node->path[letter_number];
                int edge_start = next_node->label_start;
                int edge_end = next_node->label_end;
                int edge_w_start = next_node->word_start;
                for (int i = edge_start; i <= edge_end; ++i){
                    // follow the edge and try to find out where the word should be inserted
                    if (index == word_l){
                        // end of the word, create a node here
                        // 1--wv--2      ->   1--w--3--v--2
                        
                        Node* new_node = node_construct(edge_start, i - 1, edge_w_start,
                                                        current_node, next_id());
                        full_word[new_node->id] = new_node;
                        change_parent_edge(next_node, i, new_node);
                        add_edge(new_node, next_node);
                        current_node->path[letter_number] = new_node;

                        return new_node->id;
                    }
                    else if (word[index] != all_words[i]){
                        // nowhere to go; create new node and edge
                        // 1--ab--2       ->     1--a--3--b--2
                        //                              \-c--4
                        
                        Node* transition_node = node_construct(edge_start, i - 1, edge_w_start,
                                                               current_node, -1);
                        change_parent_edge(next_node, i, transition_node);
                        add_edge(transition_node, next_node);
                        current_node->path[letter_number] = transition_node;

                        if (word_start == -1){
                            word_start = add_word(word);
                            label_end = word_start + word_l - 1;
                        }
                        label_start = word_start + index;

                        Node* new_node = node_construct(label_start, label_end, word_start,
                                                        transition_node, next_id());
                        full_word[new_node->id] = new_node;
                        add_edge(transition_node, new_node);

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


// insert a new word into the tree
int insert(char* word){
    return insert_word(word, -1, -1);
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
        free(all_words);
        all_words = NULL;
        return id;
    }

    Node* node = full_word[id];
    Node* parent = node->parent;
    char first_letter = all_words[node->label_start]; // to delete parent's edge
    full_word[id] = NULL;
    node->id = -1;

    if (child_count(node) == 0){
        // just delete the node and an edge from parent
        remove_edge(parent, first_letter);
        node_destruct(node);
    }
    else if (child_count(node) == 1){
        // unify the node with its parent
        union_with_parent(node);
    }
    // if child_count(node) > 1 we just leave it as a transition node

    if (parent->id == -1 && child_count(parent) < 2 && parent->parent != NULL){
        // we might need to delete the parent or unify it with its own parent
        Node* grandparent = parent->parent;
        first_letter = all_words[parent->label_start];
        if (child_count(parent) == 0){
            remove_edge(grandparent, first_letter);
            node_destruct(parent);
            return id;
        }
        union_with_parent(parent);
    }
    return id;
}


// instert a chosen fragment of the word with a given id. Returns -1 if
//  a word with this id does not exist or if we can't insert the fragment
int prev(int id, int start, int end){
    if (full_word[id] == NULL || start > end){
        return -1;
    }

    Node* node = full_word[id];
    if (end > (node->label_end - node->word_start)){
        return -1;
    }
    int original_word_start = node->word_start;

    // we need to recreate the word we are inserting
    char* new_word = malloc((end - start + 2) * sizeof(char));
    for (int i = start; i <= end; ++i){
        new_word[i - start] = all_words[original_word_start + i];
    }
    new_word[end - start + 1] = 0;

    int label_end = original_word_start + end;
    int word_start = original_word_start + start;

    int returned_id = insert_word(new_word, label_end, word_start);
    free(new_word);
    return returned_id;
}


// check if a pattern belongs to the tree. Returns 1 if it does, -1 otherwise
int find(char* pattern){
    int index = 0;
    Node* node = tree;
    int pattern_l = strlen(pattern);
    while (1){
        // we will break the loop upon finding the pattern / reaching NULL
        if (node == NULL){
            return -1;
        }

        int first_letter = pattern[index];
        int letter_number = first_letter - 'a';
        int label_start = -1;
        int label_end = -1;
        if(node->path[letter_number] != NULL){
            label_start = node->path[letter_number]->label_start;
            label_end = node->path[letter_number]->label_end;
        }
        if (label_start == -1){
            return -1;
        }

        for (int i = label_start; i <= label_end; ++i){
            if (pattern[index] != all_words[i]){
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
    free(all_words);
    all_words = NULL;
    all_words_current = 0;
    all_words_max = 0;
}


// returns the number of nodes
int get_node_count(){
    return node_count;
}
