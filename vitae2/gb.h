/*garbage collector funtions*/
#include <stdint.h>
#include <stdbool.h>
#ifndef GB_H_
#define GB_H_
typedef enum gbtypes {gbtype_string,gbtype_function,gbtype_table,gbtype_element}gbtypes_t;

typedef struct gc_object {
    struct gc_object* next;    // next object in the global list
    uint8_t type;       //object type
    bool marked;     // mark flag for GC
} gc_object_t;

#define common_header gc_object_t gbobj


#endif