#include "trie.h"
#include "parse.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// if nodes_info == 1 after completing a command, our program
// writes the number of nodes to stderr.
int nodes_info = 1;

void ignore(){
    nodes_info = 0;
    printf("ignored\n");
}

int main(int argc, char* argv[]){
    int vmode = 0;
    nodes_info = 0;

    if (argc > 2){
        puts("Error: too many parameters.\n");
        return 1;
    }
    else if (argc == 2){
        if (strcmp(argv[1], "-v") == 0){
            vmode = 1;
        }
        else{
            printf("Error: unknown parameter %s", argv[1]);
            return 1;
        }
    }

    Command command;
    command.string_arg = NULL;
    int result;
    // main loop: accept the command from parser and call one of the trie functions
    while (1){
        if (vmode == 1){
            nodes_info = 1;
        }
        // string_arg might get allocated, need to free it at the end
        command = get_command();
        switch (command.query){
        case INSERT:
            result = insert(command.string_arg);
            if (result != -1){
                printf("word number: %d\n", result);
            }
            else{
                ignore();
            }
            break;
        case PREV:
            result = prev(command.int_args[0], command.int_args[1], command.int_args[2]);
            if (result != -1){
                printf("word number: %d\n", result);
            }
            else{
                ignore();
            }
            break;
        case DELETE:
            result = delete(command.int_args[0]);
            if (result != -1){
                printf("deleted: %d\n", result);
            }
            else{
                ignore();
            }
            break;
        case FIND:
            nodes_info = 0;
            result = find(command.string_arg);
            if (result == -1){
                printf("NO\n");
            }
            else{
                printf("YES\n");
            }
            break;
        case CLEAR:
            clear();
            printf("cleared\n");
            break;
        case END:
	    clear();
            return 0;
            break;
        default:
            ignore();
            break;
        }
        if (nodes_info == 1){
            fprintf(stderr, "nodes: %d\n", get_node_count());
        }
        free(command.string_arg);
    }

    return 0;
}
