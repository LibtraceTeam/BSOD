#ifndef _POSITION_H
#define _POSITION_H

#include <netinet/in.h>
extern "C" void mod_get_position(float coord[3], struct in_addr ip);

#endif // _POSITION_H
