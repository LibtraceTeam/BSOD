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
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
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
#define SIDE_LENGTH 8
#define MAX_SIZE 10000



/** radial is used for the 'globe' picture. The top two octets are used to
 * determine an angle around the circle, and the last two are used to give a
 * length
 */
extern "C"
int mod_get_position(float coord[3], side_t side, direction_t dir,
		struct libtrace_packet_t *packet) { 
	
	struct libtrace_ip *ipptr = trace_get_ip(packet);
	if (!ipptr)
		return 1;

	struct in_addr ip;
	if (CHK_SOURCE(side,dir))
		ip = ipptr->ip_src;
	else
		ip = ipptr->ip_dst;

	float length, angle; 
	ip.s_addr = ntohl(ip.s_addr); 
	angle = ((ip.s_addr & 0xffff0000) >> 16) % MAX_SIZE; 
	length = (ip.s_addr & 0xffff) % MAX_SIZE;

	coord[1] = angle/MAX_SIZE * sin( (2* M_PI * length) / MAX_SIZE) 
	    * SIDE_LENGTH;
	coord[2] = angle/MAX_SIZE * cos( (2* M_PI * length) / MAX_SIZE) 
	    * SIDE_LENGTH;

	return 0;
}

