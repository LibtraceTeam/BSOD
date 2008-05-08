/* Wand Project - Ethernet Over UDP
 * $Id: daemons.h 107 2004-10-13 21:44:02Z dlawson $
 * Licensed under the GPL, see file COPYING in the top level for more
 * details.
 */
    
#ifndef DAEMONS_H
#define DAEMONS_H

// $Id: daemons.h 107 2004-10-13 21:44:02Z dlawson $

#ifdef __cplusplus
extern "C" {
#endif


void put_pid( const char *fname );
void daemonise( const char *name, int ch ) ;

extern int daemonised;

#ifdef __cplusplus
}
#endif

#endif /* DEAMONS_H */
    
