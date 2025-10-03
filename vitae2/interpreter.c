#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "structs.h"
#include "opcodes.h"
#include "hashtable.h"
#include "element_handler.h"
#include "file_handler.h"
#include "stack.h"
#include "lint.h"

static inline uint64_t float_to_uint64(double f) {
    union { double f; uint64_t u64; } pun;
    pun.f = f;
    return pun.u64;
}


//^str0="hello world",call,5,0,halt,alloc,1,pushc,#str0,dpushc,0,set,dpushc,0,get,dpushc,0,set,retv,

uint64_t* string_to_bytecode(vm_t* vm,hashtable_t* convert_to_bytecode,char* str,string_t** const_string_stack){
    char token[100];
    uint64_t* code;
    int code_size = 0;
    int code_pos = 0;
    int last_pos = 0;
    bool is_string = false;
    int read_start_point = 0;
    for(int i = 0; i < strlen(str); i++){

        //^s0="2",^f0="test"
        if (str[i]=='^'){
            int const_num = -1;
            int typ = str[++i];

            int st = ++i;
            
            while (str[i]!='=' && i < strlen(str)) i++;
            memcpy(token,str+st,i-st);
            token[i-st] = '\0';
            DEBUG_PRINT("string test : %s \n",token);
            const_num = atoi(token);
            i+=2; //skip ="
            st = i;
            //!does not ignore \"
            while (str[i]!='"' && i < strlen(str)) i++;
            memcpy(token,str+st,i-st);
            token[i-st] = '\0';

            DEBUG_PRINT("string test2 : %s \n",token);

            assert(const_num != -1);
            if (typ =='s')
                const_string_stack[const_num] = c_string(token);
            else{
                stack_push(vm->extern_function_list,c_string(token));
            }
            last_pos = i+1;
            read_start_point = i+2;
            continue;
        }
        
        if (is_string == false && (str[i] == ',')){
            code_size++;
        }

    }
    code = calloc(code_size+1,sizeof(uint64_t));
    DEBUG_PRINT("done with prestuff, code %d \n",code_size+1);
    last_pos = read_start_point;
    for(int i = read_start_point; i < strlen(str); i++){

        if (is_string == false && (str[i] == ',')){
            
            memcpy(token,str+last_pos,i-last_pos);
            token[i-last_pos] = '\0';

            last_pos = i+1;
            DEBUG_PRINT("token[%d] %s\n",code_pos,token);
            if (hash_table_check(convert_to_bytecode,token)){
                code[code_pos++] = (size_t)hash_table_get(convert_to_bytecode,token);
            }else if (strchr(token, '.') != NULL){ // float
                code[code_pos++] =float_to_uint64(atof(token));
            }else if (token[0] == '#'){ // string ,#f0, #s0
                //DEBUG_PRINT("STRING FOUND\n"); 
                int j = 0;
                int typ = token[++j];
                int st = ++j;
                
                while (token[j] != '\0') j++;
                memcpy(token,token+st,j-st);
                token[j-st] = '\0';
                
                //DEBUG_PRINT("string test3 : %s \n",token);

                //DEBUG_PRINT("TYPE '%c'\n",typ);
                int id = atoi(token);
                if (typ == 's'){
                    assert(const_string_stack[id] != NULL);
                    code[code_pos++] = (size_t)(const_string_stack[id]);
                }else{ // is extern function
                    DEBUG_PRINT("function with id: %d '%s'\n",id,((string_t*)vm->extern_function_list->data[id])->string);
                    assert(id < vm->extern_function_list->top);
                    void* f = hash_table_get(&vm->extern_function_map,((string_t*)vm->extern_function_list->data[id])->string);
                    assert(f!=NULL);
                    code[code_pos++] = (size_t)(f);
                }

            }else{ // int
                code[code_pos++] = atoi(token);
            }
        }

    }
    return code;
}

element_t vm_pop(vm_t* vm){
    return *(--vm->sp);
}
element_t* vm_pop_pointer(vm_t* vm){
    return (element_t*)(--vm->sp);
}

void vm_push(vm_t* vm, element_t val){
    vm->sp->type = val.type;
    vm->sp->value.inumber = val.value.inumber;
    vm->sp++;
}

element_t vm_get_arg(vm_t* vm,int argc, int argn){
    return *(vm->sp-argc+argn);
}
element_t* vm_get_arg_pointer(vm_t* vm,int argc, int argn){
    return (element_t*)(vm->sp-argc+argn);
}

