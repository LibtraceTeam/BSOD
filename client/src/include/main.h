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
				Private
**********************************************/
	
	//Mouse (2D)
	int iMouseX, iMouseY;
	bool bMouse[8];
	float fRot[3];
	float fZoom;

	//FPS counter
	int iFPS;
	int iFrameCounter;
	int iLastFrameTicks;
	
	//Timers for various bits of logic
	float fNextParticleUpdate;
	float fNextFlowUpdate;
		

	//gl_util.cpp
	bool done;
	SDL_Surface *surface; //used for the screen
	int videoFlags;
	
	bool utilCreateWindow(int sizeX, int sizeY, int bpp, bool fullscreen);
	void utilBeginRender();
	void utilEndRender();
	void calculateMousePoint();
	int resizeWindow(int h, int w);
	
	
	//Render.cpp
	void renderMain();
	void render2D();
	void calcFps();
	void drawStatusBar();
	
	//Texture.cpp
	bool texInit();
	void texShutdown();	
	void texRegenerate(Texture *t, byte *buffer, int width, int height);

	//Keys
	void keyInit();
	void keySetState(int code, bool pressed);
	bool keyDown(int code);
	bool mouseDown(int button){return bMouse[button];}
	void onMouseEvent(int code, int eventType);
	void handleKeyEvent( SDL_keysym *keysym , int type );

	//Update.cpp
	void updateMain();
	void utilEventLoop();
	
	//Font.cpp
	void initFont();
	
	//particles.cpp
	IParticleSystem *mParticleSystem;	
	bool initParticleSystem();	
	int iParticleMethod;
		
	//main.cpp
	static App *mSingleton;
		
	//socket.cpp
	TCPsocket mClientSocket;
	SDLNet_SocketSet mSocketSet;
	vector<byte> mDataBuf;
	bool bSkipFlow;
	bool bSkipPacket;
	bool bConnected;
	unsigned int iTime;
	
	bool openSocket();
	void updateSocket();
	void closeSocket();
	bool isConnected(){return bConnected;}
		
	//config.cpp
	bool loadConfig();
	
	//gui.cpp	
	void initGUI();
	void makeProtocolWindow();
	void makeMenuButtons();
	void makeServerWindow();
	void addProtocolEntry(string name, Color col, int index);
	void renderGUI();
	bool processGUIEvent(SDL_Event e);
	void resizeGUI(int x, int y);
	
	//GUI callback handlers
	bool onMenuButtonClicked(const CEGUI::EventArgs&);
	bool onProtocolClicked(const CEGUI::EventArgs&);
	bool onProtocolButtonClicked(const CEGUI::EventArgs&);
	bool onWndClose(const CEGUI::EventArgs&);
	bool onServerButtonClicked(const CEGUI::EventArgs&);
	bool onServerListClicked(const CEGUI::EventArgs&);
			
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
	int randInt(int low, int high);
	float randFloat();
	float randFloat(float low, float high);
	
	//gl_util.cpp
	void utilCube(float x, float y, float z);
	void utilPlane(float x, float y, float z);
	void utilShutdown(int returnCode);
	void notifyShutdown(){done = true;}
	Vector2 utilProject(float x, float y, float z);

	//Camera movement
	void camMove(float x, float y, float z);
	void camLookAt(float x, float y, float z);
	void camSetPos(float x, float y, float z);
	void beginDrag();
	void endDrag();
	
	//texture.cpp
	Texture *texLoad(string name, int flags);
	Texture *texGet(string name);
	Texture *texGenerate(string name, byte *buffer, int width, int height);
	
	//particle management
	IParticleSystem *ps(){return mParticleSystem;}
	
	//Mouse
	Vector2 getMouse(){return Vector2(iMouseX, iMouseY);}
			
	//Camera
	float fCameraX, fCameraY, fCameraZ;
	float fLookX, fLookY, fLookZ;
	float fMouseX, fMouseY, fMouseZ;
	Vector2 dragStart;
	Vector2 dragVel;
	bool bDrag;
			
	//Stats
	int getFPS(){return iFPS;}
	
	//This contains most of the flow placement logic
	FlowManager *mFlowMgr;
	
	//Flow descriptors. TOOD: clean up
	map<byte, FlowDescriptor *> mFlowDescriptors;
	FlowDescriptor *getFD(byte i){return mFlowDescriptors[i];}
	void addFlowDescriptor(byte id, Color c, string name);
	
	
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

