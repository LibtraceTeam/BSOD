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
#include <confuse.h>

#include "bsod_server.h"

#include "socket.h"
#include "packets.h"
#include "daemons.h"
#include "debug.h"

#include "RTTMap.h"
#include "Blacklist.h"

typedef struct ip ip_t;

struct sigaction sigact;

int  _fcs_bits = 32;
int restart_config = 1;
int terminate_bsod = 0;
int fd_max;

libtrace_t *trace = 0;

struct timeval tracetime;
struct timeval starttime;
struct timeval nowtime;

uint32_t ts32 = 0;
uint32_t startts32 = 0;

int background = 0;
char *pidfile = 0;

char *basedir = 0;
char *filterstring = 0;
char *colourmod = 0;
char *leftpos = 0;
char *rightpos = 0;
char *dirmod = 0;
char *macaddrfile = 0;
char *blacklistdir = 0;
const char *configfile = "/usr/local/bsod/etc/bsod_server.conf";
static char* uri = 0; 
char *server_name = 0;
char *left_image = 0;
char *right_image = 0;

int port = 32500;
int loop = 0;
int shownondata = 0;
int showdata = 1;
int showcontrol = 1;
int sampling = 0;
bool live = true;

static void sigusr_hnd(int sig);
static void sigterm_hnd(int sig);
void do_configuration(int argc, char **argv);

void *colourhandle = 0;
void *lefthandle = 0;
void *righthandle = 0;
void *dirhandle = 0;

struct modptrs_t modptrs;

void do_usage(char* name)
{
    printf("Usage: %s [-h] [-b] -C <configfile> \n", name);
    exit(0);
}

static int load_modules();
static void close_modules();
static void init_times();
static void offline_delay(struct timeval tv);
static void init_signals();
static int bsod_read_packet(libtrace_packet_t *packet, blacklist *theList,
		RTTMap *rttmap);

