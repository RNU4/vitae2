#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "./lib/stack.h"
#include "structs.h"
#include "./tokenizer_helper.h"
#ifndef _TOKENIZER_H_
#define _TOKENIZER_H_


token_t* create_token(enum token_type type, char *value ,int line_number){
    token_t * t = (token_t*)malloc(sizeof(token_t));
    t->line_number = line_number;
    t->type = type;
    t->value = cpystring(value);
    return t;
}

int is_keyword(char *str) {
    char *keywords[] = {
    "function", "if", "else", "elseif", "end", "for", 
    "while", "repeat", "until", "local","const", "extern",
    "true", "false", "nil",":","do","then"
    };
    
    printf("keyword '%s'\n",str);
    int length = sizeof(keywords) / sizeof(keywords[0]);
    for (int i = 0; i < length; i++) {
        if (strcmp(str, keywords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}
/*count ; as a newline*/
int is_newline(char *ptr) {
    return (*ptr == '\n' || *ptr == ';');
}


bool tokenizer_is_keyword_or_identifier(char *ptr){
    if (isalpha(*ptr) || *ptr == '_' || *ptr == ':') {
            return true;
    } 
    return false;
}

bool tokenizer_handle_keyword_and_identifier(tokenizer_t* tokenizer){
    char *start = tokenizer->ptr;
    while (isalnum(*tokenizer->ptr) || *tokenizer->ptr == '_') {
        tokenizer->ptr++;
    }
    
    if (start == tokenizer->ptr){
        if (*start == ':'){
            tokenizer->ptr++;
        }else{
            return false;
        }
    }

    int length = tokenizer->ptr - start;
    
    char* value = malloc(length + 1);
    strncpy(value, start, length);
    value[length] = '\0';
    int type = is_keyword(value) ? KEYWORD : IDENTIFIER;

    stack_push(tokenizer->tokens,
    (void*)create_token(type,
    value,
    tokenizer->current_line_number
    ));
}

bool tokenizer_is_comment(char *ptr){
    if (*ptr == '#') {
        return true;
    } 
    return false;
}

bool tokenizer_is_string(char *ptr){
    return (*ptr == '"' || *ptr == '\'' || *ptr == '`');
}
bool tokenizer_handle_string(tokenizer_t* tokenizer){

    char quote = *tokenizer->ptr;
    tokenizer->ptr++;
    char *start = tokenizer->ptr;
    while (*tokenizer->ptr != quote && *tokenizer->ptr != '\0') {
        tokenizer->ptr++;
    }
    int length = tokenizer->ptr - start;
    char* value = malloc(length + 1);
    strncpy(value, start, length);
    value[length] = '\0';
    stack_push(tokenizer->tokens,(void*)create_token(STRING,value,tokenizer->current_line_number));


    if (*tokenizer->ptr == quote) {
        tokenizer->ptr++;
    }
}



bool tokenizer_handle_comment(char **ptr){
    //*ptr += 1; // already checked first 2 chars so skip them
    while (**ptr != '\n' && **ptr != '\0') {
        *ptr++;
    }
    return true;
}

bool tokenizer_handle_numbers(tokenizer_t* tokenizer){
        char *start = tokenizer->ptr;



        // is it a minus number?
        token_t* token = stack_peak(tokenizer->tokens);
        if (token->type == OPERATOR && token->value[0]=='-'){
            token_t* token2 = stack_check(tokenizer->tokens,tokenizer->tokens->top-2);
            if (token2!=NULL && token2->type == OPERATOR && token2->value[0]!=')'){
                if (tokenizer->start<=start-1 && *(start-1)=='-'){
                    token_t* del = stack_pop(tokenizer->tokens);
                    free(del->value);
                    free(del);
                    *start--;
                }
            }
        }


        while (isdigit(*tokenizer->ptr) || *tokenizer->ptr=='.' ) {
            tokenizer->ptr++;
        }
        int length = tokenizer->ptr - start;
        char* value = (char*)malloc(length + 1);
        strncpy(value, start, length);
        value[length] = '\0';
        stack_push(tokenizer->tokens,(void*)create_token(NUMBER,value,tokenizer->current_line_number));
}

bool tokenizer_is_operator(char *ptr){
    if (strchr("+-*/%^#=<>;,.(){}[]$", *ptr) != NULL) {
        return true;
    }
    return false;
}

bool tokenizer_is_minus_number(char *ptr){
    if (*ptr == '-') {
        return true;
    }
    return false;
}




bool tokenizer_handle_operators(tokenizer_t* tokenizer){
    if (tokenizer->ptr+1 != NULL && strchr("=<>", *(tokenizer->ptr)) != NULL){
        if (strchr("=<>", *(tokenizer->ptr+1)) != NULL){
            char* value = (char*)malloc(3);
            value[0] = *tokenizer->ptr++;
            value[1] = *tokenizer->ptr++;
            value[2] = '\0';
            stack_push(tokenizer->tokens,(void*)create_token(OPERATOR,value,tokenizer->current_line_number));
            return true;
        }
    }
    char* value = (char*)malloc(2);
    value[0] = *tokenizer->ptr++;
    value[1] = '\0';
    stack_push(tokenizer->tokens,(void*)create_token(OPERATOR,value,tokenizer->current_line_number));
}

int tokenizer_handle_tables(tokenizer_t* tokenizer,stack_t* new_stack, int index){
    int left = 1;
    int right = 0;
    index++;

    while(left != right ){
        token_t* token = (token_t*)stack_check(tokenizer->tokens,index);
        assert(token != NULL);
        printf("token : %s \n",token->value);

        if (token->type == NEWLINE){ // we will ignore newlines and "remove" & free
           // printf("Error expected '}' got NL\n"); exit(2);
           free(token->value);
           free(token);
            index++; 
           continue;
        }

        if (index >= tokenizer->tokens->top){
            printf("Error expected '}'\n"); 
            printf("%d %d",tokenizer->tokens->top,index);
            exit(2);
        }

        if (token->type == OPERATOR && token->value[0] == '}'){
            free(token); // we replace it;
            stack_push(new_stack,(void*)create_token(OPERATOR,")",token->line_number));
            right++;
        }else if (token->type == OPERATOR && token->value[0] == '{'){
            free(token); // we replace it;
            stack_push(new_stack,(void*)create_token(FUNCTION,"create_table",token->line_number));
            stack_push(new_stack,(void*)create_token(OPERATOR,"(",token->line_number));
            left++;
        }else if (token->type == KEYWORD && token->value[0] == ':'){
            free(token); // we replace it;
            stack_push(new_stack,(void*)create_token(OPERATOR,",",token->line_number));
        }else if ((token->type == IDENTIFIER) && token_get_next_first_char(tokenizer,index) == '('){
            token->type = FUNCTION;
            
            stack_push(new_stack,token);
        } else {
            stack_push(new_stack,token);
        }

        index++;
    }

    return index-1;
}



void tokenizer_last_pass(tokenizer_t* tokenizer){
    //check for functions or keywords acting like functions
    /*
    for (int i = 0; i<tokenizer->tokens->top; i++){
        printf("line %d : [%s] \n",((token_t*)stack_check(tokenizer->tokens,i))->line_number,
        ((token_t*)stack_check(tokenizer->tokens,i))->value);
    }
*/
    token_t* last_token = NULL;
    stack_t* new_stack = stack_new(tokenizer->tokens->size);
    int for_count = 0;
    for (int i = 0 ; i < tokenizer->tokens->top; i++){
        token_t* token = (token_t*)stack_check(tokenizer->tokens,i);
        assert(token != NULL);
        assert(token->value != NULL);
        if ((token->type == IDENTIFIER) && (strcmp(token->value,"return")!=0) && token_get_next_first_char(tokenizer,i) == '('){
            if (last_token!=NULL){
                if (strcmp(last_token->value,"function") != 0)
                token->type = FUNCTION;
            }else{
            token->type = FUNCTION;
            }
            stack_push(new_stack,token);
        } else if (token->type == OPERATOR && token->value != NULL && token->value[0] == '{'){
            stack_push(new_stack,(void*)create_token(FUNCTION,"create_table",token->line_number));
            stack_push(new_stack,(void*)create_token(OPERATOR,"(",token->line_number));
            
            i = tokenizer_handle_tables(tokenizer,new_stack,i);
        }else if (token->type == IDENTIFIER && token->value != NULL && (strcmp(token->value,"return")==0)){
            
            stack_push(new_stack,token); // push return
            
            //push equal
            stack_push(new_stack,
            (void*)create_token(OPERATOR,
            cpystring("="),tokenizer->current_line_number
            ));
        }else{
            stack_push(new_stack,token);
        }

/*
    if (strcmp(value,"return")==0){
            stack_push(tokenizer->tokens,
            (void*)create_token(OPERATOR,
            cpystring("="),tokenizer->current_line_number
            ));
    }
*/
        /*convert pointer symbol '*' into '$' for easier handling*/
        /*
        if (token->type == OPERATOR && token->value[0] == '*' && (last_token == NULL || last_token->type != IDENTIFIER || last_token->type == OPERATOR) 
        && (last_token == NULL || (last_token!=NULL && last_token->value[0]!=')'))){
            token->value[0] = '$';
        }
*/
        last_token = token;
    }

    stack_free(tokenizer->tokens);
    tokenizer->tokens = new_stack;
}

stack_t *tokenize(tokenizer_t* tokenizer,char *code) {
    tokenizer->ptr = code;
    tokenizer->start = code;
    tokenizer->current_line_number = 1; // first linenumber is 1
    tokenizer->tokens = stack_new(32);

    while (*(tokenizer->ptr) != '\0') {

        if (is_newline(tokenizer->ptr)){
            stack_push(tokenizer->tokens,(void*)create_token(NEWLINE,"NL",tokenizer->current_line_number));
            tokenizer->current_line_number++;
            tokenizer->ptr++;
            continue;
        }

        if (tokenizer_is_comment(tokenizer->ptr)){
            //tokenizer_handle_comment(&tokenizer->ptr); // remove the comment
            while (*tokenizer->ptr != '\n' && *tokenizer->ptr != '\0') tokenizer->ptr++;
            continue;
        }

        if (isdigit(*tokenizer->ptr)) {
            tokenizer_handle_numbers(tokenizer);
            continue;
        }

        if (tokenizer_is_operator(tokenizer->ptr)) {
            tokenizer_handle_operators(tokenizer);
            continue;
        }

        if (tokenizer_is_keyword_or_identifier(tokenizer->ptr)) {
            tokenizer_handle_keyword_and_identifier(tokenizer);
            continue;
        }



        if (tokenizer_is_string(tokenizer->ptr)){
            tokenizer_handle_string(tokenizer);
            continue;
        }

        tokenizer->ptr++;
    }

    tokenizer_last_pass(tokenizer);
    /*end of file*/
    stack_push(tokenizer->tokens,(void*)create_token(END_OF_FILE,"EOF",tokenizer->current_line_number));


    return tokenizer->tokens;
}

#endif