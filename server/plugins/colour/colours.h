#ifndef _COLOURS_H
#define _COLOURS_H

#include <stdint.h>
/** get_colour() accepts an array of 3 chars, and the IP protocol and TCP or UDP port 
 * value. It then makes "a decision" on these two parameters, and fills in the color
 * array with appropriate RGB values.
 * 
 */

extern "C" {
void get_colour(uint8_t colour[3], int port, int protocol);
}


#endif // _COLOURS_H
