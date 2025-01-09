
/*  Copyright 1981 Gary Perlman */

#define	CALC_VERSION                   "5.2 11/2/85"
/* PGM(calc, Algebraic Modeling Calculator) */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

#define SBUFSIZ 1024
#define	FZERO	10e-60       /* smaller |values| are considered 0.0 */
#define	fzero(x) (fabs (x) < FZERO)


#define	fzero(x)      (fabs (x) < FZERO)
#define	isvarchar(c)  (isalnum (c) || (c) == '_')

#define OPERATOR     1
#define PARSERROR    1
#define	MAXVAR    1000 
#define	UNDEFINED   -99999999999.987654321

int 	Nvar;
char	*Varname[MAXVAR];
char	*Eptr;
char	Format[100] = "%.10g";    /* default print format for numbers */

typedef	union
	{
	int 	opr;        /* if OPERATOR */
	double	*num;       /* if NUMBER */
	int 	var;        /* if VARIABLE */
	} STUFF;
STUFF	Tmp1, Tmp2;  /* used in the parser to cast operators */
typedef struct enode        /* expression node in tree */
	{
	int 	etype;          /* type of node */
	STUFF	contents;
	struct	enode *left;
	struct	enode *right;
	} ENODE;
#define	ENULL ((ENODE *) NULL)
ENODE	*Expression, *Variable[MAXVAR];

double	eval (), Answer;
double	*Constant;

char	*mygetline ();

typedef union 
	{
	int 	opr;   /* an operator */
	int 	var;   /* an index into the variable table */
	double	*num;  /* a pointer to a numerical constant */
	ENODE	*ex;   /* an expression in the parse tree */
	} YYSTYPE;
#ifdef __cplusplus
#  include <stdio.h>
#  include <yacc.h>
#endif	/* __cplusplus */ 
# define NUMBER 257
# define VARIABLE 258
# define IF 259
# define THEN 260
# define ELSE 261
# define EQ 262
# define NE 263
# define GE 264
# define LE 265
# define UMINUS 266
# define ABS 267
# define EXP 268
# define LOG 269
# define SQRT 270
# define COS 271
# define TAN 272
# define SIN 273
# define ACOS 274
# define ASIN 275
# define ATAN 276
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif

/* __YYSCLASS defines the scoping/storage class for global objects
 * that are NOT renamed by the -p option.  By default these names
 * are going to be 'static' so that multi-definition errors
 * will not occur with multiple parsers.
 * If you want (unsupported) access to internal names you need
 * to define this to be null so it implies 'extern' scope.
 * This should not be used in conjunction with -p.
 */
#ifndef __YYSCLASS
# define __YYSCLASS static
#endif
YYSTYPE yylval;
__YYSCLASS YYSTYPE yyval;
typedef int yytabelem;
# define YYERRCODE 256


/*  Copyright 1986 Gary Perlman */

#include <ctype.h>

int skipnumber (
char	*string,    /* input string, probably in number format */
int 	isreal)     /* if true, then skip over real part too */
{
	register	char	*ptr;

	ptr = string;
	while (isspace (*ptr))
		ptr++;
	if (*ptr == '-' || *ptr == '+')
		ptr++;
	while (isdigit (*ptr))
		ptr++;
	if (isreal)
		{
		if (*ptr == '.')
			ptr++;
		while (isdigit (*ptr))
			ptr++;
		if (*ptr == 'E' || *ptr == 'e')
			{
			ptr++;
			if (*ptr == '+' || *ptr == '-')
				ptr++;
			while (isdigit (*ptr))
				ptr++;
			}
		}
	return (ptr - string);
	}

void errorexit (char *string)
{
	fprintf (stderr, "calc: %s\n", string);
	exit (1);
}

/* Suzanne Shouman fixed a bug here. Thanks */
int begins (char *s1, char *s2)
{
	int 	alphlag = isvarchar (*s1);
	while (*s1)
		if (*s1++ != *s2++) return (0);
	return (alphlag ? !isvarchar(*s2) : 1);
}


