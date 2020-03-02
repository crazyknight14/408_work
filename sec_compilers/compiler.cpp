// -------------------------------------------
// Program: pascomp.cc (Version 1.8)
//
// Description: This is a pascal like compiler
// Student: Your Name Here
// -------------------------------------------
#include <fstream.h>    // for handling files
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <iomanip.h>

// -------------------------------------------
// These are some global definitions
// -------------------------------------------
#define TRUE		1
#define FALSE		0

#define MAX_TABLE 	500
#define MAX_PCODE	1000
#define MAX_SYMBOL	47
#define MAX_SYM_SZ	20
#define MAX_PRAMS	31
#define MAX_STACK	1000

// -------------------------------------------
// These are some ERROR definitions
// -------------------------------------------
#define ERROR_SEMICOLON		1
#define ERROR_IDENT		2
#define ERROR_UNKNOWN		3
#define ERROR_BECOMES		4
#define ERROR_BECOMES_PROC	5
#define ERROR_PROCEDURE		6
#define ERROR_END_SYM		7
#define ERROR_DO_SYM		8
#define ERROR_THEN_SYM		9
#define ERROR_UNTIL_SYM		10
#define ERROR_VARIABLE		11
#define ERROR_OF_SYM		12
#define ERROR_CONSTANT		13
#define ERROR_COLON		14
#define ERROR_RPAREN		15
#define ERROR_IS_PROCEDURE	16
#define ERROR_REL		17
#define ERROR_PROG_SIZE		18
#define ERROR_END_PROG		19
#define ERROR_NUMBER		20
#define ERROR_LPAREN		23
#define ERROR_UNTIL             25
#define ERROR_TO_DOWNTO   	26
#define ERROR_OF             27
#define ERROR_NUMBER_IDENT      28
#define ERROR_CEND           29
#define ERROR_NOPROCEDURE       30

// -------------------------------------------
// some enumerated data types
// -------------------------------------------
typedef enum tag_symbol
{
	VARSYM,
	CONSTSYM,
	BEGINSYM,
	ENDSYM,
	CENDSYM,
	PERIOD,
	SEMICOLON,
	COLON,
	LPAREN,
	RPAREN,
	GRTHEN,
	LSTHEN,
	GREQL,
	LSEQL,
	EQL,
	BECOMES,
	IFSYM,
	IDENT,
	NUMBER,
	WRITESYM,
	WRITELNSYM,
	PROCSYM,
	NOTEQL,
	MINUS,
	PLUS,
	DIV,
	MULT,
	COMMA,
	ODDSYM,
	CALL,
	ELSESYM,
	THENSYM,
	WHILESYM,
	DOSYM,
	REPEATSYM,
	UNTILSYM,
	CASESYM,
	OFSYM,
	CEND,
	FORSYM,
	DOWNTOSYM,
	TOSYM
}symbol;

// -------------------------------------------
typedef enum tag_Objtype
{
	NOTYPE,
	CONSTANT,
	VARIABLE,
	PROCEDURE
}Objtype;

// -------------------------------------------
typedef enum tag_psyms
{
	OPR,
	CAL,
	INT,
	JPC,
	JMP,
	LIT,
	LOD,
	STO
}psyms;

typedef enum tag_intype
{
	ALPHA,
	DIGIT,
	EOL,
	NONE,
	PUNCT,
	SPACE
}intype;

// -------------------------------------------
typedef struct tag_symtable    // Symbols Table Structure
{
	char name[MAX_SYM_SZ];     // Symbol name
	Objtype kind;         	// Type of symbol
	int value,        	// Value of symbol
	level,        	// Level of symbol
	adr;          	// Symbol address
}symtable;

// -------------------------------------------
typedef struct tag_pcodetable   // PCODE Table Structure
{
	psyms function;       // pcode function
	int   level,          // pcode level static link
	adr;          // pcode address number into table
}pcodetable;//used to save the compile code

