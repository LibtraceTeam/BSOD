/*
 * This file is part of bsod-server
 *
 * Copyright (c) 2004-2011 The University of Waikato, Hamilton, New Zealand.
 * Authors: Brendon Jones
 *	    Daniel Lawson
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
#include "../../daemons.h"
#include <syslog.h>


int macsloaded = 0;
uint64_t *macs = 0;


inline uint64_t mac_to_uint64(const uint8_t mac[ETHER_ADDR_LEN]) {
    uint64_t address = 0;
    int i = 0;
    for (i = 0; i < ETHER_ADDR_LEN; i++) {
	address = address << 8;
	address = address | mac[i];
    }
    return address;
}


inline int is_local(const uint8_t mac[ETHER_ADDR_LEN]) {
    int i = 0;
    uint64_t address = mac_to_uint64(mac);
    for (i = 0; i < macsloaded; i++) {
	if (macs[i] == address)
	    return 1;
    }
    return 0;
}



/**
 * Read in all the macs from the file specified in the config.
 */ 
extern "C"
int init_module(const char* filename)
{
    FILE *fin = 0;
    char *buffer;
    int lines = 0;

    buffer = (char *)malloc(65536);
    if( (fin = fopen(filename,"r")) == NULL)
    {
	Log(LOG_DAEMON|LOG_INFO,
		"Couldn't load mac address file '%s', giving up\n", filename);
	exit(1);
    }

    while (fgets(buffer,65536,fin)) {
	lines ++;
    }

    rewind(fin);
    if (macs) {
	free(macs);
	macs = 0;
    }
    macs = (uint64_t*)malloc(sizeof(uint64_t) * lines);

    while(fgets(buffer,65536,fin)) {
	macs[macsloaded] = mac_to_uint64(trace_ether_aton(buffer,NULL));
	macsloaded++;
    }


    free(buffer);

    return(1);
}





/**
 * destmac looks at the destination MAC of the packet
 */
extern "C"
int mod_get_direction(struct libtrace_packet_t *packet)
{
    struct ether_header *ethptr;
	libtrace_linktype_t linktype;
	uint32_t rem;

    ethptr = (struct ether_header *)trace_get_layer2(packet, &linktype, &rem);


    /* Set direction bit based on upstream routers' MAC */
    /* If it is destined for a listed MAC, it's outbound (set to 0) */
    /* If it originates from a listed MAC, it's inbound (set to 1) */
    /* If its neither (slightly likely) set to 2 */
    if(is_local(ethptr->ether_dhost)) {
	return 0;
    } else if (is_local(ethptr->ether_shost)) {
	return 1;
    } else {
	return 2;
    }
    
}

