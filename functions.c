#include "functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

command op_code[LIMIT_OP] = {{"mov", 0, 2},
                            {"cmp", 1, 2},
                            {"add", 2, 2},
                            {"sub", 3, 2},
                            {"not", 4, 1},
                            {"clr", 5, 1},
                            {"lea", 6, 2},
                            {"inc", 7, 1},
                            {"dec", 8, 1},
                            {"jmp", 9, 1},
                            {"bne", 10, 1},
                            {"red", 11, 1},
                            {"prn", 12, 1},
                            {"jsr", 13, 1},
                            {"rts", 14, 0},
                            {"stop", 15, 0}};

const char *data_opt_arr[] =  {".data", ".string", ".entry", ".extern", NULL};
const char *stock_arr[] = {"@r0", "@r1", "@r2", "@r3", "@r4", "@r5", "@r6", "@r7", NULL};
/*
============================================================
 Functions to assist the reading and parsing functions
============================================================
 */
/* this functions adds ".as" to the and of the file name in order to open in and returns a pointer to the opened file. */
FILE *openFile(char *name, char *end, const char *mode){
    FILE *file;
    char *buf = (char *)malloc(strlen(name) + strlen(AS) + 1); /* memory allocation to hold the corrected file name */
    if(!buf){
        return NULL;
    }
    /* concatenating the file name */
    strcpy(buf, name);
    strcat(buf, end);

    file = fopen(buf, mode); /* open the actual file */
    free(buf);

    return file;
}

/*function to skip white spaces, returns the number of steps to skip */
int skip_white_spaces(const char* line){
    int i = 0;
    /* skip spaces and tabs */
    while(line[i] == ' ' || line[i] == '\t')
        i++;
    return i;
}
/*
============================================================
 Functions to determine the kind of sentence
============================================================
 */
/* function to check if a word from the file is a label or not,returns the length of the label */
int check_if_label_declaration(const char *word, error_ptr *head_error, int num_line){
    int i = 0;
    if(isalpha(word[i])){ /* a label must begin with a letter and can contain only letters and digits*/
        i++;
        while(isdigit(word[i]) || isalpha(word[i]))
            i++;
    }
    if(i > MAX_LABEL)
        return add_error(head_error, LABEL_DECLARATION, num_line);
    return i;
}

/* this function checks if a word is a data statement and returns the location in the array*/
int check_if_data(const char* word){
    int i = 0;
    /* compare the word to each option in the data array to find if the word is a type of data */
    while(data_opt_arr[i]){
        if(strcmp(word, data_opt_arr[i]) == 0)
            return i;
        i++;
    }
    return i;
}

/* this function checks if a word is a command by comparing it to each command there is,
 * if it is a command, it returns the op code of the command, else it returns a larger number */
command *check_if_instruction(const char* word){
    int i = 0;
    /* loop to compare to each word in the op_code array */
    while(i < LIMIT_OP ){
        if(strcmp(word, op_code[i].cmd_name) == 0)
            return &op_code[i];
        i++;
    }
    return NULL;
}

/* this function checks if there's a comma in the line to know is there are multiple arguments
 * return 1 if comma is found, 0 if not */
int check_comma(const char* line, error_ptr *heade, int n_line){
    if(*line == ','){
        /* the first character is a comma so an argument is missing */
        add_error(heade, MISSING_PARAMETER, n_line);
        return 1;
    }
    line++;
    while(*line){
        if(*line == ','){
            if(*(line+1) == '\0')
                /* since the line ends right after the comma, an argument is missing */
                add_error(heade, MISSING_PARAMETER, n_line);
            if(*(line+1) == ',')
                /* double comma */
                add_error(heade, TOO_MANY_COMMAS, n_line);
            return 1;
        }
        line++;
    }
    return 0;
}
int check_if_end(const char *str){
    int i = 0;
    while(isalpha(str[i]) || isdigit(str[i])){
        i++;
    }
    i += skip_white_spaces(str + i);
    if(str[i] == '\n')
        return 0;
    else
        return 1;
}
/* this function checks if a given argument is a legal parameter
 * return 2 if it's a legal parameter, 0 if not*/
