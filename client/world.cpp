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
	debug_display = false;
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

}

void CWorld::Draw()
{
	static int frames = 0;
	static float last = world.sys->TimerGetTime();
	static float fps = 0;
	float now = world.sys->TimerGetTime();
	char tbuf[256];

	// time_t does not seem to be unsigned in Windows and this appears to cause
	// problems when the client connects to the server after perviously disconnecting
	// malformed timestamp info? Using 64bit timestamps fixes this.
#ifdef _WIN32
	uint32 timestamp = partVis->GetLastTimestamp();
	__time64_t ts = (const __time64_t)timestamp;
	tm *timeptr = _localtime64( &ts );
	strftime(tbuf, 255, "%c", timeptr );
#else
	time_t timestamp = partVis->GetLastTimestamp();
	strftime( tbuf, 255, "%c", localtime(timestamp) );
#endif
	
	// %a %b %d %R %Z %G

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
	
    if(debug_display) {
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
		list<string>::const_iterator i = CReporter::GetLog().begin();
		int count = 0;
		for(; i != CReporter::GetLog().end(); ++i, count++) {
			if(count < 10)
				display->DrawString2(10, 60 + count * 20, *i);
		}
		

		display->End2D();
    } else {
		float w = 270, h = 20;
		float x = 0, y = display->GetHeight() - h;
		string str;

		str = tbuf;
		
		display->Begin2D();
		display->SetColour(0.4f, 0.4f, 0.9f, 0.75f);
		display->BindTexture(NULL);
		display->SetBlend(true);
		display->SetBlendMode(CDisplayManager::Transparent);
		display->Draw2DQuad((int)x, (int)y, (int)(x + w), (int)(y + h));
		display->SetBlendMode(CDisplayManager::Multiply);
		display->SetBlend(false);
		display->SetColour(1.0f, 1.0f, 1.0f);
		display->DrawString2((int)x+3, (int)y+1, str);
		display->End2D();
    }


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

