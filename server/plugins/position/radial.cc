#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
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
#define SIDE_LENGTH 8
#define MAX_SIZE 10000



/** radial is used for the 'globe' picture. The top two octets are used to
 * determine an angle around the circle, and the last two are used to give a
 * length
 */
void get_position(float coord[3], struct in_addr ip) { 
	
	
	float length, angle; 
	ip.s_addr = ntohl(ip.s_addr); 
	angle = ((ip.s_addr & 0xffff0000) >> 16) % MAX_SIZE; 
	length = (ip.s_addr & 0xffff) % MAX_SIZE;

	coord[1] = angle/MAX_SIZE * sin( (2* M_PI * length) / MAX_SIZE) 
	    * SIDE_LENGTH;
	coord[2] = angle/MAX_SIZE * cos( (2* M_PI * length) / MAX_SIZE) 
	    * SIDE_LENGTH;

}

