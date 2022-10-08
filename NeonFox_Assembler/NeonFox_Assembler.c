#include "NeonFox_Assembler.h"
#include "range_sort.h"
#include "label_hash.h"

int main(int argc, char** argv)
{
	//Parse program arguments
	unsigned int arg = 1;
	char flag[5];
	if(argc == 1)
	{
		printf("Usage: -ASM <source file> -BIN <output binary> -COE <output coe file> -MIF <output mif file> -DEBUG\n");
		printf("-BIN, -COE, -MIF are optional, but at least one must be specified\n");
		printf("-DEBUG is optional\n");
		exit(1);
	}
	while((int)arg < argc)
	{
		if(argv[arg][0] == '-')
		{
			for(unsigned int d = 0; d < 4; ++d)
			{
				flag[d] = argv[arg][d];
				if(flag[d] == 0x00)
					break;
				if((flag[d] > 0x60) & (flag[d] < 0x7b))
					flag[d] = flag[d] - 0x20;
			}
			flag[4] = 0x00;
			if(str_comp_partial(asm_string, flag))
			{
				asm_index = ++arg;
			}
			else if(str_comp_partial(bin_string, flag))
			{
				bin_index = ++arg;
			}
			else if(str_comp_partial(coe_string, flag))
			{
				coe_index = ++arg;
			}
			else if(str_comp_partial(mif_string, flag))
			{
				mif_index = ++arg;
			}
			else if(str_comp_partial(debug_string, flag))
			{
				debug_enable = 0xFF;
			}
			++arg;
		}
		else
		{
			asm_index = arg++;
		}
	}
	if((int)arg > argc)
	{
		printf("Invalid arguments!\n");
		exit(1);
	}
	if(asm_index == 0)
	{
		printf("No source file specified!\n");
		exit(1);
	}
	if(bin_index == 0 && coe_index == 0 && mif_index == 0)
	{
		printf("No output file specified!\n");
		exit(1);
	}

	//Create LST file
	char* lst_name = (char*)malloc(str_size(argv[asm_index]) + 4);
	replace_file_extension(".LST", argv[asm_index], lst_name);
	preprocess(argv[asm_index], lst_name, debug_enable);

	// Set-up scanner globals
	g_label_count = 0;
	g_curr_word_address = 0;

	// Set-up molecule list structure
	molecule_head = (molecule*)malloc(sizeof(molecule));
	current_molecule = molecule_head;
	current_molecule->s_label = NULL;
	current_molecule->s_label_ref = NULL;
	current_molecule->s_source_file = NULL;
	current_molecule->s_macro_name = NULL;
	current_molecule->s_macro_file = NULL;

	// Clear current values
	mol_invalidate_current();

	// Set-up file/macro stack structure
	init_file_stack(256);
	init_macro_stack(4);

	// Do the thing
	yyin = fopen(lst_name, "r");
	yylex();
	free(lst_name);

	
	if (debug_enable) // Print whole molecule list
	{
		current_molecule = molecule_head->next;
		while(current_molecule)
		{
			mol_print_debug(current_molecule);
			current_molecule = current_molecule->next;
		}
	}

	// Build label map
	build_label_map(molecule_head, g_label_count);

	// Set-up linked-instruction structure
	linked_instruction* instruction_head = (linked_instruction*)malloc(sizeof(linked_instruction));
	linked_instruction* current_instruction = instruction_head;
	current_instruction->next = NULL;

	// Assemble molecules into linked-instructions (create machine code)
	current_molecule = molecule_head->next;
	while(current_molecule)
	{
		current_instruction->next = (linked_instruction*)malloc(sizeof(linked_instruction));
		current_instruction = current_instruction->next;
		current_instruction->next = NULL;
		current_instruction->word_address = current_molecule->word_address;
		gen_machine_instruction(current_molecule, current_instruction);

		current_molecule = current_molecule->next;
	}
	
	//free label map
	free_label_map();

	//check for segment overlap
	unsigned int num_segments = range_table_determine_size(instruction_head);
	printf("\n\n\n------------------\n");
	printf("num_segments: %u\n", num_segments);
	if(num_segments == 0)
	{
		fprintf(stderr, "No instructions found during range-checking, exiting...");
		exit(1);
	}
	s_range* range_table = range_table_build(instruction_head, num_segments);
	range_table_print(range_table, num_segments);
	range_table_check(range_table, num_segments);
	range_table_free(range_table);

	//write binary file
	uint64_t prg_size = get_binary_size_bytes(instruction_head);
	printf("Prog size: %llu\n", prg_size);
	uint8_t* output_arr = (uint8_t*)malloc(prg_size);

	for(uint64_t d = 0; d < prg_size; ++d)
	{
		output_arr[d] = 0xFF;
	}

	fill_buf(instruction_head, output_arr);

	//free linked instructions
	current_instruction = instruction_head;
	free_instruction(current_instruction);
	
	if(bin_index)
	{
		size_t written = 0;
		FILE *f = fopen(argv[bin_index], "wb");
		while (written < (size_t)prg_size)
		{
			written += fwrite(output_arr + written, sizeof(uint8_t), (size_t)prg_size - written, f);
			if (written == 0)
			{
				printf("Error writing output file!\n");
			}
		}
		fclose(f);
	}
	if(mif_index)
		write_mif(argv[mif_index], output_arr, prg_size);
	if(coe_index)
		write_coe(argv[coe_index], output_arr, prg_size);
	free(output_arr);
	return 0;
}

