#include <math.h>
#include "structs.h"

#include "strings.h"

#include <stdint.h>


void element_print(element_t a);

int num_digits(int n) {
    if (n == 0) return 1;
    if (n < 0) return (int)log10(-n) + 2; // +1 for digits, +1 for '-'
    return (int)log10(n) + 1;
}

void element_to_string(){} //TODO

element_t element_add(vm_t* vm, element_t a, element_t b) {
    element_t c = {0};

    if (a.type == type_number) {
        if (b.type == type_number) {
            c.value.number = a.value.number + b.value.number;
            c.type = type_number;
        } else if (b.type == type_inumber) {
            c.value.number = a.value.number + (double)b.value.inumber;
            c.type = type_number;
        } else if (b.type == type_string) {
            char buffer[32];
            snprintf(buffer, sizeof(buffer), "%g", a.value.number); // convert float to string
            string_t str = {.string = buffer, .size = sizeof(buffer), .length = strlen(buffer)};
            c.value.string = join_strings2(&str, b.value.string);
            c.type = type_string;

            string_t* st = c.value.string;
            st->gbobj.next = vm->gb_collector0;
            vm->gb_collector0 = &st->gbobj;
        }
    } else if (a.type == type_inumber) {
        if (b.type == type_inumber) {
            c.value.inumber = a.value.inumber + b.value.inumber;
            c.type = type_inumber;
        } else if (b.type == type_number) {
            c.value.number = (double)a.value.inumber + b.value.number;
            c.type = type_number;
        } else if (b.type == type_string) {
            char buffer[32];
            snprintf(buffer, sizeof(buffer), "%lld", a.value.inumber);
            string_t str = {.string = buffer, .size = sizeof(buffer), .length = strlen(buffer)};
            c.value.string = join_strings2(&str, b.value.string);
            c.type = type_string;

            string_t* st = c.value.string;
            st->gbobj.next = vm->gb_collector0;
            vm->gb_collector0 = &st->gbobj;
        }
    } else if (a.type == type_string) {
        if (b.type == type_string) {
            c.value.string = join_strings2(a.value.string, b.value.string);
        } else if (b.type == type_inumber) {
            char buffer[32];
            snprintf(buffer, sizeof(buffer), "%lld", b.value.inumber);
            string_t str = {.string = buffer, .size = sizeof(buffer), .length = strlen(buffer)};
            c.value.string = join_strings2(a.value.string, &str);
        } else if (b.type == type_number) {
            char buffer[32];
            snprintf(buffer, sizeof(buffer), "%g", b.value.number);
            string_t str = {.string = buffer, .size = sizeof(buffer), .length = strlen(buffer)};
            c.value.string = join_strings2(a.value.string, &str);
        }

        string_t* st = c.value.string;
        st->gbobj.next = vm->gb_collector0;
        vm->gb_collector0 = &st->gbobj;

        c.type = type_string;
    }

    return c;
}
element_t element_sub(vm_t* vm, element_t a, element_t b) {
    element_t c = {0};

    if (a.type == type_number) {
        if (b.type == type_number) {
            c.value.number = a.value.number - b.value.number;
            c.type = type_number;
        } else if (b.type == type_inumber) {
            c.value.number = a.value.number - (double)b.value.inumber;
            c.type = type_number;
        }
    } else if (a.type == type_inumber) {
        if (b.type == type_inumber) {
            c.value.inumber = a.value.inumber - b.value.inumber;
            c.type = type_inumber;
        } else if (b.type == type_number) {
            c.value.number = (double)a.value.inumber - b.value.number;
            c.type = type_number;
        }
    }

    return c;
}
element_t element_mul(vm_t* vm, element_t a, element_t b) {
    element_t c = {0};

    if (a.type == type_number) {
        if (b.type == type_number) {
            c.value.number = a.value.number * b.value.number;
            c.type = type_number;
        } else if (b.type == type_inumber) {
            c.value.number = a.value.number * (double)b.value.inumber;
            c.type = type_number;
        }
    } else if (a.type == type_inumber) {
        if (b.type == type_inumber) {
            c.value.inumber = a.value.inumber * b.value.inumber;
            c.type = type_inumber;
        } else if (b.type == type_number) {
            c.value.number = (double)a.value.inumber * b.value.number;
            c.type = type_number;
        }
    }

    return c;
}

element_t element_div(vm_t* vm, element_t a, element_t b) {
    element_t c = {0};

    if (a.type == type_number) {
        if (b.type == type_number) {
            c.value.number = (b.value.number != 0.0) 
                             ? a.value.number / b.value.number 
                             : 0.0;
            c.type = type_number;
        } else if (b.type == type_inumber) {
            c.value.number = (b.value.inumber != 0) 
                             ? a.value.number / (double)b.value.inumber 
                             : 0.0;
            c.type = type_number;
        }
    } else if (a.type == type_inumber) {
        if (b.type == type_inumber) {
            c.value.inumber = (b.value.inumber != 0) 
                              ? a.value.inumber / b.value.inumber 
                              : 0;
            c.type = type_inumber;
        } else if (b.type == type_number) {
            c.value.number = (b.value.number != 0.0) 
                             ? (double)a.value.inumber / b.value.number 
                             : 0.0;
            c.type = type_number;
        }
    }

    return c;
}

