/* $Header$ $Log$
/* $Header$ Revision 1.3  2004/02/17 01:59:56  stj2
/* $Header$ Cvs tags will be correct at one point. Surely.
/* $Header$ */ 
#ifndef _ENET_DRIVER_H_
#define _ENET_DRIVER_H_

class CNetDriver;

class CEnetDriver : public CNetDriver
{
private:
    ENetHost *client;
    ENetPeer *server;

public:
    CEnetDriver();
    virtual ~CEnetDriver();

    virtual void Connect(string address);
	virtual void SendData(CPlayer *p);
	virtual void ReceiveData();

    void Disconnect();


    struct BungPacket
	{
		float pitch;
		float facing;
		float x_pos;
		float y_pos;
		float z_pos;
		Vector3f velocity;
		int	  player_id;
	};

};

#endif // _ENET_DRIVER_H_