void gen_machine_instruction(molecule* current_molecule, linked_instruction* current_instruction)
{
	switch(current_molecule->mnemonic_index)
	{
		case 0:		// ADD
			m_add(current_molecule, current_instruction);
			break;
		case 1:		// ADDC
			m_addc(current_molecule, current_instruction);
			break;
		case 2:		// SUB
			m_sub(current_molecule, current_instruction);
			break;
		case 3:		// SUBC
			m_subc(current_molecule, current_instruction);
			break;
		case 4:		// MOVE
			m_move(current_molecule, current_instruction);
			break;
		case 5:		// TEST
			m_test(current_molecule, current_instruction);
			break;
		case 6:		// NOT
			m_not(current_molecule, current_instruction);
			break;
		case 7:		// ROR
			m_ror(current_molecule, current_instruction);
			break;
		case 8:		// ROL
			m_rol(current_molecule, current_instruction);
			break;
		case 9:		// AND
			m_and(current_molecule, current_instruction);
			break;
		case 10:	// XOR
			m_xor(current_molecule, current_instruction);
			break;
		case 11:	// OR
			m_or(current_molecule, current_instruction);
			break;
		case 12:	// CALL 
			m_call(current_molecule, current_instruction);
			break;
		case 13:	// CALLX
			m_jmp(current_molecule, current_instruction);
			break;
		case 14:	// CALLL
			m_calll(current_molecule, current_instruction);
			break;
		case 15:	// CALLLX
			m_jmpl(current_molecule, current_instruction);
			break;
		case 16:	// RET 
			m_ret(current_molecule, current_instruction);
			break;
		case 17:	// RETX
			m_jmp(current_molecule, current_instruction);
			break;
		case 18:	// RETL 
			m_retl(current_molecule, current_instruction);
			break;
		case 19:	// RETLX
			m_jmpl(current_molecule, current_instruction);
			break;
		case 20:	// JMP
			m_jmp(current_molecule, current_instruction);
			break;
		case 21:	// JMPL
			m_jmpl(current_molecule, current_instruction);
			break;
		case 22:	// NOP 
			m_nop(current_instruction);
			break;
		case 23:	// BRZ
			m_brz(current_molecule, current_instruction);
			break;
		case 24:	// BRN
			m_brn(current_molecule, current_instruction);
			break;
		case 25:	// BRP 
			m_brp(current_molecule, current_instruction);
			break;
		case 26:	// BRA
			m_bra(current_molecule, current_instruction);
			break;
		case 27:	// BRNZ 
			m_brnz(current_molecule, current_instruction);
			break;
		case 28:	// BRNN
			m_brnn(current_molecule, current_instruction);
			break;
		case 29:	// BRNP 
			m_brnp(current_molecule, current_instruction);
			break;
		case 30:	// LIM 
			m_lim(current_molecule, current_instruction);
			break;
		case 31:	// BITT 
			m_bitt(current_molecule, current_instruction);
			break;
		case 32:	//DATA
			p_data(current_molecule, current_instruction);
			break;
		default:
			p_error(current_molecule);
			break;
	}
}


