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
					FLOW_DESCRIPTOR,
					IMAGE_DATA
				};

#pragma pack(push, 1)

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
	uint32_t ip2;};


//New packet packets
struct pack_update_t {
	unsigned char type;//1
	uint32_t ts;
	uint32_t id; 
	unsigned char packetType;
	uint16_t size;
	float speed; 
	bool dark;
};

//Expire flow packets
struct flow_remove_t {
	unsigned char type;//2
	uint32_t id;
};

//Kill all packets
struct kill_all_t {
	unsigned char type;//3
};

//Type description
struct flow_descriptor_t {
	unsigned char type; //4
	unsigned char id;
	uint8_t colour[3];
	char name[256];
};

//Image data 
struct image_data_t {
	unsigned char type; //5
	unsigned char id; // 0 = left, 1 = right
	uint32_t length; //the number of bytes following this packet of image data
};

#pragma pack(pop)

//Size in bytes of each packet
int packetSizes[] = {	sizeof(flow_update_t), sizeof(pack_update_t), 
						sizeof(flow_remove_t), sizeof(kill_all_t),
						sizeof(flow_descriptor_t), sizeof(image_data_t) };
					
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

	mDataBuf.clear();
	
	return true;
}

/*********************************************
	Connect to a server
**********************************************/
bool App::openSocket(){

	//If we're switching between servers, kill all our particles
	if(ps()){
		ps()->delAll();
	}
	
	//And flows
	if(mFlowMgr){
		mFlowMgr->delAll();
		mFlowMgr->notifyServerChange();
	}

	if(mClientSocket){
		//We're connected. Disconnect!
		disconnect(false);
	}
	
    IPaddress ip;

    LOG("Connecting to '%s:%d'\n", mServerAddr.c_str(), iServerPort);
    
	if(SDLNet_ResolveHost(&ip, (char *)mServerAddr.c_str(), iServerPort) == -1){
		ERR("Error: Unable to resolve host '%s'\n", mServerAddr.c_str());
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
    
    //Set the 'connected to server' text
    updateGUIConnectionStatus();
    
    //Set the original orientation and such
    resetCam();
      
    return true;
}

/*********************************************
  Sends the initial discovery packets 
**********************************************/
void App::beginDiscovery(){
	
	//Clear udp data
	for(int i=0;i<512;i++){
		mUDPPacket->data[i] = NULL;
	}
		
	//Remove old entries
	clearServerList(); 
	
	iCurrentUDPPort = UDP_SERVER_PORT;
	
	for(int i=0;i<UDP_PORT_RANGE;i++){
		sendDiscoveryPacket(iCurrentUDPPort++);
	}

}

/*********************************************
  Sends a UDP broadcast packet. 
**********************************************/
void App::sendDiscoveryPacket(int port){
		
	//Note that we don't actually wait for a response here. That's handled below
	//in updateSocket(). 
	if(SDLNet_ResolveHost(&mUDPPacket->address, "255.255.255.255", port) == -1){
		ERR("Error: Unable to resolve host '%s'", mServerAddr.c_str());
		return;
	}
		
	std::string data = toString("bsod2"); //For lack of something better
	
	//Copy the data into the UDP packet
	strcpy((char *)mUDPPacket->data, data.c_str());
	mUDPPacket->len = (int)data.size() + 1;
	
	//And send it off.
	SDLNet_UDP_Send(mUDPSocket, -1, mUDPPacket); 
	
	//LOG("Sent discovery to port %d\n", port);	
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
	
	if(!num){
		return;
	}
	
	
	uint32_t ipaddr = 0;
	ipaddr=SDL_SwapBE32(mUDPPacket->address.host);
	
	string remoteIP = 	toString(ipaddr>>24) + "." + 
						toString((ipaddr>>16)&0xff) + "." + 
						toString((ipaddr>>8)&0xff) + "." + 
						toString(ipaddr&0xff);
						
	int replyPort =  ntohs(mUDPPacket->address.port);	
	
	string remoteData = string((char *)mUDPPacket->data);   

	for(int i=0;i<512;i++){
		mUDPPacket->data[i] = NULL;
	}
			
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

		//LOG("%d:%d\n", i, split[i].size());
	}

	//LOG("Got %s\n", name.c_str());
	
	//And add it to the GUI				
	addServerListEntry(name, remoteIP, port);
	
	//Send out some more broadcasts. We want to make sure that we have sent 
	//replyPort + 5. 	
	
	
	while(replyPort + UDP_PORT_RANGE > iCurrentUDPPort){
		sendDiscoveryPacket(iCurrentUDPPort++);
	}
    
}
	
