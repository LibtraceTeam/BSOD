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
#include <libwandevent.h>

#include "bsod_server.h"

#include "socket.h"
#include "packets.h"
#include "daemons.h"
#include "debug.h"

#include "RTTMap.h"
#include "Blacklist.h"

typedef struct ip ip_t;

int  _fcs_bits = 32;
int restart_config = 1;
int terminate_bsod = 0;

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
char *dirparam = 0;
char *colourparam = 0;
char *leftposparam = 0;
char *rightposparam = 0;
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

void do_configuration(int argc, char **argv);

void *colourhandle = 0;
void *lefthandle = 0;
void *righthandle = 0;
void *dirhandle = 0;

struct modptrs_t modptrs;

wand_event_handler_t *wand_ev_hdl = NULL;

struct wand_signal_t sigterm_event;
struct wand_signal_t sigusr_event;
struct wand_signal_t sigpipe_event;
struct wand_signal_t sigint_event;

struct wand_fdcb_t listener;
struct wand_fdcb_t udpsocket;

struct wand_fdcb_t file_event;
struct wand_timer_t sleep_event;

struct wand_timer_t signal_timer;

void do_usage(char* name)
{
    printf("Usage: %s [-h] [-b] -C <configfile> \n", name);
    exit(0);
}

typedef struct bsod_vars {
	RTTMap *rttmap;
	blacklist *blist;
	libtrace_packet_t *packet;
} bsod_vars_t;


static int load_modules();
static void close_modules();
static void init_times();
static void init_signals();
static int bsod_read_packet(libtrace_packet_t *packet, blacklist *theList,
		RTTMap *rttmap);
static void bsod_event(bsod_vars_t *vars);
static void set_signal_timer(bsod_vars_t *vars, bool remove);
static void signal_event(bsod_vars_t *vars) ;

bsod_vars_t bsod_vars;

int main(int argc, char *argv[])
{
	// RTTMap:
	RTTMap *rttmap = NULL;
    
	// Blacklist:
	blacklist *theList = NULL;
	

	int one=1;
	struct client *new_client = NULL;

	struct timeval last_packet_time;

	// rt stuff
	static struct libtrace_filter_t *filter = 0;

	wand_event_init();
	wand_ev_hdl = wand_create_event_handler();
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
	//do_configuration(0,0);
	restart_config = 0;
	if (!load_modules()) {
		Log(LOG_DAEMON|LOG_INFO,"Failed to load modules, aborting\n");
		return 1;
	}

	setup_listen_socket(wand_ev_hdl, &modptrs, &listener, port);
	setup_udp_socket(wand_ev_hdl, &udpsocket);	

	Log(LOG_DAEMON|LOG_INFO, "Waiting for connection on port %i...\n", port);
	sleep_event.callback = NULL;
	file_event.fd = -1;
	if (enable_rttest)
		rttmap = new RTTMap();

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
		if (enable_darknet) {
			if (theList)
				delete(theList);
			theList = new blacklist( tmp );
		}
		if (enable_rttest) {
			if (rttmap)
				delete(rttmap);
			rttmap = new RTTMap();
		}

		bsod_vars.blist = theList;
		bsod_vars.rttmap = rttmap;
		bsod_vars.packet = trace_create_packet();

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


		//------- Create filter -------------
		if (filter)
			trace_destroy_filter(filter);
		filter = NULL;
		if (filterstring) {
			Log(LOG_DAEMON|LOG_INFO,"setting filter %s\n",filterstring);
			filter = trace_create_filter(filterstring);
			trace_config(trace,TRACE_OPTION_FILTER,filter);
		}
		//trace_config(trace, TRACE_OPTION_EVENT_REALTIME, &one);
		if (trace_start(trace)==-1) {
			struct trace_err_t err;
		    	err=trace_get_err(trace);
			Log(LOG_DAEMON|LOG_ALERT, 
					"Unable to connect to data source: %s\n",
					err.problem);
			exit(1);
		}

		Log(LOG_DAEMON|LOG_INFO,"Connected to data source: %s\n", uri);
		gettimeofday(&starttime, 0); // XXX

		bsod_event(&bsod_vars);
		wand_ev_hdl->running = true;
		wand_event_run(wand_ev_hdl);

		/*
		while(loop) // loop on packets
		{
			if (!bsod_read_packet(packet, theList, rttmap))
				break;
		}
		*/

		if (trace_is_err(trace)) {
			struct trace_err_t err;
			Log(LOG_DAEMON|LOG_ALERT, 
				"trace_read_packet failure: %s\n",
					err.problem);
		} else {
			// expire any outstanding flows
			//last_packet_time = trace_get_timeval(bsod_vars.packet);
			expire_flows(0, true);
		}
		// We've finished with this trace
		if (file_event.fd != -1)
			wand_del_event(wand_ev_hdl, &file_event);
		if (sleep_event.callback != NULL)
			wand_del_timer(wand_ev_hdl, &sleep_event);
		trace_destroy(trace);
		trace = NULL;
		sleep_event.callback = NULL;
		file_event.fd = -1;
		
		
		// the loop criteria is to loop if we want to loop always, or if
		// we get a USR1 we restart - the code path is the same.
	} while ((loop == 1) || (restart_config == 1));

	// if we actually get out of the outer loop, it's because we want to 
	// shut down entirely

	close_modules();
	if (rttmap)
		delete rttmap;
	if (theList)
		delete theList;
	Log(LOG_DAEMON|LOG_INFO,"Exiting...\n");

	return 0;
}

