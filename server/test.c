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
#include "rtclient.h"


#include <dagnew.h>
#include <dagapi.h>
#include "utils.h"
#include "outputs.h"
#include "inputs.h"

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
int new_fd;

struct timeval starttime;
struct timeval nowtime;

uint32_t ts32 = 0;
uint32_t startts32 = 0;


//---------------------------------------------------
int main(int argc, char *argv[])
{
    int opt;

    // socket stuff
    int listen_socket;
    int port = 32500;
    struct sockaddr_in remoteaddr; // client address
    int sin_size;


    // rt stuff
    static struct rt_client_t *rtclient;
    static char* hostname = "chasm.cs.waikato.ac.nz";
    //static char* hostname = "/scratch/bcj3/slammer.trace";
    int psize;
    int status = 0;
    uint64_t ts;
    dag_record_t *erfptr = 0;


    gettimeofday(&starttime, 0); // XXX
    

    // setup signal handlers
    sigact.sa_handler = sig_hnd;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;
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
    if(sigaction(SIGPIPE, &sigact, NULL) < 0) {
	perror("sigaction");
	exit(-1);
    }

    if (setjmp(jmpbuf))
	//goto goodbye;
	goto blah;

    //-------------------------------------------------------
    // do some command line stuff for ports and things

    while( (opt = getopt(argc, argv, "d:s:p:")) != -1)
    {
	switch(opt)
	{
	    case 'd':
		opt = atoi(optarg);
		switch(opt)
		{
		    case 0:
			//SHOW_SRC = 1;
			//SHOW_DST = 0;
			printf("Drawing outgoing packets only\n");
			break;
		    case 1:
			//SHOW_DST = 1;
			//SHOW_SRC = 0;
			printf("Drawing incoming packets only\n");
			break;
		    default:
			//SHOW_DST = SHOW_SRC = 1;
			printf("Drawing both incoming and outgoing packets\n");
			break;
		}; break;
	    case 's':
		// should I be doing some checking on this arg?
		hostname = optarg;
		break;
	    case 'p':
		// should I be doing some checking on this arg?
		port = atoi(optarg);
		break;

	    default: /* do nothing */ break;
	};
    }
    //--------------------------------------------------------
    
    while(1)
    {

	//-------Setup listen socket-----------
	listen_socket = setup_listen_socket(); // set up socket
	bind_tcp_socket(listen_socket, port); // bind and start listening

	// add the listening socket to the master set
	fdmax = listen_socket; // biggest file descriptor
	printf("Waiting for connection...\n");

	sin_size = sizeof(struct sockaddr_in);
	if ((new_fd = accept(listen_socket, (struct sockaddr *)&remoteaddr,
			&sin_size)) == -1) {
	    perror("accept");
	    exit(1);
	}
	printf("Received connection from: %s\n",inet_ntoa(remoteaddr.sin_addr));

	//------- Connect RTClient ----------
	rtclient = create_rtclient(hostname,0);
	printf("Data source Connected to: %s\n", hostname);

	init_flow_hash(new_fd);
	// loop till something breaks
	for(;;) 
	{
	    // get a packet, and do stuff to it
	    if((psize = rtclient_read_packet(rtclient, buffer, &status)) <= 0)
	    {
		perror("rtclient_read_packet");
		break;
	    }
	    erfptr = (dag_record_t *)buffer;
	    //p = (ip_t *) erfptr->rec.eth.pload;
	    ts = erfptr->ts;//i dont need to pass this btw

	    ////////////////////
	    if(startts32 == 0)
		startts32 = ts >> 32;

	    ts32 = ts >> 32;
	    gettimeofday(&nowtime, 0);

	    //printf("now %li, start %li -- now %i, start %i\n... %li vs %i", nowtime.tv_sec ,starttime.tv_sec,ts32 ,startts32,nowtime.tv_sec - starttime.tv_sec, ts32 - startts32) ;

	    while( ((nowtime.tv_sec - starttime.tv_sec) < (ts32 - startts32)))
	    {
		usleep(10);	
		gettimeofday(&nowtime, 0);

	    }
	    
	    ////////////////////////
	    if(per_packet(erfptr, psize, ts/*, new_fd*/) != 0)
		break;
	}
blah:
	printf("Cleaning up...\n");
	destroy_rtclient(rtclient);
	empty_flows();
	close(listen_socket);
    }
//goodbye:
	printf("Destroying RTClient...\n");
	destroy_rtclient(rtclient);
	printf("Removing flow information...\n");
	empty_flows();
	printf("Closing socket...\n");
	close(listen_socket);
	printf("Exiting...\n");
	exit(0);

}

void sig_hnd( int signo )
{
    longjmp(jmpbuf, 1);
}
