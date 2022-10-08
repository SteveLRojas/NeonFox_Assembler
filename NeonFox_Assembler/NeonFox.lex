
%{
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
//#include "NeonFox_Assembler.h"
#include "scannerhelper.h"

const char* DELIM_COLON = ":";
%}

DIGIT          [0-9]
DIGIT_HEX      [0-9A-F]
DIGIT_OCTAL    [0-7]
DIGIT_BINARY   [0-1]

LABEL    [a-zA-Z]([a-zA-Z0-9]|[_])*
WHITESPACE [ \t]
INST_DELIM ({WHITESPACE}+|({WHITESPACE}*[,]{WHITESPACE}*))
FILENAME (.+[.].+)
EOL (\r\n|\r|\n)




%%




{WHITESPACE}+ORG{WHITESPACE}+${DIGIT_HEX}+{WHITESPACE}*{EOL} {
   printf( "ORG: %s\n", yytext);
   set_org(yytext, 16);
}

{WHITESPACE}+ORG{WHITESPACE}+@{DIGIT_OCTAL}+{WHITESPACE}*{EOL} {
   printf( "ORG: %s\n", yytext);
   set_org(yytext, 8);
}

{WHITESPACE}+ORG{WHITESPACE}+%{DIGIT_BINARY}+{WHITESPACE}*{EOL} {
   printf( "ORG: %s\n", yytext);
   set_org(yytext, 2);
}

{WHITESPACE}+ORG{WHITESPACE}+{DIGIT}+{WHITESPACE}*{EOL} {
   printf( "ORG: %s\n", yytext);
   set_org(yytext, 10);
}


   /*-----------------------
     INSTRUCTION MNEMONICS 
   -----------------------*/

ADD {
   uint8_t mnemonicIdx = 0;
   printf( "INST: %s\n", str_for_mnemonic(mnemonicIdx));
   mol_set_mnemonic_index(mnemonicIdx);
}

ADDC {
   uint8_t mnemonicIdx = 1;
   printf( "INST: %s\n", str_for_mnemonic(mnemonicIdx));
   mol_set_mnemonic_index(mnemonicIdx);
}

SUB {
   uint8_t mnemonicIdx = 2;
   printf( "INST: %s\n", str_for_mnemonic(mnemonicIdx));
   mol_set_mnemonic_index(mnemonicIdx);
}

SUBC {
   uint8_t mnemonicIdx = 3;
   printf( "INST: %s\n", str_for_mnemonic(mnemonicIdx));
   mol_set_mnemonic_index(mnemonicIdx);
}

MOVE {
   uint8_t mnemonicIdx = 4;
   printf( "INST: %s\n", str_for_mnemonic(mnemonicIdx));
   mol_set_mnemonic_index(mnemonicIdx);
}

TEST {
   uint8_t mnemonicIdx = 5;
   printf( "INST: %s\n", str_for_mnemonic(mnemonicIdx));
   mol_set_mnemonic_index(mnemonicIdx);
}

NOT {
   uint8_t mnemonicIdx = 6;
   printf( "INST: %s\n", str_for_mnemonic(mnemonicIdx));
   mol_set_mnemonic_index(mnemonicIdx);
}

ROR {
   uint8_t mnemonicIdx = 7;
   printf( "INST: %s\n", str_for_mnemonic(mnemonicIdx));
   mol_set_mnemonic_index(mnemonicIdx);
}

ROL {
   uint8_t mnemonicIdx = 8;
   printf( "INST: %s\n", str_for_mnemonic(mnemonicIdx));
   mol_set_mnemonic_index(mnemonicIdx);
}

AND {
   uint8_t mnemonicIdx = 9;
   printf( "INST: %s\n", str_for_mnemonic(mnemonicIdx));
   mol_set_mnemonic_index(mnemonicIdx);
}

XOR {
   uint8_t mnemonicIdx = 10;
   printf( "INST: %s\n", str_for_mnemonic(mnemonicIdx));
   mol_set_mnemonic_index(mnemonicIdx);
}

OR {
   uint8_t mnemonicIdx = 11;
   printf( "INST: %s\n", str_for_mnemonic(mnemonicIdx));
   mol_set_mnemonic_index(mnemonicIdx);
}

CALL {
   uint8_t mnemonicIdx = 12;
   printf( "INST: %s\n", str_for_mnemonic(mnemonicIdx));
   mol_set_mnemonic_index(mnemonicIdx);
}

CALLX {
   uint8_t mnemonicIdx = 13;
   printf( "INST: %s\n", str_for_mnemonic(mnemonicIdx));
   mol_set_mnemonic_index(mnemonicIdx);
}

CALLL {
   uint8_t mnemonicIdx = 14;
   printf( "INST: %s\n", str_for_mnemonic(mnemonicIdx));
   mol_set_mnemonic_index(mnemonicIdx);
}

