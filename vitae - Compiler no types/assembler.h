#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "./lib/stack.h"
#include "structs.h"
#include "tokenizer_helper.h"
#include "tokenizer.h"
#include "assembler_helper/assembler_helper.h"
#include "assembler_helper/assembler_prepare_for_shunting_yard.h"

bool assembler_is_function_definition(token_t* token){
    if (token->type == KEYWORD && strcmp(token->value,"function") == 0){
        return true;
    } 
    return false;
}

int assembler_count_stars(assembler_t* assembler){
    token_t* token;
    int stars = -1;

    do{
        token = check_token(assembler->tokenizer->tokens,assembler->current_token++);
        assert(token != NULL);
        stars++;
    } while(token->value[0]=='$');
    assembler->current_token--;
    return stars;
}


void assembler_check_function_definition_end(assembler_t* assembler){

    token_t* token;

    token = check_token(assembler->tokenizer->tokens,assembler->current_token);
    assert(token != NULL);
    if (token->value[0] != ':'){
        printf("Error expected ':' got %s",token->value);
        exit(2);
    }
}

function_t* assembler_handle_function_definition(assembler_t* assembler){
    /*current token is function*/
    /*function example(int x,int y) int*/
    token_t* token;

    function_t* function_definition = function_create();
    function_definition->argument_count = 0;
    //function_definition->arguments
    /*get name of function*/
    token = check_token(assembler->tokenizer->tokens,++assembler->current_token);
    assert(token != NULL);
    if (token->type == IDENTIFIER){
        /* //TODO check if function already is defined, but not its body and compare to make sure the function definition is the same*/
        function_definition->name = token->value;
    }else{
        printf("error at line %d expected function name got '%s'\n",token->line_number,token->value);
        exit(2);
    }

    /* is '('*/
    token = check_token(assembler->tokenizer->tokens,++assembler->current_token);
    assert(token != NULL);

    if ( token->value[0] == '(' ){
        
        while(true){            

            char* variable_name = NULL;

            token = check_token(assembler->tokenizer->tokens,++assembler->current_token);
            assert(token != NULL);
            
            /*check if there is no arguments*/
            if (token->type==OPERATOR && token->value[0] == ')'){ 
                assembler->current_token++;
                break;
            }

            /*we should be at name now*/
            if (token->type==IDENTIFIER){
                variable_name = token->value;
                assembler->current_token++;
            }else{
                /*Error expected argument name*/
                printf("Error expected argument name at line %d, got '%s'",token->line_number,token->value);
                exit(2);
            }

            /*insert argument into the function definition hashtable*/
            assert(variable_name != NULL);
            /*check if argument already is defined */
            if (hash_table_check(&function_definition->arguments,variable_name)==true){
                printf("Error Repeated argument name at line %d, got '%s'",token->line_number,token->value);
                exit(2);
            }
            hash_table_insert(&function_definition->arguments,variable_name,(void*)(size_t)(function_definition->argument_count++));
            stack_push(function_definition->argument_list,cpystring(variable_name));


            /*check that we are indeed at a ',' or if finished ')'*/
            token = check_token(assembler->tokenizer->tokens,assembler->current_token);
            assert(token != NULL);

            if (token->value[0] == ','){
                continue;
            }else if ( token->value[0] == ')' ){
                assembler->current_token++;
                break;
            }else{
                /*Error expected ',' or ')' */
                printf("Error expected ',' or ')' at line %d, got '%s'",token->line_number,token->value );
                exit(2);
            }
        }

    }else{
        printf("error at line %d expected '(' got '%s'\n",token->line_number,token->value);
        exit(2);
    }


    /* make sure it ends with ":" fx funtion test(): */
    assembler_check_function_definition_end(assembler);

    printf("Function %s \n",function_definition->name);

    return function_definition;


}



expression_t* evaluate_expression(assembler_t* assembler,expression_t* expression){

    /*Evaluate left side*/
    stack_t* lstack = expression->l_tokens;

    /*Evaluate right side*/
    stack_t* rstack = expression->r_tokens;
}



