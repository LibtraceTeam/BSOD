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

#include "../../debug.h"
#include <syslog.h>

#include <GeoIP.h>

struct cc_loc
{
    char cc_code1;
    char cc_code2;
    float latitude;
    float longitude;
};
typedef struct cc_loc cc_loc_t;

static int codesloaded = 0;
static cc_loc_t **codes = 0;
static GeoIP * gi;

static void init_module(void) __attribute__((constructor));
static void init_module(void)
{
    gi = GeoIP_new(GEOIP_STANDARD);
}

void mod_init_dir(char* filename)
{
    FILE *fin = 0;
    char *buffer;
    int lines = 0;
    //int i = 0;
    int num = 0;


    buffer = (char *)malloc(65536);
    if( (fin = fopen(filename,"r")) == NULL)
    {
		Log(LOG_DAEMON|LOG_INFO,
		"Couldn't load country lat/long file '%s', giving up\n", filename);
		exit(1);
	//err(1, "Couldn't load mac address file %s, giving up", filename);
    }

    while (fgets(buffer,65536,fin)) {
		lines ++;
    }

    rewind(fin);
    if (codes) {
		free(codes);
		codes = 0;
    }
    codes = (cc_loc_t **)malloc(sizeof(cc_loc_t *) * lines);

    while(fgets(buffer,65536,fin)) {
		char cc_code1;
		char cc_code2;
		float lat_tmp;
		float long_tmp;
		num = sscanf(buffer,"%c%c,%f,%f", &cc_code1,&cc_code2, 
				&lat_tmp, &long_tmp);

		printf("%c%c,%4.0f,%4.0f\n", cc_code1, cc_code2, lat_tmp, long_tmp);

		codes[codesloaded] = (cc_loc_t *)malloc(sizeof(cc_loc_t));
		codes[codesloaded]->cc_code1 = cc_code1;
		codes[codesloaded]->cc_code2 = cc_code2;
		codes[codesloaded]->latitude = lat_tmp;
		codes[codesloaded]->longitude = long_tmp;
		codesloaded++;
    }


    free(buffer);

    return;


}


/**
 * This position module lays things out according to geographic information
 * from geoip.
 *
 * Uses:
 *  http://www.maxmind.com/download/geoip/api/c/
 *  http://www.maxmind.com/app/country_latlong
 *  latlong.png
 */
void mod_get_position(float coord[3], struct in_addr ip) {
    char *ipaddy = inet_ntoa(ip);
    const char *country = GeoIP_country_code_by_addr(gi, ipaddy);

	if (!country)
    {
		printf("country == NULL\n");
			coord[1] = 0;
			coord[2] = 0;
		return;
    }

    int found = 0;
    for (int i = 0; i < codesloaded; i++)
    {
		char bar[3];
		bar[0] = codes[i]->cc_code1;
		bar[1] = codes[i]->cc_code2;
		bar[2] = '\0';
        if ((strcasecmp(country, bar))==0)
        {
            found = 1;
            coord[1] = (codes[i]->latitude/9.0); // latitude
            coord[2] = (codes[i]->longitude/18.0); // longitude
        }
    }
    if (!found)
    {
        coord[1] = -10;
        coord[2] = -10;
    }

}

// vim:sts=4:ts=4:sw=4
