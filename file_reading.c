#include "file_reading.h"
#include "functions.h"
#include "linked_list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Parsing a file, and creating the output files. */
void reading_manager(char *name) {
    /* variable to save the location in the original file */
    int num_lines = 1;
    /* IC - Instruction Count, DC - Data Count */
    unsigned int IC = 100,DC = 0;
    /* pointers to all the linked lists to be passed through he entire program */
    label_ptr head_label;
    error_ptr head_error;
    data_ptr head_data;
    instruction_ptr head_instruction;

    int extern_flag = NO;
    int entry_flag = NO;
    char buf[MAX_LINE];
    data_ptr current;
    FILE *file = openFile(name, ".as", "r");
    if (file == NULL) {
        printf("--- ERROR --- COULDN'T OPEN FILE ---\n");
        return;
    }
    printf("File \"%s.as\" was found.\n", name);
    /* initialize all the lists pointers to NULL */
    head_label = NULL, head_error = NULL, head_data = NULL, head_instruction = NULL;

    /* loop to iterate through each line in the file, each line is read in a designated function */
    while (fgets(buf, MAX_LINE, file) != NULL) {
        parse_line(buf, num_lines, &head_label, &head_error, &head_data, &head_instruction, &IC, &DC, &extern_flag, &entry_flag);
        num_lines++;
    }
    /* print error if there are any and return to the main program to move to the next file. */
    if(head_error){
        print_errors(&head_error);
        return;
    }
    /* after gathering all the information from the file, the data list should be updated so all the variables ar completed */
    update_data_address(&IC, &head_data);
    /* print error if there are any and return to the main program to move to the next file. */
    if(head_error){
        print_errors(&head_error);
        return;
    }
    current = head_data;
    /* loop through the data list to finalize the binary representation of each data */
    while(current) {
        finalize_data(current);
        current = current->next;
    }
    /* lastly the labels are updated with the final information */
    update_labels(&head_label, &IC);

    /* after the labels and data are accurate, the instruction list can be finalized */
    update_instruction_list(&head_label, &head_error, &head_instruction);

    /* print error if there are any and return to the main program to move to the next file. */
    if(head_error){
        print_errors(&head_error);
        return;
    }

    /* create all the files */
    create_ob(&head_instruction, &head_data, name, &DC, &IC);
    if(entry_flag)
        create_ent(&head_label, name);
    if(extern_flag)
        create_ext(&head_instruction, name);
    fclose(file);
    /* free the memory occupied by the lists */
    free_lists(&head_label, &head_error, &head_data, &head_instruction);
    entry_flag = NO;
}
/* function to read each line and separate it to its relevant parts */
void parse_line(char* line, int num_lines, label_ptr *head_label, error_ptr *head_error, data_ptr *head_data, instruction_ptr *head_instruction, unsigned int *IC,
                unsigned int *DC, int *extern_flag, int *entry_flag){
    int i, dat;
    char *label;
    label_info *cur;
    int status = ERROR;
    char first_word[MAX_LABEL];
    char sec_word[MAX_LABEL];
    command *cmd_ptr;

    i = skip_white_spaces(line);
    /* skip to the next line if it's empty or it's the end  or it's a comment*/
    if(line[i] == ';' || line[i] == '\n' || line[i] == EOF)
        return;
    else if(sscanf(line + i, "%s", first_word)){
        /* determine if the line declares label, data or instruction */
        if(first_word[check_if_label_declaration(first_word, head_error, num_lines)] == ':')
            status = LABEL;
        else if(check_if_data(first_word)<LIMIT_DATA_OPT)
            status = DATA;
        else if(check_if_instruction(first_word))
            status = INSTRUCTION;
    }
    switch (status){
        case LABEL: {
            i += strlen(first_word);
            i += skip_white_spaces(line+i);
            /* cut out the next word */
            if(sscanf(line+i, "%s", sec_word)){
                dat = check_if_data(sec_word);
                switch (dat){
                    case 0: {
                        /*  .data  */
                        strcpy(first_word,strtok(first_word,":"));
                        /* find the label in the label list if it exists and update it's address */
                        if((cur = find_label(head_label,first_word))){
                            if(cur->address == 0) {
                                cur->address = *DC;
                                /* parse the rest of the line, to add the data to the data list */
                                parse_data(line + i + strlen(sec_word), dat, head_error, head_data, num_lines, IC, DC);
                                return;
                            }
                            else{
                                add_error(head_error, LABEL_EXISTS, num_lines);
                                return;
                            }
                        } else {
                            /* the label doesn't exist yet so add a new label */
                            add_label(head_label, first_word, *DC, 0, 0, 0);
                            parse_data(line + i + strlen(sec_word), dat, head_error, head_data, num_lines, IC, DC);
                            return;
                        }
                    }
                    case 1: {
                        /*  .string  */
                        label = strtok(first_word, ":");
                        /* find the label in the label list if it exists and update it's address */
                        if((cur = find_label(head_label, label))){
                            if(cur->address == 0){
                                cur->address = *DC;
                                /* parse the rest of the line, to add the data to the data list */
                                parse_data(line + i + strlen(sec_word), dat, head_error,head_data,num_lines, IC, DC);
                                return;
                            }
                            else{
                                add_error(head_error, LABEL_EXISTS, num_lines);
                                return;
                            }
                        } else {
                            /* the label doesn't exist yet so add a new label */
                            add_label(head_label, label, *DC, 0, 0, 0);
                            parse_data(line + i + strlen(sec_word), dat, head_error, head_data, num_lines, IC, DC);
                            return;
                        }
                    }
                    case 2: {
                        /*  .entry  */
                        parse_entry(head_label, head_error, num_lines, line + i + strlen(sec_word), DC);
                        *entry_flag = YES;
                        return;
                    }
                    case 3: {
                        /*  .extern  */
                        parse_extern(head_label, head_error, num_lines, line + i + strlen(sec_word));
                        *entry_flag = YES;
                        return;
                    }
                    default:break;
                }
                /* is this part is reached, the line is a command */
                if((cmd_ptr = check_if_instruction(sec_word))){
                    /* add to the label list with a positive action value */
                    add_label(head_label, first_word, *IC, 1, 0, 0);
                    i += strlen(sec_word);
                    i += skip_white_spaces(line+i);
                    line += i;
                    /*  add the command to the instruction list */
                    add_instruction(DC, IC, cmd_ptr->op_code, cmd_ptr->parameters, line, head_error, head_instruction, num_lines);
                    return;
                }
            }
        }
        case DATA: {
            dat = check_if_data(first_word);
            switch (dat){
                case 0: { /*.data */
                    parse_data(line + i + strlen(first_word), dat, head_error, head_data, num_lines, IC, DC);
                    return;
                }
                case 1: { /* .string */
                    parse_data(line + i + strlen(first_word), dat, head_error, head_data, num_lines, IC, DC);
                    return;
                }
                case 2: { /* .entry */
                    parse_entry(head_label, head_error, num_lines, line + i + strlen(first_word), DC);
                    *entry_flag = YES;
                    return;
                }
                case 3: { /* .extern */
                    parse_extern(head_label, head_error, num_lines, line + i + strlen(first_word));
                    *extern_flag = YES;
                    return;
                }
                default:break;
            }
        }
        case INSTRUCTION: {
            cmd_ptr =  check_if_instruction(first_word);
            i += strlen(first_word);
            i += skip_white_spaces(line+i);
            line += i;
            add_instruction(DC, IC, cmd_ptr->op_code, cmd_ptr->parameters, line, head_error, head_instruction, num_lines);
            return;
        }
        default:break;
    }
}