int yylex ()
	{
	extern	YYSTYPE yylval;
	char	tmpvarname[SBUFSIZ];
	int 	i;
	while (isspace (*Eptr)) Eptr++;
	if (begins ("acos", Eptr)) {Eptr += 4; return (ACOS);}
	if (begins ("asin", Eptr)) {Eptr += 4; return (ASIN);}
	if (begins ("atan", Eptr)) {Eptr += 4; return (ATAN);}
	if (begins ("cos", Eptr)) {Eptr += 3; return (COS);}
	if (begins ("sin", Eptr)) {Eptr += 3; return (SIN);}
	if (begins ("tan", Eptr)) {Eptr += 3; return (TAN);}
	if (begins ("log", Eptr)) {Eptr += 3; return (LOG);}
	if (begins ("sqrt", Eptr)) {Eptr += 4; return (SQRT);}
	if (begins ("exp", Eptr)) {Eptr += 3; return (EXP);}
	if (begins ("abs", Eptr)) {Eptr += 3; return (ABS);}
	if (begins ("if", Eptr)) {Eptr += 2; return (IF);}
	if (begins ("then", Eptr)) {Eptr += 4; return (THEN);}
	if (begins ("else", Eptr)) {Eptr += 4; return (ELSE);}
	if (*Eptr == '$')
		{
		Eptr++;
		yylval.num = &Answer;
		return (NUMBER);
		}
	if (isdigit (*Eptr) || *Eptr == '.')
		{
		Constant = (double *) malloc (sizeof (double));
		if (Constant == NULL)
			errorexit ("Out of storage space");
		*Constant = atof (Eptr);
		yylval.num = Constant;
		Eptr += skipnumber (Eptr, 1);
		return (NUMBER);
		}
	if (isvarchar (*Eptr))
		{
		for (i = 0; isvarchar (Eptr[i]); i++)
			tmpvarname[i] = Eptr[i];
		tmpvarname[i] = (char) 0;
		Eptr += i;
		i = 0;
		while (i < Nvar && strcmp (tmpvarname, Varname[i])) i++;
		if (i == Nvar)
			{
			Varname[i] = malloc ((unsigned) (strlen(tmpvarname)+1));
			if (Varname[i] == NULL)
				errorexit ("Out of storage space");
			(void) strcpy (Varname[i], tmpvarname);
			if (++Nvar == MAXVAR)
				errorexit ("Too many variables");
			}
		yylval.var = i;
		return (VARIABLE);
		}
	if (begins ("!=", Eptr)) { Eptr += 2; return (NE); }
	if (begins (">=", Eptr)) { Eptr += 2; return (GE); }
	if (begins ("<=", Eptr)) { Eptr += 2; return (LE); }
	if (begins ("==", Eptr)) { Eptr += 2; return (EQ); }
	if (begins ("**", Eptr)) { Eptr += 2; return ('^'); }
	if (begins ("&&", Eptr)) { Eptr += 2; return ('&'); }
	if (begins ("||", Eptr)) { Eptr += 2; return ('|'); }
	return ((int) *Eptr++);
}

void yyerror (char *msg)
{
	fprintf (stderr, "%s:\n", msg);
	fprintf (stderr, "Parsing error.  ");
	fprintf (stderr, "This is left in input: [%s]\n", Eptr-1);
}

ENODE *
node (datum, datatype, lson, rson)
STUFF	*datum;       /* pointer to a number or an operator */
int 	datatype;     /* NUMBER or VARIABLE or OPERATOR */
ENODE	*lson;        /* left part of tree */
ENODE	*rson;        /* right part of tree */
	{
	ENODE *newnode;
	newnode = (ENODE *) malloc (sizeof (ENODE));
	if (newnode == ENULL)
		errorexit ("Out of storage space");
	newnode->etype = datatype;
	if (datatype == OPERATOR)
		newnode->contents.opr = datum->opr;
	else if (datatype == VARIABLE)
		newnode->contents.var = datum->var;
	else /* (datatype == NUMBER) */
		newnode->contents.num = datum->num;
	newnode->left = lson;
	newnode->right = rson;
	return (newnode);
	}
	


void ptree (ENODE *expression, char *res)
{
	char tmpres[SBUFSIZ];
	
	if (expression == ENULL)
		return;
		
	if (expression->etype == VARIABLE)
		{
		sprintf (tmpres, "%s", Varname[expression->contents.var]);
		strcat(res,tmpres);
		return;
		}
	else if (expression->etype == NUMBER)
		{
		if (*expression->contents.num == UNDEFINED)
			strcat (res, "UNDEFINED");
		else
			sprintf (tmpres, Format, *expression->contents.num);
			strcat(res,tmpres);
		return;
		}
	switch	(expression->contents.opr)
		{
		case LOG: strcat(res, "log("); break;
		case ABS: strcat(res, "abs("); break;
		case EXP: strcat(res, "exp("); break;
		case SQRT: strcat(res, "sqrt("); break;
		case ATAN: strcat(res, "atan("); break;
		case ASIN: strcat(res, "asin("); break;
		case ACOS: strcat(res, "acos("); break;
		case TAN: strcat(res, "tan("); break;
		case SIN: strcat(res, "sin("); break;
		case COS: strcat(res, "cos("); break;
		case '_' : strcat(res,"-");
			ptree (expression->right,res); return;
		case '?':
			strcat(res, "(if ");
			ptree (expression->left, res);
			strcat(res, " then ");
			if (expression->right->contents.opr == ':')
				{
				ptree (expression->right->left, res);
				strcat(res, " else ");
				ptree (expression->right->right, res);
				}
			else ptree (expression->right, res);
			strcat(res,")");
			return;
		default: strcat(res,"(");
			ptree (expression->left, res);
			switch (expression->contents.opr)
				{
				case EQ: strcat(res, " == "); break;
				case NE: strcat(res, " != "); break;
				case GE: strcat(res, " >= "); break;
				case LE: strcat(res, " <= "); break;
				default: sprintf(tmpres, " %c ",expression->contents.opr);
				         strcat(res,tmpres);
				}
		}
	ptree (expression->right, res);
        strcat(res,")");
}


void printmenu ()
{
	puts ("Expressions are in standard C syntax (like algebra).");
	puts ("The following CTRL characters have special functions:");
	puts ("(You may have to precede the character with a ^V)");
	puts ("^D	end of input to CALC");
	puts ("^P	toggles the printing of equations");
	puts ("^Rfile	reads the expressions from the named file");
	puts ("^Wfile	writes all expressions to the named file");
	puts ("	If no file is supplied, then print to the screen");
}


