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

/*
* Sets the colour array (RGB) to be the colour appropriate to the 
* port/protocol being used.
*/
extern "C"
int mod_get_type(unsigned char *id_num, struct libtrace_packet_t *packet)
{

	struct libtrace_ip *ip = trace_get_ip(packet);
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
	
	return 0;

}