expression_t* assembler_handle_assignment(assembler_t* assembler,int statement_start,int statement_end){
    expression_t* expression = calloc(sizeof(expression_t),1);
    
    stack_t* temp_stack = stack_copy(assembler->tokenizer->tokens,statement_start,statement_end);
    stack_t* prepared_temp_stack = assembler_prepare_for_shunting_yard(assembler,temp_stack,0,true);
    stack_free(temp_stack);

    /*Run shunting yard to make it into rpn*/
    stack_t* rpn_stack = shunting_yard(assembler->tokenizer,prepared_temp_stack);
    stack_free(prepared_temp_stack);

    expression->l_tokens = rpn_stack;


    
    return expression;

}



expression_t* assembler_handle_expression(assembler_t* assembler,int statement_start){

    token_t* token = NULL;
    token_t* token_last = NULL;
    expression_t* expression = NULL;
    int assignment_pos = 0;
    int start = assembler->current_token;

    //dont include newlines in start of expression
  /*
    token = ((token_t*)assembler->tokenizer->tokens->data[assembler->current_token]);
    while (token->type == NEWLINE && token->type != END_OF_FILE){
        token = ((token_t*)assembler->tokenizer->tokens->data[assembler->current_token++]);

    }
        */
    /*Find statement end && if its assignment*/
    do {
        token = ((token_t*)assembler->tokenizer->tokens->data[assembler->current_token++]);
        assert(token != NULL);
        if (strcmp(token->value,"=")==0){
            /*check if there is multiple assigments and throw a error*/
            if (assignment_pos != 0){
                printf("error at line %d multiple assignments on a single line not allowed\n",token->line_number);
                exit(2);
            }
            assignment_pos = assembler->current_token - 1;
        }
        token_last = token;
    } while(token->type != KEYWORD && token->type != NEWLINE && token->type != END_OF_FILE);
    

    token = ((token_t*)assembler->tokenizer->tokens->data[start]);
    if (assignment_pos == 0 && assembler_is_function(assembler,token)){
        function_t* p = hash_table_get(&assembler->functions,token->value);
        assert(p != NULL);
        if (p->has_return){ 
            printf("error at line %d, function '%s', unused return value.\n",token->line_number,token->value);
            exit(2);
        }
    }

    /*if assignment sign is found handle it*/
    if (assignment_pos != 0){

        printf("\n\nAssignment :");
        // L stack
        expression = assembler_handle_assignment(assembler,statement_start,assignment_pos);
        
        // R stack
        //! Changed from -1 to not, might cause problems but idk
        stack_t* r_stack = stack_copy(assembler->tokenizer->tokens,assignment_pos+1,assembler->current_token); /*-1 to not include NL/EOF +1 to not include "="*/
        stack_t* r_stack_prepared = assembler_prepare_for_shunting_yard(assembler,r_stack,1,true);

        expression->r_tokens = shunting_yard(assembler->tokenizer,r_stack_prepared);

        stack_free(r_stack);
        stack_free(r_stack_prepared);
    }else{
        expression = calloc(sizeof(expression_t),1);


        stack_t* l_stack = stack_copy(assembler->tokenizer->tokens,start,assembler->current_token); /*-1 to not include NL/EOF*/
        stack_t* l_stack_prepared = assembler_prepare_for_shunting_yard(assembler,l_stack,1,false);

        expression->l_tokens = shunting_yard(assembler->tokenizer,l_stack_prepared);
        expression->r_tokens = NULL; /*calloc already sets it to null, its just for readability*/

        stack_free(l_stack);
        stack_free(l_stack_prepared);
    }
    
    if (expression->l_tokens!= NULL){
        printf("\nltokens : \n");
        for (int i = 0; i<expression->l_tokens->top; i++){
        printf("%s ",((token_t*)expression->l_tokens->data[i])->value);
        }
    }
    
    if (expression->r_tokens!= NULL){
        printf("\nrtokens : ");
        for (int i = 0; i<expression->r_tokens->top; i++){
        printf("%s ",((token_t*)expression->r_tokens->data[i])->value);
        }
    }

    assembler->current_token--; /* Not sure where it fixes it but it does*/
    assembler->current_token--; //! MODIFIED TEST
    return expression;


}

