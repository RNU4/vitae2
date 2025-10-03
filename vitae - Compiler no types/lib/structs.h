// Define a structure to represent a Lua element
typedef struct element {
    int type;          // Type of the element (e.g., number, string, table)
    union {
        double number; // Value if the element is a number
        char *string;  // Value if the element is a string
        struct table *table; // Value if the element is a table
        // Add more fields for other types as needed
    } value;
} element_t;