#include <stdio.h> 
#include <stdlib.h> 
#include <stdint.h>
#include <string.h>
#include "preprocessor.h"

extern int yylex (void);
extern FILE *yyin, *yyout;

//Represents one assembled instruction.
typedef struct LINKED_INSTRUCTION
{
	uint64_t word_address;
	uint8_t instruction_high;
	uint8_t instruction_low;
	struct LINKED_INSTRUCTION* next;
} linked_instruction;



const char debug_string[] = "-DEBUG";
const char asm_string[] = "-ASM";
const char lst_string[] = "-LST";
const char bin_string[] = "-BIN";
const char coe_string[] = "-COE";
const char mif_string[] = "-MIF";

uint8_t debug_enable;
unsigned int asm_index = 0;
unsigned int bin_index = 0;
unsigned int coe_index = 0;
unsigned int mif_index = 0;

//Machine code generation functions
void gen_machine_instruction(molecule* current_molecule, linked_instruction* current_instruction);
void m_add  (molecule* current_molecule, linked_instruction* current_instruction);
void m_addc (molecule* current_molecule, linked_instruction* current_instruction);
void m_sub  (molecule* current_molecule, linked_instruction* current_instruction);
void m_subc (molecule* current_molecule, linked_instruction* current_instruction);
void m_move (molecule* current_molecule, linked_instruction* current_instruction);
void m_test (molecule* current_molecule, linked_instruction* current_instruction);
void m_not  (molecule* current_molecule, linked_instruction* current_instruction);
void m_ror  (molecule* current_molecule, linked_instruction* current_instruction);
void m_rol  (molecule* current_molecule, linked_instruction* current_instruction);
void m_and  (molecule* current_molecule, linked_instruction* current_instruction);
void m_xor  (molecule* current_molecule, linked_instruction* current_instruction);
void m_or   (molecule* current_molecule, linked_instruction* current_instruction);
void m_call (molecule* current_molecule, linked_instruction* current_instruction);
void m_calll(molecule* current_molecule, linked_instruction* current_instruction);
void m_ret  (molecule* current_molecule, linked_instruction* current_instruction);
void m_retl (molecule* current_molecule, linked_instruction* current_instruction);
void m_jmp  (molecule* current_molecule, linked_instruction* current_instruction);
void m_jmpl (molecule* current_molecule, linked_instruction* current_instruction);
void m_nop  (linked_instruction* current_instruction);
void m_brz  (molecule* current_molecule, linked_instruction* current_instruction);
void m_brn  (molecule* current_molecule, linked_instruction* current_instruction);
void m_brp  (molecule* current_molecule, linked_instruction* current_instruction);
void m_bra  (molecule* current_molecule, linked_instruction* current_instruction);
void m_brnz (molecule* current_molecule, linked_instruction* current_instruction);
void m_brnn (molecule* current_molecule, linked_instruction* current_instruction);
void m_brnp (molecule* current_molecule, linked_instruction* current_instruction);
void m_lim  (molecule* current_molecule, linked_instruction* current_instruction);
void m_bitt (molecule* current_molecule, linked_instruction* current_instruction);
void p_data (molecule* current_molecule, linked_instruction* current_instruction);
void p_error(molecule* current_molecule);
void assemble_data(uint8_t opcode, molecule* current_molecule, linked_instruction* current_instruction);

//Helper functions
uint64_t get_sel_literal_or_label(molecule* molecule);
void free_instruction(linked_instruction* current_instruction);
uint64_t get_binary_size_bytes(linked_instruction* instruction_head);

//Output functions
void fill_buf(linked_instruction* instruction_head, uint8_t* buffer);
void replace_file_extension(char* new_ext, char* out_name, char* new_name);
void write_mif(char* out_name, uint8_t* out_data, uint64_t prg_size);
void write_coe(char* out_name, uint8_t* out_data, uint64_t prg_size);