char* add_const_to_assembler(assembler_t* assembler, char* str,int type){ //TODO rewrite it
    char str_name[100];
    void* value;
    
    if (type == STRING){
        if (hash_table_check(&assembler->string_map,str) == true){
            value = hash_table_get(&assembler->string_map,str);
        }else{
            value = NULL;

        }
    }else if (type == FUNCTION){
        if (hash_table_check(&assembler->extern_function_map,str) == true){
            value = hash_table_get(&assembler->extern_function_map,str);
        }else{
            value = NULL;

        }        
    }else{
        assert(false);
    }
    

    if (value==NULL){ // not added yet
        if (type == STRING){
            sprintf(str_name,"#s%d",assembler->string_list->top);
            hash_table_insert(&assembler->string_map,str,cpystring(str_name));
            stack_push(assembler->string_list,cpystring(str));
        }else if (type == FUNCTION){
            sprintf(str_name,"#f%d",assembler->extern_function_list->top);
            hash_table_insert(&assembler->extern_function_map,str,cpystring(str_name));
            stack_push(assembler->extern_function_list,cpystring(str));
        }else{
            assert(false);
        }
    }else{ // has been added already
        return (char*)value;
    }

    return cpystring(str_name); /*malloc & return the name*/
}


void* tokens_to_bytecode(assembler_t* assembler,stack_t* tokens){
    /*safety checks*/
    assert(tokens != NULL);
    assert(tokens->data != NULL); 

    for (int i = 0; i<tokens->top; i++){
        token_t* ltoken = ((token_t*)tokens->data[i]);
        assert(ltoken->value != NULL); // just a quick check yk
        if (ltoken->type==OPERATOR){
            stack_push(assembler->rpn_tokens,(char*)hash_table_get(&assembler->mathematical_conversion,ltoken->value));
        }else if (ltoken->type == NUMBER){
            stack_push(assembler->rpn_tokens,cpystring("pushc"));
            if(strchr(ltoken->value, '.') != NULL)
            stack_push(assembler->rpn_tokens,cpystring("type_number"));
            else
            stack_push(assembler->rpn_tokens,cpystring("type_inumber"));

            stack_push(assembler->rpn_tokens,ltoken->value);
        } else if (ltoken->type==IDENTIFIER){
            stack_push(assembler->rpn_tokens,cpystring("dpushc"));
            stack_push(assembler->rpn_tokens,cpystring(ltoken->value));
        } else if (ltoken->type==FUNCTION){ //TODO! rewrite
            char *left, *right;
            function_t* fn = NULL;
            if (strchr(ltoken->value, '$')){
                split_on_char(ltoken->value, '$', &left, &right);
                fn = (function_t*)hash_table_get(&assembler->functions,left);
            }else{
                fn = (function_t*)hash_table_get(&assembler->functions,ltoken->value);
            }
            
            assert(fn != NULL);

            if (!is_keyword(ltoken->value)){ //!temp fix
                if (fn->is_extern == false)
                stack_push(assembler->rpn_tokens,cpystring("call"));
                else
                stack_push(assembler->rpn_tokens,cpystring("call_extern"));

            }
            if (fn->is_extern == false)
            stack_push(assembler->rpn_tokens,cpystring(ltoken->value));
            else {
                
                stack_push(assembler->rpn_tokens,add_const_to_assembler(assembler,left,FUNCTION));
                stack_push(assembler->rpn_tokens,right);
            }

            if (!is_keyword(ltoken->value) && fn->is_extern == false){ //!temp fix

                int length = snprintf(NULL, 0, "&%s_arg_count", cpystring(ltoken->value));
                char* str = malloc(length + 1);
                snprintf(str, length + 1, "&%s_arg_count", cpystring(ltoken->value));

                stack_push(assembler->rpn_tokens,str);            
            }                

        }else if (ltoken->type == STRING){
            stack_push(assembler->rpn_tokens,cpystring("pushc"));
            stack_push(assembler->rpn_tokens,add_const_to_assembler(assembler,ltoken->value,STRING));
        }
    }
}

