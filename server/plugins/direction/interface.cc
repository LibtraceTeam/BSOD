#include "direction.h"
#include <stdint.h>
#include <endian.h>
#include "libtrace.h"

/**
 * interface just looks at the interface bit in the erf header
 */
int mod_get_direction(struct libtrace_packet_t packet)
{
	return trace_get_direction(&packet);
}