CALLLX {
   uint8_t mnemonicIdx = 15;
   printf( "INST: %s\n", str_for_mnemonic(mnemonicIdx));
   mol_set_mnemonic_index(mnemonicIdx);
}

RET {
   uint8_t mnemonicIdx = 16;
   printf( "INST: %s\n", str_for_mnemonic(mnemonicIdx));
   mol_set_mnemonic_index(mnemonicIdx);
}

RETX {
   uint8_t mnemonicIdx = 17;
   printf( "INST: %s\n", str_for_mnemonic(mnemonicIdx));
   mol_set_mnemonic_index(mnemonicIdx);
}

RETL {
   uint8_t mnemonicIdx = 18;
   printf( "INST: %s\n", str_for_mnemonic(mnemonicIdx));
   mol_set_mnemonic_index(mnemonicIdx);
}

RETLX {
   uint8_t mnemonicIdx = 19;
   printf( "INST: %s\n", str_for_mnemonic(mnemonicIdx));
   mol_set_mnemonic_index(mnemonicIdx);
}

JMP {
   uint8_t mnemonicIdx = 20;
   printf( "INST: %s\n", str_for_mnemonic(mnemonicIdx));
   mol_set_mnemonic_index(mnemonicIdx);
}

JMPL {
   uint8_t mnemonicIdx = 21;
   printf( "INST: %s\n", str_for_mnemonic(mnemonicIdx));
   mol_set_mnemonic_index(mnemonicIdx);
}

NOP {
   uint8_t mnemonicIdx = 22;
   printf( "INST: %s\n", str_for_mnemonic(mnemonicIdx));
   mol_set_mnemonic_index(mnemonicIdx);
}

BRZ {
   uint8_t mnemonicIdx = 23;
   printf( "INST: %s\n", str_for_mnemonic(mnemonicIdx));
   mol_set_mnemonic_index(mnemonicIdx);
}

BRN {
   uint8_t mnemonicIdx = 24;
   printf( "INST: %s\n", str_for_mnemonic(mnemonicIdx));
   mol_set_mnemonic_index(mnemonicIdx);
}

BRP {
   uint8_t mnemonicIdx = 25;
   printf( "INST: %s\n", str_for_mnemonic(mnemonicIdx));
   mol_set_mnemonic_index(mnemonicIdx);
}

BRA {
   uint8_t mnemonicIdx = 26;
   printf( "INST: %s\n", str_for_mnemonic(mnemonicIdx));
   mol_set_mnemonic_index(mnemonicIdx);
}

BRNZ {
   uint8_t mnemonicIdx = 27;
   printf( "INST: %s\n", str_for_mnemonic(mnemonicIdx));
   mol_set_mnemonic_index(mnemonicIdx);
}

BRNN {
   uint8_t mnemonicIdx = 28;
   printf( "INST: %s\n", str_for_mnemonic(mnemonicIdx));
   mol_set_mnemonic_index(mnemonicIdx);
}

BRNP {
   uint8_t mnemonicIdx = 29;
   printf( "INST: %s\n", str_for_mnemonic(mnemonicIdx));
   mol_set_mnemonic_index(mnemonicIdx);
}

LIM {
   uint8_t mnemonicIdx = 30;
   printf( "INST: %s\n", str_for_mnemonic(mnemonicIdx));
   mol_set_mnemonic_index(mnemonicIdx);
}

BITT {
   uint8_t mnemonicIdx = 31;
   printf( "INST: %s\n", str_for_mnemonic(mnemonicIdx));
   mol_set_mnemonic_index(mnemonicIdx);
}

DATA {
   uint8_t mnemonicIdx = 32;
   printf( "INST: %s\n", str_for_mnemonic(mnemonicIdx));
   mol_set_mnemonic_index(mnemonicIdx);
}


   /*----------------------
     INSTRUCTION OPERANDS 
   ----------------------*/

H {
   printf("TGT BYTE SEL: %s\n", yytext);
   mol_set_target_byte_sel(2);
}

L {
   printf("TGT BYTE SEL: %s\n", yytext);
   mol_set_target_byte_sel(1);
}

S {
   printf("TGT BYTE SEL: %s\n", yytext);
   mol_set_target_byte_sel(0);
}

W {
   printf("TGT BYTE SEL: %s\n", yytext);
   mol_set_target_byte_sel(3);
}

${DIGIT_HEX}+ {
   printf("LITN: %s\n", yytext);
   mol_set_literal(yytext + 1, 16);
}

@{DIGIT_OCTAL}+ {
   printf("LITN: %s\n", yytext);
   mol_set_literal(yytext + 1, 8);
}

[%]{DIGIT_BINARY}+ {
   printf("LITN: %s\n", yytext);
   mol_set_literal(yytext + 1, 2);
}

