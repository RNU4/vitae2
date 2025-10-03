#include "tokenizer_helper.h"
#include "structs.h"
#include "tokenizer_helper.h"
#include <strings.h>
#define MAX_DEPTH 10

char *unescape_string(char *src) {
    size_t len = strlen(src);
    char *dst = malloc(len + 1); // worst case: same length
    if (!dst) return NULL;

    char *out = dst;

    for (size_t i = 0; i < len; i++) {
        if (src[i] == '\\') {
            i++; // look at next character
            if (i >= len) break;

            switch (src[i]) {
                case 'n': *out++ = '\n'; break;
                case 't': *out++ = '\t'; break;
                case 'r': *out++ = '\r'; break;
                case '\\': *out++ = '\\'; break;
                case '"': *out++ = '"'; break;
                case '\'': *out++ = '\''; break;
                case '0': *out++ = '\0'; break; // NUL byte
                default:
                    // unknown escape → keep as-is
                    *out++ = '\\';
                    *out++ = src[i];
                    break;
            }
        } else {
            *out++ = src[i];
        }
    }

    *out = '\0';
    return dst;
}

typedef struct {
    int depth;          // nesting depth
    int index;          // unique number at this depth
} if_frame_t;

void create_then(stack_t* stack, stack_t* depth_stack, int depth_count[MAX_DEPTH]){
    int bdepth = depth_stack->top;         // current depth
    depth_count[bdepth]++;                 // increment instance counter

    if_frame_t* frame = malloc(sizeof(if_frame_t));
    frame->depth = bdepth;
    frame->index = depth_count[bdepth];    // store unique index
    stack_push(depth_stack, frame);

    char label[64];
    sprintf(label, "if_%d_%d", frame->depth, frame->index);

    stack_push(stack, cpystring("jmpf"));
    stack_push(stack, cpystring(label));
}

void create_else(stack_t* stack, stack_t* depth_stack){
    if_frame_t* frame = (if_frame_t*)stack_peak(depth_stack); // peek only

    char label[64];
    sprintf(label, "if_%d_%d", frame->depth, frame->index);

    stack_push(stack, cpystring("jmp"));
    stack_push(stack, cpystring(label));
}

void create_end(stack_t* stack, stack_t* depth_stack){
    if_frame_t* frame = (if_frame_t*)stack_pop(depth_stack);

    char label[64];
    sprintf(label, "$if_%d_%d", frame->depth, frame->index);

    stack_push(stack, cpystring(label));
    free(frame);
}


void create_while(stack_t* stack, stack_t* depth_stack, int depth_count[MAX_DEPTH]) {
    // Just create the "top-of-loop" label
    char label[64];
    sprintf(label, "$while_%d_%d", depth_stack->top, depth_count[depth_stack->top]+1);
    stack_push(stack, cpystring(label));  // emit label
}

void create_while_end(stack_t* stack, stack_t* depth_stack){
    if_frame_t* frame = (if_frame_t*)stack_pop(depth_stack);
    char label[64];

    sprintf(label,"while_%d_%d",frame->depth, frame->index); //! could crash do check for [-1]

    stack_push(stack,cpystring("jmp"));
    stack_push(stack,cpystring(label)); // label
    
    sprintf(label, "$if_%d_%d", frame->depth, frame->index);

    stack_push(stack, cpystring(label));
    free(frame);
}


