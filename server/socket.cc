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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>

#include <libwandevent.h>

#include "socket.h"
#include "packets.h"
#include "debug.h"
#include <syslog.h>
#include "bsod_server.h"
#include <list>
#include <map>

#define BSOD_PROTO_VERSION 0x14

float htonf(float x) { 
	union {
		float f;
		uint32_t i;
	} _u;
	_u.f = (x);
	_u.i=htonl(_u.i); 
	return _u.f;
}

extern int fd_max;
int max_sendq_size = 10*1024*1024;

struct client_buffer
{
	void *data;
	size_t datalen;
	size_t offset;
};

/* list of file descriptors for all connected clients */
struct client {
	int fd;
	struct client *next;
	struct client *prev;
	wand_event_handler_t *ev_hdl;
	struct wand_fdcb_t *writer;
	
	struct client_buffer buffer2;
	
	//std::list< client_buffer > buffer;
} *clients = NULL;

/* structure for flow update packets */
struct flow_update_t {
	unsigned char type;
	float x1; 
	float y1; 
	float z1; 
	float x2;
	float y2;
	float z2; 
	uint32_t count;
	uint32_t ip1;
	uint32_t ip2;
} __attribute__((packed));

/* structure for new packet packets */
struct pack_update_t {
	unsigned char type;
	uint32_t ts;
	uint32_t id; // Flow id
	unsigned char id_num; // packet type id
	uint16_t size;
	float speed; // This affects the speed of the entire flow based on RTT
	uint8_t dark;
} __attribute__((packed));

/* structure for expire flow packets */
struct flow_remove_t {
	unsigned char type;
	uint32_t id;
} __attribute__((packed));

// Kill all
struct kill_all_t {
    unsigned char type;
} __attribute__((packed));

// Colour table
struct flow_descriptor_t {
	unsigned char type;
	unsigned char id;
	uint8_t colour[3];
	char name[256];
} __attribute__((packed));

// Image data header
struct image_data_t {
	unsigned char type;
	unsigned char id; // 0 = left, 1 = right
	uint32_t length; //the number of bytes following this packet of image data
} __attribute__((packed));

struct listen_data {
	wand_event_handler_t *ev_hdl;
	struct modptrs_t *modptrs;
	bool wait_for_client;
};

struct listen_data ldata;

/* For discovery replies */
static uint16_t server_port = 0;

extern char *server_name;

/* For left and right images */
extern char *left_image, *right_image;

void client_cb(wand_event_handler_t *ev_hdl, 
		int fd, void *data, enum wand_eventtype_t ev) ;

bool no_clients(void) {

	return (clients == NULL);

}

/* Creates a new structure containing a file descriptor */
struct client* create_fd(int fd, wand_event_handler_t *ev_hdl)
{
	client *tmp = new client;
	tmp->fd = fd;
	tmp->next = NULL;
	tmp->prev = NULL;
	tmp->ev_hdl = ev_hdl;

	tmp->buffer2.data = NULL;
	tmp->buffer2.datalen = 0;
	tmp->buffer2.offset = 0;

	tmp->writer = wand_add_fd(ev_hdl, fd, EV_READ, tmp, client_cb);

	return tmp;
}

/* Adds a structure containing a file descriptor to the front of the list */
struct client *add_fd(int fd, wand_event_handler_t *ev_hdl)
{
	struct client *tmp = create_fd(fd, ev_hdl);

	if(clients == NULL)
		clients = tmp;
	else
	{
		tmp->next = clients;
		clients->prev = tmp;
		clients = tmp;
	}

	assert(tmp->fd>=0);
	return tmp;
}