// -------------------------------------------
// These are some global variables
// -------------------------------------------
int  linelength,        // line length
linecount,         // line counter
charcount,         // a character counter
codeinx,           // pcode index counter inc everytime
codeinx0,          // init pcode index
varcount0,         // Init variable counter
number;

symbol prevsym;         // holds the previous symbol
symtable table [MAX_TABLE];     // table array
pcodetable pcode [MAX_PCODE];     // pcode table
char line  [MAX_SYM_SZ];      // an identification string
char punc  [MAX_SYM_SZ];      // punction array
char symstr[MAX_SYMBOL][MAX_SYM_SZ];  // symbols array

// -------------------------------------------
// These are some function prototypes
// -------------------------------------------
void   block(symbol &, int, int);
void   statement(symbol &, int, int);
void   condition(symbol &, int, int);
void   expression(symbol &, int, int);
void   term(symbol &, int, int);
void   factor(symbol &, int, int);
char   getchar(char   &);
void   getsym(symbol &);
intype chartype(char  ch);
void   enter(Objtype, char[], symbol &, int &, int, int &);
int    base(int, int, int &);
void   gen(psyms, int, int);
void   interpret(void);
int    position(int);
void   showpcode(void);
void   openfile(void);
void   error(int);

// -------------------------------------------
// This handles our system errors
// -------------------------------------------
void error(int num)
{
	cout << endl;

	switch (num)
	{
		case ERROR_NOPROCEDURE :
			cout << "Procedure not accepted here";
			break;
		case ERROR_CEND :
			cout << "cend sym expected";
			break;
		case ERROR_NUMBER_IDENT :
			cout << "number or ident expected";
			break;
		case ERROR_OF :
			cout << "of expected";
			break;
		case ERROR_TO_DOWNTO :
			cout << "to or downto expected";
			break;
		case ERROR_UNTIL :
			cout << "Until expected";
			break;
		case ERROR_BECOMES :
			cout << "Assignment operator expected";
			break;
		case ERROR_BECOMES_PROC :
			cout << "Assignment not allowed here";
			break;
		case ERROR_COLON :
			cout << "COLON Expected";
			break;
		case ERROR_CONSTANT :
			cout << "Constant Expected";
			break;
		case ERROR_END_PROG :
			cout << "Premature end of program";
			break;
		case ERROR_DO_SYM :
			cout << "DO symbol Expected";
			break;
		case ERROR_END_SYM :
			cout << "END symbol Expected";
			break;
		case ERROR_REL :
			cout << "Relational operator expected";
			break;
		case ERROR_IDENT :
			cout << "Identifier Expected";
			break;
		case ERROR_IS_PROCEDURE :
			cout << "Assignment to PROCEDURE not allowed";
			break;
		case ERROR_NUMBER :
			cout << "A number was Expected";
			break;
		case ERROR_PROG_SIZE :
			cout << "Program size is too large...";
			break;
		case ERROR_OF_SYM :
			cout << "OF symbol Expected";
			break;
		case ERROR_RPAREN :
			cout << "RIGHT Parenthesis Expected";
			break;
		case ERROR_LPAREN :
			cout << "LEFT Parenthesis Expected";
			break;
		case ERROR_SEMICOLON :
			cout << "Semicolon Expected" ;
			break;
		case ERROR_THEN_SYM :
			cout << "THEN symbol Expected";
			break;
		case ERROR_UNKNOWN :
			cout << "Unknown Identifier";
			break;
		case ERROR_UNTIL_SYM :
			cout << "UNTIL symbol Expected";
			break;
		case ERROR_VARIABLE :
			cout << "Variable or Expression Expected";
			break;
	}
	cout << endl;
	exit(1);
}

