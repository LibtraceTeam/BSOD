#include "direction.h"
#include <stdint.h>
#include <endian.h>
#include "libtrace.h"

/**
 * interface just looks at the interface bit in the erf header
 */
int mod_get_direction(struct libtrace_t *trace, void *buffer, int caplen)
{
	return get_direction(trace,buffer,caplen);
}


