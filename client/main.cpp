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
#include "system_driver.h"
#include "world.h"
#include "octree.h"
#include "entity_manager.h"
#include "display_manager.h"
#include "bezier.h"
#include "levelloader.h"
#include "vfs.h"
#include "texture_manager.h"
#include "reporter.h"
#include "actor.h"
#include "action.h"
#include "exception.h"
#include "net_driver.h"
#include "config.h"
#include "camera.h"
#include "loading_screen.h"
#include "player.h"
#include "sound.h"
#include "partflow.h"
#include "partvis.h"


#include "md3.h"

#include "misc.h"

//
#include "script.h"


int BungMain(int argc, char *argv[])
{
	// It is probably best to set up debug/error logging first :) - Sam
	FILE *log = fopen("log.txt", "w");
	ASSERT(log);
	CReporter::SetOutput(log);
	CReporter::Report(CReporter::R_DEBUG, "Reporter initialised.\n");

	auto_ptr<CScript> config = auto_ptr<CScript>(CScript::Create());
	auto_ptr<CLoadingScreen> loadingScreen;

	srand((unsigned int)time(NULL));
	CMesh::InitIndices();
	//CCollider::Initialise();

	try {
		int width = 800, height = 600, bpp = 16;
		string netHost = "localhost:2500";
		Vector3f startLoc;
		float pitch = 0, heading = 0;
		bool fullScreen = false;
		float size = 1.0f;
		float speed = 1.0f;
		bool jitter = true;
		bool billboard = true;
		bool do_gcc = true;
		bool matrix_mode = false;
		string particle = "data/particle.png";
		CSystemDriver::DisplayType dispType = CSystemDriver::DISPLAY_OPENGL;

		Log("Parsing config file.\n");

		//world.config = new CConfig;
		//world.config->ParseFile("config.xml");
		config->Begin();
		config->ExecuteFile("data/config.lua");

		config->GetGlobal("width", &width);
		config->GetGlobal("height", &height);
		config->GetGlobal("bpp", &bpp);
		config->GetGlobal("network_host", &netHost);
		config->GetGlobal("fullscreen", &fullScreen);
		config->GetGlobal("start_location", &startLoc);
		config->GetGlobal("pitch", &pitch);
		config->GetGlobal("heading", &heading);
		config->GetGlobal("speed", &speed );
		config->GetGlobal("size", &size );
		config->GetGlobal("jitter", &jitter );
		config->GetGlobal("billboard", &billboard );
		config->GetGlobal("particle", &particle );
		config->GetGlobal("do_gcc", &do_gcc );
		config->GetGlobal("matrix_mode", &matrix_mode );

		{	string disp("opengl");
			config->GetGlobal("display", &disp);
			if(disp.compare("direct3d") == 0 || disp.compare("d3d") == 0)
				dispType = CSystemDriver::DISPLAY_DIRECT3D;
		}

		Log("Config file parsed.\n");

		world.actionHandler = new CActionHandler;

		char title[512];
		strcpy( title, "BSOD: " );
		strcat( title, netHost.c_str() );
		world.display = world.sys->InitDisplay(width, height, bpp, fullScreen, 
				dispType, title);
		world.display->Initialise();
		
		loadingScreen = auto_ptr<CLoadingScreen>(new CLoadingScreen(width, height));
		
		world.tree = new COctree;
		
		world.resources = new CResourceManager;

		world.entities = new CEntityManager;

		world.entities->GetPlayer()->SetPosition(startLoc);
		
		world.entities->GetPlayer()->SetBearing(Vector3f(pitch,heading,0));
		world.entities->GetPlayer()->GetCamera()->
			SetBearing(Vector3f(pitch,heading,0));

		loadingScreen->AddMessage("Initialising network and connecting...");
		world.netDriver = CNetDriver::Create();
		world.netDriver->Connect(netHost);

		loadingScreen->AddMessage("Creating particle visualisation...");
		world.partVis = new CPartVis( matrix_mode );
		world.entities->AddEntity(world.partVis);
		world.partVis->global_speed = speed;
		world.partVis->global_size = size;
		world.partVis->jitter = jitter;
		world.partVis->billboard = billboard;
		world.partVis->particle_img = particle;
		world.partVis->do_gcc = do_gcc;
	}
	catch(string error)
	{
		CReporter::Report(CReporter::R_FATAL, error);
		return -1;
	}
	catch(CException error)
	{
		CReporter::Report(CReporter::R_FATAL, error.GetMessage() );
		return -1;
	}

	try {
		/*loadingScreen->AddMessage("Loading level...");
		CLevelLoader lev;

		string levelName = "data/bung.barc/levels/q3/bungmap.blev";
		
		config->GetGlobal("level_name", &levelName);
		config->End();

		lev.LoadLevel(levelName, *world.tree, *loadingScreen);
		*/

		delete loadingScreen.release();
	}
	catch(string error)
	{
		CReporter::Report(CReporter::R_FATAL, error);
		return -2;
	}
	catch(CException error)
	{
		CReporter::Report(CReporter::R_FATAL, error.GetMessage());
		return -1;
	}

	int ret = 0;
	try {
		ret = world.sys->RunMessageLoop();
	}
	catch(string error)
	{
		CReporter::Report(CReporter::R_FATAL, "Old exception thrown: " + error);
		return -1;
	}
	catch(CException error)
	{
		CReporter::Report(CReporter::R_FATAL, error.GetMessage());
		return -1;
	}

	try {
		// We call cleanup here, so we know when we are deleting everything.
		// If we did this in the destructor of world, it would be at some point
		// that is somewhat unspecified, and most importantly, the log would
		// not be open making debugging very hard. This way we have full control
		// over what is happening.
		world.Cleanup();
	} catch(CException error)
	{
		CReporter::Report(CReporter::R_FATAL, error.GetMessage());
		return -1;
	}

	fclose(log);
	
	return ret;
}
