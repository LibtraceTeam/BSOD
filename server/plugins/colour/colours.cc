/*
 * This file is part of bsod-server
 *
 * Copyright (c) 2004-2011 The University of Waikato, Hamilton, New Zealand.
 * Authors: Brendon Jones
 *          Daniel Lawson
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
#include <inttypes.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stddef.h>
#include <string.h>

// ENUMs for the protocols. Number = id used when packets are sent.
typedef enum counters {
	TCP = 0,
	HTTP = 1,
	HTTPS = 2,
	MAIL = 3,
	FTP = 4,
	VPN = 5,
	DNS = 6,
	NTP = 7,
	SSH = 8,
	UDP = 9,
	ICMP = 10,
	IRC = 11,
	WINDOWS = 12,
	P2P = 13,
	GAMES = 14,
	OTHER = 15,
	LAST = 16 // This must always be the last value in the enum to mark how many items there are!
} counters_t;

// Define colours to use for the various protocols
static uint8_t countercolours[][3] = {
	{100,  5,100}, /* TCP     purple		*/
	{  5, 10,200}, /* HTTP    blue		*/
	{150,150,240}, /* HTTPS   light blue/purple	*/
	{200,  5,  5}, /* MAIL    red		*/
	{  5,150,  5}, /* FTP     green		*/
	{  5,250,  5}, /* VPN     white		*/
	{200,200,  5}, /* DNS     yellow		*/
	{ 30, 85, 30}, /* NTP     matte green	*/
	{110,110,110}, /* SSH     grey		*/
	{150,100, 50}, /* UDP     light brown	*/
	{  5,250,200}, /* ICMP    teal		*/
	{240,230,140}, /* IRC     khaki brown	*/
	{200,100,  5}, /* WINDOWS orange		*/
	{ 50,150, 50}, /* P2P     Icky green        */
	{ 85, 30, 30}, /* GAMES   Icky green        */
	{255,192,203}  /* OTHER   pink		*/
};

struct ports_t {
	uint16_t src;
	uint16_t dst;
};

#define SW_IP_OFFMASK 0xff1f

static uint16_t get_source_port(struct libtrace_ip *ip)
{
	if (6 != ip->ip_p
	  && 17 != ip->ip_p)
		return 0;
	if (0 != (ip->ip_off & SW_IP_OFFMASK))
		return 0;

	struct ports_t *port;
	port = (struct ports_t *)((ptrdiff_t)ip + (ip->ip_hl * 4));

	return htons(port->src);
}

static uint16_t get_destination_port(struct libtrace_ip *ip)
{
	if (6 != ip->ip_p
	  && 17 != ip->ip_p)
		return 0;

	if (0 != (ip->ip_off & SW_IP_OFFMASK))
		return 0;

	struct ports_t *port;
	port = (struct ports_t *)((ptrdiff_t)ip + (ip->ip_hl * 4));

	return htons(port->dst);
}

// Define the names displayed on the client when a filter is applied:
char counternames [][256] = {
	"TCP",
	"HTTP",
	"HTTPS",
	"Mail",
	"FTP",
	"VPN",
	"DNS",
	"NTP",
	"SSH",
	"UDP",
	"ICMP",
	"IRC",
	"Windows",
	"P2P",
	"GAMES",
	"Other"
};

/*
* Sets the colour array (RGB) to be the colour appropriate to the 
* port/protocol being used.
*/
extern "C"
int mod_get_colour(unsigned char *id_num, struct libtrace_packet_t *packet,
		flow_info_t *f)
{

	struct libtrace_ip *ip = trace_get_ip(packet);
	
	/* We don't use the flow info in this plugin */
	assert(f->colour_data == NULL);

	if (!ip) {
		*id_num = OTHER;
		return 0;
	}

	int protocol = ip->ip_p;
	int port = trace_get_server_port(
			protocol,
			get_source_port(ip),
			get_destination_port(ip)) == USE_SOURCE 
		? get_source_port(ip)
		: get_destination_port(ip);

	switch(port)
	{
	case 80:
		*id_num = HTTP;
		break;

	case 21: 
	case 20: 
		*id_num = FTP;
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
		*id_num = SSH;
		break;

	case 443:
		*id_num = HTTPS;
		break;

	case 6667:
		*id_num = IRC;
		break;
	case 10000:
		*id_num = VPN;
		break;
	case 123:
		*id_num = NTP;
		break;
	case 135:
	case 136:
	case 137:
	case 138:
	case 139:
	case 445:
	case 1053:
		*id_num = WINDOWS;
		break;

		// P2P:
		// GNUtella 
	case 6346:
		// Kazaa/Fasttrack (Note many other ports from 1000-4000 also used)
	case 1214:
		// EMule/EDonkey
	case 4661:
	case 4662:
	case 4665:
	case 4672:
		// iMesh
	case 4329:
		// Bittorrent (common ports - goes up to 6999)
	case 6881:
	case 6882:
	case 6883:
	case 6884:
	case 6885:
	case 6886:
	case 6887:
	case 6888:
	case 6889:
	case 6890:
	case 6891:
	case 6892:
	case 6893:
	case 6894:
	case 6895:
	case 6896:
	case 6897:
	case 6898:
	case 6899:
	case 6900:
	case 6901:
		*id_num = P2P;
		break;
		
	case 3724:		// World of Warcraft
	case 14567:		// Battlefield 1942
	case 20100:		// Soldier of Fortune II
	case 27015:		// Half-Life / Half-Life 2
	case 27016:		// Counter-Strike
	case 27960:		// Quake III Arena
	case 28070:		// Jedi Knight 2: Jedi Outcast
	case 28960:		// Call of Duty
		*id_num = GAMES;
		break;
	
	// if not a port that I'm counting give a colour based on protocol
	default:  
		switch(protocol)
		{
		case 6:
			*id_num = TCP;
			break;

		case 17:
			*id_num = UDP;
			break;

		case 1:
			*id_num = ICMP;
			break;
			/* 
			case 41: for(i=0;i<3;i++)
			colour[i] = countercolours[IPMP][i]; 
			break;
			*/
		case 37:
		case 50:
		case 51:
			*id_num = VPN;
			break;
		default:
			*id_num = OTHER;
			break;
		};break;
	};
	//printf("%u %u %u \n", colour[0], colour[1], colour[2]);
	//
	return 0;

}

void mod_get_info(uint8_t colours[3], char name[256], int id )
{
	if( id >= LAST )
	{
		// We never want anything to be pure black (it would be invisible)
		// so we mark the end of the list with pure black RGB=0,0,0).
		colours[0] = colours[1] = colours[2] = 0;
		strcpy( name, "<NULL>" );
		return;
	}
	colours[0] = countercolours[id][0];
	colours[1] = countercolours[id][1];
	colours[2] = countercolours[id][2];
	strcpy( name, counternames[id] );
}