/* Removes a given structure from the list of file descriptors */
void remove_fd(struct client *tmp)
{
	Log(LOG_DAEMON|LOG_INFO,"Removing client on fd %i\n", tmp->fd);

	wand_del_fd(tmp->ev_hdl, tmp->fd);
	
	close(tmp->fd);

	if(tmp->next == NULL && tmp->prev == NULL) // only item
	{
		clients = NULL;
	}
	else if(tmp->next != NULL && tmp->prev == NULL) // first item
	{
		tmp->next->prev = NULL;
		clients = tmp->next;
	}
	else if(tmp->next == NULL && tmp->prev != NULL) // last item
	{
		tmp->prev->next = NULL;
	}
	else if(tmp->next != NULL && tmp->prev != NULL) // middle item
	{
		tmp->next->prev = tmp->prev;
		tmp->prev->next = tmp->next;
	}

	if (tmp->buffer2.data) {
		free(tmp->buffer2.data);
	}

/*
	while (!tmp->buffer.empty()) {
		free(tmp->buffer.front().data);
		tmp->buffer.pop_front();
	}
*/

	delete tmp;
}

static void listen_cb(wand_event_handler_t *ev_hdl, int fd, void *data, 
		enum wand_eventtype_t ev) {

	char protocol_version = BSOD_PROTO_VERSION;
	struct sockaddr_in remoteaddr;
	socklen_t sock_size;
	int newfd;
	struct client * ret;
	static int first_client = 1;

	
	// handle new connections
	sock_size = sizeof(struct sockaddr_in);
	if ((newfd = accept(fd, (struct sockaddr *)&remoteaddr,
					&sock_size)) == -1) { 
		perror("accept");
	} 
	else 
	{
		fcntl(newfd, F_SETFL, O_NONBLOCK);
		if (write(newfd,&protocol_version,1) == -1) {
			Log(LOG_DAEMON | LOG_DEBUG, "Error writing protocol version: %s\n", strerror(errno));
			return;
		}

		ret=add_fd(newfd, ev_hdl);
		Log(LOG_DAEMON|LOG_DEBUG,
				"server: new connection from %s\n", 
				inet_ntoa(remoteaddr.sin_addr));
		// Update all clients with the colour table
		// This could be done better by targeting only the new
		// client.
		send_colour_table(ldata.modptrs);	

		// Send the new client the left and right images
		send_images(ret);

		send_flows(ret);
	}

	/* Force the initial event loop (which will be listening only) to
	 * stop and start reading the input trace */
	if (first_client && ldata.wait_for_client) {
		ev_hdl->running = false;
		first_client = 0;
	}

}

