#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Defining some constants */
#define TRUE		1
#define FALSE		0
#define ERROR		-1
#define NORW		22
#define TXMAX		100
#define NMAX		14
#define AL		10
#define CHARS		120
#define AMAX		2047
#define STACKSIZE 500
#define LEVMAX		20
#define CXMAX		200

/* Discrete structure definitions */
enum objects
{
	Constant, Variable, Procedure
};
typedef enum objects OBJECTS;

enum functions
{
	LIT, OPR, LOD, STO, CAL, INT, JMP, JPC
};
typedef enum functions FUNCTIONS;

enum symbol
{
	BEGINSYM, CALLSYM, CASESYM, CENDSYM, CONSTSYM, DOSYM,
	DOWNTOSYM, ELSESYM, ENDSYM, FORSYM, IFSYM, ODDSYM,
	OFSYM, PROCSYM, REPEATSYM, THENSYM, TOSYM, UNTILSYM,
	VARSYM, WHILESYM, WRITELNSYM, WRITESYM, NUL, IDENT,
	NUMBER, PLUS, MINUS, TIMES, SLASH, EQL, NEQ, LSS, LEQ,
	GTR, GEQ, LPAREN, RPAREN, COMMA, SEMICOLON, COLON,
	PERIOD, BECOMES
};
typedef enum symbol SYMBOL;

/* Table structures for code and symbols */
typedef struct table_struct
{
	char name[AL];
	OBJECTS kind;
	int val, level, adr;
} TABLE;

typedef struct instr_struct
{
	FUNCTIONS f;
	int lvl, ad;
} INSTRUCTION;

/* Function Prototypes */
int	Position(char *, int);
void	Statement(int, int);
void	Expression(int, int);
void	ListCode(void);
void	Error(int);
void	Block(int, int);
void	GetSym(void);
void	Condition(int, int);
void	Enter(OBJECTS, int, int *, int *);
void	ConstDeclaration(int, int *, int *);
void	VarDeclaration(int, int *, int *);

/* Initializing list of reserved words */
static char Word[NORW][AL] =
{
	{ "BEGIN"	  },
	{ "CALL"	   },
	{ "CASE" },
	{ "CEND"   },
	{ "CONST" },
	{ "DO"	  },
	{ "DOWNTO"	   },
	{ "ELSE" },
	{ "END"    },
	{ "FOR"	  },
	{ "IF"	   },
	{ "ODD"    },
	{ "OF"	  },
	{ "PROCEDURE" },
	{ "REPEAT" },
	{ "THEN"  },
	{ "TO"	  },
	{ "UNTIL"	   },
	{ "VAR"    },
	{ "WHILE" },
	{ "WRITE"     },
	{ "WRITELN"  }

};

/* Initialization list of discrete symbols */
static SYMBOL wsym[NORW] =
{
	BEGINSYM, CALLSYM, CASESYM, CENDSYM, CONSTSYM, DOSYM,
	DOWNTOSYM, ELSESYM, ENDSYM, FORSYM, IFSYM, ODDSYM, OFSYM,
	PROCSYM, REPEATSYM, THENSYM, TOSYM, UNTILSYM, VARSYM,
	WHILESYM, WRITESYM, WRITELNSYM
};

/* Global Variables */
TABLE table[TXMAX];
char ch, save_char;
char id[AL], a[AL];
char line[80];
int cc;
int ll;
int kk;
int num;
int codeinx, codeinx0;
SYMBOL sym;
SYMBOL ssym[CHARS] = { NUL };
INSTRUCTION code[CXMAX];
char mneumonic[11][5];
int Dx;


/* Initializing list of error messages */
const char *ErrMsg[] =
{
	"Use = instead of :=", /* 1 */
	"= must be followed by a number", /* 2 */
	"Identifier must be followed by =",/* 3 */
	"Const, Var, Procedure must be followed by an identifier",/* 4 */
	"Semicolon or comma missing",/* 5 */
	"Incorrect symbol after procedure declaration",/* 6 */
	"Statement expected",/* 7 */
	"Incorrect symbol after statement part in block",/* 8 */
	"Period expected", /* 9 */
	"Semicolon between statements is missing",/* 10 */
	"Undeclared identifier",/* 11 */
	"Assignment to constant or procedure is not allowed",/* 12 */
	"Assignment operator := expected",/* 13 */
	"Call must be followed by an identifier",/* 14 */
	"Call of a constant or a variable is meaningless",/* 15 */
	"Then expected",/* 16 */
	"Semicolon or end expected",/* 17 */
	"Do expected",/* 18 */
	"Incorrect symbol following statement",/* 19 */
	"Relational operator expected",/* 20 */
	"Procedure cannot return a value",/* 21 */
	"Right parenthesis or relational operator expected",/* 22 */
	"Until expected",/* 23 */
	"Maximum number of levels reached",/* 24 */
	"Of expected following identifier for case",/* 25 */
	"To or Downto expected",/* 26 */
	"For statement must be followed by an identifier",/* 27 */
	"Constant or number expected",/* 28 */
	"Colon expected",/* 29 */
	"Number is too large",/* 30 */
	"Left parenthesis expected",/* 31 */
	"Identifier expected",/* 32 */
	"An expression cannot begin with this symbol"/* 33 */
};

