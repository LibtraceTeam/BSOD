#include "stdafx.h"
#include "world.h"
#include "net_driver.h"
#include "exception.h"
#include "entity_manager.h"
#include "player.h"
#include "partvis.h"

#include <SDL.h>
#include <SDL_net.h>

#include "sdl_net_driver.h"

CNetDriver *CNetDriver::Create()
{
    return new CSDLNetDriver();
}
////////////////////////////////////////////////////////////////////////
CSDLNetDriver::CSDLNetDriver()
{
    // Assumes SDL_Init() has been called
    if(SDLNet_Init() == -1)
	throw CException(SDLNet_GetError());
}

CSDLNetDriver::~CSDLNetDriver()
{
    SDLNet_Quit();
}


void CSDLNetDriver::Connect(string address)
{
    string::size_type c = address.find(':', 0);
    string host = address.substr(0, c);
    string port = address.substr(c+1, string::npos);
    IPaddress ip;

    Log("Connecting to Host: '%s' Port: '%s'\n", host.c_str(), port.c_str());
    
    if(SDLNet_ResolveHost(&ip, (char *)host.c_str(), 
		(unsigned short)atoi(port.c_str())) == -1) {
	throw CException(SDLNet_GetError());
    }

    clientsock = SDLNet_TCP_Open(&ip);

    if(!clientsock)
	throw CException(SDLNet_GetError());

    set = SDLNet_AllocSocketSet(16);
    if(!set)
	throw CException(SDLNet_GetError());

    if(SDLNet_TCP_AddSocket(set, clientsock) == -1)
	throw CException(SDLNet_GetError());
    
    Log("Connected!\n");
    
}

void CSDLNetDriver::SendData(CPlayer *p)
{
    throw CException("CSDLNetDriver::SendData: This function is currently not"
	    " implemented!");
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

void CSDLNetDriver::ReceiveData()
{
    byte buffer[1024];
    int  readlen;
    union fp_union *fp;

    while(SDLNet_CheckSockets(set, 0) > 0)
    {
	if((readlen = SDLNet_TCP_Recv(clientsock, buffer, 1024)) > 0)
	{
	    int sam = databuf.size();
	    databuf.resize( sam + readlen );
	    memcpy(&databuf[sam], buffer, readlen);
	}
	if(readlen == 0)
	    break;
    }

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
