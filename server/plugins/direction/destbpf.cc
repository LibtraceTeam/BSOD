/*
 * This file is part of bsod-server
 *
 * Copyright (c) 2004-2011 The University of Waikato, Hamilton, New Zealand.
 * Author: Perry Lorier
 *          
 * All rights reserved.
 *
 * This code has been developed by the University of Waikato WAND 
 * research group. For further information please see http://www.wand.net.nz/
 *
 * bsod-server is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * bsod-server is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bsod-server; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id$
 *
 */


#include "direction.h"
#include <stdint.h>
#include "libtrace.h"

#include <stdio.h>
#include <stdlib.h>
#include <net/ethernet.h>
#include <err.h>

#include "../../debug.h"
#include <syslog.h>
#include <string.h>

static char *bpf_exp = NULL;
static struct libtrace_filter_t *filter = NULL;


/**
 * Read in all the macs from the file specified in the config.
 */ 
extern "C"
int init_module(const char* bpf)
{
	bpf_exp = strdup(bpf);
	filter = trace_create_filter(bpf);
	return(1);
}


/**
 * Free any resources allocated during the init_dir
 */
extern "C"
void end_module()
{
	trace_destroy_filter(filter);
	free(bpf_exp);
}


/**
 * destmac looks at the destination MAC of the packet
 */
extern "C"
int mod_get_direction(struct libtrace_packet_t *packet)
{

    if (trace_apply_filter(filter,packet))
	    return DIR_OUTBOUND;
    else
	    return DIR_INBOUND;
}