// -------------------------------------------
// Insert Block Identifier
// -------------------------------------------
void enter(Objtype kind, char name[], symbol &sym, int &varcount, int level, int &tableinx)
{
	tableinx++;
	strcpy(table[tableinx].name, name);
	table[tableinx].kind = kind;

	if (kind == CONSTANT)
	{
		if (sym != IDENT)
			error(ERROR_IDENT);
		getsym(sym);
		if (sym != EQL)
			error(ERROR_BECOMES);
		getsym(sym);
		if (sym != NUMBER)
			error(ERROR_NUMBER);
		table[tableinx].value = number;//for const only value matters
	}
	else if ((kind == VARIABLE))
	{
		if (sym != IDENT)
			error(ERROR_IDENT);

		table[tableinx].level = level;   // store variable level for static link level
		table[tableinx].adr = varcount++; // store address offset from base
	}
	else if (kind == PROCEDURE)
		table[tableinx].level = level;     // store procedure level

	getsym(sym);
}

// -------------------------------------------
// Locate Position
// -------------------------------------------
int position(int tableinx)
{
	int i = tableinx;
	for (strcpy(table[0].name, line); strcmp(table[i].name, line) != 0; i--);
	return i;
}

// -------------------------------------------
// Block
// -------------------------------------------
void block(symbol &sym, int level, int tableinx)
{
	int varcount = 3;//set to 3 for static link,dynamic link,and return address implicitly set
	int count = 0;

	int tableinx0 = tableinx;      // save symbol table count used to fix after CONST,VAR,and PROCEDURES DECLARE
	table[tableinx].adr = codeinx; // save pcode code index
	gen(JMP, 0, 0);                // Generare our init JMP Function,level,adr in pcode table

	while (sym == CONSTSYM || sym == VARSYM || sym == PROCSYM)
	{
		if (sym == CONSTSYM)
		{
			// ---- CONSTANT SYM ----
			getsym(sym);
			enter(CONSTANT, line, sym, varcount, level, tableinx);

			while (sym == COMMA)
			{
				getsym(sym);
				enter(CONSTANT, line, sym, varcount, level, tableinx);//int tableinx for all,set value of table line = name
			}
			if (sym != SEMICOLON)
				error(ERROR_SEMICOLON);

			getsym(sym);
		}
		// ---- VARIABLE SYM ----
		if (sym == VARSYM)
		{
			getsym(sym);
			enter(VARIABLE, line, sym, varcount, level, tableinx);//inc varcount and tableinx,used for INT 0,varcount
			while (sym == COMMA)
			{
				getsym(sym);
				enter(VARIABLE, line, sym, varcount, level, tableinx);
			}
			if (sym != SEMICOLON)
				error(ERROR_SEMICOLON);
			getsym(sym);

		}
		// ---- PROCEDURE SYM ----
		while (sym == PROCSYM)
		{
			getsym(sym);
			if (sym != IDENT)
				error(ERROR_IDENT);

			enter(PROCEDURE, line, sym, varcount, level, tableinx);//int tableinx and add level
			getsym(sym);

			block(sym, level + 1, tableinx);//inc static link for functions inside of functions, table current pointer

			if (sym != SEMICOLON)
				error(ERROR_SEMICOLON);

			getsym(sym);
		}
	}

	pcode[table[tableinx0].adr].adr = codeinx;//tableinx0 is used to fix the original jump when u go into a block
	table[tableinx0].adr = codeinx;
	codeinx0 = codeinx;
	gen(INT, 0, varcount);//inc the top of the stack by 3 + number of variable
	statement(sym, level, tableinx);
	gen(OPR, 0, 0);

	showpcode();
}

