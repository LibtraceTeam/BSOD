/*******************************************************************************
							BSOD2 Client - main.h
							
 Most source files include this to get standard headers. Also contains the 
 main application object.
*******************************************************************************/
#ifndef _MAIN_H
#define _MAIN_H

//Pull in standard libraries
#include "libs.h"

/*********************************************
		 Internal headers
**********************************************/
#include "config.h"
#include "vector.h"
#include "misc.h"
#include "texture.h"
#include "shader.h"
#include "ps_interface.h"
#include "ps_classic.h"
#include "ps_shaders.h"
#include "ps_texture.h"
#include "flowmanager.h"

/*********************************************
			Application class
**********************************************/
class App{

/*********************************************
		Private member variables
**********************************************/
		
	//main.cpp
	static App *mSingleton;		//Used so the flow manager can get at App stuff
		
	//Mouse (2D)
	bool bMouse[8];				//Whether a mouse button is held down
	int iMouseX, iMouseY;		//Screen-space coordinates of the cursor
	
	//Rotation
	float fZoom;				//Zoom amount on (0,0,0)
	float fRot[3];				//Rotation around (1,0,0), (0,1,0) and (0,0,1)

	//FPS counter
	int iFPS;					//Current frames per second
	int iFrameCounter;			
	int iLastFrameTicks;		//Records the SDL_Time() at last frame
	float fUptime;				//Seconds this has been going
	float fCleanupTimer;		//How long till the particle system gets cleaned
	float fTimeScaleScale;		//for scaling the timestep
	
	//gl_util.cpp
	bool done;					//Whether the app is running. 
	int videoFlags;				//SDL window flags
	SDL_Surface *surface;		//SDL rendering surface
		
	//particles.cpp
	int iParticleMethod;		//The type of particle system we need
	IParticleSystem *mParticleSystem; //Pointer to the PS we're using
		
	//Camera (update.cpp)
	float fCameraX, fCameraY, fCameraZ;	//Position of the camera in 3D space
	float fLookX, fLookY, fLookZ;		//View target of the camera in 3D space
	float fMouseX, fMouseY, fMouseZ;	//Where the mouse is pointing in 3D
	Vector2 dragStart;			//Screen-space point where a mouse drag started
	Vector2 dragVel;			//The rotation velocity given by a drag
	bool bDrag;					//Whether the user is dragging
			
	//Flow descriptors. 	
	map<byte, FlowDescriptor *> mFlowDescriptors;
	
	//socket.cpp
	bool bConnected;			//Whether we're connected to a server or not
	int iCurrentUDPPort;		//The current discovery port offset
	time_t iCurrentTime;			//Current timestamp of the trace
		
	//gui.cpp
	float fGUITimeout;			//Seconds until the GUI disappers. >0 = display
	
	//render.cpp
	string mStatusString;			//timestamp + server name

	//Texture flipping options
	bool bTexFlip;
	
		
/*********************************************
		Private member functions
**********************************************/

	//gl_util.cpp
	void utilEndRender();		//Concludes a frame
	void utilBeginRender();		//Begins a frame
	void calculateMousePoint();	//Uses glUnProject to update the 3D mouse state
	int resizeWindow(int h, int w);	//Resizes the GL context
	bool utilCreateWindow(int sizeX, int sizeY, int bpp, bool fullscreen);
	
	//render.cpp
	void calcFps();				//Calculates the frames per second and related
	void render2D();			//Renders the 2D elements 
	void renderMain();			//Primary rendering method
	void drawStatusBar();		//Renders the FPS meter and other status things
	
	//texture.cpp
	bool texInit();				//Starts up the texture system, loads base texs
	void texShutdown();			//Disposes all textures

	//Keys
	void keyInit();				//Sets up the keyboard input system
	bool keyDown(int code);		//Returns if a key is being held down
	void keySetState(int code, bool pressed);	//Invoked by the event handler
	void onMouseEvent(int code, int eventType);	//Invoked by the event handler
	bool mouseDown(int button){return bMouse[button];}	//Like keyDown
	void handleKeyEvent( SDL_keysym *keysym , int type ); 

	//update.cpp
	void resetCam();			//Set the default camera position and zoom
	void updateMain();			//Primary logic update
	void utilEventLoop();		//Main loop. Will not return till termination
	void generateTestData();	//Create some useless particle effects
	
	//particles.cpp
	bool initParticleSystem();	//Picks the best particle system and inits it
	
	//font.cpp
	void initFont();			//Uses libfreetype2 to load a .ttf
	void shutdownFont();
					
	//socket.cpp
	bool initSocket();
	bool openSocket();			//Connects to the server
	void disconnect(bool notify);//Disconnects from the server
	void closeSocket();			//Shuts down the networking system
	void updateSocket();		//Reads data from the server
	void beginDiscovery();		//Starts the UDP server discovery process
	void updateTCPSocket();		//Reads from our TCP connection
	void updateUDPSocket();		//Ditto from our UDP socket
	void sendDiscoveryPacket(int); //Sends the UDP broadcast discovery packet
	
	//config.cpp
	bool loadConfig();			//Loads bsod2.cfg using libconfuse
	