#define	CTRL_PRINT     16
#define	CTRL_READ      18
#define	CTRL_WRITE     23
#define	CTRL_VAR       22
#define	CTRL_FMT        6

double eval (ENODE *expression)
{
	double	tmp1, tmp2;

	if (expression == ENULL)
		return (UNDEFINED);
	if (expression->etype == NUMBER)
		return (*expression->contents.num);
	if (expression->etype == VARIABLE)
		{
		if (Variable[expression->contents.var])
			return (eval (Variable[expression->contents.var]));
		else
			return (UNDEFINED);
		}

	if ((tmp2 = eval (expression->right)) == UNDEFINED)
		return (UNDEFINED);

	switch (expression->contents.opr)
	{
	case '_': return (-tmp2);
	case '!': return (fzero (tmp2) ? 1.0 : 0.0);
	case LOG: if (tmp2 <= FZERO) return (UNDEFINED);
		else return (log (tmp2));
	case COS: return (cos (tmp2));
	case SIN: return (sin (tmp2));
	case TAN: return (tan (tmp2));
	case ACOS: return (acos (tmp2));
	case ASIN: return (asin (tmp2));
	case ATAN: return (atan (tmp2));
	case EXP: return (exp (tmp2));
	case ABS: return (fabs (tmp2));
	case SQRT: if (tmp2 < 0.0) return (UNDEFINED);
		else return (sqrt (tmp2));
	}

	if ((tmp1 = eval (expression->left)) == UNDEFINED)
		return (UNDEFINED);
	switch (expression->contents.opr)
	{
	case '+': return (tmp1 + tmp2);
	case '-': return (tmp1 - tmp2);
	case '*': return (tmp1 * tmp2);
	case '%': if ((int) tmp2 == 0) return (tmp1);
		else return ((double) (((int) tmp1) % ((int) tmp2)));
	case '/': if (fzero (tmp2)) return (UNDEFINED);
		else return (tmp1/tmp2);
	case '^': return (pow (tmp1, tmp2));
	case '&': return ((!fzero (tmp1) && !fzero (tmp2)) ? 1.0 : 0.0);
	case '|': return ((!fzero (tmp1) || !fzero (tmp2)) ? 1.0 : 0.0);
	case '>': return (tmp1 > tmp2 ? 1.0 : 0.0);
	case '<': return (tmp1 < tmp2 ? 1.0 : 0.0);
	case EQ : return (fzero (tmp1 - tmp2) ? 1.0 : 0.0);
	case NE : return (!fzero (tmp1 - tmp2) ? 1.0 : 0.0);
	case LE : return (tmp1 <= tmp2 ? 1.0 : 0.0);
	case GE : return (tmp1 >= tmp2 ? 1.0 : 0.0);
	case ':': return (0.0); /* dummy return for ? */
	case '?':
		if (expression->right->contents.opr == ':')
			return (!fzero (tmp1)
				? eval (expression->right->left)
				: eval (expression->right->right));
		else if (!fzero (tmp1)) return (eval (expression->right));
		else return (UNDEFINED);
	default: fprintf (stderr, "calc: Unknown operator '%c' = %d\n",
		expression->contents.opr, expression->contents.opr);
		return (UNDEFINED);
	}
}


int checkrecursion (
int 	varno,      /* look for recursion involving this variable */
ENODE	*expr)      /* look for recursion of varno in this expr */
{
	if (expr == ENULL || expr->etype == NUMBER)
		return (0);

	if (expr->etype == VARIABLE)
		{
		if (expr->contents.var == varno)
			return (1);
		if (checkrecursion (varno, Variable[expr->contents.var]))
			return (1);
		}

	return
		(
		checkrecursion (varno, expr->left)
		||
		checkrecursion (varno, expr->right)
		);
	}

char *
mygetline (line, ioptr) char *line; FILE *ioptr;
	{
	register int C;
	register char *lptr = line;
	while ((C = getc (ioptr)) != '\n' && C != ';' && C != EOF)
		*lptr++ = C;
	if (C == EOF) return (NULL);
	while (C != '\n' && C != EOF) C = getc (ioptr);
	*lptr = '\0';
	return (line);
	}

__YYSCLASS yytabelem yyexca[] ={
-1, 1,
	0, -1,
	-2, 0,
-1, 58,
	262, 0,
	263, 0,
	264, 0,
	265, 0,
	60, 0,
	62, 0,
	-2, 12,
-1, 59,
	262, 0,
	263, 0,
	264, 0,
	265, 0,
	60, 0,
	62, 0,
	-2, 13,
-1, 60,
	262, 0,
	263, 0,
	264, 0,
	265, 0,
	60, 0,
	62, 0,
	-2, 14,
-1, 61,
	262, 0,
	263, 0,
	264, 0,
	265, 0,
	60, 0,
	62, 0,
	-2, 15,
-1, 62,
	262, 0,
	263, 0,
	264, 0,
	265, 0,
	60, 0,
	62, 0,
	-2, 16,
-1, 63,
	262, 0,
	263, 0,
	264, 0,
	265, 0,
	60, 0,
	62, 0,
	-2, 17,
	};