// -------------------------------------------
// Statement
// -------------------------------------------
void statement(symbol &sym, int level, int tableinx)
{
	int i, cx1, cx2, cx3, cx4;
	int first;
	int count = 0;

	switch (sym)
	{
		// IDENT
		case IDENT:
			//A1:=8 * B2; example
			//checks to see if identifier is in table
			i = position(tableinx);
			if (i == 0)
				error(ERROR_UNKNOWN);

			switch (table[i].kind)
			{
				case VARIABLE:
					getsym(sym);
					if (sym != BECOMES)
						error(ERROR_BECOMES);
					getsym(sym);
					expression(sym, level, tableinx);//8 * B2 *example* put 8 and B2 onto stack and run OPR
					gen(STO, level - table[i].level, table[i].adr);
					break;
				default:
					error(ERROR_BECOMES_PROC);
			}
			break;

		// PROCEDURE CALL
		case CALL:
			getsym(sym);

			if (sym != IDENT)
				error(ERROR_IDENT);

			i = position(tableinx);

			if (i == 0)
				error(ERROR_UNKNOWN);

			if (table[i].kind != PROCEDURE)
				error(ERROR_PROCEDURE);

			getsym(sym);

			gen(CAL, level - table[i].level, table[i].adr);
			break;

		// BEGIN and END block
		case BEGINSYM:
			getsym(sym);
			statement(sym, level, tableinx);
			while (sym == SEMICOLON)
			{
				getsym(sym);
				statement(sym, level, tableinx);
			}
			if (sym != ENDSYM)
				error(ERROR_END_SYM);
			getsym(sym);
			break;

		// WHILE SYMBOL
		case WHILESYM:
			getsym(sym);
			cx1 = codeinx;
			condition(sym, level, tableinx);
			cx2 = codeinx;
			gen(JPC, 0, 0);
			if (sym != DOSYM)
				error(ERROR_DO_SYM);
			getsym(sym);
			statement(sym, level, tableinx);
			gen(JMP, 0, cx1);
			pcode[cx2].adr = codeinx;
			break;

		// IF - THEN - ELSE
		case IFSYM:
			getsym(sym);
			condition(sym, level, tableinx);
			cx1 = codeinx;
			gen(JPC, 0, 0);
			if (sym != THENSYM)
				error(ERROR_THEN_SYM);
			getsym(sym);
			statement(sym, level, tableinx);
			pcode[cx1].adr = codeinx;

			/* add your code for ELSE here */
			break;

		case REPEATSYM:
			/* add your code for REPEAT-UNTIL here */
			break;

		case FORSYM:
			/* add your code for FOR-DO here */
			break;

		case CASESYM:
			/* add your code for CASE here */
			break;

		case WRITESYM:
			/* add your code for WRITE here */
			break;

		case WRITELNSYM:
			/* add your code for WRITELN here */
			break;
	} // END of SWITCH
}

// -------------------------------------------
// Condition
// -------------------------------------------
void condition(symbol &sym, int level, int tableinx)
{
	symbol prevsym;

	// ODD symbol
	if (sym == ODDSYM)
	{
		getsym(sym);
		expression(sym, level, tableinx);
		gen(OPR, 0, 6);
	}
	else
	{
		expression(sym, level, tableinx);
		if ((sym == EQL) || (sym == NOTEQL) || (sym == LSTHEN) || (sym == LSEQL) || (sym == GRTHEN) || (sym == GREQL))
		{
			prevsym = sym;
			getsym(sym);
			expression(sym, level, tableinx);
			switch (prevsym)
			{
				case EQL:
					gen(OPR, 0, 8);
					break;
				case GREQL:
					gen(OPR, 0, 11);
					break;
				case GRTHEN:
					gen(OPR, 0, 12);
					break;
				case LSEQL:
					gen(OPR, 0, 13);
					break;
				case LSTHEN:
					gen(OPR, 0, 10);
					break;
				case NOTEQL:
					gen(OPR, 0, 9);
					break;
			}
		}
		else
			error(ERROR_REL);
	}
}

// -------------------------------------------
// Expression
// -------------------------------------------
void expression(symbol &sym, int level, int tableinx)
{
	symbol prevsym;

	if ((sym == PLUS) || (sym == MINUS))
	{
		prevsym = sym;
		getsym(sym);
		term(sym, level, tableinx);
		if (prevsym == MINUS)
			gen(OPR, 0, 1);
	}
	else
		term(sym, level, tableinx);
	while (sym == PLUS || sym == MINUS)
	{
		prevsym = sym;
		getsym(sym);
		term(sym, level, tableinx);
		if (prevsym == MINUS)
			gen(OPR, 0, 3);
		else
			gen(OPR, 0, 2);
	}
}

