/*
 * main loop(s) and the signal handlers may need looking at?
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
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

#include <assert.h>

extern "C" {
#include "rtclient2.h"


#include <dagnew.h>
#include "dagformat.h"
#include "utils.h"
#include "outputs.h"
#include "inputs.h"
#include "socket.h"
#include "packets.h"
}

#define SCANSIZE 4096

typedef struct ip ip_t;

struct sigaction sigact;
static void sig_hnd(int signo);
static jmp_buf  jmpbuf;

char buffer[SCANSIZE];
int                _fcs_bits = 32;

int fdmax;

struct timeval starttime;
struct timeval nowtime;

uint32_t ts32 = 0;
uint32_t startts32 = 0;



void do_usage(char* name)
{
    printf("Usage: %s [-s <source>] [-p <listenport>]\n", name);
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

    // rt stuff
    static struct rt_client_t *rtclient;
    static char* hostname = "chasm.cs.waikato.ac.nz"; // default is chasm
    int psize;
    int status = 0;
    uint64_t ts;
    dag_record_t *erfptr = 0;

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

    while( (opt = getopt(argc, argv, "s:p:")) != -1)
    {
	switch(opt)
	{
	    case 's':
		// should I be doing some checking on this arg?
		hostname = optarg;
		break;
	    case 'p':
		// should I be doing some checking on this arg?
		port = atoi(optarg);
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

    check_clients(true);// keep rtclient from starting till someone connects

    //------- Connect RTClient ----------
    rtclient = create_rtclient(hostname);
    printf("Connected to data source: %s\n", hostname);

    // hax to make it only slow down saved trace files...looks for a '/'
    for(tmp=hostname; *tmp != '\0'; tmp++)
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
	    if((psize = rtclient_read_packet(rtclient, buffer,SCANSIZE, &status)) <= 0)
	    {
		perror("rtclient_read_packet");
		break;
	    }
	    erfptr = (dag_record_t *)buffer;
	    ts = erfptr->ts;

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
	    if(per_packet(erfptr, psize, ts) != 0)
		break;

	}
blah:
	// any individual clean up could go here...maybe not useful?
	printf("Cleaning up...\n");
	//destroy_rtclient(rtclient);
    }
//goodbye:
	printf("Destroying RTClient...\n");
	destroy_rtclient(rtclient);
	printf("Removing flow information...\n");
	//empty_flows();
	printf("Closing socket...\n");
	close(listen_socket);
	printf("Exiting...\n");
	exit(0);

}


void sig_hnd( int signo )
{
    longjmp(jmpbuf, 1);
}

