#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <stdint.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <vector>
#include <algorithm>
#include <numeric>
#include <iostream>




#include "position.h"

/*
 * Number of /24 networks you are mapping. These are spread evenly across
 * the vertical axis
 */

#define NETCOUNT 50  


/**
 * This is used when you have a B-class or /16 CIDR network you wish you
 * visualalise on one side. The first two octets are the same, so the last two
 * octets are used for placement.  One dimension is based on the octet, the
 * other is based on the last octet.
 */

uint32_t nets[NETCOUNT] = {0};

static int check_subnet(uint32_t net) {
	int i = 0;

	for (i = 0; i < NETCOUNT; i ++) {
		if (nets[i] == net) 
			return i;
		if (nets[i] == 0) {
			nets[i] = net;
			return i;
		}
	}
	return -1;
}

void mod_get_position(float coord[3], struct in_addr ip) {

	int index = -1;
	uint32_t net = 0;

	net = ntohl(ip.s_addr) & 0xffffff00;

	if ((index = check_subnet(net)) == -1) {
		printf("increase NETCOUNT in multiplenet24.cc\n");
		assert(index != -1);
	}
	

	coord[1] = ((float) (20 / (NETCOUNT-1)) * index) - 10; 
	//coord[1] = ((float) ((ntohl(ip.s_addr) & 0x0000ff00) >> 8)/12.8) - 10;

	coord[2] = ((float) (ntohl(ip.s_addr) & 0x000000ff)/12.8) - 10;
}

