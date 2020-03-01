#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Defining some constants */
#define TRUE		1
#define FALSE		0
#define ERROR		-1
#define NORW		11
#define TXMAX		100
#define NMAX		14
#define AL		10
#define CHARS		120
#define AMAX		2047
#define LEVMAX		20

/* Discrete structure definitions */
enum objects
{
	Constant, Variable, Procedure
};
typedef enum objects OBJECTS;

enum symbol
{
	BEGINSYM, CALLSYM, CONSTSYM, DOSYM,
	ENDSYM, IFSYM, ODDSYM, PROCSYM, THENSYM,
	VARSYM, WHILESYM, NUL, IDENT, NUMBER,
	PLUS, MINUS, TIMES, SLASH, EQL, NEQ, LSS,
	LEQ, GTR, GEQ, LPAREN, RPAREN, COMMA,
	SEMICOLON, COLON, PERIOD, BECOMES
};
typedef enum symbol SYMBOL;

/* Table structures for code and symbols */
typedef struct table_struct
{
	char name[AL];
	OBJECTS kind;
} TABLE;

/* Function Prototypes */
int	Position(char *, int);
void	Statement(int);
void	Expression(int);
void	Error(int);
void	Block(int);
void	GetSym(void);
void	Condition(int);
void	Enter(OBJECTS, int *);
void	ConstDeclaration(int *);
void	VarDeclaration(int *);

/* Initializing list of reserved words */
static char Word[NORW][AL] =
{
	{ "BEGIN" },
	{ "CALL" },
	{ "CONST" },
	{ "DO" },
	{ "END" },
	{ "IF" },
	{ "ODD" },
	{ "PROCEDURE" },
	{ "THEN" },
	{ "VAR" },
	{ "WHILE" }
};

/* Initialization list of discrete symbols */
static SYMBOL wsym[NORW] =
{
	BEGINSYM, CALLSYM, CONSTSYM, DOSYM,
	ENDSYM, IFSYM, ODDSYM, PROCSYM,
	THENSYM, VARSYM, WHILESYM
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
SYMBOL sym;
SYMBOL ssym[CHARS] = { NUL };

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
	"Right parenthesis or relational operator expected",/* 21 */
	"Number is too large",/* 22 */
	"Left parenthesis expected",/* 23 */
	"Identifier expected",/* 24 */
	"An expression cannot begin with this symbol"/* 25 */
};

/* Simple Error Outputting Function */
void Error(int ErrorNumber)
{
	fputs(ErrMsg[ErrorNumber - 1], stdout);
	fputs("\n", stdout);
	exit(ERROR);
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

	while (ch == ' ' || ch == '\n' || ch == '\r')
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
				Error(22);
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

void Block(int tx)
{
	if (sym == CONSTSYM)
	{
		GetSym();
		ConstDeclaration(&tx);
		while (sym == COMMA)
		{
			GetSym();
			ConstDeclaration(&tx);
		}
		if (sym == SEMICOLON)
			GetSym();
		else
			Error(5);
	}
	if (sym == VARSYM)
	{
		GetSym();
		VarDeclaration(&tx);
		while (sym == COMMA)
		{
			GetSym();
			VarDeclaration(&tx);
		}
		if (sym == SEMICOLON)
			GetSym();
		else
			Error(5);
	}
	while (sym == PROCSYM)
	{
		GetSym();
		if (sym == IDENT)
		{
			Enter(Procedure, &tx);
			GetSym();
		}
		else
			Error(24);
		if (sym == SEMICOLON)
			GetSym();
		else
			Error(5);
		Block(tx);
		if (sym == SEMICOLON)
			GetSym();
		else
			Error(5);
	}

	Statement(tx);
}

void Enter(OBJECTS k, int *tx)
{
	int Tx = *tx;

	Tx++;
	strcpy(table[Tx].name, id);
	table[Tx].kind = k;
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

void Factor(int tx)
{
	int i;

	if (sym == IDENT)
	{
		if ((i = Position(id, tx)) == FALSE)
			Error(11);
		GetSym();
	}
	else if (sym == NUMBER)
		GetSym();
	else if (sym == LPAREN)
	{
		GetSym();
		Expression(tx);
		if (sym == RPAREN)
			GetSym();
		else
			Error(21);
	}
	else
		Error(25);
}

void Term(int tx)
{
	Factor(tx);
	while (sym == TIMES || sym == SLASH)
	{
		GetSym();
		Factor(tx);
	}
}

void Expression(int tx)
{
	if (sym == PLUS || sym == MINUS)
	{
		GetSym();
		Term(tx);
	}
	else
		Term(tx);
	while (sym == PLUS || sym == MINUS)
	{
		GetSym();
		Term(tx);
	}
}

void Condition(int tx)
{
	if (sym == ODDSYM)
	{
		GetSym();
		Expression(tx);
	}
	else
	{
		Expression(tx);
		if ((sym == EQL) || (sym == GTR) || (sym == LSS) || (sym == NEQ) || (sym == LEQ) || (sym == GEQ))
		{
			GetSym();
			Expression(tx);
		}
		else
			Error(20);
	}
}

void ConstDeclaration(int *tx)
{
	int Tx = *tx;

	if (sym == IDENT)
	{
		GetSym();
		if (sym == EQL)
		{
			GetSym();
			if (sym == NUMBER)
			{
				Enter(Constant, &Tx);
				GetSym();
			}
			else
				Error(2);
		}
		else
			Error(3);
	}
	else
		Error(24);
	*tx = Tx;
}

void VarDeclaration(int *tx)
{
	int Tx = *tx;

	if (sym == IDENT)
	{
		Enter(Variable, &Tx);
		GetSym();
	}
	else
		Error(24);
	*tx = Tx;
}

void Statement(int tx)
{
	int i;

	switch (sym)
	{
		case BEGINSYM:
			GetSym();
			Statement(tx);
			while (sym == SEMICOLON)
			{
				GetSym();
				Statement(tx);
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
			Expression(tx);
			break;
		case IFSYM:
			GetSym();
			Condition(tx);
			if (sym == THENSYM)
				GetSym();
			else
				Error(16);
			Statement(tx);
			break;
		case WHILESYM:
			GetSym();
			Condition(tx);
			if (sym == DOSYM)
			{
				GetSym();
				Statement(tx);
			}
			else
				Error(18);
			break;
		case CALLSYM:
			GetSym();
			if (sym == IDENT)
			{
				if ((i = Position(id, tx)) == FALSE)
					Error(11);
				else
					if (table[i].kind != Procedure)
						Error(15);
				GetSym();
			}
			else
				Error(14);
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

	ll = 0;
	cc = ll;
	ch = ' ';
	kk = AL;

	/* Get First symbol from file to start parsing */
	GetSym();
	Block(0);
	/* Make sure last symbol is a period in program */
	if (sym != PERIOD)
		Error(9);
	else
		fprintf(stdout, "\nSuccessful compilation!\n\n");

	return FALSE;
}
/* Ending Main Function */