{DIGIT}+ {
   printf("LITN: %s\n", yytext);
   mol_set_literal(yytext, 10);
}

['](.+)['] {
   printf("LITC: %s\n", yytext);
   mol_set_literal(yytext, 0);
}

`LL {
   printf("LIT BSEL: %s\n", yytext);
   mol_set_literal_sel(0);
}

`LH {
   printf("LIT BSEL: %s\n", yytext);
   mol_set_literal_sel(1);
}

`HL {
   printf("LIT BSEL: %s\n", yytext);
   mol_set_literal_sel(2);
}

`HH {
   printf("LIT BSEL: %s\n", yytext);
   mol_set_literal_sel(3);
}

`LW {
   printf("LIT WSEL: %s\n", yytext);
   mol_set_literal_sel(4);
}

`HW {
   printf("LIT WSEL: %s\n", yytext);
   mol_set_literal_sel(5);
}

R{DIGIT}+ {
   printf("REGS: %s\n", yytext);
   mol_set_reg(yytext + 1);
}

{LABEL} {
   printf( "LABL OR LABL REF: %s\n", yytext);
   mol_set_identifier(yytext);
}

{INST_DELIM}+ {
   mol_handle_delim(yytext);
}

{EOL} {
   printf("END OF LINE\n");
   mol_generate();
}


   /*------------------
     DEBUG DIRECTIVES
   ------------------*/

@LINE:{WHITESPACE}*{DIGIT}+{EOL} {
   char* text = strdup(yytext);
   char* token = strtok(text, DELIM_COLON);        //@LINE
   token = strtok(NULL, DELIM_COLON);              //[line_num]
   unsigned long line_num = (unsigned long)strtol(token, NULL, 10);

   printf("LINE: %s\n", token);

   if(is_macro_stack_empty()){
      peek_file()->n_source_line = line_num;
   } else {
      peek_macro()->n_macro_line = line_num;
   }

   free(text);
}

@PUSH_FILE:{WHITESPACE}*{FILENAME}{EOL} {
   char* text = strdup(yytext);
   char* token = strtok(text, DELIM_COLON);        //@PUSH_FILE
   token = strtok(NULL, DELIM_COLON);              //[file_name]

   printf("PUSH_FILE: %s\n", token);
   push_file(token);

   free(text);
}

@POP_FILE:{WHITESPACE}*{FILENAME}{EOL} {
   char* text = strdup(yytext);
   char* token = strtok(text, DELIM_COLON);        //@POP_FILE
   token = strtok(NULL, DELIM_COLON);              //[file_name]

   printf("POP_FILE: %s\n", token);
   pop_file();

   free(text);
}

@PUSH_MACRO:{WHITESPACE}*{LABEL}:{FILENAME}{EOL} {
   char* text = strdup(yytext);
   char* macro_name;
   char* macro_source_file_name; 

   macro_name = strtok(text, DELIM_COLON);         //@PUSH_FILE
   macro_name = strtok(NULL, DELIM_COLON);        
   printf("PUSH_MACRO: %s\n", macro_name);

   macro_source_file_name = strtok(NULL, DELIM_COLON);           
   printf("\tMACRO_FILE: %s\n", macro_source_file_name);

   push_macro_ref(macro_name, macro_source_file_name);

   // Increment source-file line just once for entire encountered macro expansion
   peek_file()->n_source_line++;

   free(text);
}

@POP_MACRO:{WHITESPACE}*{LABEL}{EOL} {
   char* text = strdup(yytext);
   char* token = strtok(text, DELIM_COLON);        //@PUSH_FILE
   token = strtok(NULL, DELIM_COLON);              //[macro_name]
   printf("POP_MACRO: %s\n", token);

   pop_macro();

   free(text);
}


   /*-----------------------------------------------------------------------------*/

. {
   mol_handle_unexpected(yytext);
}




%%




// int main( int argc, char **argv )
// {
//    // Set-up molecule list structure
//    molecule_head = (molecule*)malloc(sizeof(molecule));
//    current_molecule = molecule_head;
//    current_molecule->s_label = NULL;
//    current_molecule->s_label_ref = NULL;
//    current_molecule->s_source_file = NULL;
//    current_molecule->s_macro_name = NULL;
//    current_molecule->s_macro_file = NULL;

//    // Clear current values
//    mol_invalidate_current();

//    // Set-up file/macro stack structure
//    init_file_stack(256);
//    init_macro_stack(4);

//    // Do the thing
//    ++argv, --argc; /* skip over program name */
//    if ( argc > 0 )
//       yyin = fopen( argv[0], "r" );
//    else
//       yyin = stdin;
//    yylex();

//    // Print whole molecule list
//    molecule* mol = molecule_head->next;
//    while(mol)
//    {
//       mol_print_debug(mol);
//       mol = mol->next;
//    }
// }