void m_add(molecule* current_molecule, linked_instruction* current_instruction)
{
	assemble_data(0x00, current_molecule, current_instruction);
}

void m_addc(molecule* current_molecule, linked_instruction* current_instruction)
{
	assemble_data(0x01, current_molecule, current_instruction);
}

void m_sub(molecule* current_molecule, linked_instruction* current_instruction)
{
	assemble_data(0x02, current_molecule, current_instruction);
}

void m_subc(molecule* current_molecule, linked_instruction* current_instruction)
{
	assemble_data(0x03, current_molecule, current_instruction);
}

void m_move(molecule* current_molecule, linked_instruction* current_instruction)
{
	assemble_data(0x04, current_molecule, current_instruction);
}

void m_test(molecule* current_molecule, linked_instruction* current_instruction)
{
	uint8_t hl = current_molecule->target_byte_sel;
	uint8_t source = current_molecule->reg_first;

	current_instruction->instruction_high = (uint8_t)(0x40 | (hl << 2) | (uint8_t)((source & 0x1F) >> 3));
	current_instruction->instruction_low = (uint8_t)(((source & 0x07) << 5) | (source & 0x1F));

	//debug output
	if(debug_enable)
	{
		printf("%llX\t%X %X\t%s\t%u %u\n", 
			current_instruction->word_address, 
			current_instruction->instruction_high, 
			current_instruction->instruction_low,
			str_for_mnemonic(current_molecule->mnemonic_index),
			hl, source);
	}
}

void m_not(molecule* current_molecule, linked_instruction* current_instruction)
{
	assemble_data(0x05, current_molecule, current_instruction);
}

void m_ror(molecule* current_molecule, linked_instruction* current_instruction)
{
	assemble_data(0x06, current_molecule, current_instruction);
}

void m_rol(molecule* current_molecule, linked_instruction* current_instruction)
{
	assemble_data(0x07, current_molecule, current_instruction);
}

void m_and(molecule* current_molecule, linked_instruction* current_instruction)
{
	assemble_data(0x08, current_molecule, current_instruction);
}

void m_xor(molecule* current_molecule, linked_instruction* current_instruction)
{
	assemble_data(0x09, current_molecule, current_instruction);
}

void m_or(molecule* current_molecule, linked_instruction* current_instruction)
{
	assemble_data(0x0A, current_molecule, current_instruction);
}

void m_call(molecule* current_molecule, linked_instruction* current_instruction)
{
	current_instruction->instruction_high = 0xB0;
	current_instruction->instruction_low = 0x00;

	//debug output
	if(debug_enable)
	{
		printf("%llX\t%X %X\t%s\t\n", 
			current_instruction->word_address, 
			current_instruction->instruction_high, 
			current_instruction->instruction_low,
			str_for_mnemonic(current_molecule->mnemonic_index));
	}
}

void m_calll(molecule* current_molecule, linked_instruction* current_instruction)
{
	uint8_t hl = current_molecule->target_byte_sel;
	uint16_t immediate_value = (uint16_t)get_sel_literal_or_label(current_molecule);

	current_instruction->instruction_high = (uint8_t)(0xB0 | (hl << 2));
	current_instruction->instruction_low = (uint8_t)(immediate_value & 0xFF);

	//debug output
	if(debug_enable)
	{
		printf("%llX\t%X %X\t%s\t%u %u %u\n", 
			current_instruction->word_address, 
			current_instruction->instruction_high, 
			current_instruction->instruction_low,
			str_for_mnemonic(current_molecule->mnemonic_index),
			hl, current_molecule->literal_sel, current_molecule->literal);
	}
}

