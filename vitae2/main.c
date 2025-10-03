#include <time.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "structs.h"
#include "lint.h"


#define LABEL_INIT(op) [op] = &&op##_label

#define eint(x) (&((element_t){.type=type_inumber,.value = x}))

int main(){

    vm_t* vm = vm_create();
    table_t* table = table_create(vm,10,10); // 10 element big stack & hashtable

    table_insert(table,element_int(vm,1294967295),element_int(vm,4)); // gets put in hash
    table_insert(table,element_int(vm,3),element_int(vm,10));                  // gets put in stack
    table_insert(table,element_int(vm,-10),element_int(vm,10));                  // gets put in stack
    table_insert(table,element_string(vm,"hello"),element_int(vm,22));         // gets put in hash
    table_insert(table,element_string(vm,"world"),element_string(vm,"hello")); // gets put in hash



    printf("\n\n");

    table_push(table,element_int(vm,5));
    table_push(table,element_int(vm,6));
    table_push(table,element_int(vm,7));
    table_push(table,element_int(vm,8));
    table_push(table,element_int(vm,9));



    element_t* index = element_int(vm,0);
    for (int i = 0; i<table->stack->size; i++){
        index->value.inumber = i;

        element_t* obj = table_get(table,index);
        if (obj == NULL){
            printf("[%d] = NULL\n",i);
        }else{
            printf("[%d] = %lld\n",i,obj->value.inumber);
        }
    }
    
    print_table(table->hashtable);
    gc_object_t* next = vm->gb_collector0;
    while (next != NULL){
        printf("gb type %hhd\n",next->type);
        next = next->next;
    }

    
    return 0;
}

