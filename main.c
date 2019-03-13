/*
 Maman 14
 Nicole Davidovich 308418763
 */
#include "file_reading.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* Main method. sends each file name in argv to the parsing functions */
int main(int argc, char *argv[]) {
    int i;

    if (argc < 2){
        printf("--- ERROR --- MISSING FILES ---\n");
        return 1;
    }
    for (i = 1; i < argc; i++) {
        /* parse the file */
        reading_manager(argv[i]);
    }
    return 0;
}