void vm_pop_argc(vm_t* vm, int argc){
    vm->sp-=argc;
}
/*
element_t args[argc];
for (int i = 0; i < argc; i++){
    args[i] = vm_pop(vm);
}
*/
size_t test(vm_t* vm,int argc){
    
    assert(argc!=0);
    if (vm->stack > vm->sp - argc) {
        printf("Error stack underflow\n");
        exit(2);
    }
    
    int sum = 0;
    for (int i = 0; i < argc; i++){
        sum += vm_pop(vm).value.inumber;
    }
    vm_push(vm,(element_t){.type = type_inumber, .value.inumber = sum});

    return 0;
}

size_t vm_create_table(vm_t* vm,int argc){ //TODO: should rewrite to make it faster since it will prob be called a lot
    
    assert(argc % 2 == 0);
    table_t* table = table_create(vm,10,10);
    for (int i = 0; i < argc; i+=2){
        element_t* value = vm_pop_pointer(vm);
        element_t* id = vm_pop_pointer(vm);
        table_insert(vm,table,id,value);
    }
    vm_push(vm,(element_t){.type = type_table, .value.table = table});
    return 0;
}

size_t print(vm_t* vm,int argc){
    assert(argc!=0);
    if (vm->stack > vm->sp - argc) {
        printf("Error stack underflow\n");
        exit(2);
    }

    for (int i = 0; i < argc; i++){
        print_data(vm_get_arg(vm,argc,i));
    }
    vm_pop_argc(vm,argc);
    return 0;
}

size_t print_hash(vm_t* vm,int argc){
    assert(argc==1);
    if (vm->stack > vm->sp - argc) {
        printf("Error stack underflow\n");
        exit(2);
    }

    element_t e = (vm_get_arg(vm,argc,0));
    element_print(e);
    assert(e.type == type_table);
    print_table(e.value.table->hashtable);
    vm_pop_argc(vm,argc);
    return 0;
}