/*********************************************  Update the TCP connection with the server
**********************************************/
void App::updateTCPSocket(){
	
	int readlen = 0;
	
	while(SDLNet_SocketReady(mClientSocket)){
		if((readlen = SDLNet_TCP_Recv(mClientSocket, buffer, 1024)) > 0){
			int end = mDataBuf.size();
			mDataBuf.resize( end + readlen );
			memcpy(&mDataBuf[end], buffer, readlen);	
		}else{
			LOG("Connection lost!\n");
			disconnect(true);
			return;
		}
		
		if(readlen == 0){
			LOG("Connection lost!\n");
			disconnect(true);
			return;
		}
	}
	
	if(mDataBuf.size() == 0){
		return;
	}
	//LOG("Got %d bytes\n", mDataBuf.size());
	
	//At this point we should have all the queued data from the server
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
		if(type > 5){
			LOG("Bad packet type %d\n", type);
			notifyShutdown();
			return;
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
					//LOG("Discarded flow: %d\n", iFPS);
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
			
			
			Vector3 start = Vector3(ntohf(pkt->x1),
									ntohf(pkt->z1), 
									ntohf(pkt->y1));
									
			Vector3 end = Vector3(	ntohf(pkt->x2), 
									ntohf(pkt->z2), 
									ntohf(pkt->y2));
			
			start.x = -start.x;
			end.x = -end.x;
			
			mFlowMgr->newFlow(ntohl(pkt->id), src, dest, start, end);			
		}
		
		//Expired flow
		else if(type == FLOW_REMOVE){		
			flow_remove_t *pkt = (flow_remove_t *)data;						
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
				
			uint16_t size = ntohs(pkt->size);
			float rtt = ntohf(pkt->speed);

			if(rtt < 0.0f){				
				rtt = -rtt;
			}
			
			iCurrentTime = ntohl(pkt->ts);		
			
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
				addFlowDescriptor(pkt->id, Color(pkt->colour[0], pkt->colour[1], 
									pkt->colour[2]), string(pkt->name));
			}
		}
		
		else if(type == IMAGE_DATA){
		
			image_data_t *pkt = ( image_data_t *)data;
			
			//Kinda hacky - we break here as well if we don't have the whole
			//image buffer. 
			thisSize += pkt->length;
			
			if((int)mDataBuf.size() - index < thisSize){
				//we don't have the whole packet...
				break;
			}
					
			byte *buf = data + sizeof(image_data_t);
						
			if(pkt->id == 0 && mLeftTexName == ""){
				mLeftTex = texGenerate("left", buf, pkt->length);
			}else if(pkt->id == 1  && mRightTexName == ""){
				mRightTex = texGenerate("right", buf, pkt->length);
			}else{
				LOG("Ignored image ID %d\n", pkt->id);
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
                                
void App::disconnect(bool notify){

	bConnected = false;

	if(mClientSocket){
		SDLNet_TCP_Close(mClientSocket);
	
		if(SDLNet_TCP_DelSocket(mSocketSet, mClientSocket) == -1){
			ERR("%s\n", SDLNet_GetError());
			return;
		}
	}
	
	mClientSocket = NULL;
	
	//Clean up any textures we may have got
	if(mLeftTex && mLeftTexName == ""){
		texDelete(mLeftTex);
		mLeftTex = NULL;
	}
	
	if(mRightTex && mRightTexName == ""){
		texDelete(mRightTex);
		mRightTex = NULL;
	}
	
	LOG("Disconected from server\n");
	
	ps()->delAll();
	
	updateGUIConnectionStatus();		
		
	if(notify){
		messagebox("Disconnected from server " + 
						mServerAddr + ":" + 
						toString(iServerPort), 
						"Disconnected");
	}
}

/*********************************************
		Shutdown
**********************************************/
void App::closeSocket(){

	SDLNet_FreePacket(mUDPPacket);
	
	SDLNet_UDP_Close(mUDPSocket);
	SDLNet_TCP_Close(mClientSocket);

	SDLNet_FreeSocketSet(mSocketSet);

	SDLNet_Quit();
	
	LOG("Shut down networking!\n");
}
