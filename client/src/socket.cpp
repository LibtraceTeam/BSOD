#include "main.h"

float htonf( float x )
{
        union
        {
                float f;
                uint32_t i;
        } _u;
        _u.f = (x);
        _u.i = htonl(_u.i);
        return( _u.f );
        //return( (float)(htonl( *((uint32*)&x) )) );
}
#define ntohf(x) htonf(x)


/*********************************************
	Packet structures
**********************************************/
enum PacketTypes {
					FLOW_UPDATE, 
					PACKET_UPDATE,
					FLOW_REMOVE,
					FLOW_ALL_REMOVE,
					FLOW_DESCRIPTOR
				};

//Flow update packets
struct flow_update_t {
	unsigned char type; //0
	float x1;
	float y1;
	float z1;
	float x2;
	float y2;
	float z2;
	uint32_t id;
	uint32_t ip1;
	uint32_t ip2;} __attribute__((packed));


//New packet packets
struct pack_update_t {
	unsigned char type;//1
	uint32_t ts;
	uint32_t id; 
	unsigned char packetType;
	uint16_t size;
	float speed; 
	bool dark;
} __attribute__((packed));

//Expire flow packets
struct flow_remove_t {
	unsigned char type;//2
	uint32_t id;
} __attribute__((packed));

//Kill all packets
struct kill_all_t {
	unsigned char type;//3
} __attribute__((packed));

//Type description
struct flow_descriptor_t {
	unsigned char type; //4
	unsigned char id;
	uint8_t colour[3];
	char name[256];
} __attribute__((packed));

//Size in bytes of each packet
int packetSizes[] = {	sizeof(flow_update_t), sizeof(pack_update_t), 
						sizeof(flow_remove_t), sizeof(kill_all_t),
						sizeof(flow_descriptor_t) };

/*********************************************
	Connect to a server
 Note: Code mostly stolen from old BSOD :)
**********************************************/
bool App::openSocket(){
	
    IPaddress ip;

    LOG("Connecting to '%s:%d'\n", mServerAddr.c_str(), iServerPort);
    
	if(SDLNet_ResolveHost(&ip, (char *)mServerAddr.c_str(), iServerPort) == -1) {
		ERR("Error: Unable to resolve host '%s'", mServerAddr.c_str());
		return false;
	}

    mClientSocket = SDLNet_TCP_Open(&ip);

    if(!mClientSocket){
        ERR("Unable to connect to server '%s':\n %s\n",
                    mServerAddr.c_str(), SDLNet_GetError());
		return false;
	}

    mSocketSet = SDLNet_AllocSocketSet(16);
    if(!mSocketSet){
		ERR("%s\n", SDLNet_GetError());
		return false;
	}

    if(SDLNet_TCP_AddSocket(mSocketSet, mClientSocket) == -1){
    	ERR("%s\n", SDLNet_GetError());
		return false;
	}
	
    LOG("Connected to server\n");
    
    mDataBuf.clear();
    
    //default to showing all
    for(int i=0;i<MAX_FLOW_DESCRIPTORS;i++){
    	mFlowDescriptors[i] = NULL;
    }
    
    bConnected = true;
    
    return true;
}
	
byte buffer[1024];
	
