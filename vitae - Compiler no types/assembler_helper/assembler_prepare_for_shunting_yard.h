
#ifndef ASSEMBLER_PREPARE_FOR_SHUNTING_YARD_H_   /* Include guard */
#define ASSEMBLER_PREPARE_FOR_SHUNTING_YARD_H_
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


int count_function_args(stack_t *stack, int start) {
    int depth = 0;   // parenthesis nesting
    int args = 0;    // number of top-level arguments
    int in_arg = 0;  // are we currently inside an argument?

    for (int i = start; i <= stack->top; i++) {
        token_t *t = (token_t*) stack->data[i];
        char c = t->value[0];  // assume single-char tokens like '(', ')', ','
  
        if (t->type == OPERATOR && c == '(') {
            depth++;
            if (depth == 1) {
                // skip the function's opening '('
                continue;
            }
        }
        else if (t->type == OPERATOR && c == ')') {
            depth--;
            if (depth == 0) {
                if (in_arg) args++; // finalize last arg
                break;              // stop at matching ')'
            }
        }
        else if (t->type == OPERATOR && c == ',' && depth == 1) {
            if (in_arg) {
                args++;
                in_arg = 0;
            }
            continue;
        }

        // any other token inside the () counts toward an argument
        if (depth >= 1 && (t->type != OPERATOR && c != ',')) {
            in_arg = 1;
        }
    }

    return args;
}

bool assembler_prepare_for_shunting_yard_function(assembler_t* assembler, token_t* token, stack_t* temp_stack, int side, int assignment, stack_t* stack, int i){
    char str[100];
    assembler_is_function(assembler,token); // will throw error if it does not exsist
    function_t* fn = hash_table_get(&assembler->functions,token->value);
    for(int i = 0; i < stack->top; i++){
        printf("token : %s\n",((token_t*)stack->data[i])->value);
    }

    int arg_count = count_function_args(stack,i);

    if (fn->is_extern){ // count how many args there is
        printf("arg count %d\n",arg_count);
        
        sprintf(str,"%s$%d",fn->name,arg_count);
        free(token->value);
        token->value = cpystring(str);
        stack_push(temp_stack, token);
        return true;

    }else{ // check if you have right amount of arguments
        if (assignment && side == 1 && fn->has_return == false){ // right side and no return, big no no
            printf("Error at line %d, function '%s' used as right side value, but does not return a value",token->line_number,fn->name);
            exit(2);
        }

        if (arg_count!=fn->argument_count){
            printf("Error at line %d, function '%s' expected %d %s, but got %d\n",token->line_number,fn->name,fn->argument_count,fn->argument_count > 1 ? "arguments" : "argument",arg_count);
            exit(2);
        }
    }
    return false;
}

void emit_get_value(stack_t *temp_stack, stack_t *mem_clear, token_t *token) {
    stack_push(mem_clear, create_token(OPERATOR, "?", token->line_number));
    stack_push(temp_stack, stack_peak(mem_clear));
}

 void emit_access(stack_t *temp_stack, stack_t *mem_clear, token_t *token) {
    // opening parenthesis
    stack_push(mem_clear, create_token(OPERATOR, "(", token->line_number));
    stack_push(temp_stack, stack_peak(mem_clear));

    // identifier
    stack_push(temp_stack, token);

    // get value
    emit_get_value(temp_stack, mem_clear, token);

    // closing parenthesis
    stack_push(mem_clear, create_token(OPERATOR, ")", token->line_number));
    stack_push(temp_stack, stack_peak(mem_clear));
}