//----------------------------------------------------------
struct wand_fdcb_t * setup_listen_socket(wand_event_handler_t *ev_hdl, 
		struct modptrs_t *modptrs, uint16_t port,
		bool wait_flag)
{
	int yes=1;        // for setsockopt() SO_REUSEADDR, below
	int listen_socket = -1;
	struct sockaddr_in myaddr;     // server address

	// get the listener
	if ((listen_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	// stop the "address already in use" error message
	if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
		perror("setsockopt");
		exit(1);
	}


	assert(port > 0);
	assert(listen_socket >= 0);// starts at 0? 

	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = INADDR_ANY;
	myaddr.sin_port = htons(port);
	memset(&(myaddr.sin_zero), '\0', 8);
	if (bind(listen_socket, (struct sockaddr *)&myaddr, 
				sizeof(myaddr)) == -1) {
		perror("bind");
		exit(1);
	}

	// listen
	if (listen(listen_socket, 10) == -1) {
		perror("listen");
		exit(1);
	}
	

	server_port = port;

	ldata.ev_hdl = ev_hdl;
	ldata.modptrs = modptrs;
	ldata.wait_for_client = wait_flag;

	return wand_add_fd(ev_hdl, listen_socket, EV_READ, NULL, listen_cb);
}

static void udp_cb(wand_event_handler_t *ev_hdl, int fd, void *data, 
		enum wand_eventtype_t ev) {
	struct sockaddr_in sendaddr;
	int addr_len = sizeof(sendaddr);
	int numbytes;
	unsigned char buf[16];
	char response[256];
	
	sendaddr.sin_family = AF_INET;
	sendaddr.sin_port = htons(UDP_PORT);
	sendaddr.sin_addr.s_addr = INADDR_ANY;
	memset(sendaddr.sin_zero,'\0', sizeof sendaddr.sin_zero);

	if ((numbytes = recvfrom(fd, buf, sizeof(buf), 0,
					(struct sockaddr *)&sendaddr, (socklen_t *)&addr_len)) == -1){
		perror("recvfrom");
	}
	buf[numbytes] = 0;

	Log(LOG_DAEMON|LOG_DEBUG,"UDP discovery from %s\n", 
			inet_ntoa(sendaddr.sin_addr));

	//Send them a response 
	sprintf(response, "%s|%d|%s", "0.0.0.0", server_port, server_name);

	numbytes = sendto(fd, response, strlen(response) , 0, 
			(struct sockaddr *)&sendaddr, 
			sizeof(sendaddr));

	Log(LOG_DAEMON|LOG_DEBUG,"Sent a reply: '%s'\n", response);



}

//----------------------------------------------------------
struct wand_fdcb_t *setup_udp_socket(wand_event_handler_t *ev_hdl){

	int addr_len;
    	int broadcast=1;
	int udp_socket;

	if((udp_socket = socket(PF_INET, SOCK_DGRAM, 0)) == -1){
		perror("socket");
		exit(1);
	}
	if((setsockopt(udp_socket,SOL_SOCKET,SO_BROADCAST,
					&broadcast,sizeof(broadcast))) == -1){
		perror("setsockopt - SO_SOCKET ");
		exit(1);
	}
	
	struct sockaddr_in sendaddr;
	struct sockaddr_in recvaddr;
							
	//Start at a base of UDP_PORT, increment until we find an open one
	//This means we can run multiple servers on the same box without clashes 
	for(int port = UDP_PORT;;port++){

		sendaddr.sin_family = AF_INET;
		sendaddr.sin_port = htons(port);
		sendaddr.sin_addr.s_addr = INADDR_ANY;
		memset(sendaddr.sin_zero,'\0', sizeof(sendaddr.sin_zero));

		recvaddr.sin_family = AF_INET;
		recvaddr.sin_port = htons(port);
		recvaddr.sin_addr.s_addr = INADDR_ANY;
		memset(recvaddr.sin_zero,'\0',sizeof(recvaddr.sin_zero));
		
		if(bind(udp_socket, (struct sockaddr*) &recvaddr, sizeof(recvaddr)) != -1){
			printf("Bound to UDP multicast on port %d\n", port);
			break;
		}
		
		printf("UDP port %d is in use\n", port);		
	}

	return wand_add_fd(ev_hdl, udp_socket, EV_READ, NULL, udp_cb);
	
}

void client_cb(wand_event_handler_t *ev_hdl, int fd, void *data, 
		enum wand_eventtype_t ev) {

	struct client *client = (struct client *)(data);
	char *sendfrom = ((char *)client->buffer2.data) + client->buffer2.offset;

	if (ev == EV_READ) {

		Log(LOG_DAEMON | LOG_DEBUG, "Detected EOF in client callback\n");
		remove_fd(client);
		return;
	}

	while (client->buffer2.datalen - client->buffer2.offset > 0) {
		sendfrom = ((char *)client->buffer2.data) + client->buffer2.offset;
		int ret = send(client->fd,
				sendfrom,
				client->buffer2.datalen - client->buffer2.offset,
				0);
	/*
	while (!client->buffer.empty()) {
		int ret=send(client->fd,
				(char*)client->buffer.front().data+client->buffer.front().offset,
				client->buffer.front().datalen-client->buffer.front().offset,
				0);
	*/
		if (ret == -1) {
			switch (errno) {
				case EINTR: continue;
				case ENOBUFS:
				case ENOMEM:
				case EAGAIN: return;
				default:
					perror("send");
					remove_fd(client);
					return;
			}
		}


		// If we successfully wrote this data, remove it from the queue
		
		client->buffer2.offset += ret;
		assert(client->buffer2.offset <= client->buffer2.datalen);
		/*
		if (ret == (int)client->buffer.front().datalen - (int)client->buffer.front().offset) {
			free(client->buffer.front().data);
			client->buffer.pop_front();
		}
		else {
			client->buffer.front().offset+=ret;
			return;
		}
		*/
	}

	/* Nothing more to write */
	wand_set_fd_flags(ev_hdl, fd, EV_READ);

}


/* Enqueue data onto a clients sendq
 *
 * If this overflows the clients queue, disconnect the client.
 */
void enqueue_data(struct client *client,void *buffer, size_t size)
{

	if (client->buffer2.data == NULL) {
		client->buffer2.data = malloc(max_sendq_size);
		if (client->buffer2.data == NULL) {
			Log(LOG_DAEMON|LOG_ALERT,"Disconnecting %i: Out of memory\n",client->fd);
			remove_fd(client);
			return;
		}

		client->buffer2.datalen = 0;
		client->buffer2.offset = 0;
	}

	if (client->buffer2.datalen - client->buffer2.offset + size > 
			max_sendq_size) {
		
		/* Send queue is getting full - force a send */
		Log(LOG_DAEMON | LOG_DEBUG,"Forced send to client %i\n", 
				client->fd);
		client_cb(client->ev_hdl, client->fd, client, EV_WRITE);
	}

	/* If send queue is still too full, we'll have to drop the client */

	if (client->buffer2.datalen - client->buffer2.offset + size > 
			max_sendq_size) {
		
		Log(LOG_DAEMON|LOG_ALERT,"Disconnecting %i for max sendq exceeded\n",client->fd);
		remove_fd(client);
		return;
	}

	/* Check if we need to realloc the queue */
	if (client->buffer2.datalen + size >  max_sendq_size) {
		memmove(client->buffer2.data, (char *)client->buffer2.data + client->buffer2.offset, client->buffer2.datalen - client->buffer2.offset);
		client->buffer2.datalen -= client->buffer2.offset;
		client->buffer2.offset = 0;

	}

	memcpy((char *)client->buffer2.data + client->buffer2.datalen, buffer, size);
	client->buffer2.datalen += size;
	assert(client->buffer2.datalen <= max_sendq_size);		

/*
	struct client_buffer sendq;
	assert(buffer);
	client->data_waiting+=size;
	if (client->data_waiting>max_sendq_size) { // 10MB
		Log(LOG_DAEMON|LOG_ALERT,"Disconnecting %i for max sendq exceeded\n",client->fd);
		remove_fd(client);
		return;
	}
	sendq.datalen = size;
	sendq.data = malloc(sendq.datalen);
	if (sendq.data == NULL) {
		Log(LOG_DAEMON|LOG_ALERT,"Disconnecting %i: Out of memory\n",client->fd);
		remove_fd(client);
		return;
	}
	memcpy(sendq.data,buffer,sendq.datalen);
	sendq.offset = 0;
	client->buffer.push_back(sendq);
*/

	wand_set_fd_flags(client->ev_hdl, client->fd, EV_WRITE | EV_READ);

}

/*
 * Takes a packet as part of a union, works out which one it is and
 * sends it to all clients.
 */
int send_all(void *data, int size)
{
	struct client *tmp;
	struct client *next;

	assert(data);

	// send to all clients 
	for(tmp=clients;tmp != NULL; tmp = next)
	{
		next=tmp->next;
		enqueue_data(tmp, data, size);
	}

	return 0;
}

//-------------------------------------
int send_kill_flow(uint32_t id)
{
	struct flow_remove_t update;
	update.type = 0x02;
	update.id = htonl(id);

	send_all(&update,sizeof(update));

	return 0;
}

//------------------------------------------------------------------
int send_new_flow(float start[3], float end[3], uint32_t id, uint32_t ip1, uint32_t ip2 )
{
	struct flow_update_t update;
	update.type = 0x00;
	update.x1 = htonf(start[0]);
	update.y1 = htonf(start[1]);
	update.z1 = htonf(start[2]);
	update.x2 = htonf(end[0]);
	update.y2 = htonf(end[1]);
	update.z2 = htonf(end[2]);
	update.count = htonl(id);
	update.ip1 = htonl(ip1);
	update.ip2 = htonl(ip2);

	send_all(&update,sizeof(update));

	return 0;
}
//-------------------------------------------------------------------

/* Send flow information to a single client.  This differs from
 * send_new_flow in that it doesn't send to all clients 
 */
int send_update_flow(struct client *client, 
		float start[3], float end[3], uint32_t id, uint32_t ip1, uint32_t ip2 )
{

	struct flow_update_t update;
	update.type = 0x00;
	update.x1 = htonf(start[0]);
	update.y1 = htonf(start[1]);
	update.z1 = htonf(start[2]);
	update.x2 = htonf(end[0]);
	update.y2 = htonf(end[1]);
	update.z2 = htonf(end[2]);
	update.count = htonl(id);
	update.ip1 = htonl(ip1);
	update.ip2 = htonl(ip2);

	enqueue_data(client, &update, sizeof(update));

	return 0;
}

//-------------------------------------------------------------------
int send_new_packet(uint32_t ts, uint32_t id, unsigned char id_num,
		uint16_t size, float speed, bool dark)
{
	struct pack_update_t update;
	update.type = 0x01;
	update.ts = htonl(ts);
	update.id = htonl(id);
	update.id_num = id_num;
	update.size = htons(size);
	update.speed = htonf(speed);
	if (dark)
		update.dark = 1;
	else
		update.dark = 0;


	send_all(&update,sizeof(update));

	return 0;
}


int send_kill_all()
{
    struct kill_all_t kall;
    kall.type = 0x03;

    send_all(&kall,sizeof(kall));

    return 0;
}

// Sends a table of colours, their associated protocol names and id number
int send_colour_table(struct modptrs_t *modptrs)
{
	char name[256];
	int i = 0;
	struct flow_descriptor_t fd;
	do 
	{
		modptrs->info( fd.colour, name, i );
		fd.type = 0x04;
		strcpy( fd.name, name );
		fd.id = i;
		send_all(&fd,sizeof(fd));
		i++;
		if( i > 256 ) // Sanity check.
			exit( -99 );
	} while( !(fd.colour[0]==0 && fd.colour[1]==0 && fd.colour[2]==0) );

	return( 0 );
}

char *read_file(char *name, int &size){

	FILE *f = fopen(name, "rb");
	
	if(!f){
		return NULL;
	}
	
	fseek(f, 0, SEEK_END);
	size = ftell(f);

	char *buf = (char*)malloc(size);
	fseek(f, 0, SEEK_SET);
	if (fread(buf, 1, size, f) == -1) {
		Log(LOG_DAEMON | LOG_DEBUG, "Failed to read from file: %s\n",
				strerror(errno));
		return NULL;
	}

	fclose(f);
	
	return buf;
}


// Sends the left and right images (if set) to a client
int send_images(struct client *c){
	
	struct image_data_t image;
	
	image.type = 0x5;
		
	char *buf = NULL;
	int len = 0;
	
	if(left_image){
	
		buf = read_file(left_image, len);
		
		if(!buf){
			printf("Left image '%s' is unreadable!\n", left_image);
			exit(1);	
		}
		
		image.id = 0; //left
		image.length = htonl(len);
	
		enqueue_data(c, &image, sizeof(image));	
		enqueue_data(c, buf, len);
	
		printf("Sent '%s' (%d bytes)\n", left_image, len);
		
		free(buf);
	}
	
	if(right_image){
	
		buf = read_file(right_image, len);
		
		if(!buf){
			printf("Right image '%s' is unreadable!\n", right_image);
			exit(1);	
		}
		
		image.id = 1; //right
		image.length = htonl(len);
	
		enqueue_data(c, &image, sizeof(image));	
		enqueue_data(c, buf, len);
	
		printf("Sent '%s' (%d bytes)\n", right_image, len);
	
		free(buf);
	}

	return 0;
}
