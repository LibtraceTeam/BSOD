/*
 * This file is part of bsod-server
 *
 * Copyright (c) 2004-2011 The University of Waikato, Hamilton, New Zealand.
 * Authors: Brendon Jones
 *          Daniel Lawson
 *          Sebastian Dusterwald
 *          Yuwei Wang
 *          Paul Hunkin
 *          Shane Alcock
 *
 * Contributors: Perry Lorier
 *               Jamie Curtis
 *               Jesse Pouw-Waas
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



#ifndef _POSITION_H
#define _POSITION_H

#include <netinet/in.h>

typedef enum {
	DIR_UNKNOWN = -1,
	DIR_OUTBOUND = 0,
	DIR_INBOUND = 1,
	DIR_OTHER = 2
} direction_t;

typedef enum {
	SIDE_LEFT = 0,
	SIDE_RIGHT = 1
} side_t;

#define CHK_SOURCE(side,dir) ((SIDE_LEFT == (side)) ^ ((dir) == DIR_OUTBOUND))

/** initialise the module
 * @param[in] side	Which side to initialise
 * @param[in] param	The parameters from the config file
 *
 * @note this function may be called twice (once for each side) or only
 * once (if it's only used on one side).
 *
 * @note The param may be an empty string ("") if there is no parameter.
 */
extern "C"
int init_module(side_t side, const char *msg);

/** mod_get_position
 * @param[in,out] coord[3] The 3d coordinate of the position (x,y,z), 
 * 			z is ignored.
 * @param[in] side	Which side this layout module is being used for
 * @param[in] dir	Which direction this packet is travelling
 * @param[in] ip 	The IP address.
 *
 * @returns 1 on failure, 0 on success
 *
 * This function should take an ip address and return a position on a 2d plane.
 * x and y are stored in coord[0] and coord[1].  coord[2] is ignored.
 * coord[0] and coord[1] must be in the range -10..10.
 */
extern "C" int mod_get_position(float coord[3], 
		side_t side, direction_t dir,
		struct libtrace_packet_t *packet);

#endif // _POSITION_H