void m_ret(molecule* current_molecule, linked_instruction* current_instruction)
{
	current_instruction->instruction_high = 0xB2;
	current_instruction->instruction_low = 0x00;

	//debug output
	if(debug_enable)
	{
		printf("%llX\t%X %X\t%s\t\n", 
			current_instruction->word_address, 
			current_instruction->instruction_high, 
			current_instruction->instruction_low,
			str_for_mnemonic(current_molecule->mnemonic_index));
	}
}

void m_retl(molecule* current_molecule, linked_instruction* current_instruction)
{
	uint8_t hl = current_molecule->target_byte_sel;
	uint16_t immediate_value = (uint16_t)get_sel_literal_or_label(current_molecule);

	current_instruction->instruction_high = (uint8_t)(0xB2 | (hl << 2));
	current_instruction->instruction_low = (uint8_t)(immediate_value & 0xFF);

	//debug output
	if(debug_enable)
	{
		printf("%llX\t%X %X\t%s\t%u %u %u\n", 
			current_instruction->word_address, 
			current_instruction->instruction_high, 
			current_instruction->instruction_low,
			str_for_mnemonic(current_molecule->mnemonic_index),
			hl, current_molecule->literal_sel, current_molecule->literal);
	}
}

void m_jmp(molecule* current_molecule, linked_instruction* current_instruction)
{
	current_instruction->instruction_high = 0xB1;
	current_instruction->instruction_low = 0x00;

	//debug output
	if(debug_enable)
	{
		printf("%llX\t%X %X\t%s\t\n", 
			current_instruction->word_address, 
			current_instruction->instruction_high, 
			current_instruction->instruction_low,
			str_for_mnemonic(current_molecule->mnemonic_index));
	}
}

void m_jmpl(molecule* current_molecule, linked_instruction* current_instruction)
{
	uint8_t hl = current_molecule->target_byte_sel;
	uint16_t immediate_value = (uint16_t)get_sel_literal_or_label(current_molecule);

	current_instruction->instruction_high = (uint8_t)(0xB1 | (hl << 2));
	current_instruction->instruction_low = (uint8_t)(immediate_value & 0xFF);

	//debug output
	if(debug_enable)
	{
		printf("%llX\t%X %X\t%s\t%u %u %u\n", 
			current_instruction->word_address, 
			current_instruction->instruction_high, 
			current_instruction->instruction_low,
			str_for_mnemonic(current_molecule->mnemonic_index),
			hl, current_molecule->literal_sel, current_molecule->literal);
	}
}

void m_nop(linked_instruction* current_instruction)
{
	current_instruction->instruction_high = 0xC0;
	current_instruction->instruction_low = 0x00;

	//debug output
	if(debug_enable)
	{
		printf("%llX\t%X %X\t%s\t\n", 
			current_instruction->word_address, 
			current_instruction->instruction_high, 
			current_instruction->instruction_low,
			str_for_mnemonic(current_molecule->mnemonic_index));
	}
}

void m_brz(molecule* current_molecule, linked_instruction* current_instruction)
{
	uint64_t immediate_value = get_sel_literal_or_label(current_molecule);
	long branch_offset = (long int)(immediate_value - (current_instruction->word_address));

	current_instruction->instruction_high = (uint8_t)(0xC8 | ((branch_offset >> 8) & 0x03));
	current_instruction->instruction_low = (uint8_t)(branch_offset & 0xFF);

	if((branch_offset > 511) || (branch_offset < -512))
		printf("Warning: %s instruction at line %lu in file %s cannot reach target!\n", 
			str_for_mnemonic(current_molecule->mnemonic_index),
			current_molecule->n_source_line, 
			current_molecule->s_source_file);
	
	//debug output
	if(debug_enable)
	{
		printf("%llX\t%X %X\t%s\t%u %u\n", 
			current_instruction->word_address, 
			current_instruction->instruction_high, 
			current_instruction->instruction_low,
			str_for_mnemonic(current_molecule->mnemonic_index),
			current_molecule->literal_sel, current_molecule->literal);
	}
}

