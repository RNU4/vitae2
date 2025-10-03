#include "structs.h"
#include "hashtable.h"
#include <stdint.h>
#include <stdio.h>



table_t* table_create(vm_t*,int,int);
void table_free(table_t*);

vm_t* vm_create(){
    vm_t* vm = calloc(sizeof(vm_t),1);
    vm->global_table = table_create(vm,100,100);
    vm->stack_size = 256;
    vm->dstack = (uint64_t*)calloc(vm->stack_size,sizeof(uint64_t));
    vm->stack = calloc(vm->stack_size,sizeof(element_t));
    init_hash_table(&vm->extern_function_map,10);
    vm->extern_function_list = stack_new(10);
    vm->gb_collector0 = NULL;
    vm->gb_collector1 = NULL;
    vm->object_pool = NULL;
    //vm->stack = stack_new(100);
    return vm;
}



void free_gbobj(gc_object_t *gbobj){
    switch(gbobj->type){
        case gbtype_function: 
        break;
        case gbtype_table: 
            DEBUG_PRINT("free table\n");
            table_free((table_t*)gbobj);
            break;
        case gbtype_string:
            DEBUG_PRINT("free string '%s'\n",((string_t*)gbobj)->string);

            free_string((string_t*)gbobj);
             break;
        case gbtype_element:
            DEBUG_PRINT("free element ");
            #ifdef DEBUG
            element_print(*(element_t*)gbobj);
            #endif
            free((element_t*)gbobj);
        break;

        default:
            printf("ERROR TRIED TO FREE UNKOWN TYPE %d\n",gbobj->type); 
            exit(2);
        break;
    }
}

int sweep(vm_t* vm, gc_object_t **gb_list) {
    int objects_freed = 0;
    gc_object_t* next = *gb_list;
    gc_object_t* last = NULL;

    while (next != NULL) {
        if (!next->marked) {
            gc_object_t* unreached = next;
            next = next->next;

            if (last != NULL) {
                last->next = next;
            } else {
                // update head
                *gb_list = next;
            }

            free_gbobj(unreached);
            objects_freed++;

        } else {
            next->marked = false; // unmark again
            last = next;
            next = next->next;
        }
    }
    return objects_freed;
}


int sweep_count(vm_t* vm,gc_object_t *gb_list){
    int objects_count = 0;
    gc_object_t* next = gb_list;
    gc_object_t* last = NULL;
    DEBUG_PRINT("\nsweep list (");
    while (next != NULL){

        objects_count++;
        /*
        if (next->type == gbtype_string){
            printf("%s,",((string_t*)next)->string);

        }else 
        */
        DEBUG_PRINT("%d,",next->type);
        last = next;
        next = next->next;
    }
    DEBUG_PRINT(")\n");

    return objects_count;
}



void mark_table(table_t* table){
    // Hash part
    for (int i=0; i<table->hashtable->size; i++){
        if (table->hashtable->hash_table[i]->name[0] != 0){
            hash_element_t* tmp = table->hashtable->hash_table[i];
            while(tmp->next != NULL){ //? should be end of list
                element_t* obj = (element_t*)tmp->value;
                obj->gbobj.marked = true;
                if (obj->type == type_table && obj->value.table->gbobj.marked == false){ // check if it is already marked to stop niche cases of infinite loops
                    obj->value.table->gbobj.marked = true; /*Mark the found table*/
                    mark_table(obj->value.table);
                }
                tmp = tmp->next;
            }
        }
    }

    // Stack/array part
    for (int i = 0; i<table->stack->top; i++){
        if (table->stack->data[i] != NULL){
            element_t* obj = (element_t*)table->stack->data[i];
            obj->gbobj.marked = true;
            if (obj->type == type_table && obj->value.table->gbobj.marked == false){
                obj->value.table->gbobj.marked = true;                
                mark_table(obj->value.table);
            }
        }
    }

}


