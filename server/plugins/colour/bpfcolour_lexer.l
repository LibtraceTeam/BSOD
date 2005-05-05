%{
#include <string.h>
#include "bpfcolour.h"
#include <ctype.h>
#include "y.tab.h"


static char hexdigit2dec(const char s);
char hex2dec(const char *s);
%}

%option nounput
%option noyywrap
%option yylineno

%%
^#[^\n]*	{ return TOK_WHITE; }
\"[^"]*\"	{ colour_lval.str = (char *)strndup(colour_text+1,strlen(colour_text)-2); return TOK_STRING; }
[[:blank:]]	{ return TOK_WHITE; }
#[a-fA-F0-9]{6} { colour_lval.colour.red = hex2dec(colour_text+1);
		  colour_lval.colour.blue = hex2dec(colour_text+3);
		  colour_lval.colour.green = hex2dec(colour_text+5);
		  return TOK_COLOUR;
		  }
[A-Za-z0-9_]+	{ colour_lval.str = (char *)strdup(colour_text); return TOK_STRING; }
\n		{ return TOK_EOL; }
.		{ return TOK_UNKNOWN; }

%%
void colour_error(char *s)
{
	fprintf(stderr,"Parse error on line %i: %s\n",colour_lineno,s);
}

static char hexdigit2dec(const char s)
{
	const char *hexdigits="0123456789abcdef";
	return strchr(hexdigits,tolower(s))-hexdigits;
}

char hex2dec(const char *s)
{
	return (hexdigit2dec(s[0])<<8)|(hexdigit2dec(s[1]));
}
