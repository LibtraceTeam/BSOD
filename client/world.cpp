/* $Header$ $Log$
/* $Header: /home/cvs/wand-general/bsod/client/world.cpp,v 1.3 2004/02/17 01:59:56 stj2 Exp $ Revision 1.4  2004/02/17 02:05:21  stj2
/* $Header: /home/cvs/wand-general/bsod/client/world.cpp,v 1.3 2004/02/17 01:59:56 stj2 Exp $ Still trying to get cvs tags correct.
/* $Header: /home/cvs/wand-general/bsod/client/world.cpp,v 1.3 2004/02/17 01:59:56 stj2 Exp $
 $Header$ Revision 1.3  2004/02/17 01:59:56  stj2
 $Header$ Cvs tags will be correct at one point. Surely.
 $Header$ */ 
#include "stdafx.h"
#include "world.h"
#include "entity_manager.h"
#include "display_manager.h"
#include "system_driver.h"
#include "vfs.h"
#include "matrix.h"
#include "octree.h"
#include "camera.h"
#include "action.h"
#include "net_driver.h"
#include "reporter.h"
#include "misc.h"
#include "player.h"
#include "sound.h"
#include "partvis.h"

// temp
#include "gl/gl_display_manager.h"
// temp for resource manager
#include "md3.h"

CWorld world;

#include "actor.h"


CWorld::CWorld()
{
	tree = NULL;
	sys = NULL;
	display = NULL;
	actionHandler = NULL;
	physicsHandler = NULL;
	netDriver = NULL;
	soundProvider = NULL;
	entities = NULL;
	resources = NULL;
	partVis = NULL;
}

CWorld::~CWorld()
{
}

void CWorld::Cleanup()
{
	if(netDriver)
		delete netDriver;
	// Deleted by the entity manager (this is kinda hax?) - Sam
	//if(partVis)
	//    	delete partVis;
	if(tree)
		delete tree;
	if(entities)
		delete entities;
	if(sys)
		delete sys;
	if(display)
		delete display;
	if(actionHandler)
		delete actionHandler;
	if(soundProvider)
		delete soundProvider;
	if(resources)
		delete resources;

}

void CWorld::Draw()
{
	static int frames = 0;
	static float last = world.sys->TimerGetTime();
	static float fps = 0;
	float now = world.sys->TimerGetTime();
	time_t timestamp = partVis->GetLastTimestamp();
	char tbuf[256];

	strftime(tbuf, 256, "%a %H:%M:%S", localtime(&timestamp));

	display->BeginFrame2();

	display->SetCameraTransform( *world.entities->GetPlayer()->GetCamera() );

	tree->Draw(*world.entities->GetPlayer()->GetCamera());
	entities->DrawEntities();
	
	frames++;

	if(now - last > 600.0f)
	{
		fps = (float)frames / ((now - last) / 1000.0f);
		last = now;
		frames = 0;
	}
	
	display->Begin2D();
	display->SetColour(0.2f, 0.1f, 0.3f, 0.8f);
	display->BindTexture(NULL);
	display->SetBlend(true);
	display->SetBlendMode(CDisplayManager::Transparent);
	display->Draw2DQuad(0, 8, 800, 57);
	display->SetBlendMode(CDisplayManager::Multiply);
	display->SetBlend(false);
	display->SetColour(1.0f, 1.0f, 1.0f);

	display->DrawString2(10, 10, 
		bsprintf("FPS: %3.3f nodes:%d meshs:%d triangles:%d t:%s", fps, 
			COctree::nodes_drawn, display->GetNumMeshesDrawn(),
			display->GetNumTrianglesDrawn(), tbuf)
		);

	display->DrawString2(10, 35, 
		bsprintf("(%f,%f,%f) (pitch:%f,heading:%f)", 
			world.entities->GetPlayer()->GetPosition().x, 
			world.entities->GetPlayer()->GetPosition().y, 
			world.entities->GetPlayer()->GetPosition().z,
			world.entities->GetPlayer()->GetBearing().x, 
			world.entities->GetPlayer()->GetBearing().y)
		);

	// Debugging:
	display->DrawString2(10, 60, CReporter::GetLog().front());

	display->End2D();

	display->EndFrame2();
}

/**
 * This method updates all world entities. :P
 * rawk - I am god - Jesse
 * Jesse is a god - Sam
 */
void CWorld::Update(float diff)
{
	// This means we only send 'net data 10 times a second
	/*if(count_net > (1.0f / net_ps)) {
		count_net = 0.0f;
		netDriver->SendData(world.entities->GetPlayer());
		netDriver->ReceiveData();
	}*/
	netDriver->ReceiveData();
	
	entities->Update(diff);
}

