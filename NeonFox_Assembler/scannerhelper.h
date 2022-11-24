/////////////////////
// Scanner globals //
/////////////////////
extern uint32_t g_label_count;
extern uint64_t g_curr_word_address;
extern uint8_t g_debug_level;

void set_org(char* s, int radix);


////////////////////
// String helpers //
////////////////////
void to_caps(char* str);
void remove_spaces(char* s);
char* str_or_null(char* s);
const char* str_for_mnemonic(uint8_t mnemonicIdx);
const char* str_for_tgt_byte_sel(uint8_t byte_sel_idx);
const char* str_for_lit_byte_sel(uint8_t byte_sel_idx);


////////////////////
// Stack helpers  //
////////////////////
typedef struct ACTIVE_FILE
{
	char* s_source_file;
	unsigned long n_source_line;
} active_file;

typedef struct ACTIVE_MACRO
{
	char* s_macro_name;
	char* s_macro_file;
	unsigned long n_macro_line;
} active_macro;

void init_file_stack(uint32_t size);
void init_macro_stack(uint32_t size);
void push_file(char* name);
void push_macro_ref(char* name, char* file);
void pop_file();
void pop_macro();
active_file* peek_file();
active_macro* peek_macro();
int is_file_stack_empty();
int is_macro_stack_empty();


//////////////////////
// Molecule helpers //
//////////////////////
enum Field {FieldLabel, FieldMnemonic, FieldTargetByteSel, FieldLiteralSel, FieldLiteral, FieldRegFirst, FieldRegSecond, FieldLabelRef};

typedef struct MOLECULE
{
	// Instruction Components
	uint64_t word_address;
	char* s_label;					//label (labeling this line) portion of source line (if any)
	uint8_t mnemonic_index;			//number that uniquely identifies a mnemonic or key word
	uint8_t target_byte_sel;
	uint8_t literal_sel;
	uint32_t literal;
	uint8_t reg_first;
	uint8_t reg_second;
	char* s_label_ref;				//label referenced by line (if any)

	// Debugging/logging info
	char* s_source_file;
	unsigned long n_source_line;	//source line number for printing and error mesage use only
	
	char* s_macro_name;
	char* s_macro_file;
	unsigned long n_macro_line;

	// Structure
	struct MOLECULE* next;
} molecule;

extern molecule* molecule_head;
extern molecule* current_molecule;

void mol_set_mnemonic_index(uint8_t mnemonic_index);
void mol_set_target_byte_sel(uint8_t target_byte_sel);
void mol_set_literal_sel(uint8_t literal_sel);
void mol_set_literal(char* literal, int radix);
void mol_set_reg(char* reg);
void mol_set_reg_first(char* reg_first);
void mol_set_reg_second(char* reg_second);
void mol_set_identifier(char* identifier);

void mol_handle_delim(char* s);
void mol_inc_token_cnt();
void debug_set_line(char* yytext);
void debug_push_file(char* yytext);
void debug_pop_file(char* yytext);
void debug_push_macro(char* yytext);
void debug_pop_macro(char* yytext);
void mol_handle_unexpected(char* token);

void mol_invalidate_current();
void mol_generate();
void mol_print_debug(molecule* mol);

void mol_valid_and_fields(const enum Field* fields, uint8_t n_fields);
void mol_valid_or_fields(const enum Field* fields, uint8_t n_fields);
void mol_valid_xor_fields(const enum Field* fields, uint8_t n_fields);
uint8_t mol_valid_field_present(enum Field field);
void mol_err_dup_field(enum Field field);
void mol_err_req_field(enum Field field);
void mol_err_invalid_fields(const enum Field* fields, uint8_t n_fields);