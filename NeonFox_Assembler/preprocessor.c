#include "preprocessor.h"

const char include_string[] = "INCLUDE";
const char macro_string[] = "MACRO";
const char endmacro_string[] = "ENDMACRO";
const char equ_string[] = "EQU";

const char push_file_string[] = "@PUSH_FILE:";
const char pop_file_string[] = "@POP_FILE:";
const char push_macro_string[] = "@PUSH_MACRO:";
const char pop_macro_string[] = "@POP_MACRO:";

char include_name_buf[256];
char* name_table[256];

int preprocess(char* asm_name, char* lst_name, uint8_t debug_level)
{
	if(!asm_name || !(*asm_name))
	{
		printf("No ASM source file specified!\n");
		exit(1);
	}
	if(!lst_name || !(*lst_name))
	{
		printf("No LST output file specified!\n");
		exit(1);
	}

	linked_line* head = (linked_line*)malloc(sizeof(linked_line));
	linked_line* current_node = head;
	linked_macro* macro_head = (linked_macro*)malloc(sizeof(linked_macro));
	linked_macro* current_macro = macro_head;

	for(unsigned int d = 0; d < 256; ++d)
	{
		name_table[d] = NULL;
	}
	
	//initialize linked lists
	head->n_line = 0xffffffff;
	head->s_label = NULL;
	head->s_line = NULL;
	head->next = NULL;

	macro_head->formal_parameters = NULL;
	macro_head->line_head = NULL;
	macro_head->macro_name = NULL;
	macro_head->n_line = 0xffffffff;
	macro_head->name_index = 0;
	macro_head->next = NULL;

	//Load source file
	// load_file(argv[asm_index], head);
	load_file(asm_name, head);
	
	//Replace includes with source
	linked_line* new_head = (linked_line*)malloc(sizeof(linked_line));
	linked_line* prev_node = NULL;
	new_head->n_line = 0xffffffff;
	new_head->s_label = NULL;
	new_head->s_line = NULL;
	while(1)
	{
		prev_node = current_node;
		current_node = current_node->next;
		if(current_node == NULL)
			break;
		if(current_node->s_label == NULL)
			continue;
		if (str_comp_partial(current_node->s_label, include_string))
		{
			char* line = current_node->s_line;
	
			unsigned int start = 0;
			unsigned int end = 0;
			for(unsigned int i = 0; line[i]; i++)
			{
				if (line[i] == '"')
				{
					start = i + 1;
					break;
				}
			}
			for(unsigned int i = start; line[i]; i++)
			{
				if (line[i] == '"')
				{
					end = i - 1;
					break;
				}
			}
			
			// Didn't find two quotes
			if (end == 0)
			{
				fprintf(stderr, "Syntax error in Include statement in file %s at line: %lu\n", name_table[current_node->name_index], current_node->n_line);
				exit(1);
			}
			
			unsigned int buf_i = 0;
			for(unsigned int i = start; i <= end; i++)
			{
				include_name_buf[buf_i] = line[i];
				++buf_i;
			}
			include_name_buf[buf_i]='\0';
			
			load_file(include_name_buf, new_head);
			include_merge(prev_node, current_node, new_head);
			free(current_node->s_label);
			free(current_node->s_line);
			free(current_node);
			current_node = prev_node;
		}
	}
	free(new_head);

	//find and replace defines
	current_node = head;
	while(1)
	{
		prev_node = current_node;
		current_node = current_node->next;
		if(current_node == NULL)
			break;
		//if(current_node->n_line && str_comp_partial(current_node->s_line, equ_string))	//checks n_line to skip push/pop nodes
		if(!(current_node->s_label && current_node->s_label[0] == '@') && str_comp_partial(current_node->s_line, equ_string))
		{
			find_and_replace(current_node->next, current_node->s_label, current_node->s_line + 3);	//s_new may have trailing and leading spaces
			prev_node->next = current_node->next;
			free(current_node->s_label);
			free(current_node->s_line);
			free(current_node);
			current_node = prev_node;
		}
	}

	//build list of macros
	linked_line* prev_macro_line = NULL;
	linked_line* current_macro_line = NULL;
	current_node = head;
	while(1)
	{
		prev_node = current_node;
		current_node = current_node->next;
		if(current_node == NULL)
			break;
		if(current_node->s_label && str_comp_partial(current_node->s_label, macro_string))	//found start of a macro
		{
			//create new macro node
			current_macro->next = (linked_macro*)malloc(sizeof(linked_macro));
			current_macro = current_macro->next;
			current_macro->next = NULL;
			//fill line number and file fields
			current_macro->n_line = current_node->n_line;
			current_macro->name_index = current_node->name_index;
			//fill macro name and formal parameters fields
			current_macro->macro_name = current_node->s_line;
			char* formal_parameters = current_macro->macro_name;
			while(1)	//find first space after macro name to terminate the name string
			{
				if(*formal_parameters == 0x00)
				{
					formal_parameters = NULL;	//found null before any parameters...
					break;
				}
				if(*formal_parameters == ' ' || *formal_parameters == '\t')
				{
					*formal_parameters = 0x00;	//terminate macro name
					++formal_parameters;
					while(1)	//look for the start of the formal parameters
					{
						if(*formal_parameters == 0x00)
						{
							formal_parameters = NULL;	//no parameters
							break;
						}
						if(*formal_parameters != ' ' && *formal_parameters != '\t')	//found start of parameters
						{
							break;
						}
						++formal_parameters;
					}
					break;
				}
				++formal_parameters;
			}
			current_macro->formal_parameters = formal_parameters;
			//create dummy line head
			current_macro->line_head = (linked_line*)malloc(sizeof(linked_line));
			current_macro_line = current_macro->line_head;
			current_macro_line->name_index = 0;
			current_macro_line->s_label = NULL;
			current_macro_line->s_line = NULL;
			//create dummy push node
			current_macro_line->next = (linked_line*)malloc(sizeof(linked_line));
			current_macro_line = current_macro_line->next;
			current_macro_line->n_line = 0;
			current_macro_line->name_index = current_macro->name_index;
			current_macro_line->s_label = (char*)malloc(str_size(push_macro_string));
			current_macro_line->s_line = (char*)malloc(str_size(current_macro->macro_name) + str_size(name_table[current_macro->name_index]));
			strcpy(current_macro_line->s_label, push_macro_string);
			strcpy(current_macro_line->s_line, current_macro->macro_name);
			current_macro_line->s_line[strlen(current_macro->macro_name)] = ':';
			strcpy(current_macro_line->s_line + str_size(current_macro->macro_name), name_table[current_macro->name_index]);
			//attach lines to macro
			current_macro_line->next = current_node->next;
			//find end of macro
			while(1)
			{
				prev_macro_line = current_macro_line;
				current_macro_line = current_macro_line->next;
				if(current_macro_line == NULL)
				{
					printf("Could not find end of macro at line %lu in file %s\n", current_macro->n_line, name_table[current_macro->name_index]);
					exit(1);
				}
				if(current_macro_line->s_label && str_comp_partial(current_macro_line->s_label, endmacro_string))	//found end of macro
				{
					//detach macro from the main list
					prev_node->next = current_macro_line->next;
					//terminate macro with dummy pop node
					prev_macro_line->next = (linked_line*)malloc(sizeof(linked_line));
					prev_macro_line = prev_macro_line->next;
					prev_macro_line->next = NULL;
					prev_macro_line->n_line = 0;
					prev_macro_line->name_index = current_macro->name_index;
					prev_macro_line->s_label = (char*)malloc(str_size(pop_macro_string));
					prev_macro_line->s_line = (char*)malloc(str_size(current_macro->macro_name));
					strcpy(prev_macro_line->s_label, pop_macro_string);
					strcpy(prev_macro_line->s_line, current_macro->macro_name);
					//free macro start and end
					//for the macro start we still need the line and next, since those now belong to the macro
					free(current_node->s_label);
					free(current_node);
					current_node = prev_node;
					//for the macro end we only need the next, since it still forms part of the main list
					free(current_macro_line->s_label);
					free(current_macro_line->s_line);
					free(current_macro_line);
					break;
				}
			}
		}
	}

	//print macros
	if(debug_level >= 2)
	{
		current_macro = macro_head;
		while(1)
		{
			current_macro = current_macro->next;
			if(current_macro == NULL)
				break;
			printf("%lu ", current_macro->n_line);
			printf("MACRO ");
			printf("    %s ", current_macro->macro_name);
			printf("%s\n", current_macro->formal_parameters);
			current_node = current_macro->line_head;
			while(1)
			{
				current_node = current_node->next;
				if(current_node == NULL)
					break;
				printf("%lu    ", current_node->n_line);
				if(current_node->s_label)
					printf("%s", current_node->s_label);
				printf("    %s\n", current_node->s_line);
			}
		}
	}

	//expand macros
	linked_line* macro_copy_line_head = NULL;
	linked_line* current_macro_copy_line = NULL;
	current_macro = macro_head;
	while(1)
	{
		current_macro = current_macro->next;
		if(current_macro == NULL)
			break;
		char* macro_name = current_macro->macro_name;
		char* formal_params_copy = NULL;
		if(current_macro->formal_parameters)
			formal_params_copy = (char*)malloc(str_size(current_macro->formal_parameters));	//will need copy of macro params
		//find macro instances
		current_node = head;
		while(1)
		{
			prev_node = current_node;
			current_node = current_node->next;
			if(current_node == NULL)
				break;
			char* s_line = current_node->s_line;	//we can modify this only if it is a macro instance
			//if(current_node->n_line && str_comp_partial(s_line, macro_name))	//checks n_line to skip push/pop nodes
			if(!(current_node->s_label && current_node->s_label[0] == '@') && str_comp_partial(s_line, macro_name))
			{
				printf("Found instance of macro %s in line %lu of file %s\n", macro_name, current_node->n_line, name_table[current_node->name_index]);
				//create copy of macro code
				macro_copy_line_head = (linked_line*)malloc(sizeof(linked_line));
				current_macro_copy_line = macro_copy_line_head;
				current_macro_line = current_macro->line_head;
				while(1)
				{
					current_macro_line = current_macro_line->next;
					if(current_macro_line == NULL)
						break;
					//create new line
					current_macro_copy_line->next = (linked_line*)malloc(sizeof(linked_line));
					current_macro_copy_line = current_macro_copy_line->next;
					//copy line data
					current_macro_copy_line->s_line = (char*)malloc(str_size(current_macro_line->s_line));
					strcpy(current_macro_copy_line->s_line, current_macro_line->s_line);
					current_macro_copy_line->s_label = NULL;
					if(current_macro_line->s_label)
					{
						current_macro_copy_line->s_label = (char*)malloc(str_size(current_macro_line->s_label));
						strcpy(current_macro_copy_line->s_label, current_macro_line->s_label);
					}
					current_macro_copy_line->n_line = current_macro_line->n_line;
					current_macro_copy_line->name_index = current_macro_line->name_index;
					current_macro_copy_line->next = NULL;
				}
				//find and replace formal parameters
				if(current_macro->formal_parameters)	//the thing might not have any parameters
				{
					strcpy(formal_params_copy, current_macro->formal_parameters);
					remove_spaces(formal_params_copy);
					char* current_macro_param = formal_params_copy;	//this is what we want to replace
					char* current_macro_param_end = current_macro_param;
					char* current_instance_param = s_line;	//this is what we want to replace it with
					char* current_instance_param_end = current_instance_param;
					while(*current_instance_param_end)	//the first thing in the line is the macro name, skip
					{
						if(*current_instance_param_end == ' ')
						{
							*current_instance_param_end = 0x00;
							++current_instance_param_end;
							break;
						}
						++current_instance_param_end;
					}
					current_instance_param = current_instance_param_end;
					while(*current_macro_param && *current_instance_param)
					{
						//find end of current_macro_param
						while(*current_macro_param_end)
						{
							if(*current_macro_param_end == ',')
							{
								*current_macro_param_end = 0x00;
								++current_macro_param_end;
								break;
							}
							++current_macro_param_end;
						}
						while(*current_instance_param_end)
						{
							if(*current_instance_param_end == ',')
							{
								*current_instance_param_end = 0x00;
								++current_instance_param_end;
								break;
							}
							++current_instance_param_end;
						}
						find_and_replace(macro_copy_line_head->next, current_macro_param, current_instance_param);
						current_instance_param = current_instance_param_end;	//advance to next instance param
						current_macro_param = current_macro_param_end;	//advance to next macro param (if any)
					}
					//check that instance has the right number of params
					if(*current_macro_param || *current_instance_param)
					{
						printf("Warning: Macro instance at line %lu in file %s does not match macro definition.\n", current_node->n_line, name_table[current_node->name_index]);
					}
				}
				//attach macro copy to main list
				prev_node->next = macro_copy_line_head->next;
				current_macro_copy_line->next = current_node->next;
				//free copy line head
				free(macro_copy_line_head);
				//keep macro reference label (if any)
				if(current_node->s_label)
				{
					if(!prev_node->next->next->s_label)
						prev_node->next->next->s_label = current_node->s_label;
					else
						free(current_node->s_label);
				}
				//set macro reference line number
				prev_node->next->n_line = current_node->n_line;
				//free current node
				free(current_node->s_line);
				free(current_node);
				current_node = prev_node;
			}
		}
		//free formal_params_copy
		free(formal_params_copy);
	}

	//free macros
	current_macro = macro_head;
	free_macro(current_macro);

	//free name table
	for(unsigned int d = 0; d < 256; ++d)
	{
		if(name_table[d] == NULL)
			break;
		free(name_table[d]);
	}

	//print lines
	/*if(debug_enable)
	{
		current_node = head;
		while(1)
		{
			current_node = current_node->next;
			if(current_node == NULL)
				break;
			printf("%lu ", current_node->n_line);
			if(current_node->s_label)
				printf("%s", current_node->s_label);
			printf("    %s\n", current_node->s_line);
		}
	}*/

	//write lst file	
	unsigned long prev_line = 0;
	// FILE *f = fopen(argv[lst_index], "w");
	FILE *f = fopen(lst_name, "w");
	if(f == NULL)
	{
		// printf("Could not open output file: %s\n", argv[lst_index]);
		printf("Could not create LST output file: %s\n", lst_name);
		exit(1);
	}

	current_node = head;
	while(1)
	{
		current_node = current_node->next;
		if(current_node == NULL)
			break;

		if(current_node->n_line == 0)
			prev_line = 0;
		else if(++prev_line != current_node->n_line)
		{
			fprintf(f, "@LINE: %lu\n", current_node->n_line);
			prev_line = current_node->n_line;
		}

		if(current_node->s_label)
			fprintf(f, "%s", current_node->s_label);
		fprintf(f, "    %s\n", current_node->s_line);
	}

	fclose(f);

	//free memory
	current_node = head;
	free_node(current_node);
	return 0;
}

