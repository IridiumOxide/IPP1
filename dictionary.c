#include "trie.h"
#include "parse.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int vmode = 0;


int main(int argc, char* argv[]){
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

    command = get_command();

    return 0;
}