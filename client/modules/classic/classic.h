#include "module_interface.h"

#ifndef CLASSIC_MODULE
#define CLASSIC_MODULE

//Pos defines
#define MAX_FLOWS 1024
#define SLAB_SIZE 20

//Toggles between std::map and unordered_map
#define USE_TR1

#ifdef USE_TR1
	#include <tr1/unordered_map>
#endif

/*********************************************
	Particle system parameter object
 This holds all the various attributes of a PS
**********************************************/
class PSParams{
public:
	Vector3 mPos;
	Vector3 mVel;
	float life;
	float size;	
};

/*********************************************
		 	Flow structure
**********************************************/
class Flow{
public:
	float x,y,z; //end point on the left
	float x2,y2,z2; //end point on the right
		
	//3 lines of arbitary text
	string leftText[3];
	string rightText[3];
	
	//Points in screen-space (2D)
	//Note that these are not updated every frame
	Vector2 screenP1, screenP2;
	
	//Tests if a point is within a certain distance
	//of either end
	int collide(float pX, float pY, float pZ);
	
	//IDs
	int flowID; //the ID provided by the server
	int activeID; //the index into the activeFlows array
	
	bool hide;
	float shade;
	
	FlowDescriptor *mDescr;	
	PSParams mParamsStart;
	PSParams mParamsEnd;
	
	IPaddress mSrc;
	IPaddress mDst;
};


/*********************************************
	Classic visualisation module
**********************************************/
class ClassicModule : public IModule{

	float fPlaneDistance;

	//Whether we need to do a glProject or not	
	bool bNeedProject;

	//Plane textures
	Texture *mTex;
	Texture *mEarthTex;

	//List of flows
	stack<Flow *> mFreeFlows; 
	vector<Flow *> mActiveFlows;
	vector<Flow *> mViewFlows;
	
	//DNS resolving threads
	SDL_Thread *mDnsThreadLeft;
	SDL_Thread *mDnsThreadRight;
	
#ifdef USE_TR1
	std::tr1::unordered_map<int, Flow *> mFlowMap;
#else
	map<int, Flow *> mFlowMap;
#endif

	//Current flow
	Flow *mSelectedFlow;
	
	//Flow getters
	Flow *getFlowByID(int id);
	Flow *getFreeFlow();

	//Various util functions
	void utilSlab(float x, float y, float z);
	Flow *addFlow(int flowID, Vector2 p1, Vector2 p2, float speed, float size);
		
	void updateList();	
	GLuint mDisplayList;
	
	void setupParams(Flow *f, PSParams *p, Vector2 p1, Vector2 p2, float speed, float size, bool negative);
	
public:

	//Startup
	bool init();
	
	//Logic
	void update(float currentTime, float timeDelta);
	void newFlow(int flowID, IPaddress src, IPaddress dst, Vector3 start, Vector3 end);
	void newPacket(int flowID, int size, float rtt, FlowDescriptor *type);
	void delFlow(int flowID);
	void delAll();
	
	//Rendering
	void render();
	void render2d();
	
	//Termination
	void shutdown();
	
	//Interaction
	bool onClick(int button, float x, float y, float z);
	
	
};

uint32_t SuperFastHash (const char * data, int len);

#endif
