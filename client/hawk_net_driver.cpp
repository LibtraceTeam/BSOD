/* $CVSID$ */ 
#include "stdafx.h"
#include "world.h"
#include "net_driver.h"
#include "exception.h"
#include "entity_manager.h"
#include "player.h"
#include "partvis.h"

// Hawk NL network lib
#include "external/HawkNL/include/nl.h"
#include "hawk_net_driver.h"

CNetDriver *CNetDriver::Create()
{
	return new CHawkNetDriver();
}
////////////////////////////////////////////////////////////////////////
CHawkNetDriver::CHawkNetDriver()
{
	if(!nlInit())
		throw CException(GetErrorMessage());

	if(!nlSelectNetwork(NL_IP))
        throw CException(GetErrorMessage());
}

CHawkNetDriver::~CHawkNetDriver()
{
	nlClose(clientsock);
	nlShutdown();
}

string CHawkNetDriver::GetErrorMessage()
{
    NLenum err = nlGetError();
    
    if(err == NL_SYSTEM_ERROR)
    {
        return string(nlGetSystemErrorStr(nlGetSystemError()));
    }
    else
    {
        return string(nlGetErrorStr(err));
    }
}

void CHawkNetDriver::Connect(string address)
{
    NLaddress nlAddr;

    CReporter::Report(CReporter::R_MESSAGE,
	    "Using HawkNL library: '%s'\n", nlGetString(NL_VERSION));
    CReporter::Report(CReporter::R_MESSAGE,
	    "Supported network protocols: %s\n", nlGetString(NL_NETWORK_TYPES));


    /* create a client socket */
    clientsock = nlOpen(0, NL_RELIABLE); /* let the system assign the port number */

    CReporter::Report(CReporter::R_MESSAGE, "Lookup up id address for '%s'\n", address.c_str());
    nlGetAddrFromName((NLchar *)address.c_str(), &nlAddr);

    if(nlAddr.valid == NL_FALSE)
    {
	// Not sure if error message works all the time for this?
	throw CException(GetErrorMessage());
    }

    CReporter::Report(CReporter::R_MESSAGE, "Connecting...\n");

    /* Blocking! */
    nlEnable(NL_BLOCKING_IO);
    /* now connect */
    if(nlConnect(clientsock, &nlAddr) == NL_FALSE)
    {
	nlClose(clientsock);
	throw CException(GetErrorMessage());
    }
    /* Disable blocking */
    nlDisable(NL_BLOCKING_IO);
    
    // The above never seems to fail. ?????  -- because it does in the background!
    CReporter::Report(CReporter::R_MESSAGE, "Connected!\n");

    // TODO: Send 'Hello' packet and such if needed

}

void CHawkNetDriver::SendData(CPlayer *p)
{
}

struct flow_update_t {
    unsigned char type; // 0
    float x1;
    float y1;
    float z1;
    float x2;
    float y2;
    float z2;
    unsigned int id;
} __attribute__((packed));

struct pack_update_t {
    unsigned char type; // 1
    uint32 ignored, ts;
    unsigned int id;
    char colour[3];
    unsigned short size;
} __attribute__((packed));

struct flow_remove_t {
    unsigned char type; // 2
    unsigned int id;
} __attribute__((packed));

union fp_union {
    struct flow_update_t flow;
    struct pack_update_t packet;
    struct flow_remove_t rem;
};

void CHawkNetDriver::ReceiveData()
{
    NLbyte buffer[1024];
    NLint  readlen;
    union fp_union *fp;

    do {
	if((readlen = nlRead(clientsock, buffer, 1024)) > 0)
	{
	    int sam = databuf.size();
	    databuf.resize( sam + readlen );
	    memcpy(&databuf[sam], buffer, readlen);
	}
    } while(readlen);

   // Log("Databuf.size()=%d\n", databuf.size());
    world.partVis->BeginUpdate();
	    
    unsigned char *buf = &databuf[0];
    while(true) {
	const unsigned int s = databuf.end() - buf;
	fp = (fp_union *)buf;
	if(s == 0) {
	    databuf.erase(databuf.begin(), databuf.end());
	    break;
	}
	if(fp->flow.type == 0x00) {
	    // Flow
	    if(sizeof(flow_update_t) <= s) {
		/*Log("Flow: (%f,%f,%f)->(%f,%f,%f):%u\n", 
			fp->flow.x1,
			fp->flow.y1,
			fp->flow.z1,
			fp->flow.x2,
			fp->flow.y2,
			fp->flow.z2,
			fp->flow.id);*/
		
		world.partVis->UpdateFlow(
			fp->flow.id,
			Vector3f(fp->flow.x1, fp->flow.y1, fp->flow.z1),
			Vector3f(fp->flow.x2, fp->flow.y2, fp->flow.z2));

		buf += sizeof(flow_update_t);
	    } else {
		if(buf != &databuf[0])
		    databuf.erase(databuf.begin(), buf);
		break;
	    }
	} else if(fp->flow.type == 0x01) {
	    // Packet
	    if(sizeof(pack_update_t) <= s) {
		/*Log("Packet: ts:%u id:%u (%hhu,%hhu,%hhu) size:%u\n",
			fp->packet.ts,
			fp->packet.id,
			fp->packet.colour[0],
			fp->packet.colour[1],
			fp->packet.colour[2],
			fp->packet.size);*/

		world.partVis->UpdatePacket(
			fp->packet.id, fp->packet.ts,
			fp->packet.colour[0],
			fp->packet.colour[1],
			fp->packet.colour[2],
			fp->packet.size);

		buf += sizeof(pack_update_t);
	    } else {
		if(buf != &databuf[0])
		    databuf.erase(databuf.begin(), buf);
		break;
	    }
	} else if(fp->flow.type == 0x02) {
	    if(sizeof(flow_remove_t) <= s) {
		world.partVis->RemoveFlow(fp->rem.id);

		buf += sizeof(flow_remove_t);
	    } else {
		if(buf != &databuf[0])
		    databuf.erase(databuf.begin(), buf);
		break;
	    }
	} else {
	    Log("Unknown packet type...s:%u r:%u d:%u \n",
		    databuf.size(), buf-&databuf[0], databuf.end()-buf);
	    databuf.erase(databuf.begin(), databuf.end());
	    break;
	}
    }

    world.partVis->EndUpdate();
}
