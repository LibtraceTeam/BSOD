/* $Header$ $Log$
/* $Header$ Revision 1.3  2004/02/17 01:59:55  stj2
/* $Header$ Cvs tags will be correct at one point. Surely.
/* $Header$ */ 
#include "stdafx.h"
#include "world.h"
#include "net_driver.h"
#include "exception.h"
#include "entity_manager.h"
#include "player.h"

extern "C" {
    #include "external/enet/include/enet/enet.h"
}
#include "enet_driver.h"

/*CNetDriver *CNetDriver::Create()
{
	return new CEnetDriver();
}*/

CEnetDriver::CEnetDriver()
{
    client = NULL;

    if(enet_initialize() != 0) {
        throw CException("An error occured while initializing Enet.");
    }
}

CEnetDriver::~CEnetDriver()
{
    if(client)
        enet_host_destroy(client);
    enet_deinitialize();
}

void CEnetDriver::Connect(string to)
{
    client = enet_host_create(NULL, /* Client host */
                1, /* 1 outgoing connection */
                64000 / 8, /* 64k downstream */
                64000 / 8 /* 64k upstream */
            );

    if(client == NULL) {
        throw CException("Unable to create an enet host.");
    }

    ENetAddress address;
    ENetEvent event;
    ENetPeer *peer;

    enet_address_set_host(&address, to.c_str());
    address.port = 25000;

    // Connect to server, allocate only 1 channel
    peer = enet_host_connect(client, &address, 1);

    if(peer == NULL)
        throw CException("No available peers for initiating an ENet "
                " connection.");

    /* Wait up to 5 seconds for the connection attempt to succeed. */
    if (enet_host_service (client, & event, 5000) > 0 &&
        event.type == ENET_EVENT_TYPE_CONNECT)
    {
        Log((string("Connected to ") + to).c_str());
    }
    else
    {
        /* Either the 5 seconds are up or a disconnect event was 
           received. Reset the peer in the event the 5 seconds   
           had run out without any significant event.            */
        enet_peer_reset (peer);

        throw CException("Unable to connect to " + to);
    }

    server = peer;
}

void CEnetDriver::Disconnect()
{
    ENetEvent event;
    
    enet_peer_disconnect (& client -> peers [0]);

    /* Allow up to 3 seconds for the disconnect to succeed
       and drop any packets received packets. */
    while (enet_host_service (client, & event, 3000) > 0)
    {
        switch (event.type)
        {
        case ENET_EVENT_TYPE_RECEIVE:
            enet_packet_destroy (event.packet);
            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            Log("Disconnected from server successfully.\n");
            return;
	default:
	    Log("wtf?\n");
        }
    }
    
    /* We've arrived here, so the disconnect attempt didn't */
    /* succeed yet.  Force the connection down.             */
    enet_peer_reset (& client -> peers [0]);
}

void CEnetDriver::SendData(CPlayer *p)
{
    struct BungPacket bp;

    bp.pitch = p->GetBearing().y;
    bp.facing = p->GetBearing().x;

    bp.x_pos = p->GetPosition().x;
	bp.y_pos = p->GetPosition().y;
	bp.z_pos = p->GetPosition().z;

    bp.velocity = p->GetVelocity();
    // I don't think we need to set the following... its always 0 at this
    // end, the server resets it.
    bp.player_id = p->GetCamera()->id;

    
    /* Create a reliable packet of size 7 containing "packet\0" */
    ENetPacket * packet = enet_packet_create (&bp, 
                                              sizeof(bp), 
                                              0);

    assert(server);
    /* Send the packet to the peer over channel id 0. */
    /* One could also broadcast the packet by         */
    /* enet_host_broadcast (host, 0, packet);         */
    enet_peer_send (server, 0, packet);
    
    
    /* One could just use enet_host_service() instead. */
    enet_host_flush (client);
}

void CEnetDriver::ReceiveData()
{
    ENetEvent event;
    
    /* Wait up to 1 milliseconds for an event. */
    while (enet_host_service (client, & event, 1) > 0)
    {
        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            Log ("A new client connected from %x:%u. !!!!!!!!\n", 
                    event.peer -> address.host,
                    event.peer -> address.port);

            /* Store any relevant client information here. */
            // event.peer -> data = "Client information";

            break;

        case ENET_EVENT_TYPE_RECEIVE:
        {
            /*printf ("A packet of length %u containing %s was received from %s on channel %u.\n",
                    event.packet -> dataLength,
                    event.packet -> data,
                    event.peer -> data,
                    event.channelID);*/

            BungPacket *bp = (BungPacket *)event.packet->data;

           CReporter::Report(CReporter::R_DEBUG,
				"Packet: pos=(%f, %f, %f) bearing=(%f,%f) id=%d", 
				bp->x_pos, bp->y_pos, bp->z_pos, bp->facing, bp->pitch, 
                bp->player_id);
            
            world.entities->UpdateEntity(
                bp->player_id, 
				Vector3f(bp->x_pos, bp->y_pos, bp->z_pos),
				Vector3f(bp->facing, bp->pitch, 0.0f),
				bp->velocity
			);


            /* Clean up the packet now that we're done using it. */
            enet_packet_destroy (event.packet);
            
            break;
        }
        case ENET_EVENT_TYPE_DISCONNECT:
            printf ("%p disconected. !!!!!!!\n", event.peer -> data);

            /* Reset the peer's client information. */
            event.peer -> data = NULL;
	    break;
	default:
	    break;
        }
    }
}