	//gui.cpp	
	void initGUI();				//Sets up CEGUI and makes the BSOD GUI
	void renderGUI();			//Renders the GUI to the 2D screen
	void shutdownGUI();
	void makeMenuButtons();		
	void makeOptionWindow();
	void makeServerWindow();
	void makeProtocolWindow();
	void makeMessageWindow();
	void resizeGUI(int x, int y); //Invoked by the event handler
	bool processGUIEvent(SDL_Event e); 	//Passes an event to the GUI. If it
										//returns true, then the GUI is assumed
										//to have handled, and it's not 
										//processed further
	void addProtocolEntry(string name, Color col, int index); //Adds an entry
										//to the protocol window
	void clearProtocolEntries(); //Delete the GUI checkboxes
	void clearServerList();		//Called when we send a new discovery packet
	void addServerListEntry(string name, string IP, string port);
	void messagebox(string text, string title = "Message"); 
	void updateGUIConnectionStatus();
	
	//GUI callback handlers
	bool onWndClose(const CEGUI::EventArgs&);
	bool onProtocolClicked(const CEGUI::EventArgs&);
	bool onMenuButtonClicked(const CEGUI::EventArgs&);
	bool onServerListClicked(const CEGUI::EventArgs&);
	bool onOptionSliderMoved(const CEGUI::EventArgs&);
	bool onMouseCursorChanged(const CEGUI::EventArgs&);
	bool onServerButtonClicked(const CEGUI::EventArgs&);
	bool onProtocolButtonClicked(const CEGUI::EventArgs&);
	bool onDarknetCheckboxClicked(const CEGUI::EventArgs&);
	
	//Flow descriptors
	void addFlowDescriptor(byte id, Color c, string name);
	FlowDescriptor *getFD(byte i){return mFlowDescriptors[i];}
	void clearFlowDescriptors();
			
/*********************************************
				Public
**********************************************/
public:
	virtual ~App(){}
	
	//Singleton
	static App *S(){return mSingleton;}
	static void setS(App *s){mSingleton = s;}

	//Initial setup
	int init(App *a, int argc, char **argv);	//Application entry

	//font.cpp - legacy 2D GUI stuff
	void writeText(int x, int y, const char *fmt, ...);	//Writes freetype2 text
	void writeTextCentered(int x, int y, const char *fmt, ...); //Centered text
	float getTextWidth(const char *fmt, ...);

	//misc.cpp
	float randFloat(); //Return a float between 0.0-1.0
	int randInt(int low, int high);	//Return random integer
	float randFloat(float low, float high);	//Return a float between two nums
	
	//gl_util.cpp
	void utilCube(float x, float y, float z); //Render a 3D cube of size xyz
	void utilPlane(float x, float y, float z); //Render a planet of size x/z
	Vector2 utilProject(float x, float y, float z); //Wrapper for gluUnproject
	
	//Update.cpp
	void utilShutdown(int returnCode); //Called automatically on exit
	void notifyShutdown(){done = true;}	//Start the shutdown process

	//Camera
	void endDrag(); //Called when the rotation drag starts
	void beginDrag(); //Called when the rotation drag ends
	void camMove(float x, float y, float z); //Offset the camera position
	void camLookAt(float x, float y, float z); //Set the camera focus
	void camSetPos(float x, float y, float z); //Set the camera position
	void camLook(); //Calls glulookat - part of rendering
	
	//texture.cpp
	Texture *texGet(string name); //Return a loaded texture by name
	Texture *texLoad(string name, int flags); //Load a texture from disk
	Texture *texGenerate(string name, byte *buffer, int buflen);
	void texDelete(Texture *tex);
	
	//particle management
	IParticleSystem *ps(){return mParticleSystem;} 
	
	//Mouse
	Vector2 getMouse(){return Vector2((float)iMouseX, (float)iMouseY);}
						
	//Stats
	int getFPS(){return iFPS;}
	
	//Socket.cpp
	bool isConnected(){return bConnected;}
		
	//flowmanager.cpp
	FlowManager *mFlowMgr;		//Controls the placement of flows, rendering of
								//the two textured planes, rendering of the end
								//points and selection of flows
	
	
/*********************************************
		Configuration settings
**********************************************/
	
	//Screen resolution
	int iScreenX;				//Horizontal screen resolution
	int iScreenY;				//Vertical screen resolution
	bool bFullscreen;			//Whether the window is fullscreened 
	
	//server
	string mServerAddr;			//The current server we're connected to
	int iServerPort;			//The port that server uses
	
	//rendering
	float fParticleSizeScale;	//Global scaling factor applied to size
	float fParticleSpeedScale;	//Global scaling factor applied to speed
	
	//Skip
	int iDropPacketThresh; //The FPS that we start to discard packets below
	int iDropFlowThresh; //The FPS that we start to discard flows below
		
	//The textures for the left and right slabs
	Texture *mLeftTex;
	Texture *mRightTex;
	
	//The client-side overrides for the left and right textures
	string mLeftTexName;
	string mRightTexName;
	
	//Darknet
	bool bShowDarknet;
	bool bShowNonDarknet;
};




/*********************************************
				Globals
**********************************************/

//Logging funcs (misc.cpp)
extern void LOG(const char *fmt, ...);
extern void ERR(const char *fmt, ...);
extern void WARN(const char *fmt, ...);
extern int splitString(const std::string &input, const std::string &delimiter, 
				std::vector<std::string> &results, bool includeEmpties = false);

//Time scaler is global so everything can get it
extern float fTimeScale;
extern float fParticleFPS;

template<typename T> T stringTo(const std::string& s) {
  std::istringstream iss(s);
  T x;
  iss >> x;
  return x;
}

template<typename T> std::string toString(const T& x) {
  std::ostringstream oss;
  oss << x;
  return oss.str();
}


#endif