static void file_cb(struct wand_fdcb_t *evcb, enum wand_eventtype_t ev) {
	
	wand_del_event(wand_ev_hdl, evcb);
	assert(ev == EV_READ);
	evcb->fd = -1;
	wand_del_timer(wand_ev_hdl, &signal_timer);
	bsod_event((bsod_vars_t *)evcb->data);

}

static void sleep_cb(struct wand_timer_t *timer) {
	timer->callback = NULL;
	wand_del_timer(wand_ev_hdl, &signal_timer);
	bsod_event((bsod_vars_t *)timer->data);
}

static void signal_timer_cb(struct wand_timer_t *timer) {
	timer->callback = NULL;
	signal_event((bsod_vars_t *)timer->data);
}

static void set_signal_timer(bsod_vars_t *vars) {
	
	signal_timer.expire = wand_calc_expire(wand_ev_hdl, 1, 0);
	signal_timer.callback = signal_timer_cb;
	signal_timer.data = vars;
	signal_timer.next = signal_timer.prev = NULL;

	wand_add_timer(wand_ev_hdl, &signal_timer);
}


static int process_bsod_event(bsod_vars_t *vars, libtrace_eventobj_t event) {
	switch(event.type) {
		case TRACE_EVENT_IOWAIT:
			file_event.fd = event.fd;
			file_event.flags = EV_READ;
			file_event.data = vars;
			file_event.callback = file_cb;
			wand_add_event(wand_ev_hdl, &file_event);
			return 0;
		case TRACE_EVENT_SLEEP:
			int micros;
		 	micros = (int)((event.seconds - (int)event.seconds) * 1000000.0);
			sleep_event.expire = wand_calc_expire(wand_ev_hdl, 
			 		(int)event.seconds, micros);
			sleep_event.callback = sleep_cb;
			sleep_event.data = vars;
			sleep_event.prev = sleep_event.next = NULL;
			wand_add_timer(wand_ev_hdl, &sleep_event);
			return 0;
		case TRACE_EVENT_PACKET:
			if (event.size == -1) {
				wand_ev_hdl->running = false;
				return 0;
			}
			if (bsod_read_packet(vars->packet, vars->blist, 
					vars->rttmap) == 0) {
				wand_ev_hdl->running = false;
				return 0;
			}
			return 1;
		case TRACE_EVENT_TERMINATE:
			Log(LOG_DEBUG, "Terminating trace\n");
			wand_ev_hdl->running = false;
			return 0;
		default:
			Log(LOG_DAEMON | LOG_DEBUG, 
					"Unknown libtrace event type: %d\n", 
					event.type);
			return 0;
	}
}

static void signal_event(bsod_vars_t *vars) {
	// If we get a USR1, we want to restart. Break out and restart
	if (restart_config == 1) {
		wand_ev_hdl->running = false;
		return;
	}

	if (terminate_bsod) {
		loop=0;
		wand_ev_hdl->running = false;
		return;
	}
	/* Make sure we check for a signal every second, just in case we
	 * have no other events for a while */
	set_signal_timer(vars);
}			