/*********************************************
  Read data from the network and process it
**********************************************/
void App::updateSocket(){

	int readlen;
	
	while(SDLNet_CheckSockets(mSocketSet, 0) > 0){
		if((readlen = SDLNet_TCP_Recv(mClientSocket, buffer, 1024)) > 0){
			int end = mDataBuf.size();
			mDataBuf.resize( end + readlen );
			memcpy(&mDataBuf[end], buffer, readlen);
	
			//LOG("Read %d bytes\n", readlen);
		}
		
		if(readlen == 0)
			break;
	}
	
	
	if(mDataBuf.size() == 0){
		return;
	}
	//LOG("Got %d bytes\n", mDataBuf.size());
	
	//At this point we should have all the queued data from the server in mDataBuf
	int index = 0;
	
	while(index < (int)mDataBuf.size()){
	
		//Get the type byte
		byte type = mDataBuf[index];
		
		//Protocol?
		if(type == 0x20 || type == 20){
			index++;
			continue; //protocol version, ignore it
		}
		
		//Sanity check...
		if(type > 4){
			LOG("Bad packet type %d\n", type);
			utilShutdown(0);
		}
		
		//Make sure we have all the data
		int thisSize = packetSizes[type];				
		if((int)mDataBuf.size() - index < thisSize){
			//we don't have the whole packet...
			break;
		}
		
		byte *data = &mDataBuf[index];
		
		
		//New flow
		if(type == FLOW_UPDATE){
		
			if(getFPS() < iDropFlowThresh){
				bSkipFlow = !bSkipFlow;
				if(bSkipFlow){
					index += thisSize;	
					continue;
				}
			}
		
			flow_update_t *pkt = (flow_update_t *)data;
			
			struct in_addr ip1, ip2;
			ip1.s_addr = ntohl(pkt->ip1);
			ip2.s_addr = ntohl(pkt->ip2);
					
			IPaddress src;
			IPaddress dest;
			
			src.host = ip1.s_addr;
			dest.host = ip2.s_addr;
			uint16_t proto = pkt->type;
			
			
			Vector3 start = Vector3(ntohf(pkt->x1), ntohf(pkt->z1), ntohf(pkt->y1));
			Vector3 end = Vector3(ntohf(pkt->x2), ntohf(pkt->z2), ntohf(pkt->y2));
			
			start.x = -start.x;
			end.x = -end.x;
			
			//LOG("%s: %f/%f/%f\n", inet_ntoa(ip1), start.x, start.y, start.z);
			//LOG("%s: %f/%f/%f\n\n", inet_ntoa(ip2), end.x, end.y, end.z);
			
			mCurrentModule->newFlow(ntohl(pkt->id), src, dest, start, end);			
		}
		
		
		
		//Expired flow
		else if(type == FLOW_REMOVE){
		
			flow_remove_t *pkt = (flow_remove_t *)data;
			
			//LOG("Delete flow %u\n", ntohl(pkt->id));	
			
			mCurrentModule->delFlow(ntohl(pkt->id));		
		
		}
		
		
		
		//New packet
		else if(type == PACKET_UPDATE){
		
			pack_update_t *pkt = ( pack_update_t *)data;
			
			if(!getFD(pkt->packetType)->bShown){
				index += thisSize;	
				continue;
			}
		
			if(getFPS() < iDropPacketThresh){
				bSkipPacket = !bSkipPacket;
				if(bSkipPacket){
					index += thisSize;	
					continue;
				}
			}
				
			//if(pkt->packetType == 6)
				//LOG("Packet: %d, %f\n", ntohs(pkt->size), ntohf(pkt->speed));

			uint16_t size = ntohs(pkt->size);
			float rtt = ntohf(pkt->speed);

			if(rtt < 0.0f){				
				rtt = -rtt;
			}
		
			
		
			
			mCurrentModule->newPacket(ntohl(pkt->id), size, rtt, getFD(pkt->packetType));	
			
			iTime = ntohl(pkt->ts);			
		}
		
		
		//Delete all
		else if(type == FLOW_ALL_REMOVE){
		
			mCurrentModule->delAll();
			
		}
		
		
		//Type -> info data
		else if(type == FLOW_DESCRIPTOR){
		
			flow_descriptor_t *pkt = ( flow_descriptor_t *)data;
						
			if(strcmp(pkt->name, "<NULL>") == 0){
				//Ignore it!
			}else{					
				addFlowDescriptor(pkt->id, Color(pkt->colour[0], pkt->colour[1], pkt->colour[2]), string(pkt->name));
			}
		}
		
		//Increment index to go past this packet
		index += thisSize;		
	}
	
	//Clean up buffer
	if(index == (int)mDataBuf.size())
		mDataBuf.clear();
	else 
		mDataBuf.erase(mDataBuf.begin(), mDataBuf.begin() + index);
}

/*********************************************
		Shutdown
**********************************************/
void App::closeSocket(){
	SDLNet_Quit();
	
	LOG("Shut down networking!\n");
}