/* Simple Error Outputting Function */
void Error(int ErrorNumber)
{
	fputs(ErrMsg[ErrorNumber - 1], stdout);
	fputs("\n", stdout);
	exit(ERROR);
}

/* Low Level Code Generating Function */
void Gen(FUNCTIONS x, int y, int z)
{
	if (codeinx > CXMAX)
	{
		fprintf(stdout, "Program too long.\n");
		exit(ERROR);
	}
	code[codeinx].f   = x;
	code[codeinx].lvl = y;
	code[codeinx].ad  = z;
	codeinx++;
}

/* Low Level Code Outputting Function */
void ListCode(void)
{
	int i;
	fprintf(stdout, "\n");
	for (i = codeinx0; i < codeinx; i++)
		fprintf(stdout, "%d %s %d %d\n", i, mneumonic[code[i].f],
		        code[i].lvl, code[i].ad);
}

/* Function to find new base */
int Base(int *s, int L, int b)
{
	int b1;
	b1 = b;
	while (L > 0)
	{
		b1 = s[b1];
		L--;
	}
	return b1;
}

/* Code Interpretting Function */
void Interpret(void)
{
	int p, b, t;
	INSTRUCTION i;
	int s[STACKSIZE] = { 0 };

	fprintf(stdout, "Start PL/0\n");
	t = 0;
	b = 1;
	p = 0;
	s[1] = 0;
	s[2] = 0;
	s[3] = 0;
	do
	{
		i = code[p];
		p++;
		switch (i.f)
		{
			case LIT:
				t++;
				s[t] = i.ad;
				continue;
			case OPR:
				switch (i.ad)
				{
					case 0:
						t = b - 1;
						b = s[t + 2];
						p = s[t + 3];
						continue;
					case 1:
						s[t] = -s[t];
						continue;
					case 2:
						t--;
						s[t] = s[t] + s[t + 1];
						continue;
					case 3:
						t--;
						s[t] = s[t] - s[t + 1];
						continue;
					case 4:
						t--;
						s[t] = s[t] * s[t + 1];
						continue;
					case 5:
						t--;
						s[t] = s[t] / s[t + 1];
						continue;
					case 6:
						s[t] = (s[t] % 2 > 0);
						continue;
					case 8:
						t--;
						s[t] = (s[t] == s[t + 1]);
						continue;
					case 9:
						t--;
						s[t] = (s[t] != s[t + 1]);
						continue;
					case 10:
						t--;
						s[t] = (s[t] < s[t + 1]);
						continue;
					case 11:
						t--;
						s[t] = (s[t] >= s[t + 1]);
						continue;
					case 12:
						t--;
						s[t] = (s[t] > s[t + 1]);
						continue;
					case 13:
						t--;
						s[t] = (s[t] <= s[t + 1]);
						continue;
					case 14:
						fprintf(stdout, "%d ", s[t]);
						t--;
						continue;
					case 15:
						fprintf(stdout, "\n");
						continue;
				}
				continue;
			case LOD:
				t++;
				s[t] = s[Base(s, i.lvl, b) + i.ad];
				continue;
			case STO:
				s[Base(s, i.lvl, b) + i.ad] = s[t];
				t--;
				continue;
			case CAL:
				s[t + 1] = Base(s, i.lvl, b);
				s[t + 2] = b;
				s[t + 3] = p;
				b = t + 1;
				p = i.ad;
				continue;
			case INT:
				t = t + i.ad;
				continue;
			case JMP:
				p = i.ad;
				continue;
			case JPC:
				if (s[t] == i.lvl)
					p = i.ad;
				t--;
				continue;
		}
	} while (p != 0);
	fprintf(stdout, "End PL/0\n");
}

