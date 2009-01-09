#include "main.h"

/*********************************************
Checks if a point is close to either end of a flow
**********************************************/
int Flow::collide(float pX, float pY, float pZ){	

	if(hide){
		return 0;
	}

	Vector3 p = Vector3(pX, pY, pZ);
	Vector3 left = Vector3(x,y,z);
	Vector3 right = Vector3(x2,y2,z2);
	
	float d = 0.05f; //max dist that is judged as a collision
	//note that this isn't sqrt'd...
		
	if(p.fastDistTo(left) < d){
		return 1;
	}else if(p.fastDistTo(right) < d){
		return 2;
	}else{
		return 0;
	}
}

/*********************************************
		Return a free flow object
**********************************************/
Flow *FlowManager::getFreeFlow(){

	if(mFreeFlows.empty()){
		Flow *f = new Flow;
		return f;
	}
	
	Flow *f = mFreeFlows.top();
	mFreeFlows.pop();
	
	return f;
}

Flow *FlowManager::getFlowByID(int id){
	return mFlowMap[id];
}

void FlowManager::setupParams(Flow *f, PSParams *p, Vector2 p1, Vector2 p2, float speed, float size, bool negative){
	
	if(!p){
		LOG("Bad params in setup!\n");
		return;
	}
	
	float d = fPlaneDistance / 2.0f;	
		
	if(negative){
		speed *= -1; //otherwise the calculations break...
	}
	
	//Set up the standard parameters
	if(negative){
		p->mPos = Vector3(d, p2.y, p2.x);	
	}else{	
		p->mPos = Vector3(-d, p1.y, p1.x);
	}	
	p->life = speed;		
	p->size = size;		
	
	
	//Figure out the velocity. This is measured in units per second. 
	p->mVel.x = fPlaneDistance / speed; 
	p->mVel.z = ((p2.x - p1.x) / speed);	
	p->mVel.y = ((p2.y - p1.y) / speed);
	
	//Inverse stuff if we're negative
	if(negative){
		p->mVel.x *= -1;
		p->mVel.z = ((p1.x - p2.x) / speed);	
		p->mVel.y = ((p1.y - p2.y) / speed);
	}
					
	
}

/*********************************************
		 	Create a flow
**********************************************/
Flow *FlowManager::addFlow(int flowID, Vector2 p1, Vector2 p2, float speed, float size){

	//speed = how many seconds it takes to get across, so smaller=faster
	//If speed<0, we assume it's going left. If not, right.
	bool negative = speed < 0.0f;	
	Flow *f = getFreeFlow();
	
	//HACK! We invert it here. Probably should fix this higher up
	setupParams(f, &f->mParamsStart, p1, p2, -speed, size, !negative);
		
	//setupParams(f, &f->mParamsEnd, p1, p2, speed, size, negative);
	
	//LOG("Flow: Start at %f/, end at %f/\n", f->mParamsStart.mPos.x, f->mParamsEnd.mPos.x);
	
	float d = fPlaneDistance / 2.0f;				
	f->x = -d; f->y = p1.y; f->z = p1.x;
	f->x2 = d; f->y2 = p2.y; f->z2 = p2.x;
	
	f->mDescr = NULL;
	
	mActiveFlows.push_back(f);
	f->activeID = mActiveFlows.size() - 1;
	
	mFlowMap[flowID] = f;
			
	f->hide = true; //we show it when we have some packets...
	f->shade = 0.0f;
	
	for(int i=0;i<3;i++){
		f->leftText[i] = f->rightText[i] = "";
	}
			
	return f;
}