void m_brn(molecule* current_molecule, linked_instruction* current_instruction)
{
	uint64_t immediate_value = get_sel_literal_or_label(current_molecule);
	long branch_offset = (long int)(immediate_value - (current_instruction->word_address));

	current_instruction->instruction_high = (uint8_t)(0xC4 | ((branch_offset >> 8) & 0x03));
	current_instruction->instruction_low = (uint8_t)(branch_offset & 0xFF);

	if((branch_offset > 511) || (branch_offset < -512))
		printf("Warning: %s instruction at line %lu in file %s cannot reach target!\n", 
			str_for_mnemonic(current_molecule->mnemonic_index),
			current_molecule->n_source_line, 
			current_molecule->s_source_file);

	//debug output
	if(debug_enable)
	{
		printf("%llX\t%X %X\t%s\t%u %u\n", 
			current_instruction->word_address, 
			current_instruction->instruction_high, 
			current_instruction->instruction_low,
			str_for_mnemonic(current_molecule->mnemonic_index),
			current_molecule->literal_sel, current_molecule->literal);
	}
}

void m_brp(molecule* current_molecule, linked_instruction* current_instruction)
{
	uint64_t immediate_value = get_sel_literal_or_label(current_molecule);
	long branch_offset = (long int)(immediate_value - (current_instruction->word_address));

	current_instruction->instruction_high = (uint8_t)(0xCC | ((branch_offset >> 8) & 0x03));
	current_instruction->instruction_low = (uint8_t)(branch_offset & 0xFF);

	if((branch_offset > 511) || (branch_offset < -512))
		printf("Warning: %s instruction at line %lu in file %s cannot reach target!\n", 
			str_for_mnemonic(current_molecule->mnemonic_index),
			current_molecule->n_source_line, 
			current_molecule->s_source_file);

	//debug output
	if(debug_enable)
	{
		printf("%llX\t%X %X\t%s\t%u %u\n", 
			current_instruction->word_address, 
			current_instruction->instruction_high, 
			current_instruction->instruction_low,
			str_for_mnemonic(current_molecule->mnemonic_index),
			current_molecule->literal_sel, current_molecule->literal);
	}
}

void m_bra(molecule* current_molecule, linked_instruction* current_instruction)
{
	uint64_t immediate_value = get_sel_literal_or_label(current_molecule);
	long branch_offset = (long int)(immediate_value - (current_instruction->word_address));

	current_instruction->instruction_high = (uint8_t)(0xD0 | ((branch_offset >> 8) & 0x03));
	current_instruction->instruction_low = (uint8_t)(branch_offset & 0xFF);

	if((branch_offset > 511) || (branch_offset < -512))
		printf("Warning: %s instruction at line %lu in file %s cannot reach target!\n", 
			str_for_mnemonic(current_molecule->mnemonic_index),
			current_molecule->n_source_line, 
			current_molecule->s_source_file);

	//debug output
	if(debug_enable)
	{
		printf("%llX\t%X %X\t%s\t%u %u\n", 
			current_instruction->word_address, 
			current_instruction->instruction_high, 
			current_instruction->instruction_low,
			str_for_mnemonic(current_molecule->mnemonic_index),
			current_molecule->literal_sel, current_molecule->literal);
	}
}

