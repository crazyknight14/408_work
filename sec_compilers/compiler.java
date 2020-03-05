// **********************************************
//
// Pascal 0 Compiler Version 2 written in Java
//
// **********************************************

import java.util.*;
import java.io.*;

public class compiler
{
		static private final int TRUE = 1;
		static private final int FALSE = -1;
		static private final int NORW = 22;
		static private final int TXMAX = 100;
		static private final int NMAX = 14;
		static private final int AL = 20;
		static private final int AMAX = 2047;
		static private final int STACKSIZE = 500;
		static private final int LEVMAX = 20;
		static private final int CXMAX = 500;

		public static enum OBJECTS
		{
			Constant,
			Variable,
			Procedure,
			None
		}

		public static enum FUNCTIONS
		{
			LIT(0), OPR(1), LOD(2), STO(3), CAL(4), INT(5), JMP(6), JPC(7);

			private final int ordinal;

			FUNCTIONS(int init_ord)
			{
				ordinal = init_ord;
			}

			public int getValue()
			{
				return ordinal;
			}
		};

		public static enum SYMBOL
		{
			BEGINSYM,
			CALLSYM,
			CASESYM,
			CENDSYM,
			CONSTSYM,
			DOSYM,
			DOWNTOSYM,
			ELSESYM,
			ENDSYM,
			FORSYM,
			IFSYM,
			ODDSYM,
			OFSYM,
			PROCSYM,
			REPEATSYM,
			THENSYM,
			TOSYM,
			UNTILSYM,
			VARSYM,
			WHILESYM,
			WRITELNSYM,
			WRITESYM,
			NUL,
			IDENT,
			NUMBER,
			PLUS,
			MINUS,
			TIMES,
			SLASH,
			EQL,
			NEQ,
			LSS,
			LEQ,
			GTR,
			GEQ,
			LPAREN,
			RPAREN,
			COMMA,
			SEMICOLON,
			COLON,
			PERIOD,
			BECOMES
		}

		public static SYMBOL[] wsym =
		{
			SYMBOL.BEGINSYM,
			SYMBOL.CALLSYM,
			SYMBOL.CASESYM,
			SYMBOL.CENDSYM,
			SYMBOL.CONSTSYM,
			SYMBOL.DOSYM,
			SYMBOL.DOWNTOSYM,
			SYMBOL.ELSESYM,
			SYMBOL.ENDSYM,
			SYMBOL.FORSYM,
			SYMBOL.IFSYM,
			SYMBOL.ODDSYM,
			SYMBOL.OFSYM,
			SYMBOL.PROCSYM,
			SYMBOL.REPEATSYM,
			SYMBOL.THENSYM,
			SYMBOL.TOSYM,
			SYMBOL.UNTILSYM,
			SYMBOL.VARSYM,
			SYMBOL.WHILESYM,
			SYMBOL.WRITESYM,
			SYMBOL.WRITELNSYM
		};

		public static class table_struct
		{
				public String name;
				public OBJECTS kind;
				public int val, level, adr;

				public table_struct(String init_name, OBJECTS init_kind)
				{
					this.name = init_name;
					this.kind = init_kind;
				}
		}

		public static class INSTRUCTION
		{
				public FUNCTIONS f;
				public int lvl, ad;
		}

		public static String Word[] =
		{
			"BEGIN",
			"CALL",
			"CASE",
			"CEND",
			"CONST",
			"DO",
			"DOWNTO",
			"ELSE",
			"END",
			"FOR",
			"IF",
			"ODD",
			"OF",
			"PROCEDURE",
			"REPEAT",
			"THEN",
			"TO",
			"UNTIL",
			"VAR",
			"WHILE",
			"WRITE",
			"WRITELN"
		};