// -------------------------------------------
// TERM
// -------------------------------------------
void term(symbol &sym, int level, int tableinx)
{
	symbol prevsym;

	factor(sym, level, tableinx);
	while ((sym == MULT) || (sym == DIV))
	{
		prevsym = sym;
		getsym(sym);
		factor(sym, level, tableinx);
		if (prevsym == DIV)
			gen(OPR, 0, 5);
		else
			gen(OPR, 0, 4);
	}
}

// -------------------------------------------
// FACTOR
// -------------------------------------------
void factor(symbol &sym, int level, int tableinx)
{
	int i;
	int count = 0;

	switch (sym)
	{
		// IDENTIFER
		case IDENT:
			i = position(tableinx);
			if (i == 0)
				error(ERROR_UNKNOWN);
			if (table[i].kind == PROCEDURE)
				error(ERROR_IS_PROCEDURE);

			switch (table[i].kind)
			{
				case VARIABLE:
					gen(LOD, level - table[i].level, table[i].adr);
					break;
				case CONSTANT:
					gen(LIT, 0, table[i].value);
					break;
			}
			getsym(sym);
			break;

		// NUMBER
		case NUMBER:
			gen(LIT, 0, number);
			getsym(sym);
			break;

		// LEFT PARENTHESE
		case LPAREN:
			getsym(sym);
			expression(sym, level, tableinx);
			if (sym != RPAREN)
				error(ERROR_RPAREN);
			getsym(sym);
			break;

		default:
			error(ERROR_VARIABLE);
	}
}

// -------------------------------------------
// Generate the PCODE
// -------------------------------------------
void gen(psyms function, int level, int adr)
{
	if (codeinx > MAX_PCODE)
		error(ERROR_PROG_SIZE);
	pcode[codeinx].function = function;
	pcode[codeinx].level    = level;
	pcode[codeinx].adr      = adr;
	codeinx++;
}

// -------------------------------------------
// This is our GET CHARACTER function
// -------------------------------------------
char getchar(char &ch)
{
	static char line[255];           // local array
	if (charcount == linelength)
	{
		charcount = linelength = 0;       // zero out counters
		cin.get(ch);
		while (chartype(ch) != EOL && cin)
		{
			line[linelength] = ch;
			cin.get(ch);
			linelength++;
		}
		if (linelength == 0 && charcount == 0 && !cin)
			error(ERROR_END_PROG);

		line[linelength] = 0;
		cout << line << endl;

		line[linelength] = ' ';
		linecount++;                    // count lines
		linelength++;                   //
	}
	ch = toupper(line[charcount]);
	charcount++;                      // count characters
	return ch;
}

