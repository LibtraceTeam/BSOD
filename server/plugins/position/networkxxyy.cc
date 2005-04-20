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
 * $Id$
 *
 */


#include <stdint.h>
#include <unistd.h>
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
#include <libtrace.h>


/**
 * networkxxyy uses the first two octets of the address as the X axis
 * and the second two as the Y axis.
 */
extern "C"
int mod_get_position(float coord[3], side_t side, direction_t dir,
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


	coord[1] = ((float) ((ntohl(ip.s_addr) & 0xffff0000) >> 16)/(12.8*256)) - 10;

	coord[2] = ((float) (ntohl(ip.s_addr) & 0x0000ffff)/(12.8*256)) - 10;

	return 0;
}

