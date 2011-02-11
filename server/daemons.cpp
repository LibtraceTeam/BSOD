/* Wand Project - Ethernet Over UDP
 * $Id: daemons.c 367 2007-02-23 03:04:27Z perry $
 * Licensed under the GPL, see file COPYING in the top level for more
 * details.
 */

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
