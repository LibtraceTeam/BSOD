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

/*
 * Number of /24 networks you are mapping. These are spread evenly across
 * the vertical axis
 */

#define NETCOUNT 50  


/** 
 * This is used if you have multiple C-class or /24 CIDR networks you wish
 * to visualise on one side. Different networks are spread across the 
 * vertical axis, and the last octect in each address is used to determine
 * the horizontal position
 */

uint32_t nets[NETCOUNT] = {0};

static int check_subnet(uint32_t net) {
	int i = 0;

	for (i = 0; i < NETCOUNT; i ++) {
		if (nets[i] == net) 
			return i;
		if (nets[i] == 0) {
			nets[i] = net;
			return i;
		}
	}
	return -1;
}

void mod_get_position(float coord[3], struct in_addr ip) {

	int index = -1;
	uint32_t net = 0;

	net = ntohl(ip.s_addr) & 0xffffff00;

	if ((index = check_subnet(net)) == -1) {
		printf("increase NETCOUNT in multiplenet24.cc\n");
		assert(index != -1);
	}
	

	coord[1] = 20.0 * index / (NETCOUNT-1) - 10;
	

	coord[2] = ((float) (ntohl(ip.s_addr) & 0x000000ff)/12.8) - 10;
}

