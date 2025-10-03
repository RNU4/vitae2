#include <stdio.h>
#include <stdint.h>
int main() {
    // The number to store
    unsigned int number = 2000000;
    
    // Array to store the four numbers
    uint8_t arr[4];
    
    // Fill the array with the number
    *((uint32_t*)arr) = number;
    
    
    // Print the array to verify
    for (int i = 0; i < 4; i++) {
        printf("%u,", arr[i]);
    }
    
    return 0;
}