void m_brnz(molecule* current_molecule, linked_instruction* current_instruction)
{
	uint64_t immediate_value = get_sel_literal_or_label(current_molecule);
	long branch_offset = (long int)(immediate_value - (current_instruction->word_address));

	current_instruction->instruction_high = (uint8_t)(0xD8 | ((branch_offset >> 8) & 0x03));
	current_instruction->instruction_low = (uint8_t)(branch_offset & 0xFF);

	if((branch_offset > 511) || (branch_offset < -512))
		printf("Warning: %s instruction at line %lu in file %s cannot reach target!\n", 
			str_for_mnemonic(current_molecule->mnemonic_index),
			current_molecule->n_source_line, 
			current_molecule->s_source_file);

	//debug output
	if(debug_enable)
	{
		printf("%llX\t%X %X\t%s\t%u %u\n", 
			current_instruction->word_address, 
			current_instruction->instruction_high, 
			current_instruction->instruction_low,
			str_for_mnemonic(current_molecule->mnemonic_index),
			current_molecule->literal_sel, current_molecule->literal);
	}
}

void m_brnn(molecule* current_molecule, linked_instruction* current_instruction)
{
	uint64_t immediate_value = get_sel_literal_or_label(current_molecule);
	long branch_offset = (long int)(immediate_value - (current_instruction->word_address));

	current_instruction->instruction_high = (uint8_t)(0xD4 | ((branch_offset >> 8) & 0x03));
	current_instruction->instruction_low = (uint8_t)(branch_offset & 0xFF);

	if((branch_offset > 511) || (branch_offset < -512))
		printf("Warning: %s instruction at line %lu in file %s cannot reach target!\n", 
			str_for_mnemonic(current_molecule->mnemonic_index),
			current_molecule->n_source_line, 
			current_molecule->s_source_file);

	//debug output
	if(debug_enable)
	{
		printf("%llX\t%X %X\t%s\t%u %u\n", 
			current_instruction->word_address, 
			current_instruction->instruction_high, 
			current_instruction->instruction_low,
			str_for_mnemonic(current_molecule->mnemonic_index),
			current_molecule->literal_sel, current_molecule->literal);
	}
}

void m_brnp(molecule* current_molecule, linked_instruction* current_instruction)
{
	uint64_t immediate_value = get_sel_literal_or_label(current_molecule);
	long branch_offset = (long int)(immediate_value - (current_instruction->word_address));

	current_instruction->instruction_high = (uint8_t)(0xDC | ((branch_offset >> 8) & 0x03));
	current_instruction->instruction_low = (uint8_t)(branch_offset & 0xFF);

	if((branch_offset > 511) || (branch_offset < -512))
		printf("Warning: %s instruction at line %lu in file %s cannot reach target!\n", 
			str_for_mnemonic(current_molecule->mnemonic_index),
			current_molecule->n_source_line, 
			current_molecule->s_source_file);

	//debug output
	if(debug_enable)
	{
		printf("%llX\t%X %X\t%s\t%u %u\n", 
			current_instruction->word_address, 
			current_instruction->instruction_high, 
			current_instruction->instruction_low,
			str_for_mnemonic(current_molecule->mnemonic_index),
			current_molecule->literal_sel, current_molecule->literal);
	}
}

void m_lim(molecule* current_molecule, linked_instruction* current_instruction)
{
	uint8_t hl = current_molecule->target_byte_sel;
	uint16_t immediate_value = (uint16_t)get_sel_literal_or_label(current_molecule);
	uint8_t dest = current_molecule->reg_first;

	current_instruction->instruction_high = (uint8_t)(0xE0 | (hl << 2) | (uint8_t)(dest & 0x03));
	current_instruction->instruction_low = (uint8_t)(immediate_value & 0xFF);

	if((dest >> 2) != 0x04)
		printf("Warning: %s instruction at line %lu in file %s specifies invalid destination!\n",
			str_for_mnemonic(current_molecule->mnemonic_index),
			current_molecule->n_source_line, 
			current_molecule->s_source_file);

	//debug output
	if(debug_enable)
	{
		printf("%llX\t%X %X\t%s\t%u %u %u\n", 
		current_instruction->word_address, 
		current_instruction->instruction_high, 
		current_instruction->instruction_low,
		str_for_mnemonic(current_molecule->mnemonic_index),
		hl, dest, immediate_value);
	}
}

