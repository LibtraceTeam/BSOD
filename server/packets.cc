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
#include "lru"


extern "C" {
#include "utils.h"
#include "rtclient.h"
#include "dagnew.h"
#include "dagapi.h"
#include "packets.h"
#include "socket.h" // please explain why this works having it in twice
}

#include "socket.h" // but breaks in a different place if i leave one out

#define SIDE_LENGTH 8
#define MAX_SIZE 10000

typedef struct ip ip_t;
uint32_t lastts = 0;
uint32_t id = 0;

typedef enum counters {
    TCP = 0,
    HTTP = 1,
    HTTPS = 2,
    MAIL = 3,
    FTP = 4,
    NNTP = 5,
    DNS = 6,
    NTP = 7,
    SSH = 8,
    UDP = 9,
    ICMP = 10,
    IRC = 11,
    WINDOWS = 12,
    OTHER = 13
} counters_t;

static uint8_t countercolours[][3] = {
    {100,  0,100}, /* TCP     purple		*/
    {  0,  0,200}, /* HTTP    blue		*/
    {150,150,240}, /* HTTPS   light blue/purple	*/
    {200,  0,  0}, /* MAIL    red		*/
    {  0,200,  0}, /* FTP     green		*/
    {250,250,250}, /* NNTP    white		*/
    {200,200,  0}, /* DNS     yellow		*/
    { 30, 85, 30}, /* NTP     matte green	*/
    {110,110,110}, /* SSH     grey		*/
    {150,100, 50}, /* UDP     light brown	*/
    {  0,250,200}, /* ICMP    teal		*/
    {240,230,140}, /* IRC     khaki brown	*/
    {200,100,  0}, /* WINDOWS orange		*/
    {255,192,203}  /* OTHER   pink		*/
};

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

/* This iterator was causing problems when using the '->' operator 
 * rather than (*flow_iterator).
 */
