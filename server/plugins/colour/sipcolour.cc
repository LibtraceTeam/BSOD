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
	UDP = 1,
	REGISTER = 2,
	INVITE = 3,
	ACK = 4,
	BYE = 5,
	CANCEL = 6,
	RES_1XX = 7,
	RES_2XX = 8,
	RES_3XX = 9,
	RES_4XX = 10,
	RES_5XX = 11,
	RES_6XX = 12,
	RES_7XX = 13,
	RES_8XX = 14,
	RES_9XX = 15,
	RES_0XX = 16,
	OTHER = 17,
	LAST = 18 // This must always be the last value in the enum to mark how many items there are!
} counters_t;

// Define colours to use for the various protocols
static uint8_t countercolours[][3] = {
	{100,  5,100}, /* TCP     purple		*/
	{150,100, 50}, /* UDP     light brown	*/
	{100,  5,  5}, /* REGISTER Red */
	{100,100,  5}, /* INVITE */
	{ 50,  5,  5}, /* ACK */
	{ 50,  5, 50}, /* BYE */
	{ 50,  5,100}, /* CANCEL */
	{ 50, 50, 50}, /* 1xx */
	{ 50, 50,100}, /* 2xx */
	{ 50,100, 50}, /* 3xx */
	{ 50,100,100}, /* 4xx */
	{  5,  5,  5}, /* 5xx */
	{  5,  5, 50}, /* 6xx */
	{  5,  5,100}, /* 7xx */
	{  5, 50,  5}, /* 8xx */
	{  5, 50, 50}, /* 9xx */ 
	{  5, 50,100}, /* 0xx */ 
	{255,192,203}  /* OTHER   pink		*/
};

// Define the names displayed on the client when a filter is applied:
char counternames [][256] = {
	"TCP",
	"UDP",
	"REGISTER",
	"INVITE",
	"ACK",
	"BYE",
	"CANCEL",
	"1xx",
	"2xx",
	"3xx",
	"4xx",
	"5xx",
	"6xx",
	"7xx",
	"8xx",
	"9xx",
	"0xx",
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

	void *payload;
	uint8_t proto;
	uint32_t remain;

	assert(f->colour_data == NULL);

	payload = trace_get_transport(packet,&proto,&remain);
	if (!payload) {
		*id_num = OTHER;
		return 0;
	}
	switch(proto) {
		case 6: /* TCP */
			payload = trace_get_payload_from_tcp((libtrace_tcp_t*)payload,&remain);
			*id_num = TCP;
			break;
		case 17: /* UDP */
			payload = trace_get_payload_from_udp((libtrace_udp_t*)payload,&remain);
			*id_num = UDP;
			break;
		default:
			*id_num = OTHER;
			return 0;
	}

	if (remain < 3 || !payload) {
		*id_num = OTHER;
		return 0;
	}

	if (strncmp((char*)payload,"REG",3)==0) *id_num=REGISTER;
	if (strncmp((char*)payload,"INV",3)==0) *id_num=INVITE;
	if (strncmp((char*)payload,"ACK",3)==0) *id_num=ACK;
	if (strncmp((char*)payload,"BYE",3)==0) *id_num=BYE;
	if (strncmp((char*)payload,"CAN",3)==0) *id_num=CANCEL;
	if (*(char*)payload=='1') *id_num=RES_1XX;
	if (*(char*)payload=='2') *id_num=RES_2XX;
	if (*(char*)payload=='3') *id_num=RES_3XX;
	if (*(char*)payload=='4') *id_num=RES_4XX;
	if (*(char*)payload=='5') *id_num=RES_5XX;
	if (*(char*)payload=='6') *id_num=RES_6XX;
	if (*(char*)payload=='7') *id_num=RES_7XX;
	if (*(char*)payload=='8') *id_num=RES_8XX;
	if (*(char*)payload=='9') *id_num=RES_9XX;
	if (*(char*)payload=='0') *id_num=RES_0XX;
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
