#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parse.h"

#define MAX_WORD_LENGTH 100500

char buffer[MAX_WORD_LENGTH];


int is_small_letter(char x){
    if (x >= 'a' && x <= 'z'){
        return 1;
    }
    else{
        return -1;
    }
}


int is_a_space(char x){
    if (x == ' '){
        return 1;
    }
    else{
        return -1;
    }
}


int is_a_digit(char x){
    if (x >= '0' && x <= '9'){
        return 1;
    }
    else{
        return -1;
    }
}


int parse_number(char* number){
    // number is guaranteed to contain only digits, and less than 8 of them.
    int result = 0;
    for (int i = 0; i < strlen(number); ++i){
        if (number[i] == '0' && result == 0 && i != strlen(number) - 1){
            // leading 0 - that's an error.
            return -1;
        }
        else{
            result *= 10;
            result += number[i] - '0';
        }
    }
    return result;
}


// returns a struct containing command enum and arguments based on file input.
Command get_command(){
    Command new_command;
    new_command.string_arg = NULL;
    // '$' - a word; '#' - a number; ' ' - 1 or more spaces; '!' - 0 or more spaces, then endline
    static const char* ins = "insert $!";
    static const char* pre = "prev # # #!";
    static const char* del = "delete #!";
    static const char* fin = "find $!";
    static const char* cle = "clear!";

    const char* expression;
    void* status = fgets(buffer, MAX_WORD_LENGTH, stdin);
    int index = 0;
    int current_int_arg = 0;
    if (status == NULL){
        new_command.query = END;
        return new_command;
    }
    while (is_a_space(buffer[index]) == 1){
        index++;
    }
    // now we are on the first non-space character.

    char first_letter = buffer[index];
    switch (first_letter){
    case 'i':
        new_command.query = INSERT;
        expression = ins;
        break;
    case 'p':
        new_command.query = PREV;
        expression = pre;
        break;
    case 'd':
        new_command.query = DELETE;
        expression = del;
        break;
    case 'f':
        new_command.query = FIND;
        expression = fin;
        break;
    case 'c':
        new_command.query = CLEAR;
        expression = cle;
        break;
    default:
        new_command.query = IGNORE;
        return new_command;
        break;
    }

    for (int i = 0; i < strlen(expression); ++i){
        if (expression[i] == ' '){
            if (is_a_space(buffer[index]) == -1){
                new_command.query = IGNORE;
                return new_command;
            }
            ++index;

            while (is_a_space(buffer[index]) == 1){
                ++index;
            }
        }
        else if (expression[i] == '#'){
            char* number = calloc(7, sizeof(char));
            int number_length = 0;
            while (is_a_digit(buffer[index]) == 1){
                if (number_length < 6){
                    number[number_length] = buffer[index];
                }
                ++number_length;
                ++index;
            }
            // We don't accept numbers longer than 6 digits because they're too big anyway.
            if (number_length == 0 || number_length > 6 || (is_a_space(buffer[index]) == -1 && buffer[index] != '\n')){
                new_command.query = IGNORE;
                free(number);
                return new_command;
            }
            int result = parse_number(number);
            if (result == -1){
                new_command.query = IGNORE;
                free(number);
                return new_command;
            }
            new_command.int_args[current_int_arg] = result;
            current_int_arg++;
            free(number);
        }
        else if (expression[i] == '$'){
            int word_begin = index;
            int word_length = 0;
            while (is_small_letter(buffer[index]) == 1){
                ++word_length;
                ++index;
            }
            if (word_length == 0 || (is_a_space(buffer[index]) == -1 && buffer[index] != '\n')){
                // there needs to be whitespace / endline after the word.
                new_command.query = IGNORE;
                return new_command;
            }
            index = word_begin;
            new_command.string_arg = calloc(word_length + 1, sizeof(char));
            for (int i = 0; i < word_length; i++){
                new_command.string_arg[i] = buffer[index];
                ++index;
            }
        }
        else if (expression[i] == '!'){
            while (is_a_space(buffer[index]) == 1){
                ++index;
            }
            if (buffer[index] != '\n'){
                new_command.query = IGNORE;
            }
        }
        else{
            if (expression[i] == buffer[index]){
                ++index;
            }
            else{
                new_command.query = IGNORE;
                return new_command;
            }
        }
    }
    return new_command;
}