int main(int argc, char *argv[])
{
	// RTTMap:
	RTTMap *rttmap = new RTTMap();
    
	// Blacklist:
	blacklist *theList;
	
	// socket stuff
	int listen_socket;
	fd_set listen_set, event_set;
	
	// UDP socket
	int udp_socket;

	struct client *new_client = NULL;

	struct timeval last_packet_time;

	// rt stuff
	static struct libtrace_filter_t *filter = 0;
	struct libtrace_packet_t *packet;

	FD_ZERO(&listen_set);
	FD_ZERO(&event_set);
	init_signals();

	//-------------------------------------------------------
	// do some command line stuff for ports and things

	do_configuration(argc,argv);

	//-------------------------------------------------------
	// daemonise, and write out pidfile.

	if (background == 1) {
		// don't chdir
		daemonise(argv[0],0);
		// write out pidfile
		put_pid(pidfile);

	}
	

	//-------Setup listen socket-----------
	listen_socket = setup_listen_socket(); // set up socket
	bind_tcp_socket(listen_socket, port); // bind and start listening
	
	//And set up the UDP socket
	udp_socket = setup_udp_socket();

	// add the listening socket to the master set
	fd_max = max(listen_socket, udp_socket); // biggest file descriptor
	Log(LOG_DAEMON|LOG_INFO, "Waiting for connection on port %i...\n", port);

	do { // loop on loop variable - restart input
		if (restart_config == 1) {
			Log(LOG_DAEMON|LOG_INFO,"Rereading configuration file upon user request\n");
			restart_config = 0;
			close_modules();
			do_configuration(0,0);
			if (!load_modules()) {
				Log(LOG_DAEMON|LOG_INFO,"Failed to load modules, aborting\n");
				return 1;
			}
		} 
		
		/* set up directory where we should store our blacklist stuff */
		char tmp[4096];
		snprintf(tmp,4096,"%s%s",basedir,blacklistdir);
		Log(LOG_DAEMON|LOG_INFO,"Saving blacklist info to '%s'\n", tmp);
		theList = new blacklist( tmp );

		//------- ---------------------------
		// keep rtclient from starting till someone connects
		// but if we already have clients, don't bother
		if (!new_client) {
			new_client = check_clients(&modptrs, true);
		}

		// reset the timers and packet objects
		kill_all();
		init_times();
		init_packets();

		if (terminate_bsod) {
			loop=0;
			break;
		}

		//------- Connect trace ----------
		trace = trace_create(uri);

		if (trace_is_err(trace))
		{
		    struct trace_err_t err;
		    err=trace_get_err(trace);
		    Log(LOG_DAEMON|LOG_ALERT, 
			    "Unable to connect to data source: %s\n",
			   	 err.problem);
		    exit(1);
		}

		Log(LOG_DAEMON|LOG_INFO,"Connected to data source: %s\n", uri);
		gettimeofday(&starttime, 0); // XXX

		//----- Check live status --------
		if ((!strncmp(uri,"erf:",4)) || (!strncmp(uri,"pcap:",5))) {
			// erf or pcap trace, slow down!
			Log(LOG_DAEMON|LOG_INFO,"Attempting to replay trace in real time\n");
			live = false;
		} else {
			live = true;
		}

		//------- Create filter -------------
		if (filter)
			trace_destroy_filter(filter);
		filter = 0;
		if (filterstring) {
			Log(LOG_DAEMON|LOG_INFO,"setting filter %s\n",filterstring);
			filter = trace_create_filter(filterstring);
			trace_config(trace,TRACE_OPTION_FILTER,filter);
		}

		if (trace_start(trace)==-1) {
			struct trace_err_t err;
			Log(LOG_DAEMON|LOG_ALERT, 
					"Unable to connect to data source: %s\n",
					err.problem);
			exit(1);
		}

		packet=trace_create_packet();

		while(loop) // loop on packets
		{
			if (!bsod_read_packet(packet, theList, rttmap))
				break;
		}

		if (trace_is_err(trace)) {
			struct trace_err_t err;
			Log(LOG_DAEMON|LOG_ALERT, 
				"trace_read_packet failure: %s\n",
					err.problem);
		} else {
			// expire any outstanding flows
			last_packet_time = trace_get_timeval(packet);
			expire_flows(last_packet_time.tv_sec+3600);
		}
		// We've finished with this trace
		trace_destroy(trace);
		trace = NULL;

		
		// the loop criteria is to loop if we want to loop always, or if
		// we get a USR1 we restart - the code path is the same.
	} while ((loop == 1) || (restart_config == 1));

	// if we actually get out of the outer loop, it's because we want to 
	// shut down entirely
	close(listen_socket);
	close(udp_socket);

	close_modules();
	delete rttmap;
	delete theList;
	Log(LOG_DAEMON|LOG_INFO,"Exiting...\n");

	return 0;
}

