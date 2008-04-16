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
 * $Id: bsod_server.h 269 2005-05-05 01:28:27Z perry $
 *
 */



#ifndef _BSOD_SERVER_H
#define _BSOD_SERVER_H
#include <stdint.h>
#include "libtrace.h"

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


typedef int (* colfptr)(unsigned char*,struct libtrace_packet_t *);
typedef void (* inffptr)(uint8_t*,char[256],int);
typedef int (* posfptr)(float[3], 
		side_t side,
		direction_t dir,
		struct libtrace_packet_t *);
typedef int (* dirfptr)(struct libtrace_packet_t *);
typedef void (* initdirfptr)(char* );
typedef int (* initfuncfptr)(const char *);
typedef int (* initsidefptr)(side_t side, const char *);
typedef int (* endfptr)();
typedef int (* endsidefptr)(side_t side);
struct modptrs_t {
	colfptr colour;
	inffptr info;
	posfptr left;
	posfptr right;
	dirfptr direction;
	initdirfptr init_dir;
};


#endif // _BSOD_SERVER_H
