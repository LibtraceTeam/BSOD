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
 * $Id: bsod_server.cc 412 2007-11-13 03:14:24Z spa1 $
 *
 */


/*
 * main loop(s) and the signal handlers may need looking at?
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <signal.h>
#include <netdb.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <errno.h>
#include <assert.h>
#include <syslog.h>
#include <map>

#include <libtrace.h>
// remove this header after replace with libconfuse
//#include <libconfig.h>
#include <confuse.h>
#include "bsod_server.h"

#include "socket.h"
#include "packets.h"
#include "daemons.h"
#include "debug.h"

#include "RTTMap.h"
#include "Blacklist.h"

int restart_config = 1;
int terminate_bsod = 0;
int fd_max;

libtrace_t *trace = 0;

struct timeval tracetime;
struct timeval starttime;
struct timeval nowtime;

int background = 0;
char *pidfile = 0;
char *basedir = 0;
char *filterstring = 0;
char *dirmod = 0;
char *colmod = 0;
char *leftpos = 0;
char *rightpos = 0;
char *macaddrfile = 0;
char *blacklistdir = 0;
char *configfile = "/usr/local/bsod/etc/bsod_server.conf";
static char* uri = 0; 

int port = 32500;
int loop = 0;
int shownondata = 0;
int showdata = 1;
int showcontrol = 1;
int sampling = 0;
bool live = true;
void *dirhandle = 0;
void *colourhandle = 0;
void *lefthandle = 0;
void *righthandle = 0;

static void sigusr_hnd(int sig);
static void sigterm_hnd(int sig);
void do_configuration(int argc, char **argv);

struct modptrs_t modptrs;

void do_usage(char* name) {
    printf("Usage: %s [-h] [-b] -C <configfile> \n", name);
    exit(0);
}

static int load_modules();
static void close_modules();
static void init_times();
static void offline_delay(struct timeval tv);
static void init_signals();
static int bsod_read_packet(libtrace_packet_t *packet, 
			blacklist *theList, RTTMap *rttmap);
void create_filter(struct libtrace_filter_t ** filter, char * filterstr);
int check_trace_err(libtrace_t * trace);
void daemonise(char * arg);
int restart();

int main(int argc, char *argv[]) {
	RTTMap *rttmap = new RTTMap();
	blacklist *theList;
	int listen_socket;
	fd_set listen_set, event_set;
	struct client *new_client = NULL;
	struct timeval last_packet_time;
	static struct libtrace_filter_t *filter = 0;
	struct libtrace_packet_t *packet;

	FD_ZERO(&listen_set);
	FD_ZERO(&event_set);
	init_signals();

	// do some command line stuff for ports and things
	do_configuration(argc,argv);

	// daemonise, and write out pidfile.
	daemonise(argv[0]);
	
	//-------Setup listen socket-----------
	listen_socket = setup_listen_socket(); // set up socket
	bind_tcp_socket(listen_socket, port); // bind and start listening

	// add the listening socket to the master set
	fd_max = listen_socket; // biggest file descriptor
	Log(LOG_DAEMON|LOG_INFO, "Waiting for connection on port %i...\n", port);

	do { // loop on loop variable - restart input
		if (restart_config && !restart()) {
			Log(LOG_INFO, "here.\n");
			return 1;
		}
		
		/* set up directory where we should store our blacklist stuff */
		char tmp[4096];
		snprintf(tmp,4096,"%s%s",basedir,blacklistdir);
		Log(LOG_DAEMON|LOG_INFO,"Saving blacklist info to '%s'\n", tmp);
		theList = new blacklist( tmp );

		if (!new_client)
			new_client = check_clients(&modptrs, true);

		// reset the timers and packet objects
		kill_all();
		init_times();
		init_packets();

		if (terminate_bsod) break;
		//------- trace ----------
		trace = trace_create(uri);
		if (check_trace_err(trace)) exit(1);

		create_filter(&filter, filterstring);
		packet=trace_create_packet();
		Log(LOG_DAEMON|LOG_INFO,"Connected to data source: %s\n", uri);
		gettimeofday(&starttime, 0); // XXX

		//----- Check live status --------
		if ((!strncmp(uri,"erf:",4)) || (!strncmp(uri,"pcap:",5))) {
			Log(LOG_DAEMON|LOG_INFO,"Attempting to replay trace in real time\n");
			live = false;
		} // default live = true
		
		if (trace_start(trace)==-1 && check_trace_err(trace))
			exit(1);

		while(loop && bsod_read_packet(packet, theList, rttmap)) { }

		if (!check_trace_err(trace)) {
			// expire any outstanding flows
			last_packet_time = trace_get_timeval(packet);
			expire_flows(last_packet_time.tv_sec+3600);
		}
		// We've finished with this trace
		trace_destroy(trace);
		trace = NULL;
	} while ((loop == 1) || (restart_config == 1));

	// clean up
	close(listen_socket);
	close_modules();
	delete rttmap;
	delete theList;
	Log(LOG_DAEMON|LOG_INFO,"Exiting...\n");

	return 0;
}

