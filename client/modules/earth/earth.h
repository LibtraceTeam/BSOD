#include "module_interface.h"

#ifndef EARTH_MODULE
#define EARTH_MODULE

#define RADIUS 10
#define NUM_STARS 500
#define STAR_DISTANCE 250

class Star{
public:
	Vector3 pos;
};

class FlowPoint{
public:
	Vector3 pos1;
	Vector3 pos2;
	Color c;
	FlowProtocol type;
};

/*********************************************
	Earth visualisation module
**********************************************/
class EarthModule : public IModule{

	//Plane textures
	Texture *mTex;
	
	//GL quadratic
	GLUquadric *mQuadric; 
	
	//x/y/z
	bool rotate[3];
	float angle[3];
	
	//Stars
	Star mStars[NUM_STARS];
	
	vector<FlowPoint> mFlowPoints;
	map<int, FlowPoint *> mFlowMap;

public:

	//Startup
	bool init();
	
	//Logic
	void update(float currentTime, float timeDelta);
	void newFlow(int flowID, IPaddress src, IPaddress dst, FlowProtocol type, FlowDirection dir);
	void newPacket(int flowID, int size, float rtt);
	void delFlow(int flowID);
	void delAll();
	
	//Rendering
	void render();
	void render2d();
	
	//Termination
	void shutdown();
	
	//Configuration
	bool setConfig(string key, string value);
	void getConfigExports(vector<string> &keys);
	
	//Interaction
	void onClick(int button, float x, float y, float z);
	
	
};

#endif
