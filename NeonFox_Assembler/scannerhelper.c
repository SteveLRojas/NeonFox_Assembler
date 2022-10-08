#include <stdio.h> 
#include <stdlib.h> 
#include <stdint.h>
#include <string.h>
#include "scannerhelper.h"


////////////////////////////
// Scanner global helpers //
////////////////////////////
uint32_t g_label_count;
uint64_t g_curr_word_address;

void set_org(char* s, int radix)
{
	while((*s < 'A' || *s > 'F') && (*s < '0' || *s > '9'))
	{
		++s;
	}

	g_curr_word_address = (uint64_t)strtol(s, NULL, radix);

   // Update file/macro line number
   if(is_macro_stack_empty()){
      peek_file()->n_source_line++;
   } else {
      peek_macro()->n_macro_line++;
   }
}


////////////////////
// String helpers //
////////////////////
const char* mnemonics[] = {"ADD", "ADDC", "SUB", "SUBC", "MOVE", "TEST", "NOT", "ROR", "ROL", "AND", "XOR", "OR", "CALL", "CALLX", "CALLL", "CALLLX", "RET", "RETX", "RETL", "RETLX", "JMP", "JMPL", "NOP", "BRZ", "BRN", "BRP", "BRA", "BRNZ", "BRNN", "BRNP", "LIM", "BITT", "DATA", "ORG"};

void to_caps(char* str)
{
   unsigned int d = 0;
   char prev_char = 0x00;
   while(str[d])
   {
      //do not modifiy substrings that appear in quotes
      if(str[d] == 0x22)   //double quotes
      {
         while(1)
         {
            ++d;
            if(!str[d])
               return;
            if(str[d] == 0x22 && prev_char != 0x5C)
               break;
            prev_char = str[d];
         }
      }
      if(str[d] == 0x27)   //single quotes
      {
         while(1)
         {
            ++d;
            if(!str[d])
               return;
            if(str[d] == 0x27 && prev_char != 0x5C)
               break;
            prev_char = str[d];
         }
      }
      if(str[d] == ';') //skip comments
         return;
      if((str[d] > 0x60) & (str[d] < 0x7B))
         str[d] = str[d] - 0x20;
      prev_char = str[d];
      ++d;
   }
   return;
}

void remove_spaces(char* s)
{
   char* d = s;
   char prev_char = *d;
   do
   {
      //do not modify substrings inside quotes
      if(prev_char == 0x22)   //double quates
      {
         while(1)
         {
            prev_char = (*s++ = *d++);
            if(!prev_char)
               return;
            if(*d == 0x22 && prev_char != 0x5C)
               break;
         }
         prev_char = (*s++ = *d++);
      }
      if(prev_char == 0x27)   //single quates
      {
         while(1)
         {
            prev_char = (*s++ = *d++);
            if(!prev_char)
               return;
            if(*d == 0x27 && prev_char != 0x5C)
               break;
         }
         prev_char = (*s++ = *d++);
      }
      while (*d == ' ' || *d == '\t')
      {
         ++d;
      }
      if(*d == ';')
      {
         *d = '\0';
      }
      prev_char = (*s++ = *d++);
   } while (prev_char);
}

inline char* str_or_null(char* s)
{
   return s ? s : "NULL";
}

inline const char* str_for_mnemonic(uint8_t mnemonicIdx)
{
   return (mnemonicIdx == (uint8_t)(-1)) ? "NONE" : mnemonics[mnemonicIdx];
}


////////////////////
// Stack helpers  //
////////////////////
active_file* file_stack;
uint32_t file_sp;

active_macro* macro_stack;
uint32_t macro_sp;

void init_file_stack(uint32_t size)
{
   file_stack = (active_file*)malloc(sizeof(active_file) * size);
   file_sp = (uint32_t)(-1);
}

void init_macro_stack(uint32_t size)
{
   macro_stack = (active_macro*)malloc(sizeof(active_macro) * size);
   macro_sp = (uint32_t)(-1);
}

void push_file(char* name)
{
   ++file_sp;
   file_stack[file_sp].s_source_file = strdup(name);
   remove_spaces(file_stack[file_sp].s_source_file);
   to_caps(file_stack[file_sp].s_source_file);

   // Remove trailing newline/CRLF
   if(file_stack[file_sp].s_source_file[strlen(file_stack[file_sp].s_source_file) - 2] == '\r')
      file_stack[file_sp].s_source_file[strlen(file_stack[file_sp].s_source_file) - 2] = '\0';
   else
      file_stack[file_sp].s_source_file[strlen(file_stack[file_sp].s_source_file) - 1] = '\0';

   file_stack[file_sp].n_source_line = 1;
}

