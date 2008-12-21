#include "main.h"
	
/*********************************************
		 	Entry
**********************************************/
bool FlowManager::init(){	
	
	mTex = App::S()->texLoad("waikato.png", 0);
	mEarthTex = App::S()->texLoad("earth.png", 0);
	
	if(!mTex || !mEarthTex){
		LOG("Couldn't get the texture!\n");
		return false;
	}
		
	mSelectedFlow = NULL;
	bNeedProject = false;	
	
	mViewFlows.clear();
	
	fPlaneDistance = ((float)App::S()->iScreenX / (float)App::S()->iScreenY) * 30.0f;

	mDisplayList = glGenLists(1);
	
	return true;
}

/*********************************************
		 	Update
**********************************************/
void FlowManager::update(float currentTime, float timeDelta){

	fPlaneDistance = ((float)App::S()->iScreenX / (float)App::S()->iScreenY) * 30.0f;
	
	fTimeScale = timeDelta;

	//hack! We should only set this if the camera has moved...
	bNeedProject = true; 
		
	//Update all visable flows
	for(int i=0;i<(int)mViewFlows.size();i++){

		if(!mViewFlows[i]){
			continue;
		}
	
		//if(mViewFlows[i]->hide){
		//	continue;
		//}
				
		if(mViewFlows[i]->shade > 0.0f){
			mViewFlows[i]->shade -= timeDelta * 0.1f;
			
			if(mViewFlows[i]->shade < 0.0f){
				mViewFlows[i]->hide = true;
				
				//Remove it from viewflows
				mViewFlows[i] = mViewFlows[mViewFlows.size() - 1];
				mViewFlows.pop_back();
			}
		}
			
	}
		
}


/*********************************************
 Called when the parent program has a new flow
**********************************************/
void FlowManager::newFlow(int flowID, IPaddress src, IPaddress dst, Vector3 start, Vector3 end){

	start = start * 2;
	end = end * 2;

	//Turn the SDLNet structs into in_addrs for later
	struct in_addr ip1, ip2;
	ip1.s_addr = src.host;
	ip2.s_addr = dst.host;
		
	//How many seconds it takes to go from one side to the other
	//This will be scaled per-particle based on the RTT
	float speed = 10.0f;
		
	//Size of the particle
	float size = 1.0f;
	
	Flow *f = NULL;
	
	if(end.x < 0){
		speed = -10; //hack! this means it will go the other way	
		f = addFlow(flowID, Vector2(start.y, start.z), Vector2(end.y, end.z), speed, size);
	}else{
		f = addFlow(flowID, Vector2(end.y, end.z), Vector2(start.y, start.z), speed, size);
	}
		
	//Make the flow object
	
	//clear text to start
	for(int i=0;i<3;i++){
		f->leftText[i] = f->rightText[i] = "";
	}			
	
	f->mDst = dst;
	f->mSrc = src;
	
	//All done!
}

/*********************************************
	Parent program has a new packet
**********************************************/
void FlowManager::newPacket(int flowID, int size, float rtt, FlowDescriptor *type){
		
	Flow *f = getFlowByID(flowID);
	
	if(f && type){
		
		//Figure out size multiplier
		//NOTE: We set base size to 1.0, so this is actually the *actual* size
		float sizeMultiplier = (( (float)size) / 1400.0f);
		
		if(sizeMultiplier < 0.4f)	sizeMultiplier = 0.4f;
		if(sizeMultiplier > 2.0f)	sizeMultiplier = 2.0f;

		sizeMultiplier /= 5.0f; //yes, particles are *small*
		
		sizeMultiplier *= App::S()->fParticleSizeScale;
				
		//And speed multiplier
		float speedMultiplier = (3.0f / rtt) * App::S()->fParticleSpeedScale;
						
		//And add the particle
		PSParams *p = &f->mParamsStart;
		//if(rtt > 0)	p = &f->mParamsStart;
		//else			p = &f->mParamsEnd;
					
		if(!p){
			LOG("Bad params!\n");
			return;
		}
		
		
				
		App::S()->ps()->add(p->mPos, p->mVel, type->mColor, p->size, p->life);
		
		f->mDescr = type;
				
		//Unhide the system, it's had a packet
		f->hide = false;
		
		//And fade it in
		if(f->shade < 1.0f){
		
			if(f->shade <= 0.0f){
				mViewFlows.push_back(f);
			}
		
			f->shade += 0.1f;
		}
						
	}else{
		//LOG("Bad flow ID %d\n", flowID);
	}
}

