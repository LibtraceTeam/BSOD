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
 * $Id: packets.cc 392 2007-03-19 04:43:08Z jpc2 $
 *
 */

#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <vector>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <syslog.h>
#include <map>

#include "lru"


#include "libtrace.h"
#include "dagformat.h"


#include "packets.h"
#include "debug.h"
#include "socket.h" 
#include "bsod_server.h"

#include "RTTMap.h"

#ifndef LIBTRACE_API_VERSION
# error "Your version of libtrace is too old, you need 2.0.16 or higher"
#else
# if LIBTRACE_API_VERSION < 0x010010
#  error "Your version of libtrace is too old, you need 2.0.16 or higher"
# endif
#endif


// Direction fix while the RT client direction stuff is screwed:

#define EXPIRE_SECS 20

typedef struct ip ip_t;
static uint32_t lastts = 0;
static uint32_t id = 0;

extern int shownondata;
extern int showdata;
extern int showcontrol;

bool enable_rttest = true;
bool enable_darknet = true;

struct flow_id_t {
	float start[3];
	float end[3];
	unsigned char type; // Type of flow. ("colour")
	uint32_t ip1;
	uint32_t ip2;
};

struct flow_info_t {
	uint32_t flow_id;
	uint32_t time; 
};

static inline bool operator <(const flow_id_t &a, const flow_id_t &b) {
	if(a.ip1 != b.ip1)
		return a.ip1 < b.ip1;
	if(a.ip2 != b.ip2)
		return a.ip2 < b.ip2;
	return a.type < b.type;
}

typedef lru<flow_id_t,flow_info_t> flow_lru_t;
flow_lru_t flows;

/* 
 * zeroes id and empties flows. maybe useful if all clients  disconnect
 * and we want to close the connection to the data source?
 * */
void init_packets() {
	id = 0;
	lastts = 0;
	while(!flows.empty())
		flows.erase(flows.front().first);
}

//------------------------------------------------------------

/**
 * A flow can only be expired after a number of seconds of inactivity.
 * This should be longer than they are displayed on the clients visualisation
 * otherwise they might be deleted too early and have them disappear from
 * the display.
 */
void expire_flows(uint32_t time) {
	uint32_t tmpid; 
	//remove flows till we find one that hasnt expired
	while( !flows.empty() && 
		(time - flows.front().second.time > EXPIRE_SECS)) {
		tmpid = flows.front().second.flow_id;
		flows.erase(flows.front().first);	

		if(send_kill_flow(tmpid) != 0)
			return;
	}
}

//--------------------------------------------------------

/*
 * When a new client connects, send them information about every flow that is
 * in progress.
 */ 
void send_flows(struct client *client)
{
	Log(LOG_DAEMON|LOG_INFO,"Updating new client with all flows in progress...\n");
	for(flow_lru_t::const_iterator flow_iterator=flows.begin();
			flow_iterator!=flows.end();
			++flow_iterator) {
		send_update_flow(client,(*flow_iterator).first.start, 
					(*flow_iterator).first.end,
					(*flow_iterator).second.flow_id, 
					(*flow_iterator).first.ip1, (*flow_iterator).first.ip2 );
	}
}

//------------------------------------------------------------
// BNG RTT code
float calculate_rtt(libtrace_packet_t * packet, libtrace_ip_t * p, libtrace_tcp_t * tcpptr, RTTMap * map, bool isTCP) {
	// RTT calculation stuff:
	float speed = -2.0f; // Default. (1.0f)

	if( !enable_rttest ) return speed;

	double now = trace_get_seconds( packet );
	static double last = now;
	double the_time = 0.0;
	PacketTS pts = map->GetTimeStamp( packet );
	uint16_t dport = trace_get_destination_port( packet );
	uint16_t sport = trace_get_source_port( packet );
	Flow * rtt_flow = new Flow( p->ip_src.s_addr, p->ip_dst.s_addr, sport, dport );
	Flow * rtt_flow_inverse = new Flow( p->ip_dst.s_addr, p->ip_src.s_addr, dport, sport );

	if( pts.ts > 0 ) { // add packet:
		map->Add( rtt_flow, pts.ts, now );
	} else if( isTCP && tcpptr) {
		if( tcpptr->syn == 1 && tcpptr->ack == 0 ) { // SYN:
			map->Add( rtt_flow, htonl(tcpptr->seq), now );
		} else if( tcpptr->syn == 1 && tcpptr->ack == 1 ) { // SYN/ACK:
			map->Update( rtt_flow_inverse, htonl(tcpptr->ack_seq)-1, htonl(tcpptr->seq) );
		} else if( tcpptr->syn == 0 && tcpptr->ack == 1 ) { // ACK:
			if( (the_time = map->Retrieve( rtt_flow, htonl(tcpptr->ack_seq) - 1 )) >= 0.0f )
				speed = convert_speed( now-the_time );
		}
	} else if( p->ip_p == 1 ) { // isICMP
		libtrace_icmp *icmpInfo = trace_get_icmp( packet );
		if( icmpInfo != NULL ) {
			if( icmpInfo->type == 0 ) { // Echo reply:
				if( (the_time = map->Retrieve( rtt_flow_inverse, icmpInfo->un.echo.id ) ) >= 0.0f )
					speed = convert_speed(now-the_time);
			} else if( icmpInfo->type == 8 ) { // Echo:
				map->Add( rtt_flow, icmpInfo->un.echo.id, now );
			}
		}
	}

	if( pts.ts_echo > 0 ) {
		if((the_time = map->Retrieve(rtt_flow_inverse, pts.ts_echo)) >= 0.0) {
			speed = convert_speed( now-the_time );
		} else if( speed == -2.0f ) {
			speed = -1.0f; // -1 tells the client to keep old values
		}
	}

	if( now - last > 60 ) {
		map->Flush( now );
		last = now;
	}
	delete rtt_flow;
	delete rtt_flow_inverse;
	
	return speed;
}