void push_macro_ref(char* name, char* file)
{
   ++macro_sp;
   macro_stack[macro_sp].s_macro_name = strdup(name);
   remove_spaces(macro_stack[macro_sp].s_macro_name);
   to_caps(macro_stack[macro_sp].s_macro_name);

   macro_stack[macro_sp].s_macro_file = strdup(file);
   remove_spaces(macro_stack[macro_sp].s_macro_file);
   to_caps(macro_stack[macro_sp].s_macro_file);

   // Remove trailing newline/CRLF
   if(macro_stack[macro_sp].s_macro_file[strlen(macro_stack[macro_sp].s_macro_file) - 2] == '\r')
      macro_stack[macro_sp].s_macro_file[strlen(macro_stack[macro_sp].s_macro_file) - 2] = '\0';
   else
      macro_stack[macro_sp].s_macro_file[strlen(macro_stack[macro_sp].s_macro_file) - 1] = '\0';

   macro_stack[macro_sp].n_macro_line = 1;
}

void pop_file()
{
   free(file_stack[file_sp].s_source_file);
   --file_sp;
}

void pop_macro()
{
   free(macro_stack[macro_sp].s_macro_name);
   free(macro_stack[macro_sp].s_macro_file);
   --macro_sp;
}

inline active_file* peek_file()
{
   return file_stack + file_sp;
}

inline active_macro* peek_macro()
{
   return macro_stack + macro_sp;
}

inline int is_file_stack_empty()
{
   return file_sp == (uint32_t)(-1);
}

inline int is_macro_stack_empty()
{
   return macro_sp == (uint32_t)(-1);
}


//////////////////////
// Molecule helpers //
//////////////////////
const char* FieldNames[] = {"Label", "Mnemonic Index", "Target Byte Sel", "Literal Sel", "Literal", "First Register", "Second Register", "Label Reference"};

char* 	s_curr_label;	
uint8_t 	curr_mnemonic_index;		
uint8_t 	curr_target_byte_sel;
uint8_t 	curr_literal_sel;
uint32_t curr_literal;
uint8_t 	curr_reg_first;
uint8_t 	curr_reg_second;
char* 	s_curr_label_ref;
uint8_t  curr_token_count;

uint8_t has_mnemonic_index;
uint8_t has_target_byte_sel;
uint8_t has_literal_sel;
uint8_t has_literal;
uint8_t has_reg_first;
uint8_t has_reg_second;

molecule* molecule_head;
molecule* current_molecule;

void mol_set_mnemonic_index(uint8_t mnemonic_index)
{
   if(has_mnemonic_index)
   {
      mol_err_dup_field(FieldMnemonic);
   }

   curr_mnemonic_index = mnemonic_index;
   has_mnemonic_index = 1;
   mol_inc_token_cnt();
}

void mol_set_target_byte_sel(uint8_t target_byte_sel)
{
   if(has_target_byte_sel)
   {
      mol_err_dup_field(FieldTargetByteSel);
   }

   curr_target_byte_sel = target_byte_sel;
   has_target_byte_sel = 1;
   mol_inc_token_cnt();
}

void mol_set_literal_sel(uint8_t literal_sel)
{
   if(has_literal_sel)
   {
      mol_err_dup_field(FieldLiteralSel);
   }

   curr_literal_sel = literal_sel;
   has_literal_sel = 1;
   mol_inc_token_cnt();
}

void mol_set_literal(char* literal, int radix)
{
   if(has_literal)
   {
      mol_err_dup_field(FieldLiteral);
   }

   has_literal = 1;
   mol_inc_token_cnt();

   // handle char
   if(literal[0] == '\'')
   {
      if(literal[1] == '\\') // is escaped char
      {
         if(literal[2] == 't' || literal[2] == 'T')
         {
            curr_literal = '\t';
            return;
         }
         else if(literal[2] == 'n' || literal[2] == 'N')
         {
            curr_literal = '\n';
            return;
         }
         else if(literal[2] == '\\')
         {
            curr_literal = '\\';
            return;
         }
         else if(literal[2] == '0')
         {
            curr_literal = '\0';
            return;
         }
         else
         {
            fprintf(stderr, "literal [%s] did not match immediate char syntax at line: %lu in file %s\n", 
               literal,
               peek_file()->n_source_line,
               peek_file()->s_source_file);
            exit(1);
         }  
      }
      else  //is not escaped char
      {
         curr_literal = (uint32_t)literal[1];
         return;
      }
   }

   // stuff that is not a char
   curr_literal = (uint32_t)strtol(literal, NULL, radix);
   return;
}

