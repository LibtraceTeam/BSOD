/*
 * This file is part of bsod-server
 *
 * Copyright (c) 2004-2011 The University of Waikato, Hamilton, New Zealand.
 * Authors: Brendon Jones
 *          Daniel Lawson
 *          Sebastian Dusterwald
 *          Yuwei Wang
 *          Paul Hunkin
 *          Shane Alcock
 *
 * Contributors: Perry Lorier
 *               Jamie Curtis
 *               Jesse Pouw-Waas
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

/* Borrowed from the old WanD project */

#include <sys/stat.h> /* for umask */
#include <unistd.h> /* for getpid, write, close, fork, setsid, chdir */
#include <fcntl.h> /* for creat,open */
#include <stdio.h> /* for snprintf */
#include <syslog.h> /* for openlog */
#include <string.h> /* for strrchr */
#include <stdlib.h> 
#include <assert.h>

#include "daemons.h"

/* Flag set if we are a daemon or not. If we are then set to one. Used
 * to tell if we should send output to screen or syslog.
 */

int daemonised = 0;

void put_pid( char *fname )
{
        const char *defname = "WandProject";
        char buf[512];
        int fd;

        if( fname == NULL ) {
                fname = (char *)defname;
                snprintf( buf, 512, "/var/run/%s.pid", fname );
        } else {
                snprintf( buf, 512, "%s", fname );
        }
        fd=creat(buf,0660);
        if (fd<0)
                return;
        sprintf(buf,"%i\n",getpid());
        if (write(fd,buf,strlen(buf)) != (signed)strlen(buf)) {
                close(fd);
                return;
        }
        close(fd);
}

void daemonise(char *name, int ch) 
{
        int rv;

        switch (fork()) {
                case 0:
                        break;
                case -1:
                        perror("fork");
                        exit(1);
                default:
                        _exit(0);
        }
        setsid();
        switch (fork()) {
                case 0:
                        break;
                case -1:
                        perror("fork2");
                        exit(1);
                default:
                        _exit(0);
        }
	if (ch) {
        	if (chdir("/") == -1) {
			perror("chdir");
			exit(1);
		}
	}

        umask(0133);
        close(0);
        close(1);
        close(2);
        rv = open("/dev/null",O_RDONLY);
        assert(rv == 0);
        rv = open("/dev/console",O_WRONLY);
        if (rv == -1) {
                rv=open("/dev/null",O_WRONLY);
        }
        assert(rv == 1);
        rv = dup(rv);
        assert(rv == 2);

        daemonised = 1;

        name = strrchr(name,'/') ? strrchr(name,'/') + 1 : name;

        openlog(name, LOG_PID, LOG_DAEMON);
}	
