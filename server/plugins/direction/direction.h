#ifndef _DIRECTION_H
#define _DIRECTION_H

#include <endian.h>
#include <stdint.h>
#include "libtrace.h"


extern "C" {
    void mod_init_dir(char* filename);
    int mod_get_direction(struct libtrace_packet_t packet);   
}

#endif // _DIRECTION_H