/*********************************************
	Flow has expired
**********************************************/
void FlowManager::delFlow(int flowID){
	
	Flow *f = getFlowByID(flowID);
	
	if(!f){
		//LOG("Tried to delete non-existant flow %d\n", flowID);
		return;
	}
		
	mFlowMap.erase(flowID); //remove it from the map of flows we know about	
	mFreeFlows.push(f);
	
	if(mSelectedFlow == f){
		mSelectedFlow = NULL;
		
		//Delete the DNS threads, in case we were trying to resolve still
		SDL_KillThread(mDnsThreadLeft);
		SDL_KillThread(mDnsThreadRight);
	}
	
	//Swap this flow with the last active flow
	int index = f->activeID;
	mActiveFlows[index] = mActiveFlows[mActiveFlows.size() - 1];
	mActiveFlows[index]->activeID = index;
	
	//And resize the vector to remove this flow
	mActiveFlows.pop_back();
		
}

/*********************************************
	All flows have expired
**********************************************/
void FlowManager::delAll(){
	
	//Delete everything in the free list
	while(!mFreeFlows.empty()){

		Flow *f = mFreeFlows.top();
		mFreeFlows.pop();	
		delete f;
	
	}
	
	//delete everything in the map. This includes the contents of mActiveFlows and mViewFlows
#ifdef USE_TR1
	for(std::tr1::unordered_map<int, Flow *>::const_iterator it = mFlowMap.begin(); it != mFlowMap.end(); ++it){
		if(it->second)
			delete it->second;
	}
#else
	for(map<int, Flow *>::const_iterator it = mFlowMap.begin(); it != mFlowMap.end(); ++it){
		if(it->second)
			delete it->second;
	}
#endif

	mActiveFlows.clear();
	mViewFlows.clear();
	
	mSelectedFlow = NULL;
}
	
/*********************************************
	Update the display list
**********************************************/
void FlowManager::updateList(){

	glNewList(mDisplayList,GL_COMPILE);


	glColor3f(1,1,1);
	glEnable(GL_TEXTURE_2D);
	
	//Left slab	
	mTex->bind();
	
	float d = fPlaneDistance / 2.0f;
	
	glPushMatrix();
		glTranslatef(-d, 0, 0);
		glRotatef(90, 0, 1, 0);
		App::S()->utilPlane(SLAB_SIZE, SLAB_SIZE,0.1);	
	glPopMatrix();
	
	
	
	//Right slab
	mEarthTex->bind();
	
	glPushMatrix();
		glTranslatef(d, 0, 0);
		glRotatef(270, 0, 1, 0);
		App::S()->utilPlane(SLAB_SIZE, SLAB_SIZE,0.1);	
	glPopMatrix();
	
	

	glDisable(GL_TEXTURE_2D);

	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glPointSize(2.0f);
	
	
	glBegin(GL_POINTS);	
	
	
	for(int i=0;i<(int)mViewFlows.size();i++){
	
		if(!mViewFlows[i]){
			continue;
		}
		
		if(mViewFlows[i]->hide){
			continue;
		}
		
		FlowDescriptor *d = mViewFlows[i]->mDescr;	
		
		if(!d){
			continue;
		}
			
		float shade = mViewFlows[i]->shade;	
					
		glColor4f(d->mColor.r * shade, d->mColor.g * shade, d->mColor.b * shade, shade);			
		
		glVertex3f(mViewFlows[i]->x, mViewFlows[i]->y, mViewFlows[i]->z);		
		glVertex3f(mViewFlows[i]->x2, mViewFlows[i]->y2, mViewFlows[i]->z2);	
		
		
	}
	glEnd();
	
		
	glDisable(GL_BLEND);
		
	glEndList();
	

}

float fRenderTimer = 2.0f;
	
/*********************************************
		 	3D rendering
**********************************************/
void FlowManager::render(){

	//Update the point timer
	fRenderTimer += fTimeScale;
	
	if(fRenderTimer > 1.5f){
		updateList(); //rerender
		fRenderTimer = 0.0f;
	}
	
	//Call the existing list
	glCallList(mDisplayList);

		
		
	//Selected flow
	if(mSelectedFlow){
		FlowDescriptor *d = mSelectedFlow->mDescr;			
		if(!d){
			return;
		}
		
		glColor4f(d->mColor.r, d->mColor.g, d->mColor.b, 1.0f);
	
		glBegin(GL_LINES);
		glVertex3f(mSelectedFlow->x, mSelectedFlow->y, mSelectedFlow->z);		
		glVertex3f(mSelectedFlow->x2, mSelectedFlow->y2, mSelectedFlow->z2);	
		glEnd();
		glPointSize(4.0f);
		glBegin(GL_POINTS);		
		glVertex3f(mSelectedFlow->x, mSelectedFlow->y, mSelectedFlow->z);		
		glVertex3f(mSelectedFlow->x2, mSelectedFlow->y2, mSelectedFlow->z2);	
		glEnd();
		
		if(bNeedProject){
			mSelectedFlow->screenP1 = App::S()->utilProject(mSelectedFlow->x, mSelectedFlow->y, mSelectedFlow->z);		
			mSelectedFlow->screenP2 = App::S()->utilProject(mSelectedFlow->x2, mSelectedFlow->y2, mSelectedFlow->z2);	
			
			bNeedProject = false;
		}
		
	}	
		
	glDisable(GL_BLEND);	
	glDepthMask(GL_TRUE);
		
}



