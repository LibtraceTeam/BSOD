#ifndef _INTERFACE_H
#define _INTERFACE_H

#include <endian.h>
#include <stdint.h>
#include "libtrace.h"
/**
 * interface just looks at the interface bit in the erf header
 */

extern "C" int mod_get_direction(struct libtrace_packet_t packet);


#endif // _INTERFACE_H
