#ifndef _BSOD_SERVER_H
#define _BSOD_SERVER_H
#include <stdint.h>

typedef void (* colfptr)(uint8_t[3],int,int);
typedef int (* posfptr)(float[3],struct in_addr);
struct modptrs_t {
	colfptr colour;
	posfptr left;
	posfptr right;
};


#endif // _BSOD_SERVER_H
