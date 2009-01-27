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
 * $Id: socket.cc 393 2007-03-19 04:48:41Z jpc2 $
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

#include "socket.h"
#include "debug.h"
#include <syslog.h>
#include "bsod_server.h"
#include <list>

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
	int data_waiting;
	std::list< client_buffer > buffer;
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
	bool dark;
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

int listen_socket;
int udp_socket;
fd_set read_fds;
fd_set write_fds;

/* For discovery replies */
extern int port;
extern char *server_name;

/* For left and right images */
extern char *left_image, *right_image;

/* Creates a new structure containing a file descriptor */
struct client* create_fd(int fd)
{
	client *tmp = new client;
	tmp->data_waiting = 0;
	tmp->fd = fd;
	tmp->next = NULL;
	tmp->prev = NULL;

	return tmp;
}

/* Adds a structure containing a file descriptor to the front of the list */
struct client *add_fd(int fd)
{
	struct client *tmp = create_fd(fd);

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

	FD_CLR(tmp->fd, &read_fds);
	FD_CLR(tmp->fd, &write_fds);
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

	while (!tmp->buffer.empty()) {
		free(tmp->buffer.front().data);
		tmp->buffer.pop_front();
	}

	delete tmp;
}

//----------------------------------------------------------
int setup_listen_socket()
{
	int yes=1;        // for setsockopt() SO_REUSEADDR, below

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

	// reset the set and add the listening socket to it
	FD_ZERO(&read_fds);
	FD_ZERO(&write_fds);
	FD_SET(listen_socket, &read_fds);

	return listen_socket;
}


//----------------------------------------------------------
int setup_udp_socket(){

	int addr_len;
    int broadcast=1;

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
	
	//Add the UDP socket to the socket set so we can select() on it
	FD_SET(udp_socket, &read_fds);
		
	return udp_socket;
}

//-------------------------------------------------------------
int bind_tcp_socket(int listener, int port)
{
	struct sockaddr_in myaddr;     // server address

	assert(port > 0);
	assert(listener >= 0);// starts at 0? 

	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = INADDR_ANY;
	myaddr.sin_port = htons(port);
	memset(&(myaddr.sin_zero), '\0', 8);
	if (bind(listener, (struct sockaddr *)&myaddr, sizeof(myaddr)) == -1) {
		perror("bind");
		exit(1);
	}

	// listen
	if (listen(listener, 10) == -1) {
		perror("listen");
		exit(1);
	}
	return 0;
}

/*
 * Writes all waiting data out to the client immediately
 *
 * returns 1 if the client is fine
 *         0 if the client is "dead"
 */
int flush_data(struct client *client)
{
	while (!client->buffer.empty()) {
		int ret=send(client->fd,
				(char*)client->buffer.front().data+client->buffer.front().offset,
				client->buffer.front().datalen-client->buffer.front().offset,
				0);

		if (ret == -1) {
			switch (errno) {
				case EINTR: continue;
				case ENOBUFS:
				case ENOMEM:
				case EAGAIN: return 1;
				default:
					perror("send");
					return 0;
			}
		}

		client->data_waiting-=ret;

		// If we successfully wrote this data, remove it from the queue
		if (ret == (int)client->buffer.front().datalen - (int)client->buffer.front().offset) {
			free(client->buffer.front().data);
			client->buffer.pop_front();
		}
		else {
			client->buffer.front().offset+=ret;
			return 1;
		}
	}

	/* Nothing more to write */
	FD_CLR(client->fd, &write_fds);

	return 1;
}

//-----------------------------------------------------------------
/* Pickup any new clients. */
struct client *check_clients(struct modptrs_t *modptrs, bool wait)
{
	/* Protocol version is a single byte, the upper nibble is the 
	 * major version, and the lower nibble is the minor
	 * version.  The number is the lowest release version that can
	 * understand this protocol.
	 * examples:
	 *  1.2 == 0x12
	 *  10.13 = 0xad
	 */
	char protocol_version = 0x14;
	struct sockaddr_in remoteaddr;
	socklen_t sock_size;
	int newfd;
	struct timeval tv;
	struct timeval *tvp;
	struct client *tmp = clients;
	struct client *ret = NULL;
	fd_set xread_fds, xwrite_fds;
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	if (wait) {
		tvp = NULL;
	}
	else {
		tvp = &tv;
	}