int send_flows(int fd)
{
    flow_lru_t::const_iterator flow_iterator;

    printf("Updating new client with all flows in progress...\n");
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
float compact(struct in_addr ip, int offset)
{
    unsigned i;
    unsigned sum = 0;
    uint32_t number = ntohl(ip.s_addr);

    if(offset)
    {
	number = number & 0xaaaaaaaa;
	number = number >> 1;
    }
    else
	number = number & 0x55555555;

    for(i=0; i<16; i++)
    {
	if(number & 0x1)
	    sum = sum + (int)(pow(2, i));

	number = number >> 2;

    }

    // hard coded range -10 -> +10
    return (((float)sum / 32.7675) / 100) - 10;
}
//---------------------------------------------------------------

/**
 * SPlits addresses within the uni up. The first half is the same in all cases
 * so it only uses the first half. One dimension is based on the third
 * quarter, the other is based on the last quarter.
 */
float compact2(struct in_addr ip, int offset)
{
    uint32_t number = ntohl(ip.s_addr);

    if(offset)
	number = ((number & 0x000ff00) >> 8) ;
    else
	number = number & 0x000000ff;

    // hard coded range -10 -> +10
    return ((float)number / 12.8) - 10;
}

//-------------------------------------------------------------
int get_start_pos(float start[3], struct in_addr source, int iface)
{
    float length;
    float angle;

    
    if(iface == 0)
    {
	    start[1] = compact2( source, 1);
	    start[0] = -10;
	    start[2] = compact2( source, 0 );
    }
    else if(iface == 1)
    {
	source.s_addr = ntohl(source.s_addr);
	angle = ((source.s_addr & 0xffff0000) >> 16) % MAX_SIZE;
	length = (source.s_addr & 0xffff) % MAX_SIZE;

	start[0] = 10;
	start[1] = angle/MAX_SIZE * sin( (2* M_PI * length) / MAX_SIZE) 
	    * SIDE_LENGTH;
	start[2] = angle/MAX_SIZE * cos( (2* M_PI * length) / MAX_SIZE) 
	    * SIDE_LENGTH;


    }
    else
	return 1;

    return 0;
}
//------------------------------------------------------------
int get_end_pos(float end[3], struct in_addr dest, int iface)
{
    float length;
    float angle;
    
    if(iface == 0)
    {
	dest.s_addr = ntohl(dest.s_addr);
	angle = ((dest.s_addr & 0xffff0000) >> 16) % MAX_SIZE;
	length = (dest.s_addr & 0xffff) % MAX_SIZE;

	end[0] = 10;
	end[1] = angle/MAX_SIZE * sin( (2* M_PI * length) / MAX_SIZE) 
	    * SIDE_LENGTH;
	end[2] = angle/MAX_SIZE * cos( (2* M_PI * length) / MAX_SIZE) 
	    * SIDE_LENGTH;
    }
    else if(iface == 1)
    {
	    end[1] = compact2( dest, 1);
	    end[0] = -10;
	    end[2] = compact2( dest, 0 );
    }
    else
	return 1;

    return 0;

}
//-----------------------------------------------------------

static int  is_server_port(int port) {
    if (port <= 0)
	return -1;
    assert(port > 0);
    if (port < 1024 || port == 6667) // hack to get irc
	return port;
    return 0;
}

//------------------------------------------------------------
short get_port(int port1, int port2)
{
    int sport1 = 0, sport2 = 0;

    sport1 = is_server_port(port1);
    sport2 = is_server_port(port2);

    if(sport1 & sport2) {
	if(sport1 == 6667)
	    return sport1;
	if(sport2 == 6667)
	    return sport2;

	if (sport1 < sport2)
	    return port1;
	else
	    return port2;
    }
    if(sport1)
	return port1;
    if(sport2)
	return port2;
    return 0;
}
//------------------------------------------------------------

/*
 * Sets the colour array (RGB) to be the colour appropriate to the 
 * port/protocol being used.
 */
void get_colour(uint8_t color[3], int port, int protocol)
{

    int i;
    
    switch(port)
    {
	case 80: for(i=0;i<3;i++)
		     color[i] = countercolours[HTTP][i];
		 break;

	case 21: 
	case 20: for(i=0;i<3;i++)
		     color[i] = countercolours[FTP][i];
		 break;

	case 110: /* pop3 */
	case 143: /* imap2 */
	case 220: /* imap3 */
	case 993: /* imap over ssl */
	case 995: /* pop3 over ssl */
	case 465: /* smtp over ssl */
	case 25: for(i=0;i<3;i++)
		     color[i] = countercolours[MAIL][i];
		 break;

		 /* white is just too confusing when other things blend */
/*
	case 119: for(i=0;i<3;i++)
		      color[i] = countercolours[NNTP][i];
		  break;
*/
	case 53: for(i=0;i<3;i++)
		     color[i] = countercolours[DNS][i];
		 break;

	case 22:
	case 23: for(i=0;i<3;i++)
		     color[i] = countercolours[SSH][i];
		 break;
		 
	case 443: for(i=0;i<3;i++)
		      color[i] = countercolours[HTTPS][i];
		  break;

	case 6667: for(i=0;i<3;i++)
		       color[i] = countercolours[IRC][i];
		   break;
	case 123: for(i=0;i<3;i++)
		      color[i] = countercolours[NTP][i];
		  break;
	case 135:
	case 136:
	case 137:
	case 138:
	case 139:
	case 445: for(i=0;i<3;i++)
		      color[i] = countercolours[WINDOWS][i];
		  break;

      // if not a port that I'm counting give a colour based on protocol
	default:  
		  switch(protocol)
		  {
		      case 6: for(i=0;i<3;i++)
				  color[i] = countercolours[TCP][i];	
			      break;

		      case 17: for(i=0;i<3;i++)
				   color[i] = countercolours[UDP][i];
			       break;

		      case 1: for(i=0;i<3;i++)
				  color[i] = countercolours[ICMP][i]; 
			      break;
			      /* 
				 case 41: for(i=0;i<3;i++)
				 color[i] = countercolours[IPMP][i]; 
				 break;
				 */
		      default: for(i=0;i<3;i++)
				   color[i] = countercolours[OTHER][i]; 
			       break;
		  };break;
    };
	//printf("%u %u %u \n", color[0], color[1], color[2]);

}





//------------------------------------------------------------
int per_packet(const dag_record_t *erfptr, uint32_t caplen, uint64_t ts)
{

    int hlen = 0;
    struct tcphdr *tcpptr = 0;
    struct udphdr *udpptr = 0;
    uint32_t ts32;
    float start[3];
    float end[3];

    flow_id_t tmpid;
    flow_info_t current;

    assert(erfptr != NULL);
    assert(caplen > 0);
    
    ip_t *p = (ip_t *) erfptr->rec.eth.pload;
    ts32 = ts >> 32;
    assert(ts32-lastts >= 0);

    // expire old flows - once a second is often enough for now
    if(ts32-lastts > 0)
    {
	printf("bling: %i\n", ts32);
	expire_flows(ts32);
    }

    lastts = ts32;

    /*
     * Ports are in the same place in the udp and tcp headers, but they
     * should probably be seperated in case something changes, and to 
     * allow new protocols to be included easy
     */
    if(p->ip_p == 6)
    {
	hlen = p->ip_hl * 4;
	tcpptr = (struct tcphdr *) ((uint8_t *)p  + hlen);
	assert(tcpptr);

	/* bail out if this packet has no data */
	if((ntohs(p->ip_len)) - (tcpptr->doff*4 + hlen) == 0)
	    return 0; 
	
	tmpid.sourceport = ntohs(tcpptr->source);
	tmpid.destport = ntohs(tcpptr->dest);
    }
    else if(p->ip_p == 17)
    {
	hlen = p->ip_hl * 4;
	udpptr = (struct udphdr *) ((uint8_t *)p  + hlen);
	assert(udpptr);

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

    tmpid.sourceip = p->ip_src;
    tmpid.destip = p->ip_dst;


    if ( flows.find(tmpid) == flows.end() ) // this is a new flow
    {
	flow_info_t flow_info;
	
	// populate start and end arrays
	// also checks that we want traffic from this iface
	if(get_start_pos(start, tmpid.sourceip, erfptr->flags.iface) != 0)
	    return 0;
	if(get_end_pos(end, tmpid.destip, erfptr->flags.iface) != 0)
	    return 0;

	flow_info.id = id;
	flow_info.time = ts32; 
	flow_info.start[0] = start[0];
	flow_info.start[1] = start[1];
	flow_info.start[2] = start[2];
	flow_info.end[0] = end[0];
	flow_info.end[1] = end[1];
	flow_info.end[2] = end[2];

	get_colour(flow_info.colour, 
		get_port(tmpid.sourceport, tmpid.destport), p->ip_p);
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