int restart() {
	Log(LOG_DAEMON|LOG_INFO,"Rereading configuration file upon user request\n");
	restart_config = 0;
	close_modules();
	do_configuration(0,0);
	if (!load_modules()) {
		Log(LOG_DAEMON|LOG_INFO,"Failed to load modules, aborting\n");
		return 0;
	}
	return 1;
}

void daemonise(char * arg) {
	if (background == 1) {
		// don't chdir
		daemonise(arg, 0);
		// write out pidfile
		put_pid(pidfile);
	}
}

// 1 for error, 0 for clean
int check_trace_err(libtrace_t * trace) {
	struct trace_err_t err;
	if(trace_is_err(trace)) {
		Log(LOG_DAEMON|LOG_ALERT,
				"trace error: %s\n",
				err.problem);
		return 1;
	}
	return 0;
}

void create_filter(struct libtrace_filter_t ** filter, char * filterstr) {
	if (*filter)
		trace_destroy_filter(*filter);
	*filter = 0;
	if (filterstr) {
		Log(LOG_DAEMON|LOG_INFO,"setting filter %s\n",filterstr);
		*filter = trace_create_filter(filterstr);
		trace_config(trace,TRACE_OPTION_FILTER,filter);
	}
}

static int bsod_read_packet(libtrace_packet_t *packet, 
		blacklist *theList, RTTMap *rttmap) {
	static time_t next_save = 0;
	static int packet_count = 0;
	struct timeval packettime;
	int psize = 0;
	struct client *new_client = NULL;

	// If we get a USR1, we want to restart. Break out of
	// this while() and go into cleanup
	if (restart_config) return 0;

	if (terminate_bsod) {
		loop=0;
		return 0;
	}
	
	/* check for new clients */
	new_client = check_clients(&modptrs, false);
	if(new_client) send_flows(new_client);

	/* get a packet, and process it */
	if((psize = trace_read_packet(trace, packet)) <= 0) return 0;

	++packet_count;

	// If we're sampling packets, skip packets that
	// don't meet our sampling criteria
	if (sampling && packet_count % sampling != 0) return 1;

	packettime = trace_get_timeval(packet);

	/* this time checking only matters when reading from a
	 * prerecorded trace file. It limits it to a seconds
	 * worth of data a second, which is still a large
	 * chunk
	 */ 
	if(!live) offline_delay(packettime);
	
	// if sending fails, assume we just lost a client
	if(per_packet(packet, packettime.tv_sec, &modptrs, rttmap, theList)) return 1;

	if (packettime.tv_sec > next_save) {
		/* Save the blacklist every 5 min */
		next_save = packettime.tv_sec + 300;
		theList->save();
	}
	return 1;
}


static void sigusr_hnd(int signo) {
	restart_config = 1;
}

static void sigterm_hnd(int signo) {
	terminate_bsod = 1;
}

static void init_signals() {
	struct sigaction sigact;
	// setup signal handlers
	sigact.sa_handler = sigusr_hnd;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;
	if(sigaction(SIGUSR1, &sigact, NULL) < 0) {
		Log(LOG_DAEMON|LOG_DEBUG,"sigaction SIGUSR1: %d\n",errno);
	}
	
	sigact.sa_handler = SIG_IGN;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;

	if(sigaction(SIGPIPE, &sigact, NULL) < 0) {
		perror("sigaction");
		exit(-1);
	}

	sigact.sa_handler = sigterm_hnd;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;

	if (sigaction(SIGTERM, &sigact, NULL) < 0) {
		perror("sigaction(SIGTERM)");
		exit(1);
	}
	if (sigaction(SIGINT, &sigact, NULL) < 0) {
		perror("sigaction(SIGINT)");
		exit(1);
	}
}

