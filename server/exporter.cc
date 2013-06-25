/*
 * This file is part of bsod-server
 *
 * Copyright (c) 2004-2013 The University of Waikato, Hamilton, New Zealand.
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
#include <signal.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>

#include <map>

#include <libfifo.h>
#include <libwandevent.h>
#include <pthread.h>

#include "bsod_server.h"
#include "debug.h"
#include "exporter.h"

typedef std::map<int, fifo_ptr_t *> ClientMap;

ClientMap clients;
struct fifo_t *fifo = NULL;

pthread_mutex_t clientmutex;
pthread_mutex_t emptymutex;
pthread_cond_t emptycond;

wand_event_handler_t *ev_hdl = NULL;

/* Stuff to send to a client upon connection */
struct image_data_t leftimagehdr;
struct image_data_t rightimagehdr;
char *leftimage = NULL;
char *rightimage = NULL;
char *colour_msg = NULL;
int colour_msg_size = 0;

static char *read_file(char *name, int *size){

        FILE *f = fopen(name, "rb");

        if(!f){
                return NULL;
        }

        fseek(f, 0, SEEK_END);
        *size = ftell(f);

        char *buf = (char*)malloc(*size);
        fseek(f, 0, SEEK_SET);
        if (fread(buf, 1, *size, f) == -1) {
                Log(LOG_DAEMON | LOG_DEBUG, "Failed to read from file: %s\n",
                                strerror(errno));
                return NULL;
        }

        fclose(f);

        return buf;
}

static void form_colour_msg(struct modptrs_t *modptrs) {

	struct flow_descriptor_t fd[256];
	char *savednames[256];
	char name[256];
	int i = 0;
	char *ptr;

	int needed = 0;

	while(i < 256) {
		modptrs->info(fd[i].colour, name, i);
		
		if (fd[i].colour[0] == 0 && fd[i].colour[1] == 0 && 
				fd[i].colour[2] == 0)
			break;
		
		fd[i].type = BSOD_PKTTYPE_COLOURS;
		fd[i].id = i;
		fd[i].namelen = strlen(name) + 1;
		savednames[i] = strdup(name);

		needed += sizeof(struct flow_descriptor_t);
		needed += (fd[i].namelen);
		i++;
	}	

	colour_msg = (char *)malloc(needed);
	colour_msg_size = needed;
	ptr = colour_msg;

	for (int j = 0; j < i; j++) {
		memcpy(ptr, &(fd[j]), sizeof(struct flow_descriptor_t));
		ptr += sizeof(struct flow_descriptor_t);
		memcpy(ptr, savednames[j], fd[j].namelen);
		ptr += fd[j].namelen;
		
		free(savednames[j]);
	}

}

void init_exporter(wand_event_handler_t *evhdl, struct export_params *eps) {
	
	int imagelen = 0;

	ev_hdl = evhdl;
	fifo = create_fifo(eps->fifo_size, NULL);
	pthread_mutex_init(&clientmutex, NULL);
	pthread_mutex_init(&emptymutex, NULL);
	pthread_cond_init(&emptycond, NULL);	

	if (eps->left_image_file) {
		leftimage = read_file(eps->left_image_file, &imagelen);	

		if (!leftimage) {
			Log(LOG_DAEMON | LOG_INFO, "Left image '%s' is unreadable!\n", eps->left_image_file);
			exit(1);
		}

		leftimagehdr.type = BSOD_PKTTYPE_IMAGE;
		leftimagehdr.id = BSOD_IMAGE_LEFT;
		leftimagehdr.length = htonl(imagelen);
	}
	
	if (eps->right_image_file) {
		rightimage = read_file(eps->right_image_file, &imagelen);	

		if (!rightimage) {
			Log(LOG_DAEMON | LOG_INFO, "Left image '%s' is unreadable!\n", eps->left_image_file);
			exit(1);
		}

		rightimagehdr.type = BSOD_PKTTYPE_IMAGE;
		rightimagehdr.id = BSOD_IMAGE_RIGHT;
		rightimagehdr.length = htonl(imagelen);
	}
	
	form_colour_msg(eps->modptrs);		

}

static int write_fifo() {
	fifo_offset_t most_rem = 0;
	fifo_ptr_t *newtail = fifo->head;
	ClientMap::iterator it;

	pthread_mutex_lock(&clientmutex);

	for (it = clients.begin(); it != clients.end(); it++) {
		fifo_offset_t to_send = fifo_ptr_available(fifo, fifo->head,
				it->second);
		fifo_offset_t sent = fifo_ptr_read_fd(fifo, it->second, 
				it->first, to_send);

		fifo_ptr_update(fifo, it->second, sent);
		
		fifo_offset_t rem = fifo_ptr_available(fifo, fifo->head,
				it->second);

		if (most_rem < rem) {
			most_rem = rem;
			newtail = it->second;
		}
	}
	pthread_mutex_unlock(&clientmutex);

	fifo_ptr_assign(fifo, fifo->tail, newtail);
	return 0;
}

void *exporter_thread(void *ptr) {

	int rc;
	sigset_t signalmask;

	sigemptyset(&signalmask);
	sigaddset(&signalmask, SIGINT);
	sigaddset(&signalmask, SIGTERM);

	rc = pthread_sigmask(SIG_BLOCK, &signalmask, NULL);
	if (rc != 0) {
		Log(LOG_DAEMON | LOG_WARNING, "Error blocking signals in export thread!\n");
		pthread_exit(NULL);
	}

	while (true) {
		pthread_mutex_lock(&emptymutex);
		assert(fifo);

		if (fifo_ptr_available(fifo, fifo->head, fifo->tail) == 0) {
			pthread_cond_wait(&emptycond, &emptymutex);
		}

		pthread_mutex_unlock(&emptymutex);
		write_fifo();
	}

	pthread_mutex_destroy(&emptymutex);
	pthread_cond_destroy(&emptycond);
	pthread_exit(NULL);

}