void handle_function_arguments(assembler_t* assembler,function_t* f){
    char str[128];

    printf("%d\n",f->argument_list->top);
    int i;
    for (i = 0; i<f->argument_list->top; i++){
        sprintf(str,"!%s,%d",cpystring(f->argument_list->data[i]),(-((f->argument_list->top-i))));
        stack_push(assembler->rpn_tokens,cpystring(str));
    }
    sprintf(str,"!return,%d",(-i));
    stack_push(assembler->rpn_tokens,cpystring(str));
}

void handle_function_variables(assembler_t* assembler,tokenizer_t* tokenizer,function_t* f){
    char str[128];
    int start_count = 1;
    int end_count = 0;
    int func_current_token = assembler->current_token+1; // skip the ':'
    token_t* ftoken_last = NULL;
    stack_t* local_var_stack = stack_new(10);
    while(end_count != start_count){
        token_t* ftoken = ((token_t*)tokenizer->tokens->data[func_current_token++]);
        printf("next token : '%s'\n",ftoken->value);

        /*check varibles definitions and put them in a list*/
        token_t* ftoken_next = ((token_t*)tokenizer->tokens->data[func_current_token]);
        assert(ftoken_next != NULL); //never null because of EOF token

        if (ftoken_next->type == EOF){
            token_t* start_token = ((token_t*)tokenizer->tokens->data[assembler->current_token]);
            assert(start_token != NULL);
            printf("ERROR expected 'end' to close function, at line %d",start_token->line_number);
            exit(2);
        }

        // if its not "return"
        if (ftoken->type == IDENTIFIER && (ftoken_next->type == OPERATOR && strcmp(ftoken_next->value,"=")==0 && strcmp(ftoken->value,"return"))){
            if (ftoken_last == NULL || strcmp(ftoken_last->value,".")!=0){
            //VARIBLE SET
            printf("VARIBLE DEF: %s\n",ftoken->value);
            //check if it is already in list
            bool check = false;
            for (int i = 0; i<local_var_stack->top; i++){
                if (strcmp(((token_t*)local_var_stack->data[i])->value,ftoken->value) == 0){
                    check = true; break;
                }
            }
            if (check == false)
            stack_push(local_var_stack,ftoken);
            }
        }
        
        // if function contains return (check f->has_return so no repeats)
        if (f->has_return == false && strcmp(ftoken_next->value,"=")==0 && strcmp(ftoken->value,"return")==0){
            f->has_return = true;
        } 

        //if function contains returns and there is no return in end of function
        
        //! does not work correctly
        /*
        if (f->has_return == true && ftoken_next->type == KEYWORD && strcmp(ftoken_next->value,"end")==0 
        && strcmp(ftoken->value,"=")!=0 && strcmp(ftoken_last->value,"return")!=0) {
            printf("Error expect end of function '%s' to return value at line %d",f->name,ftoken_next->line_number);
            exit(2);
        }*/

        //keeps going till end of function
        if (ftoken->type == KEYWORD){ // is end
            if ((strcmp(ftoken->value,"end")  == 0)){
                end_count++;
            }else if (ftoken->value[0]  == ':' || strcmp(ftoken->value,"then")  == 0 || strcmp(ftoken->value,"do")  == 0){
                start_count++;

            }
        }
        ftoken_last = ftoken;
    }
    //there is no way the token is not end, also make it into a different token
    token_t* ftoken = ((token_t*)tokenizer->tokens->data[func_current_token-1]);
    free(ftoken->value);
    if (!f->has_return){
        ftoken->value = cpystring("function_end");
    }else{
        ftoken->value = cpystring("function_end_ret");
    }

    printf("total local var count: %d\n",local_var_stack->top);
    int local_var_count = 0;
    for (int i = 0; i<local_var_stack->top; i++){
        //[var,x,0]

        //make sure a argument does not get added
        bool check = false;
        for (int j = 0; j<f->argument_list->top; j++){
            
            if (strcmp(((token_t*)local_var_stack->data[i])->value,f->argument_list->data[j])==0){
                check = true;
                break;
            }
        }
        if (check == false){
            sprintf(str,"!%s,%d",((token_t*)local_var_stack->data[i])->value,local_var_count++);
            stack_push(assembler->rpn_tokens,cpystring(str));
        }
    }
    f->local_variables_count = local_var_count;
}

