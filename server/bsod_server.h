#ifndef _BSOD_SERVER_H
#define _BSOD_SERVER_H
#include <stdint.h>
#include "libtrace.h"

typedef void (* colfptr)(uint8_t[3],int,int);
typedef int (* posfptr)(float[3],struct in_addr);
typedef int (* dirfptr)(struct libtrace_t *, void *, int);
struct modptrs_t {
	colfptr colour;
	posfptr left;
	posfptr right;
	dirfptr direction;
};


#endif // _BSOD_SERVER_H
