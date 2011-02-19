%option noyywrap case-sensitive

%{
#define __NO_WX_HEADERS__
#include <fstream>
#include <stdlib.h>
#include <string.h>


#include "gt-bison.tab.hh"

#define yylval yy_gt_lval

extern int gt_lineno;
std::fstream *yy_gt_fs;

#define YY_INPUT(buf,result,max_size)		\
	{					\
		char _foobuf;			\
		if (yy_gt_fs->get (_foobuf)) {	\
			buf[0] = _foobuf;	\
			result = 1;		\
		} else				\
			result = YY_NULL;	\
	}

%}
LWS		[ \t]+
CRRET		\r
NEWLINE		\n
DIGIT		[0-9]

TAG		[A-Za-z0-9_]+

DEC_NUMBER	{DIGIT}+

QSTRING		\"(([^"])|(\\\"))*\"

/************** Start of lexical rules **************/
%%

	/* Kill all LWS */
{LWS}
{CRRET}
	/* Treat newlines (almost) the same as LWS */
{NEWLINE}	{ gt_lineno++; }

	/* Integer constants */
{DEC_NUMBER}	{
			yylval.integer = atoi (yytext);
			return INTEGER;
		}

	/* Quoted Strings */
{QSTRING}	{
			int len = strlen (yytext);

			yylval.str = new char [len-1];
			memcpy (yylval.str, &yytext[1], (len - 2));
			yylval.str[len-2] = 0;
			return STRING;
		}

	/* Reserved keywords */
"info"		{ return kwINFO; }
"vertex"	{ return kwVERTEX; }
"edge"		{ return kwEDGE; }
"at"		{ return kwAT; }
"with"		{ return kwWITH; }
"weight"	{ return kwWEIGHT; }

"--"		{ return kwDASH; }
"<-"		{ return kwLEFT; }
"->"		{ return kwRIGHT; }

{TAG}		{
			int len = strlen (yytext);

			yylval.str = new char [len+1];
			memcpy (yylval.str, yytext, len);
			yylval.str[len] = 0;
			return TAG;
		}

	/* Any other character */
.		{ return yytext[0]; }
%%
/************** End of lexical rules **************/
