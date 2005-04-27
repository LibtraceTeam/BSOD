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


#define EXPIRE_SECS 20

typedef struct ip ip_t;
uint32_t lastts = 0;
uint32_t id = 0;

extern int shownondata;
extern int showdata;
extern int showcontrol;


struct flow_id_t {
	struct in_addr sourceip;
	uint32_t sourceport;
	struct in_addr destip;
	uint32_t destport;
};

struct flow_info_t {
	uint32_t id;
	uint32_t time; 
	unsigned char id_num; // Type of flow.
	float start[3];
	float end[3];
};

bool operator < (const flow_id_t &a, const flow_id_t &b) {
	if (a.sourceport != b.sourceport)
		return a.sourceport < b.sourceport;
	if (a.destport != b.destport)
		return a.destport < b.destport;
	if (a.sourceip.s_addr != b.sourceip.s_addr)
		return a.sourceip.s_addr < b.sourceip.s_addr;
	if (a.destip.s_addr != b.destip.s_addr)
		return a.destip.s_addr < b.destip.s_addr;

	// if all items are the same, it is not less than
	return false;
}

bool operator == (const flow_id_t &a, const flow_id_t &b) {
	/* Ports are the most unique thing, so check them first */
	if (a.sourceport != b.sourceport)
		return false;
	if (a.destport != b.destport)
		return false;
	if (a.sourceip.s_addr != b.sourceip.s_addr)
		return false;
	if (a.destip.s_addr != b.destip.s_addr)
		return false;

	return true;
}