//------------------------------------------------------------
int get_payload_size(libtrace_packet_t * packet, libtrace_ip_t * ip, int * force_display, bool * isTCP) {
	int hlen = 0;
	libtrace_tcp_t * tcpptr;
	libtrace_udp_t * udpptr;
	int datasize = -1;
	if(ip->ip_p == 6) {
		tcpptr = trace_get_tcp(packet);
		if(!tcpptr) return -1;
		*isTCP = true;
		hlen = ip->ip_hl * 4;
		datasize = (ntohs(ip->ip_len)) - (tcpptr->doff*4 + hlen);
		if(showcontrol && (tcpptr->syn || tcpptr->fin || tcpptr->rst))
			*force_display = 1;
	} else if(ip->ip_p == 17) {
		udpptr = trace_get_udp(packet);
		hlen = ip->ip_hl * 4;
		datasize = (ntohs(ip->ip_len)) - sizeof(libtrace_udp_t);
	}
	return datasize;
}

//------------------------------------------------------------
flow_lru_t::iterator update_flow(flow_id_t tmpid, time_t secs, bool * new_flow) {
	flow_lru_t::iterator current = flows.find(tmpid);

	if ( current == flows.end() ) { // this is a new flow
		std::pair<flow_id_t,flow_info_t> flowdata;

		flowdata.second.flow_id = id;
		flowdata.second.time = secs;
		id++;
		flowdata.first = tmpid;
		current = flows.insert(flowdata);
		*new_flow = true;
	} else { // this is a flow we've already seen
		current->second.time = secs; // update time last seen
	}

	return current;
}

//------------------------------------------------------------
bool is_darknet(int direction, libtrace_ip_t * ip, blacklist *theList) {
	if (!enable_darknet) 
		return false;
	
	switch(direction) {
		case DIR_INBOUND:
			return theList->is_dark(ip->ip_dst.s_addr);
		case DIR_OUTBOUND:
			theList->set_light(ip->ip_src.s_addr);
			return false;
	}
	return false;
}

//------------------------------------------------------------
int per_packet(struct libtrace_packet_t *packet, time_t secs, 
		struct modptrs_t *modptrs, RTTMap *map, blacklist *theList ) {
	bool isTCP = false;
	bool new_flow = false;
	bool is_dark = false;
	libtrace_tcp_t *tcpptr = 0;
	int datasize = -1;
	int direction = -1;
	int force_display = 0;

	flow_id_t tmpid;
	flow_lru_t::iterator current;

	libtrace_ip_t *ip = trace_get_ip(packet);
	if (!ip) return 0;

	// calculate the payload size
	datasize = get_payload_size(packet, ip, &force_display, &isTCP);

	// get traffic type
	if (modptrs->colour(&(tmpid.type),
                        packet) != 0)
                return 0;

	// get direction
	//tmpid.dir = modptrs->direction(packet);
	direction = modptrs->direction(packet);
#ifdef INVERSE_DIRECTION
	direction = !direction;
#endif

	// Get the right IP addresses for each end of the flow:
	if( direction == DIR_OUTBOUND ) {
		tmpid.ip1 = ip->ip_src.s_addr;
		tmpid.ip2 = ip->ip_dst.s_addr;
	} else {
		tmpid.ip1 = ip->ip_dst.s_addr;
		tmpid.ip2 = ip->ip_src.s_addr;
	}

	current = update_flow(tmpid, secs, &new_flow);
	if(new_flow && send_new_flow(tmpid.start, tmpid.end,
				current->second.flow_id, tmpid.ip1, tmpid.ip2)!=0)
		return 1;
	
	expire_flows(secs);

	float speed = calculate_rtt(packet, ip, tcpptr, map, isTCP);

	is_dark = is_darknet(direction, ip, theList);	

	if (datasize > -1 && !force_display) {
		// if we don't show non-data, and datasize is 0, return early
		if ((shownondata == 0) && (datasize == 0))
			return 0;
		// if we don't show data, and datasize is > 0, return early
		if ((showdata == 0) && (datasize > 0))
			return 0;
	}

	if(send_new_packet(secs, current->second.flow_id, tmpid.type, 
				ntohs(ip->ip_len), speed, is_dark) !=0)
		return 1;

	return 0;
}


// Convert the speed into something that looks good (vaguely logarithmic)
// This function could probably be replaced with something a little more clever.
float convert_speed( float speed )
{
	if( speed <= 0.0005f ) 	return( 4.0f ); 
	if( speed <= 0.005f ) 	return( 3.0f );
	if( speed <= 0.05f )	return( 2.0f ); 
	if( speed <= 0.25f )	return( 1.75f ); 
	if( speed <= 0.5f )	return( 1.5f ); 
	if( speed <= 1.0f )	return( 1.25f ); 
	if( speed <= 2.5f )	return( 1.01f ); 
	if( speed <= 5.0f )	return( 0.75f );
	return( 0.5f );
}

void kill_all() {
    send_kill_all();
}
