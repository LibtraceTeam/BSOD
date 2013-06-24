/*
 * This file is part of bsod-server
 *
 * Copyright (c) 2004-2013 The University of Waikato, Hamilton, New Zealand.
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

#ifndef BSODEXPORTER_H_
#define BSODEXPORTER_H_

#include <libwandevent.h>

#include "bsod_server.h"

#define BSOD_PROTOCOL_VERSION 0x15

#define BSOD_PKTTYPE_FLOW 0x0
#define BSOD_PKTTYPE_PACKET 0x1
#define BSOD_PKTTYPE_KILLFLOW 0x2
#define BSOD_PKTTYPE_KILLALL 0x3
#define BSOD_PKTTYPE_COLOURS 0x4
#define BSOD_PKTTYPE_IMAGE 0x5

#define BSOD_IMAGE_LEFT 0
#define BSOD_IMAGE_RIGHT 1

struct export_params {

	int fifo_size;
	char *left_image_file;
	char *right_image_file;
	struct modptrs_t *modptrs;
};

struct image_data_t {
	unsigned char type;
	unsigned char id; // 0 = left, 1 = right
	uint32_t length;
} __attribute__((packed));

// Colour table
struct flow_descriptor_t {
        unsigned char type;
        unsigned char id;
        uint8_t colour[3];
        uint8_t namelen;
} __attribute__((packed));

/* structure for flow update packets */
struct flow_update_t {
        unsigned char type;
        float x1;
        float y1;
        float z1;
        float x2;
        float y2;
        float z2;
        uint32_t count;
        uint32_t ip1;
        uint32_t ip2;
} __attribute__((packed));

/* structure for new packet packets */
struct pack_update_t {
        unsigned char type;
        uint32_t ts;
        uint32_t id; // Flow id
        unsigned char id_num; // packet type id
        uint16_t size;
        float speed; // This affects the speed of the entire flow based on RTT
        uint8_t dark;
} __attribute__((packed));

/* structure for expire flow packets */
struct flow_remove_t {
        unsigned char type;
        uint32_t id;
} __attribute__((packed));

// Kill all
struct kill_all_t {
    unsigned char type;
} __attribute__((packed));


void init_exporter(wand_event_handler_t *evhdl, struct export_params *eps);
void *exporter_thread(void *ptr);
int create_client(int fd);

void export_new_packet(uint32_t ts, uint32_t id, unsigned char id_num,
		uint16_t size, float speed, bool dark);
void export_new_flow(float start[3], float end[3], uint32_t id, uint32_t ip1,
		uint32_t ip2);
void export_kill_flow(uint32_t id);
void export_kill_all(void);
void export_existing_flow(int fd, float start[3], float end[3], uint32_t id,
		uint32_t ip1, uint32_t ip2);

bool activeClients(void);

#endif
