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


#include <stdio.h> 
#include <syslog.h> 
#include <stdlib.h> 
#include <stdarg.h> 
#include <assert.h> 

#include "debug.h"
#include "daemons.h"

extern int daemonised;

void Log(int priority, char *fmt, ...)
{
        va_list ap;
        char buffer[513];

        assert(daemonised == 0 || daemonised == 1);

        va_start(ap, fmt);

        if(! daemonised){
                vfprintf(stderr, fmt, ap);
        } else {
                vsnprintf(buffer, sizeof(buffer), fmt, ap);
                syslog(priority, "%s", buffer);
        }
        va_end(ap);

}