void mark_vm_stack(vm_t* vm){
    //printf("Marking %d elements\n",vm->sp - vm->stack);
    for (element_t* obj = vm->stack; obj < vm->sp; obj++){
        //printf("Marking\n");
        //element_print(*obj);
        if (obj->type == type_string){
            obj->value.string->gbobj.marked = true;
            //printf("##########################MARKED STRING##########################\n");
        }

        if (obj->type == type_table){ // check if it is already marked to stop infinite loops
            //printf("##########################MARKED TABLE##########################\n");
            obj->gbobj.marked = true;
            obj->value.table->gbobj.marked = true;
            mark_table(obj->value.table);
        }
    }
    //printf("Done marking\n");
}


element_t* copy_non_refrence_object(vm_t* vm,element_t* p){

    element_t *copy = malloc(sizeof(element_t));
    if (!copy) {
        printf("Malloc error copying");
        return false; // Memory allocation failure
    }

    // Copy the element's type
    copy->type = p->type;
    copy->gbobj.marked = false;
    copy->gbobj.type = gbtype_element;
    copy->gbobj.next = vm->gb_collector0;
    vm->gb_collector0 = &copy->gbobj;
    // Handle the copying based on the type
    if (p->type == type_string) {
        if (p->value.string) { // str != null
            copy->value.string = copy_string(p->value.string,0,p->value.string->length);
        } 
    }else{
        copy->value.inumber = p->value.inumber; // 64bit value
    }
    return copy;
}


/*
x[9999] = 4 // hashmap
x[0] = 3
x[1] = 4
...
...
x[9998] = 5
x[9999] // hashmap -> stack
*/


element_t* element_string(vm_t* vm,char* str){
    element_t* e = malloc(sizeof(element_t));
    e->type = type_string;
    e->gbobj.marked = false;
    e->gbobj.type = gbtype_string;

    e->value.string = c_string(str);

    //put it in linked list
    e->gbobj.next = vm->gb_collector0;
    vm->gb_collector0 = &e->gbobj;

    return e;
}

element_t* element_int(vm_t* vm,int64_t num){
    element_t* e = malloc(sizeof(element_t));
    e->type = type_inumber;
    e->gbobj.marked = false;
    e->gbobj.type = gbtype_element;
    e->value.inumber = num;

    //put it in linked list
    e->gbobj.next = vm->gb_collector0;
    vm->gb_collector0 = &e->gbobj;

    return e;
}

table_t* table_create(vm_t* vm,int hashmap_size,int stack_size){
    table_t* t = calloc(sizeof(table_t),1);
    t->hashtable = calloc(sizeof(hashtable_t),1);
    init_hash_table(t->hashtable,hashmap_size);
    t->stack = stack_new(stack_size);
    t->list_backlog = NULL;
    t->gbobj.marked = false;
    t->gbobj.type = gbtype_table;
    t->gbobj.next = vm->gb_collector0;
    vm->gb_collector0 = &t->gbobj;
    return t;
}

void table_free(table_t* table){
    //hash
    for (int i=0; i<table->hashtable->size; i++){
        if (table->hashtable->hash_table[i]->name[0] != 0){
            hash_element_t* tmp = table->hashtable->hash_table[i];
            while(tmp->next != NULL){ //? should be end of list
                hash_element_t* tmp2 = tmp->next;
                free(tmp->name); // free string
                free(tmp);
                tmp = tmp2;
            }
        }
    }
    free(table->hashtable->hash_table);
    free(table->hashtable);

    //stack
    stack_free(table->stack);

    //backlog
    node_t* next = table->list_backlog;
    while (next != NULL){
        //remove node
        node_t* tmp = next->next;
        free(next);
        next = tmp;
    }
    free(table);
}