int main(){
    hashtable_t convert_to_bytecode;
    init_hash_table(&convert_to_bytecode,20);
    
    hash_table_insert(&convert_to_bytecode,"halt",(void*)halt);
    hash_table_insert(&convert_to_bytecode,"pop",(void*)pop);
    hash_table_insert(&convert_to_bytecode,"pushc",(void*)pushc);
    hash_table_insert(&convert_to_bytecode,"add",(void*)add);
    hash_table_insert(&convert_to_bytecode,"sub",(void*)sub);

    hash_table_insert(&convert_to_bytecode,"mul",(void*)mul);
    hash_table_insert(&convert_to_bytecode,"div",(void*)_div);
    hash_table_insert(&convert_to_bytecode,"mod",(void*)mod);


    hash_table_insert(&convert_to_bytecode,"get",(void*)get);
    hash_table_insert(&convert_to_bytecode,"set",(void*)set);
    hash_table_insert(&convert_to_bytecode,"dset",(void*)dset);
    hash_table_insert(&convert_to_bytecode,"dget",(void*)dget);
    hash_table_insert(&convert_to_bytecode,"dpushc",(void*)dpushc);
    hash_table_insert(&convert_to_bytecode,"dstorec",(void*)dstorec);
    hash_table_insert(&convert_to_bytecode,"dloadc",(void*)dloadc);
    hash_table_insert(&convert_to_bytecode,"eql",(void*)eql);
    hash_table_insert(&convert_to_bytecode,"nql",(void*)nql);
    hash_table_insert(&convert_to_bytecode,"grt",(void*)grt);
    hash_table_insert(&convert_to_bytecode,"lss",(void*)lss);
    hash_table_insert(&convert_to_bytecode,"jmp",(void*)jmp);
    hash_table_insert(&convert_to_bytecode,"jmpr",(void*)jmpr);
    hash_table_insert(&convert_to_bytecode,"jmpf",(void*)jmpf);
    hash_table_insert(&convert_to_bytecode,"jmpt",(void*)jmpt);
    hash_table_insert(&convert_to_bytecode,"alloc",(void*)alloc);
    hash_table_insert(&convert_to_bytecode,"concat",(void*)concat);
    hash_table_insert(&convert_to_bytecode,"call",(void*)call);
    hash_table_insert(&convert_to_bytecode,"call_extern",(void*)call_extern);

    hash_table_insert(&convert_to_bytecode,"ret",(void*)ret);
    hash_table_insert(&convert_to_bytecode,"retv",(void*)retv);
    hash_table_insert(&convert_to_bytecode,"table_access",(void*)table_access);
    hash_table_insert(&convert_to_bytecode,"table_alloc",(void*)table_alloc);
    hash_table_insert(&convert_to_bytecode,"table_set",(void*)table_set);

    hash_table_insert(&convert_to_bytecode,"type_inumber",(void*)type_inumber);
    hash_table_insert(&convert_to_bytecode,"type_number",(void*)type_number);
    hash_table_insert(&convert_to_bytecode,"type_string",(void*)type_string);


    vm_t* vm = vm_create();
    uint64_t* dstack = vm->dstack;
    element_t* stack = vm->stack;
    string_t** const_string_stack = calloc(128,sizeof(string_t**));

    vm->sp = stack;
    vm->dp = stack;

    element_t left;
    element_t right;
    element_t p;

    hash_table_insert(&vm->extern_function_map,"test",test);
    hash_table_insert(&vm->extern_function_map,"print",print);
    hash_table_insert(&vm->extern_function_map,"create_table",vm_create_table);
    hash_table_insert(&vm->extern_function_map,"print_hash",print_hash);


    uint64_t* code = string_to_bytecode(vm,&convert_to_bytecode,read_file("./test.rpn"),const_string_stack);

    uint64_t* opcode = code;
    while(*(opcode) != halt){

        
        switch (*(opcode))
        {

        case pushc:
   
            DEBUG_PRINT("pushc[%d] %lld %lld\n",vm->sp-stack,*(opcode+1),*(opcode+2));

            //printf("test %lld\n",&const_stack[0]);
            //element_print(const_stack[0]);
            //element_print(**(element_t**)(opcode+2));
            vm->sp->type = *(++opcode);
            vm->sp->value.inumber = *(++opcode);
            vm->sp++;
            opcode++;
            //exit(2);
        break;

        case add:
            DEBUG_PRINT("add\n");
            right = *(--vm->sp);
            left = *(--vm->sp);
            #ifdef DEBUG
            element_print(right);
            element_print(left);
            #endif

            *vm->sp++ = element_add(vm,left,right);
            opcode++; // next instruction
            #ifdef DEBUG
            printf("Add result : ");
            element_print(*(vm->sp-1));
            #endif
        break;
        case mul:
            DEBUG_PRINT("mul\n");
            right = *(--vm->sp);
            left = *(--vm->sp);

            *vm->sp++ = element_mul(vm,left,right);
            opcode++; // next instruction
            #ifdef DEBUG
            printf("Mul result : ");
            element_print(*(vm->sp-1));
            #endif
        break;
        case sub:
            DEBUG_PRINT("sub\n");
            right = *(--vm->sp);
            left = *(--vm->sp);

            *vm->sp++ = element_sub(vm,left,right);
            opcode++; // next instruction
            #ifdef DEBUG
            printf("Sub result : ");
            element_print(*(vm->sp-1));
            #endif
        break;
        case _div:
            DEBUG_PRINT("div\n");
            right = *(--vm->sp);
            left = *(--vm->sp);

            *vm->sp++ = element_div(vm,left,right);
            opcode++; // next instruction
            #ifdef DEBUG
            printf("Div result : ");
            element_print(*(vm->sp-1));
            #endif
        break;

        case mod:
            DEBUG_PRINT("mod\n");
            right = *(--vm->sp);
            left = *(--vm->sp);

            *vm->sp++ = element_mod(vm,left,right);
            opcode++; // next instruction
            #ifdef DEBUG
            printf("Mod result : ");
            element_print(*(vm->sp-1));
            #endif
        break;

        case eql:
            #ifdef DEBUG
            element_print(right);
            #endif
            right = *(--vm->sp);
            left = *(--vm->sp);
            #ifdef DEBUG
            printf("compare:\n");
            element_print(right);
            element_print(left);
            #endif
            *vm->sp++ = element_equal(left,right);
            #ifdef DEBUG
            printf("Eql result : ");
            element_print(*(vm->sp-1));
            #endif
            opcode++;
        break;

        case grt:
            #ifdef DEBUG
            element_print(right);
            #endif
            right = *(--vm->sp);
            left = *(--vm->sp);
            #ifdef DEBUG
            printf("compare greater:\n");
            element_print(right);
            element_print(left);
            #endif
            *vm->sp++ = element_greater(left,right);
            #ifdef DEBUG
            printf("Eql result : ");
            element_print(*(vm->sp-1));
            #endif
            opcode++;
        break;

        case lss:
            #ifdef DEBUG
            element_print(right);
            #endif
            right = *(--vm->sp);
            left = *(--vm->sp);
            #ifdef DEBUG
            printf("compare less:\n");
            element_print(right);
            element_print(left);
            #endif
            *vm->sp++ = element_less(left,right);
            #ifdef DEBUG
            printf("Eql result : ");
            element_print(*(vm->sp-1));
            #endif
            opcode++;
        break;
        
        // Args:    Addr
        // Example: jmp, 25
        // Jumps to address 25
        case jmp:
            opcode = code + *(opcode+1);
        break;

        // Depends on either ( eql, nql, grt, lss ) being called before
        // Args:    Addr
        // Example: eql, jmpt, 25
        // Jumps to address 25 if the previous equality check is true
        case jmpt:
            #define check (*(--vm->sp)).value.inumber 
            opcode = (check ? code + *(opcode+1) : opcode+2);
            #undef check
        break;

        // Depends on either ( eql, nql, grt, lss ) being called before
        // Args:    Addr
        // Example: eql, jmpf, 25
        // Jumps to address 25 if the previous equality check is false
         case jmpf:
            #define check (*(--vm->sp)).value.inumber 
            opcode = (!check ? code + *(opcode+1) : opcode+2);
            #undef check
        break;

        case alloc:
        DEBUG_PRINT("alloc\n");
        vm->sp+=*(++opcode);
        opcode++;
        break;

        case halt: 
        DEBUG_PRINT("halt\n");
        break;

        // Args:    offset
        // Example: jmpr, 4  
        // Skips 4 instructions
        case jmpr: 
            opcode += *(opcode+1);
        break;

        // No operation, therefore just goes to the next op code.
        case nop: 
            opcode++; 
        break;
        // Args:    val
        // Example: dstorec, 25
        // Pushes 25 to the top of dp
        case dstorec: 
            DEBUG_PRINT("dstorec[%d] %lld\n",vm->sp-stack,*(opcode+1));

            *(vm->dp+*(++opcode)) = (*(--vm->sp));
/*
            vm->sp->type = type_pointer;
            vm->sp->value.inumber = (size_t)(vm->dp+*(++opcode));
            vm->sp++;

            DEBUG_PRINT("set\n");
            right = *(--vm->sp);
            left = *(--vm->sp);
            *(element_t*)((size_t)right.value.inumber) = left;*/
            opcode++; 
        break;

        // Args: Addr
        // Example: dloadc, 0 
        // Moves the bottom element to the top of dp
        case dloadc: 
            DEBUG_PRINT("dloadc[%d] %lld\n", vm->sp - stack, *(opcode+1));
            // Compute address
  //          element_t *addr = vm->dp + *(++opcode);
            // Push value from that address directly
//            *(vm->sp++) = *addr;
            *(vm->sp++) = *(vm->dp+*(++opcode));

            #ifdef DEBUG
            element_print(*(vm->sp-1));
            #endif

           // opcode++; // move to next instruction
            opcode++;
        break;
        
        case dpushc:
            DEBUG_PRINT("dpushc[%d] %lld\n",vm->sp-stack,*(opcode+1));
            vm->sp->type = type_pointer;
            vm->sp->value.inumber = (size_t)(vm->dp+*(++opcode));
            vm->sp++;
            opcode++;

        break;

        case set: 
            DEBUG_PRINT("set\n");
            right = *(--vm->sp);
            left = *(--vm->sp);
            *(element_t*)((size_t)right.value.inumber) = left;
            opcode++; // next instruction
        break; // TODO

        case get: 
            DEBUG_PRINT("get %d\n",((size_t)(*(vm->sp-1)).value.inumber));
            *(vm->sp++) = *((element_t*)((size_t)(*(--vm->sp)).value.inumber));
            #ifdef DEBUG
            element_print(*(vm->sp-1));
            #endif
            //element_print(*(sp-1));
            opcode++; // next instruction
        break; // TODO

        case table_set:
            right = *(--vm->sp);
            left = *(--vm->sp);

            #ifdef DEBUG
            printf("\nTable set");
 
            element_print(left);
            element_print(right);
            element_print((vm->sp-1));
            printf("\n");
            #endif
            
            if (left.type == type_string) {                
                if (right.type != type_inumber || (vm->sp-1)->type != type_inumber){
                    printf("Error expect inumber when accessing string, got %d %d\n",right.type,(vm->sp-1)->type);
                    exit(2);
                }
                left.value.string->string[right.value.inumber] = (--vm->sp)->value.inumber;
            } else if (left.type == type_table) { 
                table_insert(vm,left.value.table,&right,(--vm->sp));
            }else{
                printf("Error [table_set] expected table/string got '%d'\n",left.type); 
                exit(2);
            }
            opcode++; // next instruction

        break;
       
        /*
        case table_alloc:
            *sp++ = table_create(vm,10,10);
        break;
        */

        case table_access: // try to access member, if it does not exsist create one with the value nill
            DEBUG_PRINT("\ntable_access:\n");
            right = *(--vm->sp);
            left = *(--vm->sp);
  

            element_t* e = NULL;
            if (left.type == type_table){
            e = table_get(p.value.table,&right);
            #ifdef DEBUG
            
            element_print(left);
            element_print(right);
            element_print(*e);
            printf("\n");
            #endif

            }else if (left.type == type_string){
                if (right.type != type_inumber){
                    printf("Error [table_access] for string expected inumber got %d\n");
                    exit(2);
                }
                
                e = &(element_t){
                    .type = type_inumber, 
                    .value.inumber = check_char(left.value.string,right.value.inumber)
                };

            }else{
                printf("Error expected  table/string got '%d'\n",left.type); element_print(right); element_print(left); exit(2);
            }

            if (e == NULL){ //create the element
                *vm->sp++ = (element_t){0}; // empty element
            }else{
                *vm->sp = *e;
                *vm->sp++;
            }
            opcode++; // next instruction

        break;

        // Args: fn, argc
        // Example usage: call,test,3
        // Calls the function test with 3 arguments
        case call: 
            // Function frame
            // 1: JMP return location 
            //TODO 2: Function Parameters add +1 to (size_t)(opcode + 2)
            // Locals are left as junk values to be overwritten later using dset/dget
            *(dstack++) = (size_t)(opcode + 3); /*return adress*/
            *(dstack++) = (size_t)vm->dp;
            *(dstack++) = (size_t)(vm->sp - (*(opcode+2))); /* save stack pointer and "pop" the arguments on return, -1 for return value*/
            DEBUG_PRINT("call return adr %d\n",(size_t)(code + 3));
            vm->dp = vm->sp; 
            opcode = code + *(opcode+1);
        break;

        case call_extern:
            //printf("EXTERN FUNCTION CALL");
            DEBUG_PRINT("call extern\n");

            ((external_func)*(((external_func)*(++opcode))))(vm,*(++opcode));
            opcode++;
        break;
        
        case ret: 
            vm->sp = (element_t*)((size_t)(*(--dstack)));
            vm->dp = (element_t*)((size_t)(*(--dstack)));
            opcode = (int64_t*)((size_t)(*(--dstack)));
            
            DEBUG_PRINT("return adr %d %d\n",(size_t)(*(dstack)),*opcode);
            
        
        break;

        case retv: 
            vm->sp = ((element_t*)((size_t)(*(--dstack)))+1);
            vm->dp = (element_t*)((size_t)(*(--dstack)));
            opcode = (int64_t*)((size_t)(*(--dstack)));
            
            DEBUG_PRINT("return adr %d %d\n",(size_t)(*(dstack)),*opcode);
            
        
        break;
        }
        //TODO this should only be done every x tick, no need to clear memory after every instruction
        mark_vm_stack(vm);
        sweep(vm,&vm->gb_collector0);


    }
    for (int i = 0; i<10; i++){
        printf("[%d] ",i);
        element_print(stack[i]);
    }

    printf("############################ MARK & SWEEP TEST ###########################\n");
    
    mark_vm_stack(vm);    
    int object_count = sweep_count(vm,vm->gb_collector0);
    int objects_freed = sweep(vm,&vm->gb_collector0);
    printf("done with sweep, total list size %d and %d total elements freed\n",object_count,objects_freed);
    
}


/*


if(true){
x+=1
}


*/