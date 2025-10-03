#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./lib/stack.h"
#include "structs.h"

#ifndef TOKENIZER_HELPER_H_   /* Include guard */
#define TOKENIZER_HELPER_H_

int token_get_next_first_char(tokenizer_t* tokenizer, int i){
    if (i+1 < tokenizer->tokens->top){
        return (((token_t*)tokenizer->tokens->data[i+1])->value[0]);
    }else{
        return -1;
    }
}

token_t* next_token(tokenizer_t* tokenizer, int i){
    if (i+1 < tokenizer->tokens->top ){
        return ((token_t*)tokenizer->tokens->data[i+1]);
    } else{
        return NULL;
    }
}

token_t* check_token(stack_t* stack, int i){
    if (i < stack->top && i>=0){
        return ((token_t*)stack->data[i]);
    } else{
        return NULL;
    }
}


char* cpystring(char* str){
    char *heap_str = malloc(strlen(str) + 1);
    strcpy(heap_str, str);
    return heap_str;
}

void split_on_char(const char *str, char c, char **left, char **right) {
    const char *pos = strchr(str, c);
    if (!pos) {
        // char not found → left = full string, right = empty
        *left = strdup(str);
        *right = strdup("");
        return;
    }

    size_t left_len = pos - str;
    *left = (char*)malloc(left_len + 1);
    strncpy(*left, str, left_len);
    (*left)[left_len] = '\0';

    *right = strdup(pos + 1); // everything after the char
}

#endif