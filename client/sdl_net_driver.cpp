/*
Copyright (c) Sam Jansen and Jesse Baker.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

3. The end-user documentation included with the redistribution, if
any, must include the following acknowledgment: "This product includes
software developed by Sam Jansen and Jesse Baker 
(see http://www.wand.net.nz/~stj2/bung)."
Alternately, this acknowledgment may appear in the software itself, if
and wherever such third-party acknowledgments normally appear.

4. The hosted project names must not be used to endorse or promote
products derived from this software without prior written
permission. For written permission, please contact sam@wand.net.nz or
jtb5@cs.waikato.ac.nz.

5. Products derived from this software may not use the "Bung" name
nor may "Bung" appear in their names without prior written
permission of Sam Jansen or Jesse Baker.

THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL COLLABNET OR ITS CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
// $Id$
#include "stdafx.h"
#include "world.h"
#include "net_driver.h"
#include "exception.h"
#include "entity_manager.h"
#include "player.h"
#include "partflow.h"
#include "partvis.h"
#include "misc.h"

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
        // Note: SDLNet_GetError() doesn't seem to say anything useful here.
	throw CException(bsprintf("Error: Unable to resolve host '%s'",
                    host.c_str()));
    }

    clientsock = SDLNet_TCP_Open(&ip);

    if(!clientsock)
	throw CException(bsprintf("Unable to connect to server '%s': '%s'",
                    host.c_str(), SDLNet_GetError()));

    set = SDLNet_AllocSocketSet(16);
    if(!set)
	throw CException(SDLNet_GetError());

    if(SDLNet_TCP_AddSocket(set, clientsock) == -1)
	throw CException(SDLNet_GetError());
    
    Log("Connected!\n");

    char version;
    if(SDLNet_TCP_Recv(clientsock, &version, 1) != 1) {
	    Log("Unable to read server version\n");
	    throw CException("Unable to read server version\n");
    }

    /* Versions lower than 0x12 are ancient */
    if (version < 0x10) {
	    Log("Server version is ancient\n");
	    throw CException("Server version is ancient\n");
    }

    const int min_version = 0x13; // Minimum version known
    const int max_version = 0x13; // Maximum version known

    if (version < min_version) {
	    throw CException(bsprintf("Server version is %i.%i, need at least %i.%i\n",
			    version>>4,version&0x0F,
			    min_version>>4,max_version&0x0f));
    }

    // Maximum protocol version known is uh, 1.2
    if (version > max_version) {
	    throw CException(bsprintf("Server version is %i.%i, client version is only %i.%i\n",
			    version>>4,version&0x0F,
			    max_version>>4,max_version&0x0f));
    }
    
}

void CSDLNetDriver::SendData(CPlayer *p)
{
    throw CException("CSDLNetDriver::SendData: This function is currently not"
	    " implemented!");
}

#ifdef _WIN32
#pragma pack(push)
#pragma pack(1)
#define PACKED
#else
#define PACKED __attribute__((packed))
#endif

struct flow_update_t {
    unsigned char type; // 0
    float x1;
    float y1;
    float z1;
    float x2;
    float y2;
    float z2;
    unsigned int id;
} PACKED;

struct pack_update_t {
    unsigned char type; // 1
    uint32 ignored, ts;
    unsigned int id;
    unsigned char id_num;
    unsigned short size;
    float speed;
	bool dark;
} PACKED;

struct flow_remove_t {
    unsigned char type; // 2
    unsigned int id;
} PACKED;

struct kill_all_t {
	bool all; // 3
} PACKED;

struct flow_descriptor
{
	unsigned char type;
	unsigned char id;
	byte colour[3];
	char name[256];
} PACKED;

union fp_union {
    struct flow_update_t flow;
    struct pack_update_t packet;
    struct flow_remove_t rem;
    struct kill_all_t kall;
	struct flow_descriptor fdesc;
};

#ifdef _WIN32
#pragma pack(pop)
#endif

//#define NET_DEBUG

