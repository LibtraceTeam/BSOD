#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>

extern "C" {
#include "socket.h"
}

/* list of file descriptors for all connected clients */
struct client {
    int fd;
    struct client *next;
    struct client *prev;
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
} __attribute__((packed));

/* structure for new packet packets */
struct pack_update_t {
    unsigned char type;
    uint64_t ts;
    uint32_t id;
    uint8_t colour[3];
    uint16_t size;
} __attribute__((packed));

/* structure for expire flow packets */
struct flow_remove_t {
    unsigned char type;
    uint32_t id;
} __attribute__((packed));

int listen_socket;
fd_set read_fds;
int fd_max;


/* hack to get the max fd in here, do it a better way */
void hax_fdmax(int fd)
{
    fd_max = fd;
}

/* Creates a new structure containing a file descriptor */
struct client* create_fd(int fd)
{
    client *tmp = new client;
    tmp->fd = fd;
    tmp->next = NULL;
    tmp->prev = NULL;

    return tmp;
}

/* Adds a structure containing a file descriptor to the front of the list */
void add_fd(int fd)
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

}

/* Removes a given structure from the list of file descriptors */
void remove_fd(struct client *tmp)
{
    printf("Removing client on fd %i\n", tmp->fd);

    close(tmp->fd);

    if(tmp->next == NULL && tmp->prev == NULL) // only item
    {
	printf("first and only\n");
	clients = NULL;
	free(tmp);
	return;
    }

    if(tmp->next != NULL && tmp->prev == NULL) // first item
    {
	tmp->next->prev = NULL;
	clients = tmp->next;
	free(tmp);
	return;
    }
    
    if(tmp->next == NULL && tmp->prev != NULL) // last item
    {
	tmp->prev->next = NULL;
	free(tmp);
	return;

    }

    if(tmp->next != NULL && tmp->prev != NULL) // middle item
    {
	tmp->next->prev = tmp->prev;
	tmp->prev->next = tmp->next;
	free(tmp);
	return;
    }
    /*
    if(tmp->next != NULL || tmp->prev != NULL)
    {
	if(tmp->next != NULL)
	    tmp->next->prev = tmp->prev;
	if(tmp->prev != NULL)
	    tmp->prev->next = tmp->next;
    }
    ////////////////////
    else if(tmp->next == NULL && tmp->prev == NULL)
    {
	printf("first and only (SHOULD)\n");
	clients = tmp->next;
    }
*/
    free(tmp);
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

    // reset the set and add the listening socket to it, we'll need it later
    FD_ZERO(&read_fds);
    FD_SET(listen_socket, &read_fds);
    
    return listen_socket;
}

//-------------------------------------------------------------
int bind_tcp_socket(int listener, int port)
{
    struct sockaddr_in myaddr;     // server address
    
    assert(port > 0); // care if it is less than 1024?
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


//-----------------------------------------------------------------
// pickup any new clients
int check_clients(bool wait)
{
    struct sockaddr_in remoteaddr;
    socklen_t sock_size;
    int newfd;
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    FD_SET(listen_socket, &read_fds);

    if(wait) // wait on the first time through so the rtclient isnt running
    {
	if (select(fd_max+1, &read_fds, NULL, NULL, NULL) == -1) {
	    perror("select");
	    exit(1);
	}
    }
    else // timeout instantly if there are no new clients
    {
	if (select(fd_max+1, &read_fds, NULL, NULL, &tv) == -1) {
	    perror("select");
	    exit(1);
	}
    }

    /* if listen_socket is in the set, we have a new client */
    if (FD_ISSET(listen_socket, &read_fds)) 
    {
	// handle new connections
	sock_size = sizeof(struct sockaddr_in);
	if ((newfd = accept(listen_socket, (struct sockaddr *)&remoteaddr,
			&sock_size)) == -1) { 
	    perror("accept");
	} else {
	    FD_SET(newfd, &read_fds);
	    add_fd(newfd);
	    if (newfd > fd_max) {    // keep track of the maximum
		fd_max = newfd;
	    }
	    printf("server: new connection from %s on "
		    "socket %d\n", inet_ntoa(remoteaddr.sin_addr), newfd);
	}
	return newfd;
    }

    return -1;
}


// make a nice function to send to all that can be plugged in

//-------------------------------------
int send_kill_flow(uint32_t id)
{
    struct client *tmp = clients;
    struct flow_remove_t update;
    update.type = 0x02;
    update.id = id;

   // send to all clients 
    while(tmp != NULL)
    {
	if(send(tmp->fd, &update, sizeof(struct flow_remove_t), 0) 
		!= sizeof(struct flow_remove_t )){
	    perror("send_kill_flow");
	    printf("Couldn't send all data - broken pipe?\n");
	    remove_fd(tmp);
	    return 1;
	}
	tmp = tmp->next;
    }
    

    return 0;
}

//------------------------------------------------------------------
int send_new_flow(float start[3], float end[3], uint32_t id)
{
    struct client *tmp = clients;
    struct flow_update_t update;
    update.type = 0x00;
    update.x1 = start[0];
    update.y1 = start[1];
    update.z1 = start[2];
    update.x2 = end[0];
    update.y2 = end[1];
    update.z2 = end[2];
    update.count = id;

    // send to all clients
    while(tmp != NULL)
    {
	if(send(tmp->fd, &update, sizeof(struct flow_update_t), 0) 
		!= sizeof(struct flow_update_t )){
	    perror("send_new_flow");
	    printf("Couldn't send all data - broken pipe?\n");
	    remove_fd(tmp);
	    return 1;
	}
	tmp = tmp->next;
    }
    return 0;
}
//-------------------------------------------------------------------

int send_update_flow(int fd, float start[3], float end[3], uint32_t id)
{

    struct flow_update_t update;
    update.type = 0x00;
    update.x1 = start[0];
    update.y1 = start[1];
    update.z1 = start[2];
    update.x2 = end[0];
    update.y2 = end[1];
    update.z2 = end[2];
    update.count = id;


    // send to single client 
    if(send(fd, &update, sizeof(struct flow_update_t), 0) 
	    != sizeof(struct flow_update_t )){
	perror("send_new_flow");
	printf("Couldn't send all data - broken pipe?\n");
	return 1;
    }
    return 0;
}

//-------------------------------------------------------------------
int send_new_packet(uint64_t ts, uint32_t id, uint8_t colour[3],uint16_t size)
{
    struct client *tmp = clients;
    struct pack_update_t update;
    update.type = 0x01;
    update.ts = ts;
    update.id = id;
    update.colour[0] = colour[0];
    update.colour[1] = colour[1];
    update.colour[2] = colour[2];
    update.size = size;

    // send to all clients
    while(tmp != NULL)
    {
	if(send(tmp->fd, &update, sizeof(struct pack_update_t), 0) 
		!= sizeof(struct pack_update_t )){
	    perror("send_new_packet");
	    printf("Couldn't send all data - broken pipe?\n");
	    remove_fd(tmp);
	    return 1;
	}
	tmp = tmp->next;
    }
    return 0;
}
