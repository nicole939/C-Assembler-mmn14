
#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#define MAX_LABEL 31
#define MAX_MEMORY 1024
#define NO 0
#define YES 1
#define LEA 6
#define CMP 1
#define PRN 12
typedef enum {A, E, R} ARE;

/*
====================================================================================================================
 Label data structure
====================================================================================================================
 */
typedef struct label_info {
    char name[MAX_LABEL];
    unsigned int address; /* address of the label appearance */
    int action; /* 1 if it's a command, 0 if not */
    int external; /* 1 if it's an external value, 0  if not */
    int entry; /* 1 if it's an entry value, 0 if not */
} label_info;
typedef struct label_node * label_ptr;
typedef struct label_node {
    label_info label;
    label_ptr next;
} label_node;
/*
=====================================================================================================================
 Label functions
=====================================================================================================================
*/
/* function to add a label node to the list */
int add_label(label_ptr *head, char name[MAX_LABEL], unsigned int address, int action, int external, int entry);
/* function to find a label node in the list using the label name. the function returns a pointer to the node or NULL if doesn't exist */
label_info *find_label(label_ptr *head, char* name);
/* function to free all the memory occupied by the label list */
void free_label_list(label_ptr *head);
/*
=====================================================================================================================
 Error data structure
=====================================================================================================================
*/
/* all the different error types to choose from */
typedef enum { LABEL_DECLARATION, LABEL_EXISTS, OUT_OF_MEMORY,
    MISSING_COMMA, MISSING_PARAMETER, WRONG_PARAMETER, TOO_MANY_COMMAS,
    LABEL_NOT_DECLARED, INCORRECT_STOCK, REDUNDANT_PARAMETER, ILLEGAL_ARGUMENT,
    EXCESSIVE_TEXT} error_opt;

typedef struct error_info{
    int type; /* the type of the error */
    int line; /* number of line in the file where the error occurred */
} error_info;
typedef struct error_node * error_ptr;
typedef struct error_node {
    error_info error;
    error_ptr next;
} error_node;
/*
====================================================================================================================
Error functions
====================================================================================================================
*/
/* function to add an error node to the list */
int add_error(error_ptr *head, int error_opt, int line);
/* function to free all the memory occupied by the error list */
void free_error_list(error_ptr *head);
/* print all the errors that were found */
void print_errors(error_ptr *head);
/*
=====================================================================================================================
Data data structure
=====================================================================================================================
*/
typedef enum {STRING, NUMBER} data_type;
typedef struct data_info {
    int type; /* type of data saved, STRING or NUMBER */
    char c; /* character variable saved */
    int num; /* int variable saved */
    int address; /* the data address in the memory */
} data_info;
typedef struct data_node * data_ptr;
typedef struct data_node {
    data_info data;
    unsigned int final : 12;
    data_ptr next;
} data_node;
/*
=====================================================================================================================
 Data functions
=====================================================================================================================
*/
/* function to add a data number node to the list */
int add_data_number(unsigned int *IC, unsigned int *DC, int number, data_ptr *headd, error_ptr *heade, int line);
/* function to add a data character node tp the list */
int add_data_char(unsigned int *IC, unsigned int *DC, char c, data_ptr *headd, error_ptr *heade, int line);
/* function to free all the memory occupied by the data list */
void free_data_list(data_ptr *head);
/*
=====================================================================================================================
 Instruction data structure
=====================================================================================================================
 the data structure is build to hold all the information of the command line and
 two arguments p1 and p2 to hold the structure representing the extra lines added.
 the extra lines are represented by union structures so they could optionally hold
 information of a stock, label or numerical argument, depends on the input, and the
 three of them represented in different structs
 */
typedef enum {NONE = 0, NMBR = 1, LBL = 3, STCK = 5} prm_type;

typedef struct op_stock {
    unsigned int r_or : 5; /* representation of the origin stock */
    unsigned int r_de : 5; /* representation of the destination stock */
    unsigned int ARE : 2;
} op_stock;

typedef struct op_num {
    unsigned int number : 10;
    unsigned int ARE : 2;
} op_num;

typedef struct op_label {
    char *name;
    unsigned int adrs : 10;
    unsigned int ARE : 2;
} op_label;

typedef union  op_type {
    op_stock stock;
    op_num num;
    op_label label;
} op_type;

typedef struct instruction_info {
    unsigned int op_code : 4;
    unsigned int adrs_origin : 3; /* the address method for the origin  */
    unsigned int adrs_destination : 3; /* the address method for the destination */
    unsigned int ARE : 2;
    int IC;
    int line_num; /* the line number in the file where the instruction is listed */
    op_type p1; /* origin extra word */
    op_type p2; /* destination extra word */
} instruction_info;

typedef struct instruction_node * instruction_ptr;

typedef struct instruction_node {
    instruction_info data;
    unsigned int final_first : 12; /* representation of the instruction word  */
    unsigned int final_second : 12; /* representation of the first added word */
    unsigned int final_third : 12; /* representation of the second added word */
    instruction_ptr next;
} instruction_node;
/*
=====================================================================================================================
 Instruction functions
=====================================================================================================================
*/
/* function to add a instruction node to the list */
int add_instruction(unsigned int *DC, unsigned int *IC, unsigned int op_code, int parameters, char *str, error_ptr *heade, instruction_ptr *headi, int line);
/* function to free all the memory occupied by the instruction list */
void free_instruction_list(instruction_ptr *head);
/*
=====================================================================================================================
 All Lists
=====================================================================================================================
*/
/* function to call all the different free list functions */
void free_lists(label_ptr *headl, error_ptr *heade, data_ptr *headd, instruction_ptr *headi);

#endif /* LINKED_LIST_H */
