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

#include "lib/config.h"
#include "bsod_server.h"


#include "dagformat.h"

#include "socket.h"
#include "packets.h"
#include "daemons.h"
#include "debug.h"

#include "RTTMap.h"
#include "Blacklist.h"

extern "C"
char *strndup(const char *, size_t);

typedef struct ip ip_t;

struct sigaction sigact;

int  _fcs_bits = 32;
int restart_config = 1;
int terminate_bsod = 0;
int fd_max;

struct libtrace_t *trace = 0;

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
char *configfile = "/usr/local/bsod/etc/bsod_server.conf";
static char* uri = 0; 

int port = 32500;
int loop = 0;
int shownondata = 0;
int showdata = 1;
int showcontrol = 1;
int sampling = 0;

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

static void load_modules();
static void close_modules();
static void init_times();
static void offline_delay(struct timeval tv);

int main(int argc, char *argv[])
{
	// RTTMap:
	RTTMap *rttmap = new RTTMap();
    
	// Blacklist:
	blacklist *theList;
	
	// socket stuff
	int listen_socket;
	fd_set listen_set, event_set;
	int new_client = 0;

	bool live = true;


	struct timeval packettime;

	int psize = 0;
	int packet_count = 0;

	// rt stuff
	static struct libtrace_filter_t *filter = 0;
	struct libtrace_packet_t packet;
	time_t next_save = 0;
	time_t lastts = 0;



	FD_ZERO(&listen_set);
	FD_ZERO(&event_set);


	// setup signal handlers
	sigact.sa_handler = sigusr_hnd;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;
	if(sigaction(SIGUSR1, &sigact, NULL) < 0) {
		printf("sigaction SIGUSR1: %d\n",errno);
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

	// add the listening socket to the master set
	fd_max = listen_socket; // biggest file descriptor
	Log(LOG_DAEMON|LOG_INFO, "Waiting for connection on port %i...\n", port);




	do { // loop on loop variable - restart input
		if (restart_config == 1) {
			Log(LOG_DAEMON|LOG_INFO,"Rereading configuration file upon user request\n");
			restart_config = 0;
			close_modules();
			do_configuration(0,0);
			load_modules();
		} 
		
		/* set up directory where we should store our blacklist stuff */
		char tmp[4096];
		snprintf(tmp,4096,"%s%s",basedir,blacklistdir);
		Log(LOG_DAEMON|LOG_INFO,"Saving blacklist info to '%s'\n", tmp);
		theList = new blacklist( tmp );

		//------- ---------------------------
		// keep rtclient from starting till someone connects
		// but if we already have clients, don't bother
		if (new_client == 0) {
			new_client = check_clients(&modptrs, true);
		}

		// reset the timers and packet objects
		kill_all();
		init_times();
		init_packets();

		//------- Connect trace ----------
		if( (trace = trace_create(uri)) == NULL)
		{
		    Log(LOG_DAEMON|LOG_ALERT, 
			    "Unable to connect to data source: %s\n", uri);
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
			free(filter);
		filter = 0;
		if (filterstring) {
			Log(LOG_DAEMON|LOG_INFO,"setting filter %s\n",filterstring);
			filter = trace_bpf_setfilter(filterstring);
		}

		while(1) // loop on packets
		{
			// If we get a USR1, we want to restart. Break out of this while()
			// and go into cleanup
			if (restart_config == 1) {
				break;
			}

			if (terminate_bsod)
				break;
			
			/* check for new clients */
			new_client = check_clients(&modptrs, false);
			if(new_client > 0)// is zero valid?
				send_flows(new_client);

			/* get a packet, and process it */
			if((psize = trace_read_packet(trace, &packet)) <= 0) {
			    if (psize < 0) {
				perror("libtrace_read_packet");
			    }
			    break;
			}

			++packet_count;

			// If we're sampling packets, skip packets that
			// don't meet our sampling criteria
			if (sampling && packet_count % sampling != 0)
				continue;

			// if we have a filter, and if the filter doesn't match, 
			// continue the inner while() loop
			if (filter)  
				if (!trace_bpf_filter(filter,&packet)) 
					continue;

			packettime = trace_get_timeval(&packet);
		
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
			if(per_packet(&packet, packettime.tv_sec, &modptrs, rttmap, theList)!=0)
				continue;

			if (packettime.tv_sec > next_save) {
				/* Save the blacklist every 5 min */
				next_save = packettime.tv_sec + 300;
				theList->save();
			}

			/* Every second update the current time */
			if (packettime.tv_sec - lastts >= 1) {
				printf("%s\r",asctime(gmtime((time_t*)&packettime.tv_sec)));
				lastts=packettime.tv_sec;
			}
		}
		// We've finished with this trace
		trace_destroy(trace);
		trace = 0;

		// expire any outstanding flows
		expire_flows(packettime.tv_sec+3600);
		
		// the loop criteria is to loop if we want to loop always, or if
		// we get a USR1 we restart - the code path is the same.
	} while ((loop == 1) || (restart_config == 1));

	// if we actually get out of the outer loop, it's because we want to 
	// shut down entirely
	close(listen_socket);

	close_modules();
	delete rttmap;
	delete theList;
	Log(LOG_DAEMON|LOG_INFO,"Exiting...\n");
	exit(0);

}


static void sigusr_hnd(int signo) {
	restart_config = 1;
}

static void sigterm_hnd(int signo) {
	terminate_bsod = 1;
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

	
}

void do_configuration(int argc, char **argv) {
	int opt;

	// void everything
	set_defaults();

	// initialise config parser
	config_t main_config[] = {
		{"pidfile", TYPE_STR|TYPE_NULL, &pidfile},
		{"background", TYPE_INT|TYPE_NULL, &background},
		{"basedir", TYPE_STR|TYPE_NULL, &basedir},
		{"source", TYPE_STR|TYPE_NULL, &uri},
		{"listenport", TYPE_INT|TYPE_NULL, &port},
		{"filter", TYPE_STR|TYPE_NULL, &filterstring},
		{"colour_module",TYPE_STR|TYPE_NULL, &colourmod},
		{"rpos_module",TYPE_STR|TYPE_NULL, &rightpos},
		{"lpos_module",TYPE_STR|TYPE_NULL, &leftpos},
		{"dir_module",TYPE_STR|TYPE_NULL, &dirmod},
		{"loop",TYPE_INT|TYPE_NULL, &loop},
		{"shownondata", TYPE_INT|TYPE_NULL, &shownondata},
		{"showdata", TYPE_INT|TYPE_NULL, &showdata},
		{"showcontrol", TYPE_INT|TYPE_NULL, &showcontrol},
		{"macaddrfile", TYPE_STR|TYPE_NULL, &macaddrfile},
		{"blacklistdir", TYPE_STR|TYPE_NULL, &blacklistdir},
		{"darknet", TYPE_BOOL|TYPE_NULL, &enable_darknet},
		{"rttest", TYPE_BOOL|TYPE_NULL, &enable_rttest},
		{"sampling", TYPE_INT|TYPE_NULL, &sampling},
		{0,0,0}
	};

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
		if (parse_config(main_config,configfile)) {
			Log(LOG_DAEMON|LOG_ALERT,"Bad config file %s, giving up\n",
					configfile);
			exit(1);
		}
	}
	// if any options were omitted from the config file,
	// set them here
	fix_defaults();
	printf("Darknet: %s\n",enable_darknet ? "Yes" : "No");
	printf("RTTEst: %s\n",enable_rttest ? "Yes" : "No");
}

/** Parse out the arguments and the driver name 
 * @note driver/args are allocated on the heap and should be free()'d
 * later
 */
static void parse_args(const char *line, char **driver, char **args)
{
	char *tok;
	tok = strchr(line,' ');
	if (!tok) {
		*driver = strdup(line);
		*args = strdup("");
		return;
	}

	*driver = strndup(line,tok-line);
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

	printf("Loading module %s...\n",tmp);

	void *handle = dlopen(tmp,RTLD_LAZY);
	if (!handle) {
		Log(LOG_DAEMON|LOG_ALERT,"Couldn't load module %s\n",driver);
		assert(0);
	}

	initfuncfptr init_func = (initfuncfptr)dlsym(handle,"init_module");

	if (init_func) {
		printf(" Initialising module %s...\n",tmp);
		if (!init_func(args)) {
			Log(LOG_DAEMON|LOG_ALERT,
			     "Initialisation function failed for %s\n",driver);
			dlclose(handle);
			handle = NULL;
		}
	}
	else {
		printf(" Not Initialising module %s\n",tmp);
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

	void *handle = dlopen(tmp,RTLD_LAZY);
	if (!handle) {
		Log(LOG_DAEMON|LOG_ALERT,"Couldn't load module %s\n",driver);
		assert(0);
	}

	initsidefptr init_func = (initsidefptr)dlsym(handle,"init_module");

	if (init_func) {
		if (!init_func(side,args)) {
			Log(LOG_DAEMON|LOG_ALERT,
			     "Initialisation function failed for %s\n",driver);
			dlclose(handle);
			handle = NULL;
		}
	}

	free(driver);
	free(args);

	return handle;
}

static void load_modules() {
	char *error = 0;
	char tmp[4096];

	//------- Load up modules -----------
	
	colourhandle = get_module(colourmod);

	modptrs.colour = (colfptr) dlsym(colourhandle, "mod_get_colour");
	if ((error = (char*)dlerror()) != NULL) {
		Log(LOG_DAEMON|LOG_ALERT,"%s\n",error);
		assert(modptrs.colour);
	}

	modptrs.info = (inffptr) dlsym(colourhandle, "mod_get_info");
	if ((error = (char*)dlerror()) != NULL) {
		Log(LOG_DAEMON|LOG_ALERT,"%s\n",error);
		assert(modptrs.info);
	}

	lefthandle = get_position_module(SIDE_LEFT, leftpos);
	modptrs.left = (posfptr) dlsym(lefthandle, "mod_get_position");
	if ((error = (char*)dlerror()) != NULL) {
		Log(LOG_DAEMON|LOG_ALERT,"%s\n",error);
		assert(modptrs.left);
	}

	righthandle = get_position_module(SIDE_RIGHT,rightpos);
	modptrs.right = (posfptr) dlsym(righthandle,"mod_get_position");
	if ((error = (char*)dlerror()) != NULL) {
		Log(LOG_DAEMON|LOG_ALERT,"%s\n",error);
		assert(modptrs.right);
	}


	dirhandle = get_module(dirmod);
	if (!dirhandle) {
		Log(LOG_DAEMON|LOG_ALERT,"Couldn't load module %s\n",tmp);
		assert(dirhandle);
	}

	modptrs.direction = (dirfptr) dlsym(dirhandle,"mod_get_direction");
	if ((error = (char*)dlerror()) != NULL) {
		Log(LOG_DAEMON|LOG_ALERT,"%s\n",error);
		assert(modptrs.direction);
	}

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
