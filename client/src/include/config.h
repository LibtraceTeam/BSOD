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
#define SLAB_SIZE 40
#define GUI_HIDE_DELAY 5.0f //seconds

//Toggles between std::map and unordered_map
#ifndef _WINDOWS 
	#define USE_TR1 //On Windows we don't have TR1, at least not without pain
	#define PACKED __attribute__((packed));
#else
	#define PACKED //MSVC doesn't have __attribute__
	#define vsnprintf _vsnprintf //Windows puts a _ on the start of this...
#endif

//UDP ports
#define UDP_SERVER_PORT 2080 //the base port the server listens to broadcast on
#define UDP_PORT_RANGE 5

//Displayed in the options window
#define CLIENT_VERSION 0.1