# define YYNPROD 36
# define YYLAST 456
__YYSCLASS yytabelem yyact[]={

     7,    25,     5,    23,    32,    37,     1,     3,    22,    20,
     0,    21,     6,    24,     0,    23,     0,     0,     0,     0,
    22,    20,    23,    21,    71,    24,    29,    22,    31,    34,
    23,    32,    24,     0,     0,    22,    20,     0,    21,     0,
    24,    23,    32,     0,     0,    68,    22,    20,     0,    21,
     0,    24,     0,    29,     0,    31,    34,     0,     0,     0,
    25,     0,     0,     0,    29,     0,    31,    34,    23,    32,
     0,     0,    25,    22,    20,     0,    21,     0,    24,    25,
     0,     0,     0,     0,     0,     0,     0,    25,     0,     0,
    33,    29,     0,    31,    34,    23,    32,     0,    25,     0,
    22,    20,     0,    21,     0,    24,    23,    32,     0,     0,
     0,    22,    20,     0,    21,     0,    24,    33,    29,     0,
    31,     0,     0,     0,     0,    25,     0,     0,    33,    29,
     0,    31,     0,    23,    32,     0,     0,     0,    22,    20,
     0,    21,     0,    24,     0,     0,     0,     0,     0,     0,
     0,     0,    25,     0,    23,    33,    29,     0,    31,    22,
    20,     0,    21,    25,    24,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,    29,     0,    31,
     0,     0,    33,     0,     0,     0,     0,     0,     0,     0,
    25,     0,     0,    33,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,    25,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,    19,     4,     8,    35,    26,    27,
    30,    28,     0,     0,    17,    16,    15,    18,    12,    14,
    13,     9,    10,    11,     0,     0,     0,     0,     0,     0,
     0,     0,     0,    70,    35,    26,    27,    30,    28,     0,
     0,     0,     0,     0,     0,    35,    26,    27,    30,    28,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,    35,    26,    27,    30,    28,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,    35,
    26,    27,    30,    28,     0,     0,     0,     0,     0,     0,
     0,    26,    27,    30,    28,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,    26,    27,
    30,    28,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,    26,
    27,    30,    28,     2,     0,     0,     0,    36,     0,    38,
    39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
    49,    50,    51,     0,    52,    53,    54,    55,    56,    57,
    58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
     0,    69,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,    72,    73 };
__YYSCLASS yytabelem yypact[]={

   -33, -3000,    31,   -33,   -56,   -33,   -33,   -33,   -33,   -33,
   -33,   -33,   -33,   -33,   -33,   -33,   -33,   -33,   -33, -3000,
   -33,   -33,   -33,   -33,   -33,   -33,   -33,   -33,   -33,   -33,
   -33,   -33,   -33,   -33,   -33,   -33,     4,   -33,    31, -3000,
   117,    -7, -3000, -3000, -3000, -3000, -3000, -3000, -3000, -3000,
 -3000, -3000,   -15,   -15,   -93,   -93,   -93,   -93,   -22,   -22,
   -22,   -22,   -22,   -22,   117,    96,   -34,    69, -3000,    31,
   -33,   -33,    58,    69 };
__YYSCLASS yytabelem yypgo[]={

     0,   383,     6 };
__YYSCLASS yytabelem yyr1[]={

     0,     2,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1 };
__YYSCLASS yytabelem yyr2[]={

     0,     3,     7,     7,     5,     7,     7,     7,     7,     7,
     7,     5,     7,     7,     7,     7,     7,     7,     7,     7,
     5,    11,     9,     7,     5,     5,     5,     5,     5,     5,
     5,     5,     5,     5,     3,     3 };
__YYSCLASS yytabelem yychk[]={

 -3000,    -2,    -1,    40,   258,    35,    45,    33,   259,   274,
   275,   276,   271,   273,   272,   269,   268,   267,   270,   257,
    43,    45,    42,    37,    47,    94,   262,   263,   265,    60,
   264,    62,    38,   124,    63,   261,    -1,    61,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    -1,
   260,    58,    -1,    -1 };
__YYSCLASS yytabelem yydef[]={

     0,    -2,     1,     0,    34,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,    35,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     4,    11,
    20,     0,    24,    25,    26,    27,    28,    29,    30,    31,
    32,    33,     5,     6,     7,     8,     9,    10,    -2,    -2,
    -2,    -2,    -2,    -2,    18,    19,     0,    23,     2,     3,
     0,     0,    22,    21 };
typedef struct { char *t_name; int t_val; } yytoktype;
#ifndef YYDEBUG
#	define YYDEBUG	0	/* don't allow debugging */
#endif

#if YYDEBUG