void mol_set_reg(char* reg)
{
   if(!has_reg_first)
   {
      mol_set_reg_first(reg);
   }
   else if(!has_reg_second)
   {
      mol_set_reg_second(reg);
   }
   else
   {
      mol_err_dup_field(FieldRegSecond);
   }
}

inline void mol_set_reg_first(char* reg_first)
{
   curr_reg_first = (uint8_t)strtol(reg_first, NULL, 10);
   has_reg_first = 1;
   mol_inc_token_cnt();
}

inline void mol_set_reg_second(char* reg_second)
{
   curr_reg_second = (uint8_t)strtol(reg_second, NULL, 10);
   has_reg_second = 1;
   mol_inc_token_cnt();
}

void mol_set_identifier(char* identifier)
{
   if (curr_token_count == 0)
   {
      // label
      if(s_curr_label)
      {
         mol_err_dup_field(FieldLabel);
      }

      s_curr_label = strdup(identifier);
      to_caps(s_curr_label);
   }
   else // gt 0
   {
      // label reference
      if(s_curr_label_ref)
      {
         mol_err_dup_field(FieldLabelRef);
      }

      s_curr_label_ref = strdup(identifier);
      to_caps(s_curr_label_ref);
   }

   mol_inc_token_cnt();
}

void mol_handle_delim(char* s)
{
   mol_inc_token_cnt();
}

inline void mol_inc_token_cnt()
{
   curr_token_count++;
}

void mol_handle_unexpected(char* token)
{
   fprintf(stderr, "\nUnexpected char \'%s\' at line: %lu in file %s!\n", 
      token,
      peek_file()->n_source_line, 
      peek_file()->s_source_file);
   exit(1);
}

void mol_invalidate_current()
{
   s_curr_label = NULL;
   curr_mnemonic_index = (uint8_t)(-1);
   curr_target_byte_sel = (uint8_t)(-1);
   curr_literal_sel = (uint8_t)(-1);
   curr_literal = (uint32_t)(-1);
   curr_reg_first = (uint8_t)(-1);
   curr_reg_second = (uint8_t)(-1);
   s_curr_label_ref = NULL;

   has_mnemonic_index = 0;
   has_target_byte_sel = 0;
   has_literal_sel = 0;
   has_literal = 0;
   has_reg_first = 0;
   has_reg_second = 0;

   curr_token_count = 0;
}

