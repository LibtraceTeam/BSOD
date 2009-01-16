#include "main.h"

App *App::mSingleton = NULL;
int returnCode = 0;

float fParticleFPS = 0.025;

/*********************************************
		Application init
**********************************************/
int App::init(App *a, int argc, char **argv){

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

	//Connect	
	if(mServerAddr != ""){
		if(!openSocket()){
			LOG("Something went wrong with the socket, bailing out\n");
			return 0;
		}
	}else{
		LOG("No server specified\n");
	}
	
			
	//Make the window	
	if(!utilCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SCREEN_FULLSCREEN)){
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
	
	//font
	initFont();
		
	//Time etc
	iFPS = 0;
	fTimeScale = 1.0f/60.0f;	
	fZoom = 0.0f;
	fGUITimeout = 0.0f;
	
	//Particle system
	if(!initParticleSystem()){
		return 1;
	}
				
	//Flow manager
	mFlowMgr = new FlowManager();
	mFlowMgr->init();
		
	//Camera
	camSetPos(0, 0, SLAB_SIZE);
	camLookAt(0, 0, 0); 
	resetCam();
	
	//gui
	initGUI();
				
	//Current time
	iLastFrameTicks = SDL_GetTicks();
			
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
		delete mParticleSystem;
	}
	
	shutdownGUI();
	
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
