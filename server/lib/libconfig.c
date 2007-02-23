#include "config.h"
#include "config_internal.h"
#include <libconfig.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <syslog.h>
#include <assert.h>


extern FILE *yyin;

//extern int daemonised;

extern int yyparse();

static config_t *config_table;

//int daemonised = 0;

int set_config_int(char *option,int value)
{
  config_t *item;
  //printf("Setting %s to %i\n",option,value);
  for(item=config_table;item->key;item++) {
    if (!strcmp(item->key,option)) {
      if ((item->type&TYPE_MASK) == TYPE_INT 
	  || ((item->type&TYPE_MASK)==TYPE_BOOL && (value==0 || value==1))) {
	if (item->type&TYPE_MULTI) {
	  conf_array_t *a;
	  if (!item->value) {
	    a=malloc(sizeof(conf_array_t));
	    item->value=a;
	    a->items=0;
	    a->data=NULL;
	  }
	  else
	    a=item->value;
	  ++a->items;
	  a->data=realloc(a->data,a->items*sizeof(void *));
	  a->data[a->items-1].i=value;
	  return 0;
	}
	else {
	  *((int *)item->value)=value;
	  return 0;
	}
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
  //printf("Setting `%s' to %s\n",option,value);
  for(item=config_table;item->key;item++) {
    if (!strcmp(item->key,option)) {
      if ((item->type&TYPE_MASK) == TYPE_STR) {
	if (item->type&TYPE_MULTI) {
	  conf_array_t *a;
	  if (!item->value) {
	    a=malloc(sizeof(conf_array_t));
	    item->value=a;
	    a->items=0;
	    a->data=NULL;
	  }
	  else
	    a=item->value;
	  ++a->items;
	  a->data=realloc(a->data,a->items*sizeof(void *));
	  a->data[a->items-1].s=value;
	  return 0;
	}
	else {
	  if (*((char **)item->value))
	    free(*((char **)item->value));
      	  *((char**)item->value)=value;
	  return 0;
	}
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

void logger(int priority, char *fmt, ...)
{
        va_list ap;
        char buffer[513];

        assert(daemonised == 0 || daemonised == 1);

        va_start(ap, fmt);

        if(! daemonised){
                vfprintf(stderr, fmt, ap);
        } else {
                vsnprintf(buffer, sizeof(buffer), fmt, ap);
                syslog(priority, "%s", buffer);
        }
        va_end(ap);

}

// TABS ARE EIGHT CHARACTORS WHO THE HELL INDENTED THIS FILE?!
// vim: sw=2
