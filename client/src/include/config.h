/*******************************************************************************
							BSOD2 Client - config.h
							
 This provides some #defines for compile-time options and defaults for config
 file options. 
*******************************************************************************/

#define DEFAULT_PORT 54567
#define CONFIG_FILE "bsod2.cfg"
#define PARTICLE_FPS 0.025 //40fps (0.3333 = 30fps)
#define MAX_FLOW_DESCRIPTORS 64
#define MAX_PARTICLES 1000000 //global cap of 1m particles for particle systems
							  //that have a limit

#define OPTION_RESET_ROTATION 0
#define OPTION_ROTATE_X 1
#define OPTION_ROTATE_Y 2
#define OPTION_ROTATE_Z 3

//#define ENABLE_PS_SHADERS

//gui
#define WINDOW_TITLE_HEIGHT 24

//screen width, height, and bit depth
//These used to be numbers, in order to keep
//compatibility, they changed to point to the vars in App 
#define SCREEN_WIDTH  App::S()->iScreenX
#define SCREEN_HEIGHT App::S()->iScreenY
#define SCREEN_BPP     32
#define SCREEN_FULLSCREEN App::S()->bFullscreen

#define PI 3.14159265



//Pos defines
#define MAX_FLOWS 1024
#define SLAB_SIZE 20

//Toggles between std::map and unordered_map
#define USE_TR1