void m_bitt(molecule* current_molecule, linked_instruction* current_instruction)
{
	uint8_t hl = current_molecule->target_byte_sel;
	uint16_t bit = (uint16_t)get_sel_literal_or_label(current_molecule);
	uint8_t source = current_molecule->reg_first;

	if(hl == 2)	//high byte
		bit += 8;
	else if(hl == 0)	//swap bytes
		bit = bit ^ 8;

	current_instruction->instruction_high = (uint8_t)(0xF0 | ((source >> 3) & 0x03));
	current_instruction->instruction_low = (uint8_t)(((source & 0x07) << 5) | (bit & 0x0F));

	//debug output
	if(debug_enable)
	{
		printf("%llX\t%X %X\t%s\t%u %u\n", 
		current_instruction->word_address, 
		current_instruction->instruction_high, 
		current_instruction->instruction_low,
		str_for_mnemonic(current_molecule->mnemonic_index),
		source, bit);
	}
}

void p_data(molecule* current_molecule, linked_instruction* current_instruction)
{
	uint64_t immediate_value = get_sel_literal_or_label(current_molecule);
	
	current_instruction->instruction_high = (uint8_t)(immediate_value >> 8);
	current_instruction->instruction_low = immediate_value & 0xFF;

	if(immediate_value >> 16)
	{
		printf("Warning: Data at line %lu in file %s truncated to 16 bits!\n", 
			current_molecule->n_source_line, 
			current_molecule->s_source_file);
	}

	//debug output
	if(debug_enable)
	{
		printf("%llX\t%X %X\t%s\t\n", 
			current_instruction->word_address, 
			current_instruction->instruction_high, 
			current_instruction->instruction_low,
			str_for_mnemonic(current_molecule->mnemonic_index));
	}
}

void p_error(molecule* current_molecule)
{
	printf("ERROR: Invalid mnemonic index \'%d\' in at line %lu in file %s\n", 
		current_molecule->mnemonic_index,
		current_molecule->n_source_line, 
		current_molecule->s_source_file);

	exit(1);
}

void assemble_data(uint8_t opcode, molecule* current_molecule, linked_instruction* current_instruction)
{
	uint8_t hl = current_molecule->target_byte_sel;
	uint8_t source = current_molecule->reg_first;
	uint8_t dest = current_molecule->reg_second;

	current_instruction->instruction_high = (uint8_t)((opcode << 4) | (hl << 2) | (uint8_t)((source & 0x1F) >> 3));
	current_instruction->instruction_low = (uint8_t)(((source & 0x07) << 5) | (dest & 0x1F));

	//debug output
	if(debug_enable)
	{
		printf("%llX\t%X %X\t%s\t%u %u %u\n", 
		current_instruction->word_address, 
		current_instruction->instruction_high, 
		current_instruction->instruction_low,
		str_for_mnemonic(current_molecule->mnemonic_index),
		hl, source, dest);
	}
}

uint64_t get_sel_literal_or_label(molecule* molecule)
{
	char* s_label_ref = molecule->s_label_ref;
	uint64_t literal = molecule->literal;
	uint8_t literal_sel = molecule->literal_sel;

	if (s_label_ref)
	{
		literal = get_label_value(s_label_ref, current_molecule->n_source_line, current_molecule->s_source_file);
	}

	switch(literal_sel)
	{
		case 0: // LL
			return ((uint16_t)literal & 0xFF);
		case 1: // LH
			return (((uint16_t)literal >> 8) & 0xFF);
		case 2: // HL
			return (((uint16_t)literal >> 16) & 0xFF);
		case 3: // HH
			return (((uint16_t)literal >> 24) & 0xFF);
		case 4: // LW
			return (uint16_t)literal;
		case 5:	// HW
			return ((uint16_t)(literal >> 16));
		default: // Assume Label
			return literal;
	}
}

