/* $CVSID$ */ 
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
