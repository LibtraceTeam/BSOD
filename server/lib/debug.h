/* Wand Project - Ethernet Over UDP
 * $Id$
 * Licensed under the GPL, see file COPYING in the top level for more
 * details.
 */

#ifndef DEBUG_H
#define DEBUG_H 
    
#ifdef __cplusplus
extern "C" {
#endif

	/* The grand unified output system. All output should go through this
	 * function.
	 *
	 * You must specify both a module name that this output has come from
	 * and the level of the message.
	 *
	 * The module names are all defined later
	 *
	 * The levels are a integer between 0 and 15
	 *
	 * 0 being the quietest and 15 being the most verbose.
	 * 
	 * 0 - Absolute SILENCE. Should only ever be used for special temporary
	 * 	applications. Never in final code.
	 *
	 * 	SYSLOG level of this alert = LOG_ALERT
	 *
	 * 1 - Used for failure messages and extream crirical conditions
	 *
	 * 	SYSLOG level of this alert = LOG_ALERT
	 *
	 * 2 - Critical conditions
	 *
	 * 	SYSLOG level of this alert = LOG_CRIT
	 *
	 * 3 - Error conditions
	 *
	 * 	SYSLOG level of this alert = LOG_ERR
	 *
	 * 4 - Warning conditions
	 *
	 * 	SYSLOG level of this alert = LOG_WARNING
	 *
	 * 5 - Normal, but significant, condition
	 *
	 * 	SYSLOG level of this alert = LOG_NOTICE
	 *
	 * 6 - Informational message
	 *
	 * 	SYSLOG level of this alert = LOG_INFO
	 *
	 * 7 - Debug messages
	 * 
	 * 	SYSLOG level of this alert = LOG_DEBUG
	 *
	 * 8 - 15 Increasingly verbose debuggind
	 *
	 * 	SYSLOG level of this alert = LOG_DEBUG
	 */
	
void logger(int module, int level, const char *format, ...);

/* Define the module names */

#define MOD_MISC 0	/* Misc output. Hopefully used less and less */
#define MOD_CONFIG 1	/* Output for the configuration parsing. */
#define MOD_DRIVERS 2	/* Anything to do with etud's drivers. */
#define MOD_INIT 3	/* Anything during code setup and initialisation */
#define MOD_IPC 4	/* wand <-> etud IPC control */
#define MOD_NETWORK 5	/* Anything related to the networking code */
#define MOD_IF 6	/* Anything related to the interfaces code */
#define MOD_LIST 7	/* All the stuff to do with etud's list code */

#ifdef __cplusplus
}
#endif

#endif /* DEBUG_H */