stack_t* final_pass(assembler_t* assembler){
    printf("Final pass:\n");
    stack_t* stack = stack_new(10);
    stack_t* depth_stack = stack_new(16);
    int depth_count[10] = {0};
    int bdepth = 0;
    int stack_offset = assembler->string_list->top+assembler->extern_function_list->top; //const strings are added but should not be counted for pos 
    printf("stack offset %d\n",stack_offset);
    init_hash_table(&assembler->local_variables,10);
    for (int i = 0; i < assembler->string_list->top; i++){
        char* unescape_str = unescape_string(assembler->string_list->data[i]);
        int length = snprintf(NULL, 0, "^s%d=\"%s\"", i,unescape_str);
        char* str = malloc(length + 1);
        snprintf(str, length + 1, "^s%d=\"%s\"", i,unescape_str);
        free(unescape_str);
        stack_push(stack,str);
    }
    for (int i = 0; i < assembler->extern_function_list->top; i++){
        int length = snprintf(NULL, 0, "^f%d=\"%s\"", i,assembler->extern_function_list->data[i]);
        char* str = malloc(length + 1);
        snprintf(str, length + 1, "^f%d=\"%s\"", i,assembler->extern_function_list->data[i]);
        stack_push(stack,str);
    }
    /*for return*/
    //stack_push(stack,cpystring("alloc"));
    //stack_push(stack,cpystring("1")); 
    /*goto main function*/
    if (hash_table_check(&assembler->labels,"main")){
        printf("Error expected function 'main'\n");
        exit(2);
    }else{
        stack_push(stack,cpystring("call")); 
        stack_push(stack,cpystring("main")); 
        stack_push(stack,cpystring("0")); 

        stack_push(stack,cpystring("halt")); 

    }

    for (int i = 0; i < assembler->rpn_tokens->top; i++){
        char* token = (char*)assembler->rpn_tokens->data[i];
        assert(token != NULL);

        if (token[0] == '!'){ // var thing

            //get name
            char* var_name = ++token;
            int length = strlen(var_name);
            int j;
            for (j = 0; j < length; j++){
                if (var_name[j]==','){
                    var_name = memcpy((char*)calloc(j,1),var_name,j++);
                    printf("Var name : %s \n",var_name);
                    break;
                }
            }
            //get value
            char* var_value = memcpy((char*)calloc(length-j,1),token+j,length-j);
            printf("Var value : %s \n",var_value);

            //check if already in table and replace if it is
            if (hash_table_check(&assembler->local_variables,var_name)){
                void* pointer = hash_table_get(&assembler->local_variables,var_name);
                free(pointer);
                hash_table_insert(&assembler->local_variables,var_name,var_value);
            }else{
                hash_table_insert(&assembler->local_variables,var_name,var_value);
            }

        }else if (token[0]=='#'){ // string standin
            if (token[1]=='s')
            stack_push(stack,cpystring("type_string"));
            stack_push(stack,token);

        }else if (hash_table_check(&assembler->local_variables,token)){
            char* pointer = hash_table_get(&assembler->local_variables,token);
            stack_push(stack,cpystring(pointer));

            //make return work
            if (strcmp(token,"return") == 0){
                if (i+1 <= assembler->rpn_tokens->top){
                    token = stack_check(assembler->rpn_tokens,++i); /*get next token*/
                    assert(token != NULL);
                    if (strcmp(token,"set") == 0){
                        stack_push(stack,token);
                        stack_push(stack,cpystring("retv"));
                    }else{
                        printf("Fatal internal error expected 'set' instruction after 'return'\n");
                        exit(2);
                    }
                }
            }
        }else if (strcmp(token,"then") == 0){
            create_then(stack, depth_stack, depth_count);
        }else if (strcmp(token,"else") == 0){
            create_else(stack, depth_stack);


        }else if (strcmp(token,"end") == 0){
            create_end(stack, depth_stack);
        }else if (strcmp(token,"while_end") == 0){
            create_while_end(stack,depth_stack);
            
           // assert(bdepth-1>=0);


        }else if (strcmp(token,"if") == 0){

        }else if (strcmp(token,"while") == 0){
            
            create_while(stack,depth_stack,depth_count);
            
        }else{
        // Peephole optimization: detect dpushc <num> get
        char* last1 = stack_check(stack, stack->top-1);
        char* last2 = stack_check(stack, stack->top-2); // token before last

        if (last1 && last2 && strcmp(token, "get") == 0 && strcmp(last2, "dpushc") == 0) {
            // Turn into dloadc <num>
            free(stack->data[stack->top-2]);
            stack->data[stack->top-2] = strdup("dloadc");
            
            // Drop the "get" token by not pushing it
        }else if (last1 && last2 && strcmp(token, "set") == 0 && strcmp(last2, "dpushc") == 0){
            free(stack->data[stack->top-2]);
            stack->data[stack->top-2] = strdup("dstorec");
        }else{
            // Otherwise, just push normally
            stack_push(stack, token);   
        }
   
    }

    }

    stack_t* tmp_stack = stack_new(stack->top);
    for (int i = 0; i<stack->top; i++){
        char* token = (char*)stack->data[i];
        if  (token[0]=='$'){
            int* p = malloc(sizeof(int));
            *p = tmp_stack->top-stack_offset;
            hash_table_insert(&assembler->labels,++token ,(void*)p);
        }else{
            stack_push(tmp_stack,token);
        }
    }
    stack_free(stack);  
    stack = tmp_stack;
    tmp_stack = stack_new(stack->top);

    for (int i = 0; i<stack->top; i++){
        char* token = (char*)stack->data[i];
        if (hash_table_check(&assembler->labels,token)){
            
            int* pointer = hash_table_get(&assembler->labels,token);
            int length = snprintf(NULL, 0, "%d", *pointer);
            char* str = malloc(length + 1);
            snprintf(str, length + 1, "%d", *pointer);
            stack_push(tmp_stack,str);

        }else{
            stack_push(tmp_stack,token);           
        }
    }

    stack_free(stack);
    stack = tmp_stack;

    printf("FINAL PASS : \n");
    for (int i = 0; i < stack->top; i++){
        printf("token[%d]: %s\n",i,stack->data[i]);
    }
    return stack;
}