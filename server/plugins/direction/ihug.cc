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


#include "direction.h"
#include <stdint.h>
#include <endian.h>
#include "libtrace.h"

#include <stdio.h>
#include <stdlib.h>
#include <net/ethernet.h>
#include <err.h>

#include "../../debug.h"
#include <syslog.h>

/**
* Look at the IP and see if it comes from the iHug IP range:
*/
int mod_get_direction(struct libtrace_packet_t packet)
{
	/*struct ether_header *ethptr;

	ethptr = (struct ether_header *)trace_get_link(&packet);
*/

	/* Set direction bit based on upstream routers' MAC */
	/* If it is destined for a listed MAC, it's outbound (set to 0) */
	/* If it originates from a listed MAC, it's inbound (set to 1) */
	/* If its neither (slightly likely) set to 2 */
/*	if(is_local(ethptr->ether_dhost)) {
		return 0;
	} else if (is_local(ethptr->ether_shost)) {
		return 1;
	} else {
		return 2;
	}*/
	return( 0 );
}

