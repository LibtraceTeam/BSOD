#include "main.h"
	
/*********************************************
		 	Entry
**********************************************/
bool FlowManager::init(){	


	mLeftFlowTexture.init();
	mRightFlowTexture.init();

	mSelectedFlow = NULL;
	bNeedProject = false;	
	
	mViewFlows.clear();
	
	fPlaneDistance = ((float)App::S()->iScreenX / 
						(float)App::S()->iScreenY) * 40.0f;

	mDisplayList = glGenLists(1);
	
	return true;
}

/*********************************************
		 	Update
**********************************************/
void FlowManager::update(float currentTime, float timeDelta){
/*
	fPlaneDistance = ((float)App::S()->iScreenX / 
						(float)App::S()->iScreenY) * 30.0f;
*/
	
	//hack! We should really only set this if the camera has moved...
	bNeedProject = true; 
		
}


/*********************************************
 Called when the parent program has a new flow
**********************************************/
void FlowManager::newFlow(int flowID, IPaddress src, IPaddress dst, 
						  Vector3 start, Vector3 end){
						  
						  
	//LOG("Added %d\n", flowID);

	start = start * 2;
	end = end * 2;

	//Turn the SDLNet structs into in_addrs for later
	struct in_addr ip1, ip2;
	ip1.s_addr = src.host;
	ip2.s_addr = dst.host;
		
	//How many seconds it takes to go from one side to the other
	//This will be scaled per-particle based on the RTT and global scaling
	float speed = 10.0f;
		
	//Size of the particle
	float size = 1.0f;
	
	Flow *f = NULL;
	
	if(end.x < 0){
		//Note the -speed
		f = addFlow(flowID, Vector2(start.y, start.z), 
					Vector2(end.y, end.z), -speed, size);					
	}else{
		f = addFlow(flowID, Vector2(end.y, end.z), 
					Vector2(start.y, start.z), speed, size);				
	}
	
	//Make the flow object
	
	//clear text to start
	for(int i=0;i<3;i++){
		f->leftText[i] = f->rightText[i] = "";
	}			
	
	f->mDst = dst;
	f->mSrc = src;
	f->hasSetTexture = false;
	f->shade = 0.0f;
	f->flowID = flowID;
	
	//All done!
}