void table_check_backlog(table_t* table){
    

    node_t* next = table->list_backlog;
    while (next != NULL){
        if ((size_t)next->value < table->stack->size){ // remove from hash put into array
            #if (_WIN32)
                char snum[8]; // should be the max length of a 32bit num in string form
                itoa((size_t)next->value, snum, 10);
            #else
                char snum[19]; // should be the max length of a 64bit num in string form
                ltoa((size_t)next->value, snum, 10); //64bit only compile
            #endif

            element_t* hobj = hash_table_delete(table->hashtable,snum);
            assert(hobj != NULL); /*idk something went wrong if its null*/

            table->stack->data[(size_t)next->value] = hobj;
            
            //remove node
            node_t* tmp = next->next;
            remove_node(&table->list_backlog,next);
            next = tmp;
        }else{
            next = next->next;

        }
    }
}

//Returns null if it does not exist
element_t* table_get(table_t* table,element_t* id){
    if (id->type == type_inumber){ // check hashmap & stack depending on the size

        if (id->value.inumber < table->stack->size){ // value is inside stack
            if (id->value.inumber>=0){
                return (element_t*)table->stack->data[id->value.inumber];
            }else{ // minus numbers
                char snum[9]; // should be the max size of a 32bit num
                itoa(id->value.inumber, snum, 10);
                return (element_t*)hash_table_get(table->hashtable,snum);
            }

        }else{ // value is inside hashmap
            char snum[8]; // should be the max size of a 32bit num
            itoa(id->value.inumber, snum, 10);
            return (element_t*)hash_table_get(table->hashtable,snum);
        }

    }else if (id->type == type_string){ //check hashmap
        return (element_t*)hash_table_get(table->hashtable,id->value.string->string);
    } else { // also check hashmap but first convert to a string (bool,float)
        //TODO make later
        printf("TABLE ERROR : \n");
        element_print(*id);
        exit(2);
    }
}

int table_push(vm_t* vm,table_t* table,element_t* p){
    element_t* obj;
    if (p->type == type_function || p->type == type_table){ // use refrence
        obj = p;
    }else{ // copy it
        obj = copy_non_refrence_object(vm,p);
    }

    if (table->stack->top+2 >= table->stack->size){
        grow_stack(table->stack,32);
    }

    if (table->stack->data[table->stack->top-1] == NULL){
        table->stack->data[table->stack->top++]=obj;
        return table->stack->top;
    }else{
        while(table->stack->data[table->stack->top-1] != NULL) table->stack->top++;
        
        table->stack->data[table->stack->top-1]=obj; // -1 because top is 1 ahead
    }

}


bool table_insert(vm_t* vm,table_t* table,element_t* id ,element_t* p){
    element_t* obj;
   // if ((p->type == type_function || p->type == type_table || p->type == type_pointer)){ // use refrence
   //     obj = p;
   // }else{ // copy it
    obj = copy_non_refrence_object(vm,p);
   // }

    if (id->type == type_inumber){ // put it in the array
        if(id->value.inumber < 0){
            
            char snum[9];
            itoa(id->value.inumber, snum, 10);
            hash_table_insert(table->hashtable,snum,obj);

        } else if (id->value.inumber < table->stack->size){ // fits inside
            table->stack->data[id->value.inumber] = obj;

        }else if (id->value.inumber < table->stack->size+32){ // grow it and check conditions again
            grow_stack(table->stack,32); //? should check for error

            //Insert value after table has grown
            table->stack->data[id->value.inumber] = obj;
            
            //check backlog
            table_check_backlog(table);

        }else{ // save to backlog
            insert_at_head(&table->list_backlog,create_new_node((void*)((size_t)id->value.inumber))); // save non fitting index

            char snum[8]; // should be the max size of a 32bit num
            itoa(id->value.inumber, snum, 10);
            hash_table_insert(table->hashtable,snum,obj);

        }

    }else if(id->type == type_string){
        char* name = id->value.string->string;

        if (p->type == type_function || p->type == type_table){ // use refrence
            hash_table_insert(table->hashtable,name,obj);
        }else{
            // Insert the copied element into the hash table
            hash_table_insert(table->hashtable, name, obj);
        }
    }
}
