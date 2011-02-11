#include "colours.h"
#include <libtrace.h>
#include <libprotoident.h>
#include <libflowmanager.h>
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
	{150,150,240}, /* HTTPS		light blue/purple */
	{200,  5,  5}, /* MAIL		red */
	{200,200,  5}, /* DNS		yellow	*/
	{  5,150,  5}, /* P2P		green */
	{250,120, 80}, /* P2P_UDP	coral */
	{ 80, 80,  5}, /* Windows	olive */
	{ 85, 30, 30}, /* Games		Icky green? */
	{200,100,  5}, /* Malware	orange */
	{ 30, 85, 30}, /* VOIP		matte green */
	{250,250,250}, /* Tunnelling	white */
	{220,160,220}, /* Streaming	plum */
	{ 50, 80, 80}, /* Services	dark slate grey */
	{120,100,240}, /* Files		medium slate blue */
	{110,110,110}, /* Remote	grey */
	{240,230,140}, /* Chat		khaki brown */
	{  5,250,200}, /* ICMP		teal */
	{255,192,203}, /* Other		pink	*/
	{100,  5,100}, /* TCP		purple */
	{150,100, 50}  /* UDP		light brown */


};

typedef struct lpi_col {
	uint8_t seen_dir0;
	uint8_t seen_dir1;
	uint8_t transport;

	lpi_module_t *protocol;
	lpi_data_t lpi;
} lpi_col_t;



static void init_lpi_flow(Flow *f, uint8_t transport) {
	
	lpi_col_t *col = (lpi_col_t *)malloc(sizeof(lpi_col_t));

	lpi_init_data(&col->lpi);
	col->seen_dir0 = 0;
	col->seen_dir1 = 0;
	col->transport = transport;
	col->protocol = NULL;
	f->extension = col;
	
}

static void expire_lpi_flows(double ts, bool exp_flag) {

	Flow *expired;

        /* Loop until libflowmanager has no more expired flows available */
        while ((expired = lfm_expire_next_flow(ts, exp_flag)) != NULL) {
		lpi_col_t *col = (lpi_col_t *)expired->extension;

		free(col);
		delete(expired);
        
	}


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

static void guess_using_port(unsigned char *id_num, libtrace_packet_t *packet)
{
	/* Borrowed most of this from the standard colours module, except
	 * for all the dodgy guessing, e.g. P2P etc */
	struct libtrace_ip *ip = trace_get_ip(packet);
        if (!ip) {
                *id_num = OTHER;
                return;
        }

        int protocol = ip->ip_p;
        int port = trace_get_server_port(
                        protocol,
                        trace_get_source_port(packet),
                        trace_get_destination_port(packet)) == USE_SOURCE
                ? trace_get_source_port(packet)
                : trace_get_destination_port(packet);

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
int mod_get_colour(unsigned char *id_num, libtrace_packet_t *packet) {

	Flow *f;
	lpi_col_t *col = NULL;
 	uint8_t dir;
        bool is_new = false;

        libtrace_tcp_t *tcp = NULL;
        libtrace_ip_t *ip = NULL;
        double ts;

        uint16_t l3_type;

        uint16_t src_port;
        uint16_t dst_port;
	static bool lpi_init_called = false;
	uint32_t plen = 0;

	if (lpi_init_called == false) {
		lpi_init_library();
		lpi_init_called = true;
	}

        /* Libflowmanager only deals with IP traffic, so ignore anything
         * that does not have an IP header */
        ip = (libtrace_ip_t *)trace_get_layer3(packet, &l3_type, NULL);
        if (l3_type != 0x0800 || ip == NULL) {
		*id_num = OTHER;
		return 0;
	}

        /* Expire all suitably idle flows */
        ts = trace_get_seconds(packet);
        expire_lpi_flows(ts, false);

	 /* Many trace formats do not support direction tagging (e.g. PCAP), so
         * using trace_get_direction() is not an ideal approach. The one we
         * use here is not the nicest, but it is pretty consistent and 
         * reliable. Feel free to replace this with something more suitable
         * for your own needs!.
         */

        src_port = trace_get_source_port(packet);
        dst_port = trace_get_destination_port(packet);

        if (src_port == dst_port) {
                if (ip->ip_src.s_addr < ip->ip_dst.s_addr)
                        dir = 0;
                else
                        dir = 1;
        } else {
                if (trace_get_server_port(ip->ip_p, src_port, dst_port) == USE_SOURCE)
                        dir = 0;
                else
                        dir = 1;
        }

        /* Ignore packets where the IP addresses are the same - something is
         * probably screwy and it's REALLY hard to determine direction */
        if (ip->ip_src.s_addr == ip->ip_dst.s_addr) {
		*id_num = OTHER;
                return 0;
	}
	/* Match the packet to a Flow - this will create a new flow if
         * there is no matching flow already in the Flow map and set the
         * is_new flag to true. */
        f = lfm_match_packet_to_flow(packet, dir, &is_new);

        if (f == NULL) {
		/* We've probably missed the handshake - maybe we can get
		 * something useful from the port numbers...? */	
		guess_using_port(id_num, packet);
                return 0;
        }

	tcp = trace_get_tcp(packet);
        /* If the returned flow is new, you will probably want to allocate and
         * initialise any custom data that you intend to track for the flow */
        if (is_new) {
                init_lpi_flow(f, ip->ip_p);
                col = (lpi_col_t *)f->extension;
        } else {
                col = (lpi_col_t *)f->extension;
        }
	/* Pass the packet into libprotoident so that it can extract any
         * info it needs from this packet */
        lpi_update_data(packet, &col->lpi, dir);

        /* Update TCP state for TCP flows. The TCP state determines how long
         * the flow can be idle before being expired by libflowmanager. For
         * instance, flows for which we have only seen a SYN will expire much
         * quicker than a TCP connection that has completed the handshake */
        if (tcp) {
                lfm_check_tcp_flags(f, tcp, dir, ts);
        }

        /* Tell libflowmanager to update the expiry time for this flow */
        lfm_update_flow_expiry_timeout(f, ts);

	plen = trace_get_payload_length(packet);

	guess_protocol(id_num, col, dir, plen);
	
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
