/*
 * This file is part of bsod-server
 *
 * Copyright (c) 2004-2011 The University of Waikato, Hamilton, New Zealand.
 * Author: Shane Alcock
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

#include "colours.h"
#include <libtrace.h>
#include <libprotoident.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

/* Colour module based on libprotoident */
typedef enum counters {
	HTTP,
	HTTPS,
	MAIL,
	DNS,
	P2P,
	P2P_UDP,
	WINDOWS,
	GAMES,
	MALWARE,
	VOIP,
	TUNNELLING,
	STREAMING,
	SERVICES,
	FILES,
	REMOTE,
	CHAT,
	ICMP,
	OTHER,
	UNK_TCP,
	UNK_UDP,
	LAST
} counters_t;

char counternames [][256] = {
	"HTTP",
	"HTTPS",
	"Mail",
	"DNS",
	"P2P",
	"P2P UDP",
	"Windows",
	"Games",
	"Malware",
	"VOIP",
	"Tunnelling",
	"Streaming",
	"Services",
	"Files",
	"Remote Access",
	"Chat",
	"ICMP",
	"Other",
	"Unknown TCP",
	"Unknown UDP"
};

static uint8_t countercolours[][3] = {

	{  5, 10,200}, /* HTTP		blue */
	{150,150,240}, /* HTTPS		light blue */
	{200,  5,  5}, /* MAIL		red */
	{200,200,  5}, /* DNS		yellow	*/
	{  5,150,  5}, /* P2P		green */
	{220,160,220}, /* P2P_UDP	plum */
	{ 80, 80,  5}, /* Windows	olive */
	{130,  5,  5}, /* Games		maroon */
	{200,100,  5}, /* Malware	orange */
	{ 30, 85, 30}, /* VOIP		matte green */
	{250,250,250}, /* Tunnelling	white */
	{170,250, 50}, /* Streaming	yellow-green */
	{ 50, 80, 80}, /* Services	dark slate grey */
	{120,100,240}, /* Files		medium slate blue */
	{110,110,110}, /* Remote	grey */
	{240,230,140}, /* Chat		khaki brown */
	{  5,250,200}, /* ICMP		cyan */
	{  5,130,130}, /* Other		teal	*/
	{250,  5,250}, /* TCP		magenta */
	{150,100, 50}  /* UDP		light brown */


};

typedef struct lpi_col {
	uint8_t seen_dir0;
	uint8_t seen_dir1;
	uint8_t transport;
	bool use_ports;

	lpi_module_t *protocol;
	lpi_data_t lpi;
} lpi_col_t;



static lpi_col_t *init_lpi_flow(uint8_t transport, libtrace_packet_t *packet) {
	
	libtrace_tcp_t *tcp = NULL;
	lpi_col_t *col = (lpi_col_t *)malloc(sizeof(lpi_col_t));

	lpi_init_data(&col->lpi);
	col->seen_dir0 = 0;
	col->seen_dir1 = 0;
	col->transport = transport;
	col->protocol = NULL;
	col->use_ports = true;

	if (transport != TRACE_IPPROTO_TCP) {
		col->use_ports = false;
		return col;
	}

	tcp = trace_get_tcp(packet);

	if (tcp && tcp->syn) {
		col->use_ports = false;
	}
	else
		col->use_ports = true;

	return col;
	
}

static bool check_needed(lpi_col_t *col, uint8_t dir) {
	if (col->seen_dir0 && col->seen_dir1)
		return false;
	if (dir == 0 && col->seen_dir0) 
		return false;
	if (dir == 1 && col->seen_dir1) 
		return false;
	return true;
}

static void guess_protocol(unsigned char *id_num, lpi_col_t *col, uint8_t dir,
		uint32_t plen) {
	lpi_category_t cat;

	/* If we've seen traffic in both directions, we aren't going to
	 * change our protocol estimate */
	if (check_needed(col, dir)) {
		col->protocol = lpi_guess_protocol(&col->lpi);
	}

	if (dir == 0 && plen > 0)
		col->seen_dir0 = 1;
	if (dir == 1 && plen > 0)
		col->seen_dir1 = 1;

	cat = lpi_categorise(col->protocol);

	switch(col->protocol->protocol) {
		case LPI_PROTO_HTTP:
			*id_num = HTTP;
			return;
		case LPI_PROTO_HTTPS:
			*id_num = HTTPS;
			return;
		case LPI_PROTO_DNS:
		case LPI_PROTO_UDP_DNS:
			*id_num = DNS;
			return;
		case LPI_PROTO_UNKNOWN:
			*id_num = UNK_TCP;
			return;
		case LPI_PROTO_UDP:
			*id_num = UNK_UDP;
			return;
		case LPI_PROTO_ICMP:
			*id_num = ICMP;
			return;
		case LPI_PROTO_UDP_NETBIOS:
		case LPI_PROTO_NETBIOS:
		case LPI_PROTO_UDP_WIN_MESSAGE:
			*id_num = WINDOWS;
			return;

		case LPI_PROTO_NO_PAYLOAD:
			if (col->transport == 6) {
				*id_num = UNK_TCP;
				return;
			}
			if (col->transport == 17) {
				*id_num = UNK_UDP;
				return;
			}
			break;
		case LPI_PROTO_UNSUPPORTED:
			if (col->transport == 37 || col->transport == 50 ||
					col->transport == 51 ||
					col->transport == 47) {
				*id_num = TUNNELLING;
				return;
			}
			if (col->transport == 17) {
				*id_num = UNK_UDP;
				return;
			}
			break;
		default:
			break;
	}

	switch(cat) {
		case LPI_CATEGORY_MAIL:
			*id_num = MAIL;
			return;
		case LPI_CATEGORY_P2P:
			*id_num = P2P;
			return;
		case LPI_CATEGORY_P2P_STRUCTURE:
			*id_num = P2P_UDP;
			return;
		case LPI_CATEGORY_SERVICES:
			*id_num = SERVICES;
			return;
		case LPI_CATEGORY_CHAT:
			*id_num = CHAT;
			return;
		case LPI_CATEGORY_REMOTE:
			*id_num = REMOTE;
			return;
		case LPI_CATEGORY_STREAMING:
			*id_num = STREAMING;
			return;
		case LPI_CATEGORY_P2PTV:
			*id_num = P2P;
			return;
		case LPI_CATEGORY_GAMING:
			*id_num = GAMES;
			return;
		case LPI_CATEGORY_FILES:
			*id_num = FILES;
			return;
		case LPI_CATEGORY_TUNNELLING:
			*id_num = TUNNELLING;
			return;
		case LPI_CATEGORY_VOIP:
			*id_num = VOIP;
			return;
		case LPI_CATEGORY_MALWARE:
			*id_num = MALWARE;
			return;
		case LPI_CATEGORY_NO_CATEGORY:
			if (col->transport == 6) {
				*id_num = UNK_TCP;
				return;
			}
			if (col->transport == 17) {
				*id_num = UNK_UDP;
				return;
			}
			break;
		default:
			*id_num = OTHER;
			return;
	}	


}