inline void free_instruction(linked_instruction* current_instruction)
{
	linked_instruction* next_instruction = NULL;
	while(1)
	{
		if(current_instruction == NULL)
			break;
		next_instruction = current_instruction->next;
		free(current_instruction);
		current_instruction = next_instruction;
	}
}

inline uint64_t get_binary_size_bytes(linked_instruction* instruction_head)
{
	uint64_t end = 0;
	linked_instruction* current_instruction = instruction_head->next;

	while(current_instruction)
	{
		if (current_instruction->word_address > end)
			end = current_instruction->word_address;

		current_instruction = current_instruction->next;
	}
	
	// Convert words->bytes
	return (end * 2) + 2;
}

inline void fill_buf(linked_instruction* instruction_head, uint8_t* buffer)
{
	linked_instruction* current_instruction = instruction_head->next;

	while(current_instruction)
	{
		buffer[(current_instruction->word_address * 2)] = current_instruction->instruction_low;
		buffer[(current_instruction->word_address * 2) + 1] = current_instruction->instruction_high;

		current_instruction = current_instruction->next;
	}
}

void replace_file_extension(char* new_ext, char* out_name, char* new_name)
{
	uint8_t count = 0;
	uint8_t count_copy;
	//copy string and get its length
	do
	{
		new_name[count] = out_name[count];
		++count;
	}while(out_name[count]);
	count_copy = count;
	//search for a '.' starting from the end
	while(count)
	{
		count = count - 1;
		if(new_name[count] == '.')	//replace it and what follows with .mif
		{
			for(unsigned int d = 0; d < 5; ++d)
			{
				new_name[count] = new_ext[d];
				count = count + 1;
			}
			break;
		}
	}
	//check if the file name had no '.'
	if(count == 0)
	{
		count = count_copy;
		for(unsigned int d = 0; d < 5; ++d)
		{
			new_name[count] = new_ext[d];
			count = count + 1;
		}
	}
}

void write_mif(char* out_name, uint8_t* out_data, uint64_t prg_size)
{
	static char mif_string[] = ".mif";
	char new_name[32];
	replace_file_extension(mif_string, out_name, new_name);

	FILE* mif_file = fopen(new_name, "w");
	if(mif_file == NULL)
	{
		printf("Error creating mif file!");
		exit(1);
	}
	fprintf(mif_file, "DEPTH = %llu;\n", prg_size / 2);	//prg_size is in bytes, we want words
	fprintf(mif_file, "WIDTH = 16;\n");
	fprintf(mif_file, "ADDRESS_RADIX = HEX;\n");
	fprintf(mif_file, "DATA_RADIX = HEX;\n");
	fprintf(mif_file, "CONTENT\nBEGIN\n");
	for(uint64_t address = 0; address < (prg_size / 2); ++address)
	{
		fprintf(mif_file, "%llX : %04X;\n", address, ((out_data[address * 2 + 1] << 8) | out_data[address * 2]));
	}
	fprintf(mif_file, "END;\n");
	fclose(mif_file);
}

void write_coe(char* out_name, uint8_t* out_data, uint64_t prg_size)
{
	static char coe_string[] = ".coe";
	char new_name[32];
	replace_file_extension(coe_string, out_name, new_name);

	FILE* coe_file = fopen(new_name, "w");
	if(coe_file == NULL)
	{
		printf("Error creating coe file!");
		exit(1);
	}
	fprintf(coe_file, "memory_initialization_radix=16;\n");
	fprintf(coe_file, "memory_initialization_vector=\n");
	for(uint64_t address = 0; address < (prg_size / 2); ++address)
	{
		if((address + 1) < (prg_size / 2))
			fprintf(coe_file, "%04X,\n", ((out_data[address * 2 + 1] << 8) | out_data[address * 2]));
		else
			fprintf(coe_file, "%04X;\n", ((out_data[address * 2 + 1] << 8) | out_data[address * 2]));
	}
	fclose(coe_file);
}
