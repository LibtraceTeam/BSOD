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
 * networkxxyy uses the first two octets of the address as the X axis
 * and the second two as the Y axis.
 */
void mod_get_position(float coord[3], struct in_addr ip) {

	coord[1] = ((float) ((ntohl(ip.s_addr) & 0xffff0000) >> 16)/(12.8*256)) - 10;

	coord[2] = ((float) (ntohl(ip.s_addr) & 0x0000ffff)/(12.8*256)) - 10;
}

