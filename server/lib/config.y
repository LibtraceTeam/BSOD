%{
#include "config_internal.h"
#include <stdio.h>

int yylex();
void yyerror(const char *);

%}
%token <str>TOK_IDENTIFIER
%token <str>TOK_STRING
%token TOK_FALSE
%token TOK_TRUE
%token TOK_WHITE
%token TOK_EOL
%token TOK_UNKNOWN
%token <i>TOK_INTEGER

%union {
	char *str;
	int i;
}

%%

config
: /* Empty */ 
| config lws TOK_EOL
| config lws directive TOK_EOL
| error TOK_EOL { printf("Expected configuration directive\n"); }
;

directive
: TOK_IDENTIFIER lws TOK_INTEGER lws { set_config_int($1,$3); }
| TOK_IDENTIFIER lws TOK_STRING lws  { set_config_str($1,$3); }
| TOK_IDENTIFIER lws TOK_TRUE lws    { set_config_int($1,1); }
| TOK_IDENTIFIER lws TOK_FALSE lws   { set_config_int($1,0); }
;

lws
: /* empty */
| lws TOK_WHITE { }
;

