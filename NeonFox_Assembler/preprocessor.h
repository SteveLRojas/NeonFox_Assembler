#include <stdio.h> 
#include <stdlib.h> 
#include <stdint.h>
#include <string.h>
#include "scannerhelper.h"

int preprocess(char* asm_name, char* lst_name, uint8_t debug_enable);

//Represents a line of the source code after initial parsing.
typedef struct LINKED_LINE
{
	char* s_label;				//label portion of source line (if any)
	char* s_line;				//mnemonic or key word portion of source line (always present)
	unsigned long n_line;		//source line number for printing and error mesage use only
	uint8_t name_index;			//identifies the file that contains this line of source code
	struct LINKED_LINE* next;
} linked_line;

typedef struct LINKED_MACRO
{
	linked_line* line_head;
	char* macro_name;
	char* formal_parameters;
	unsigned long n_line;
	uint8_t name_index;
	struct LINKED_MACRO* next;
} linked_macro;

//Assembler core functions
void load_file(char* file_name, linked_line* head);
void include_merge(linked_line* prev_node, linked_line* include_node, linked_line* new_head);
void find_and_replace(linked_line* current_node, char* s_replace, char* s_new);

//Helper functions
void free_node(linked_line* current_node);
void free_macro(linked_macro* current_macro);
int str_comp_partial(const char* str1, const char* str2);
int str_find_word(char* where, char* what, unsigned int* start, unsigned int* end);
void str_replace(char** where, char* s_new, unsigned int word_start, unsigned int word_end);
unsigned int str_size(const char* s_input);