__YYSCLASS yytoktype yytoks[] =
{
	"NUMBER",	257,
	"VARIABLE",	258,
	"#",	35,
	"=",	61,
	"?",	63,
	"IF",	259,
	"THEN",	260,
	":",	58,
	"ELSE",	261,
	"|",	124,
	"&",	38,
	"!",	33,
	"EQ",	262,
	"NE",	263,
	"GE",	264,
	"LE",	265,
	"<",	60,
	">",	62,
	"+",	43,
	"-",	45,
	"*",	42,
	"/",	47,
	"%",	37,
	"^",	94,
	"UMINUS",	266,
	"ABS",	267,
	"EXP",	268,
	"LOG",	269,
	"SQRT",	270,
	"COS",	271,
	"TAN",	272,
	"SIN",	273,
	"ACOS",	274,
	"ASIN",	275,
	"ATAN",	276,
	"-unknown-",	-1	/* ends search */
};

__YYSCLASS char * yyreds[] =
{
	"-no such reduction-",
	"start : expr",
	"expr : '(' expr ')'",
	"expr : VARIABLE '=' expr",
	"expr : '#' expr",
	"expr : expr '+' expr",
	"expr : expr '-' expr",
	"expr : expr '*' expr",
	"expr : expr '%' expr",
	"expr : expr '/' expr",
	"expr : expr '^' expr",
	"expr : '-' expr",
	"expr : expr EQ expr",
	"expr : expr NE expr",
	"expr : expr LE expr",
	"expr : expr '<' expr",
	"expr : expr GE expr",
	"expr : expr '>' expr",
	"expr : expr '&' expr",
	"expr : expr '|' expr",
	"expr : '!' expr",
	"expr : expr '?' expr ':' expr",
	"expr : IF expr THEN expr",
	"expr : expr ELSE expr",
	"expr : ACOS expr",
	"expr : ASIN expr",
	"expr : ATAN expr",
	"expr : COS expr",
	"expr : SIN expr",
	"expr : TAN expr",
	"expr : LOG expr",
	"expr : EXP expr",
	"expr : ABS expr",
	"expr : SQRT expr",
	"expr : VARIABLE",
	"expr : NUMBER",
};
#endif /* YYDEBUG */
#define YYFLAG  (-3000)
/* @(#) $Revision: 70.7 $ */    

/*
** Skeleton parser driver for yacc output
*/

#if defined(NLS) && !defined(NL_SETN)
#include <msgbuf.h>
#endif

#ifndef nl_msg
#define nl_msg(i,s) (s)
#endif

/*
** yacc user known macros and defines
*/
#define YYERROR		goto yyerrlab

#ifndef __RUNTIME_YYMAXDEPTH
#define YYACCEPT	return(0)
#define YYABORT		return(1)
#else
#define YYACCEPT	{free_stacks(); return(0);}
#define YYABORT		{free_stacks(); return(1);}
#endif

#define YYBACKUP( newtoken, newvalue )\
{\
	if ( yychar >= 0 || ( yyr2[ yytmp ] >> 1 ) != 1 )\
	{\
		yyerror( (nl_msg(30001,"syntax error - cannot backup")) );\
		goto yyerrlab;\
	}\
	yychar = newtoken;\
	yystate = *yyps;\
	yylval = newvalue;\
	goto yynewstate;\
}

#define YYRECOVERING()	(!!yyerrflag)
#ifndef YYDEBUG
#	define YYDEBUG	1	/* make debugging available */
#endif

/*
** user known globals
*/
int yydebug;			/* set to 1 to get debugging */

/*
** driver internal defines
*/
/* define for YYFLAG now generated by yacc program. */
/*#define YYFLAG		(FLAGVAL)*/

/*
** global variables used by the parser
*/
# ifndef __RUNTIME_YYMAXDEPTH
__YYSCLASS YYSTYPE yyv[ YYMAXDEPTH ];	/* value stack */
__YYSCLASS int yys[ YYMAXDEPTH ];		/* state stack */
# else
__YYSCLASS YYSTYPE *yyv;			/* pointer to malloc'ed value stack */
__YYSCLASS int *yys;			/* pointer to malloc'ed stack stack */

#if defined(__STDC__) || defined (__cplusplus)
#include <stdlib.h>
#else
	extern char *malloc();
	extern char *realloc();
	extern void free();
#endif /* __STDC__ or __cplusplus */


static int allocate_stacks(); 
static void free_stacks();
# ifndef YYINCREMENT
# define YYINCREMENT (YYMAXDEPTH/2) + 10
# endif
# endif	/* __RUNTIME_YYMAXDEPTH */
long  yymaxdepth = YYMAXDEPTH;

__YYSCLASS YYSTYPE *yypv;			/* top of value stack */
__YYSCLASS int *yyps;			/* top of state stack */

__YYSCLASS int yystate;			/* current state */
__YYSCLASS int yytmp;			/* extra var (lasts between blocks) */

int yynerrs;			/* number of errors */
__YYSCLASS int yyerrflag;			/* error recovery flag */
int yychar;			/* current input token number */



