#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
//#include "trace.h"
#include "rtclient.h"
#include "dagnew.h"
#include "dagapi.h"
#include <getopt.h>
#include <string.h>
#include <search.h>
#include <time.h>
#include <math.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <assert.h>

#include "utils.h"
//#include "outputs.h"
//#include "inputs.h"

#include "packets.h"
#include "socket.h"
#include <glib.h>


#define LENGTH 20;
#define SHOW_SRC 1
#define SHOW_DST 1


typedef struct ip ip_t;
uint32_t lastts = 0;
uint32_t count = 0;
int total = 0;//XXX

struct quaduple_t {
    struct in_addr sourceip;
    uint32_t sourceport;
    struct in_addr destip;
    uint32_t destport;
    uint32_t id;
    uint32_t time; 
    char colour[3];
    struct quaduple_t *next;
} *head = NULL;


GHashTable *flow_hash = NULL;
int new_fd;

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

static unsigned char countercolours[][3] = {
    {100,  0,100}, /* TCP     purple		*/
    { 20, 20,255}, /* HTTP    blue		*/
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
    {255,145,  0}, /* WINDOWS orange		*/
    {255,192,203}  /* OTHER   pink		*/
};



guint id_hash(gconstpointer id)
{
    
    const struct quaduple_t *p1 = (struct quaduple_t *)id;
    return (p1->destip.s_addr + p1->sourceip.s_addr 
		+ p1->sourceport + p1->destport) % 2000;

    /*
    const struct quaduple_t *p1 = (struct quaduple_t *)id;

    return p1->id;
    */
}


gboolean id_equal_func(gconstpointer a, gconstpointer b)
{
    
    const struct quaduple_t *p1 = (struct quaduple_t *)a;
    const struct quaduple_t *p2 = (struct quaduple_t *)b;

    if (p1->sourceip.s_addr != p2->sourceip.s_addr)
	return FALSE;	
    if (p1->destport != p2->destport)
	return FALSE;	
    if (p1->destip.s_addr != p2->destip.s_addr)
	return FALSE;	
    if (p1->sourceport != p2->sourceport)
	return FALSE;	

    return TRUE;
/*
    const struct quaduple_t *p1 = (struct quaduple_t *)a;
    const struct quaduple_t *p2 = (struct quaduple_t *)b;

    return (p1->id == p2->id);
    */
}


gboolean expire_flow(gpointer key, gpointer value, gpointer user_data)
{
    const struct quaduple_t *p1 = (struct quaduple_t *)key;
    const uint32_t *time = (uint32_t *) user_data;

    //printf("time = %i\n", *time);
    
    if(*time - p1->time >= 5)
    {
	printf("removing flow %i\n", p1->id);
	send_kill_flow(new_fd, p1->id);
	free(p1);
	return TRUE;
    }
    
    return FALSE;
}

void init_flow_hash(int fd)
{
    assert(flow_hash == NULL);
    flow_hash = g_hash_table_new(id_hash, id_equal_func);

    new_fd = fd;
}






void empty_flows()
{
    struct quaduple_t *tmp;

    count = 0;
    
    while (head!=NULL) {
	tmp=head->next;
	free(head);
	head=tmp;
    }
    head = NULL;
}




static int  is_server_port(int port) {
    if (port <= 0)
	return -1;
    assert(port > 0);
    if (port < 1024 || port == 6667) // hack to get irc
	return port;
    return 0;
}

short get_port(ip_t *ipptr){
    int port1 = 0, port2 = 0;
    int sport1 = 0, sport2 = 0;
    int hlen = 0;
    struct tcphdr *tcpptr = 0;

    hlen = ipptr->ip_hl * 4;

    tcpptr = (struct tcphdr *) ( ((void *)ipptr)  + hlen);

    assert(ipptr);
    port1 = ntohs(tcpptr->source);
    port2 = ntohs(tcpptr->dest);

    sport1 = is_server_port(port1);
    sport2 = is_server_port(port2);

    if(sport1 & sport2) {
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







/*
 * Takes a number with every second bit possibly set, and returns
 * the result of that being compacted together (ie a number half the 
 * number of bits), made to fit within the available space
 */
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
	    sum = sum + pow(2, i);

	number = number >> 2;

    }

    return (((float)sum / 32.7675) / 100) - 10;
}

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

    return ((float)number / 12.8) - 10;

}



/*
 * Sets the colour array (RGB) to be the colour appropriate to the 
 * port/protocol being used.
 */
void get_colour(char color[3], int port, int protocol)
{

    int i;
    color[0] = 0;
    color[1] = 0;
    color[2] = 0;
/*
    if(port == 25 || port == 110 || port == 143 || port == 220)
    {
	for(i=0;i<3;i++)
	    color[i] = countercolours[MAIL][i];
    }
    else
    {
	for(i=0;i<3;i++)
	    color[i] = countercolours[HTTP][i];
    }
    return;
  */  

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

	case 119: for(i=0;i<3;i++)
		      color[i] = countercolours[NNTP][i];
		  break;

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

}


int get_start_pos(float start[3], struct in_addr source, int iface)
{
   if(iface == 0 && SHOW_SRC)
       start[0] = -10;  
   else if(iface == 1 && SHOW_DST)
       start[0] = 10;  
   else
       return 1;

   start[1] = (((float)(source.s_addr & 0xffff))/3276.8) - 10;
   start[2] = (((float)( (source.s_addr & 0xffff0000)>>16  ))/3276.8) - 10;
    
   return 0;

}

