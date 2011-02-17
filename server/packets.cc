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
//#include "dagformat.h"


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
//#define INVERSE_DIRECTION

#define EXPIRE_SECS 20

typedef struct ip ip_t;
static uint32_t lastts = 0;
static uint32_t id = 0;

extern int shownondata;
extern int showdata;
extern int showcontrol;

bool enable_rttest = true;
bool enable_darknet = true;


static inline bool operator <(const flow_id_t &a, const flow_id_t &b) {
	/*
	if (a.start[2]!=b.start[2]) 
		return a.start[2] < b.start[2];
	if (a.end[2]!=b.end[2]) 
		return a.end[2] < b.end[2];
	if (a.start[1]!=b.start[1]) 
		return a.start[1] < b.start[1];
	if (a.end[1]!=b.end[1]) 
		return a.end[1] < b.end[1];
	if (a.start[0]!=b.start[0]) 
		return a.start[0] < b.start[0];
	if (a.end[0]!=b.end[0]) 
		return a.end[0] < b.end[0];
	*/
	if (a.ip1 != b.ip1)
		return a.ip1 < b.ip1;
	if (a.ip2 != b.ip2)
		return a.ip2 < b.ip2;
	if (a.port1 != b.port1)
		return a.port1 < b.port1;
	return a.port2 < b.port2;
}

typedef lru<flow_id_t,flow_info_t *> flow_lru_t;
flow_lru_t flows;

/* 
 * zeroes id and empties flows. maybe useful if all clients  disconnect
 * and we want to close the connection to the data source?
 * */