static int bsod_read_packet(libtrace_packet_t *packet, blacklist *theList,
		RTTMap *rttmap) {
	static time_t next_save = 0;
	static int packet_count = 0;
	
	struct timeval packettime;
	int psize = 0;
	struct client *new_client = NULL;

	// If we get a USR1, we want to restart. Break out of
	// this while() and go into cleanup
	if (restart_config == 1) {
		return 0;
	}

	if (terminate_bsod) {
		loop=0;
		return 0;
	}
	
	/* check for new clients */
	new_client = check_clients(&modptrs, false);
	if(new_client)
		send_flows(new_client);

	/* get a packet, and process it */
	if((psize = trace_read_packet(trace, packet)) <= 0) {
	    return 0;
	}

	++packet_count;

	// If we're sampling packets, skip packets that
	// don't meet our sampling criteria
	if (sampling && packet_count % sampling != 0)
		return 1;

	packettime = trace_get_timeval(packet);

	/* this time checking only matters when reading from a
	 * prerecorded trace file. It limits it to a seconds
	 * worth of data a second, which is still a large
	 * chunk
	 */ 
		
	if(!live)
	{
		offline_delay(packettime);
	}

	
	// if sending fails, assume we just lost a client
	if(per_packet(packet, packettime.tv_sec, &modptrs, rttmap, theList)!=0) {
		return 1;
	}

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

void set_defaults() {
	uri = 0;
	filterstring = 0;
	colourmod = 0;
	rightpos = 0;
	leftpos = 0;
	dirmod = 0;
	basedir = 0;
	loop = 0;
	shownondata = 0;
	showdata = 1;
	showcontrol = 1;
	sampling = 0;
}

void fix_defaults() {
	if (!basedir)
		basedir=strdup("/usr/local/bsod/");
	if (!colourmod)
		colourmod=strdup("plugins/colour/colours.so");
	if (!leftpos)
		leftpos=strdup("plugins/position/network16.so");
	if (!rightpos)
		rightpos=strdup("plugins/position/radial.so");
	if (!dirmod)
		dirmod=strdup("plugins/direction/interface.so");
	if (!pidfile)
		pidfile=strdup("/var/run/bsod_server.pid");
	if (!macaddrfile)
		macaddrfile=strdup("etc/mac_addrs");
	if (!blacklistdir)
		blacklistdir=strdup("blist/");
	if (!server_name){
		if(uri)	server_name=strdup(uri);
		else server_name=strdup("Unnamed");
	}
		
	
	//left_image and right_image can be NULL, it means the client will use the
	//default images and provides compatibility for the old client if needed
}

void do_configuration(int argc, char **argv) {
	int opt;

	// void everything
	set_defaults();
	
	cfg_opt_t opts[] =	{
		CFG_STR((char *)"pidfile", (char *)"", CFGF_NONE),
		CFG_INT((char *)"background", 0, CFGF_NONE),
		CFG_STR((char *)"basedir", NULL, CFGF_NONE),
		CFG_STR((char *)"source", NULL, CFGF_NONE),
		CFG_INT((char *)"listenport", 34567, CFGF_NONE),
		CFG_STR((char *)"filter", NULL, CFGF_NONE),
		CFG_STR((char *)"colour_module", NULL, CFGF_NONE),
		CFG_STR((char *)"rpos_module", NULL, CFGF_NONE),
		CFG_STR((char *)"lpos_module", NULL, CFGF_NONE),
		CFG_STR((char *)"dir_module", NULL, CFGF_NONE),
		CFG_INT((char *)"loop", 0, CFGF_NONE),
		CFG_INT((char *)"shownondata", 0, CFGF_NONE),
		CFG_INT((char *)"showdata", 1, CFGF_NONE),
		CFG_INT((char *)"showcontrol", 1, CFGF_NONE),
		CFG_STR((char *)"macaddrfile", NULL, CFGF_NONE),
		CFG_STR((char *)"blacklistdir", NULL, CFGF_NONE),
		CFG_BOOL((char *)"darknet", cfg_false, CFGF_NONE),
		CFG_BOOL((char *)"rttest", cfg_false, CFGF_NONE),
		CFG_INT((char *)"sampling", 0, CFGF_NONE),
		CFG_STR((char *)"name", NULL, CFGF_NONE),
		CFG_STR((char *)"left_image", NULL, CFGF_NONE),
		CFG_STR((char *)"right_image", NULL, CFGF_NONE),
		CFG_INT((char *)"sendq", 10*1024*1024, CFGF_NONE),
		CFG_END()
	};
	cfg_t *cfg;

	cfg = cfg_init(opts, CFGF_NONE);
	
	// read cmdline opts
	while( argv && (opt = getopt(argc, argv, "hbC:")) != -1)
	{
		switch(opt)
		{
			case 'b':
				background = 1;
				break;
			case 'h':
				do_usage(argv[0]);
				break;
			case 'C':
				configfile = optarg;
				break;
			default: do_usage(argv[0]);
		};
	}

	// parse configfile opts
	if (configfile) {
		
		if(cfg_parse(cfg, configfile) == CFG_PARSE_ERROR){		
			Log(LOG_DAEMON|LOG_ALERT,"Bad config file %s, giving up\n",
					configfile);
			exit(1);
		}
		
		pidfile = cfg_getstr(cfg, "pidfile");
		background = cfg_getint(cfg, "background");
		basedir = cfg_getstr(cfg, "basedir");
		uri = cfg_getstr(cfg, "source");
		port = cfg_getint(cfg, "listenport");
		filterstring = cfg_getstr(cfg, "filter");
		colourmod = cfg_getstr(cfg, "colour_module");
		rightpos = cfg_getstr(cfg, "rpos_module");
		leftpos = cfg_getstr(cfg, "lpos_module");
		dirmod = cfg_getstr(cfg, "dir_module");
		loop = cfg_getint(cfg, "loop");
		shownondata = cfg_getint(cfg, "shownondata");
		showdata = cfg_getint(cfg, "showdata");
		showcontrol = cfg_getint(cfg, "showcontrol");
		macaddrfile = cfg_getstr(cfg, "macaddrfile");
		blacklistdir = cfg_getstr(cfg, "blacklistdir");
		enable_darknet = cfg_getbool(cfg, "darknet");
		enable_rttest = cfg_getbool(cfg, "rttest");
		sampling = cfg_getint(cfg, "sampling");
		server_name = cfg_getstr(cfg, "name");
		left_image = cfg_getstr(cfg, "left_image");
		right_image = cfg_getstr(cfg, "right_image");
		max_sendq_size = cfg_getint(cfg, "sendq");
		
		printf("URI: %s\n", uri);
		
	}
	// if any options were omitted from the config file,
	// set them here
	fix_defaults();
	Log(LOG_DAEMON|LOG_DEBUG,"Darknet: %s\n",enable_darknet ? "Yes" : "No");
	Log(LOG_DAEMON|LOG_DEBUG,"RTTEst: %s\n",enable_rttest ? "Yes" : "No");
}

/** Parse out the arguments and the driver name 
 * @note driver/args are allocated on the heap and should be free()'d
 * later
 */
static void parse_args(const char *line, char **driver, char **args)
{
	const char *tok;
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
static void *get_module(const char *name)
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

	initfuncfptr init_func = (initfuncfptr)dlsym(handle,"init_module");

	if (init_func) {
		Log(LOG_DAEMON|LOG_DEBUG," Initialising module %s...\n",tmp);
		if (!init_func(args)) {
			Log(LOG_DAEMON|LOG_ALERT,
			     "Initialisation function failed for %s\n",driver);
			dlclose(handle);
			handle = NULL;
		}
	}
	else {
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
	
	colourhandle = get_module(colourmod);
	if (!colourhandle) {
		return 0;
	}

	modptrs.colour = (colfptr) dlsym(colourhandle, "mod_get_colour");
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


	dirhandle = get_module(dirmod);
	if (!dirhandle) {
		Log(LOG_DAEMON|LOG_ALERT,"Couldn't load module %s\n",tmp);
		return 0;
	}

	modptrs.direction = (dirfptr) dlsym(dirhandle,"mod_get_direction");
	if ((error = (char*)dlerror()) != NULL) {
		Log(LOG_DAEMON|LOG_ALERT,"%s\n",error);
		return 0;
	}

	return 1;
}



static void close_modules() {
	if(colourhandle) {
		endfptr end_module = (endfptr)dlsym(colourhandle,"end_module");
		if (end_module) {
			end_module();
		}
		dlclose(colourhandle);
	}
	modptrs.colour = 0;
	if(lefthandle) {
		endsidefptr end_position_module =
			(endsidefptr)dlsym(lefthandle,"end_module");
		if (end_position_module)
			end_position_module(SIDE_LEFT);
		dlclose(lefthandle);
	}
	modptrs.left = 0;
	if(righthandle) {
		endsidefptr end_position_module =
			(endsidefptr)dlsym(righthandle,"end_module");
		if (end_position_module)
			end_position_module(SIDE_RIGHT);
		dlclose(righthandle);
	}
	modptrs.right = 0;
	if(dirhandle) {
		endfptr end_module = (endfptr)dlsym(dirhandle,"end_module");
		if (end_module) {
			end_module();
		}
		dlclose(dirhandle);
	}
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
	ts32 = 0;
	startts32 = 0;

}


static void offline_delay(struct timeval tv){
	struct timeval diff;
	struct timeval delta;

	if(startts32 == 0)
		startts32 = tv.tv_sec;
	if(tracetime.tv_sec == 0)
		tracetime = tv;

	ts32 = tv.tv_sec;
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
