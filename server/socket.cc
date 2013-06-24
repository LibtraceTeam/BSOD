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
#include "exporter.h"
#include <list>
#include <map>

bool wait_for_client = true;

/* For discovery replies */
static uint16_t server_port = 0;

extern char *server_name;

static void listen_cb(wand_event_handler_t *ev_hdl, int fd, void *data, 
		enum wand_eventtype_t ev) {

	struct sockaddr_in remoteaddr;
	socklen_t sock_size;
	int newfd;
	static int first_client = 1;

	
	// handle new connections
	sock_size = sizeof(struct sockaddr_in);
	if ((newfd = accept(fd, (struct sockaddr *)&remoteaddr,
					&sock_size)) == -1) { 
		perror("accept");
	} 
	else 
	{
		if (create_client(newfd) < 0) {
			Log(LOG_DAEMON | LOG_DEBUG, "Failed to initialise new client on fd %d\n", newfd);
			return;
		}

		send_existing_flows(newfd);
	}

	/* Force the initial event loop (which will be listening only) to
	 * stop and start reading the input trace */
	if (first_client && wait_for_client) {
		ev_hdl->running = false;
		first_client = 0;
	}

}

//----------------------------------------------------------
struct wand_fdcb_t * setup_listen_socket(wand_event_handler_t *ev_hdl, 
		uint16_t port, bool wait_flag)
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
	wait_for_client = wait_flag;

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
