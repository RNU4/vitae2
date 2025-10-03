#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "file_handler.h"
#include "structs.h"
#include "./lib/stack.h"
#include "tokenizer.h"
#include "./shunting_yard.h"
#include "assembler.h"
#include "final_pass.h"
stack_t *tokenize(tokenizer_t* tokenizer,char *code);

char* type_to_string[] = {
    "KEYWORD",
    "IDENTIFIER",
    "NUMBER",
    "FUNCTION",
    "STRING",
    "OPERATOR",
    "COMMENT",
    "NEWLINE",
    "EOF",
}; 
/*
line 1 type IDENTIFIER : [function] 
line 1 type FUNCTION : [add] 
line 1 type OPERATOR : [(]
line 1 type IDENTIFIER : [int]
line 1 type IDENTIFIER : [a]
line 1 type OPERATOR : [,]
line 1 type IDENTIFIER : [int]
line 1 type IDENTIFIER : [b]
line 1 type OPERATOR : [)]
line 1 type KEYWORD : [int]
*/

int main(){
    //char code[] = "function add(int* arg1,int arg2,int arg4) int:";//"function add(int a, int b) int\n return a + b \nend\n";

    //char code[] = "x=max(2.3,3)\n;";
    //har code[] = "(*(x.y*2 + 1))";
    // "function add(a,b): x = a[2] end"
/*
    char code[] = 
    "function main():\n"
        "a = 1.1\n" 
        "b = 2\n" 
        "c = test(a,b,10)+1\n"
    "end\n"
    "function test(x,y,z):\n"
        "return = x+y+z\n"
    "end\n";
*/
    char* code = read_file("./code.pocket");

    
    tokenizer_t* tokenizer = calloc(sizeof(tokenizer_t),1);
    tokenize(tokenizer,code);
    for (int i = 0; i<tokenizer->tokens->top; i++){
        printf("line %d type %s : [%s] \n",((token_t*)stack_check(tokenizer->tokens,i))->line_number,
        type_to_string[((token_t*)stack_check(tokenizer->tokens,i))->type],
        ((token_t*)stack_check(tokenizer->tokens,i))->value);
    }


    printf("\nassembler:\n");
    
    char* str = stack_to_string(final_pass(assemble(tokenizer)),",");
    file_write("test.rpn",str,strlen(str));
}