/*********************************************
	Parent program has a new packet
**********************************************/
void FlowManager::newPacket(int flowID, int size, float rtt, 
							FlowDescriptor *type){
		
	Flow *f = getFlowByID(flowID);
	
	if(f && type){
		
		//Figure out size multiplier
		//Note that we partition into small, medium and large packets. It makes
		//it faster to render, as it minimizes state changes. Feel free to add
		//more distinct sizes here though, it should deal. 
		float sizeMultiplier = 1.0f;
		
		if(size < 128) sizeMultiplier = 0.75f;
		else if(size > 1400) sizeMultiplier = 1.25f;
		
		//And speed multiplier
		float speedMultiplier = (1.0f / rtt);
								
		//And add the particle. 
		PSParams *p = &f->mParamsStart;
					
		if(!p){
			LOG("Bad params!\n");
			return;
		}
		
		//Add the particle		
		App::S()->ps()->add(p->mPos, p->mVel * speedMultiplier, 
							type->mColor, sizeMultiplier, 
							p->life / speedMultiplier);
		
		f->mDescr = type;
						
		//Set the texture		
		Color c = f->mDescr->mColor;
		
		f->hide = false;
		
		//Set the texture colour if needed		
		if(!f->hasSetTexture){
			if(f->x < 0){
				mLeftFlowTexture.set(-f->z, -f->y, c);	
				mRightFlowTexture.set(f->z2, -f->y2, c);	
			}else{
				mLeftFlowTexture.set(-f->z2, -f->y2, c);	
				mRightFlowTexture.set(f->z, -f->y, c);	
			}	
			
			f->hasSetTexture = true;			
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
	
	
	//LOG("Removed %d\n", flowID);
	
	//Clear the texture
	Color blank = Color(0.01f, 0.01f, 0.01f);
	if(f->x < 0){
		mLeftFlowTexture.set(-f->z, -f->y, 		blank);	
		mRightFlowTexture.set(f->z2, -f->y2, 	blank);	
	}else{
		mLeftFlowTexture.set(-f->z2, -f->y2, 	blank);	
		mRightFlowTexture.set(f->z, -f->y, 		blank);	
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
	Toggle the 'hide' state 
**********************************************/
void FlowManager::showType(FlowDescriptor *type, bool show){
	for(int i=0;i<(int)mActiveFlows.size();i++){
	
		Flow *f = mActiveFlows[i];			
		if(!mActiveFlows[i]){
			continue;
		}
		
		if(f->mDescr == type){
			f->hide = !show;
		}
		
		f->hasSetTexture = false;
	}
	
	if(!show){
		mLeftFlowTexture.clearColor(type->mColor);
		mRightFlowTexture.clearColor(type->mColor);
	}
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
/*
#ifdef USE_TR1
	for(std::tr1::unordered_map<int, Flow *>::const_iterator it = 
		mFlowMap.begin(); it != mFlowMap.end(); ++it){
		if(it->second)
			delete it->second;
	}
#else
	for(map<int, Flow *>::const_iterator it = mFlowMap.begin(); 
		it != mFlowMap.end(); ++it){
		if(it->second)
			delete it->second;
	}
#endif
*/

	for(int i=0;i<(int)mActiveFlows.size();i++){
		delete mActiveFlows[i];
	}
	
	mFlowMap.clear();

	mActiveFlows.clear();
	mViewFlows.clear();
	
	mSelectedFlow = NULL;
	
	mLeftFlowTexture.clear();
	mRightFlowTexture.clear();
	
	mLeftFlowTexture.regenerate();
	mRightFlowTexture.regenerate();
}
	
/*********************************************
	Update the display list
**********************************************/
/*
void FlowManager::updateList(){

	glNewList(mDisplayList,GL_COMPILE);	
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glPointSize(3.0f);
		
	glBegin(GL_POINTS);		
	
	
	for(int i=0;i<(int)mViewFlows.size();i++){
	
		if(!mViewFlows[i]) continue;		
		if(mViewFlows[i]->hide)	continue;
		
		FlowDescriptor *d = mViewFlows[i]->mDescr;	
		
		if(!d) continue;
			
		float shade = 1.0f; //mViewFlows[i]->shade;	
							
		glColor4f(	d->mColor.r * shade, 
					d->mColor.g * shade, 
					d->mColor.b * shade, 
					shade	);			
		
		glVertex3f(mViewFlows[i]->x, mViewFlows[i]->y, mViewFlows[i]->z);		
		glVertex3f(mViewFlows[i]->x2, mViewFlows[i]->y2, mViewFlows[i]->z2);			
	}
	
	glEnd();	
				
	glDisable(GL_BLEND);		
	glEndList();
}
*/

float fRenderTimer = 0.0f;
bool isLeft = false;
	
/*********************************************
		 	3D rendering
**********************************************/
void FlowManager::render(){

	//Update the point timer
	fRenderTimer += fTimeScale;
	
	if(fRenderTimer > 5.0f){
		update(0.0f, fRenderTimer); //Kinda nasty having it here, but it means
									//that it's synced with the render properly
		//updateList(); //rerender
		
		if(isLeft)
			mLeftFlowTexture.regenerate();
		else		
			mRightFlowTexture.regenerate();
		
		isLeft = !isLeft;
			
		fRenderTimer = 0.0f;
	}
	
	
	glEnable(GL_TEXTURE_2D);
	
			
	//Left slab	
	if(App::S()->mLeftTex){
			
		glEnable(GL_TEXTURE_2D);
		App::S()->mLeftTex->bind();
				
		float f = MAX(fFade[0], 0.0f);
		glColor3f(f,f,f);
		
		if(f < 1.0f){
			fFade[0] += fTimeScale;
		}
		
	}else{
		glDisable(GL_TEXTURE_2D);
		glColor3f(0.0f, 0.0f, 0.0f);
		fFade[0] = -1.0f;
	}
	
	
	float d = fPlaneDistance / 2.0f;
	
	glPushMatrix();

		glTranslatef(-d, 0, 0);
		glRotatef(90, 0, 1, 0);
		
		App::S()->utilPlane(SLAB_SIZE, SLAB_SIZE,0.1);					
		
		mLeftFlowTexture.bind();			
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE,GL_ONE);
		glColor4f(1,1,1,1);
		App::S()->utilPlane(SLAB_SIZE, SLAB_SIZE,0.1);	
		glDisable(GL_BLEND);
		
	glPopMatrix();
	
	
	//Right slab
	if(App::S()->mRightTex){
		glEnable(GL_TEXTURE_2D);
		App::S()->mRightTex->bind();
		
		float f = MAX(fFade[1], 0.0f);
		glColor3f(f,f,f);
		
		if(f < 1.0f){
			fFade[1] += fTimeScale;
		}
		
	}else{
		glDisable(GL_TEXTURE_2D);
		glColor3f(0.0f, 0.0f, 0.0f);
		fFade[1] = -1.0f;
	}
	
	glPushMatrix();
		glTranslatef(d, 0, 0);
		glRotatef(270, 0, 1, 0);
		App::S()->utilPlane(SLAB_SIZE, SLAB_SIZE,0.1);
		
		mRightFlowTexture.bind();			
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE,GL_ONE);
		glColor4f(1,1,1,1);
		App::S()->utilPlane(SLAB_SIZE, SLAB_SIZE,0.1);	
		glDisable(GL_BLEND);	
	glPopMatrix();	
	
	//Call the existing list
	//glCallList(mDisplayList);		
}

/*********************************************
 	Renders the selected flow indicator
**********************************************/
void FlowManager::renderSelection(){

	glDisable(GL_BLEND);	
	glDepthMask(GL_TRUE);
	glDisable(GL_TEXTURE_2D);
	
	//Selected flow
	if(mSelectedFlow){
		FlowDescriptor *d = mSelectedFlow->mDescr;			
		if(!d){
			return;
		}
		
		glColor4f(0.25f, 0.25f, 0.25f,1.0f);
		glLineWidth(6.0f);
		
		glBegin(GL_LINES);
			glVertex3f(mSelectedFlow->x, mSelectedFlow->y, mSelectedFlow->z);		
			glVertex3f(mSelectedFlow->x2, mSelectedFlow->y2, mSelectedFlow->z2);	
		glEnd();
						
		glPointSize(4.0f);
		glBegin(GL_POINTS);		
			glVertex3f(mSelectedFlow->x, mSelectedFlow->y, mSelectedFlow->z);		
			glVertex3f(mSelectedFlow->x2, mSelectedFlow->y2, mSelectedFlow->z2);	
		glEnd();
		
		glColor4f(d->mColor.r, d->mColor.g, d->mColor.b, 1.0f);
		glLineWidth(3.0f);
	
		glBegin(GL_LINES);
			glVertex3f(mSelectedFlow->x, mSelectedFlow->y, mSelectedFlow->z);		
			glVertex3f(mSelectedFlow->x2, mSelectedFlow->y2, mSelectedFlow->z2);	
		glEnd();
		
		mSelectedFlow->screenP1 = App::S()->utilProject(mSelectedFlow->x, mSelectedFlow->y, mSelectedFlow->z);		
		mSelectedFlow->screenP2 = App::S()->utilProject(mSelectedFlow->x2, mSelectedFlow->y2, mSelectedFlow->z2);		
	}		
}



/*********************************************
		 	2D rendering
**********************************************/
void FlowManager::render2d(){

	glDisable(GL_DEPTH_TEST);
	
	//App::S()->writeText(10, 50, "%d/%d", 
	//					mActiveFlows.size(), mViewFlows.size());
	
	if(mSelectedFlow){	
		Vector2 v1 = mSelectedFlow->screenP1;
		Vector2 v2 = mSelectedFlow->screenP2;	
				
		float b = 5.0f;//border
								
		//Draw a background for them
		float w = 250;
		float h = 60;
		glColor4f(0.0f, 0.0f, 0.0f, 0.85f);
		glDisable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glBlendEquationEXT(GL_SUBTRACT);
		
		glBegin(GL_QUADS);
			glVertex2f(v1.x - b, v1.y - b);
			glVertex2f(v1.x + w, v1.y - b);
			glVertex2f(v1.x + w, v1.y + h);
			glVertex2f(v1.x - b, v1.y + h);
		glEnd();
		
		glBegin(GL_QUADS);
			glVertex2f(v2.x - b, v2.y - b);
			glVertex2f(v2.x + w, v2.y - b);
			glVertex2f(v2.x + w, v2.y + h);
			glVertex2f(v2.x - b, v2.y + h);
		glEnd();
		
		FlowDescriptor *d = mSelectedFlow->mDescr;			
		glColor4f(d->mColor.r, d->mColor.g, d->mColor.b, 1.0f);
		
		glLineWidth(1.0f);
		
		glBegin(GL_LINES);
			glVertex2f(v1.x - b, v1.y - b);
			glVertex2f(v1.x + w, v1.y - b);
			
			glVertex2f(v1.x + w, v1.y - b);
			glVertex2f(v1.x + w, v1.y + h);
			
			glVertex2f(v1.x + w, v1.y + h);
			glVertex2f(v1.x - b, v1.y + h);
			
			glVertex2f(v1.x - b, v1.y + h);
			glVertex2f(v1.x - b, v1.y - b);
		glEnd();
		
		glBegin(GL_LINES);
			glVertex2f(v2.x - b, v2.y - b);
			glVertex2f(v2.x + w, v2.y - b);
			
			glVertex2f(v2.x + w, v2.y - b);
			glVertex2f(v2.x + w, v2.y + h);
			
			glVertex2f(v2.x + w, v2.y + h);
			glVertex2f(v2.x - b, v2.y + h);
			
			glVertex2f(v2.x - b, v2.y + h);
			glVertex2f(v2.x - b, v2.y - b);
		glEnd();
		
		glBlendEquationEXT(GL_ADD);
		glEnable(GL_TEXTURE_2D);
		glColor4f(1,1,1,1);
		
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
	
	glDeleteLists(mDisplayList, 1);

	LOG("Shutdown\n");
	
}

void FlowManager::notifyServerChange(){
	for(int i=0;i<2;i++){
		fFade[i] = -1.0f;
	}
}

int DnsLeft(void *data){
	//LOG("Resolving left\n");
	Flow *f = (Flow *)data;		
	int id = f->flowID;
	
	//LOG("Resolving with %d\n", id);
	
	const char *left = SDLNet_ResolveIP(&f->mSrc);
	
	if(!App::S()->mFlowMgr->flowValid(id)){
		//LOG("Invalid flow %d, ignoring (%s)\n", id, left);	
		return false;
	}
		
	if(left){	f->leftText[2] = string( left );	}
	else{	f->leftText[2] = "";	}	
	//LOG("Resoved left: %s\n", left);	
	return left ? true : false;
}

int DnsRight(void *data){
	//LOG("Resolving right\n");
	Flow *f = (Flow *)data;		
	int id = f->flowID;
	
	const char *right = SDLNet_ResolveIP(&f->mDst);
		
	
	if(!App::S()->mFlowMgr->flowValid(id)){
		//LOG("Invalid flow %d, ignoring (%s)\n", id, right);	
		return false;
	}
		
		
	if(right){	f->rightText[2] = string( right );	}
	else{	f->rightText[2] = "";	}	
	
	//LOG("Resoved right: %s\n", right);	
	return right ? true : false;
}

/*********************************************
		 	Interaction
**********************************************/
bool FlowManager::onClick(int button, float x, float y, float z){
	
	//if(x > 50 || x < -50){
		//We clicked on empty space, discard
	//	return false;
	//}
		
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
		
		if(mSelectedFlow){
			//We had already selected one!
			//if(mDnsThreadLeft) SDL_KillThread(mDnsThreadLeft);
			//if(mDnsThreadRight) SDL_KillThread(mDnsThreadRight);
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
			f->leftText[2] = " "; //hack!
			mDnsThreadLeft = SDL_CreateThread(DnsLeft, f);
		}
		
		if(f->rightText[2] == ""){
			f->rightText[2] = " ";
			mDnsThreadRight = SDL_CreateThread(DnsRight, f);
		}
		
		return true; //handled
	}
	
	//We didn't click anything
	mSelectedFlow = NULL;
	
	return false;
}






/*********************************************
   The side textures with start/end points
**********************************************/
void FlowTexture::init(){
	
	bIsValid = false;
	
	clear();
}

void FlowTexture::destroy(){
	glDeleteTextures(1, &mTexture);
}

void FlowTexture::setPx(int x, int y, Color c){
	
	int index = ((y * FLOW_TEX_SIZE) + x) * 3;
	
	data[index + 0] = (byte)(c.b * 255);
	data[index + 1] = (byte)(c.g * 255);
	data[index + 2] = (byte)(c.r * 255);
}

void FlowTexture::regenerate(){

	destroy();
	
	int len = FLOW_TEX_SIZE * FLOW_TEX_SIZE * 3;

	//glError();
	
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &mTexture);	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mTexture);
	
	//glError();
   
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
	
	//glError();
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, FLOW_TEX_SIZE, FLOW_TEX_SIZE, 0, GL_BGR, GL_UNSIGNED_BYTE, &data);
	
	//glError();

    //gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, iSizeX, iSizeY, GL_RGB, GL_UNSIGNED_BYTE, data);
   
    bIsValid = true;
    
    //LOG("**********Updated %d\n", mTexture);
}

