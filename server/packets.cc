#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <vector>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <syslog.h>

#include "lru"


#include "libtrace.h"
#include "dagformat.h"


#include "packets.h"
#include "debug.h"
#include "socket.h" 
#include "bsod_server.h"



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
	uint8_t colour[3];
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
 * A little bit hax - a flow can only be expired after all its packets
 * have left the clients display. Currently a packet takes 4.5 seconds
 * to travel the clients display space, and the server is removing them
 * after 5 seconds of inactivity.
 */
void expire_flows(uint32_t time)
{
	uint32_t tmpid; 
	//remove flows till we find one that hasnt expired
	while( !flows.empty() && (time - flows.front().second.time > 4)) 
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
	flow_lru_t::const_iterator flow_iterator;

	Log(LOG_DAEMON|LOG_INFO,"Updating new client with all flows in progress...\n");
	for(flow_iterator =flows.begin();flow_iterator!=flows.end();flow_iterator++)
	{
		if(send_update_flow(fd, (*flow_iterator).second.start, 
					(*flow_iterator).second.end, 
					(*flow_iterator).second.id) != 0)
			return 1;
	}
	return 0;
}

//------------------------------------------------------------
short get_port(uint8_t protocol, uint16_t source, uint16_t dest)
{
    if(trace_get_server_port(protocol, source, dest) == USE_DEST)
	return dest;

    return source;
}
	
//-------------------------------------------------------------
int get_start_pos(float start[3], struct in_addr source, int iface, struct modptrs_t *modptrs)
{
	if(iface == 0) {
		start[0] = -10;
		modptrs->left(start, source);
	} else if(iface == 1) {
		start[0] = 10;
		modptrs->right(start,source);
	} else
		return 1;

	return 0;
}
//------------------------------------------------------------
int get_end_pos(float end[3], struct in_addr dest, int iface, struct modptrs_t *modptrs)
{
	if(iface == 0) {
		end[0] = 10;
		modptrs->right(end,dest);
	} else if(iface == 1) {
		end[0] = -10;
		modptrs->left(end,dest);
	} else
		return 1;

	return 0;
}

//------------------------------------------------------------
int per_packet(struct libtrace_packet_t packet, uint64_t ts, struct modptrs_t *modptrs)
{

	int hlen = 0;
	struct tcphdr *tcpptr = 0;
	struct udphdr *udpptr = 0;
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
	if(p->ip_p == 6)
	{
		hlen = p->ip_hl * 4;
		tcpptr = (struct tcphdr *) ((uint8_t *)p  + hlen);
		assert(tcpptr);

		datasize = (ntohs(p->ip_len)) - (tcpptr->doff*4 + hlen);

		tmpid.sourceport = ntohs(tcpptr->source);
		tmpid.destport = ntohs(tcpptr->dest);
		
		if(showcontrol && (tcpptr->syn || tcpptr->fin || tcpptr->rst))
			force_display = 1;
	}
	else if(p->ip_p == 17)
	{
		hlen = p->ip_hl * 4;
		udpptr = (struct udphdr *) ((uint8_t *)p  + hlen);
		assert(udpptr);

		datasize = (ntohs(p->ip_len)) - sizeof(struct libtrace_udp);
		tmpid.sourceport = ntohs(udpptr->source);
		tmpid.destport = ntohs(udpptr->dest);
	}
	else if(p->ip_p == 1)
	{
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

		direction = modptrs->direction(packet);

		// populate start and end arrays
		// also checks that we want traffic from this iface
		if(get_start_pos(start, 
				tmpid.sourceip, 
				direction,
				/*erfptr->flags.iface,*/
				modptrs) != 0)
			return 0;
		if(get_end_pos(end, 
				tmpid.destip, 
				direction,
				/*erfptr->flags.iface,*/
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

		modptrs->colour(flow_info.colour, 
				get_port(p->ip_p, tmpid.sourceport, 
				    tmpid.destport), 
				p->ip_p);
		
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
	if(send_new_packet(ts, current.id, current.colour, ntohs(p->ip_len)) !=0)
		return 1;


	return 0;
}
