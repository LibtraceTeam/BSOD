#ifndef CONFIG_H
#define CONFIG_H

#define TYPE_INT	1
#define TYPE_STR	2
#define TYPE_BOOL	3
#define TYPE_MASK	3

#define TYPE_NOTNULL	0x10
#define TYPE_NULL	0x20

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  	char *key;
	int type;
	void *value;
	} config_t;

int parse_config(config_t *config,char *filename);
/* Example:
 *
 * #include <stdio.h>
 * #include "config.h"
 *
 * int foo;
 *
 * struct config_t config[] = {
 * 	{ "foo", TYPE_INT|TYPE_NOTNULL, &foo }
 * 	{ NULL, 0, NULL }
 * 	};
 *
 * int main(int argc,char **argv) 
 * {
 *  if(!parse_config(config,"/usr/local/etc/wand.conf")) {
 *    return 1;
 *  }
 *
 *  printf("foo=%i",foo);
 *  return 0;
 * }
 */

#ifdef __cplusplus
}
#endif
#endif
