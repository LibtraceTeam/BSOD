/*
 * This file is part of bsod-server
 *
 * Copyright (c) 2004 The University of Waikato, Hamilton, New Zealand.
 * Authors: Brendon Jones
 *	    Daniel Lawson
 *	    Sebastian Dusterwald
 *          
 * All rights reserved.
 *
 * This code has been developed by the University of Waikato WAND 
 * research group. For further information please see http://www.wand.net.nz/
 *
 * bsod-server is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * bsod-server is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bsod-server; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id$
 *
 */


#include "colours.h"
#include <stdint.h>

typedef enum counters {
    TCP = 0,
    HTTP = 1,
    HTTPS = 2,
    MAIL = 3,
    FTP = 4,
    VPN = 5,
    DNS = 6,
    NTP = 7,
    SSH = 8,
    UDP = 9,
    ICMP = 10,
    IRC = 11,
    WINDOWS = 12,
    OTHER = 13
} counters_t;

static uint8_t countercolours[][3] = {
    {100,  0,100}, /* TCP     purple		*/
    {  0,  0,200}, /* HTTP    blue		*/
    {150,150,240}, /* HTTPS   light blue/purple	*/
    {200,  0,  0}, /* MAIL    red		*/
    {  0,150,  0}, /* FTP     green		*/
    {  0,250,  0}, /* VPN     white		*/
    {200,200,  0}, /* DNS     yellow		*/
    { 30, 85, 30}, /* NTP     matte green	*/
    {110,110,110}, /* SSH     grey		*/
    {150,100, 50}, /* UDP     light brown	*/
    {  0,250,200}, /* ICMP    teal		*/
    {240,230,140}, /* IRC     khaki brown	*/
    {200,100,  0}, /* WINDOWS orange		*/
    {255,192,203}  /* OTHER   pink		*/
};


/*
 * Sets the colour array (RGB) to be the colour appropriate to the 
 * port/protocol being used.
 */
void mod_get_colour(uint8_t colour[3], int port, int protocol)
{

    int i;
    
    switch(port)
    {
	case 80: for(i=0;i<3;i++)
		     colour[i] = countercolours[HTTP][i];
		 break;

	case 21: 
	case 20: for(i=0;i<3;i++)
		     colour[i] = countercolours[FTP][i];
		 break;

	case 110: /* pop3 */
	case 143: /* imap2 */
	case 220: /* imap3 */
	case 993: /* imap over ssl */
	case 995: /* pop3 over ssl */
	case 465: /* smtp over ssl */
	case 25: for(i=0;i<3;i++)
		     colour[i] = countercolours[MAIL][i];
		 break;

		 /* white is just too confusing when other things blend */
/*
	case 119: for(i=0;i<3;i++)
		      colour[i] = countercolours[NNTP][i];
		  break;
*/
	case 53: for(i=0;i<3;i++)
		     colour[i] = countercolours[DNS][i];
		 break;

	case 22:
	case 23: for(i=0;i<3;i++)
		     colour[i] = countercolours[SSH][i];
		 break;
		 
	case 443: for(i=0;i<3;i++)
		      colour[i] = countercolours[HTTPS][i];
		  break;

	case 6667: for(i=0;i<3;i++)
		       colour[i] = countercolours[IRC][i];
		   break;
	case 10000: for(i=0;i<3;i++)
			    colour[i] = countercolours[VPN][i];
		    break;
	case 123: for(i=0;i<3;i++)
		      colour[i] = countercolours[NTP][i];
		  break;
	case 135:
	case 136:
	case 137:
	case 138:
	case 139:
	case 445: for(i=0;i<3;i++)
		      colour[i] = countercolours[WINDOWS][i];
		  break;

      // if not a port that I'm counting give a colour based on protocol
	default:  
		  switch(protocol)
		  {
		      	case 6: for(i=0;i<3;i++)
				  colour[i] = countercolours[TCP][i];	
			      break;

		      	case 17: for(i=0;i<3;i++)
				   colour[i] = countercolours[UDP][i];
			       break;

		      	case 1: for(i=0;i<3;i++)
				  colour[i] = countercolours[ICMP][i]; 
			      break;
			      /* 
				 case 41: for(i=0;i<3;i++)
				 colour[i] = countercolours[IPMP][i]; 
				 break;
				 */
			case 37:
			case 50:
			case 51: for (i=0;i<3; i++)
					 colour[i] = countercolours[VPN][i];
		      	default: for(i=0;i<3;i++)
				   colour[i] = countercolours[OTHER][i]; 
			       break;
		  };break;
    };
	//printf("%u %u %u \n", colour[0], colour[1], colour[2]);

}