// -------------------------------------------
// This is our GETSYM
// -------------------------------------------
void getsym(symbol &sym)
{
	char ch;
	int index = 0;

	do
		getchar(ch);
	while (chartype(ch) == SPACE || chartype(ch) == EOL);

	if (chartype(ch) == ALPHA)
	{
		do
		{
			line[index++] = ch;
			getchar(ch);
		} while (chartype(ch) == ALPHA || chartype(ch) == DIGIT || ch  == '_');
		line[index] = '\0';
		charcount--;
		if (strcmp(line, "BEGIN") == 0)
			sym = BEGINSYM;
		else if (strcmp(line, "CALL") == 0)
			sym = CALL;
		else if (strcmp(line, "CASE") == 0)
			sym = CASESYM;
		else if (strcmp(line, "CONST") == 0)
			sym = CONSTSYM;
		else if (strcmp(line, "DO") == 0)
			sym = DOSYM;
		else if (strcmp(line, "CEND") == 0)
			sym = CENDSYM;
		else if (strcmp(line, "REPEAT") == 0)
			sym = REPEATSYM;
		else if (strcmp(line, "FOR") == 0)
			sym = FORSYM;
		else if (strcmp(line, "UNTIL") == 0)
			sym = UNTILSYM;
		else if (strcmp(line, "TO") == 0)
			sym = TOSYM;
		else if (strcmp(line, "ELSE") == 0)
			sym = ELSESYM;
		else if (strcmp(line, "DOWNTO") == 0)
			sym = DOWNTOSYM;
		else if (strcmp(line, "WRITELN") == 0)
			sym = WRITELNSYM;
		else if (strcmp(line, "WRITE") == 0)
			sym = WRITESYM;
		else if (strcmp(line, "END") == 0)
			sym = ENDSYM;
		else if (strcmp(line, "IF") == 0)
			sym = IFSYM;
		else if (strcmp(line, "ODD") == 0)
			sym = ODDSYM;
		else if (strcmp(line, "OF") == 0)
			sym = OFSYM;
		else if (strcmp(line, "PROCEDURE") == 0)
			sym = PROCSYM;
		else if (strcmp(line, "THEN") == 0)
			sym = THENSYM;
		else if (strcmp(line, "VAR") == 0)
			sym = VARSYM;
		else if (strcmp(line, "WHILE") == 0)
			sym = WHILESYM;
		else
			sym = IDENT;
																								strcpy(symstr[sym], line);
		return;
	}

	if (chartype(ch) == DIGIT)
	{
		char strnum[10];
		sym = NUMBER;
		number = 0;
		do
		{
			strnum[index++] = ch;
			number = 10 * number + int(ch - 48);
			getchar(ch);
		} while (chartype(ch) == DIGIT);
		charcount--;
		strnum[index] = '\0';
		strcpy(symstr[sym], strnum);
		return;
	}

	if (chartype(ch) == PUNCT)
	{
		punc[index++] = ch;
		if (ch == ':' || ch == '<' || ch == '>')
		{
			getchar(ch);
			if (chartype(ch) == PUNCT && (ch == '=') || (ch == '>'))
				punc[index++] = ch;
			else
				charcount--;
		}
		punc[index] = '\0';
		if (strcmp(punc, ":=") == 0)
			sym = BECOMES;
		else if (strcmp(punc, ":") == 0)
			sym = COLON;
		else if (strcmp(punc, ",") == 0)
			sym = COMMA;
		else if (strcmp(punc, "/") == 0)
			sym = DIV;
		else if (strcmp(punc, "=") == 0)
			sym = EQL;
		else if (strcmp(punc, ">=") == 0)
			sym = GREQL;
		else if (strcmp(punc, ">") == 0)
			sym = GRTHEN;
		else if (strcmp(punc, "(") == 0)
			sym = LPAREN;
		else if (strcmp(punc, "<=") == 0)
			sym = LSEQL;
		else if (strcmp(punc, "<") == 0)
			sym = LSTHEN;
		else if (strcmp(punc, "-") == 0)
			sym = MINUS;
		else if (strcmp(punc, "*") == 0)
			sym = MULT;
		else if (strcmp(punc, "<>") == 0)
			sym = NOTEQL;
		else if (strcmp(punc, ".") == 0)
			sym = PERIOD;
		else if (strcmp(punc, "+") == 0)
			sym = PLUS;
		else if (strcmp(punc, ")") == 0)
			sym = RPAREN;
		else if (strcmp(punc, ";") == 0)
			sym = SEMICOLON;

		strcpy(symstr[sym], punc);

		return;
	}
}

// -------------------------------------------
// determine a character type
// -------------------------------------------
intype chartype(char ch)
{
	if (ch == '\n' || ch == '\r')
		return EOL;        // character END-OF-LINE
	if (isspace(ch))
		return SPACE;      // character SPACE
	if (isdigit(ch))
		return DIGIT;      // character DIGIT
	if (isalpha(ch))
		return ALPHA;      // character ALPHA
	if (ispunct(ch))
		return PUNCT;      // character PUNCTUATION
	return NONE;
}

// -------------------------------------------
// Find the base
// -------------------------------------------
int base(int lev, int basereg, int stack [])
{
	int base1;
	for (base1 = basereg; lev > 0; base1 = stack[base1], lev--);
	return base1;
}

