#ifndef FILE_READING_H
#define FILE_READING_H

#include "linked_list.h"
#include <stdio.h>

#define MAX_INT 10

/* Parsing a file, and creating the output files. */
void reading_manager(char *name);
/* function to read each line and separate it to its relevant parts */
void parse_line(char* line, int num_lines, label_ptr *head_label, error_ptr *head_error, data_ptr *head_data,
        instruction_ptr *head_instruction, unsigned int *IC, unsigned int *DC, int *extern_flag, int *entry_flag);
/* function to read and process a data line */
int parse_data(char* line, int type, error_ptr *heade, data_ptr *headd, int n_line, unsigned int *IC, unsigned int *DC);
/* function to read and process an entry line */
int parse_entry(label_ptr *head_label, error_ptr *head_error, int n_line, char* line, unsigned int *DC);
/* function to read and process an extern line */
int parse_extern(label_ptr *head_label, error_ptr *head_error, int n_line, char* line);

#endif /* FILE_READING_H */
