%{
#include <stdio.h>
#include "bpfcolour.h"
#define YYERROR_VERBOSE 1

extern int colour_lex();
void colour_error(const char *);
%}
%token <str>TOK_STRING
%token <colour>TOK_COLOUR
%token TOK_WHITE
%token TOK_UNKNOWN
%token TOK_EOL

%union {
	char *str;
	struct colour_t colour;
}
%%

config
:
| config lws TOK_EOL
| config lws directive TOK_EOL
/* | error TOK_EOL { printf("Expected config line\n"); }*/
;

directive
: TOK_STRING lws TOK_COLOUR lws TOK_STRING { 
	add_expression($1,$3,$5);
	}
;

lws
: /* empty */
| lws TOK_WHITE { }
;

%%

