#include "linked_list.h"
#include "functions.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
/*
==========================================================================================================
 Label List
==========================================================================================================
*/
/* function to add a label node to the list */
int add_label(label_ptr *head, char *name, unsigned int address, int action, int external, int entry) {
    label_ptr new, cur;
    new = (label_ptr) malloc(sizeof(label_node));
    if (!new) {
        printf("No memory, function failed");
        return 0;
    }
    name = strtok(name, ":");
    strcpy(new->label.name, name);
    new->label.address = address;
    new->label.action = action;
    new->label.external = external;
    new->label.entry = entry;
    new->next = NULL;

    if(*head == NULL)
        *head = new;
    else {
        cur = *head;
        while (cur->next) {
            cur = cur->next;
        }
        cur->next = new;
    }
    return 0;
}
/* function to find a label node in the list using the label name. the function returns a pointer to the node or NULL if doesn't exist */
label_info *find_label(label_ptr *head, char* name){
    label_ptr cur;
    cur  = *head; /* pointer to the head to pass all the list members */
    while(cur){ /* while current label node is not empty */
        if(strcmp(cur->label.name, name) == 0){
            return &cur->label; /* return the existing label with that name */
        }
        cur = cur->next;
    }
    return NULL;
}
/* function to free all the memory occupied by the label list */
void free_label_list(label_ptr *head){
    label_ptr cur;
    while(*head){/* while next node is no empty, free the current node and adjust pointers */
        cur = (*head)->next;
        free(*head);
        *head = cur;
    }
}
/*
==========================================================================================================
 Error List
==========================================================================================================
*/
/* function to add an error node to the list */
int add_error(error_ptr *head, int error_opt, int line){
    error_ptr new, cur;
    new = (error_ptr)malloc(sizeof(error_node));
    if(!new){
        printf("No memory, function failed");
        return 0;
    }
    new->error.type = error_opt;
    new->error.line = line;
    new->next = NULL;

    if(*head == NULL)
        *head = new;
    else {
        cur = *head;
        while (cur->next) {
            cur = cur->next;
        }
        cur->next = new;
    }
    return 1;
}
/* function to free all the memory occupied by the error instruction list */
void free_error_list(error_ptr *head){
        error_ptr cur;
        while(*head){
            cur = (*head)->next;
            free(*head);
            *head = cur;
        }
}
/* print all the errors that were found */
void print_errors(error_ptr *head){
    error_ptr cur;
    cur = *head;
    while(cur){
        switch (cur->error.type){
            case LABEL_DECLARATION:{
                printf("Incorrect label declaration in line %d.\n", cur->error.line);
                break;
            }
            case LABEL_EXISTS:{
                printf("Re-assignment of the same label in line %d.\n", cur->error.line);
                break;
            }
            case OUT_OF_MEMORY:{
                printf("No more free memory, file is too long from line %d.\n", cur->error.line);
                break;
            }
            case MISSING_COMMA:{
               printf("Missing comma between arguments in line %d.\n", cur->error.line);
                break;
            }
            case MISSING_PARAMETER:{
                printf("Missing parameter on line %d.\n", cur->error.line);
                break;
             }
            case WRONG_PARAMETER:{
                printf("The parameter is incorrect in line %d.\n", cur->error.line);
                break;
            }
            case TOO_MANY_COMMAS: {
                printf("There are too many commas in line %d.\n", cur->error.line);
                break;
            }
            case LABEL_NOT_DECLARED: {
                printf("The label mentioned was not declared in line %d.\n", cur->error.line);
                break;
            }
            case INCORRECT_STOCK:{
                printf("The stock mentioned does not exist. in line %d.\n", cur->error.line);
                break;
            }
            case REDUNDANT_PARAMETER: {
                printf("There is an unnecessary argument in line %d.\n", cur->error.line);
                break;
            }
            case ILLEGAL_ARGUMENT: {
                printf("The argument is not corresponding with the instruction. in line %d.\n", cur->error.line);
                break;
            }
            default:
                break;
        }
        cur = cur->next;
    }
}
/*
==========================================================================================================
Data List
==========================================================================================================
*/
/* function to add a data number node to the list */
int add_data_number(unsigned int *IC, unsigned int *DC, int number, data_ptr *headd, error_ptr *heade, int line){
    if((*IC + *DC) > MAX_MEMORY){
        add_error(heade, OUT_OF_MEMORY, line);
        return 0;
    }
    else{
        data_ptr new, cur;
        new = (data_ptr)malloc(sizeof(data_node));
        if(!new){
            printf("No memory, function failed");
            return 0;
        }
        if(number < 0)
            number = complete_two_data(number);
        new->data.type = NUMBER;
        new->data.num = number;
        new->data.address = *DC;
        new->next = NULL;

        if(*headd == NULL)
            *headd = new;
        else {
            cur = *headd;
            while (cur->next) {
                cur = cur->next;
            }
            cur->next = new;
        }
        return 1;
    }
}
/* function to add a data character node to the list */
int add_data_char(unsigned int *IC, unsigned int *DC, char c, data_ptr *headd, error_ptr *heade, int line){
    data_ptr new, cur;
    if((*IC + *DC) > MAX_MEMORY){
        add_error(heade, OUT_OF_MEMORY, line);
        return 0;
    }
    new = (data_ptr)malloc(sizeof(data_node));
    if(!new){
        printf("No memory, function failed");
        return 0;
    }
    new->data.type = STRING;
    new->data.c = c;
    new->data.address = *DC;
    new->next = NULL;

    if(*headd == NULL)
        *headd = new;
    else {
        cur = *headd;
        while (cur->next) {
            cur = cur->next;
        }
        cur->next = new;
    }
    return 1;
}
/* function to free all the memory occupied by the data list */
void free_data_list(data_ptr *head){
    data_ptr cur;
    while(*head){
        cur = (*head)->next;
        free(*head);
        *head = cur;
    }
}
/*
===========================================================================================================
Instruction List
===========================================================================================================
 */