int check_if_param(char *param, error_ptr *heade, int n_line) {
    if (*param == '\n' || *param == '\0' || *param == EOF)
        return 0;
    /* the number can start with + or - */
    if (*param == '+' || *param == '-')
        param++;
    while (*param) {
        if (isdigit(*param))
            param++;
        else if (*param == '\n' || *param == '\0' || *param == EOF) {
            return 1;
        } else {
            add_error(heade, WRONG_PARAMETER, n_line);
            return 0;
        }
    }
    return 1;
}
/*
=============================================================
 Functions to update the different lists after reading the entire file
=============================================================
 */
/* this function updated all the nodes in the data list to their current address */
void update_data_address(unsigned int *IC, data_ptr *head){
    data_ptr cur;
    cur = *head;
    while (cur){
        /* add the IC counter to each address in the data list */
        cur->data.address += *IC;
        cur = cur->next;
    }
}
/* this function updates all the nodes in the label list with the current information */
void update_labels(label_ptr *headl, unsigned int *IC){
    label_ptr cur;
    cur = *headl;
    while(cur){
        if(cur->label.external)
            /* if the label is marked external, the address should be set to zero */
            cur->label.address = 0;
        else if(!cur->label.action)
            /* update the label address in case it's an instruction label */
            cur->label.address += *IC;
        cur = cur->next;
    }
}
/* function to update the information in the instructions list , only label references need to be updated*/
void update_instruction_list(label_ptr *head_label, error_ptr *head_error, instruction_ptr *head_instruction){
    instruction_ptr current;
    label_info *lbl;
    current = *head_instruction;
    /* iterate through the instruction list to update all the nodes */
    while(current){
        /* the origin argument is a label */
        if(current->data.adrs_origin == LBL){
            /* check to make sure the label was declared */
            lbl = find_label(head_label, current->data.p1.label.name);
            if(!lbl){
                add_error(head_error, LABEL_NOT_DECLARED, current->data.line_num);
            }
            else{
                current->data.p1.label.adrs = lbl->address;
                if(lbl->external) {
                    current->data.p1.label.ARE = E;
                    current->data.p1.label.adrs = NONE;
                }
                else
                    current->data.p1.label.ARE = R;
            }
        }
        /* the destination argument is a label */
        if(current->data.adrs_destination == LBL){
            lbl = find_label(head_label, current->data.p2.label.name);
            if(!lbl){
                add_error(head_error, LABEL_NOT_DECLARED, current->data.line_num);
            }
            else{
                current->data.p2.label.adrs = lbl->address;
                if(lbl->external)
                    current->data.p2.label.ARE = E;
                else
                    current->data.p2.label.ARE = R;
            }
        }
        /* after the instruction node has been updated, the binary representation should be set */
        finalize_instruction(current);
        current = current->next;
    }
}
/*
==============================================================
 Functions to create final representations of the each line
==============================================================
 */