int find_matching_end(stack_t* tokens, int start_index) {
    int depth = 0;

    for (int i = start_index; i < tokens->top; i++) {
        token_t* token = (token_t*)tokens->data[i];
            printf("token found %s\n",token->value);

        if (token->type == KEYWORD) {
            if (strcmp(token->value, "do") == 0 ||
                strcmp(token->value, "then") == 0) {
                depth++;
            } else if (strcmp(token->value, "end") == 0) {
                depth--;
                if (depth == 0) {
                    return i; // Found matching 'end'
                }
            }
        }
    }

    fprintf(stderr, "Error: unmatched block opener at line %d\n",
            ((token_t*)tokens->data[start_index])->line_number);
    exit(2);
}

assembler_t* assemble(tokenizer_t* tokenizer){

    assembler_t* assembler = (assembler_t*)calloc(sizeof(assembler_t),1);

    init_hash_table(&assembler->global_variables,20);

    init_hash_table(&assembler->functions,20);

    init_hash_table(&assembler->labels,20);
    init_hash_table(&assembler->mathematical_conversion,20);
    init_hash_table(&assembler->string_map,10);
    init_hash_table(&assembler->extern_function_map,10);

    hash_table_insert(&assembler->mathematical_conversion,"==",(void*)cpystring("eql"));
    hash_table_insert(&assembler->mathematical_conversion,"<",(void*)cpystring("lss"));
    hash_table_insert(&assembler->mathematical_conversion,">",(void*)cpystring("grt"));
    hash_table_insert(&assembler->mathematical_conversion,"!=",(void*)cpystring("nql"));
    hash_table_insert(&assembler->mathematical_conversion,"+",(void*)cpystring("add"));
    hash_table_insert(&assembler->mathematical_conversion,"-",(void*)cpystring("sub"));
    hash_table_insert(&assembler->mathematical_conversion,"*",(void*)cpystring("mul"));
    hash_table_insert(&assembler->mathematical_conversion,"/",(void*)cpystring("div"));

    hash_table_insert(&assembler->mathematical_conversion,"%",(void*)cpystring("mod"));

    hash_table_insert(&assembler->mathematical_conversion,"!",(void*)cpystring("set"));
    hash_table_insert(&assembler->mathematical_conversion,"|",(void*)cpystring("table_set"));
    hash_table_insert(&assembler->mathematical_conversion,"?",(void*)cpystring("get"));
    hash_table_insert(&assembler->mathematical_conversion,"@",(void*)cpystring("table_access"));
    //x hello @ 4 @ ?

    assembler->memory_to_clear = stack_new(20);

    assembler->rpn_tokens = stack_new(20);
    assembler->string_list = stack_new(20);
    assembler->extern_function_list = stack_new(20);

    assembler->tokenizer = tokenizer;
    int statement_start = 0;
    char str[128];
    for (assembler->current_token = 0; assembler->current_token<assembler->tokenizer->tokens->top; assembler->current_token++){
        
        token_t* token = ((token_t*)tokenizer->tokens->data[assembler->current_token]);
        assert(token != NULL);
            printf("Current token : %s %d\n",token->value,token->line_number);

        if (assembler_is_function_definition(token)){
            
            function_t* f = assembler_handle_function_definition(assembler);
            hash_table_insert(&assembler->functions,f->name,(void*)f);


            /*Set arg count label*/
            int length = snprintf(NULL, 0, "&%s_arg_count", f->name);
            char* str = malloc(length + 1);
            snprintf(str, length + 1, "&%s_arg_count", f->name);

            int* p = malloc(sizeof(int));
            *p = f->argument_count;

            hash_table_insert(&assembler->labels,str,p);
            

            /*create label*/
            sprintf(str,"$%s",f->name);
            stack_push(assembler->rpn_tokens,cpystring(str));
            
            print_table(&f->arguments);
            //quick test stuff should be own function
            handle_function_arguments(assembler,f);
            handle_function_variables(assembler,tokenizer,f);
            printf("token : %d %s \n",token->line_number,token->value);
            //allocate space for local variables
            if (f->local_variables_count > 0 ){
                stack_push(assembler->rpn_tokens,cpystring("alloc"));

                sprintf(str,"%d",f->local_variables_count);
                stack_push(assembler->rpn_tokens,cpystring(str));
            }
            continue;
        }
        if (assembler_is_newline(token)){
            continue;
        }

        
        
        if (token->type == KEYWORD){
            if (strcmp(token->value,":")==0){
                stack_push(assembler->rpn_tokens,cpystring("then"));
                continue;
            }
            if (strcmp(token->value,"function_end")==0){
                stack_push(assembler->rpn_tokens,cpystring("ret"));
                continue;
            }

            if (strcmp(token->value,"extern")==0){
                token = ((token_t*)tokenizer->tokens->data[++assembler->current_token]);
                assert(token->type != END_OF_FILE);
                if (token->type != IDENTIFIER) {printf("Error expect function name after extern at line %d, got '%s' \n",token->line_number,token->value); exit(2);}
                function_t* f = function_create();
                f->name = cpystring(token->value);
                printf("Extern function name %s",f->name);
                f->is_extern = true;
                hash_table_insert(&assembler->functions,f->name,(void*)f);
                
                continue;
            }

            if (strcmp(token->value,"while") == 0){
                stack_push(assembler->rpn_tokens,cpystring(token->value));
                int while_index = find_matching_end(tokenizer->tokens,assembler->current_token);
                token = stack_check(tokenizer->tokens,while_index);
                assert(token != NULL);
                free(token->value);
                token->value = cpystring("while_end");

                continue;
            }
            //Error handling already makes sure that the function has a return
            if (strcmp(token->value,"function_end_ret")==0){
                printf("TESTY TESTY : %s \n",stack_peak(assembler->rpn_tokens));

                //            printf("Error expect end of function '%s' to return value at line %d",f->name,ftoken_next->line_number);

                continue;
            }            

            stack_push(assembler->rpn_tokens,cpystring(token->value));
            continue;
        }

        if (assembler_is_expression(token)){
            expression_t* exp = assembler_handle_expression(assembler,assembler->current_token);
            //printf("hjadwjiadwjipadwjwdipawad");
            if (exp->r_tokens!=NULL){
                /*nothing special needed here*/
                tokens_to_bytecode(assembler,exp->r_tokens);

            }

            if (exp->l_tokens!=NULL){
                /*check if it has a valid lvalue fx " x[2+2] = "*/
                /*unvalid lvalue would be something like x+1 =*/
                /*Only way to do this seems like to "precalculate" it to verify it makes sense */

                /*last instruction in the left side to set the value*/
                tokens_to_bytecode(assembler,exp->l_tokens);

            }
            free(exp);
            //printf("token : %s\n",((token_t*)tokenizer->tokens->data[assembler->current_token])->value);
            continue;
        }
        


      //  if (assembler_is_assignment(token)){
            //assembler_handle_assignment(assembler,statement_start);
       //     continue;
       // }

    }

    printf("\nbytecode:\n");
    for (int i = 0; i<assembler->rpn_tokens->top; i++){
        printf("%s\n",assembler->rpn_tokens->data[i]);
    }
    return assembler;
}

bool tokens_to_rpn(){

}
