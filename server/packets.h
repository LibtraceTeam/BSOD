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

#ifndef _PACKETS_H
#define _PACKETS_H

#include "dagformat.h"
#include "libtrace.h"
#include "RTTMap.h"
#include "Blacklist.h"

#define SIDE_LENGTH 8
#define MAX_SIZE 10000



int per_packet(struct libtrace_packet_t packet, uint64_t ts, struct modptrs_t *modptrs, RTTMap *map, blacklist *theList);
float convert_speed( float speed );

void kill_all();
void expire_flows(uint32_t time);
void init_packets();
int send_flows(int fd);
#endif // _PACKETS_H
