%{
#include <string.h>
#include "config.parser.h"
%}

%x double_quote

%%

\"	BEGIN(double_quote);
<double_quote>[^\"]*	{ yylval.str=(char *)strdup(yytext); return TOK_STRING; }
<double_quote>\"	BEGIN(INITIAL);

false		{ return TOK_FALSE; }
off		{ return TOK_FALSE; }
true		{ return TOK_TRUE; }
on		{ return TOK_TRUE; }
[0-9]+		{ yylval.i=atoi(yytext); return TOK_INTEGER; }
[_A-Za-z0-9]+	{ yylval.str=(char*)strdup(yytext); return TOK_IDENTIFIER; }
\n		{ return TOK_EOL; }
[[:blank:]]	{ return TOK_WHITE; }
#.*\n		{ return TOK_WHITE; }
.		{ return TOK_UNKNOWN; }

%%

void yyerror(char *s)
{
	fprintf(stderr,"error: %s\n",s);
}
