#ifndef _SDL_NET_DRIVER_H_
#define _SDL_NET_DRIVER_H_

class CNetDriver;

class CSDLNetDriver : public CNetDriver
{
private:
	TCPsocket       clientsock;
	SDLNet_SocketSet set;
	
	vector<byte>	databuf;
	
public:
	CSDLNetDriver();
	virtual ~CSDLNetDriver();

	virtual void Connect(string address);
	virtual void SendData(CPlayer *p);
	virtual void ReceiveData();

};

#endif