typedef lru<flow_id_t,flow_info_t> flow_lru_t;
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
	uint32_t tmpid; 
	//remove flows till we find one that hasnt expired
	while( !flows.empty() && 
		(time - flows.front().second.time > EXPIRE_SECS)) 
	{
		tmpid = flows.front().second.id;
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
int send_flows(int fd)
{
	Log(LOG_DAEMON|LOG_INFO,"Updating new client with all flows in progress...\n");
	for(flow_lru_t::const_iterator flow_iterator=flows.begin();
			flow_iterator!=flows.end();
			++flow_iterator)
	{
		if(send_update_flow(fd, (*flow_iterator).second.start, 
					(*flow_iterator).second.end, 
					(*flow_iterator).second.id) != 0)
			return 1;
	}
	return 0;
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
int per_packet(struct libtrace_packet_t packet, uint64_t ts, struct modptrs_t *modptrs, RTTMap *map, blacklist *theList )
{

	int hlen = 0;
	struct libtrace_tcp *tcpptr = 0;
	struct libtrace_udp *udpptr = 0;
	uint32_t ts32;
	float start[3];
	float end[3];
	int direction = -1;
	int datasize = -1;
	int force_display = 0;


	flow_id_t tmpid;
	flow_info_t current;

	assert(packet.buffer != NULL);
	assert(packet.size > 0);

	struct libtrace_ip *p = trace_get_ip(&packet);

	if (!p) {
		return 0;
	}
	//ip_t *p = (ip_t *) erfptr->rec.eth.pload;
	ts32 = ts >> 32;
	assert(ts32-lastts >= 0);

	assert(modptrs);
	assert(modptrs->colour);
	// expire old flows - once a second is often enough for now
	if(ts32-lastts > 0)
	{
		expire_flows(ts32);
	}

	lastts = ts32;


	/*
	 * Find the port numbers in the packet
	 */ 
	bool isTCP = false;
	bool isICMP = false;
	if((tcpptr = trace_get_tcp(&packet)))
	{
		isTCP = true;
		hlen = p->ip_hl * 4;

		datasize = (ntohs(p->ip_len)) - (tcpptr->doff*4 + hlen);

		tmpid.sourceport = ntohs(tcpptr->source);
		tmpid.destport = ntohs(tcpptr->dest);
		
		if(showcontrol && (tcpptr->syn || tcpptr->fin || tcpptr->rst))
			force_display = 1;
	}
	else if((udpptr = trace_get_udp(&packet)))
	{
		hlen = p->ip_hl * 4;

		datasize = (ntohs(p->ip_len)) - sizeof(struct libtrace_udp);
		tmpid.sourceport = ntohs(udpptr->source);
		tmpid.destport = ntohs(udpptr->dest);
	}
	else if(p->ip_p == 1)
	{
		isICMP = true;
		tmpid.sourceport = 0;
		tmpid.destport = 0;
	}
	else //for now, set the ports to zero and let the protocol count instead
	{
		tmpid.sourceport = 0;
		tmpid.destport = 0;
	}

	if (datasize > -1 && !force_display) {
		// if we don't show non-data, and datasize is 0, return early
		if ((shownondata == 0) && (datasize == 0))
			return 0;
		// if we don't show data, and datasize is > 0, return early
		if ((showdata == 0) && (datasize > 0))
			return 0;
	
	}

	tmpid.sourceip = p->ip_src;
	tmpid.destip = p->ip_dst;



	if ( flows.find(tmpid) == flows.end() ) // this is a new flow
	{
		flow_info_t flow_info;

		direction = modptrs->direction(&packet);

		// populate start and end arrays
		// also checks that we want traffic from this iface
		if(get_start_pos(start, 
				&packet,
				direction,
				modptrs) != 0)
			return 0;
		if(get_end_pos(end, 
				&packet,
				direction,
				modptrs) != 0)
			return 0;

		flow_info.id = id;
		flow_info.time = ts32; 
		flow_info.start[0] = start[0];
		flow_info.start[1] = start[1];
		flow_info.start[2] = start[2];
		flow_info.end[0] = end[0];
		flow_info.end[1] = end[1];
		flow_info.end[2] = end[2];

		modptrs->colour(&(flow_info.id_num), 
				&packet);
		
		id++;

		flows[tmpid] = flow_info;
		current = flow_info;

		if(send_new_flow(start, end, flow_info.id) != 0)
			return 1;

	}
	else // this is a flow we've already seen
	{
		flows[tmpid].time = ts32; // update time last seen
		current = flows[tmpid];
	}

	// RTT calculation stuff:
	static long int pcount = 0;
	//static long int pcount_nortt = 0;
	pcount++;
	double now = trace_get_seconds( &packet );
	static double last = now;
	PacketTS pts = map->GetTimeStamp( &packet );
	Flow *rtt_flow = new Flow( tmpid.sourceip.s_addr, tmpid.destip.s_addr, 
		tmpid.sourceport, tmpid.destport );
	Flow *rtt_flow_inverse = new Flow( tmpid.destip.s_addr, 
		tmpid.sourceip.s_addr, tmpid.destport, tmpid.sourceport );
	double the_time = 0.0;
	float speed = -2.0f; // Default. (1.0f)

	if( pts.ts > 0 )
	{
	    // Add packet:
	    map->Add( rtt_flow, pts.ts, now );
	}
	else if( isTCP )
	{
	    libtrace_tcp *tcpInfo = trace_get_tcp( &packet );
	    if( tcpInfo != NULL )
	    {
		if( tcpInfo->syn == 1 && tcpInfo->ack == 0 )
		{
		    // SYN:
		    map->Add( rtt_flow, htonl(tcpInfo->seq), now );
		}
		else if( tcpInfo->syn == 1 && tcpInfo->ack == 1 )
		{
		    // SYN/ACK:
		    map->Update( rtt_flow_inverse, htonl(tcpInfo->ack_seq) - 1,
			    htonl(tcpInfo->seq) );

		}
		else if( tcpInfo->syn == 0 && tcpInfo->ack == 1 )
		{
		    // ACK:
		    if( (the_time = map->Retrieve( rtt_flow, 
				    htonl(tcpInfo->ack_seq) - 1 )) >= 0.0f )
		    {
			speed = convert_speed( now-the_time );
		    }
		}
	    }
	}
	else if( isICMP )
	{
	    // Process packet as ICMP:
	    libtrace_icmp *icmpInfo = trace_get_icmp( &packet );
	    if( icmpInfo != NULL )
	    {
		if( icmpInfo->type == 0 )
		{
		    // Echo reply:
		    if( (the_time = map->Retrieve( rtt_flow_inverse, 
				    icmpInfo->un.echo.id ) ) >= 0.0f )
			speed = convert_speed(now-the_time);
		}
		else if( icmpInfo->type == 8 )
		{
		    // Echo:
		    map->Add( rtt_flow, icmpInfo->un.echo.id, now );
		}
	    }
	}


	if( pts.ts_echo > 0 )
	{
	    if((the_time = map->Retrieve(rtt_flow_inverse, pts.ts_echo)) >= 0.0)
	    {	
		// Calculate a sensible speed value 
		// (2.0 = fastest, 1.0 = default, 0.0 = stopped)
		speed = convert_speed( now-the_time );
	    }
	    else if( speed == -2.0f )
	    {
		speed = -1.0f; // -1 tells the client to keep old values
	    }
	}

	delete rtt_flow;
	delete rtt_flow_inverse;

	if( now - last > 60 )
	{
	    int dropped = map->Flush( now );
	    pcount -= dropped;
	    last = now;
	}

	/* check the packet against the addresses in the blacklist */
	if( theList->poke( &packet ) == 1 )
	{
	    /* darknet traffic - going to address with no machine */
	    if(send_new_packet(ts, current.id, current.id_num, 
			ntohs(p->ip_len), speed, true) !=0)
		return 1;
	}
	else
	{
	    /* normal traffic - going to address with a machine */
	    if(send_new_packet(ts, current.id, current.id_num, 
			ntohs(p->ip_len), speed, false) !=0)
		return 1;
	}

	return 0;
}



float convert_speed( float speed )
{
	if( speed <= 0.0005f )
		return( 4.0f );

	if( speed <= 0.005f )
		return( 3.0f );

	if( speed <= 0.05f )
		return( 2.0f );

	if( speed <= 0.25f )
		return( 1.75f );

	if( speed <= 0.5f )
		return( 1.5f );

	if( speed <= 1.0f )
		return( 1.25f );

	if( speed <= 2.5f )
		return( 1.01f );

	if( speed <= 5.0f )
		return( 0.75f );

	return( 0.5f );
}

void kill_all()
{
    send_kill_all();
}
