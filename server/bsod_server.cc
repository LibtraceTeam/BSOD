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

#include <libtrace.h>

#include "lib/config.h"
#include "bsod_server.h"


#include "dagformat.h"

#include "socket.h"
#include "packets.h"


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

char *filterstring = 0;
char *colourmod = 0;
char *leftpos = 0;
char *rightpos = 0;
char *dirmod = 0;
char *configfile = "./bsod_server.config";
static char* uri = 0; // default is nothing
int port = 32500;
int loop = 0;
int shownondata = 0;
int showdata = 1;

static void sigusr_hnd(int sig);
void do_configuration(int argc, char **argv);

void *colourhandle = 0;
void *lefthandle = 0;
void *righthandle = 0;
void *dirhandle = 0;

struct modptrs_t modptrs;

void do_usage(char* name)
{
    printf("Usage: %s [-h] -C <configfile> \n", name);
    exit(0);
}

static void load_modules();
static void close_modules();
static void init_times();
void offline_delay(struct timeval tv);

int main(int argc, char *argv[])
{
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
	//-------Setup listen socket-----------
	listen_socket = setup_listen_socket(); // set up socket
	bind_tcp_socket(listen_socket, port); // bind and start listening

	// add the listening socket to the master set
	fd_max = listen_socket; // biggest file descriptor
	printf("Waiting for connection on port %i...\n", port);




	do { // loop on loop variable - restart input
		if (restart_config == 1) {
			restart_config = 0;
			close_modules();
			do_configuration(argc,argv);
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
		printf("Connected to data source: %s\n", uri);
		gettimeofday(&starttime, 0); // XXX

		//----- Check live status --------
		if ((!strncmp(uri,"erf:",4)) || (!strncmp(uri,"pcap:",5))) {
			// erf or pcap trace, slow down!
			printf("Attempting to replay trace in real time\n");
			live = false;
		} else {
			live = true;
		}

		//------- Create filter -------------
		if (filter)
			free(filter);
		filter = 0;
		if (filterstring) {
			printf("setting filter %s\n",filterstring);
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
			if(per_packet(packet, ts, &modptrs) != 0)
				break;
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
	printf("Exiting...\n");
	exit(0);

}


void sig_hnd( int signo )
{
    longjmp(jmpbuf, 1);
}

static void sigusr_hnd(int signo) {
	printf("siguser handler\n");
	restart_config = 1;
}

void set_defaults() {
	uri = 0;
	filterstring = 0;
	colourmod = 0;
	rightpos = 0;
	leftpos = 0;
	dirmod = 0;
	loop = 0;
	shownondata = 0;
	showdata = 1;
}

void do_configuration(int argc, char **argv) {
	int opt;

	set_defaults();

	config_t main_config[] = {
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
		{0,0,0}

	};
	while( (opt = getopt(argc, argv, "hs:p:f:c:l:r:d:LC:")) != -1)
	{
		switch(opt)
		{
			case 'h':
				do_usage(argv[0]);
				break;
			case 'C':
				configfile = optarg;
				break;
			default: do_usage(argv[0]);
		};
	}

	if (configfile) {
		if (parse_config(main_config,configfile)) {
			fprintf(stderr,"Bad config file %s, giving up\n",
					configfile);
			exit(1);
		}
	}
}


static void load_modules() {
	char *error = 0;

	//------- Load up modules -----------
	colourhandle = dlopen(colourmod,RTLD_LAZY);
	assert(colourhandle);

	modptrs.colour = (colfptr) dlsym(colourhandle, "mod_get_colour");
	if ((error = dlerror()) != NULL) {
		printf("%s\n",error);
		assert(modptrs.colour);
	}

	lefthandle = dlopen(leftpos,RTLD_LAZY);
	assert(lefthandle);
	modptrs.left = (posfptr) dlsym(lefthandle, "mod_get_position");
	if ((error = dlerror()) != NULL) {
		printf("%s\n",error);
		assert(modptrs.left);
	}

	righthandle = dlopen(rightpos,RTLD_LAZY);
	assert(righthandle);
	modptrs.right = (posfptr) dlsym(righthandle,"mod_get_position");
	if ((error = dlerror()) != NULL) {
		printf("%s\n",error);
		assert(modptrs.right);
	}

	dirhandle = dlopen(dirmod,RTLD_LAZY);
	assert(dirhandle);
	modptrs.direction = (dirfptr) dlsym(dirhandle,"mod_get_direction");
	if ((error = dlerror()) != NULL) {
		printf("%s\n",error);
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


void offline_delay(struct timeval tv){
	uint32_t dagparts_hax = 0;
	uint64_t ts = 0;
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
/*
	// check that we are in the right second, and arent 
	// getting ahead of ourselves
	while( ((uint32_t)(nowtime.tv_sec - starttime.tv_sec) < (ts32 - startts32)))
	{
		usleep(100);	
		gettimeofday(&nowtime, 0);

	}


	//check that we are in the right part of the second and arent
	//getting ahead of ourselves. could go into more detail
	//here, but is not super important
      //if(ts & 0x0000000080000000)
	if(ts & 0x0000000080000000)
		dagparts_hax += 500000;
	if(ts & 0x0000000040000000)
		dagparts_hax += 250000;
	if(ts & 0x0000000020000000)
		dagparts_hax += 125000;

	while((uint32_t)(nowtime.tv_usec) < dagparts_hax)
	{
		usleep(100);
		
		gettimeofday(&nowtime, 0);
	}

}
*/