void load_file(char* file_name, linked_line* head)
{
	printf("Loading file %s\n", file_name);
	linked_line* current_node = head;
	unsigned int line_start = 0;
	unsigned int line_end = 0;
	unsigned int label_end;
	unsigned long current_source_line = 0;
	char* floating_label = (char*)malloc(sizeof(char) * 128);
	unsigned int floating_label_size = 0;
	char* current_line = (char*)malloc(sizeof(char) * 256);
	uint8_t name_index = 0;
	
	while(name_table[name_index])
	{
		++name_index;
	}
	name_table[name_index] = (char*)malloc(str_size(file_name));
	strcpy(name_table[name_index], file_name);

	FILE* fp_in = fopen(file_name, "r");
	if(fp_in == NULL)
	{
		printf("Failed to load source file: %s\n", file_name);
		exit(1);
	}

	current_node->next = (linked_line*)malloc(sizeof(linked_line));
	current_node = current_node->next;
	current_node->next = NULL;
	current_node->n_line = 0;
	current_node->name_index = name_index;
	current_node->s_label = (char*)malloc(str_size(push_file_string));
	current_node->s_line = (char*)malloc(str_size(file_name));
	strcpy(current_node->s_label, push_file_string);
	strcpy(current_node->s_line, file_name);

	while (fgets(current_line, 256, fp_in))
	{
		++current_source_line;
		to_caps(current_line);

		if((current_line[0] != ';') && (current_line[0] != 0x0A) && (current_line[0] != 0x0D) && (current_line[0] != 0x00))
		{
			if((current_line[0] >= 'A') && (current_line[0] <= 'Z'))	//alpha char
			{
				//line contains a label
				//extract label
				label_end = 0;
				while((current_line[label_end] != ' ') && (current_line[label_end] != 0x0a) && (current_line[label_end] != '\t') && (current_line[label_end] != 0x0d) && (current_line[label_end] != 0x00))
				{
					++label_end;
					if(label_end == 128)
					{
						printf("syntax error in line %lu in file %s\n", current_source_line, name_table[name_index]);
						exit(1);
					}
				}
				//extract rest of the line
				line_start = label_end;
				line_end = 0;
				//find line start
				while(((current_line[line_start] < 'A') || (current_line[line_start] > 'Z')) && (current_line[line_start] != 0x22))
				{
					if((current_line[line_start] == ';') || (current_line[line_start] == 0x0a) || (current_line[line_start] == 0x0D) || (current_line[line_start] == 0x00))
					{
						//line is only a label
						line_start = 0;
						break;
					}
					++line_start;
					if(line_start == 256)
					{
						printf("syntax error in line %lu in file %s\n", current_source_line, name_table[name_index]);
						exit(1);
					}
				}
				if(line_start == 0)
				{
					//line is only a label
					for(unsigned int i = 0; i < label_end; ++i)
					{
						floating_label[i] = current_line[i];
					}
					floating_label[label_end] = 0x00;
					floating_label_size = label_end + 1;
				}
				else
				{
					//line has code
					//find line end
					line_end = line_start;
					while((current_line[line_end] != ';') && (current_line[line_end] != 0x0a) && (current_line[line_end] != 0x0D) && (current_line[line_end] != 0x00))
					{
						++line_end;
						if(line_end == 256)
						{
							printf("syntax error in line %lu in file %s\n", current_source_line, name_table[name_index]);
							exit(1);
						}
					}
					//create new node
					floating_label_size = 0;	//this line has its own label
					current_node->next = (linked_line*)malloc(sizeof(linked_line));
					current_node = current_node->next;
					current_node->next = NULL;
					current_node->n_line = current_source_line;
					current_node->name_index = name_index;
					current_node->s_label = (char*)malloc(sizeof(char) * (label_end + 1));
					current_node->s_line = (char*)malloc(sizeof(char) * (line_end + 1 - line_start));
					for(unsigned int i = 0; i < label_end; ++i)
					{
						current_node->s_label[i] = current_line[i];
					}
					current_node->s_label[label_end] = 0x00;
					for(unsigned int i = 0; i < (line_end - line_start); ++i)
					{
						current_node->s_line[i] = current_line[i + line_start];
					}
					current_node->s_line[line_end - line_start] = 0x00;
				}
			}
			else if((current_line[0] == 0x20) || (current_line[0] == 0x09))
			{
				//line has no label
				line_start = 0;
				line_end = 0;
				//find line start
				while((current_line[line_start] < 0x41) || (current_line[line_start] > 0x5A))
				{
					if((current_line[line_start] == ';') || (current_line[line_start] == 0x0a) || (current_line[line_start] == 0x0D) || (current_line[line_start] == 0x00))
					{
						//line is empty
						line_start = 0;
						break;
					}
					++line_start;
					if(line_start == 256)
					{
						printf("syntax error in line %lu in file %s\n", current_source_line, name_table[name_index]);
						exit(1);
					}
				}
				if(line_start != 0)
				{
					//line is not empty
					//find line end
					line_end = line_start;
					while((current_line[line_end] != ';') && (current_line[line_end] != 0x0a) && (current_line[line_end] != 0x0D) && (current_line[line_end] != 0x00))
					{
						++line_end;
						if(line_end == 256)
						{
							printf("syntax error in line %lu in file %s\n", current_source_line, name_table[name_index]);
							exit(1);
						}
					}
					//create new node
					current_node->next = (linked_line*)malloc(sizeof(linked_line));
					current_node = current_node->next;
					current_node->next = NULL;
					current_node->n_line = current_source_line;
					current_node->name_index = name_index;
					if(floating_label_size)
					{
						current_node->s_label = (char*)malloc(sizeof(char) * floating_label_size);
						for(unsigned int i = 0; i < floating_label_size; ++i)
						{
							current_node->s_label[i] = floating_label[i];
						}
					}
					else
						current_node->s_label = NULL;
					current_node->s_line = (char*)malloc(sizeof(char) * (line_end + 1 - line_start));
					for(unsigned int i = 0; i < (line_end - line_start); ++i)
					{
						current_node->s_line[i] = current_line[i + line_start];
					}
					current_node->s_line[line_end - line_start] = 0x00;
					floating_label_size = 0;	//floating label has been used
				}
			}
			else
			{
				printf("syntax error in line %lu in file %s\n", current_source_line, name_table[name_index]);
				exit(1);
			}
		}
	}

	current_node->next = (linked_line*)malloc(sizeof(linked_line));
	current_node = current_node->next;
	current_node->next = NULL;
	current_node->n_line = 0;
	current_node->name_index = name_index;
	current_node->s_label = (char*)malloc(str_size(pop_file_string));
	current_node->s_line = (char*)malloc(str_size(file_name));
	strcpy(current_node->s_label, pop_file_string);
	strcpy(current_node->s_line, file_name);

	free(floating_label);
	free(current_line);
	fclose(fp_in);
}

