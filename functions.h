
#ifndef FUNCTIONS_H

#include <stdio.h>
#include "linked_list.h"

#define FUNCTIONS_H

#define MAX_DATA 4096
#define MAX_LINE 80
#define LIMIT_OP 16
#define LIMIT_STOCK 8
#define BITS_IN_LINE 12
#define LIMIT_DATA_OPT 4

#define AS ".as"
#define OB ".ob"
#define ENT ".ent"
#define EXT ".ext"

typedef enum {LABEL, DATA, INSTRUCTION, ERROR} line_opt;

typedef struct command {
    const char *cmd_name;
    unsigned int op_code;
    int parameters;
} command;

extern const char* stock_arr[];
/*
============================================================
 Functions to assist the reading and parsing functions
============================================================
 */
/* this functions adds ".as" to the and of the file name in order to open in and returns a pointer to the opened file. */
FILE *openFile(char *name, char *end, const char *mode);
/*function to skip white spaces, returns the number of steps to skip */
int skip_white_spaces(const char* line);

/*
============================================================
 Functions to determine the kind of sentence
============================================================
 */
/* function to check if a word from the file is a label or not, return 1 if yes, 0 if no */
int check_if_label_declaration(const char *word, error_ptr *head_error, int num_line);
/* this function checks if a word is a data statement */
int check_if_data(const char* word);
/* this function checks if a word is a command by comparing it to each command there is,
 * if it is a command, it returns the op code of the command, else it returns a larger number */
command *check_if_instruction(const char* word);
/* this function checks if there's a comma in the line to know is there are multiple arguments */
int check_comma(const char* line, error_ptr *heade, int n_line);
/* this function checks if we reached the end of the line */
int check_if_end(const char *str);
/* this function checks if a given argument is a legal parameter */
int check_if_param(char *param, error_ptr *heade, int n_line);
/*
============================================================
Functions to update the different lists after reading the entire file
============================================================
 */
/* this function updated all the nodes in the data list to their current address */
void update_data_address(unsigned int *IC, data_ptr *head);
/* this function updates all the nodes in the label list with the current information */
void update_labels(label_ptr *headl, unsigned int *IC);
/* function to update the information in the instructions list */
void update_instruction_list(label_ptr *head_label, error_ptr *head_error, instruction_ptr *head_instruction);
/*
============================================================
Functions to create final representations of the each line
============================================================
 */
/* build all the final binary representations of the instruction and their extra words */
void finalize_instruction(instruction_ptr ptr);
/* build all the final binary representations of the data */
void finalize_data(data_ptr ptr);
/*
============================================================
 Functions to create the output files
============================================================
 */
/* create the ".ent" file and add to it all the entry variables */
void create_ent(label_ptr *head, char *name);
/* create the ".ext" file and add to it all the extern variables */
void create_ext(instruction_ptr *headi, char *name);
/* create the ".ob" file and add to it all the final representations of instructions and data */
void create_ob(instruction_ptr *headi, data_ptr *headd, char *name, unsigned int *DC, unsigned int *IC);
/*
=============================================================
Functions for number manipulations
=============================================================
 */
/* function to convert a number from a string to base 64 */
void intToBase64(unsigned int num, char *result);
/* return the representation of a number in a 2's complement, it will be saved in an instruction node, so the representation is 10 bits*/
int complete_two_instruction(int num);
/* return the representation of a number in a 2's complement, it will be saved in a data node, so the representation is 12 bits*/
int complete_two_data(int num);

#endif /* FUNCTIONS_H */
