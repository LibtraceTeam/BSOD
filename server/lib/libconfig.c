#include "config_internal.h"
#include "debug.h"
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern FILE *yyin;

extern int yyparse();

static config_t *config_table;

int set_config_int(char *option,int value)
{
  config_t *item;
  //printf("Setting %s to %i\n",option,value);
  for(item=config_table;item->key;item++) {
    if (!strcmp(item->key,option)) {
      if ((item->type&TYPE_MASK) == TYPE_INT 
	  || ((item->type&TYPE_MASK)==TYPE_BOOL && (value==0 || value==1))) {
      	*((int *)item->value)=value;
	return 0;
      }
      else {
	printf("%s does not take an int\n",option);
      }
    }
  }
  printf("Unknown configuration option %s\n",option);
  return 1; 
}

int set_config_str(char *option,char *value)
{
  config_t *item;
  //printf("Setting %s to %s\n",option,value);
  for(item=config_table;item->key;item++) {
    if (!strcmp(item->key,option)) {
      if ((item->type&TYPE_MASK) == TYPE_STR) {
	if (*((char **)item->value))
	  free(*((char **)item->value));
      	*((char**)item->value)=value;
	return 0;
      }
      else {
	printf("%s does not take an string\n",option);
      }
    }
  }
  printf("Unknown configuration option %s\n",option);
  return 1; 
}

int parse_config(config_t *config,char *filename)
{
  yyin = fopen(filename,"r");
  config_table=config;
  if (!yyin) {
    return 1;
  }
  yyparse();
  fclose(yyin);
  return 0;
}