static void bsod_event(bsod_vars_t *vars) {

	struct libtrace_eventobj_t event;
	int poll_again = 1;

	// If we get a USR1, we want to restart. Break out and restart
	if (restart_config == 1) {
		wand_ev_hdl->running = false;
		return;
	}

	if (terminate_bsod) {
		loop=0;
		wand_ev_hdl->running = false;
		return;
	}
	
	do {
		if (!vars->packet)
			vars->packet = trace_create_packet();
		event = trace_event(trace, vars->packet);
		poll_again = process_bsod_event(vars, event);
	} while (poll_again);

	/* Make sure we check for a signal every second, just in case we
	 * have no other events for a while */
	set_signal_timer(vars);

}

static int bsod_read_packet(libtrace_packet_t *packet, blacklist *theList,
		RTTMap *rttmap) {
	static int packet_count = 0;
	static int next_save = 0;
	
	struct timeval packettime;

	/* check for new clients */
	/*
	new_client = check_clients(&modptrs, false);
	if(new_client)
		send_flows(new_client);
	*/

	++packet_count;

	// If we're sampling packets, skip packets that
	// don't meet our sampling criteria
	if (sampling && packet_count % sampling != 0)
		return 1;

	packettime = trace_get_timeval(packet);

	// if sending fails, assume we just lost a client
	if(per_packet(packet, packettime.tv_sec, &modptrs, rttmap, theList)!=0) {
		return 1;
	}

	if (enable_darknet && packettime.tv_sec > next_save) {
		/* Save the blacklist every 5 min */
		next_save = packettime.tv_sec + 300;
		theList->save();
	}
	return 1;
}



static void sigterm_cb(struct wand_signal_t *signal) {
	terminate_bsod = 1;
}

static void sigusr_cb(struct wand_signal_t *signal) {
	restart_config = 1;
}

static void sigpipe_cb(struct wand_signal_t *signal) {
	return;
}


static void init_signals() {
	// setup signal handlers
	
	sigterm_event.signum = SIGTERM;
	sigterm_event.callback = sigterm_cb;
	sigterm_event.data = NULL;
	wand_add_signal(&sigterm_event);
	
	sigint_event.signum = SIGINT;
	sigint_event.callback = sigterm_cb;
	sigint_event.data = NULL;
	wand_add_signal(&sigint_event);

	sigusr_event.signum = SIGUSR1;
	sigusr_event.callback = sigusr_cb;
	sigusr_event.data = NULL;
	wand_add_signal(&sigusr_event);

	sigpipe_event.signum = SIGPIPE;
	sigpipe_event.callback = sigpipe_cb;
	sigpipe_event.data = NULL;
	wand_add_signal(&sigpipe_event);
	

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
		CFG_STR((char *)"dirparam", NULL, CFGF_NONE),
		CFG_STR((char *)"colourparam", NULL, CFGF_NONE),
		CFG_STR((char *)"lpos_param", NULL, CFGF_NONE),
		CFG_STR((char *)"rpos_param", NULL, CFGF_NONE),
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
		dirparam = cfg_getstr(cfg, "dirparam");
		colourparam = cfg_getstr(cfg, "colourparam");
		leftposparam = cfg_getstr(cfg, "lpos_param");
		rightposparam = cfg_getstr(cfg, "rpos_param");
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
static void *get_module(const char *name, char *param)
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
		if (!init_func(param)) {
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
static void *get_position_module(side_t side, const char *name, char *posparam)
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
		if (!init_func(posparam)) {
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
	
	colourhandle = get_module(colourmod, colourparam);
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

	lefthandle = get_position_module(SIDE_LEFT, leftpos, leftposparam);
	if (!lefthandle) {
		return 0;
	}
	modptrs.left = (posfptr) dlsym(lefthandle, "mod_get_position");
	if ((error = (char*)dlerror()) != NULL) {
		Log(LOG_DAEMON|LOG_ALERT,"%s\n",error);
		return 0;
	}

	righthandle = get_position_module(SIDE_RIGHT,rightpos, rightposparam);
	if (!righthandle) {
		return 0;
	}
	modptrs.right = (posfptr) dlsym(righthandle,"mod_get_position");
	if ((error = (char*)dlerror()) != NULL) {
		Log(LOG_DAEMON|LOG_ALERT,"%s\n",error);
		return 0;
	}


	dirhandle = get_module(dirmod, dirparam);
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