element_t element_mod(vm_t* vm, element_t a, element_t b) {
    element_t c = {0};

    if (a.type == type_inumber) {
        if (b.type == type_inumber) {
            c.value.inumber = (b.value.inumber != 0) 
                              ? (a.value.inumber % b.value.inumber) 
                              : 0;
            c.type = type_inumber;
        } else if (b.type == type_number) {
            c.value.number = (b.value.number != 0.0) 
                             ? fmod((double)a.value.inumber, b.value.number) 
                             : 0.0;
            c.type = type_number;
        }
    } else if (a.type == type_number) {
        if (b.type == type_number) {
            c.value.number = (b.value.number != 0.0) 
                             ? fmod(a.value.number, b.value.number) 
                             : 0.0;
            c.type = type_number;
        } else if (b.type == type_inumber) {
            c.value.number = (b.value.inumber != 0) 
                             ? fmod(a.value.number, (double)b.value.inumber) 
                             : 0.0;
            c.type = type_number;
        }
    }

    return c;
}

element_t element_equal(element_t a,element_t b){
    element_t c;
    c.type = type_inumber;

    if (a.type == type_number){
        if (b.type == type_number){ // same type
            c.value.inumber = (a.value.number == b.value.number);
        }else if (b.type == type_inumber){
            c.value.inumber = (a.value.number == (double)b.value.inumber);
        }
    }else if (a.type == type_inumber){
        if (b.type == type_inumber){ // same type
            c.value.inumber = a.value.inumber == b.value.inumber;
        }else if (b.type == type_number){
            c.value.inumber = ((double)a.value.inumber == b.value.number);
        }
    }else if (a.type == type_string || b.type == type_string){
        if (a.type == type_string && b.type != type_string){
            c.value.inumber = 0;
        }else if (b.type == type_string && a.type != type_string){
            c.value.inumber = 0;
        }else{ // both are strings
            c.value.inumber = compare_strings(a.value.string,b.value.string);
        }
    }else{ // if they compare tables or some weird this will work because it would check if both pointers are the same
        c.value.inumber = a.value.inumber == b.value.inumber;
    }
    return c;
}

element_t element_less(element_t a,element_t b){
    element_t c;
    c.type = type_inumber;
    if (a.type == type_number){
        if (b.type == type_number){ // same type
            c.value.inumber = (a.value.number < b.value.number);
        }else if (b.type == type_inumber){
            c.value.inumber = (a.value.number < (double)b.value.inumber);
        }
    }else if (a.type == type_inumber){
        if (b.type == type_inumber){ // same type
            c.value.inumber = a.value.inumber < b.value.inumber;
        }else if (b.type == type_number){
            c.value.inumber = ((double)a.value.inumber < b.value.number);
        }
    }else if (a.type == type_string || b.type == type_string){ /*a string cant be less? so its always false*/
        c.value.inumber = 0;
    }else{
        c.value.inumber = 0;
    }
    return c;
}

element_t element_greater(element_t a,element_t b){
    element_t c;
    c.type = type_inumber;
    if (a.type == type_number){
        if (b.type == type_number){ // same type
            c.value.inumber = (a.value.number > b.value.number);
        }else if (b.type == type_inumber){
            c.value.inumber = (a.value.number > (double)b.value.inumber);
        }
    }else if (a.type == type_inumber){
        if (b.type == type_inumber){ // same type
            c.value.inumber = a.value.inumber > b.value.inumber;
        }else if (b.type == type_number){
            c.value.inumber = ((double)a.value.inumber > b.value.number);
        }
    }else if (a.type == type_string || b.type == type_string){ /*a string cant be less? so its always false*/
        c.value.inumber = 0;
    }else{
        c.value.inumber = 0;
    }
    return c;
}

void element_print(element_t a){
    switch(a.type){
        case type_number: 
        printf("number: %f\n",a.value.number);
        break;
        case type_inumber: 
        printf("inumber:  %lld\n",a.value.inumber);
        break;
        case type_pointer:
        printf("pointer:  %lld value: ",a.value.inumber);
        element_print(*(element_t*)(size_t)(a.value.inumber));
        break;

        case type_string: 
        if (a.value.string != NULL && a.value.string->string != NULL)
        printf("STRING '%s'\n",a.value.string->string);
        else 
        printf("STRING NIL\n");
        break;
        case type_table:
        printf("Table: %d\n",a.value.table->hashtable);
        break;
        case type_nil:
        printf("NIL\n");
        break;
        default:
        printf("UNKOWN: %d\n",a.value);

        break;

    }
}


void print_data(element_t a){
    switch(a.type){
        case type_number: 
        printf("%f",a.value.number);
        break;
        case type_inumber: 
        printf("%lld",a.value.inumber);
        break;
        case type_pointer:
        printf("%lld",a.value.inumber);
        break;

        case type_string: 
        if (a.value.string != NULL && a.value.string->string != NULL)
        printf("%s",a.value.string->string);
        else 
        printf("NIL\n");
        break;
        case type_table:
        printf("%d",a.value.table->hashtable);
        break;
        case type_nil:
        printf("NIL\n");
        break;
        default:
        printf("%d",a.value);
        break;
    }
}


void print_table(hashtable_t* ht){
    printf("------------------------\n");
    for (int i=0; i<ht->size; i++){
        if (ht->hash_table[i]->name[0] == 0){ //!this is probaly gonna crash someday
            printf("%i\t---\n",i);
        } else{
            printf("%i\t",i);//,ht->hash_table[i]->name,(int*)ht->hash_table[i]->value);

            hash_element_t* tmp = ht->hash_table[i];
            while(tmp->next !=NULL){ //!this is probaly gonna crash someday
                printf(" [\"%s\"] = ",tmp->name); element_print(*((element_t*)tmp->value));
                tmp = tmp->next;
            }

            printf("\n");
        }
    }
    printf("------------------------\n");
}