void init_packets()
{
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
void expire_flows(uint32_t time)
{
	uint32_t tmpid, tmpid2; 
	//remove flows till we find one that hasnt expired
	while( !flows.empty() && 
		(time - flows.front().second->time > EXPIRE_SECS)) 
	{
		tmpid = flows.front().second->flow_id[0];
		tmpid2 = flows.front().second->flow_id[1];
		delete(flows.front().second);
		flows.erase(flows.front().first);	

		if(send_kill_flow(tmpid) != 0)
			return;
		if(send_kill_flow(tmpid2) != 0)
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
			++flow_iterator)
	{
		send_update_flow(client, (*flow_iterator).first.start, 
					(*flow_iterator).first.end, 
					(*flow_iterator).second->flow_id[0], 
					(*flow_iterator).first.ip1, (*flow_iterator).first.ip2 );
		send_update_flow(client, (*flow_iterator).first.end, 
					(*flow_iterator).first.start, 
					(*flow_iterator).second->flow_id[1], 
					(*flow_iterator).first.ip2, (*flow_iterator).first.ip1 );
	}
}

//-------------------------------------------------------------
int get_start_pos(float start[3], struct libtrace_packet_t *packet, 
		int iface, struct modptrs_t *modptrs)
{
	if(iface == DIR_OUTBOUND) {
		start[0] = -10;
		modptrs->left(start, SIDE_RIGHT, DIR_OUTBOUND, packet);
	} else if(iface == DIR_INBOUND) {
		start[0] = 10;
		modptrs->right(start, SIDE_LEFT, DIR_INBOUND, packet);
	} else
		return 1;

	return 0;
}
//------------------------------------------------------------
int get_end_pos(float end[3], struct libtrace_packet_t *packet, 
		int iface, struct modptrs_t *modptrs)
{
	if(iface == DIR_OUTBOUND) {
		end[0] = 10;
		modptrs->right(end, SIDE_LEFT, DIR_OUTBOUND, packet);
	} else if(iface == DIR_INBOUND) {
		end[0] = -10;
		modptrs->left(end, SIDE_RIGHT, DIR_INBOUND, packet);
	} else
		return 1;

	return 0;
}

//------------------------------------------------------------
int per_packet(struct libtrace_packet_t *packet, time_t secs, 
		struct modptrs_t *modptrs, RTTMap *map, blacklist *theList )
{

	int hlen = 0;
	struct libtrace_tcp *tcpptr = 0;
	struct libtrace_udp *udpptr = 0;
	int direction = -1;
	int datasize = -1;
	int force_display = 0;
	uint16_t src_port = 0;
	uint16_t dest_port = 0;

	flow_id_t tmpid;
	flow_lru_t::iterator current;

	struct libtrace_ip *p = trace_get_ip(packet);

	if (!p) {
		return 0;
	}
	assert(secs-lastts >= 0);

	assert(modptrs);
	assert(modptrs->colour);

	/*
	 * Find the port numbers in the packet
	 */ 
	bool isTCP = false;
	if((tcpptr = trace_get_tcp(packet)))
	{
		isTCP = true;
		hlen = p->ip_hl * 4;

		datasize = (ntohs(p->ip_len)) - (tcpptr->doff*4 + hlen);

		if(showcontrol && (tcpptr->syn || tcpptr->fin || tcpptr->rst))
			force_display = 1;
		src_port = tcpptr->source;
		dest_port = tcpptr->dest;
	}
	else if((udpptr = trace_get_udp(packet)))
	{
		hlen = p->ip_hl * 4;

		datasize = (ntohs(p->ip_len)) - sizeof(struct libtrace_udp);
		src_port = udpptr->source;
		dest_port = udpptr->dest;
	}

	direction = modptrs->direction(packet);

#ifdef INVERSE_DIRECTION
	direction = !direction;
#endif

	// populate start and end arrays
	// also checks that we want traffic from this iface
	if(get_start_pos(tmpid.start, 
			packet,
			direction,
			modptrs) != 0)
		return 0;

	if(get_end_pos(tmpid.end, 
			packet,
			direction,
			modptrs) != 0)
		return 0;


	// Get the right IP addresses for each end of the flow:
	libtrace_ip *tmpip = trace_get_ip( packet );
	if( direction == DIR_OUTBOUND )
	//if( tmpid.start[0] < tmpid.end[0] )
	{
		tmpid.ip1 = tmpip->ip_src.s_addr;
		tmpid.ip2 = tmpip->ip_dst.s_addr;
		tmpid.port1 = src_port;
		tmpid.port2 = dest_port;
	}
	else
	{
		tmpid.ip1 = tmpip->ip_dst.s_addr;
		tmpid.ip2 = tmpip->ip_src.s_addr;
		tmpid.port1 = dest_port;
		tmpid.port2 = src_port;
	}

	current = flows.find(tmpid);

	if ( current == flows.end() ) // this is a new flow
	{
		std::pair<flow_id_t,flow_info_t *> flowdata;

		flowdata.second = new flow_info_t;
		if (direction == DIR_OUTBOUND) {
			flowdata.second->flow_id[0] = id;
			flowdata.second->flow_id[1] = id + 1;
		} else {
			flowdata.second->flow_id[1] = id;
			flowdata.second->flow_id[0] = id + 1;
		}

		flowdata.second->time = secs;
		flowdata.second->colour_data = NULL;

		id+=2;

		flowdata.first = tmpid;

		current = flows.insert(flowdata);

		if(send_new_flow(tmpid.start, tmpid.end,
					current->second->flow_id[0], tmpid.ip1, tmpid.ip2)!=0)
			return 1;

		if(send_new_flow(tmpid.end, tmpid.start,
					current->second->flow_id[1], tmpid.ip1, tmpid.ip2)!=0)
			return 1;
	}
	else // this is a flow we've already seen
	{
		current->second->time = secs; // update time last seen
	}

	if (modptrs->colour(&(tmpid.type), 
			packet, current->second) != 0)
		return 0;


	// Expire flows is efficient, and will only expire flows that have uh
	// expired, there is no need to only run it once a second
	// We want to run this after we've touched the current flow so we
	// don't expire it only to recreate it again
	expire_flows(secs);

	// BGN RTT code #####################################################################################
	// RTT calculation stuff:
	float speed = -2.0f; // Default. (1.0f)

	if( enable_rttest) 
	{
		double now = trace_get_seconds( packet );
		static double last = now;
		uint16_t dport;
		uint16_t sport;
		PacketTS pts;
		Flow *rtt_flow;
		Flow *rtt_flow_inverse;
		double the_time = 0.0;

		assert(map);

		pts = map->GetTimeStamp( packet );
		dport = trace_get_destination_port( packet );
		sport = trace_get_source_port( packet );

		rtt_flow = new Flow( p->ip_src.s_addr, p->ip_dst.s_addr, 
				sport, dport );
		rtt_flow_inverse = new Flow( p->ip_dst.s_addr, p->ip_src.s_addr,
				dport, sport );

		if( pts.ts > 0 )
		{
			// Add packet:
			map->Add( rtt_flow, pts.ts, now );
		}
		else if( isTCP )
		{
			if( tcpptr != NULL )
			{
				if( tcpptr->syn == 1 && tcpptr->ack == 0 )
				{
					// SYN:
					map->Add( rtt_flow, htonl(tcpptr->seq), now );
				}
				else if( tcpptr->syn == 1 && tcpptr->ack == 1 )
				{
					// SYN/ACK:
					map->Update( rtt_flow_inverse, htonl(tcpptr->ack_seq) - 1,
							htonl(tcpptr->seq) );

				}
				else if( tcpptr->syn == 0 && tcpptr->ack == 1 )
				{
					// ACK:
					if( (the_time = map->Retrieve( rtt_flow, 
									htonl(tcpptr->ack_seq) - 1 )) >= 0.0f )
					{
						speed = convert_speed( now-the_time );
					}
				}
			}
		}
		else if( p->ip_p == 1 ) // isICMP
		{
			// Process packet as ICMP:
			libtrace_icmp *icmpInfo = trace_get_icmp( packet );
			if( icmpInfo != NULL )
			{
				if( icmpInfo->type == 0 )
				{
					// Echo reply:
					if( (the_time = map->Retrieve( rtt_flow_inverse, icmpInfo->un.echo.id ) ) >= 0.0f )
					{
						speed = convert_speed(now-the_time);
					}
				}
				else if( icmpInfo->type == 8 )
				{
					// Echo:
					map->Add( rtt_flow, icmpInfo->un.echo.id, now );
				}
			}
		}


		// BGN broken -------------------------------------------------------------------------
		
		// d^_^b
		if( pts.ts_echo > 0 )
		{
			if((the_time = map->Retrieve(rtt_flow_inverse, pts.ts_echo)) >= 0.0)
			{
				speed = convert_speed( now-the_time );
			}
			else if( speed == -2.0f )
			{
				speed = -1.0f; // -1 tells the client to keep old values
			}
		}

		// END broken -------------------------------------------------------------------------

		delete rtt_flow;
		delete rtt_flow_inverse;

		if( now - last > 60 )
		{
			map->Flush( now );
			last = now;
		}
	}
	// END RTT code. #####################################################################################

	bool is_dark = false;

	if (enable_darknet) 
	{
		switch(direction) 
		{
			case DIR_INBOUND:
				is_dark = theList->is_dark(p->ip_dst.s_addr);
				break;
			case DIR_OUTBOUND:
				theList->set_light(p->ip_src.s_addr);
				break;
		}
	}

	/* The RTT estimator and darknet blacklist both need access to syns
	 * and non data packets, so don't skip them until now
	 */

	if (datasize > -1 && !force_display) {
		// if we don't show non-data, and datasize is 0, return early
		if ((shownondata == 0) && (datasize == 0))
			return 0;
		// if we don't show data, and datasize is > 0, return early
		if ((showdata == 0) && (datasize > 0))
			return 0;
	
	}

	uint32_t send_id;

	if (direction == DIR_OUTBOUND) {
		send_id = current->second->flow_id[0];
	} else {
		send_id = current->second->flow_id[1];
	}
	if(send_new_packet(secs, send_id, 
			tmpid.type, ntohs(p->ip_len), speed, 
			is_dark) !=0)
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

void kill_all()
{
    send_kill_all();
}
