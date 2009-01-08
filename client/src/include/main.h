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
#include "flowmanager.h"

/*********************************************
			Application class
**********************************************/
class App{

/*********************************************
		Private member variables
**********************************************/
	
	//Mouse (2D)
	bool bMouse[8];
	int iMouseX, iMouseY;
	
	//Rotation
	float fZoom;
	float fRot[3];

	//FPS counter
	int iFPS;
	int iFrameCounter;
	int iLastFrameTicks;	
	
	//main.cpp
	static App *mSingleton;
	
	//Timers for various bits of logic
	float fNextFlowUpdate;		

	//gl_util.cpp
	bool done;
	int videoFlags;
	SDL_Surface *surface;
		
	//particles.cpp
	int iParticleMethod;
	IParticleSystem *mParticleSystem;	
		
	//Camera (update.cpp)
	float fCameraX, fCameraY, fCameraZ;
	float fLookX, fLookY, fLookZ;
	float fMouseX, fMouseY, fMouseZ;
	Vector2 dragStart;
	Vector2 dragVel;
	bool bDrag;	
		
	//flowmanager.cpp
	FlowManager *mFlowMgr;
	
	//Flow descriptors. 
	map<byte, FlowDescriptor *> mFlowDescriptors;
	
	//socket.cpp
	bool bConnected;
		
/*********************************************
		Private member functions
**********************************************/

	//gl_util.cpp
	void utilEndRender();
	void utilBeginRender();
	void calculateMousePoint();
	int resizeWindow(int h, int w);	
	bool utilCreateWindow(int sizeX, int sizeY, int bpp, bool fullscreen);
	
	//render.cpp
	void calcFps();
	void render2D();
	void renderMain();
	void drawStatusBar();
	
	//texture.cpp
	bool texInit();
	void texShutdown();	
	void texRegenerate(Texture *t, byte *buffer, int width, int height);

	//Keys
	void keyInit();
	bool keyDown(int code);
	void keySetState(int code, bool pressed);
	void onMouseEvent(int code, int eventType);
	bool mouseDown(int button){return bMouse[button];}	
	void handleKeyEvent( SDL_keysym *keysym , int type );

	//update.cpp
	void updateMain();
	void utilEventLoop();
	
	//particles.cpp
	bool initParticleSystem();	
	
	//font.cpp
	void initFont();
					
	//socket.cpp
	bool openSocket();
	void closeSocket();
	void updateSocket();
	bool isConnected(){return bConnected;}
		
	//config.cpp
	bool loadConfig();
	
	//gui.cpp	
	void initGUI();
	void renderGUI();
	void makeMenuButtons();
	void makeServerWindow();
	void makeProtocolWindow();
	void resizeGUI(int x, int y);
	bool processGUIEvent(SDL_Event e);
	void addProtocolEntry(string name, Color col, int index);
	
	//GUI callback handlers
	bool onWndClose(const CEGUI::EventArgs&);
	bool onProtocolClicked(const CEGUI::EventArgs&);
	bool onMenuButtonClicked(const CEGUI::EventArgs&);
	bool onServerListClicked(const CEGUI::EventArgs&);
	bool onMouseCursorChanged(const CEGUI::EventArgs&);
	bool onServerButtonClicked(const CEGUI::EventArgs&);
	bool onProtocolButtonClicked(const CEGUI::EventArgs&);
	
	//Flow descriptors
	void addFlowDescriptor(byte id, Color c, string name);
	FlowDescriptor *getFD(byte i){return mFlowDescriptors[i];}
			
/*********************************************
				Public
**********************************************/
public:
	virtual ~App(){}
	
	//Singleton
	static App *S(){return mSingleton;}
	static void setS(App *s){mSingleton = s;}

	//Initial setup
	int init(App *a, int argc, char **argv);

	//font.cpp	
	void writeText(int x, int y, const char *fmt, ...);
	void writeTextCentered(int x, int y, const char *fmt, ...);

	//misc.cpp
	float randFloat();
	int randInt(int low, int high);
	float randFloat(float low, float high);
	
	//gl_util.cpp
	void utilCube(float x, float y, float z);
	void utilPlane(float x, float y, float z);
	Vector2 utilProject(float x, float y, float z);
	
	//Update.cpp
	void utilShutdown(int returnCode);
	void notifyShutdown(){done = true;}

	//Camera movement
	void endDrag();
	void beginDrag();
	void camMove(float x, float y, float z);
	void camLookAt(float x, float y, float z);
	void camSetPos(float x, float y, float z);
	
	//texture.cpp
	Texture *texGet(string name);
	Texture *texLoad(string name, int flags);
	Texture *texGenerate(string name, byte *buffer, int width, int height);
	
	//particle management
	IParticleSystem *ps(){return mParticleSystem;}
	
	//Mouse
	Vector2 getMouse(){return Vector2(iMouseX, iMouseY);}
						
	//Stats
	int getFPS(){return iFPS;}
	
	
/*********************************************
		Configuration settings
**********************************************/
	
	//Screen resolution
	int iScreenX;
	int iScreenY;
	bool bFullscreen;
	
	//server
	string mServerAddr;
	int iServerPort;
	
	//rendering
	float fParticleSizeScale;
	float fParticleSpeedScale;
	
	//Skip
	int iDropPacketThresh;
	int iDropFlowThresh;
		
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