void do_configuration(int argc, char **argv) {
	int opt;
	// read cmdline opts
	while( argv && (opt = getopt(argc, argv, "hbC:")) != -1) {
		switch(opt) {
			case 'b':
				background = 1;	break;
			case 'h':
				do_usage(argv[0]); break;
			case 'C':
				configfile = optarg; break;
			default: do_usage(argv[0]);
		};
	}

	cfg_opt_t opts[] = {
		CFG_STR("source", "", CFGF_NONE),
		CFG_STR("basedir", "/usr/local/bsod/", CFGF_NONE),
		CFG_STR("dir_module", "plugins/direction/interface.so", CFGF_NONE),
		CFG_STR("colour_module", "plugin/colour/colours.so", CFGF_NONE),
		CFG_STR("lpos_module", "plugin/position/random.so", CFGF_NONE),
		CFG_STR("rpos_module", "plugin/position/radial.so", CFGF_NONE),
		CFG_INT("shownondata", 1, CFGF_NONE),
		CFG_INT("showdata", 1, CFGF_NONE),
		CFG_INT("showcontrol", 1, CFGF_NONE),
		CFG_STR("blacklistdir", "blist/", CFGF_NONE),
		CFG_STR("pidfile", "/var/run/bsod_server.pid", CFGF_NONE),
		CFG_INT("listenport", 34567, CFGF_NONE),
		CFG_INT("loop", 1, CFGF_NONE),
		CFG_INT("background", background, CFGF_NONE),
		CFG_STR("filter", 0, CFGF_NONE),
		CFG_STR("macaddrfile", "/etc/mac_addrs", CFGF_NONE),
		CFG_END()
	};

	cfg_t *cfg;
	cfg = cfg_init(opts, CFGF_NONE);
	if(cfg_parse(cfg, configfile) == CFG_PARSE_ERROR) {
		fprintf(stderr, "failed on reading configure file\n");
		return;
	}

	// read back configurations
	uri = cfg_getstr(cfg, "source");
	basedir = cfg_getstr(cfg, "basedir");
	dirmod = cfg_getstr(cfg, "dir_module");
	colmod = cfg_getstr(cfg, "colour_module");
	leftpos = cfg_getstr(cfg, "lpos_module");
	rightpos = cfg_getstr(cfg, "rpos_module");
	shownondata = cfg_getint(cfg, "shownondata");
	showdata = cfg_getint(cfg, "showdata");
	showcontrol = cfg_getint(cfg, "showcontrol");
	blacklistdir = cfg_getstr(cfg, "blacklistdir");
	pidfile = cfg_getstr(cfg, "pidfile");
	port = cfg_getint(cfg, "listenport");
	loop = cfg_getint(cfg, "loop");
	background = cfg_getint(cfg, "background");
	filterstring = cfg_getstr(cfg, "filter");
	macaddrfile = cfg_getstr(cfg, "macaddrfile");

	//cfg_free(cfg);
}

/** Parse out the arguments and the driver name 
 * @note driver/args are allocated on the heap and should be free()'d
 * later
 */
static void parse_args(const char *line, char **driver, char **args) {
	char *tok;
	tok = strchr(line,' ');
	if (!tok) {
		*driver = strdup(line);
		*args = strdup("");
		return;
	}

	/* strndup */
	*driver = (char*)malloc(tok-line+1);
	strncpy(*driver,line,tok-line);
	(*driver)[tok-line]='\0';

	*args = strdup(tok+1);
}

/**
 * Load the module, calling it's initialisation function if appropriate
 */
static void *get_module(const char *name) {
	char tmp[4096];
	char *driver;
	char *args;

	parse_args(name,&driver,&args);
	
	snprintf(tmp,sizeof(tmp),"%s%s",basedir,driver);

	Log(LOG_DAEMON|LOG_DEBUG,"Loading module %s...\n",tmp);

	void *handle = dlopen(tmp,RTLD_LAZY);
	if (!handle) {
		Log(LOG_DAEMON|LOG_ALERT,"Couldn't load module %s: %s\n",
				driver,dlerror());
		return NULL;
	}

	initfuncfptr init_func = (initfuncfptr)dlsym(handle,"init_module");

	if (init_func) {
		Log(LOG_DAEMON|LOG_DEBUG," Initialising module %s...\n",tmp);
		if (!init_func(args)) {
			Log(LOG_DAEMON|LOG_ALERT,
			     "Initialisation function failed for %s\n",driver);
			dlclose(handle);
			handle = NULL;
		}
	} else {
		Log(LOG_DAEMON|LOG_DEBUG," No initialisation required for %s\n",tmp);
		dlerror(); /* This is needed to clear the error flag */
	}

	free(driver);
	free(args);

	return handle;
}

/**
 * Load a position module, calling it's initialisation function if appropriate
 */
