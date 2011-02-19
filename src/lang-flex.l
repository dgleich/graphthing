%option noyywrap case-sensitive

%{
#include "wx/string.h"

extern "C" {
#include <stdlib.h>
#include <string.h>
}

#include "lang.h"
#include "lang-bison.tab.hh"

#define yylval yy_lang_lval

extern int lang_lineno;
extern char *yy_lang_data;

#define YY_INPUT(buf,result,max_size) 		\
	{					\
		if (*yy_lang_data == '\0')	\
			result = YY_NULL;	\
		else {				\
			buf[0] = *yy_lang_data++;	\
			result = 1;		\
		}				\
	}

%}

LWS		[ \t]+
CRRET		\r
NEWLINE		\n

QSTRING		\"(([^"])|(\\\"))*\"

/************** Start of lexical rules **************/
%%

	/* Kill all LWS */
{LWS}
{CRRET}
	/* Treat newlines (almost) the same as LWS */
{NEWLINE}	{ ++lang_lineno; }

	/* Kill single comments */
"#".*{NEWLINE}	{ ++lang_lineno; }

	/* Quoted Strings */
{QSTRING}	{

			/* Fix up quoted quotation marks */
			char *s = yytext;
			while (*s) {
				char *p = strchr (s, '\\');
				if (!p)
					break;
				int plen = strlen (p);
				s = p + 1;
				if (strncmp (p, "\\\"", 2))
					continue;
				memmove (p, s, plen);	// include '\0'
			}

			int len = strlen (yytext);
			char tmp = yytext[len - 1];
			yytext[len - 1] = 0;
			yylval.string = new wxString (&yytext[1], wxConvUTF8);
			yytext[len - 1] = tmp;
			return STRING;
		}

	/* Languages */

"Catalan"	{ yylval.lang = Catalan; return LANGUAGE; }
"ChineseSimp"	{ yylval.lang = ChineseSimp; return LANGUAGE; }
"Danish"	{ yylval.lang = Danish; return LANGUAGE; }
"Dutch"		{ yylval.lang = Dutch; return LANGUAGE; }
"Esperanto"	{ yylval.lang = Esperanto; return LANGUAGE; }
"Finnish"	{ yylval.lang = Finnish; return LANGUAGE; }
"French"	{ yylval.lang = French; return LANGUAGE; }
"German"	{ yylval.lang = German; return LANGUAGE; }
"Greek"		{ yylval.lang = Greek; return LANGUAGE; }
"Italian"	{ yylval.lang = Italian; return LANGUAGE; }
"Norwegian"	{ yylval.lang = Norwegian; return LANGUAGE; }
"Polish"	{ yylval.lang = Polish; return LANGUAGE; }
"Portuguese"	{ yylval.lang = Portuguese; return LANGUAGE; }
"Romanian"	{ yylval.lang = Romanian; return LANGUAGE; }
"Spanish"	{ yylval.lang = Spanish; return LANGUAGE; }
"Swedish"	{ yylval.lang = Swedish; return LANGUAGE; }
"SwedishChef"	{ yylval.lang = SwedishChef; return LANGUAGE; }


"literal"	{ return kwLITERAL; }
"subst"		{ return kwSUBST; }

	/* Any other character */
.		{ return yytext[0]; }
%%
/************** End of lexical rules **************/
