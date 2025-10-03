#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./lib/stack.h"
#include "structs.h"
#include "./tokenizer_helper.h"
/*
Operator	Precedence	Associativity
^	4	Right
*	3	Left
/	3	Left
+	2	Left
-	2	Left
*/
enum shunting_yard_precedence {
    SY_OPERATOR = 0,
    SY_PRECEDENCE = 1,
    SY_ASSOCIATIVITY = 2,
    SY_POW = 0,
    SY_MOD = 1,

    SY_MUL = 1,
    SY_DIV = 2,
    SY_ADD = 3,
    SY_SUB = 4,
    SY_SET = 5,
    SY_GREATER = 6,
    SY_LESS = 7,
    SY_MEMBER_ACCESS = 8,
    SY_SET_VALUE = 8,
    SY_TABLE_SET_VALUE = 8,
    SY_GET_VALUE = 8,

    SY_POINTER = 9,

};
int sy_get_info(char c){
    switch (c)
    {
    case '^': return SY_POW;
    case '*': return SY_MUL;
    case '/': return SY_DIV;
    case '+': return SY_ADD;
    case '-': return SY_SUB;
    case '%': return SY_MOD;

    case '=': return SY_SET;
    case '>': return SY_GREATER;
    case '<': return SY_LESS;
    case '@': return SY_MEMBER_ACCESS;
    case '|': return SY_TABLE_SET_VALUE;
    case '!': return SY_SET_VALUE;
    case '?': return SY_GET_VALUE;

    case '$': return SY_POINTER;

    default:
        return -1; /*error i guess*/
        break;
    }
}

/*//TODO add string support?*/
stack_t* shunting_yard(tokenizer_t* tokenizer,stack_t* stack){
    /*
    SY_ASSOCIATIVITY is not used since ^ would be pow() instead and ^ being a binary operation
    just keeping it there if i want to reuse the function for other projects
    */

    char* precedence[] = {
        "^\4\1",
        "*\3\0",
        "/\3\0",
        "+\2\0",
        "-\2\0", 
        "=\1\0", 
        "<\5\0", 
        ">\5\0", 
        ".\5\0", 
        "$\5\0",
    };

    stack_t* output = stack_new(20);
    stack_t* operator_stack = stack_new(20);

    

    for (int i = 0; i<stack->top; i++){
        token_t* token = ((token_t*)stack->data[i]);
        token_t* token_next = next_token(tokenizer,i);
        int sy_info = sy_get_info(token->value[0]);
        //printf("token %d %s\n",token->type,token->value);
        if (token->type == NUMBER || token->type == IDENTIFIER || token->type == STRING){ // append number, variable and string
            stack_push(output,token);
        } else if (token->type == FUNCTION){
            stack_push(operator_stack,token);
        } else if (sy_info !=-1 || token->type == FUNCTION){ // its a "operator"
            while (operator_stack->top > 0 && 
                /*compare precedence*/
                ((sy_get_info(((token_t*)stack_peak(operator_stack))->value[0]) == -1) ? 0 : //? function 0 presedent? maybe 5 just test later
                precedence[sy_get_info(((token_t*)stack_peak(operator_stack))->value[0])][SY_PRECEDENCE] )
                >= precedence[sy_info][SY_PRECEDENCE] &&

                ((token_t*)stack_peak(operator_stack))->value[0] != '(' ) {
                    /*pop o2 from the operator stack into the output queue*/
                    stack_push(output,stack_pop(operator_stack));
            }

            /*combine < = into one <=*/
            if ((token->value[0] == '=' || token->value[0] == '<' || token->value[0] == '>') 
            && token_next->value[0]=='='){
                
                token_t* t = malloc(sizeof(token));
                t->value = calloc(3,sizeof(char));
                t->value[0] = token->value[0];
                t->value[1] = token_next->value[0];
                t->line_number = token->line_number;
                t->type = OPERATOR;
                stack_push(operator_stack,t);
                i++; //skip the next token
            }else{
                /*push o1 onto the operator stack*/
                stack_push(operator_stack,token); 
            }




        } else if (token->value[0]==','){
            while (operator_stack->top > 0 &&  ((token_t*)stack_peak(operator_stack))->value[0] != '(' ){ /* while the operator at the top of the operator stack is not a left parenthesis*/
                stack_push(output,stack_pop(operator_stack)); /*pop the operator from the operator stack into the output queue*/

            }
        } else if (token->value[0]=='('){
            stack_push(operator_stack,token);
        }else if (token->value[0]==')'){
            while (operator_stack->top > 0 &&  ((token_t*)stack_peak(operator_stack))->value[0] != '(' ){ 
                stack_push(output,stack_pop(operator_stack));

            }
            /* If the stack runs out without finding a left parenthesis, then there are mismatched parentheses. */
            if (operator_stack->top <= 0) {
                printf("Error at line %d: Expected matching parenthesis\n",token->line_number);
                exit(2);
            }

            /*
            {assert there is a left parenthesis at the top of the operator stack}
            op the left parenthesis from the operator stack and discard it*/
            if (operator_stack->top > 0 && ((token_t*)stack_peak(operator_stack))->value[0] == '(' ){
                stack_pop(operator_stack);
            }

            if (operator_stack->top > 0 && ((token_t*)stack_peak(operator_stack))->type == FUNCTION){
                stack_push(output,stack_pop(operator_stack));
            }
        }
    }

    /* After the while loop, pop the remaining items from the operator stack into the output queue. */
    while(operator_stack->top != 0){
        /* If the operator token on the top of the stack is a parenthesis, then there are mismatched parentheses. */
        token_t* token = (token_t*)stack_pop(operator_stack);
        if (token->value[0] == '(' || token->value[0] == ')'){
            printf("Error at line %d : Expected matching parenthesis",token->line_number);
            exit(2);
        }
        stack_push(output,token);
    }

    stack_free(operator_stack); /*free the stack since it is not used anymore*/
    /*
    printf("\nOutput\n");
    for (int i = 0; i<output->top; i++){
    printf("%s ",
    ((token_t*)output->data[i])->value);
    }

    */
    return output;
}