		public static char Char_Word[][] =
		{
			{'B', 'E', 'G', 'I', 'N'},
			{ 'C', 'A', 'L', 'L'},
			{ 'C', 'A', 'S', 'E'},
			{ 'C', 'E', 'N', 'D'},
			{ 'C', 'O', 'N', 'S', 'T'},
			{ 'D', 'O', },
			{ 'D', 'O', 'W', 'N', 'T', 'O'},
			{ 'E', 'L', 'S', 'E'},
			{ 'E', 'N', 'D'},
			{ 'F', 'O', 'R'},
			{ 'I', 'F'},
			{ 'O', 'D', 'D'},
			{ 'O', 'F'},
			{ 'P', 'R', 'O', 'C', 'E', 'D', 'U', 'R', 'E'},
			{ 'R', 'E', 'P', 'E', 'A', 'T'},
			{ 'T', 'H', 'E', 'N'},
			{ 'T', 'O'},
			{ 'U', 'N', 'T', 'I', 'L'},
			{ 'V', 'A', 'R'},
			{ 'W', 'H', 'I', 'L', 'E'},
			{ 'W', 'R', 'I', 'T', 'E' },
			{ 'W', 'R', 'I', 'T', 'E', 'L', 'N'}
		};

		static Scanner input;
		static table_struct []table = new table_struct[TXMAX];
		static INSTRUCTION code[] = new INSTRUCTION[CXMAX];
		static SYMBOL sym;
		static char ch;
		static char id[] = new char[AL];
		static char a[] = new char[AL];
		static char line[] = new char[80];
		static String temp_id, str_line;
		static String mneumonic[] = new String[20];
		static int id_length, cc, ll, kk, num, codeinx, codeinx0;

