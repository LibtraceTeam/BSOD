/* $Header$ $Log$
/* $Header: /home/cvs/wand-general/bsod/client/main.cpp,v 1.4 2004/02/17 01:59:56 stj2 Exp $ Revision 1.5  2004/02/17 02:05:21  stj2
/* $Header: /home/cvs/wand-general/bsod/client/main.cpp,v 1.4 2004/02/17 01:59:56 stj2 Exp $ Still trying to get cvs tags correct.
/* $Header: /home/cvs/wand-general/bsod/client/main.cpp,v 1.4 2004/02/17 01:59:56 stj2 Exp $
 $Header$ Revision 1.4  2004/02/17 01:59:56  stj2
 $Header$ Cvs tags will be correct at one point. Surely.
 $Header$ */ 
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

	srand(time(NULL));
	CMesh::InitIndices();
	//CCollider::Initialise();

	try {
		int width = 800, height = 600, bpp = 16;
		string netHost = "localhost:2500";
		Vector3f startLoc;
		float pitch = 0, heading = 0;
		bool fullScreen = false;
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

		{	string disp("opengl");
			config->GetGlobal("display", &disp);
			if(disp.compare("direct3d") == 0 || disp.compare("d3d") == 0)
				dispType = CSystemDriver::DISPLAY_DIRECT3D;
		}

		Log("Config file parsed.\n");

		world.actionHandler = new CActionHandler;

		world.display = world.sys->InitDisplay(width, height, bpp, fullScreen, 
				dispType);
		world.display->Initialise();
		
		loadingScreen = auto_ptr<CLoadingScreen>(new CLoadingScreen(width, height));
		
		world.tree = new COctree;
		
		world.resources = new CResourceManager;

		world.entities = new CEntityManager;

		world.entities->GetPlayer()->SetPosition(startLoc);
		
		world.entities->GetPlayer()->SetBearing(Vector3f(pitch,heading,0));
		world.entities->GetPlayer()->GetCamera()->
			SetBearing(Vector3f(pitch,heading,0));


		loadingScreen->AddMessage("Creating particle visualisation...");
		world.partVis = new CPartVis();
		world.entities->AddEntity(world.partVis);

		loadingScreen->AddMessage("Initialising network and connecting...");
		world.netDriver = CNetDriver::Create();
		world.netDriver->Connect(netHost);

		//loadingScreen->AddMessage("Loading level archive...");
		//CVFS::LoadArchive("data/bung.barc");

		//loadingScreen->AddMessage("Loading texture archive...");
		//CVFS::LoadArchive("data/textures.barc");
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
