/*
 * This file is part of bsod-server
 *
 * Copyright (c) 2004 The University of Waikato, Hamilton, New Zealand.
 * Author: Jesse Pouw-Waas
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

#ifndef BLACKLIST_H
#define BLACKLIST_H

#include "string"
#include "stdio.h"
#include "stdlib.h"
#include "libtrace.h"
#include "time.h"
#include <arpa/inet.h>

class blacklist
{
public:
	blacklist(const char *path, int bl_cnt, int save_int);
	~blacklist();

	bool poke(libtrace_packet_t *packet);
private:
	struct BL
	{
		std::string FName;
		bool InUse;
		bool List[256][256];
		BL *next;
	}*head,*cur;
	
	std::string BLpath;
	int BL_cnt;

	double  ptime,
		ctime,
		Save_int;

	bool read(uint8_t *ip);
	bool write(uint8_t *ip);
	bool load();
	bool save(double);
	void delBL(BL*die);
	char* itoa(int input);
};

#endif
