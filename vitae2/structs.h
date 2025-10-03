#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "gb.h"
#include "hashtable.h"
#include "stack.h"
#include "strings.h"
#include "linked_list.h"

#ifndef STRUCTS_H_   /* Include guard */
#define STRUCTS_H_

typedef enum types {type_nil,type_string,type_inumber,type_number,type_pointer,type_boolean,type_function,type_table}types_t;

//#define DEBUG

#ifdef DEBUG
    #define DEBUG_PRINT(fmt, ...) \
        do { fprintf(stderr,fmt, ##__VA_ARGS__); } while (0)
#else
    #define DEBUG_PRINT(fmt, ...) \
        do { } while (0)
#endif

typedef struct table
{
    common_header; //for garbage collection
    hashtable_t *hashtable; // hashtable
    stack_t *stack; // stack
    node_t *list_backlog; //linked list
}table_t;

/*
stack backlog is if you do x[9999] = 44 while the size of the array part is only 10 
it will be put inside hashtable and inside backlog
when the array part gets resized it will check backlog and see if values fit inside 
*/


typedef struct element {
    common_header;
    union { // -- 64 bit
        double number; // float
        int64_t inumber; // int 
        string_t *string;
        table_t *table; 
    } value;
    uint8_t type; // Type of the element (number, inumber, string, table)
} element_t;

typedef struct vm {
    //stack_t* stack;
    table_t* global_table;
    int stack_size;
    uint64_t* dstack;
    element_t* stack;
    element_t* sp;
    element_t* dp;

    //higher gen means less checks if they are still in use
    gc_object_t* gb_collector0; // list of all gen0 objects
    gc_object_t* gb_collector1; // list of all gen1 objects

    hashtable_t extern_function_map;
    stack_t* extern_function_list;

    node_t* object_pool; // objects not in use who can be reused since alloc & dealloc is slow, so its best to avoid it

}vm_t;

typedef size_t (*external_func)(vm_t* instance, int argc);


#endif