/* build all the final binary representations of the instruction and their extra words */
void finalize_instruction(instruction_ptr ptr){
    unsigned int temp = 0;
    /* set the binary representation for the command */
    temp |= ptr->data.adrs_origin; /* takes up 3 bits */
    temp <<= 4;
    temp |= ptr->data.op_code; /* takes up 4 bits */
    temp <<= 3;
    temp |= ptr->data.adrs_destination;  /* takes up 3 bits */
    temp <<= 2;
    temp |= ptr->data.ARE; /* takes up 2 bits */
    ptr->final_first = temp;
    /* set the binary representation for the first added word */
    temp = 0;
    switch (ptr->data.adrs_origin){
        case (LBL): {
            temp |= ptr->data.p1.label.adrs; /* takes up 10 bits */
            temp <<= 2;
            temp |= ptr->data.p1.label.ARE; /* takes up 2 bits */
            break;
        }
        case (STCK): {
            temp |= ptr->data.p1.stock.r_or; /* takes up 5 bits */
            temp <<= 5;
            if(ptr->data.adrs_destination == STCK){
                temp |= ptr->data.p1.stock.r_de; /* takes up 5 bits */
                temp <<= 2;
                temp |= ptr->data.p1.stock.ARE; /* takes up 2 bits */
                ptr->final_third = temp;
               return;
            }
            temp <<= 2;
            temp |= ptr->data.p1.stock.ARE; /* takes up 2 bits */
            break;
        }
        case (NMBR): {
            temp |= ptr->data.p1.num.number; /* takes up 10 bits */
            temp <<= 2;
            /* The value for the last two bits is 00 */
            break;
        }
        default:break;
    }
    ptr->final_second = temp;
    temp = 0;
    switch (ptr->data.adrs_destination){
        case(LBL):{
            temp |= ptr->data.p2.label.adrs; /* takes up 10 bits */
            temp <<= 2;
            temp |= ptr->data.p2.label.ARE; /* takes up 2 bits */
            break;
        }
        case(STCK):{
            temp |= ptr->data.p2.stock.r_de; /* takes up 5 bits */
            temp <<= 2;
            temp |= ptr->data.p2.stock.ARE; /* takes up 2 bits */
            break;
        }
        case(NMBR):{
            temp |= ptr->data.p2.num.number; /* takes up 10 bits */
            temp <<= 2;
            /* The value for the last two bits is 00 */
            break;
        }
        default:break;
    }
    ptr->final_third = temp;
}
/* build all the final binary representations of the data */
void finalize_data(data_ptr ptr){
    unsigned int temp = 0;
    switch (ptr->data.type){
        case STRING:{
            temp |= (int) ptr->data.c; /* cast the character value stored in the data node to an int */
            break;
        }
        case NUMBER:{
            temp |= ptr->data.num; /* save the number value stored in the data node */
            break;
        }
        default:break;
    }
    ptr->final = temp;
}
/*
==============================================================
 Functions to create the output files
==============================================================
 */
