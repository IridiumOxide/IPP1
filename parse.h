#pragma once

typedef enum{
    INSERT,
    PREV,
    DELETE,
    FIND,
    CLEAR,
    END,
    IGNORE
} query_type;

// all information our program needs to process a parsed query
typedef struct{
    query_type query;
    char* string_arg;
    int int_args[3];
} Command;

// process one line of input and return necessary information
Command get_command();