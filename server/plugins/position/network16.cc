/*
 * This file is part of bsod-server
 *
 * Copyright (c) 2004 The University of Waikato, Hamilton, New Zealand.
 * Authors: Brendon Jones
 *	    Daniel Lawson
 *	    Sebastian Dusterwald
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
 * $Id: network16.cc 243 2005-04-20 04:47:21Z perry $
 *
 */



#include <unistd.h>
#include <stdint.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <vector>
#include <algorithm>
#include <numeric>
#include <iostream>

#include "position.h"
#include "libtrace.h"


/**
 * This is used when you have a B-class or /16 CIDR network you wish you
 * visualalise on one side. The first two octets are the same, so the last two
 * octets are used for placement.  One dimension is based on the third octet, 
 * the other is based on the last octet.
 */
extern "C"
int mod_get_position(float coord[3], 
		side_t side, direction_t dir,
		struct libtrace_packet_t *packet) {
	struct libtrace_ip *ipptr = trace_get_ip(packet);
	struct in_addr ip;
	if (!ipptr)
		return 1;

	if (CHK_SOURCE(side,dir)) {
		ip = ipptr->ip_src;
	}
	else {
		ip = ipptr->ip_dst;
	}

	coord[1] = ((float) ((ntohl(ip.s_addr) & 0x0000ff00) >> 8)/12.8) - 10;

	coord[2] = ((float) (ntohl(ip.s_addr) & 0x000000ff)/12.8) - 10;

	return 0;
}

