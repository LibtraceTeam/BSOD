#ifndef _MODULE_H
#define _MODULE_H

class App;

/*********************************************
	 	Flow descriptor object
**********************************************/
class FlowDescriptor{
public:
	string mName;
	Color mColor;
	byte id;	
	
	bool bShown;
};

/*********************************************
 		Module interface
**********************************************/
class IModule{
public:

	//Startup
	virtual bool init(){return true;}
	
	//Logic
	virtual void update(float currentTime, float timeDelta){}
	virtual void newFlow(int flowID, IPaddress src, IPaddress dst, Vector3 start, Vector3 end){}; 
	virtual void newPacket(int flowID, int size, float rtt, FlowDescriptor *type){}; 
	virtual void delFlow(int flowID){};
	virtual void delAll(){};
	
	//Rendering
	virtual void render(){}
	virtual void render2d(){}
	
	//Termination
	virtual void shutdown(){}
		
	//Interaction. Returns true if it was handled
	virtual bool onClick(int button, float x, float y, float z){}
};

/*********************************************
	The current module that is loaded
**********************************************/
extern IModule *mCurrentModule;

#endif
