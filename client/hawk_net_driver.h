/* $CVSID$ */ 
#ifndef _HAWK_NET_DRIVER_H_
#define _HAWK_NET_DRIVER_H_

class CNetDriver;

class CHawkNetDriver : public CNetDriver
{
private:
	NLsocket    clientsock;
	
	string		GetErrorMessage();
	vector<byte>	databuf;
	
public:
	CHawkNetDriver();
	virtual ~CHawkNetDriver();

	virtual void Connect(string address);
	virtual void SendData(CPlayer *p);
	virtual void ReceiveData();

};

#endif