// -------------------------------------------
// This is our Interpreter
// -------------------------------------------
void interpret(void)
{
	int progreg = 0, basereg = 0, top = 0, stack[MAX_STACK] = {0};
	pcodetable inst;

	cout << endl << "Start PL/0" << endl;

	do
	{
		inst = pcode[progreg];
		progreg++;
		switch (inst.function)
		{
			case LIT:
				top++;
				stack[top] = inst.adr;
				continue;
			case OPR:
				switch (inst.adr)
				{
					case 0:
						top = basereg - 1;
						basereg = stack[top + 2];
						progreg = stack[top + 3];
						continue;
					case 1:
						stack[top] = -stack[top];
						continue;
					case 2:
						top--;
						stack[top] = stack[top] + stack[top + 1];
						continue;
					case 3:
						top--;
						stack[top] = stack[top] - stack[top + 1];
						continue;
					case 4:
						top--;
						stack[top] = stack[top] * stack[top + 1];
						continue;
					case 5:
						top--;
						stack[top] = stack[top] / stack[top + 1];
						continue;
					case 6:
						stack[top] = (stack[top] % 2 > 0);
						continue;
					case 8:
						top--;
						stack[top] = (stack[top] == stack[top + 1]);
						continue;
					case 9:
						top--;
						stack[top] = (stack[top] != stack[top + 1]);
						continue;
					case 10:
						top--;
						stack[top] = (stack[top] < stack[top + 1]);
						continue;
					case 11:
						top--;
						stack[top] = (stack[top] >= stack[top + 1]);
						continue;
					case 12:
						top--;
						stack[top] = (stack[top] > stack[top + 1]);
						continue;
					case 13:
						top--;
						stack[top] = (stack[top] <= stack[top + 1]);
						continue;
					case 14:
						cout << stack[top] << " ";
						top--;
						continue;
					case 15:
						cout << endl;
						continue;
				}
				continue;
			case LOD:
				top++;
				stack[top] = stack[base(inst.level,basereg,stack)+inst.adr];
				continue;
			case STO:
				stack[base(inst.level,basereg,stack)+inst.adr] = stack[top];
				top--;
				continue;
			case CAL:
				stack[top + 1] = base(inst.level, basereg, stack);
				stack[top + 2] = basereg;
				stack[top + 3] = progreg;
				basereg = top + 1;
				progreg = inst.adr;
				continue;
			case INT:
				top = top + inst.adr;
				continue;
			case JMP:
				progreg = inst.adr;
				continue;
			case JPC:
				if (stack[top] == inst.level)
					progreg = inst.adr;
				top--;
				continue;
		}
	} while (progreg != 0);
	cout << "End PL/0\n";
}

// -------------------------------------------
// This shows our PCODE
// -------------------------------------------
void showpcode(void)
{
	int startpoint;

	startpoint = codeinx0;

	cout << endl;
	for (int i = startpoint; i < codeinx; ++i)
	{
		cout << i << " ";

		switch (pcode[i].function)
		{
			case CAL :
				cout << "CAL ";
				break;
			case INT :
				cout << "INT ";
				break;
			case JMP :
				cout << "JMP ";
				break;
			case JPC :
				cout << "JPC ";
				break;
			case LIT :
				cout << "LIT ";
				break;
			case LOD :
				cout << "LOD ";
				break;
			case OPR :
				cout << "OPR ";
				break;
			case STO :
				cout << "STO ";
				break;
		}
		cout << pcode[i].level << ' ' << pcode[i].adr << endl;
	}
}

// -------------------------------------------
// This is our main program entry
// -------------------------------------------
int main(int argc, char* argv[])
{
	symbol sym;
	int i;
	char filen[40];
	// Initialize some variables
	linelength = 0;
	charcount  = 0;
	linecount  = 0;
	codeinx  = 0;

	getsym(sym);                  // get the first symbol
	block(sym, 0, 0);                 // start processing

	cout << "\nSuccessful compilation!\n";
	interpret();

	return 0;
}