void CSDLNetDriver::ReceiveData()
{
    byte buffer[1024];
    int  readlen;
    union fp_union *fp;

    while(SDLNet_CheckSockets(set, 0) > 0)
    {
	if((readlen = SDLNet_TCP_Recv(clientsock, buffer, 1024)) > 0)
	{
	    int sam = (int)databuf.size();
	    databuf.resize( sam + readlen );
	    memcpy(&databuf[sam], buffer, readlen);

#ifdef NET_DEBUG
	    Log("Read %d bytes\n", readlen);
#endif
	}
	if(readlen == 0)
	    break;
    }

    // Log("Databuf.size()=%d\n", databuf.size());
    world.partVis->BeginUpdate();

    unsigned char *buf = &databuf[0];
    while( true ) 
    {
	const unsigned int s = (const unsigned int)(&databuf[databuf.size()] - &buf[0]);
	fp = (fp_union *)buf;
	if(s == 0) 
	{
	    databuf.erase(databuf.begin(), databuf.end());
	    break;
	}
	if(fp->flow.type == 0x00) 
	{
	    // Flow
	    if(sizeof(flow_update_t) <= s) 
	    {
#ifdef NET_DEBUG
		Log("Flow: (%f,%f,%f)->(%f,%f,%f):%u\n", 
			fp->flow.x1,
			fp->flow.y1,
			fp->flow.z1,
			fp->flow.x2,
			fp->flow.y2,
			fp->flow.z2,
			fp->flow.id);
#endif

		world.partVis->UpdateFlow(
			fp->flow.id,
			Vector3f(fp->flow.x1, fp->flow.y1, fp->flow.z1),
			Vector3f(fp->flow.x2, fp->flow.y2, fp->flow.z2));

		buf += sizeof(flow_update_t);
	    } 
	    else 
	    {
		if(buf != &databuf[0])
		    databuf.erase(databuf.begin(), databuf.begin() + (buf - &databuf[0]));
		break;
	    }
	} 
	else if(fp->flow.type == 0x01) 
	{
	    // Packet
	    if(sizeof(pack_update_t) <= s) {
#ifdef NET_DEBUG
		Log("Packet: ts:%u id:%u (%d,%d,%d) size:%u\n",
			fp->packet.ts,
			fp->packet.id,
			(int)fp->packet.colour[0],
			(int)fp->packet.colour[1],
			(int)fp->packet.colour[2],
			fp->packet.size);
#endif
		//if( world.partVis->fps > 30.0f )//world.partVis->packetsFrame < 25 )
		//{
		world.partVis->packetsFrame++;
		//static uint32 tsfudge = fp->packet.ts;
		//tsfudge++;
		//Log( "%d\n", fp->packet.id_num );
		world.partVis->UpdatePacket(
			fp->packet.id, 
			fp->packet.ts,
			fp->packet.id_num,
			fp->packet.size,
			fp->packet.speed,
			fp->packet.dark);
		//}

		buf += sizeof(pack_update_t);
	    } 
	    else 
	    {
		if(buf != &databuf[0])
		    databuf.erase(databuf.begin(), databuf.begin() 
			    + (buf - &databuf[0]));
		break;
	    }
	} 
	else if(fp->flow.type == 0x02) 
	{
	    if(sizeof(flow_remove_t) <= s) 
	    {
		world.partVis->RemoveFlow(fp->rem.id);

		buf += sizeof(flow_remove_t);
	    } 
	    else 
	    {
		if(buf != &databuf[0])
		    databuf.erase(databuf.begin(), databuf.begin() 
			    + (buf - &databuf[0]));
		break;
	    }
	}
	else if( fp->flow.type == 0x03 )
	{
	    // Kill all existing flows!
	    world.partVis->KillAll();
	    buf += sizeof(kill_all_t);
	}
	else if( fp->flow.type == 0x04 )
	{
		// Flow descriptor.
		// Check if we have it already, if we don't add it, if we do
		// update it if its different, else do nothing.
		int tempK = fp->fdesc.id;
		int ppK = tempK;
		if( world.partVis->fdmap.find( fp->fdesc.id ) == world.partVis->fdmap.end() )
		{
			// Not in the map yet so lets add it:
			FlowDescriptor *fd = new FlowDescriptor();
			fd->colour[0] = fp->fdesc.colour[0];
			fd->colour[1] = fp->fdesc.colour[1];
			fd->colour[2] = fp->fdesc.colour[2];
			strcpy( fd->name, fp->fdesc.name );
			Log( "New flow desc: %s %d %d %d ID: %d", fd->name, fd->colour[0], fd->colour[1], fd->colour[2], fp->fdesc.id );
			world.partVis->fdmap.insert( FlowDescMap::value_type(fp->fdesc.id, fd) );
		}
		buf += sizeof(flow_descriptor);
	}
	else 
	{
	    Log("Unknown packet type...s:%u r:%u d:%u \n",
		    databuf.size(), 
		    (int)(buf-&databuf[0]), 
		    (int)(&databuf[databuf.size()]-buf)
	       );
	    databuf.erase(databuf.begin(), databuf.end());
	    break;
	}
    }

    world.partVis->EndUpdate();
}