/* function to add a instruction node to the list */
int add_instruction(unsigned int *DC, unsigned int *IC, unsigned int op_code, int parameters, char *str, error_ptr *heade, instruction_ptr *headi, int line){
    unsigned int j = 0;
    int i = 0;
    int temp;
    char *token;
    instruction_ptr new, current;
    if((*IC + *DC) > MAX_MEMORY){
        add_error(heade, OUT_OF_MEMORY, line);
        return 0;
    }
    else {
        new = (instruction_ptr)malloc(sizeof(instruction_node));
        if(!new){
            printf("----ERROR----NO MEMORY----");
            exit(1);
        }
        new->data.op_code = op_code; /* the number of the command */
        new->data.ARE = A;
        new->data.line_num = line; /* number of line in the file (in case of future errors) */
        new->data.IC = *IC;
        new->next = NULL;
        (*IC)++;
        switch (parameters){
            /* there are no extra words */
            case 0:{
                new->data.adrs_origin = NONE;
                new->data.adrs_destination = NONE;
                new->data.ARE = A;
                /* check if it's the end of the input */
                i += skip_white_spaces(str);
                if(str[i] != '\n'){
                    add_error(heade, REDUNDANT_PARAMETER, line);
                    return 0;
                }
                break;
            }
            case 1:{
                /* there is only one extra word */
                (*IC)++;
                i += skip_white_spaces(str);
                if(str[i] == '@') {
                    /* the argument is a stock */
                    if(op_code == LEA){
                        /* according to the instructions, this command cannot receive a stock as an argument */
                        add_error(heade, ILLEGAL_ARGUMENT, line);
                        return 0;
                    }
                    while (stock_arr[j]) {
                        if (!strcmp(str, stock_arr[j])) {
                            /* the stock referenced is found */
                            new->data.adrs_origin = NONE;
                            new->data.adrs_destination = STCK;
                            new->data.p2.stock.ARE = A;
                            new->data.p2.stock.r_or = NONE;
                            new->data.p2.stock.r_de = j;
                            /* check for errors in the end of the input */
                            i += strlen(stock_arr[j]);
                            i += skip_white_spaces(str+i);
                            if(str[i] == ','){
                                add_error(heade, REDUNDANT_PARAMETER, line);
                                return 0;
                            }
                            if(str[i] != '\n'){
                                add_error(heade, EXCESSIVE_TEXT, line);
                                return 0;
                            }
                            break;
                        }
                        j++;
                    }
                    /* if the stock in input doesn't match any of the stocks in the array, it doesn't exist */
                    if(j == LIMIT_STOCK)
                        add_error(heade, INCORRECT_STOCK, line);
                }
                else {
                    if (check_if_label_declaration(str, heade, line)) {
                        /* the argument is a reference to a label */
                        new->data.adrs_destination = LBL;
                        /* if there's another comma ot more text after the label */
                        if(check_if_end(str)){
                            add_error(heade, REDUNDANT_PARAMETER, line);
                            return 0;
                        }
                        new->data.p2.label.name = (char*)malloc(MAX_LABEL);
                        if(!new->data.p2.label.name)
                            exit(1);
                        sscanf(str, "%s", str);
                        strcpy(new->data.p2.label.name, str);
                    } else if (check_if_param(str, heade, line)) {
                        if(op_code == LEA){
                            /* according tot he instructions lea command can't receive a number as an argument */
                            add_error(heade, ILLEGAL_ARGUMENT, line);
                            return 0;
                        }
                        /* the argument is an explicit number */
                        new->data.adrs_destination = NMBR;
                        new->data.p2.num.ARE = A;
                        sscanf(str, "%d", &temp);
                        /* in case the number is negative so it will be represented correctly */
                        new->data.p2.num.number = (unsigned int) complete_two_instruction(temp);
                        if(check_if_end(str+1)){
                            add_error(heade, REDUNDANT_PARAMETER, line);
                            return 0;
                        }
                    } else {
                        /* the argument doesn't fit */
                        add_error(heade, WRONG_PARAMETER, line);
                        return 0;
                    }
                }
                break;
            }
            case 2:{
                /* this command receives two arguments */
                if (check_comma(str, heade, line)) {
                    token = strtok(str, ","); /* cut out the first argument */
                    if(sscanf(token, "%s", token)) { /* get only the argument without any spaces */
                        /* insert the first argument */
                        /* iterate through all pf the stocks */
                        if (token[0] == '@') {
                            while (stock_arr[j]) {
                                if (!strcmp(token, stock_arr[j])) {
                                    /* the argument is a reference to one of the stocks */
                                    new->data.adrs_origin = STCK;
                                    new->data.p1.stock.ARE = A;
                                    new->data.p1.stock.r_or = j;
                                    (*IC)++;
                                    break;
                                }
                                j++;
                            }
                            if(j == LIMIT_STOCK)
                                add_error(heade, INCORRECT_STOCK, line);
                        }
                        else {
                            /* since it's not a stock check if its a label or an explicit number */
                            if (check_if_label_declaration(token, heade, line)) {
                                /* the argument is a reference to a label */
                                new->data.adrs_origin = LBL;
                                new->data.p1.label.ARE = R;
                                new->data.p1.label.name = (char*)malloc(MAX_LABEL);
                                if(!new->data.p1.label.name)
                                    exit(1);
                                sscanf(token, "%s", token);
                                strcpy(new->data.p1.label.name, token);
                                (*IC)++;
                            } else if (check_if_param(token, heade, line)) {
                                /* the argument is an explicit number */
                                new->data.adrs_origin = NMBR;
                                new->data.p1.num.ARE = A;
                                new->data.p1.num.number = (unsigned int) complete_two_instruction((int) *token);
                                (*IC)++;
                            } else {
                                /* the argument doesn't fit */
                                return 0;
                            }
                        }
                    }
                    token = strtok(NULL, ",");
                    sscanf(token, "%s", token);
                    j = 0;
                    /* insert the second argument */
                    if (token[0] == '@') {
                        while (stock_arr[j]) {
                            if (!strcmp(token, stock_arr[j])) {
                                /*the argument is a reference to one of the stocks */
                                new->data.adrs_destination = STCK;
                                if (new->data.adrs_origin == STCK) {
                                    new->data.p1.stock.r_de = j;
                                } else {
                                    new->data.p2.stock.r_de = j;
                                    new->data.p2.stock.ARE = A;
                                }
                                token = strtok(NULL, ",");
                                if(token){
                                    add_error(heade, REDUNDANT_PARAMETER, line);
                                    return 0;
                                }
                                break;
                            }
                            j++;
                        }
                    }
                    else {
                        if (check_if_label_declaration(token, heade, line)) {
                            /* the argument is a reference to a label */
                            new->data.adrs_destination = LBL;
                            new->data.p2.label.ARE = R;
                            sscanf(token, "%s", token);
                            new->data.p2.label.name = (char*)malloc(MAX_LABEL);
                            if(!new->data.p2.label.name)
                                exit(1);
                            strcpy(new->data.p2.label.name, token);
                            (*IC)++;
                        } else if (check_if_param(token, heade, line)) {
                            if (op_code == CMP || op_code == PRN) {
                                /* the argument is an explicit number */
                                new->data.adrs_destination = NMBR;
                                new->data.p2.num.number = (unsigned int) complete_two_instruction((int) *token);
                                new->data.p2.num.ARE = A;
                                (*IC)++;
                            }
                            else{
                                add_error(heade, ILLEGAL_ARGUMENT, line);
                                return 0;
                            }
                        } else {
                            /* the argument doesn't fit */
                            return 0;
                        }
                        token = strtok(NULL, ",");
                        if(token){
                            add_error(heade, REDUNDANT_PARAMETER, line);
                            return 0;
                        }
                    }
                } else {
                    /* the input doesn't match the necessary structure */
                    add_error(heade, MISSING_COMMA, line);
                    return 0;
                }
            }
            default:break;
        }
        if(*headi == NULL)
            *headi = new;
        else {
            current = (*headi)->next;
            while (current) {
                headi = &(*headi)->next;
                current = current->next;
            }
            (*headi)->next = new;
        }
        return 1;
    }
}
/* function to free all the memory occupied by the instruction list */
void free_instruction_list(instruction_ptr *head){
    instruction_ptr cur;
    while(*head){
        cur = (*head)->next;
        free(*head);
        *head = cur;
    }
}
/*
===========================================================================================================
All Lists
===========================================================================================================
 */
/* function to call all the different free list functions */
void free_lists(label_ptr *headl, error_ptr *heade, data_ptr *headd, instruction_ptr *headi){
    free_error_list(heade);
    free_data_list(headd);
    free_instruction_list(headi);
    free_label_list(headl);
}