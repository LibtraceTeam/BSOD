/* Wand Project - Ethernet Over UDP
 * $Id$
 * Licensed under the GPL, see file COPYING in the top level for more
 * details.
 */
    
#ifndef DAEMONS_H
#define DAEMONS_H

// $Id$

#ifdef __cplusplus
extern "C" {
#endif


void put_pid( char *fname );
void daemonise( char *name, int ch ) ;

extern int daemonised;

#ifdef __cplusplus
}
#endif

#endif /* DEAMONS_H */
    