void mol_pregen_checks()
{
   // All-instruction requirements
   if(!has_mnemonic_index)
   {
      mol_err_req_field(FieldMnemonic);
   }

   // Instruction-specific requirements
   switch(curr_mnemonic_index)
   {
      case 0:     // ADD
         mol_valid_and_fields((const enum Field[]){FieldTargetByteSel, FieldRegFirst, FieldRegSecond}, 3);
         break;
      case 1:     // ADDC
         mol_valid_and_fields((const enum Field[]){FieldTargetByteSel, FieldRegFirst, FieldRegSecond}, 3);
         break;
      case 2:     // SUB
         mol_valid_and_fields((const enum Field[]){FieldTargetByteSel, FieldRegFirst, FieldRegSecond}, 3);
         break;
      case 3:     // SUBC
         mol_valid_and_fields((const enum Field[]){FieldTargetByteSel, FieldRegFirst, FieldRegSecond}, 3);
         break;
      case 4:     // MOVE
         mol_valid_and_fields((const enum Field[]){FieldTargetByteSel, FieldRegFirst, FieldRegSecond}, 3);
         break;
      case 5:     // TEST
         mol_valid_and_fields((const enum Field[]){FieldTargetByteSel, FieldRegFirst}, 2);
         break;
      case 6:     // NOT
         mol_valid_and_fields((const enum Field[]){FieldTargetByteSel, FieldRegFirst, FieldRegSecond}, 3);
         break;
      case 7:     // ROR
         mol_valid_and_fields((const enum Field[]){FieldTargetByteSel, FieldRegFirst, FieldRegSecond}, 3);
         break;
      case 8:     // ROL
         mol_valid_and_fields((const enum Field[]){FieldTargetByteSel, FieldRegFirst, FieldRegSecond}, 3);
         break;
      case 9:     // AND
         mol_valid_and_fields((const enum Field[]){FieldTargetByteSel, FieldRegFirst, FieldRegSecond}, 3);
         break;
      case 10:    // XOR
         mol_valid_and_fields((const enum Field[]){FieldTargetByteSel, FieldRegFirst, FieldRegSecond}, 3);
         break;
      case 11:    // OR
         mol_valid_and_fields((const enum Field[]){FieldTargetByteSel, FieldRegFirst, FieldRegSecond}, 3);
         break;
      case 12:    // CALL 
         break;
      case 13:    // CALLX
         break;
      case 14:    // CALLL
         // mol_valid_and_fields((const enum Field[]){FieldLiteralSel}, 1);
         mol_valid_xor_fields((const enum Field[]){FieldLabelRef, FieldLiteral}, 2);
         break;
      case 15:    // CALLLX
         // mol_valid_and_fields((const enum Field[]){FieldLiteralSel}, 1);
         mol_valid_xor_fields((const enum Field[]){FieldLabelRef, FieldLiteral}, 2);
         break;
      case 16:    // RET 
         break;
      case 17:    // RETX
         break;
      case 18:    // RETL 
         // mol_valid_and_fields((const enum Field[]){FieldLiteralSel}, 1);
         mol_valid_xor_fields((const enum Field[]){FieldLabelRef, FieldLiteral}, 2);
         break;
      case 19:    // RETLX
         // mol_valid_and_fields((const enum Field[]){FieldLiteralSel}, 1);
         mol_valid_xor_fields((const enum Field[]){FieldLabelRef, FieldLiteral}, 2);
         break;
      case 20:    // JMP
         break;
      case 21:    // JMPL
         // mol_valid_and_fields((const enum Field[]){FieldLiteralSel}, 1);
         mol_valid_xor_fields((const enum Field[]){FieldLabelRef, FieldLiteral}, 2);
         break;
      case 22:    // NOP 
         break;
      case 23:    // BRZ
         mol_valid_xor_fields((const enum Field[]){FieldLabelRef, FieldLiteral}, 2);
         break;
      case 24:    // BRN
         mol_valid_xor_fields((const enum Field[]){FieldLabelRef, FieldLiteral}, 2);
         break;
      case 25:    // BRP 
         mol_valid_xor_fields((const enum Field[]){FieldLabelRef, FieldLiteral}, 2);
         break;
      case 26:    // BRA
         mol_valid_xor_fields((const enum Field[]){FieldLabelRef, FieldLiteral}, 2);
         break;
      case 27:    // BRNZ 
         mol_valid_xor_fields((const enum Field[]){FieldLabelRef, FieldLiteral}, 2);
         break;
      case 28:    // BRNN
         mol_valid_xor_fields((const enum Field[]){FieldLabelRef, FieldLiteral}, 2);
         break;
      case 29:    // BRNP 
         mol_valid_xor_fields((const enum Field[]){FieldLabelRef, FieldLiteral}, 2);
         break;
      case 30:    // LIM 
         mol_valid_and_fields((const enum Field[]){FieldTargetByteSel, FieldRegFirst}, 2);
         mol_valid_xor_fields((const enum Field[]){FieldLabelRef, FieldLiteral}, 2);
         break;
      case 31:    // BITT 
         mol_valid_and_fields((const enum Field[]){FieldTargetByteSel, FieldRegFirst}, 2);
         mol_valid_xor_fields((const enum Field[]){FieldLabelRef, FieldLiteral}, 2);
         break;
      case 32:    // DATA
         mol_valid_xor_fields((const enum Field[]){FieldLabelRef, FieldLiteral}, 2);
         break;
      default:
         // p_error(current_molecule);
         break;
   }
}

void mol_generate()
{
   // Validate our fields before generating
   mol_pregen_checks();

	// Update scanner globals
	if(s_curr_label)
	{
		g_label_count++;
	}

	// Add new molecule to end of chain
	current_molecule->next = (molecule*)malloc(sizeof(molecule));
	current_molecule = current_molecule->next;
	current_molecule->next = NULL;

	// Copy current values to new molecule
	current_molecule->word_address 	= g_curr_word_address++; // Increment address after assign
	current_molecule->s_label = s_curr_label;
	current_molecule->mnemonic_index = curr_mnemonic_index;
	current_molecule->target_byte_sel = curr_target_byte_sel;
	current_molecule->literal_sel = curr_literal_sel;
	current_molecule->literal = curr_literal;
	current_molecule->reg_first = curr_reg_first;
	current_molecule->reg_second = curr_reg_second;
	current_molecule->s_label_ref = s_curr_label_ref;
	current_molecule->s_source_file = strdup(peek_file()->s_source_file);
	current_molecule->n_source_line = peek_file()->n_source_line;

	if(!is_macro_stack_empty())
	{
		current_molecule->s_macro_name = strdup(peek_macro()->s_macro_name);
		current_molecule->s_macro_file = strdup(peek_macro()->s_macro_file);
		current_molecule->n_macro_line = peek_macro()->n_macro_line;  
	}
	else
	{
		current_molecule->s_macro_name = NULL;
		current_molecule->s_macro_file = NULL;
      current_molecule->n_macro_line = 0;
	}

	// Clear current values
	mol_invalidate_current();

   // Update file/macro line number
   if(is_macro_stack_empty()){
      peek_file()->n_source_line++;
   } else {
      peek_macro()->n_macro_line++;
   }
}

