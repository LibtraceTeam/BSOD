/*
 * main loop(s) and the signal handlers may need looking at?
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <signal.h>
#include <setjmp.h>
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

typedef struct ip ip_t;

struct sigaction sigact;
static void sig_hnd(int signo);
static jmp_buf  jmpbuf;

int  _fcs_bits = 32;
int restart_config = 1;
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
char *configfile = "/usr/local/bsod/etc/bsod_server.conf";
static char* uri = 0; 

int port = 32500;
int loop = 0;
int shownondata = 0;
int showdata = 1;
int showcontrol = 1;

static void sigusr_hnd(int sig);
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
    
	// socket stuff
	int listen_socket;
	fd_set listen_set, event_set;
	int new_client = 0;

	bool live = true;


	struct timeval packettime;

	int psize = 0;

	// rt stuff
	static struct libtrace_filter_t *filter = 0;
	struct libtrace_packet_t packet;
	uint64_t ts;



	FD_ZERO(&listen_set);
	FD_ZERO(&event_set);


	// setup signal handlers
	sigact.sa_handler = sigusr_hnd;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;
	if(sigaction(SIGUSR1, &sigact, NULL) < 0) {
		printf("sigaction SIGUSR1: %d\n",errno);
	}
	
	sigact.sa_handler = sig_hnd;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;

	if(sigaction(SIGPIPE, &sigact, NULL) < 0) {
		perror("sigaction");
		exit(-1);
	}

	if (setjmp(jmpbuf))
		goto goodbye;

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

		//------- ---------------------------
		// keep rtclient from starting till someone connects
		// but if we already have clients, don't bother
		if (new_client == 0) {
			new_client = check_clients(true);
		}

		// reset the timers and packet objects
		init_times();
		init_packets();

		//------- Connect trace ----------
		trace = trace_create(uri);
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
			
			/* check for new clients */
			new_client = check_clients(false);
			if(new_client > 0)// is zero valid?
				send_flows(new_client);

			/* get a packet, and process it */
			if((psize = trace_read_packet(trace, &packet)) <= 0) {
				perror("libtrace_read_packet");
				break;
			}

			// if we have a filter, and if the filter doesn't match, 
			// continue the inner while() loop
			if (filter)  
				if (!trace_bpf_filter(filter,&packet)) 
					continue;

			ts = trace_get_erf_timestamp(&packet);
			packettime = trace_get_timeval(&packet);
		
			
			/* this time checking only matters when reading from a prerecorded
			 * trace file. It limits it to a seconds worth of data a second,
			 * which is still a large chunk*/
				
			if(!live)
			{
				offline_delay(packettime);
			}
			
			// if sending fails, assume we just lost a client
			if(per_packet(packet, ts, &modptrs, rttmap) != 0)
				continue;
		}
		// We've finished with this trace
		trace_destroy(trace);
		trace = 0;

		// expire any outstanding flows
		expire_flows(ts32+10);
		
		// the loop criteria is to loop if we want to loop always, or if
		// we get a USR1 we restart - the code path is the same.
	} while ((loop == 1) || (restart_config == 1));

	// if we actually get out of the outer loop, it's because we want to 
	// shut down entirely
goodbye:
	close(listen_socket);

	close_modules();
	delete rttmap;
	Log(LOG_DAEMON|LOG_INFO,"Exiting...\n");
	exit(0);

}


void sig_hnd( int signo )
{
    longjmp(jmpbuf, 1);
}

static void sigusr_hnd(int signo) {
	restart_config = 1;
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
		macaddrfile=strdup("/usr/local/bsod/etc/mac_addrs");

	
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
}


static void load_modules() {
	char *error = 0;
	char tmp[4096];

	//------- Load up modules -----------
	
	snprintf(tmp,4096,"%s%s",basedir,colourmod);
	colourhandle = dlopen(tmp,RTLD_LAZY);
	if (!colourhandle) {
		Log(LOG_DAEMON|LOG_ALERT,"Couldn't load module %s\n",tmp);
		assert(colourhandle);
	}

	modptrs.colour = (colfptr) dlsym(colourhandle, "mod_get_colour");
	if ((error = dlerror()) != NULL) {
		Log(LOG_DAEMON|LOG_ALERT,"%s\n",error);
		assert(modptrs.colour);
	}


	
	snprintf(tmp,4096,"%s%s",basedir,leftpos);
	lefthandle = dlopen(tmp,RTLD_LAZY);
	if (!lefthandle) {
		Log(LOG_DAEMON|LOG_ALERT,"Couldn't load module %s\n",tmp);
		assert(lefthandle);
	}

	modptrs.left = (posfptr) dlsym(lefthandle, "mod_get_position");
	if ((error = dlerror()) != NULL) {
		Log(LOG_DAEMON|LOG_ALERT,"%s\n",error);
		assert(modptrs.left);
	}


	
	snprintf(tmp,4096,"%s%s",basedir,rightpos);
	righthandle = dlopen(tmp,RTLD_LAZY);
	if (!righthandle) {
		Log(LOG_DAEMON|LOG_ALERT,"Couldn't load module %s\n",tmp);
		assert(righthandle);
	}
	
	modptrs.right = (posfptr) dlsym(righthandle,"mod_get_position");
	if ((error = dlerror()) != NULL) {
		Log(LOG_DAEMON|LOG_ALERT,"%s\n",error);
		assert(modptrs.right);
	}


	
	snprintf(tmp,4096,"%s%s",basedir,dirmod);
	dirhandle = dlopen(tmp,RTLD_LAZY);
	if (!dirhandle) {
		Log(LOG_DAEMON|LOG_ALERT,"Couldn't load module %s\n",tmp);
		assert(dirhandle);
	}

	modptrs.init_dir = (initdirfptr) dlsym(dirhandle,"mod_init_dir");
	if ((error = dlerror()) != NULL) {
		Log(LOG_DAEMON|LOG_ALERT,"%s\n",error);
		assert(modptrs.init_dir);
	}

	/* 
	 * initialise the mac addresses we are looking for.
	 * would be nice to use _init here, but there's no easy way
	 * to pass it the information it needs 
	 */
	modptrs.init_dir(macaddrfile);

	modptrs.direction = (dirfptr) dlsym(dirhandle,"mod_get_direction");
	if ((error = dlerror()) != NULL) {
		Log(LOG_DAEMON|LOG_ALERT,"%s\n",error);
		assert(modptrs.direction);
	}

}



static void close_modules() {
	if(colourhandle) {
		dlclose(colourhandle);
	}
	modptrs.colour = 0;
	if(lefthandle) {
		dlclose(lefthandle);
	}
	modptrs.left = 0;
	if(righthandle) {
		dlclose(righthandle);
	}
	modptrs.right = 0;
	if(dirhandle) {
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
