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


#ifndef _POSITION_H
#define _POSITION_H

#include <netinet/in.h>
/** mod_get_position
 * @param[out] coord[3] The 3d coordinate of the position (x,y,z), z is ignored.
 * @param[in] ip 	The IP address.
 *
 * This function should take an ip address and return a position on a 2d plane.
 * x and y are stored in coord[0] and coord[1].  coord[2] is ignored.
 * coord[0] and coord[1] must be in the range -10..10.
 */
extern "C" void mod_get_position(float coord[3], struct in_addr ip);

#endif // _POSITION_H
