
// $Id$

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