void include_merge(linked_line* prev_node, linked_line* include_node, linked_line* new_head)
{
	linked_line* current_node = new_head;
	prev_node->next = new_head->next;
	while(1)
	{
		if(current_node->next == NULL)
		{
			current_node->next = include_node->next;
			break;
		}
		current_node = current_node->next;
	}
}

inline void free_node(linked_line* current_node)
{
	linked_line* next_node = NULL;
	while(1)
	{
		if(current_node == NULL)
			break;
		next_node = current_node->next;
		if(current_node->s_label)
			free(current_node->s_label);
		if(current_node->s_line)
			free(current_node->s_line);
		free(current_node);
		current_node = next_node;
	}
}

inline void free_macro(linked_macro* current_macro)
{
	linked_macro* next_macro = NULL;
	while(1)
	{
		if(current_macro == NULL)
			break;
		next_macro = current_macro->next;
		free_node(current_macro->line_head);
		free(current_macro->macro_name);	//this also frees the formal parameters
		free(current_macro);
		current_macro = next_macro;
	}
}

int str_comp_partial(const char* str1, const char* str2)
{
	for(int i = 0; str1[i] && str2[i]; ++i)
	{
		if(str1[i] != str2[i])
			return 0;
	}
	return 1;
}

inline void find_and_replace(linked_line* current_node, char* s_replace, char* s_new)
{
	unsigned int word_start;
	unsigned int word_end;
	remove_spaces(s_new);
	while(1)
	{
		if(current_node == NULL)
			break;
		if(current_node->s_label)	//replace labels
			if(str_find_word(current_node->s_label, s_replace, &word_start, &word_end))
				str_replace(&(current_node->s_label), s_new, word_start, word_end);
		while(str_find_word(current_node->s_line, s_replace, &word_start, &word_end))	//replace operands and mnemonics
		{
			str_replace(&(current_node->s_line), s_new, word_start, word_end);
		}
		current_node = current_node->next;
	}
}

