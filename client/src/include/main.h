#ifndef _MAIN_H
#define _MAIN_H

//Toggle pixel-shader particle system
//#define ENABLE_PS_SHADERS


#include "libs.h"

/*********************************************
		 Internal headers
**********************************************/
#include "vector.h"
#include "misc.h"
#include "renderable.h"
#include "texture.h"
#include "module.h"
#include "ps_interface.h"
#include "ps_classic.h"
#include "ps_shaders.h"
#include "gui.h"

#define DEFAULT_PORT 54567
#define CONFIG_FILE "bsod2.cfg"
#define PARTICLE_FPS 0.025 //40fps (0.3333 = 30fps)
#define MAX_FLOW_DESCRIPTORS 64

#define OPTION_RESET_ROTATION 0
#define OPTION_ROTATE_X 1
#define OPTION_ROTATE_Y 2
#define OPTION_ROTATE_Z 3


/*********************************************
				Application class
**********************************************/
class App{

/*********************************************
				Internal stuff
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
	float fNextParticleUpdate;
	
	void handleKeyEvent( SDL_keysym *keysym , int type );

	//gl_util.cpp
	bool done;
	bool utilCreateWindow(int sizeX, int sizeY, int bpp, bool fullscreen);
	void utilEventLoop();
	void utilBeginRender();
	void utilEndRender();
	void calculateMousePoint();
	
	
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
	void onKeyEvent(int code, int eventType);
	bool mouseDown(int button){return bMouse[button];}
	void onMouseEvent(int code, int eventType);


	//Update.cpp
	void updateMain();
	void generateTestData();
	
	//module.cpp
	bool loadModule(string name);

	//Font.cpp
	void initFont();
	
	//particles.cpp
	IParticleSystem *mParticleSystem;	
	void initParticleSystem();
	
	int iParticleMethod;
	

	//gui.cpp
	UIScreen mUIRoot;		
	
	//main.cpp
	static App *mSingleton;
	
	//GUI
	void makeGUI();
	void addOption(int i, string text, UIWindow *wnd);
	
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
	bool bOptions[32];
	
	string mRenderModule;
	
	//flow_descr.cpp
	FlowDescriptor *mFlowDescriptors[MAX_FLOW_DESCRIPTORS];
	void addFlowDescriptor(byte id, Color c, string name);
	
/*********************************************
	Things modules can access using mApp
**********************************************/
public:
	virtual ~App(){}
	
	//Singleton
	static App *S(){return mSingleton;}
	static void setS(App *s){mSingleton = s;}

	//Initial setup
	int init(App *a, int argc, char **argv);

	//font.cpp	
	virtual void writeText(int x, int y, const char *fmt, ...);
	virtual void writeTextCentered(int x, int y, const char *fmt, ...);

	//misc.cpp
	virtual int randInt(int low, int high);
	virtual float randFloat();
	virtual float randFloat(float low, float high);
	virtual void setOption(int index, bool enabled);
	
	//gl_util.cpp
	virtual void utilCube(float x, float y, float z);
	virtual void utilShutdown(int returnCode);
	virtual void notifyShutdown(){done = true;}
	virtual Vector2 utilProject(float x, float y, float z);

	//flow_descr.cpp
	virtual FlowDescriptor *getFD(byte id);

	//Camera movement
	virtual void camMove(float x, float y, float z);
	virtual void camLookAt(float x, float y, float z);
	virtual void camSetPos(float x, float y, float z);
	virtual void beginDrag();
	virtual void endDrag();
	
	//texture.cpp
	virtual Texture *texLoad(string name, int flags);
	virtual Texture *texGet(string name);
	virtual Texture *texGenerate(string name, byte *buffer, int width, int height);
	
	//particle management
	virtual IParticleSystem *ps(){return mParticleSystem;}
	
	//Mouse
	virtual Vector2 getMouse(){return Vector2(iMouseX, iMouseY);}
			
	//GUI element maker
	template<typename T>
	T *guiCreate(const char *name, Vector2 pos, 
							Vector2 size, UIElement *parent){				
		T *element = new T;
		element->initGeneric(name, pos, size);
		element->init();
		if(parent) parent->addChild(element);
		return element;						
	}
	
	virtual UIScreen *getUIRoot(){return &mUIRoot;}
		
	//Module	
	IModule *mCurrentModule;
	
	//Camera
	float fCameraX, fCameraY, fCameraZ;
	float fLookX, fLookY, fLookZ;
	float fMouseX, fMouseY, fMouseZ;
	Vector2 dragStart;
	bool bDrag;
	
	//GUI objects
	UIWindow *wndOptions;
	UIWindow *wndProto;
		
	virtual int getFPS(){return iFPS;}
	
	
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
	
	//CGL
	bool bCGLCompat;
	bool bHeadless;
	
	void onKey(char code);

};




/*********************************************
				Other
**********************************************/

#ifndef MODULE

//Logging funcs (misc.cpp)
extern void LOG(const char *fmt, ...);
extern void ERR(const char *fmt, ...);
extern void WARN(const char *fmt, ...);
extern int splitString(const std::string &input, const std::string &delimiter, 
				std::vector<std::string> &results, bool includeEmpties = false);

#endif

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

//Point sprite functions
extern PFNGLPOINTPARAMETERFARBPROC  glPointParameterfARB;
extern PFNGLPOINTPARAMETERFVARBPROC glPointParameterfvARB;


#endif