/* function to read and process a data line */
int parse_data(char* line, int type, error_ptr *heade, data_ptr *headd, int n_line, unsigned int *IC, unsigned int *DC){
    int i, count, temp;
    char number[MAX_INT];
    char *num;
    /* handle differently .data, .string, .entry, .extern */
    switch (type){
        case 0: { /* .data */
            count = 0;
            strcpy(number, "\0");
            /* cut out the number value from the line */
            num = strtok(line, ",");
            /* this loop is to iterate through all the number values in the line */
            while (num != NULL) {
                i =  skip_white_spaces(num);
                if(check_if_param(num+i, heade, n_line)) {
                    /* cut our only the number without spaces */
                    sscanf(num, "%d", &temp);
                    if (add_data_number(IC, DC, temp, headd, heade, n_line)) {
                        count++;
                        (*DC)++;
                    } else
                        return 0;
                    num = strtok(NULL, ",");
                }
                else
                    return 0;
            }
            break;
        }
        case 1: { /* .string */
            count = 0;
            i = skip_white_spaces(line);
            /* since it's a string it must start with a quotation mark */
            if(line[i] == '"')
                i++;
            if(line[i] == '\"')
                i++;
            /* iterate to add each character to the data list */
            while(line[i] != '\"') {
                if(add_data_char(IC,DC,line[i], headd,heade,n_line)) {
                    (*DC)++;
                    i++;
                    count++;
                }
                else{
                    return 0;
                }
            }
            if(count > 0){
                /* since at least one character was added to the data list we need to add the end of string sign */
                if(add_data_char(IC, DC, '\0', headd,heade,n_line)){
                    (*DC)++;
                    i++;
                    count++;
                }
                else
                    return 0;
            }
            i += skip_white_spaces(line + i);
            /* return the number of characters added to the data list only if it's the end of the line */
            if(line[i] == '\n')
                return count;
            else{
                add_error(heade, WRONG_PARAMETER, n_line);
                return 0;
            }
        }
        default:break;
    }
    return 0;
}
/* function to read and process an entry line */
int parse_entry(label_ptr *head_label, error_ptr *head_error, int n_line, char* line, unsigned int *DC){
    char word[MAX_LABEL];
    if(sscanf(line, "%s", word)){
        if(find_label(head_label, word)){
            /* if the label already exists, just update it to hold a positive value for entry */
            find_label(head_label, word)->entry = 1;
            return 1;
        }
        else{
            /* since the label doesn't exist yet, add a new one with a positive value for entry */
            return(add_label(head_label, word, *DC, 0, 0, 1));
        }
    }
    else{
        add_error(head_error, MISSING_PARAMETER,n_line);
        return 0;
    }
}
/* function to read and process an extern line */
int parse_extern(label_ptr *head_label, error_ptr *head_error, int n_line, char* line){
    char word[MAX_LABEL];
    if(sscanf(line, "%s", word))
        /* add a new label tagged as extern */
        return add_label(head_label,word, 0,0,1,0);

    else{
        add_error(head_error, MISSING_PARAMETER,n_line);
        return 0;
    }
}