static int export_message(int fd, char *buf, int buflen) {

	int sent = 0;

        /* Continue sending until ALL of the buffer has been sent correctly */
        while (sent < buflen) {
                int ret = send(fd, buf + sent, buflen - sent, 0);
                if (ret == -1) {
                        Log(LOG_DAEMON | LOG_INFO, 
					"Error sending message to client!\n");
                        return -1;
                }
                sent += ret;
        }
	return 0;

}

static void disconnect_client(wand_event_handler_t *ev_hdl, int fd,
		void *data, enum wand_eventtype_t ev) {

	pthread_mutex_lock(&clientmutex);
	ClientMap::iterator it = clients.find(fd);

	if (it != clients.end()) {
		fifo_dealloc_ptr(fifo, it->second);
		clients.erase(fd);
		wand_del_fd(ev_hdl, fd);
		close(fd);

	}
	pthread_mutex_unlock(&clientmutex);

}

int create_client(int fd) {

	/* First, we have to send all the stuff that a new client needs.
	 * We can't push this onto the fifo, because the other connected
	 * clients already have it and probably shouldn't be sent it again.
	 */

	/* Send the protocol version */
	char version = BSOD_PROTOCOL_VERSION;

	if (export_message(fd, &version, sizeof(char)) < 0) {
		close(fd);
		return -1;
	}
	
	/* Send the colour table to the client */
	if (export_message(fd, colour_msg, colour_msg_size) < 0) {
		close(fd);
		return -1;
	}

	
	/* Send images to the client */
	if (export_message(fd, (char *)&leftimagehdr, 
			sizeof(leftimagehdr)) < 0) {
		close(fd);
		return -1;
	}

	if (export_message(fd, leftimage, ntohl(leftimagehdr.length)) < 0) {
		close(fd);
		return -1;
	}

	if (export_message(fd, (char *)&rightimagehdr, 
			sizeof(rightimagehdr)) < 0) {
		close(fd);
		return -1;
	}

	if (export_message(fd, rightimage, ntohl(rightimagehdr.length)) < 0) {
		close(fd);
		return -1;
	}

	/* Send details about all existing flows to the client */


	/* Create a new fd event to detect a client disconnect */
	if (wand_add_fd(ev_hdl, fd, EV_READ, NULL, disconnect_client) == NULL)
	{
		close(fd);
		return -1;
	}

	pthread_mutex_lock(&clientmutex);
	clients[fd] = fifo_alloc_ptr(fifo, fifo->head->offset);
	pthread_mutex_unlock(&clientmutex);
	
	return fd;
	
}

static int push_onto_fifo(void *buffer, size_t size) {
	if (size == 0)
		return 0;

	if (fifo_write(fifo, buffer, size) == 0) {
		Log(LOG_DAEMON | LOG_ERR, "Fifo has filled up!\n");
		return -1;
	}

	pthread_cond_signal(&emptycond);
	return 0;

}

static float htonf(float x) {
        union {
                float f;
                uint32_t i;
        } _u;
        _u.f = (x);
        _u.i=htonl(_u.i);
        return _u.f;
}


void export_new_packet(uint32_t ts, uint32_t id, unsigned char id_num,
		uint16_t size, float speed, bool dark) {

	struct pack_update_t update;
        update.type = BSOD_PKTTYPE_PACKET;
        update.ts = htonl(ts);
        update.id = htonl(id);
        update.id_num = id_num;
        update.size = htons(size);
        update.speed = htonf(speed);
        if (dark)
                update.dark = 1;
        else
                update.dark = 0;

	push_onto_fifo(&update, sizeof(update));	

}

void export_new_flow(float start[3], float end[3], uint32_t id, uint32_t ip1,
		uint32_t ip2) {

	struct flow_update_t update;
        update.type = BSOD_PKTTYPE_FLOW;
        update.x1 = htonf(start[0]);
        update.y1 = htonf(start[1]);
        update.z1 = htonf(start[2]);
        update.x2 = htonf(end[0]);
        update.y2 = htonf(end[1]);
        update.z2 = htonf(end[2]);
        update.count = htonl(id);
        update.ip1 = htonl(ip1);
        update.ip2 = htonl(ip2);

	push_onto_fifo(&update, sizeof(update));

}

void export_kill_flow(uint32_t id) {
	struct flow_remove_t update;
        update.type = BSOD_PKTTYPE_KILLFLOW;
        update.id = htonl(id);

	push_onto_fifo(&update, sizeof(update));
}

void export_kill_all(void) {

	struct kill_all_t kall;
	kall.type = BSOD_PKTTYPE_KILLALL;

	push_onto_fifo(&kall, sizeof(kall));

}

/* Only exports the flow to a single (newly-connected) client. This WON'T
 * insert the flow into the fifo -- all of the other clients should already
 * know about this flow.
 */
void export_existing_flow(int fd, float start[3], float end[3], uint32_t id, 
		uint32_t ip1, uint32_t ip2) {

	struct flow_update_t update;
        update.type = BSOD_PKTTYPE_FLOW;
        update.x1 = htonf(start[0]);
        update.y1 = htonf(start[1]);
        update.z1 = htonf(start[2]);
        update.x2 = htonf(end[0]);
        update.y2 = htonf(end[1]);
        update.z2 = htonf(end[2]);
        update.count = htonl(id);
        update.ip1 = htonl(ip1);
        update.ip2 = htonl(ip2);

	export_message(fd, (char *)&update, sizeof(update));
}

bool activeClients(void) {
	return (!clients.empty());	
}