inline int str_find_word(char* where, char* what, unsigned int* start, unsigned int* end)
{
	unsigned int what_size = str_size(what);
	unsigned int where_size = str_size(where);
	if(what_size > where_size)
		return 0;
	for(unsigned int offset = 0; offset <= (where_size - what_size); ++offset)
	{
		for(unsigned int d = 0; d < what_size; ++d)
		{
			if(!what[d])
			{
				if(where[offset + d] && !(where[offset + d] == 0x09 || where[offset + d] == 0x20 || where[offset + d] == 0x2C || where[offset + d] == 0x28 || where[offset + d] == 0x29))
					break;
				if(offset)
					if(!(where[offset - 1] == 0x09 || where[offset - 1] == 0x20 || where[offset - 1] == 0x2C || where[offset - 1] == 0x28 || where[offset - 1] == 0x29))
						break;
				*start = offset;
				*end = offset + d;
				return 1;
			}
			if(where[offset + d] != what[d])
				break;
		}
	}
	return 0;
}

inline void str_replace(char** where, char* s_new, unsigned int word_start, unsigned int word_end)
{
	char* old_string = *where;
	unsigned int old_size = str_size(*where);
	unsigned int pre_length = word_start;
	unsigned int word_length = (unsigned int)strlen(s_new);
	unsigned int post_size = old_size - word_end;
	unsigned int new_size = pre_length + word_length + post_size;
	char* new_string = (char*)malloc(new_size);
	unsigned int offset = 0;
	for(unsigned int d = 0; d < pre_length; ++d)
	{
		new_string[offset] = old_string[d];
		++offset;
	}
	for(unsigned int d = 0; d < word_length; ++d)
	{
		new_string[offset] = s_new[d];
		++offset;
	}
	for(unsigned int d = word_end; d < old_size; ++d)
	{
		new_string[offset] = old_string[d];
		++offset;
	}
	free(*where);
	*where = new_string;
}

unsigned int str_size(const char* s_input)
{
	unsigned int size = 0;
	for(unsigned int d = 0; 1; ++d)
	{
		++size;
		if(s_input[d] == 0x00)
			break;
	}
	return size;
}
