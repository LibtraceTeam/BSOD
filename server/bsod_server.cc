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

#include <assert.h>

#include "libtrace.h"

#include "bsod_server.h"


#include "dagformat.h"

#include "socket.h"
#include "packets.h"

#define SCANSIZE 4096

typedef struct ip ip_t;

struct sigaction sigact;
static void sig_hnd(int signo);
static jmp_buf  jmpbuf;

char buffer[SCANSIZE];
int                _fcs_bits = 32;

int fdmax;

struct libtrace_t *trace = 0;

struct timeval starttime;
struct timeval nowtime;

uint32_t ts32 = 0;
uint32_t startts32 = 0;


//void get_colour(uint8_t[3],int,int);

void do_usage(char* name)
{
    printf("Usage: %s [-h] [-s <source>] [-p <listenport>] [-f <filter>] [-c <colour module] [-l <left position module] [-r <right position module>] [-d <direction module>]\n", name);
    exit(0);
}


//---------------------------------------------------
int main(int argc, char *argv[])
{
	int opt;

	// socket stuff
	int listen_socket;
	int port = 32500;
	fd_set listen_set;
	struct timeval tv;
	int new_client;

	bool live = true;
	char *tmp;

	char *filterstring = 0;
	char *colourmod = "plugins/colour/colours.so";
	char *leftpos =   "plugins/position/network16.so";
	char *rightpos =  "plugins/position/radial.so";
	char *dirmod =    "plugins/direction/interface.so";

	void *colourhandle = 0;
	void *lefthandle = 0;
	void *righthandle = 0;
	void *dirhandle = 0;

	struct modptrs_t modptrs;

	// rt stuff
	static struct libtrace_filter_t *filter = 0;
	struct libtrace_packet_t packet;
	static char* uri = 0; // default is chasm
	int psize;
	uint64_t ts;

	char *error = 0;

	gettimeofday(&starttime, 0); // XXX

	FD_ZERO(&listen_set);

	tv.tv_sec = 0;
	tv.tv_usec = 0;


	// setup signal handlers
	sigact.sa_handler = sig_hnd;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;
	/*
	   if(sigaction(SIGHUP, &sigact, NULL) < 0) {
	   perror("sigaction");
	   exit(-1);
	   }

	   if(sigaction(SIGINT, &sigact, NULL) < 0) {
	   perror("sigaction");
	   exit(-1);
	   }


	   if(sigaction(SIGTERM, &sigact, NULL) < 0) {
	   perror("sigaction");
	   exit(-1);
	   }
	   */

	if(sigaction(SIGPIPE, &sigact, NULL) < 0) {
		perror("sigaction");
		exit(-1);
	}

	if (setjmp(jmpbuf))
		//goto goodbye;
		goto blah;

	//-------------------------------------------------------
	// do some command line stuff for ports and things

	while( (opt = getopt(argc, argv, "hs:p:f:c:l:r:d:")) != -1)
	{
		switch(opt)
		{
			case 'h':
				do_usage(argv[0]);
				break;
			case 's':
				// should I be doing some checking on this arg?
				uri = optarg;
				break;
			case 'p':
				// should I be doing some checking on this arg?
				port = atoi(optarg);
				break;
			case 'f':
				filterstring = optarg;
				break;
			case 'c':
				colourmod = optarg;
				break;
			case 'r':
				rightpos = optarg;
				break;
			case 'l':
				leftpos = optarg;
				break;
			case 'd':
				dirmod = optarg;
				break;
			default: do_usage(argv[0]);
		};
	}
	//--------------------------------------------------------

	init_packets();

	//-------Setup listen socket-----------
	listen_socket = setup_listen_socket(); // set up socket
	bind_tcp_socket(listen_socket, port); // bind and start listening

	// add the listening socket to the master set
	fdmax = listen_socket; // biggest file descriptor
	printf("Waiting for connection on port %i...\n", port);

	hax_fdmax(fdmax);//XXX



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


	//------- ---------------------------

	check_clients(true);// keep rtclient from starting till someone connects


	//------- Connect RTClient ----------
	trace = trace_create(uri);
	printf("Connected to data source: %s\n", uri);

	//------- Create filter -------------
	if (filterstring) {
		printf("setting filter %s\n",filterstring);
		filter = trace_bpf_setfilter(filterstring);
	}

	// hax to make it only slow down saved trace files...looks for a '/'
	for(tmp=uri; *tmp != '\0'; tmp++)
		if(*tmp == '/')
		{
			printf("Attempting to replay trace in real time\n");
			live = false;
			break;
		}

	// someone explain why i have 2 loops here...think its cause I did
	// have some clean up code which left/moved, and how it fitted in
	// with signal handlers
	while(1)
	{
		// loop till something breaks

		for(;;) 
		{
			/* check for new clients */
			new_client = check_clients(false);
			if(new_client > 0)// is zero valid?
				send_flows(new_client);

			/* get a packet, and process it */
			if((psize = trace_read_packet(trace, &packet)) <= 0)
			{
				perror("libtrace_read_packet");
				break;
			}

			if (filter)  
				if (!trace_bpf_filter(filter,&packet)) 
					continue;

			ts = trace_get_erf_timestamp(&packet);

			/* this time checking only matters when reading from a prerecorded
			 * trace file. It limits it to a seconds worth of data a second,
			 * which is still a large chunk*/
			if(!live)
			{
				if(startts32 == 0)
					startts32 = ts >> 32;

				ts32 = ts >> 32;
				gettimeofday(&nowtime, 0);

				// check that we are in the right second, and arent 
				// getting ahead of ourselves
				while( ((uint32_t)(nowtime.tv_sec - starttime.tv_sec) < (ts32 - startts32)))
				{
					usleep(10);	
					gettimeofday(&nowtime, 0);

				}

				//check that we are in the right part of the second and arent
				//getting ahead of ourselves. could go into more detail
				//here, but is not super important
				uint32_t dagparts_hax = 0;
				if(ts & 0x0000000080000000)
					dagparts_hax += 500000;
				if(ts & 0x0000000040000000)
					dagparts_hax += 250000;
				if(ts & 0x0000000020000000)
					dagparts_hax += 125000;

				while((uint32_t)(nowtime.tv_usec) < dagparts_hax)
				{
					usleep(1);
					gettimeofday(&nowtime, 0);
				}
			}
			// if sending fails, assume we just lost a client
			if(per_packet(packet, ts, &modptrs) != 0)
				break;

		}
blah:
		// any individual clean up could go here...maybe not useful?
		printf("Cleaning up...\n");
		//destroy_rtclient(rtclient);
	}
	//goodbye:
	printf("Destroying libtrace...\n");
	trace_destroy(trace);
	printf("Removing flow information...\n");
	//empty_flows();
	printf("Closing socket...\n");
	close(listen_socket);


	if(colourhandle) {
		dlclose(colourhandle);
	}
	if(lefthandle) {
		dlclose(lefthandle);
	}
	if(righthandle) {
		dlclose(righthandle);
	}
	printf("Exiting...\n");
	exit(0);

}


void sig_hnd( int signo )
{
    longjmp(jmpbuf, 1);
}

