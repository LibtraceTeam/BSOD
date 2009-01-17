/*******************************************************************************
							BSOD2 Client - config.h
							
 This provides some #defines for compile-time options and defaults for config
 file options. 
*******************************************************************************/

#define DEFAULT_PORT 54567
#define CONFIG_FILE "bsod2.cfg"
#define PARTICLE_FPS fParticleFPS //hack!
#define MAX_FLOW_DESCRIPTORS 64
#define MAX_PARTICLES 1000000 //global cap of 1m particles for particle systems
							  //that have a limit

//screen width, height, and bit depth
//These used to be numbers, in order to keep
//compatibility, they changed to point to the vars in App 
#define SCREEN_WIDTH  App::S()->iScreenX
#define SCREEN_HEIGHT App::S()->iScreenY
#define SCREEN_BPP     32
#define SCREEN_FULLSCREEN App::S()->bFullscreen

#ifndef PI
	#define PI 3.14159265
#endif

#define SLAB_SIZE 40

#define GUI_HIDE_DELAY 5.0f

//Toggles between std::map and unordered_map
#ifndef _WINDOWS //On Windows we don't have TR1, at least not without jumping through hoops
#define USE_TR1
#endif

#ifndef _WINDOWS
#define PACKED __attribute__((packed));
#else
#define PACKED //MSVC doesn't have __attribute__
#define vsnprintf _vsnprintf //Windows for some reason puts a _ on the start of this...
#endif

#define CAMERA_SPEED 0.5f

//UDP ports
#define UDP_SERVER_PORT 2080 //the port the server listens to broadcast on

#define CLIENT_VERSION 0.1

const float infinity = std::numeric_limits<float>::infinity();
