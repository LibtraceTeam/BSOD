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
#include "camera.h"
#include "loading_screen.h"
#include "player.h"
#include "sound.h"
#include "partflow.h"
#include "partvis.h"


#include "md3.h"

#include "misc.h"

#include "lib/libconfig.h"

int BungMain(int argc, char *argv[])
{
	// It is probably best to set up debug/error logging first :) - Sam
	FILE *log = fopen("log.txt", "w");
	ASSERT(log);
	CReporter::SetOutput(log);
	CReporter::Report(CReporter::R_DEBUG, "Reporter initialised.\n");

	auto_ptr<CLoadingScreen> loadingScreen;

	srand((unsigned int)time(NULL));
	CMesh::InitIndices();

	try {
		int width = 800, height = 600, bpp = 16;
		char *netHost = strdup("localhost:32400");
		double startx = -10.0f;
		double starty = 10.0f;
		double startz = 0.0f;
		double pitch = 0, heading = 0;
		bool fullScreen = false;
		double size = 1.0f;
		double speed = 1.0f;
		bool jitter = true;
		bool billboard = false;
		bool do_gcc = true;
		bool matrix_mode = false;
		double alpha = 0.5f;
		char *particle = strdup("data/particle.png");
		bool show_menu = false;
		bool show_cursor = false;
		double start_x = -10.0f;
		double end_x = 10.0f;
		bool screen_saver = false;
		char* disp = strdup("opengl");
		char* configfile = "bsod.conf";


		CSystemDriver::DisplayType dispType = CSystemDriver::DISPLAY_OPENGL;

		Log("Parsing config file.\n");

		config_t main_config[] = {
		{ "width", 		TYPE_INT|TYPE_NULL, &width },
		{ "height", 		TYPE_INT|TYPE_NULL, &height },
		{ "bpp", 		TYPE_INT|TYPE_NULL, &bpp },
		{ "fullscreen", 	TYPE_BOOL|TYPE_NULL, &fullScreen },
		{ "pitch",		TYPE_DOUBLE|TYPE_NULL, &pitch },
		{ "heading",		TYPE_DOUBLE|TYPE_NULL, &heading },
		{ "jitter",		TYPE_BOOL|TYPE_NULL, &jitter },
		{ "pointsprites",	TYPE_BOOL|TYPE_NULL, &billboard },
		{ "gc",			TYPE_BOOL|TYPE_NULL, &do_gcc },
		{ "matrix_mode",	TYPE_BOOL|TYPE_NULL, &matrix_mode },
		{ "particle_opacity",	TYPE_DOUBLE|TYPE_NULL, &alpha },
		{ "show_menu",		TYPE_BOOL|TYPE_NULL, &show_menu },
		{ "show_cursor",	TYPE_BOOL|TYPE_NULL, &show_cursor },
		{ "start_x",		TYPE_DOUBLE|TYPE_NULL, &start_x },
		{ "end_x",		TYPE_DOUBLE|TYPE_NULL, &end_x },
		{ "screen_saver",	TYPE_BOOL|TYPE_NULL, &screen_saver },
		{ "server",		TYPE_STR|TYPE_NULL, &netHost },
		{ "particle",		TYPE_STR|TYPE_NULL, &particle },
		{ "startx",		TYPE_DOUBLE|TYPE_NULL, &startx },
		{ "starty",		TYPE_DOUBLE|TYPE_NULL, &starty },
		{ "startz",		TYPE_DOUBLE|TYPE_NULL, &startz },
		{ "driver",		TYPE_STR|TYPE_NULL, &disp },
		{ "size" ,		TYPE_DOUBLE|TYPE_NULL, &size },
		{ "speed",		TYPE_DOUBLE|TYPE_NULL, &speed },
		{ 0, 0, 0 },
		};

		if (argc>1) {
			configfile = argv[1];
		}

		if (parse_config(main_config, configfile)) {
			Log("Bad config file %s, giving up\n", configfile);
			return -1;
		}

		Log("Config file parsed.\n");
		Log("Size: %f\n", size);

		/* Figure out which display driver we're using */
		if (strcmp(disp,"direct3d") == 0 
		  ||strcmp(disp,"d3d") == 0)
			dispType = CSystemDriver::DISPLAY_DIRECT3D;
		free(disp);

		/* Right, setup the world */
		world.actionHandler = new CActionHandler;

		/* Set the title */
		char title[512];
		strcpy( title, "BSOD: " );
		strcat( title, netHost );
		world.display = world.sys->InitDisplay(width, height, bpp, 
				fullScreen, 
				dispType, title);

		world.display->Initialise( billboard );
		
		loadingScreen = auto_ptr<CLoadingScreen>(new CLoadingScreen(width, height));
		
		world.tree = new COctree;
		
		world.resources = new CResourceManager;

		world.entities = new CEntityManager;

		/* Position the user and set them looking the right direction */
		world.entities->GetPlayer()->SetPosition(
				Vector3f(startx,starty,startz));
		
		world.entities->GetPlayer()->SetBearing(
				Vector3f(pitch,heading,0));

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
		world.partVis->global_alpha = alpha;
		world.partVis->no_gui = !show_menu;
		world.actionHandler->no_cursor = !show_cursor;
		world.partVis->start_x = start_x;
		world.partVis->end_x = end_x;
		world.actionHandler->screen_saver = screen_saver;
		if( screen_saver )
			Log( "Running as a screensaver\n" );

		world.partVis->Initialise();
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
