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

typedef struct{
    query_type query;
    char* string_arg;
    int int_args[3];
} Command;

Command get_command();