void mol_print_debug(molecule* mol)
{
   printf("MOL @ %s:%lu\n", str_or_null(mol->s_source_file), mol->n_source_line);
   printf("    address:         %llu\n", mol->word_address);
   printf("    s_label:         %s\n", str_or_null(mol->s_label));
   printf("    mnemonic_index:  %u (%s)\n", mol->mnemonic_index, str_for_mnemonic(mol->mnemonic_index));
   printf("    target_byte_sel: %u\n", mol->target_byte_sel);
   printf("    literal_sel:     %u\n", mol->literal_sel);
   printf("    literal:         %u\n", mol->literal);
   printf("    reg_first:       %u\n", mol->reg_first);
   printf("    reg_second:      %u\n", mol->reg_second);
   printf("    s_label_ref:     %s\n", str_or_null(mol->s_label_ref));

   printf("    s_macro_name:    %s\n", str_or_null(mol->s_macro_name));
   printf("    s_macro_file:    %s\n", str_or_null(mol->s_macro_file));
   printf("    n_macro_line:    %lu\n", mol->n_macro_line);
   printf("\n");
}

void mol_valid_and_fields(const enum Field* fields, uint8_t n_fields)
{
   for(uint8_t i = 0; i < n_fields; i++)
   {
      if(!mol_valid_field_present(fields[i])) 
         mol_err_req_field(fields[i]);
   }
}

void mol_valid_or_fields(const enum Field* fields, uint8_t n_fields)
{
   for(uint8_t i = 0; i < n_fields; i++)
   {
      if(mol_valid_field_present(fields[i])) 
         return;
   }
   mol_err_invalid_fields(fields, n_fields);
}

void mol_valid_xor_fields(const enum Field* fields, uint8_t n_fields)
{
   uint8_t xor = 0;

   for(uint8_t i = 0; i < n_fields; i++)
   {
      xor = xor ^ mol_valid_field_present(fields[i]);
   }
 
   if(!xor)
      mol_err_invalid_fields(fields, n_fields);
}

uint8_t mol_valid_field_present(enum Field field)
{
   switch(field)
   {
      case FieldLabel:        return (s_curr_label ? 1 : 0);
      case FieldMnemonic:     return has_mnemonic_index;
      case FieldTargetByteSel:return has_target_byte_sel;
      case FieldLiteralSel:   return has_literal_sel;
      case FieldLiteral:      return has_literal;
      case FieldRegFirst:     return has_reg_first;
      case FieldRegSecond:    return has_reg_second;
      case FieldLabelRef:     return (s_curr_label_ref ? 1 : 0);
      default:
         fprintf(stderr, "mol_valid_req_fields could not match the Field somehow");
         exit(1);
   }
}

void mol_err_dup_field(enum Field field)
{
   fprintf(stderr, "Attempting to set \'%s\' for molecule that already has one! Line: %lu in file %s\n", 
      FieldNames[field],
      peek_file()->n_source_line, 
      peek_file()->s_source_file);

   exit(1);
}

void mol_err_req_field(enum Field field)
{
   fprintf(stderr, "Missing \'%s\' for molecule! Line: %lu in file %s\n", 
      FieldNames[field],
      peek_file()->n_source_line, 
      peek_file()->s_source_file);

   exit(1);
}

void mol_err_invalid_fields(const enum Field* fields, uint8_t n_fields)
{
   fprintf(stderr, "\nInvalid Molecule State - Missing/Extraneous Fields:\n");
   
   for(uint8_t i = 0; i < n_fields; i++)
   {
      fprintf(stderr, "\t%s\n", FieldNames[fields[i]]);
   }

   fprintf(stderr, "Line: %lu in file %s\n", 
      peek_file()->n_source_line, 
      peek_file()->s_source_file);

   exit(1); 
}