/*********************************************
		 	2D rendering
**********************************************/
void FlowManager::render2d(){

	//App::S()->writeText(550, 7, "(%d flows, %d viewable)", mActiveFlows.size(), mViewFlows.size());
	
	if(mSelectedFlow){	
		Vector2 v1 = mSelectedFlow->screenP1;
		Vector2 v2 = mSelectedFlow->screenP2;	
		
		if(v1.x < 10){
			v1.x = 10;
		}
		
		if(v2.x > App::S()->iScreenX - 120){
			v2.x = App::S()->iScreenX - 120;
		}
		
		
		float y = 20;
		for(int i=0;i<3;i++){
			App::S()->writeText((int)v1.x, (int)(v1.y + (y * i)), "%s", mSelectedFlow->leftText[i].c_str());
			App::S()->writeText((int)v2.x, (int)(v2.y + (y * i)), "%s", mSelectedFlow->rightText[i].c_str());
		}
	}
}

/*********************************************
		 	Shutdown
**********************************************/
void FlowManager::shutdown(){

	delAll();

	LOG("Shutdown\n");
	
}

int DnsLeft(void *data){
	//LOG("Resolving left\n");
	Flow *f = (Flow *)data;		
	const char *left = SDLNet_ResolveIP(&f->mSrc);		
	if(left){	f->leftText[2] = string( left );	}
	else{	f->leftText[2] = "Couldn't resolve";	}	
	//LOG("Resoved left: %s\n", f->leftText[2].c_str());	
	return left ? true : false;
}

int DnsRight(void *data){
	//LOG("Resolving right\n");
	Flow *f = (Flow *)data;		
	const char *right = SDLNet_ResolveIP(&f->mDst);		
	if(right){	f->rightText[2] = string( right );	}
	else{	f->rightText[2] = "Couldn't resolve";	}	
	//LOG("Resoved right: %s\n", f->rightText[2].c_str());	
	return right ? true : false;
}

/*********************************************
		 	Interaction
**********************************************/
bool FlowManager::onClick(int button, float x, float y, float z){
	float planeX = z;
	float planeY = y;
	
	//Go through all the flows that have 'dots' on either side
	for(int i=0;i<(int)mActiveFlows.size();i++){
	
		Flow *f = mActiveFlows[i];			
		if(!mActiveFlows[i]){
			continue;
		}
		
		if(mActiveFlows[i]->hide){
			continue;
		}
		
		//Check if we clicked on this flow
		//TODO: We should keep checking, and select the *closest*. This picks the *first* hit
		int c = f->collide(x,y,z);
		
		if(c == 0){
			continue;
		}
		
		mSelectedFlow = f;	
		bNeedProject = true;
		
		//Set the left and right texts
		struct in_addr ip1, ip2;
        ip1.s_addr = f->mSrc.host;
        ip2.s_addr = f->mDst.host;

		//Set the left and right IPs
		f->leftText[0] = string(inet_ntoa(ip1));
		f->rightText[0] = string(inet_ntoa(ip2));
		
		//Set the description from the FlowDescriptor name
		if(f->mDescr){
			f->leftText[1] = f->mDescr->mName;
			f->rightText[1] = f->mDescr->mName;
		}
		
		//Spawn two (short-lived) threads to resolve DNS if necessary
		if(f->leftText[2] == ""){
			f->leftText[2] = "(Resolving)";
			mDnsThreadLeft = SDL_CreateThread(DnsLeft, f);
		}
		
		if(f->rightText[2] == ""){
			f->rightText[2] = "(Resolving)";
			mDnsThreadRight = SDL_CreateThread(DnsRight, f);
		}
		
		return true; //handled
	}
	
	//We didn't click anything
	mSelectedFlow = NULL;
	
	return false;
}