/* create the ".ent" file and add to it all the entry variables */
void create_ent(label_ptr *head, char *name){
    size_t length = strlen(name)+strlen(ENT);
    label_ptr cur;
    FILE *entry_file;
    char*str = (char*)malloc(sizeof(char)*length);
    if(!str) {
        fprintf(stderr, "Error Failed to allocate memory for a new string");
        return ;
    }
    /* create the file name */
    strcpy(str, name);
    strcat(str, ENT);

    /*Open the file for writing*/
    entry_file = fopen(str, "w+");

    if(!entry_file)
        return;
    cur = *head;
    /* iterate through the label list to print entry valued labels to the file */
    while(cur){
        if(cur->label.entry)
            fprintf(entry_file,"%s\t%d\n", cur->label.name, cur->label.address);
        cur = cur->next;
    }
    fclose(entry_file);
}
/* create the ".ext" file and add to it all the extern variables */
void create_ext(instruction_ptr *headi, char *name){
    size_t length = strlen(name)+strlen(EXT);
    FILE *extern_file;
    instruction_ptr curr;
    char*str = (char*)malloc(sizeof(char)*length);
    if(!str)
    {
        fprintf(stderr, "Error Failed to allocate memory for a new string");
        return;
    }
    /* create the file name */
    strcpy(str, name);
    strcat(str, EXT);

    /*Open the file for writing*/
    extern_file = fopen(str, "w+");

    if(!extern_file)
        return;
    curr = *headi;
    /* iterate through the instruction list to print external valued labels to the file */
    while(curr){
        if(curr->data.adrs_origin == LBL){
            if(curr->data.p1.label.ARE == E){
                fprintf(extern_file, "%s\t%d\n", curr->data.p1.label.name, curr->data.IC);
            }
        }
        if(curr->data.adrs_destination == LBL){
            if(curr->data.p2.label.ARE == E) {
                if (curr->final_first == 0)
                    fprintf(extern_file, "%s\t%d\n", curr->data.p2.label.name, curr->data.IC);
                else
                    fprintf(extern_file, "%s\t%d\n", curr->data.p2.label.name, curr->data.IC+1);
            }
        }
        curr = curr->next;
    }

    fclose(extern_file);
}
/* create the ".ob" file and add to it all the final representations of instructions and data */
void create_ob(instruction_ptr *headi, data_ptr *headd, char *name, unsigned int *DC, unsigned int *IC){
    instruction_ptr  current_instruction;
    data_ptr current_data;
    size_t length = strlen(name)+strlen(OB);
    FILE *object_file;
    char *result = (char*)malloc(BITS_IN_LINE);
    char*str = (char*)malloc(sizeof(char)*length);
    if(!result){
        fprintf(stderr, "Error Failed to allocate memory for a new string");
        return;
    }
    if(!str)
    {
        fprintf(stderr, "Error Failed to allocate memory for a new string");
        return;
    }
    /* create file name */
    strcpy(str, name);
    strcat(str, OB);

    /*Open the file for writing*/
    object_file = fopen(str, "w+");
    /* print to file the number of command lines and data line in memory */
    fprintf(object_file, "%d %d\n", *IC-100, *DC);
    current_instruction = *headi;
    /* loop to iterate through the instruction list */
    while(current_instruction){
        if(current_instruction->final_first != 0){
            /* save to result the converted representation of the command line */
            intToBase64(current_instruction->final_first, result);
            fprintf(object_file, "%s\n", result);
            if(current_instruction->final_second != 0){
                /* save to result the converted representation of the first extra line */
                intToBase64(current_instruction->final_second, result);
                fprintf(object_file, "%s\n", result);
            }
            if(current_instruction->final_third != 0){
                /* save to result the converted representation of the second extra line */
                intToBase64(current_instruction->final_third, result);
                fprintf(object_file, "%s\n", result);
            }
        }
        current_instruction = current_instruction->next;
    }
    current_data = *headd;
    /* loop to iterate through the data list */
    while(current_data){
        /* save to result the converted representation of the data line */
        intToBase64(current_data->final, result);
        fprintf(object_file, "%s\n", result);
        current_data = current_data->next;
    }
}
/*
=============================================================
Functions for number manipulations
=============================================================
 */
/* function to convert a number from decimal to base 64 and return it as a string */
void intToBase64(unsigned int num, char *result){
    char *converter = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"; /* base 64 characters */
    char d[2];
    int i = 0, j, x;  /* variables to control the conversion */
    if(num == 0){
        /* the representation of 0 in base 64 is 'AA' */
        strcpy(result, "AA");
        return;
    }
    /* since the original number is in decimal base, every time it is divided by 64, the remainder value in 64 is the result*/
    while(num){
        d[i] = converter[num % 64];
        i++;
        num = num/64; /* divide the number by 64 to convert the rest of the number to base 64 */
    }
    /* since the number was only divided once, the left 6 bits are all 0 */
    if(i == 1){
        d[i] = 'A';
        i++;
    }
    /* the d string needs to be reversed to hold the correct value */
    for(x=0, j=i-1; j>=0; x++,j--){
        result[x] = d[j];

    }
    result[x] = '\0';
}
/* return the representation of a number in a 2's complement, it will be saved in an instruction node, so the representation is 10 bits*/
int complete_two_instruction(int num) {
    if(num < 0) {
        num = abs(num);
        return MAX_MEMORY - num;
    }
    return num;
}
/* return the representation of a number in a 2's complement, it will be saved in a data node, so the representation is 12 bits*/
int complete_two_data(int num) {
    if(num < 0) {
        num = abs(num);
        return MAX_DATA - num;
    }
    return num;
}