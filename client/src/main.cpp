#include "main.h"

App *App::mSingleton = NULL;



#include <sys/select.h>

int kbhit()
{
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}

int inputThread(void *unused){

	while(true){

		char c = std::cin.get();
		
		App::S()->onKey(c);
	
	}
}

void App::onKey(char c){

	if(c == 10){
		return;
	}

	//LOG("Hit %d\n", c);

	if(c == 'x')	bOptions[OPTION_ROTATE_X] = !bOptions[OPTION_ROTATE_X];			
	if(c == 'y')	bOptions[OPTION_ROTATE_Y] = !bOptions[OPTION_ROTATE_Y];			
	if(c == 'z')	bOptions[OPTION_ROTATE_Z] = !bOptions[OPTION_ROTATE_Z];		

	if(c == 'w')	fRot[0] += 10;
	if(c == 's')	fRot[0] -= 10;
	if(c == 'a')	fRot[1] += 10;
	if(c == 'd')	fRot[1] -= 10;		
	if(c == 'q')	fRot[2] += 10;
	if(c == 'e')	fRot[2] -= 10;


	if(c == '-')	fZoom += 1;
	if(c == '=')	fZoom -= 1;


	if(c == 'r'){
		for(int i=0;i<3;i++){
			fRot[i] = 0.0f;
			fZoom = 0.0f;
			bDrag = false;
		}	
		fRot[0] = 20;
	
	
		bOptions[OPTION_ROTATE_X] = false;
		bOptions[OPTION_ROTATE_Y] = false;
		bOptions[OPTION_ROTATE_Z] = false;		
	}
}





/*********************************************
		Application init
**********************************************/
int App::init(App *a, int argc, char **argv){

	mSingleton = a;

	//Banner
	LOG("\n");
	
	LOG("**********************************************\n");
	LOG("               BSOD Client\n");
	LOG("**********************************************\n");
		
	srand(time(0));
	bConnected = false;
	
	//Load the configuration
	if(!loadConfig()){
		LOG("Something went wrong with the config file, bailing out\n");
		return 0;
	}			
	
	//and SDL_net
	if(SDLNet_Init()==-1) {
	    LOG("SDLNet_Init: %s\n", SDLNet_GetError());
	    return 2;
	}
	
	//Connect
	if(mServerAddr != ""){
		if(!openSocket()){
			LOG("Something went wrong with the socket, bailing out\n");
			return 0;
		}
	}else{
		LOG("WARNING: No server specified - will generate phoney test data!\n");
	}
					 	
	//Start SDL
	if(SDL_Init(0)==-1) {
	    LOG("SDL_Init: %s\n", SDL_GetError());
	    return 1;
	}
	
			
	//Make the window	
	if(!utilCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SCREEN_FULLSCREEN)){
		ERR("Couldn't make window!\n");
		utilShutdown(1);
	}

	
	keyInit();
		
	//Texturing
	texInit();
	
	//font
	initFont();
		
	iFPS = 0;
	fTimeScale = iFPS / 1000;
	
	initParticleSystem();
	
	fZoom = 0.0f;
	iTime = 0;
	
	//Set up the root UIScreen so that the module init can do GUI stuff if it really wants
	mUIRoot.initGeneric("screen", Vector2(0,0), Vector2(SCREEN_WIDTH, SCREEN_HEIGHT));
	
	//Rotataion
	for(int i=0;i<3;i++){
		fRot[i] = 0.0f;
	}
	
	fRot[0] = 20;
	
	//Set default options
	setOption(OPTION_ROTATE_X, false);
	setOption(OPTION_ROTATE_Y, false);
	setOption(OPTION_ROTATE_Z, false);
		
	//Load the rendering module
	loadModule(mRenderModule);
			
	//Make the application GUI
	makeGUI();	
	
	for(int i=0;i<MAX_FLOW_DESCRIPTORS;i++){
		mFlowDescriptors[i] = NULL;
	}
				
	//If we didn't connect, add some fake FDs
	if(!isConnected()){
		for(int i=0;i<MAX_FLOW_DESCRIPTORS;i++){
			addFlowDescriptor(i, Color(randFloat(0, 1), randFloat(0, 1), randFloat(0, 1)), "FlowType " + toString(i));
		}
	}
	
	iLastFrameTicks = SDL_GetTicks();
	
	SDL_Thread *thread;
    int i;

    thread = SDL_CreateThread(inputThread, NULL);

	LOG("Loaded, about to go to eventLoop\n");	
		
	//And hand off control to the event loop
	utilEventLoop();
					
	utilShutdown(0);
		
	return 0;
}


/*********************************************
				Entry
**********************************************/
int main( int argc, char **argv )
{		
	App *a = new App();
	int r = a->init(a, argc, argv);	
	delete a;
	
	printf("Goodbye\n");
	
	return r;
}
