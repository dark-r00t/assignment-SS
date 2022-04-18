/* 
	This is the header file for Almalki's Spring 2022 Systems Software Project.
	If you choose to alter this, you MUST make a note of that in your
	readme file, otherwise you will lose 5 points.
*/

typedef enum token_type {
	periodsym = 1, varsym, lbracketsym, procsym, rbracketsym, 
	callsym, commasym, beginsym, semicolonsym, endsym, 
	assignsym, ifsym, questionsym, dosym, colonsym, 
	readsym, lparenthesissym, writesym, rparenthesissym, identsym, 
	eqlsym, numbersym, neqsym, modsym, lsssym, 
	divsym, leqsym, multsym, gtrsym, subsym, 
	geqsym, addsym, whilesym
} token_type;

typedef struct lexeme {
	char name[12];
	int value;
	token_type type;
} lexeme;

typedef struct instruction {
	int opcode;
	int r;
	int l;
	int m;
} instruction;

typedef struct symbol {
	int kind;			// 1 for variables, 2 for arrays, 3 for procedures
	char name[12];
	int size;			// length of array 
	int level;
	int addr;			// address for variables, first entry address for arrays, first instruction index for procedures
	int mark;			// if the symbol is active
} symbol;

lexeme *lexanalyzer(char *input, int printFlag);
instruction *parse(lexeme *list, int printTable, int printCode);
void execute_program(instruction *code, int printFlag);