/*
** yyparse - return 0 if worked, 1 if syntax error not recovered from
*/
int yyparse()
{
	register YYSTYPE *yypvt;	/* top of value stack for $vars */

	/*
	** Initialize externals - yyparse may be called more than once
	*/
# ifdef __RUNTIME_YYMAXDEPTH
	if (allocate_stacks()) YYABORT;
# endif
	yypv = &yyv[-1];
	yyps = &yys[-1];
	yystate = 0;
	yytmp = 0;
	yynerrs = 0;
	yyerrflag = 0;
	yychar = -1;

	goto yystack;
	{
		register YYSTYPE *yy_pv;	/* top of value stack */
		register int *yy_ps;		/* top of state stack */
		register int yy_state;		/* current state */
		register int  yy_n;		/* internal state number info */

		/*
		** get globals into registers.
		** branch to here only if YYBACKUP was called.
		*/
	yynewstate:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;
		goto yy_newstate;

		/*
		** get globals into registers.
		** either we just started, or we just finished a reduction
		*/
	yystack:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;

		/*
		** top of for (;;) loop while no reductions done
		*/
	yy_stack:
		/*
		** put a state and value onto the stacks
		*/
#if YYDEBUG
		/*
		** if debugging, look up token value in list of value vs.
		** name pairs.  0 and negative (-1) are special values.
		** Note: linear search is used since time is not a real
		** consideration while debugging.
		*/
		if ( yydebug )
		{
			register int yy_i;

			printf( "State %d, token ", yy_state );
			if ( yychar == 0 )
				printf( "end-of-file\n" );
			else if ( yychar < 0 )
				printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ++yy_ps >= &yys[ yymaxdepth ] )	/* room on stack? */
		{
# ifndef __RUNTIME_YYMAXDEPTH
			yyerror( (nl_msg(30002,"yacc stack overflow")) );
			YYABORT;
# else
			/* save old stack bases to recalculate pointers */
			YYSTYPE * yyv_old = yyv;
			int * yys_old = yys;
			yymaxdepth += YYINCREMENT;
			yys = (int *) realloc(yys, yymaxdepth * sizeof(int));
			yyv = (YYSTYPE *) realloc(yyv, yymaxdepth * sizeof(YYSTYPE));
			if (yys==0 || yyv==0) {
			    yyerror( (nl_msg(30002,"yacc stack overflow")) );
			    YYABORT;
			    }
			/* Reset pointers into stack */
			yy_ps = (yy_ps - yys_old) + yys;
			yyps = (yyps - yys_old) + yys;
			yy_pv = (yy_pv - yyv_old) + yyv;
			yypv = (yypv - yyv_old) + yyv;
# endif

		}
		*yy_ps = yy_state;
		*++yy_pv = yyval;

		/*
		** we have a new state - find out what to do
		*/
	yy_newstate:
		if ( ( yy_n = yypact[ yy_state ] ) <= YYFLAG )
			goto yydefault;		/* simple state */
#if YYDEBUG
		/*
		** if debugging, need to mark whether new token grabbed
		*/
		yytmp = yychar < 0;
#endif
		if ( ( yychar < 0 ) && ( ( yychar = yylex() ) < 0 ) )
			yychar = 0;		/* reached EOF */
#if YYDEBUG
		if ( yydebug && yytmp )
		{
			register int yy_i;

			printf( "Received token " );
			if ( yychar == 0 )
				printf( "end-of-file\n" );
			else if ( yychar < 0 )
				printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ( ( yy_n += yychar ) < 0 ) || ( yy_n >= YYLAST ) )
			goto yydefault;
		if ( yychk[ yy_n = yyact[ yy_n ] ] == yychar )	/*valid shift*/
		{
			yychar = -1;
			yyval = yylval;
			yy_state = yy_n;
			if ( yyerrflag > 0 )
				yyerrflag--;
			goto yy_stack;
		}

	yydefault:
		if ( ( yy_n = yydef[ yy_state ] ) == -2 )
		{
#if YYDEBUG
			yytmp = yychar < 0;
#endif
			if ( ( yychar < 0 ) && ( ( yychar = yylex() ) < 0 ) )
				yychar = 0;		/* reached EOF */
#if YYDEBUG
			if ( yydebug && yytmp )
			{
				register int yy_i;

				printf( "Received token " );
				if ( yychar == 0 )
					printf( "end-of-file\n" );
				else if ( yychar < 0 )
					printf( "-none-\n" );
				else
				{
					for ( yy_i = 0;
						yytoks[yy_i].t_val >= 0;
						yy_i++ )
					{
						if ( yytoks[yy_i].t_val
							== yychar )
						{
							break;
						}
					}
					printf( "%s\n", yytoks[yy_i].t_name );
				}
			}
#endif /* YYDEBUG */
			/*
			** look through exception table
			*/
			{
				register int *yyxi = yyexca;

				while ( ( *yyxi != -1 ) ||
					( yyxi[1] != yy_state ) )
				{
					yyxi += 2;
				}
				while ( ( *(yyxi += 2) >= 0 ) &&
					( *yyxi != yychar ) )
					;
				if ( ( yy_n = yyxi[1] ) < 0 )
					YYACCEPT;
			}
		}

		/*
		** check for syntax error
		*/
		if ( yy_n == 0 )	/* have an error */
		{
			/* no worry about speed here! */
			switch ( yyerrflag )
			{
			case 0:		/* new error */
				yyerror( (nl_msg(30003,"syntax error")) );
				yynerrs++;
				goto skip_init;
			yyerrlab:
				/*
				** get globals into registers.
				** we have a user generated syntax type error
				*/
				yy_pv = yypv;
				yy_ps = yyps;
				yy_state = yystate;
				yynerrs++;
			skip_init:
			case 1:
			case 2:		/* incompletely recovered error */
					/* try again... */
				yyerrflag = 3;
				/*
				** find state where "error" is a legal
				** shift action
				*/
				while ( yy_ps >= yys )
				{
					yy_n = yypact[ *yy_ps ] + YYERRCODE;
					if ( yy_n >= 0 && yy_n < YYLAST &&
						yychk[yyact[yy_n]] == YYERRCODE)					{
						/*
						** simulate shift of "error"
						*/
						yy_state = yyact[ yy_n ];
						goto yy_stack;
					}
					/*
					** current state has no shift on
					** "error", pop stack
					*/
#if YYDEBUG
#	define _POP_ "Error recovery pops state %d, uncovers state %d\n"
					if ( yydebug )
						printf( _POP_, *yy_ps,
							yy_ps[-1] );
#	undef _POP_
#endif
					yy_ps--;
					yy_pv--;
				}
				/*
				** there is no state on stack with "error" as
				** a valid shift.  give up.
				*/
				YYABORT;
			case 3:		/* no shift yet; eat a token */
#if YYDEBUG
				/*
				** if debugging, look up token in list of
				** pairs.  0 and negative shouldn't occur,
				** but since timing doesn't matter when
				** debugging, it doesn't hurt to leave the
				** tests here.
				*/
				if ( yydebug )
				{
					register int yy_i;

					printf( "Error recovery discards " );
					if ( yychar == 0 )
						printf( "token end-of-file\n" );
					else if ( yychar < 0 )
						printf( "token -none-\n" );
					else
					{
						for ( yy_i = 0;
							yytoks[yy_i].t_val >= 0;
							yy_i++ )
						{
							if ( yytoks[yy_i].t_val
								== yychar )
							{
								break;
							}
						}
						printf( "token %s\n",
							yytoks[yy_i].t_name );
					}
				}
#endif /* YYDEBUG */
				if ( yychar == 0 )	/* reached EOF. quit */
					YYABORT;
				yychar = -1;
				goto yy_newstate;
			}
		}/* end if ( yy_n == 0 ) */
		/*
		** reduction by production yy_n
		** put stack tops, etc. so things right after switch
		*/
#if YYDEBUG
		/*
		** if debugging, print the string that is the user's
		** specification of the reduction which is just about
		** to be done.
		*/
		if ( yydebug )
			printf( "Reduce by (%d) \"%s\"\n",
				yy_n, yyreds[ yy_n ] );
#endif
		yytmp = yy_n;			/* value to switch over */
		yypvt = yy_pv;			/* $vars top of value stack */
		/*
		** Look in goto table for next state
		** Sorry about using yy_state here as temporary
		** register variable, but why not, if it works...
		** If yyr2[ yy_n ] doesn't have the low order bit
		** set, then there is no action to be done for
		** this reduction.  So, no saving & unsaving of
		** registers done.  The only difference between the
		** code just after the if and the body of the if is
		** the goto yy_stack in the body.  This way the test
		** can be made before the choice of what to do is needed.
		*/
		{
			/* length of production doubled with extra bit */
			register int yy_len = yyr2[ yy_n ];

			if ( !( yy_len & 01 ) )
			{
				yy_len >>= 1;
				yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
				yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
					*( yy_ps -= yy_len ) + 1;
				if ( yy_state >= YYLAST ||
					yychk[ yy_state =
					yyact[ yy_state ] ] != -yy_n )
				{
					yy_state = yyact[ yypgo[ yy_n ] ];
				}
				goto yy_stack;
			}
			yy_len >>= 1;
			yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
			yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
				*( yy_ps -= yy_len ) + 1;
			if ( yy_state >= YYLAST ||
				yychk[ yy_state = yyact[ yy_state ] ] != -yy_n )
			{
				yy_state = yyact[ yypgo[ yy_n ] ];
			}
		}
					/* save until reenter driver code */
		yystate = yy_state;
		yyps = yy_ps;
		yypv = yy_pv;
	}
	/*
	** code supplied by user is placed in this switch
	*/
	switch( yytmp )
	{
		
case 1:

 { Expression = yypvt[-0].ex;} break;
case 2:

 { yyval.ex = yypvt[-1].ex; } break;
case 3:


			{
			if (checkrecursion (yypvt[-2].var, yypvt[-0].ex))
				{
				fprintf (stderr, "calc: Can't have recursive definitions\n");
				Variable[yypvt[-2].var] = NULL;
				}
			else Variable[yypvt[-2].var] = yypvt[-0].ex;
			yyval.ex = yypvt[-0].ex;
		} break;
case 4:

		{
		Constant = (double *) malloc (sizeof (double));
		if (Constant == NULL)
			errorexit ("Out of storage space");
		*Constant = eval (yypvt[-0].ex);
		Tmp1.num = Constant;
		yyval.ex = node (&Tmp1, NUMBER, ENULL, ENULL);
		} break;
case 5:


		{
		Tmp1.opr = '+';
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 6:

		{
		Tmp1.opr = '-';
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 7:

		{
		Tmp1.opr = '*';
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 8:

		{
		Tmp1.opr = '%';
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 9:

		{
		Tmp1.opr = '/';
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 10:

		{
		Tmp1.opr = '^';
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 11:

		{
		Tmp1.opr = '_';
		yyval.ex = node (&Tmp1, OPERATOR, ENULL, yypvt[-0].ex);
		} break;
case 12:

		{
		Tmp1.opr = EQ;
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 13:

		{
		Tmp1.opr = NE;
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 14:

		{
		Tmp1.opr = LE;
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 15:

		{
		Tmp1.opr = '<';
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 16:

		{
		Tmp1.opr = GE;
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 17:

		{
		Tmp1.opr = '>';
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 18:

		{
		Tmp1.opr = '&';
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 19:

		{
		Tmp1.opr = '|';
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 20:

		{
		Tmp1.opr = '!';
		yyval.ex = node (&Tmp1, OPERATOR, ENULL, yypvt[-0].ex);
		} break;
case 21:

		{
		Tmp1.opr = '?';
		Tmp2.opr = ':';
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-4].ex, node (&Tmp2, OPERATOR, yypvt[-2].ex, yypvt[-0].ex));
		} break;
case 22:

		{
		Tmp1.opr = '?';
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 23:

		{
		Tmp1.opr = ':';
		yyval.ex = node (&Tmp1, OPERATOR, yypvt[-2].ex, yypvt[-0].ex);
		} break;
case 24:

		{
		Tmp1.opr = ACOS;
		yyval.ex = node (&Tmp1, OPERATOR, ENULL, yypvt[-0].ex);
		} break;
case 25:

		{
		Tmp1.opr = ASIN;
		yyval.ex = node (&Tmp1, OPERATOR, ENULL, yypvt[-0].ex);
		} break;
case 26:

		{
		Tmp1.opr = ATAN;
		yyval.ex = node (&Tmp1, OPERATOR, ENULL, yypvt[-0].ex);
		} break;
case 27:

		{
		Tmp1.opr = COS;
		yyval.ex = node (&Tmp1, OPERATOR, ENULL, yypvt[-0].ex);
		} break;
case 28:

		{
		Tmp1.opr = SIN;
		yyval.ex = node (&Tmp1, OPERATOR, ENULL, yypvt[-0].ex);
		} break;
case 29:

		{
		Tmp1.opr = TAN;
		yyval.ex = node (&Tmp1, OPERATOR, ENULL, yypvt[-0].ex);
		} break;
case 30:

		{
		Tmp1.opr = LOG;
		yyval.ex = node (&Tmp1, OPERATOR, ENULL, yypvt[-0].ex);
		} break;
case 31:

		{
		Tmp1.opr = EXP;
		yyval.ex = node (&Tmp1, OPERATOR, ENULL, yypvt[-0].ex);
		} break;
case 32:

		{
		Tmp1.opr = ABS;
		yyval.ex = node (&Tmp1, OPERATOR, ENULL, yypvt[-0].ex);
		} break;
case 33:

		{
		Tmp1.opr = SQRT;
		yyval.ex = node (&Tmp1, OPERATOR, ENULL, yypvt[-0].ex);
		} break;
case 34:

		{
		Tmp1.var = yypvt[-0].var;
		yyval.ex = node (&Tmp1, VARIABLE, ENULL, ENULL);
		} break;
case 35:

		{
		Tmp1.num = yypvt[-0].num;
		yyval.ex = node (&Tmp1, NUMBER, ENULL, ENULL);
		} break;
	}
	goto yystack;		/* reset registers in driver code */
}

# ifdef __RUNTIME_YYMAXDEPTH

static int allocate_stacks() {
	/* allocate the yys and yyv stacks */
	yys = (int *) malloc(yymaxdepth * sizeof(int));
	yyv = (YYSTYPE *) malloc(yymaxdepth * sizeof(YYSTYPE));

	if (yys==0 || yyv==0) {
	   yyerror( (nl_msg(30004,"unable to allocate space for yacc stacks")) );
	   return(1);
	   }
	else return(0);

}


static void free_stacks() {
	if (yys!=0) free((char *) yys);
	if (yyv!=0) free((char *) yyv);
}

# endif  /* defined(__RUNTIME_YYMAXDEPTH) */


int process_expression (char *xpr, char *res)
{
	char	exprline[SBUFSIZ];
	char    tmpres[SBUFSIZ];
	
	strcpy(exprline, xpr);
	strcpy(res,"");

	Eptr = exprline;
	while (isspace (*Eptr)) {
		Eptr++;
	}
	if (*Eptr == '\0' || *Eptr == '?')
	{
			printmenu ();
			return(0);
	}
	
		if (yyparse() == PARSERROR) {
			return(0);
		}

		ptree (Expression, res);

		if (fzero (Answer = eval (Expression))) {
			Answer = 0.0;
		}

		strcat(res," =");

		if (Answer == UNDEFINED) {
			strcat(res," UNDEFINED");
			return(0);
		} else {
			sprintf (tmpres, Format, Answer);
			strcat(res," ");
			strcat(res,tmpres);
			return(1);
		}
	
}

#ifdef STANDALONE_CALCULATOR
int main (argc, argv) int argc; char *argv[];
{
	char res[SBUFSIZ];
	
	if(process_expression (argv[1],res) == 1) {
		printf("%s\n", res);
		return(0);
	} else {
		return(1);		
	}
}
#endif