static void *get_position_module(side_t side, const char *name)
{
	char tmp[4096];
	char *driver;
	char *args;

	parse_args(name,&driver,&args);
	
	snprintf(tmp,sizeof(tmp),"%s%s",basedir,driver);

	Log(LOG_DAEMON|LOG_DEBUG,"Loading module %s...\n",tmp);
	void *handle = dlopen(tmp,RTLD_LAZY);
	if (!handle) {
		Log(LOG_DAEMON|LOG_ALERT,"Couldn't load module %s: %s\n",
				driver,dlerror());
		return NULL;
	}

	initsidefptr init_func = (initsidefptr)dlsym(handle,"init_module");

	if (init_func) {
		Log(LOG_DAEMON|LOG_DEBUG," Initialising module %s...\n",tmp);
		if (!init_func(side,args)) {
			Log(LOG_DAEMON|LOG_ALERT,
			     "Initialisation function failed for %s\n",driver);
			dlclose(handle);
			handle = NULL;
		}
	}
	else {
		Log(LOG_DAEMON|LOG_DEBUG," Initialisation not required for %s\n",tmp);
		dlerror();
	}

	free(driver);
	free(args);

	return handle;
}


static int load_modules() {
	char *error = 0;
	char tmp[4096];

	//------- Load up modules -----------
	dirhandle = get_module(dirmod);
	if (!dirhandle) {
		Log(LOG_DAEMON|LOG_ALERT,"Couldn't load direction module %s\n",tmp);
		return 0;
	}

	colourhandle = get_module(colmod);
	modptrs.direction = (dirfptr) dlsym(dirhandle,"mod_get_direction");
	if ((error = (char*)dlerror()) != NULL) {
		Log(LOG_DAEMON|LOG_ALERT,"%s\n",error);
		return 0;
	}

	modptrs.colour = (colfptr)dlsym(colourhandle, "mod_get_type");
	if ((error = (char*)dlerror()) != NULL) {
		Log(LOG_DAEMON|LOG_ALERT,"%s\n",error);
		return 0;
	}
	
	modptrs.info = (inffptr) dlsym(colourhandle, "mod_get_info");
	if ((error = (char*)dlerror()) != NULL) {
		Log(LOG_DAEMON|LOG_ALERT,"%s\n",error);
		Log(LOG_DAEMON|LOG_ALERT,"Are you using an old colour module?\n");
		return 0;
	}

	lefthandle = get_position_module(SIDE_LEFT, leftpos);
	if (!lefthandle) {
		return 0;
	}

	modptrs.left = (posfptr) dlsym(lefthandle, "mod_get_position");
	if ((error = (char*)dlerror()) != NULL) {
		Log(LOG_DAEMON|LOG_ALERT,"%s\n",error);
		return 0;
	}

	righthandle = get_position_module(SIDE_RIGHT,rightpos);
	if (!righthandle) {
		return 0;
	}
	modptrs.right = (posfptr) dlsym(righthandle,"mod_get_position");
	if ((error = (char*)dlerror()) != NULL) {
		Log(LOG_DAEMON|LOG_ALERT,"%s\n",error);
		return 0;
	}

	return 1;
}



static void close_modules() {
	if(dirhandle) {
		endfptr end_module = (endfptr)dlsym(dirhandle,"end_module");
		if (end_module) {
			end_module();
		}
		dlclose(dirhandle);
	}
	if(colourhandle) {
                endfptr end_module = (endfptr)dlsym(colourhandle,"end_module");
                if (end_module) {
                        end_module();
                }
                dlclose(colourhandle);
        }
        modptrs.colour = 0;
	modptrs.init_dir = 0;
	modptrs.direction = 0;
}	

static void init_times() {
	nowtime.tv_sec = 0;
	nowtime.tv_usec = 0;
	starttime.tv_sec = 0;
	starttime.tv_usec = 0;
	tracetime.tv_sec = 0;
	tracetime.tv_usec = 0;
}


static void offline_delay(struct timeval tv){
	struct timeval diff;
	struct timeval delta;

	if(tracetime.tv_sec == 0)
		tracetime = tv;

	gettimeofday(&nowtime, 0);

	// time since we started the trace
	timersub(&nowtime,&starttime,&delta);
	
	// add this to the trace timer
	timeradd(&delta,&tracetime,&delta);
	

	// check to see if current trace time is ahead of this
	if (timercmp(&tv, &delta, >)) {
		timersub(&tv,&delta,&diff);
		usleep((diff.tv_sec * 1000000) + diff.tv_usec);
	}

	return;

}