		/* Initializing list of error messages */
		public static String ErrMsg[] =
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
			"An expression cannot begin with this symbol",/* 33 */
			"Incorrect Object Type. Must be Const." /*34*/
		};

		/* Simple Error Outputting Function */
		public static void Error(int ErrorNumber)
		{
			System.out.println(ErrMsg[ErrorNumber - 1]);
			System.exit(-1);
		}

		/* Low Level Code Generating Function */
		public static void Gen(FUNCTIONS x, int y, int z)
		{
			if (codeinx > CXMAX)
			{
				System.out.println("Program too long.");
				System.exit(-1);
			}
			code[codeinx].f   = x;
			code[codeinx].lvl = y;
			code[codeinx].ad  = z;
			codeinx++;
		}

		/* Low Level Code Outputting Function */
		public static void ListCode()
		{
			int i;

			System.out.println();
			for (i = codeinx0; i < codeinx; i++)
			{
				System.out.printf("%d %s %d %d\n", i, mneumonic[code[i].f.getValue()],
				                  code[i].lvl, code[i].ad);
			}
			System.out.println();
		}

		/* Function to find new base */
		public static int Base(int s[], int L, int b)
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
		public static void Interpret()
		{
			int p, b, t;
			INSTRUCTION i;
			int s[] = new int[STACKSIZE];

			System.out.println("Start PL/0");
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
								if (s[t] % 2 > 0)
									s[t] = 1;
								else
									s[t] = 0;
								continue;
							case 8:
								t--;
								if (s[t] == s[t + 1])
									s[t] = 1;
								else
									s[t] = 0;
								continue;
							case 9:
								t--;
								if (s[t] != s[t + 1])
									s[t] = 1;
								else
									s[t] = 0;
								continue;
							case 10:
								t--;
								if (s[t] < s[t + 1])
									s[t] = 1;
								else
									s[t] = 0;
								continue;
							case 11:
								t--;
								if (s[t] >= s[t + 1])
									s[t] = 1;
								else
									s[t] = 0;
								continue;
							case 12:
								t--;
								if (s[t] > s[t + 1])
									s[t] = 1;
								else
									s[t] = 0;
								continue;
							case 13:
								t--;
								if (s[t] <= s[t + 1])
									s[t] = 1;
								else
									s[t] = 0;
								continue;
							case 14:
								System.out.printf("%d ", s[t]);
								t--;
								continue;
							case 15:
								System.out.println("");
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
			}
			while (p != 0);

			System.out.println("End PL/0");
		}

		public static void GetChar()
		{
			if (cc == ll)
			{
				if (input.hasNext())
				{
					ll = 0;
					cc = 0;

					str_line = input.nextLine();
					line = str_line.toCharArray();
					System.out.println(str_line);
					ll = str_line.length();

					if (ll > 0)
					{
						if (line[ll-1] == 13) ll--;
						if (line[ll-1] == 10) ll--;
						if (line[ll-1] == 13) ll--;
						if (line[ll-1] == 10) ll--;
						ch = line[cc++];
					}
					else
						ch = ' ';
				}
			}
			else
				ch = line[cc++];

			while (ch == '\t')
				ch = line[cc++];
		}

		public static void GetSym()
		{
			int i, j, k;

			while (ch == ' ' || ch == '\r' || ch == '\n')
				GetChar();

			if (ch >= 'A' && ch <= 'Z')
			{
				k = 0;
				int x = 0;

				for (; x < AL; x++)
				{
					a[x]  = '\0';
					id[x] = '\0';
				}

				do
				{
					if (k < AL)
						a[k++] = ch;
					if (cc == ll)
					{
						GetChar();
						break;
					}
					else
						GetChar();
				} while ((ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9'));

				id = a;

				i = 0;
				j = NORW - 1;

				if (k == 1)
					id_length = 1;
				else
					id_length = k--;

				do
				{
					k = i + j;
					k = k / 2;

					temp_id = String.copyValueOf(id, 0, id_length);

					if (id[0] <= Char_Word[k][0])
					{
						if (temp_id.compareTo(Word[k]) <= 0)
							j = k - 1;
					}
					if (id[0] >= Char_Word[k][0])
					{
						if (temp_id.compareTo(Word[k]) >= 0)
							i = k + 1;
					}
				} while (i <= j);

				if (i - 1 > j)
					sym = wsym[k];
				else
					sym = SYMBOL.IDENT;
			}
			else if (ch >= '0' && ch <= '9')
			{
				k = 0;
				num = 0;
				sym = SYMBOL.NUMBER;

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
					sym = SYMBOL.BECOMES;
					GetChar();
				}
				else
					sym = SYMBOL.COLON;
			}
			else if (ch == '>')
			{
				GetChar();

				if (ch == '=')
				{
					sym = SYMBOL.GEQ;
					GetChar();
				}
				else
					sym = SYMBOL.GTR;
			}
			else if (ch == '<')
			{
				GetChar();

				if (ch == '=')
				{
					sym = SYMBOL.LEQ;
					GetChar();
				}
				else if (ch == '>')
				{
					sym = SYMBOL.NEQ;
					GetChar();
				}
				else
					sym = SYMBOL.LSS;
			}
			else
			{
				if (ch == '+')
					sym = SYMBOL.PLUS;
				else if (ch == '-')
					sym = SYMBOL.MINUS;
				else if (ch == '*')
					sym = SYMBOL.TIMES;
				else if (ch == '/')
					sym = SYMBOL.SLASH;
				else if (ch == '(')
					sym = SYMBOL.LPAREN;
				else if (ch == ')')
					sym = SYMBOL.RPAREN;
				else if (ch == '=')
					sym = SYMBOL.EQL;
				else if (ch == '.')
					sym = SYMBOL.PERIOD;
				else if (ch == ',')
					sym = SYMBOL.COMMA;
				else if (ch == ';')
					sym = SYMBOL.SEMICOLON;

				GetChar();
			}
		}

		public static int Enter(OBJECTS k, int lev, int dx, int[] tx)
		{
			tx[0]++;
			table[tx[0]].name = String.valueOf(id);
			table[tx[0]].kind = k;

			switch (k)
			{
				case Constant:
					if (num > AMAX) Error(30);
					table[tx[0]].val = num;
					break;
				case Variable:
					table[tx[0]].level = lev;
					table[tx[0]].adr = dx++;
					break;
				case Procedure:
					table[tx[0]].level = lev;
					break;
			}

			return dx;
		}

		public static int Position(char id[], int tx)
		{
			int i = tx;

			table[0].name = String.valueOf(id);

			while (!table[i].name.equals(String.valueOf(id)))
				i--;

			return i;
		}

		public static void Block(int lev, int tx)
		{
			int tabinx0 = tx;
			int dx = 3;

			table[tx].adr = codeinx;
			Gen(FUNCTIONS.JMP, 0, 0);
			if (lev > LEVMAX)
				Error(24);

			while ((sym == SYMBOL.CONSTSYM) || (sym == SYMBOL.VARSYM) || (sym == SYMBOL.PROCSYM))
			{
				if (sym == SYMBOL.CONSTSYM)
				{
					GetSym();
					int[] txx = new int[1];
					txx[0] = tx;
					dx = ConstDeclaration(lev, dx, txx);
					tx = txx[0];
					while (sym == SYMBOL.COMMA)
					{
						GetSym();
						txx[0] = tx;
						dx = ConstDeclaration(lev, dx, txx);
						tx = txx[0];
					}
					if (sym == SYMBOL.SEMICOLON)
						GetSym();
					else
						Error(5);
				} /* End if (CONSTSYM) */

				if (sym == SYMBOL.VARSYM)
				{
					GetSym();
					int[] txx = new int[1];
					txx[0] = tx;
					dx = VarDeclaration(lev, dx, txx);
					tx = txx[0];
					while (sym == SYMBOL.COMMA)
					{
						GetSym();
						txx[0] = tx;
						dx = VarDeclaration(lev, dx, txx);
						tx = txx[0];
					}

					if (sym == SYMBOL.SEMICOLON)
						GetSym();
					else
						Error(5);
				} /* END if (VARSYM) */

				while (sym == SYMBOL.PROCSYM)
				{
					GetSym();

					if (sym == SYMBOL.IDENT)
					{
						int[] txx = new int[1];
						txx[0] = tx;
						dx = Enter(OBJECTS.Procedure, lev, dx, txx);
						tx = txx[0];
						GetSym();
					}
					else
						Error(6);

					if (sym == SYMBOL.SEMICOLON)
						GetSym();
					else
						Error(5);

					Block(lev + 1, tx);

					if (sym == SYMBOL.SEMICOLON)
						GetSym();
					else
						Error(5);
				}
			}

			/* mark up first jump to the proper place */
			code[table[tabinx0].adr].ad = codeinx;
			table[tabinx0].adr = codeinx;
			codeinx0 = codeinx;

			Gen(FUNCTIONS.INT, 0, dx);
			Statement(lev, tx);
			Gen(FUNCTIONS.OPR, 0, 0);

			/* Print out code for each block */
			ListCode();
		}

		public static void Factor(int lev, int tx)
		{
			int i;

			if (sym == SYMBOL.IDENT)
			{
				if ((i = Position(id, tx)) == FALSE)
					Error(11);
				switch (table[i].kind)
				{
					case Constant :
						Gen(FUNCTIONS.LIT, 0, table[i].val);
						break;
					case Variable :
						Gen(FUNCTIONS.LOD, lev - table[i].level, table[i].adr);
						break;
					case Procedure:
						Error(21);
						break;
				}
				GetSym();
			}
			else if (sym == SYMBOL.NUMBER)
			{
				GetSym();
				Gen(FUNCTIONS.LIT, 0, num);
			}

			else if (sym == SYMBOL.LPAREN)
			{
				GetSym();
				Expression(lev, tx);
				if (sym == SYMBOL.RPAREN)
					GetSym();
				else
					Error(22);
			}
			else
				Error(33);
		}

		public static void Term(int lev, int tx)
		{
			SYMBOL  multop;

			Factor(lev, tx);

			while (sym == SYMBOL.TIMES || sym == SYMBOL.SLASH)
			{
				multop = sym;
				GetSym();
				Factor(lev, tx);
				switch (multop)
				{
					case TIMES:
						Gen(FUNCTIONS.OPR, 0, 4);
						break;
					case SLASH:
						Gen(FUNCTIONS.OPR, 0, 5);
						break;
				}
			}
		}

		public static void Expression(int lev, int tx)
		{
			SYMBOL addop;

			if (sym == SYMBOL.PLUS || sym == SYMBOL.MINUS)
			{
				addop = sym;
				GetSym();
				Term(lev, tx);
				if (addop == SYMBOL.MINUS)
					Gen(FUNCTIONS.OPR, 0, 1);
			}
			else
				Term(lev, tx);

			while (sym == SYMBOL.PLUS || sym == SYMBOL.MINUS)
			{
				addop = sym;
				GetSym();
				Term(lev, tx);
				switch (addop)
				{
					case PLUS:
						Gen(FUNCTIONS.OPR, 0, 2);
						break;
					case MINUS:
						Gen(FUNCTIONS.OPR, 0, 3);
						break;
				}
			}
		}

		public static void Condition(int lev, int tx)
		{
			SYMBOL tmp;

			if (sym == SYMBOL.ODDSYM)
			{
				GetSym();
				Expression(lev, tx);
				Gen(FUNCTIONS.OPR, 0, 6);
			}
			else
			{
				Expression(lev, tx);

				if ((sym == SYMBOL.EQL) || (sym == SYMBOL.GTR) || (sym == SYMBOL.LSS) ||
				                (sym == SYMBOL.NEQ) || (sym == SYMBOL.LEQ) || (sym == SYMBOL.GEQ))
				{
					tmp = sym;
					GetSym();
					Expression(lev, tx);
					switch (tmp)
					{
						case EQL:
							Gen(FUNCTIONS.OPR, 0, 8);
							break;
						case NEQ:
							Gen(FUNCTIONS.OPR, 0, 9);
							break;
						case LSS:
							Gen(FUNCTIONS.OPR, 0, 10);
							break;
						case GEQ:
							Gen(FUNCTIONS.OPR, 0, 11);
							break;
						case GTR:
							Gen(FUNCTIONS.OPR, 0, 12);
							break;
						case LEQ:
							Gen(FUNCTIONS.OPR, 0, 13);
							break;
					}
				}
				else
					Error(20);
			}
		}

		public static int ConstDeclaration(int lev, int dx, int[] tx)
		{
			if (sym == SYMBOL.IDENT)
			{
				GetSym();
				if (sym == SYMBOL.EQL)
				{
					GetSym();
					if (sym == SYMBOL.NUMBER)
					{
						dx = Enter(OBJECTS.Constant, lev, dx, tx);
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

			return dx;
		}

		public static int VarDeclaration(int lev, int dx, int[] tx)
		{
			if (sym == SYMBOL.IDENT)
			{
				dx = Enter(OBJECTS.Variable, lev, dx, tx);
				GetSym();
			}
			else
				Error(4);

			return dx;
		}

		public static void Statement(int lev, int tx)
		{
			int i, cx1, cx2;

			switch (sym)
			{
				case BEGINSYM:
					GetSym();
					Statement(lev, tx);
					while (sym == SYMBOL.SEMICOLON)
					{
						GetSym();
						Statement(lev, tx);
					}
					if (sym == SYMBOL.ENDSYM)
						GetSym();
					else
						Error(17);
					break;

				case IDENT:
					if ((i = Position(id, tx)) == FALSE)
						Error(11);
					else
						if (table[i].kind != OBJECTS.Variable)
							Error(12);
					GetSym();
					if (sym == SYMBOL.BECOMES)
						GetSym();
					else
						Error(13);
					Expression(lev, tx);
					Gen(FUNCTIONS.STO, lev - table[i].level, table[i].adr);
					break;

				case IFSYM:
					GetSym();
					Condition(lev, tx);
					cx1 = codeinx;
					Gen(FUNCTIONS.JPC, 0, 0);
					if (sym == SYMBOL.THENSYM)
						GetSym();
					else
						Error(16);
					Statement(lev, tx);
					code[cx1].ad = codeinx;
					/* Place Your Code for Else Here */
					if(sym == SYMBOL.ELSESYM){
						GetSym();
						Statement(lev, tx);
					}
					break; /* IFSYM */

				case WHILESYM:
					GetSym();
					cx1 = codeinx;
					Condition(lev, tx);
					cx2 = codeinx;
					Gen(FUNCTIONS.JPC, 0, 0);
					if (sym == SYMBOL.DOSYM)
					{
						GetSym();
						Statement(lev, tx);
						Gen(FUNCTIONS.JMP, 0, cx1);
						code[cx2].ad = codeinx;
					}
					else
						Error(18);
					break; /* WHILESYM */

				case REPEATSYM:
					/* Place Your Code for Repeat-Until Here */
				// BEGIN ADDITION
					do{
						GetSym();
						Statement(lev, tx);                
					}while(sym == SYMBOL.SEMICOLON);
					if (sym == SYMBOL.UNTILSYM){
						GetSym();
					} 
					else 
						Error(23);
					Condition(lev, tx);
				// END ADDITION
					break; /* REPEATSYM */

				case WRITELNSYM:
					/* Place Your Code for Writeln Here */
				// BEGIN ADDITION
					GetSym();
					if(sym == SYMBOL.LPAREN){
						do{
							GetSym();
							Expression(lev, tx);
						}while(sym == SYMBOL.COMMA);
					}
					else
						Error(31);
					if(sym == SYMBOL.RPAREN){
						GetSym();
					}
					else
						Error(22);
				// END ADDITION
					break; /* WRITELNSYM */

				case CALLSYM:
					GetSym();
					if (sym == SYMBOL.IDENT)
					{
						if ((i = Position(id, tx)) == FALSE)
							Error(11);
						else
							if (table[i].kind != OBJECTS.Procedure)
								Error(15);
						Gen(FUNCTIONS.CAL, lev - table[i].level, table[i].adr);
						GetSym();
					}
					else
						Error(14);
					break;

				case CASESYM:
					/* Place Your Code for Case Here */
				// BEGIN ADDITION
					GetSym();
					Expression(lev, tx);
					if(sym == SYMBOL.OFSYM){
						GetSym();
					}
					else{
						Error(25);
					}
					while(sym == SYMBOL.NUMBER|| sym == SYMBOL.IDENT){
						if(sym == SYMBOL.IDENT){
							i = Position(id, tx);
							if (table[i].kind != OBJECTS.Constant){
								Error(34); // Object Type Error
							}
						}
						GetSym();
						if(sym == SYMBOL.COLON){
							GetSym();
							Statement(lev, tx);
						}
						else{
							Error(29);
						}
						if(sym == SYMBOL.SEMICOLON){
							GetSym();
						}
						else{
							Error(17);
						}					
					}
					if(sym == SYMBOL.CENDSYM){
						GetSym();						
					}
					else{
						Error(22);
					}
				// END ADDITION
					break; /* CASESYM */

				case FORSYM:
					/* Place Your Code for For-Do Here */
				// BEGIN ADDITION
					GetSym();
					if(sym != SYMBOL.IDENT){
						Error(11);
					}
					i = Position(id, tx);
					if (table[i].kind != OBJECTS.Variable){
						Error(12);
					}
					GetSym();
					
					if(sym == SYMBOL.BECOMES){
						GetSym();
						Expression(lev, tx);
					}
					else{
						Error(13);/*missing := error*/
					}
					if(sym == SYMBOL.TOSYM || sym == SYMBOL.DOWNTOSYM){
						GetSym();
						Expression(lev, tx);
						if(sym == SYMBOL.DOSYM){
							GetSym();
							Statement(lev, tx);
						}
						else{
							Error(18);/*missing DO error*/
						}
					}
					else{
						Error(26);/*missing TO or DOWNTO error*/
					}
				// END ADDITION
					break; /* FORSYM */

				case WRITESYM:
					/* Place Your Code for Write Here */
				// BEGIN ADDITION
					GetSym();
					if(sym == SYMBOL.LPAREN){
						do{
							GetSym();
							Expression(lev, tx);
						}while(sym == SYMBOL.COMMA);
					}
					else
						Error(31);
					if(sym == SYMBOL.RPAREN){
						GetSym();
					}
					else
						Error(22);
				// END ADDITION
					break;
			}
		}

		public static void main(String[] args)
		{
			mneumonic[FUNCTIONS.LIT.getValue()] = "LIT";
			mneumonic[FUNCTIONS.LOD.getValue()] = "LOD";
			mneumonic[FUNCTIONS.INT.getValue()] = "INT";
			mneumonic[FUNCTIONS.JPC.getValue()] = "JPC";
			mneumonic[FUNCTIONS.OPR.getValue()] = "OPR";
			mneumonic[FUNCTIONS.CAL.getValue()] = "CAL";
			mneumonic[FUNCTIONS.JMP.getValue()] = "JMP";
			mneumonic[FUNCTIONS.STO.getValue()] = "STO";

			/* Open Pascal File */
			try
			{
				input = new Scanner(System.in);
			}
			catch (Exception e)
			{
				System.err.println("Error Getting Input");
				System.exit(1);
			}

			cc = ll;
			ll = 0;
			ch = ' ';
			kk = AL;

			for (int q = 0; q < TXMAX; q++)
			{
				table[q] = new table_struct("", OBJECTS.None);
			}

			for (int q = 0; q < CXMAX; q++)
			{
				code[q] = new INSTRUCTION();
			}

			GetSym();

			Block(0, 0);

			if (sym != SYMBOL.PERIOD)
				Error(9);
			else
				System.out.println("Successful compilation!\n");

			Interpret();

			input.close();
		}
}

