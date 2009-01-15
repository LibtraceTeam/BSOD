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
	Socket connection globals
**********************************************/
TCPsocket mClientSocket = NULL;
UDPsocket mUDPSocket = NULL;
SDLNet_SocketSet mSocketSet = NULL;
vector<byte> mDataBuf;
UDPpacket *mUDPPacket = NULL;

bool bSkipFlow = false;
bool bSkipPacket = false;
bool bConnected = false;
unsigned int iTime = 0;
	
byte buffer[1024];


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
					
/*********************************************	Sets up the initial networking
**********************************************/	
bool App::initSocket(){
	
	mClientSocket = NULL;
	
	//Open UDP
	mUDPSocket = SDLNet_UDP_Open(0);
	
	if(!mUDPSocket){
        ERR("Unable to open UDP socket: %s\n", SDLNet_GetError());
		return false;
	}
	
	//Allocate memory for the UDP packet
	if (!(mUDPPacket = SDLNet_AllocPacket(512)))
	{
		ERR("SDLNet_AllocPacket: %s\n", SDLNet_GetError());
		return false;
	}


	//Set up socket set
    mSocketSet = SDLNet_AllocSocketSet(16);
    if(!mSocketSet){
		ERR("%s\n", SDLNet_GetError());
		return false;
	}
 	
 	if(SDLNet_UDP_AddSocket(mSocketSet, mUDPSocket) == -1){
    	ERR("%s\n", SDLNet_GetError());
		return false;
	}
	
	return true;
}

/*********************************************
	Connect to a server
**********************************************/
bool App::openSocket(){

	//If we're switching between servers, kill all our particles
	ps()->delAll();

	if(mClientSocket){
		//We're connected. Disconnect!
		SDLNet_TCP_Close(mClientSocket);
		bConnected = false;
		
		if(SDLNet_TCP_DelSocket(mSocketSet, mClientSocket) == -1){
			ERR("%s\n", SDLNet_GetError());
			return false;
		}
		
		LOG("Disconected from server\n");
		
	}
	
    IPaddress ip;

    LOG("Connecting to '%s:%d'\n", mServerAddr.c_str(), iServerPort);
    
	if(SDLNet_ResolveHost(&ip, (char *)mServerAddr.c_str(), iServerPort) == -1) {
		ERR("Error: Unable to resolve host '%s'", mServerAddr.c_str());
		return false;
	}

	//Open TCP
    mClientSocket = SDLNet_TCP_Open(&ip);

    if(!mClientSocket){
        ERR("Unable to connect to server '%s':\n %s\n",
                    mServerAddr.c_str(), SDLNet_GetError());
		return false;
	}
	
    if(SDLNet_TCP_AddSocket(mSocketSet, mClientSocket) == -1){
    	ERR("%s\n", SDLNet_GetError());
		return false;
	}
   
    LOG("Connected to server\n");
    
    mDataBuf.clear();
        
	fGUITimeout = 10.0f; //Make sure people see the UI
    bConnected = true;
      
    return true;
}

/*********************************************
  Sends a UDP broadcast packet. 
**********************************************/
void App::sendDiscoveryPacket(){

	//Remove old entries
	clearServerList(); 
		
	//Note that we don't actually wait for a response here. That's handled below
	//in updateSocket(). 
	IPaddress srvadd;
	
	mUDPPacket->address.host = INADDR_ANY;	//Destination is everyone
	mUDPPacket->address.port = htons(UDP_SERVER_PORT);
	
	std::string data = toString(VERSION); //For lack of something better to send
	
	//Copy the data into the UDP packet
	strcpy((char *)mUDPPacket->data, data.c_str());
	mUDPPacket->len = data.size() + 1;
	
	//And send it off.
	SDLNet_UDP_Send(mUDPSocket, -1, mUDPPacket); 
	
	//LOG("Sent discovery!\n");	
}
	
	
/*********************************************
  Read data from the network and process it
**********************************************/
void App::updateSocket(){

	while(SDLNet_CheckSockets(mSocketSet, 0) > 0){
			
		if(SDLNet_SocketReady(mClientSocket)){
			updateTCPSocket();
		}
		
		if(SDLNet_SocketReady(mUDPSocket)){
			updateUDPSocket();
		}
	}
}
	
/*********************************************  Update the UDP broadcast socket
**********************************************/
void App::updateUDPSocket(){

	int num = SDLNet_UDP_Recv(mUDPSocket, mUDPPacket);
    if(num) {
    	uint32_t ipaddr = 0;
    	ipaddr=SDL_SwapBE32(mUDPPacket->address.host);
    	
    	string remoteIP = 	toString(ipaddr>>24) + "." + 
    						toString((ipaddr>>16)&0xff) + "." + 
    						toString((ipaddr>>8)&0xff) + "." + 
    						toString(ipaddr&0xff);
		
		string remoteData = string((char *)mUDPPacket->data);    						
   			
		vector<string> split;
		int n = splitString(remoteData, "|", split);
		
		if(n < 2){
			ERR("Invalid UDP data: %d: '%s'\n", n, mUDPPacket->data);
			return;
		}
		
		//If we got sent a valid IP, override the one we got from the packet
		if(split[0] != "0.0.0.0"){
			remoteIP = split[0];
		}
		
		string port = split[1];
		
		//Put the name back together
		string name = "";
		for(int i=2;i<(int)split.size();i++){
			name += split[i];			
			if(i != (int)split.size() - 1)	name += "|";
		}
		
		//And add it to the GUI
		
		//LOG("Got remote server '%s': %s:%s\n", name.c_str(), remoteIP.c_str(), port.c_str());
		
		addServerListEntry(name, remoteIP, port);
    }	
}
	
/*********************************************  Update the TCP connection with the server
**********************************************/
void App::updateTCPSocket(){
	
	int readlen;
	
	while(SDLNet_SocketReady(mClientSocket)){
		if((readlen = SDLNet_TCP_Recv(mClientSocket, buffer, 1024)) > 0){
			int end = mDataBuf.size();
			mDataBuf.resize( end + readlen );
			memcpy(&mDataBuf[end], buffer, readlen);	
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
			
			mFlowMgr->newFlow(ntohl(pkt->id), src, dest, start, end);			
		}
		
		
		
		//Expired flow
		else if(type == FLOW_REMOVE){
		
			flow_remove_t *pkt = (flow_remove_t *)data;
			
			//LOG("Delete flow %u\n", ntohl(pkt->id));	
			
			mFlowMgr->delFlow(ntohl(pkt->id));		
		
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
		
			
		
			
			mFlowMgr->newPacket(ntohl(pkt->id), size, rtt, getFD(pkt->packetType));
			
			iTime = ntohl(pkt->ts);			
		}
		
		
		//Delete all
		else if(type == FLOW_ALL_REMOVE){
		
			mFlowMgr->delAll();
			
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
		Flow descriptor management
**********************************************/
void App::addFlowDescriptor(byte id, Color c, string name){

    if(mFlowDescriptors[id]){
            return; //we already have one!
    }
    

    FlowDescriptor *f = new FlowDescriptor;
    f->id = id;
    f->mColor = c;
    f->mName = name;
    f->bShown = true; //show by default
    
    mFlowDescriptors[id] = f;
    
    addProtocolEntry(name, c, id);
}
                                


/*********************************************
		Shutdown
**********************************************/
void App::closeSocket(){

	SDLNet_FreePacket(mUDPPacket);

	SDLNet_Quit();
	
	LOG("Shut down networking!\n");
}
