#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>

#include "socket.h"




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


struct pack_update_t {
    unsigned char type;
    uint64_t ts;
    uint32_t id;
    char colour[3];
    uint16_t size;
} __attribute__((packed));

struct flow_remove_t {
    unsigned char type;
    uint32_t id;
} __attribute__((packed));

int listen_socket;



//----------------------------------------------------------
int setup_listen_socket()
{
    int yes=1;        // for setsockopt() SO_REUSEADDR, below

    // get the listener
    if ((listen_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	perror("socket");
	exit(1);
    }

    // lose the pesky "address already in use" error message
    if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &yes,
		sizeof(int)) == -1) {
	perror("setsockopt");
	exit(1);
    }
    
    return listen_socket;
}

//-------------------------------------------------------------
int bind_tcp_socket(int listener, int port)
{
    struct sockaddr_in myaddr;     // server address
    
    assert(port > 0); // care is less than 1024?
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
int check_client(int *fdmax, fd_set *master)
{
    return 0;
}

//-------------------------------------
int send_kill_flow(int new_fd, uint32_t id)
{

    struct flow_remove_t update;
    update.type = 0x02;
    update.id = id;

    if(send(new_fd, &update, sizeof(struct flow_remove_t), 0) 
	    != sizeof(struct flow_remove_t )){
	perror("send_kill_flow");
	printf("Couldn't send all data - broken pipe?\n");
	return 1;
    }
    return 0;
}

//------------------------------------------------------------------
int send_new_flow(int new_fd, float start[3], float end[3], uint32_t id)
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

    //printf("----sending flow %i\n", id);

    if(send(new_fd, &update, sizeof(struct flow_update_t), 0) 
	    != sizeof(struct flow_update_t )){
	perror("send_new_flow");
	printf("Couldn't send all data - broken pipe?\n");
	return 1;
    }
    return 0;
}

//-------------------------------------------------------------------
int send_new_packet(int new_fd, uint64_t ts, uint32_t id, char colour[3],uint16_t size)
{

    struct pack_update_t update;
    update.type = 0x01;
    update.ts = ts;
    update.id = id;
    update.colour[0] = colour[0];
    update.colour[1] = colour[1];
    update.colour[2] = colour[2];
    update.size = size;

    //printf("----sending packet %i\n", id);

    if(send(new_fd, &update, sizeof(struct pack_update_t), 0) 
	    != sizeof(struct pack_update_t )){
	perror("send_new_packet");
	printf("Couldn't send all data - broken pipe?\n");
	return 1;
    }
    return 0;
}