int get_end_pos(float end[3], struct in_addr dest, int iface)
{

   if(iface == 0 && SHOW_SRC)
       end[0] = 10;  
   else if(iface == 1 && SHOW_DST)
       end[0] = -10;  
   else
       return 1;

   end[1] = (((float)(dest.s_addr & 0xffff))/3276.8) - 10;
   end[2] = (((float)( (dest.s_addr & 0xffff0000)>>16  ))/3276.8) - 10;
    
   return 0;

}
/*
int get_start_pos(float start[3], struct in_addr source, int iface)
{
    // create packet at starting location
    if(iface == 0 && SHOW_SRC)//local source
    {
	start[1] = compact2( source, 1);
	start[0] = -10;
	start[2] = compact2( source, 0 );

    }
    else if(iface == 1 && SHOW_DST)//foreign source
    {
	start[1] = compact( source, 1);
	start[0] = 10;
	start[2] = compact( source, 0 );
    }
    else
	return -1; // ignore any packets which arent on iface 0 or 1

    return 0;
}
*/
/*
int get_end_pos(float end[3], struct in_addr dest, int iface)
{
    // create packet at starting location
    if(iface == 0 && SHOW_SRC)//local source
    {
	end[1] = compact( dest , 1 );
	end[0] = 10;
	end[2] = compact( dest , 0 );

	//printf("%f %f\n", end[1], end[2]);

    }
    else if(iface == 1 && SHOW_DST)//foreign source
    {
	end[1] = compact2( dest, 1 );
	end[0] = -10;
	end[2] = compact2( dest , 0 );
    }
    else
	return -1; // ignore any packets which arent on iface 0 or 1

    return 0;
}
*/





//-----------------------------------------------------------

int per_packet(const dag_record_t *erfptr, uint32_t caplen, uint64_t ts/*, int new_fd*/)
{
    ip_t *p = (ip_t *) erfptr->rec.eth.pload;
    int sport = 0, dport = 0;
    struct in_addr sourceip, destip;
    int hlen = 0;
    struct tcphdr *tcpptr = 0;
    struct quaduple_t *tmp, *saved;
    //struct quaduple_t *prev = NULL;
    //uint32_t tmpid;
    uint32_t ts32;
    uint32_t *tsptr;

    ts32 = ts >> 32;
    tsptr = &ts32;

    // hardocded hacks
    //uint16_t size = 10; ////

    assert(erfptr != NULL);
    assert(caplen > 0);
    assert(ts32-lastts >= 0);

    
    // check for expired flows every second

    if(ts32 - lastts > 0)
	g_hash_table_foreach_remove (flow_hash, expire_flow, tsptr);
    
    

    hlen = p->ip_hl * 4;
    tcpptr = (struct tcphdr *) ( ((void *)p)  + hlen);

    sport = ntohs(tcpptr->source);
    dport = ntohs(tcpptr->dest);

    sourceip = p->ip_src;
    destip = p->ip_dst;

    // normalise to make lower ip first
    if (destip.s_addr<sourceip.s_addr || 
	    (destip.s_addr==sourceip.s_addr && dport<sport))
    {
	struct in_addr tmpip;
	uint32_t tmpport;
	tmpip=destip;
	destip=sourceip;
	sourceip=tmpip;

	tmpport=dport;
	dport=sport;
	sport=tmpport;
    }

    // look for a match
    /*
    for(tmp=head;tmp;tmp=tmp->next) {
	if (tmp->sourceip.s_addr != sourceip.s_addr)
	    continue;
	if (tmp->destport != dport)
	    continue;
	if (tmp->destip.s_addr != destip.s_addr)
	    continue;
	if (tmp->sourceport != sport)
	    continue;
	//printf("found packet id = %i\n", tmp->id);
	break;
    }
*/

    
    //if (tmp == NULL) {
	float start[3];
	float end[3];

	// populate start and end arrays
	// also checks that we want this iface
	if(get_start_pos(start, sourceip, erfptr->flags.iface) != 0)
	{
	    //printf("start pos error------------------\n");
	    return 0;
	}
	if(get_end_pos(end, destip, erfptr->flags.iface) != 0)
	{
	    //printf("end pos error------------------\n");
	    return 0;
	}

	// We didn't find it 
	tmp = malloc(sizeof(struct quaduple_t));
	tmp->sourceip=sourceip;
	tmp->destip=destip;
	tmp->sourceport=sport;
	tmp->destport=dport;
	tmp->next=head;
	//tmp->id = count;
	tmp->time = ts32;

	get_colour(tmp->colour, get_port(p), p->ip_p);

	//head=tmp;
	saved = g_hash_table_lookup (flow_hash, tmp);

	if(saved == NULL)
	{
	    printf("CREATING FLOW %i\n", count);
	    tmp->id = count;
	    g_hash_table_insert ( flow_hash, tmp, tmp);
	    
	    if(send_new_flow(new_fd, start, end, count) != 0)
		return 1;
	    count++;
	    total++;
	    //printf("current flows = %i\n", total);
	    saved = tmp;//just for now
	}
    //}
    //tmp->time = ts32;// update with time we saw this packet
    //printf("packet: %i\n", saved->id);
    if(send_new_packet(new_fd, ts, saved->id, saved->colour, caplen) !=0)
	return 1;

    //blah = &ts32;
    //expire_flow(saved, saved, blah);
    
    return 0;
}
