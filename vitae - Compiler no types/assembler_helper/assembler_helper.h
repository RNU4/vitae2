
#ifndef ASSEMBLER_HELPER_H_   /* Include guard */
#define ASSEMBLER_HELPER_H_
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../lib/stack.h"
#include "../structs.h"
#include "../tokenizer_helper.h"
#include "../tokenizer.h"

bool assembler_is_function(assembler_t* assembler, token_t* token){
    if (token->type == FUNCTION){
        if (hash_table_check(&assembler->functions,token->value) == false){
            /*function refrenced but is not declared*/
            printf("error at line %d function '%s' has not been declared\n",token->line_number,token->value);
            exit(2);
        }
        return true;
    } 
    return false;
}

bool assembler_is_expression(token_t* token){
    if (token->type == IDENTIFIER || token->type == NUMBER || token->type == FUNCTION || token->type == OPERATOR){
        return true;
    }
    return false;
}

bool assembler_is_newline(token_t* token){
    if (token->type == NEWLINE || token->type == EOF){
        return true;
    }
    return false;
}

bool assembler_is_assignment(token_t* token){
    /*the token could be '==' so make sure its only '='*/
    if (token->type == OPERATOR && strcmp(token->value,"=") == 0){
        return true;
    }
    return false;
}
#endif