void GetChar(void)
{
	if (cc == ll)
	{
		if (feof(stdin))
		{
			if (sym == ENDSYM && save_char == '.')
			{
				sym = PERIOD;
				ch = '.';
			}
			else
				if (sym != PERIOD)
					fprintf(stdout, "Program Incomplete.\n");
		}
		else
		{
			ll = 0;
			cc = 0;
			fgets(line, sizeof(line), stdin);
			fprintf(stdout, "%s", line);
			ll = strlen(line) - 1;
			save_char = line[ll];
			line[ll++] = ' ';
			if (ll != cc)
			{
				ch = line[cc++];
				while ((ch == '\t') && (cc != ll))
					ch = line[cc++];
			}
		}
	}
	else
	{
		if (ll != cc)
		{
			ch = line[cc++];
			while ((ch == '\t') && (cc != ll))
				ch = line[cc++];
		}
	}
}

void GetSym(void)
{
	int i, j, k;
	while (ch == ' ' || ch == '\r' || ch == '\n')
		GetChar();
	if (ch >= 'A' && ch <= 'Z')
	{
		k = 0;
		{	/* Clear out temp arrays to ensure string
			   comparison come out right */
			int x = 0;
			for (; x < AL; x++)
			{
				a[x]  = '\0';
				id[x] = '\0';
			}
		}
		if (k < AL)
			a[k++] = ch;
		GetChar();
		while ((ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9'))
		{
			if (k < AL)
				a[k++] = ch;
			if (cc != ll)
				GetChar();
			else
				ch = ' ';
		}

		if (k >= kk)
			kk = k;
		else
		{
			do
			{
				a[kk] = ' ';
				kk = kk - 1;
			} while (kk != k);
		}
		strncpy(id, a, strlen(a));
		i = 0;
		j = NORW;
		do
		{
			k = (i + j) / 2;
			if (id[0] <= Word[k][0])
				if ((strcmp(id, Word[k])) <= 0)
					j = k - 1;
			if (id[0] >= Word[k][0])
				if ((strcmp(id, Word[k])) >= 0)
					i = k + 1;
		} while (i <= j);


		if (i - 1 > j)
			sym = wsym[k];
		else
			sym = IDENT;
	}
	else if (ch >= '0' && ch <= '9')
	{
		k = 0;
		num = 0;
		sym = NUMBER;
		do
		{
			if (k >= NMAX)
				Error(30);
			num = 10 * num + (ch - '0');
			k++;
			GetChar();
		} while (ch >= '0' && ch <= '9');
	}
	else if (ch == ':')
	{
		GetChar();
		if (ch == '=')
		{
			sym = BECOMES;
			GetChar();
		}
		else
			sym = COLON;
	}

	else if (ch == '<')
	{
		GetChar();
		if (ch == '=')
		{
			sym = LEQ;
			GetChar();
		}
		else if (ch == '>')
		{
			sym = NEQ;
			GetChar();
		}
		else
			sym = LSS;
	}
	else if (ch == '>')
	{
		GetChar();
		if (ch == '=')
		{
			sym = GEQ;
			GetChar();
		}
		else
			sym = GTR;
	}
	else
	{
		sym = ssym[ch];
		GetChar();
	}
}

void Block(int lev, int tx)
{
	int dx = 3, tabinx0 = tx;

	table[tx].adr = codeinx;
	Gen(JMP, 0, 0);
	if (lev > LEVMAX)
		Error(24);

	while ((sym == CONSTSYM) || (sym == VARSYM) || (sym == PROCSYM))
	{
		if (sym == CONSTSYM)
		{
			GetSym();
			ConstDeclaration(lev, &dx, &tx);
			while (sym == COMMA)
			{
				GetSym();
				ConstDeclaration(lev, &dx, &tx);
			}
			if (sym == SEMICOLON)
				GetSym();
			else
				Error(5);
		} /* End if (CONSTSYM) */

		if (sym == VARSYM)
		{
			GetSym();
			VarDeclaration(lev, &dx, &tx);
			while (sym == COMMA)
			{
				GetSym();
				VarDeclaration(lev, &dx, &tx);
			}

			if (sym == SEMICOLON)
				GetSym();
			else
				Error(5);
		} /* END if (VARSYM) */



		while (sym == PROCSYM)
		{
			GetSym();
			if (sym == IDENT)
			{
				Enter(Procedure, lev, &dx, &tx);
				GetSym();
			}
			else
				Error(6);
			if (sym == SEMICOLON)
				GetSym();
			else
				Error(5);
			Block(lev + 1, tx);
			if (sym == SEMICOLON)
				GetSym();
			else
				Error(5);
		}
	}

	/* mark up first jump to the proper place */
	code[table[tabinx0].adr].ad = codeinx;
	table[tabinx0].adr = codeinx;
	codeinx0 = codeinx;
	Gen(INT, 0, dx);
	Statement(lev, tx);
	Gen(OPR, 0, 0);
	/* Print out code for each block */
	ListCode();
}

void Enter(OBJECTS k, int lev, int *dx, int *tx)
{
	int Dx = *dx, Tx = *tx;
	Tx++;
	strcpy(table[Tx].name, id);
	table[Tx].kind = k;
	switch (k)
	{
		case Constant:
			if (num > AMAX)
				Error(30);
			table[Tx].val = num;
			break;
		case Variable:
			table[Tx].level = lev;
			table[Tx].adr = Dx++;
			break;
		case Procedure:
			table[Tx].level = lev;
			break;
	}
	*dx = Dx;
	*tx = Tx;
}

int Position(char id[CHARS], int tx)
{
	int i;
	strcpy(table[0].name, id);
	i = tx;
	while (strcmp(table[i].name, id) != 0)
		i--;
	return i;
}

void Factor(int lev, int tx)
{
	int i;

	if (sym == IDENT)
	{
		if ((i = Position(id, tx)) == FALSE)
			Error(11);
		switch (table[i].kind)
		{
			case Constant :
				Gen(LIT, 0, table[i].val);
				break;
			case Variable :
				Gen(LOD, lev - table[i].level, table[i].adr);
				break;
			case Procedure:
				Error(21);
				break;
		}
		GetSym();
	}
	else if (sym == NUMBER)
	{
		GetSym();
		Gen(LIT, 0, num);
	}
	else if (sym == LPAREN)
	{
		GetSym();
		Expression(lev, tx);
		if (sym == RPAREN)
			GetSym();
		else
			Error(22);
	}
	else
		Error(32);
}

void Term(int lev, int tx)
{
	SYMBOL  multop;

	Factor(lev, tx);
	while (sym == TIMES || sym == SLASH)
	{
		multop = sym;
		GetSym();
		Factor(lev, tx);
		switch (multop)
		{
			case TIMES:
				Gen(OPR, 0, 4);
				break;
			case SLASH:
				Gen(OPR, 0, 5);
				break;
		}
	}
}

void Expression(int lev, int tx)
{
	SYMBOL addop;

	if (sym == PLUS || sym == MINUS)
	{
		addop = sym;
		GetSym();
		Term(lev, tx);
		if (addop == MINUS)
			Gen(OPR, 0, 1);
	}
	else
		Term(lev, tx);
	while (sym == PLUS || sym == MINUS)
	{
		addop = sym;
		GetSym();
		Term(lev, tx);
		switch (addop)
		{
			case PLUS:
				Gen(OPR, 0, 2);
				break;
			case MINUS:
				Gen(OPR, 0, 3);
				break;
		}
	}
}

void Condition(int lev, int tx)
{
	SYMBOL tmp;

	if (sym == ODDSYM)
	{
		GetSym();
		Expression(lev, tx);
		Gen(OPR, 0, 6);
	}
	else
	{
		Expression(lev, tx);
		if ((sym == EQL) || (sym == GTR) || (sym == LSS) || (sym == NEQ) || (sym == LEQ) || (sym == GEQ))
		{
			tmp = sym;
			GetSym();
			Expression(lev, tx);
			switch (tmp)
			{
				case EQL:
					Gen(OPR, 0, 8);
					break;
				case NEQ:
					Gen(OPR, 0, 9);
					break;
				case LSS:
					Gen(OPR, 0, 10);
					break;
				case GEQ:
					Gen(OPR, 0, 11);
					break;
				case GTR:
					Gen(OPR, 0, 12);
					break;
				case LEQ:
					Gen(OPR, 0, 13);
					break;
			}
		}
		else
			Error(20);
	}
}

void ConstDeclaration(int lev, int *dx, int *tx)
{
	int Dx = *dx;
	int Tx = *tx;

	if (sym == IDENT)
	{
		GetSym();
		if (sym == EQL)
		{
			GetSym();
			if (sym == NUMBER)
			{
				Enter(Constant, lev, &Dx, &Tx);
				GetSym();
			}
			else
				Error(2);
		}
		else
			Error(3);
	}
	else
		Error(4);
	*dx = Dx;
	*tx = Tx;
}

void VarDeclaration(int lev, int *dx, int *tx)
{
	int Dx = *dx;
	int Tx = *tx;

	if (sym == IDENT)
	{
		Enter(Variable, lev, &Dx, &Tx);
		GetSym();
	}
	else
		Error(4);
	*dx = Dx;
	*tx = Tx;
}

void Statement(int lev, int tx)
{
	int i, cx1, cx2;

	switch (sym)
	{
		case BEGINSYM:
			GetSym();
			Statement(lev, tx);
			while (sym == SEMICOLON)
			{
				GetSym();
				Statement(lev, tx);
			}
			if (sym == ENDSYM)
				GetSym();
			else
				Error(17);
			break;

		case IDENT:
			if ((i = Position(id, tx)) == FALSE)
				Error(11);
			else
				if (table[i].kind != Variable)
					Error(12);
			GetSym();
			if (sym == BECOMES)
				GetSym();
			else
				Error(13);
			Expression(lev, tx);
			Gen(STO, lev - table[i].level, table[i].adr);
			break;

		case IFSYM:
			GetSym();
			Condition(lev, tx);
			cx1 = codeinx;
			Gen(JPC, 0, 0);
			if (sym == THENSYM)
				GetSym();
			else
				Error(16);
			Statement(lev, tx);
			code[cx1].ad = codeinx;
			/* Place Your Code for Else Here */
			break; /* IFSYM */

		case WHILESYM:
			GetSym();
			cx1 = codeinx;
			Condition(lev, tx);
			cx2 = codeinx;
			Gen(JPC, 0, 0);
			if (sym == DOSYM)
			{
				GetSym();
				Statement(lev, tx);
				Gen(JMP, 0, cx1);
				code[cx2].ad = codeinx;
			}
			else
				Error(18);
			break; /* WHILESYM */

		case REPEATSYM:
			/* Place Your Code for Repeat-Until Here */
			break; /* REPEATSYM */

		case WRITELNSYM:
			/* Place Your Code for Writeln Here */
			break; /* WRITELNSYM */

		case CALLSYM:
			GetSym();
			if (sym == IDENT)
			{
				if ((i = Position(id, tx)) == FALSE) Error(11);
				else
					if (table[i].kind != Procedure)
						Error(15);
				Gen(CAL, lev - table[i].level, table[i].adr);
				GetSym();
			}
			else
				Error(14);
			break;

		case CASESYM:
			/* Place Your Code for Case Here */
			break; /* CASESYM */

		case FORSYM:
			/* Place Your Code for For-Do Here */
			break; /* FORSYM */

		case WRITESYM:
			/* Place Your Code for Write Here */
			break;
	}
}

/* Beginning Main Function */
int main(void)
{
	/* Assign the operatorsr to matching discrete type */
	ssym['+'] = PLUS;
	ssym['-'] = MINUS;
	ssym['*'] = TIMES;
	ssym['/'] = SLASH;
	ssym['('] = LPAREN;
	ssym[')'] = RPAREN;
	ssym['='] = EQL;
	ssym['#'] = NEQ;
	ssym['.'] = PERIOD;
	ssym[','] = COMMA;
	ssym['<'] = LSS;
	ssym['>'] = GTR;
	ssym['"'] = LEQ;
	ssym['@'] = GEQ;
	ssym[';'] = SEMICOLON;
	ssym[':'] = COLON;

	strncpy(mneumonic[LIT], "LIT", 3);
	strncpy(mneumonic[OPR], "OPR", 3);
	strncpy(mneumonic[LOD], "LOD", 3);
	strncpy(mneumonic[CAL], "CAL", 3);
	strncpy(mneumonic[INT], "INT", 3);
	strncpy(mneumonic[JMP], "JMP", 3);
	strncpy(mneumonic[JPC], "JPC", 3);
	strncpy(mneumonic[STO], "STO", 3);

	ll = 0;
	cc = ll;
	ch = ' ';
	kk = AL;

	/* Get First symbol from file to start parsing */
	GetSym();
	Block(0, 0);
	/* Make sure last symbol is a period in program */
	if (sym != PERIOD)
		Error(9);
	else
		fprintf(stdout, "\nSuccessful compilation!\n\n");

	/* Interpret code after syntax is parsed */
	Interpret();

	/* Close the data file */
	return FALSE;
}
/* Ending Main Function */