void FlowTexture::bind(){	

	//if(!bIsValid){
	//	regenerate();
	//}

	glBindTexture(GL_TEXTURE_2D, mTexture);
}

void FlowTexture::set(float x, float y, Color c){

	x += (SLAB_SIZE / 2);
	y += (SLAB_SIZE / 2);

	x = (x / SLAB_SIZE) * FLOW_TEX_SIZE;
	y = (y / SLAB_SIZE) * FLOW_TEX_SIZE;
	
	if(x < 0 || x > FLOW_TEX_SIZE - 1 || y < 0 || y > FLOW_TEX_SIZE - 1){
		//LOG("%f/%f\n", x, y);
		return;
	}
	
	setPx((int)x, (int)y, c);
}

void FlowTexture::clear(){

	int len = FLOW_TEX_SIZE * FLOW_TEX_SIZE * 3;

	for(int i=0;i<len;i++){
		data[i] = 1;
	}
}

void FlowTexture::clearColor(Color c){

	//LOG("Cleared %f/%f/%f\n", c.r, c.g, c.b);
	
	int len = FLOW_TEX_SIZE * FLOW_TEX_SIZE * 3;

	byte r = (byte)(c.b * 255);
	byte g = (byte)(c.g * 255);
	byte b = (byte)(c.r * 255);
	
	for(int i=0;i<len;i+=3){
		if(	data[i + 0] == r &&
			data[i + 1] == g &&
			data[i + 2] == b){
				
				data[i + 0] = 1;
				data[i + 1] = 1;
				data[i + 2] = 1;		
		}
	}		
}