/*
TODO should take strings into account when checking for $
*/
// TODO rewrite it because this is just a mess, it should be 2 functions and also it uses a lot of "fixes" to fix bad logic
// it will defo break or cause issues but it should be fine as a testing ground for now
stack_t* assembler_prepare_for_shunting_yard(assembler_t* assembler,stack_t* stack, int side,int assignment) {

    char str[100];
    stack_t* temp_stack = stack_new(stack->size);
    printf("stack:\n");

    for (int i = 0; i < stack->top; i++) {
    token_t* token = ((token_t*)stack->data[i]);
    printf("%s\n",token->value);
    }
    
    token_t* last_token = NULL;
    token_t* next_token = NULL;
    for (int i = 0; i < stack->top; i++) {
        token_t* token = ((token_t*)stack->data[i]);
        next_token = stack_check(stack,i+1);
        assert(token != NULL);
        assert(token->value != NULL);

        if (token->type == FUNCTION){
            if (assembler_prepare_for_shunting_yard_function(assembler, token, temp_stack, side, assignment, stack, i)) continue;
        }

        if (last_token!=NULL && last_token->type == IDENTIFIER && token->type == OPERATOR && token->value[0]=='.'){
            if (i+1 > stack->top){
                printf("ERROR: expected identifier at %d\n",token->line_number);
                exit(2);
            }
            stack_push(assembler->memory_to_clear,create_token(OPERATOR, "(", token->line_number));
            stack_push(temp_stack, stack_peak(assembler->memory_to_clear));
            
            token_t* token = ((token_t*)stack->data[++i]);
            assert(token != NULL);
            assert(token->value != NULL);
            if (token->type!=IDENTIFIER){
                printf("ERROR: expected identifier at %d\n",token->line_number);
                exit(2);    
            }
            token->type = STRING;
            stack_push(temp_stack, token);

            if ((i+1!=stack->top || side != 0)){
                stack_push(assembler->memory_to_clear,create_token(OPERATOR, ")", token->line_number));
                stack_push(temp_stack, stack_peak(assembler->memory_to_clear));

                stack_push(assembler->memory_to_clear,create_token(OPERATOR, "(", token->line_number));
                stack_push(temp_stack, stack_peak(assembler->memory_to_clear));


                stack_push(assembler->memory_to_clear,create_token(OPERATOR, "@", token->line_number));
                stack_push(temp_stack, stack_peak(assembler->memory_to_clear));
            }else if (i+1==stack->top && side == 0){
                stack_push(assembler->memory_to_clear,create_token(OPERATOR, "|", token->line_number));
                stack_push(temp_stack, stack_peak(assembler->memory_to_clear));

            }else{
                printf("ERROR: at line %d\n",token->line_number);
                exit(2);     
            }

            stack_push(assembler->memory_to_clear,create_token(OPERATOR, ")", token->line_number));
            stack_push(temp_stack, stack_peak(assembler->memory_to_clear));
            continue;
        }

        if (token->type == OPERATOR && token->value[0] == '['){
   
            if (last_token != NULL && last_token->type == OPERATOR && last_token->value[0] == ']' && side == 1){
              /*
                stack_push(temp_stack, token);
                stack_push(assembler->memory_to_clear,create_token(OPERATOR, "?", token->line_number)); // get value
                stack_push(temp_stack, stack_peak(assembler->memory_to_clear));    */
            }
            
            stack_push(assembler->memory_to_clear,create_token(OPERATOR, "(", token->line_number));
            stack_push(temp_stack, stack_peak(assembler->memory_to_clear));

        } else if (token->type == OPERATOR && token->value[0] == ']'){

            if ((i+1!=stack->top || side != 0)){ //!modified test
                stack_push(assembler->memory_to_clear,create_token(OPERATOR, ")", token->line_number));
                stack_push(temp_stack, stack_peak(assembler->memory_to_clear));

                stack_push(assembler->memory_to_clear,create_token(OPERATOR, "(", token->line_number));
                stack_push(temp_stack, stack_peak(assembler->memory_to_clear));


                stack_push(assembler->memory_to_clear,create_token(OPERATOR, "@", token->line_number));
                stack_push(temp_stack, stack_peak(assembler->memory_to_clear));
            }

            stack_push(assembler->memory_to_clear,create_token(OPERATOR, ")", token->line_number));
            stack_push(temp_stack, stack_peak(assembler->memory_to_clear));

        }

        if (i+1==stack->top){ // end of stack
            if (side == 0){ //left
                //TODO check if valid lvalue
                //if there is more left error yk
                token_t* tmp_token = ((token_t*)stack->data[0]);
                if (tmp_token->type!=IDENTIFIER || (token->value[0]!=']' && stack->top>1)){
                    printf("ERROR: invalid lvaue at line %d\n",token->line_number);
                    exit(2);
                }
                stack_push(temp_stack, stack->data[i]); 
                if (token->value[0]!=']'){ //!modified test set table value
                    stack_push(assembler->memory_to_clear,create_token(OPERATOR, "!", token->line_number)); // set value
                }else{ //set normal varible value
                  stack_push(assembler->memory_to_clear,create_token(OPERATOR, "|", token->line_number)); // set value  
                }
                stack_push(temp_stack, stack_peak(assembler->memory_to_clear));
            }
        }else{ // get value
            bool is_table_access = ( next_token != NULL && next_token->type == OPERATOR && (next_token->value[0] == '[' || next_token->value[0] == '.'));
            bool is_variable = (token->type == IDENTIFIER &&(side != 0 || i != 0) && !is_table_access);

            if (token->type == IDENTIFIER) {
                if (is_table_access) {
                    emit_access(temp_stack, assembler->memory_to_clear, token);
                } else if (is_variable) {
                   // stack_push(temp_stack, token); // identifier
                   // emit_get_value(temp_stack, assembler->memory_to_clear, token);
                    emit_access(temp_stack, assembler->memory_to_clear, token);

                } else {
                    printf("INTERNAL ERROR\n");
                    assert(false);
                    //stack_push(temp_stack, token);
                }
            } else {
                stack_push(temp_stack, token);
            }
        }

        
        last_token = token;
    }
        return temp_stack;
}

#endif