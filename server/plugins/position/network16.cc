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


/**
 * This is used when you have a B-class or /16 CIDR network you wish you
 * visualalise on one side. The first two octets are the same, so the last two
 * octets are used for placement.  One dimension is based on the octet, the
 * other is based on the last octet.
 */
void get_position(float coord[3], struct in_addr ip) {

	coord[1] = ((float) ((ntohl(ip.s_addr) & 0x0000ff00) >> 8)/12.8) - 10;

	coord[2] = ((float) (ntohl(ip.s_addr) & 0x000000ff)/12.8) - 10;
}

