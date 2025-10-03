#ifndef _STACK_H_
#define _STACK_H_
#include "./lib/stack.h"
#include "./hashtable.h"
// Define token types
#define pointer_size 4


enum token_type {
    KEYWORD = (uint8_t)0,
    IDENTIFIER, // +-*/%^#=<>;,.(){}[]
    NUMBER,
    FUNCTION,
    STRING,
    OPERATOR,
    COMMENT,
    NEWLINE,
    END_OF_FILE,

};


typedef struct {
    enum token_type type;
    char *value;
    int line_number; // what line the token is on for error reporting
} token_t;

typedef struct {
    stack_t* tokens;
    char* ptr; // string pointer
    char* start; // string pointer
    int current_line_number; // what line the token is on for error reporting
} tokenizer_t;
 

typedef struct {
    bool lvalue;
    bool rvalue;
    stack_t* l_tokens;
    stack_t* r_tokens;
    /*if it is first time assigment like x = 3 or sum*/
    //variable_t lvalue; // lvalue = rvalue

} expression_t;



typedef struct {
    char* name;
    bool is_extern; /*extern c function, if true the rest of the struct dont really matter*/
    bool has_return;
    
    hashtable_t arguments; // variable_t* 
    int argument_count;
    stack_t* argument_list; // variable_t* 
    
    stack_t* tokens; // tokens inside function;
    hashtable_t local_variables;
    int local_variables_count;
    bool referenced; // if never used dont include it in the code unless the INCLUDE_ALL_FUNCTIONS flag is set
} function_t;

function_t* function_create(){
    function_t* f = (function_t*)calloc(sizeof(function_t),1);
    f->has_return = false;
    f->argument_list = stack_new(5);
    init_hash_table(&f->arguments,10);

    init_hash_table(&f->local_variables,10);
    return f;
}


enum optimization_level {
    O0 = (uint8_t)0, /*default does nothing*/
    O1,
    OSIZE,
    OFAST, // ofast is designed to possibly break code but make it faster
};


enum compiler_flags {
    INLINE_FUNCTIONS = (uint8_t)0, /*inlines functions instead of calling them*/
    COMBINE_UINT_ADD, /*addu8 addu8 = addu16, this will break some code*/
    COMBINE_PUSH, /*push8 push8 = push16*/
    USE_SUPER_INSTRUCTIONS, /*combine instructions into a predefined larger instruction*/
    USE_CONST_INSTRUCTIONS, /*will use const versions of instructions when possible*/
    INCLUDE_ALL_FUNCTIONS, /*include all functions no matter if refrenced or not*/
    REMOVE_UNREFRENCED_VARIABLES, /*will remove variables who are not refrenced*/
    REMOVE_UNREFRENCED_DATA, /*will remove for example strings who are not refrenced*/
    PRECALCULATE_VARIABLES, /*will attempt to calculate stuff like x=2+5 into x=7*/

    /*this will do stuff like storing a 8 bit number and then make it into a 32 bit number 
    when using instructions requiring 32bit argument. (slows program down)*/
    MINIMIZE_DATA, 
    /*this will attempt to compress the bytecode, the trade off is the interpreter needs to unpack it.
    if the resulting binary is bigger it will be discarded for the original binary*/
    COMPRESS_DATA,

    //? implementation at later point
    AUTOTYPE, /*will attempt to auto asign a type.*/
    AUTOCONVERT, /*will attempt to auto convert a type*/

    /*compiler flags that change how code is written*/
    USE_NEWLINE_INSTEAD_OF_COLON,
    };

typedef struct {
    tokenizer_t* tokenizer;
    hashtable_t global_variables;
    stack_t* rpn_tokens;

    stack_t* memory_to_clear;


    hashtable_t string_map;
    stack_t* string_list;
    
    hashtable_t extern_function_map;
    stack_t* extern_function_list;

    hashtable_t labels;
    hashtable_t local_variables;


    hashtable_t functions;
    hashtable_t mathematical_conversion;

    enum optimization_level optimization_level;
    int current_token;
    uint32_t flags;
    int64_t* bytecode;

} assembler_t;


#endif