/* 
 * This file is part of BSOD client
 *
 * Copyright (c) 2011 The University of Waikato, Hamilton, New Zealand.
 *
 * Author: Paul Hunkin
 *
 * Contributors: Shane Alcock
 *
 * All rights reserved.
 *
 * This code has been developed by the University of Waikato WAND research
 * group. For further information please see http://www.wand.net.nz/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $Id$
 */


/*******************************************************************************
							BSOD2 Client - flowmanager.h
							
 Previously the 'classic' visualisation module before simplification. 
*******************************************************************************/

#ifdef USE_TR1
	#include <tr1/unordered_map>
#endif

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
	bool hasSetTexture;
	float shade;
	
	FlowDescriptor *mDescr;	
	PSParams mParamsStart;
	PSParams mParamsEnd;
	
	IPaddress mSrc;
	IPaddress mDst;
};

#define FLOW_TEX_SIZE 512

/*********************************************
		 	Flow texture
**********************************************/
class FlowTexture{
	byte data[FLOW_TEX_SIZE * FLOW_TEX_SIZE * 3];
	
	GLuint mTexture;
	
	void setPx(int x, int y, Color c);
	
	bool bIsValid;
public:
	void init();
	void set(float x, float y, Color c);
	void bind();
	void regenerate();
	void destroy();
	void clear();
	void clearColor(Color c);
};


/*********************************************
	Classic visualisation module
**********************************************/
class FlowManager{

	float fPlaneDistance;

	//Whether we need to do a glProject or not	
	bool bNeedProject;

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

	//Fade on the left and right slabs
	float fFade[2];

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
	
	void setupParams(Flow *f, PSParams *p, Vector2 p1, Vector2 p2, 
					 float speed, float size, bool negative);
					 
		
	//The various flow textures
	FlowTexture mLeftFlowTexture;
	FlowTexture mRightFlowTexture;
	
public:

	//Startup
	bool init();
	
	//Logic
	void update(float currentTime, float timeDelta);
	void newFlow(int flowID, 
				IPaddress src, IPaddress dst, 
				Vector3 start, Vector3 end);
	void newPacket(int flowID, int size, float rtt, FlowDescriptor *type);
	void delFlow(int flowID);
	void delAll();
	void notifyServerChange();
	void showType(FlowDescriptor *type, bool show);
	
	//Rendering
	void render();
	void render2d();
	void renderSelection(); //For stuff that has to be on top of the particles
	
	//Termination
	void shutdown();
	
	//Interaction
	bool onClick(int button, float x, float y, float z);
	
	float getPlaneDistance(){ return fPlaneDistance; }	
	
	bool flowValid(int id){ return getFlowByID(id) != NULL; }
};


