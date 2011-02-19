%{
#include "wx/string.h"

extern "C" {
#include <stdio.h>
#include <string.h>
}
#include <stack>
#include "lang.h"

#define YYDEBUG 1
#define YYERROR_VERBOSE

int lang_lineno = 1;

std::stack<PhraseBlock *> pbs;

int yy_lang_error (char *s);
int yy_lang_lex (void);
%}

%union {
	wxString *string;
	Language lang;
}

/* Basic tokens */
%token <string> STRING
%token <lang> LANGUAGE

%token kwLITERAL "literal"	kwSUBST "subst"

/* Construction non-terminals */
%type <string> phrase

%%	/* Grammar rules and actions follow */

input:	  /* empty */
	| input phrase_block
	;

phrase_block:
	  phrase '='
		{
			PhraseBlock *b = new PhraseBlock (*$1);
			delete $1;
			pbs.push (b);
		}
	  phrase_def ';'
		{
			translator->add_phrase (pbs.top ());
			pbs.pop ();
		}
	;

phrase_def:
	  '{'
		{
			pbs.top ()->set_type (PhraseBlock::regular);
		}
	  translations '}'
	| phrase_concat
	| "literal"
		{
			pbs.top ()->set_type (PhraseBlock::literal);
		}
	| phrase "subst" phrase '/' phrase
		{
			pbs.top ()->set_type (PhraseBlock::replace);
			pbs.top ()->set_param (0, *$1);
			pbs.top ()->set_param (1, *$3);
			pbs.top ()->set_param (2, *$5);
			delete $1;
			delete $3;
			delete $5;
		}
	;

phrase_concat:
	  phrase '+' phrase
		{
			pbs.top ()->set_type (PhraseBlock::concat);
			pbs.top ()->set_param (0, *$1);
			pbs.top ()->set_param (0, *$3);
			delete $1;
			delete $3;
		}
	| phrase_concat '+' phrase
		{
			pbs.top ()->set_param (0, *$3);
			delete $3;
		}
	;

translations:
	  /* empty */
	| translations translation
	;

translation:
	  LANGUAGE '=' phrase
		{
			pbs.top ()->add ($1, *$3);
			delete $3;
		}
	;

/***********************************/

phrase:
	  STRING	{ $$ = $1; }
	| phrase STRING
		{
			$1->append (*$2);
			$$ = $1;
			delete $2;
		}
	;

%%

extern char *yy_lang_text;

int yy_lang_error (char *s)
{
	fprintf (stderr, "lang-parse: %s in line %i, at symbol \"%s\"\n",
			s, lang_lineno, yy_lang_text);
	return 1;
}
