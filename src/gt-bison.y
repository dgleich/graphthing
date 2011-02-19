%{
#include "wx/string.h"

#include <iostream>
#include <stdio.h>
#include <string.h>
#include "edge.h"
#include "graph.h"
#include "vertex.h"


#define YYDEBUG 1
#define YYERROR_VERBOSE

int gt_lineno = 1;
Graph *new_graph;
Edge *e;

extern char *yy_gt_text;

int yy_gt_error (char *s);
int yy_gt_lex (void);
%}

%union {
	int integer;
	char *str;
}

/* Basic tokens */
%token <integer> INTEGER
%token <str> STRING TAG

/* Construction non-terminals */
%type <integer> arrow

/* Keywords */
%token kwINFO "info"    kwDESCRIPTION "description"
%token kwVERTEX "vertex"    kwEDGE "edge"    kwAT "at"    kwWITH "with"
%token kwWEIGHT "weight"

%token kwDASH "--"    kwLEFT "<-"    kwRIGHT "->"



%%	/* Grammar rules and actions follow */

input:	  info_block objects
	;

info_block:
	  "info" '{'
		info_tags
	  '}'
	;

info_tags:
	  /* empty */
	| info_tags info_tag
	;

info_tag:
	  TAG '=' STRING
		{
			wxString tag ($1, wxConvUTF8), value ($3, wxConvUTF8);

			new_graph->set_tag (tag, value);
			delete[] $1;
			delete[] $3;
		}
	;

objects:
	  /* empty */
	| objects vertex
	| objects edge
	;

vertex:
	  "vertex" STRING "at" '(' INTEGER ',' INTEGER ')'
		{
			Vertex *v;

			v = new Vertex ($2, $5, $7);
			new_graph->add (v);
			delete[] $2;
		}
	;

edge:
	  "edge" STRING arrow STRING
		{
			if (!new_graph->find ($2)) {
				yy_gt_text = $2;
				yy_gt_error ("Unknown vertex");
				YYERROR;
			}
			if (!new_graph->find ($4)) {
				yy_gt_text = $4;
				yy_gt_error ("Unknown vertex");
				YYERROR;
			}

			e = new Edge (new_graph->find ($2),
					new_graph->find ($4),
					($3 == 1) ? false : true);
			delete[] $2;
			delete[] $4;
			if ($3 == 2) {
				Vertex *tmp = e->v;
				e->v = e->w;
				e->w = tmp;
			}
		}
	  edge_properties
		{ new_graph->add (e); }
	;

arrow:	  "--"		{ $$ = 1; }
	| "<-"		{ $$ = 2; }
	| "->"		{ $$ = 3; }
	;

edge_properties:
	  /* empty */
	| "with" edge_property
	;

edge_property:
	  "weight" INTEGER	{ e->weight = $2; }
	;

%%

int yy_gt_error (char *s)
{
	fprintf (stderr, "gt-parse: %s in line %i, at symbol \"%s\"\n",
					s, gt_lineno, yy_gt_text);
	return 1;
}