	xread_fds = read_fds;
	xwrite_fds = write_fds;

	while (select(fd_max+1, &xread_fds, &xwrite_fds, NULL, tvp) == -1) {
		switch (errno) {
			case EAGAIN: continue;
			case EINTR: return NULL;
			default:
				    perror("select");
				    exit(1);
		}
	}
	
	
	
	/* For every client that can accept data, send it anything that it
	 * has queued 
	 */
	struct client *next;
	while(tmp) {
		next = tmp->next;
		if (!flush_data(tmp)) {
			remove_fd(tmp);
		}
		tmp = next;
	}

	/* if listen_socket is in the set, we have a new client */
	if (FD_ISSET(listen_socket, &xread_fds))
	{
		// handle new connections
		sock_size = sizeof(struct sockaddr_in);
		if ((newfd = accept(listen_socket, (struct sockaddr *)&remoteaddr,
						&sock_size)) == -1) { 
			perror("accept");
		} 
		else 
		{
			fcntl(newfd, F_SETFL, O_NONBLOCK);
			FD_SET(newfd, &xread_fds);
			write(newfd,&protocol_version,1);
			ret=add_fd(newfd);
			if (newfd > fd_max) 
			{    
				// keep track of the maximum
				fd_max = newfd;
			}
			Log(LOG_DAEMON|LOG_DEBUG,
					"server: new connection from %s\n", 
					inet_ntoa(remoteaddr.sin_addr));
			// Update all clients with the colour table
			// This could be done better by targeting only the new
			// client.
			send_colour_table(modptrs);	
			
			// Send the new client the left and right images
			send_images(ret);
		}
	}
	
	//Check for UDP
	else if (FD_ISSET(udp_socket, &xread_fds)){
			
		struct sockaddr_in sendaddr;

		sendaddr.sin_family = AF_INET;
        sendaddr.sin_port = htons(UDP_PORT);
        sendaddr.sin_addr.s_addr = INADDR_ANY;
        memset(sendaddr.sin_zero,'\0', sizeof sendaddr.sin_zero);
 
 		int addr_len = sizeof(sendaddr);
		int numbytes;
		unsigned char buf[16];
        if ((numbytes = recvfrom(udp_socket, buf, sizeof(buf), 0,
              (struct sockaddr *)&sendaddr, (socklen_t *)&addr_len)) == -1){
       		perror("recvfrom");
       	}
       	buf[numbytes] = 0;
       	
       	Log(LOG_DAEMON|LOG_DEBUG,"UDP discovery from %s\n", 
       								inet_ntoa(sendaddr.sin_addr));
       	       
       	//Send them a response 
       	char response[256];
       	sprintf(response, "%s|%d|%s", "0.0.0.0", port, server_name);
		  
		numbytes = sendto(udp_socket, response, strlen(response) , 0, 
							(struct sockaddr *)&sendaddr, 
							sizeof(sendaddr));
		
		Log(LOG_DAEMON|LOG_DEBUG,"Sent a reply: '%s'\n", response);
       	
       	
	}

	return ret;
}

/* Enqueue data onto a clients sendq
 *
 * If this overflows the clients queue, disconnect the client.
 */
void enqueue_data(struct client *client,void *buffer, size_t size)
{
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
	FD_SET(client->fd,&write_fds);
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
	update.dark = dark;

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
	fread(buf, 1, size, f);
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
		image.length = len;
	
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
		image.length = len;
	
		enqueue_data(c, &image, sizeof(image));	
		enqueue_data(c, buf, len);
	
		printf("Sent '%s' (%d bytes)\n", right_image, len);
	
		free(buf);
	}

	return 0;
}
