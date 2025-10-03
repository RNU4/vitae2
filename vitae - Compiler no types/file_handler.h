#ifndef FILE_HANDLER_H_   /* Include guard */
#define FILE_HANDLER_H_
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

char *read_file(char *filename) {
    FILE *f = fopen(filename, "rt");
    assert(f);
    fseek(f, 0, SEEK_END);
    long length = ftell(f); //-8 to remove extra chars that are somehow added to end of the file
    fseek(f, 0, SEEK_SET);
    char *buffer = (char *) calloc(1,length + 1);
    buffer[length] = '\0';
    fread(buffer, 1, length, f);
    fclose(f);
    return buffer;
}

int *read_file_int(char *filename) {
    FILE *f = fopen(filename, "rt");
    assert(f);
    fseek(f, 0, SEEK_END);
    long length = ftell(f); //-8 to remove extra chars that are somehow added to end of the file
    fseek(f, 0, SEEK_SET);
    char *buffer = calloc(1,length + sizeof(int));
    fread(buffer, 1, length, f);
    fclose(f);
    return (int*)buffer;
}

int read_integers_from_file(const char *filename, int **buffer, size_t *num_integers) {
    // Open a file for reading
    FILE *file = fopen(filename, "rb"); // "rb" for binary mode, "r" for text mode

    if (file == NULL) {
        perror("Error opening file");
        return 0; // Return 0 to indicate failure
    }

    // Seek to the end of the file to determine its size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET); // Reset file position to the beginning

    // Determine the number of integers in the file
    *num_integers = file_size / sizeof(int);

    // Allocate a buffer to hold the integers
    *buffer = (int *)malloc((*num_integers) * sizeof(int));

    if (*buffer == NULL) {
        perror("Error allocating memory");
        fclose(file);
        return 0; // Return 0 to indicate failure
    }

    // Read the file into the buffer
    size_t elements_read = fread(*buffer, sizeof(int), *num_integers, file);

    if (elements_read != *num_integers) {
        perror("Error reading from file");
        free(*buffer);
        fclose(file);
        return 0; // Return 0 to indicate failure
    }

    // Close the file
    fclose(file);

    return 1; // Return 1 to indicate success
}

int file_write(char *filename, char* buffer,size_t buffer_size){
        // Open a file for writing
    FILE *file = fopen(filename, "wb"); // "wb" for binary mode, "w" for text mode

    if (file == NULL) {
        perror("Error opening file");
        return 1; // Exit with an error code
    }

    // Write the buffer to the file
    size_t bytes_written = fwrite(buffer, 1, buffer_size, file);

    if (bytes_written != buffer_size) {
        perror("Error writing to file");
        fclose(file);
        return 1; // Exit with an error code
    }

    // Close the file
    fclose(file);

    printf("Buffer successfully written to file.\n");

    return 0; // Exit without errors

}
/*bool file_exist(char *filename){
    FILE *f;
    if((f = fopen(filename,"r"))!=NULL)
        {
            // file exists
            fclose(f);
            return true;
        }
    else
        {
            //File not found, no memory leak since 'file' == NULL
            //fclose(file) would cause an error
            return false;
        }
}*/

#endif