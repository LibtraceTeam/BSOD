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

#ifndef _SOCKET_H
#define _SOCKET_H
int setup_listen_socket();
int bind_tcp_socket(int listener, int port);
int check_clients(bool wait);
int send_new_flow(float start[3], float end[3], uint32_t count);
int send_update_flow(int fd, float start[3], float end[3], uint32_t count);
int send_new_packet(uint64_t ts, uint32_t id, uint8_t colour[3], 
	uint16_t size, float speed, bool dark);
int send_kill_flow(uint32_t id);
void hax_fdmax(int fd);

#endif // _SOCKET_H
