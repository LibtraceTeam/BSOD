/* 
 * This file is part of BSOD client
 *
 * Copyright (c) 2011 The University of Waikato, Hamilton, New Zealand.
 *
 * Author: Paul Hunkin
 *
 * Contributors: Shane Alcock
 *
 * All rights reserved.
 *
 * This code has been developed by the University of Waikato WAND research
 * group. For further information please see http://www.wand.net.nz/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $Id$
 */


#include "main.h"

App *App::mSingleton = NULL;
int returnCode = 0;

float fParticleFPS = 0.025;

/*********************************************
		Application init
**********************************************/
int App::init(App *a, int argc, char **argv){

	openLog("bsod.log");

	mSingleton = a;
	mFlowMgr = NULL;

	//Banner
	LOG("\n");
	
	LOG("**********************************************\n");
	LOG("               BSOD Client\n");
	LOG("**********************************************\n");
		
	srand(time(0));
	bConnected = false;
	done = false;
	
	iCurrentTime = 0;

	mParticleSystem = NULL;
	mFlowMgr = NULL;
	
	//Load the configuration
	if(!loadConfig()){
		LOG("Something went wrong with the config file, bailing out\n");
		return 0;
	}	
	
					 	
	//Start SDL
	if(SDL_Init(0)==-1) {
	    LOG("SDL_Init: %s\n", SDL_GetError());
	    return 1;
	}
			
	
	//and SDL_net
	if(SDLNet_Init()==-1) {
	    LOG("SDLNet_Init: %s\n", SDLNet_GetError());
	    return 2;
	}
	
	//Set up the socket
	initSocket();

	
	//Make the window	
	if(!utilCreateWindow(iScreenX, iScreenY, 0, bFullscreen)){
		ERR("Couldn't make window!\n");
		return 1;
	}

	//Input
	keyInit();
	
	for(int i=0;i<8;i++){
		bMouse[i] = false;
	}
		
	//Texturing
	if(!texInit()){
		return 1;
	}
	
	//Flow manager
	mFlowMgr = new FlowManager();
	mFlowMgr->init();
	
	//font
	initFont();
		
	//Time etc
	iFPS = 60;
	fTimeScale = 1.0f/60.0f;	
	fZoom = 0.0f;
	fGUITimeout = 0.0f;
	
	//Particle system
	if(!initParticleSystem()){
		return 1;
	}
				
		
	//Camera
	camSetPos(0, -SLAB_SIZE/3, SLAB_SIZE);
	camLookAt(0, 0, 0); 
	resetCam();
	
	//gui
#ifdef ENABLE_GUI
	try{
		initGUI();
	}
	catch(...) { 
		ERR("Caught CEGUI exception, bailing out!\n");
		return 0;
	}
#endif
			
	//Current time
	iLastFrameTicks = SDL_GetTicks();
	fUptime = 0.0f;
	fTimeScaleScale = 1.0f/1000.0f; //normal scaling
	fCleanupTimer = CLEANUP_TIMER;

	//Connect	
	if(mServerAddr != ""){
		if(!openSocket(true)){
			LOG("Something went wrong with the socket, bailing out\n");
			return 0;
		}
	}else{
		LOG("No server specified\n");
	}
	
			
			
	//At this point, all the setup should be done
	LOG("Loaded, about to go to eventLoop\n");	

		
	//And hand off control to the event loop
	utilEventLoop();
					
	utilShutdown(0);
		
	return 0;
}

/*********************************************
		Exits the program, cleanup
**********************************************/
void App::utilShutdown( int r )
{
	LOG("Shutting down with returnCode %d\n", r);
		
	//Terminate the network connection
	closeSocket();
    
	LOG("Shutting down PS (of type %d)\n", mParticleSystem->getType());
		   
	//Shut down the particle system
	if(mParticleSystem){
		mParticleSystem->shutdown();
	}
	
#ifdef ENABLE_GUI
	shutdownGUI();
#endif
	
	LOG("Freeing textures\n");
		   
	//Free textures
	texShutdown();
	shutdownFont();
	
	//Clean up some other components
	if(mFlowMgr){
		mFlowMgr->shutdown();
		delete mFlowMgr;
	}
	
	//This will ensure we break out of the eventloop
	done = true;    
	
	closeLog();
	
	//Shut down SDL
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
    SDL_Quit( );   
    
    //So main() knows what to return
    returnCode = r;
}


/*********************************************
				Entry
**********************************************/
int main( int argc, char **argv )
{		
	App *a = new App();
	a->init(a, argc, argv);	
	delete a;
	
	printf("Goodbye\n");
		
	return returnCode;
}