static void guess_using_port(unsigned char *id_num, lpi_col_t *col)
{
	/* Borrowed most of this from the standard colours module, except
	 * for all the dodgy guessing, e.g. P2P etc */
        int protocol = col->transport;
        int port = trace_get_server_port(
                        protocol,
                        col->lpi.server_port,
                        col->lpi.client_port) == USE_SOURCE
                ? col->lpi.server_port
                : col->lpi.client_port;

        switch(port)
        {
        case 80:
                *id_num = HTTP;
                break;

        case 21:
        case 20:
                *id_num = FILES;
                break;

        case 110: /* pop3 */
        case 143: /* imap2 */
        case 220: /* imap3 */
        case 993: /* imap over ssl */
        case 995: /* pop3 over ssl */
        case 465: /* smtp over ssl */
        case 25:
                *id_num = MAIL;
                break;

        case 53:
                *id_num = DNS;
                break;

        case 22:
        case 23:
                *id_num = REMOTE;	/* SSH / Telnet */
                break;

        case 443:
                *id_num = HTTPS;
                break;
	case 6667:
                *id_num = CHAT;		/* IRC */
                break;
        case 10000:
                *id_num = TUNNELLING;	/* Cisco VPN */
                break;
        case 123:
                *id_num = SERVICES;	/* NTP */
                break;
	default:
                switch(protocol)
                {
                case 6:
                        *id_num = UNK_TCP;
                        break;

                case 17:
                        *id_num = UNK_UDP;
                        break;

                case 1:
                        *id_num = ICMP;
                        break;
                case 37:
                case 50:
                case 51:
                        *id_num = TUNNELLING;
                        break;
                default:
                        *id_num = OTHER;
                        break;
                };break;
        };



}

extern "C"
int mod_get_colour(unsigned char *id_num, libtrace_packet_t *packet,
		flow_info_t *f) {

	lpi_col_t *col = NULL;
 	uint8_t dir;

        libtrace_ip_t *ip = NULL;

        uint16_t l3_type;

        uint16_t src_port;
        uint16_t dst_port;
	static bool lpi_init_called = false;
	uint32_t plen = 0;

	if (lpi_init_called == false) {
		lpi_init_library();
		lpi_init_called = true;
	}
        
	ip = (libtrace_ip_t *)trace_get_layer3(packet, &l3_type, NULL);
        if (l3_type != 0x0800 || ip == NULL) {
		*id_num = OTHER;
		return 0;
	}

	if (f->colour_data == NULL) {
		col = init_lpi_flow(ip->ip_p, packet);
		f->colour_data = col;
	} else {
		col = (lpi_col_t *)f->colour_data;
	}

	if (ip->ip_src.s_addr < ip->ip_dst.s_addr)
		dir = 0;
	else
		dir = 1;

        /* Ignore packets where the IP addresses are the same - something is
         * probably screwy and it's REALLY hard to determine direction */
        if (ip->ip_src.s_addr == ip->ip_dst.s_addr) {
		*id_num = OTHER;
                return 0;
	}

	/* Pass the packet into libprotoident so that it can extract any
         * info it needs from this packet */
        lpi_update_data(packet, &col->lpi, dir);

	plen = trace_get_payload_length(packet);

	if (col->use_ports) {
		guess_using_port(id_num, col);
	} else {
		guess_protocol(id_num, col, dir, plen);
	}
	
	return 0;

}

void mod_get_info(uint8_t colours[3], char name[256], int id) {
	
	if (id >= LAST) {
		/* Nothing should be black - it will be invisible - so we
		 * can use that to mark the end of the list */
		
		colours[0] = colours[1] = colours[2] = 0;
		strcpy(name, "<NULL>");
		return;
	}

	colours[0] = countercolours[id][0];
	colours[1] = countercolours[id][1];
	colours[2] = countercolours[id][2];

	strcpy(name, counternames[id]);
}
