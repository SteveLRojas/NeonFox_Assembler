
%{
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
//#include "NeonFox_Assembler.h"
#include "scannerhelper.h"

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
   set_org(yytext, 16);
}

{WHITESPACE}+ORG{WHITESPACE}+@{DIGIT_OCTAL}+{WHITESPACE}*{EOL} {
   set_org(yytext, 8);
}

{WHITESPACE}+ORG{WHITESPACE}+%{DIGIT_BINARY}+{WHITESPACE}*{EOL} {
   set_org(yytext, 2);
}

{WHITESPACE}+ORG{WHITESPACE}+{DIGIT}+{WHITESPACE}*{EOL} {
   set_org(yytext, 10);
}


   /*-----------------------
     INSTRUCTION MNEMONICS 
   -----------------------*/

ADD {
   uint8_t mnemonicIdx = 0;
   mol_set_mnemonic_index(mnemonicIdx);
}

ADDC {
   uint8_t mnemonicIdx = 1;
   mol_set_mnemonic_index(mnemonicIdx);
}

SUB {
   uint8_t mnemonicIdx = 2;
   mol_set_mnemonic_index(mnemonicIdx);
}

SUBC {
   uint8_t mnemonicIdx = 3;
   mol_set_mnemonic_index(mnemonicIdx);
}

MOVE {
   uint8_t mnemonicIdx = 4;
   mol_set_mnemonic_index(mnemonicIdx);
}

TEST {
   uint8_t mnemonicIdx = 5;
   mol_set_mnemonic_index(mnemonicIdx);
}

NOT {
   uint8_t mnemonicIdx = 6;
   mol_set_mnemonic_index(mnemonicIdx);
}

ROR {
   uint8_t mnemonicIdx = 7;
   mol_set_mnemonic_index(mnemonicIdx);
}

ROL {
   uint8_t mnemonicIdx = 8;
   mol_set_mnemonic_index(mnemonicIdx);
}

AND {
   uint8_t mnemonicIdx = 9;
   mol_set_mnemonic_index(mnemonicIdx);
}

XOR {
   uint8_t mnemonicIdx = 10;
   mol_set_mnemonic_index(mnemonicIdx);
}

OR {
   uint8_t mnemonicIdx = 11;
   mol_set_mnemonic_index(mnemonicIdx);
}

CALL {
   uint8_t mnemonicIdx = 12;
   mol_set_mnemonic_index(mnemonicIdx);
}

CALLX {
   uint8_t mnemonicIdx = 13;
   mol_set_mnemonic_index(mnemonicIdx);
}

CALLL {
   uint8_t mnemonicIdx = 14;
   mol_set_mnemonic_index(mnemonicIdx);
}

CALLLX {
   uint8_t mnemonicIdx = 15;
   mol_set_mnemonic_index(mnemonicIdx);
}

RET {
   uint8_t mnemonicIdx = 16;
   mol_set_mnemonic_index(mnemonicIdx);
}

RETX {
   uint8_t mnemonicIdx = 17;
   mol_set_mnemonic_index(mnemonicIdx);
}

RETL {
   uint8_t mnemonicIdx = 18;
   mol_set_mnemonic_index(mnemonicIdx);
}

RETLX {
   uint8_t mnemonicIdx = 19;
   mol_set_mnemonic_index(mnemonicIdx);
}

JMP {
   uint8_t mnemonicIdx = 20;
   mol_set_mnemonic_index(mnemonicIdx);
}

JMPL {
   uint8_t mnemonicIdx = 21;
   mol_set_mnemonic_index(mnemonicIdx);
}

NOP {
   uint8_t mnemonicIdx = 22;
   mol_set_mnemonic_index(mnemonicIdx);
}

BRZ {
   uint8_t mnemonicIdx = 23;
   mol_set_mnemonic_index(mnemonicIdx);
}

BRN {
   uint8_t mnemonicIdx = 24;
   mol_set_mnemonic_index(mnemonicIdx);
}

BRP {
   uint8_t mnemonicIdx = 25;
   mol_set_mnemonic_index(mnemonicIdx);
}

BRA {
   uint8_t mnemonicIdx = 26;
   mol_set_mnemonic_index(mnemonicIdx);
}

BRNZ {
   uint8_t mnemonicIdx = 27;
   mol_set_mnemonic_index(mnemonicIdx);
}

BRNN {
   uint8_t mnemonicIdx = 28;
   mol_set_mnemonic_index(mnemonicIdx);
}

BRNP {
   uint8_t mnemonicIdx = 29;
   mol_set_mnemonic_index(mnemonicIdx);
}

LIM {
   uint8_t mnemonicIdx = 30;
   mol_set_mnemonic_index(mnemonicIdx);
}

BITT {
   uint8_t mnemonicIdx = 31;
   mol_set_mnemonic_index(mnemonicIdx);
}

DATA {
   uint8_t mnemonicIdx = 32;
   mol_set_mnemonic_index(mnemonicIdx);
}


   /*----------------------
     INSTRUCTION OPERANDS 
   ----------------------*/

H {
   mol_set_target_byte_sel(2);
}

L {
   mol_set_target_byte_sel(1);
}

S {
   mol_set_target_byte_sel(0);
}

W {
   mol_set_target_byte_sel(3);
}

${DIGIT_HEX}+ {
   mol_set_literal(yytext + 1, 16);
}

@{DIGIT_OCTAL}+ {
   mol_set_literal(yytext + 1, 8);
}

[%]{DIGIT_BINARY}+ {
   mol_set_literal(yytext + 1, 2);
}

{DIGIT}+ {
   mol_set_literal(yytext, 10);
}

['](.+)['] {
   mol_set_literal(yytext, 0);
}

`LL {
   mol_set_literal_sel(0);
}

`LH {
   mol_set_literal_sel(1);
}

`HL {
   mol_set_literal_sel(2);
}

`HH {
   mol_set_literal_sel(3);
}

`LW {
   mol_set_literal_sel(4);
}

`HW {
   mol_set_literal_sel(5);
}

R{DIGIT}+ {
   mol_set_reg(yytext + 1);
}

{LABEL} {
   mol_set_identifier(yytext);
}

{INST_DELIM}+ {
   mol_handle_delim(yytext);
}

{EOL} {
   mol_generate();
}


   /*------------------
     DEBUG DIRECTIVES
   ------------------*/

@LINE:{WHITESPACE}*{DIGIT}+{EOL} {
   debug_set_line(yytext);
}

@PUSH_FILE:{WHITESPACE}*{FILENAME}{EOL} {
   debug_push_file(yytext);
}

@POP_FILE:{WHITESPACE}*{FILENAME}{EOL} {
   debug_pop_file(yytext);
}

@PUSH_MACRO:{WHITESPACE}*{LABEL}:{FILENAME}{EOL} {
   debug_push_macro(yytext);
}

@POP_MACRO:{WHITESPACE}*{LABEL}{EOL} {
   debug_pop_macro(yytext);
}


   /*-----------------------------------------------------------------------------*/

. {
   mol_handle_unexpected(yytext);
}


%%
