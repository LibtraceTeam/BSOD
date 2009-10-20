#include "main.h"

#define MAX_SIZE 10.0f
//#define DISABLE_SIZE

/*******************************************************************************
							PointSprites
*******************************************************************************/
void PSSprites::render(){
	
	mTexture->bind();
	
	int count = 0;
	int bad = 0;
	
	//Set GL state
	glEnable(GL_TEXTURE_2D);		
	glDepthMask(GL_FALSE);		
	glEnable(GL_BLEND);							
	//glBlendFunc(GL_SRC_ALPHA,GL_ONE);
	glBlendFunc(GL_ONE, GL_ONE);
	
	//glEnableClientState(GL_VERTEX_ARRAY);

#ifdef DISABLE_SIZE
	glPointSize(5.0f);
	glBegin(GL_POINTS);	
#endif
	//Scaling	
	float scale = setSizeScale();
			
	map<float, ParticleCollection *>::const_iterator itr;
	
	//Go through each of the particle collections
	for(itr = mParticleCollections.begin(); 
		itr != mParticleCollections.end(); ++itr){	
			
		ParticleCollection *collection = itr->second;		
		
		//This may be hidden by colour or size
		if(!collection->bShown){
			continue;
		}
			
		vector<Particle *> *list = &collection->mParticles;
		
		//Make sure we've got at least one particle
		if(list->size() == 0) {
			continue; 
		}
		
		collection->mColor.bind();
		
		//Set the state for this collection
#ifndef DISABLE_SIZE
		glPointSize(collection->fSize * scale);
		glBegin(GL_POINTS);
#endif
		
		bad++; //Count the number of state changes
		count += list->size(); //Count the number of particles
				
		//Now render all the particles in this list	
		//TODO: Use glDrawArrays!													
		for(int i=0;i<(int)list->size();i++){			
			Particle *p = (*list)[i];		
			glVertex3f(p->x, p->y, p->z);
		}
		
#ifndef DISABLE_SIZE
		glEnd();
#endif
		
	}	

#ifdef DISABLE_SIZE	
	glEnd();
#endif

	//And clean up
	glDisable(GL_BLEND);	
	glDepthMask(GL_TRUE);
	//glDisableClientState(GL_VERTEX_ARRAY);
	
	iNumActive = count;
}


/*********************************************
 Figure out and set the dynamic scaling factor 
**********************************************/
float PSSprites::setSizeScale(){
	float scale = (1.0f/(float)iNumActive) * 150000;	
	if(scale < 5.0f){ scale = 5.0f; }
	else if(scale > MAX_SIZE){	
		scale = MAX_SIZE;
	}	
	scale *= App::S()->fParticleSizeScale;	
				
	//Set a default size here
	glPointSize(scale); 	
	glPointParameterfARB( GL_POINT_SIZE_MIN_ARB, App::S()->fParticleSizeScale );
	glPointParameterfARB( GL_POINT_SIZE_MAX_ARB, fMaxSize );
	
	return scale;
}


/*********************************************
	Set up the point sprites
**********************************************/
bool PSSprites::init(){

	if (!glewIsSupported("GL_VERSION_1_4  GL_ARB_point_sprite")){
		ERR("No point sprite support!\n");
	  	return false;
	}

	glGetFloatv( GL_POINT_SIZE_MAX_ARB, &fMaxSize );
			
	glPointSize( fMaxSize );
	glPointParameterfARB( GL_POINT_FADE_THRESHOLD_SIZE_ARB, 60.0f );
	glPointParameterfARB( GL_POINT_SIZE_MIN_ARB, 1.0f );
	glPointParameterfARB( GL_POINT_SIZE_MAX_ARB, fMaxSize );
	glTexEnvf( GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE );
	glEnable( GL_POINT_SPRITE_ARB );

	return PSClassic::init();
}

/*******************************************************************************
							Classic particle system
*******************************************************************************/
bool PSClassic::init(){

	iLastColorChanges = 0;
	
	fTime = 0.0f;

	while(!mFree.empty()){
		mFree.pop();
	}
		
	for(int i=MAX_PARTICLES - 1;i>=0;i--){
		mParticles[i].active = false;
		mParticles[i].life = 0.0f;
		mParticles[i].index = i;
		mParticles[i].r = 1.0f;
		mParticles[i].g = 1.0f;
		mParticles[i].b = 1.0f;
		mParticles[i].a = 1.0f;	
		
		mFree.push(&mParticles[i]);		
	}
	
	iNumActive = 0;
	
	bNeedRecompile = true;
	mDisplayList = glGenLists(1);
	
	mTexture = App::S()->mParticleTex;
	
	if(!mTexture){
		return false;
	}
					
	return true;
}

void PSClassic::shutdown(){
	//clear the stack
	while(!mFree.empty()){
		mFree.pop();
	}
	
	delAll();
	
	
	map<float, ParticleCollection *>::const_iterator itr;
	
	//Go through each of the particle collections
	for(itr = mParticleCollections.begin(); 
		itr != mParticleCollections.end(); ++itr){	
			
		ParticleCollection *collection = itr->second;	
		delete collection;
	}
	
	iNumActive = 0;
}

/*********************************************
	Move particles, expire as necessary
**********************************************/
void PSClassic::update(){
	
	float speedScale = App::S()->fParticleSpeedScale * PARTICLE_FPS;
	fTime += fTimeScale * App::S()->fParticleSpeedScale;

	map<float, ParticleCollection *>::const_iterator itr;
	
	for(itr = mParticleCollections.begin(); 
		itr != mParticleCollections.end(); ++itr){	
			
		ParticleCollection *collection = itr->second;			
		vector<Particle *> *list = &collection->mParticles;
		
		for(int i=0;i<(int)list->size();i++){			
			Particle *p = (*list)[i];			
			
			if(p->life < 0){
				del(collection, i);
				i--;
				continue;
			}
								
			//Move the particle
			p->x += p->vx * speedScale;
			p->y += p->vy * speedScale;
			p->z += p->vz * speedScale;
			p->life -= speedScale;	
							
			//TODO: More particle logic here?
		}
	}
	
	//LOG("ClassicUpdate!\n");
	
}

/*********************************************
	Render using standard drawing
**********************************************/
void PSClassic::render(){

	mTexture->bind();
	
	int count = 0;
	int bad = 0;
	
	//Set GL state
	glEnable(GL_TEXTURE_2D);		
	glDepthMask(GL_FALSE);		
	glEnable(GL_BLEND);							
	//glBlendFunc(GL_SRC_ALPHA,GL_ONE);	
	glBlendFunc(GL_ONE, GL_ONE);	
	
	//Scaling	
	float scale = setSizeScale();
			
	map<float, ParticleCollection *>::const_iterator itr;
		
	//Go through each of the particle collections
	for(itr = mParticleCollections.begin(); 
		itr != mParticleCollections.end(); ++itr){	
			
		ParticleCollection *collection = itr->second;	
		
		//This may be hidden by colour or size
		if(!collection->bShown){
			continue;
		}
				
		vector<Particle *> *list = &collection->mParticles;
		
		//Make sure we've got at least one particle
		if(list->size() == 0) {
			continue; 
		}
		
		//Set the state for this collection
		float s = collection->fSize * scale / 20.0f;
		collection->mColor.bind();
		
		bad++; //Count the number of state changes
		count += list->size(); //Count the number of particles
		
		for(int i=0;i<(int)list->size();i++){			
			Particle *p = (*list)[i];	
				
			float x = p->x;
			float y = p->y; 
			float z = p->z;

			glBegin(GL_TRIANGLE_STRIP);	
				glTexCoord2f(1,1); glVertex3f(x+s, y+s, z); // Top Right
				glTexCoord2f(0,1); glVertex3f(x-s, y+s, z); // Top Left
				glTexCoord2f(1,0); glVertex3f(x+s, y-s, z); // Bottom Right
				glTexCoord2f(0,0); glVertex3f(x-s, y-s, z); // Bottom Left
			glEnd();

				
		}
		
	}	
	

	//And clean up
	glDisable(GL_BLEND);	
	glDepthMask(GL_TRUE);
	
	iNumActive = count;
}


/*********************************************
 Figure out and set the dynamic scaling factor 
**********************************************/
float PSClassic::setSizeScale(){
	float scale = (1.0f/(float)iNumActive) * 150000;	
	if(scale < 5.0f){ scale = 5.0f; }
	else if(scale > MAX_SIZE * App::S()->fParticleSizeScale){	
		scale = MAX_SIZE * App::S()->fParticleSizeScale;
	}	
	scale *= App::S()->fParticleSizeScale;						
	return scale;
}

/*********************************************
	Add a new particle with defaults
**********************************************/
void PSClassic::add(Vector3 pos, Vector3 speed, Color col, 
					float size, float life){

	//First make sure we have capacity
	if(mFree.empty()){
		return;
	}
	
	//And sanity check to make sure it's valid
	Particle *p = mFree.top();
	
	if(!p){
		return;
	}
	
	//Take it off the list
	mFree.pop();
			
	//Apply some jitter
	float jitter = 0.0f;
	
	//Nasty hack! If we're in the title screen, jitter ruins it.
	if(App::S()->isConnected()){
		jitter = App::S()->randFloat(0, 1.0f);
	}
	
	pos = pos + speed * jitter;		
	p->life = life - jitter;
	
	//Position
	p->x = pos.x;
	p->y = pos.y;
	p->z = pos.z;
	
	//Velocity
	p->vx = speed.x;
	p->vy = speed.y;
	p->vz = speed.z;
		
	//Color
	p->r = col.r;
	p->g = col.g;
	p->b = col.b;
	
	//Size
	p->size = size;
	
	//Timestamp
	p->timestamp = fTime;
				
	//Add to the collection of particles
	getCollection(col, size)->mParticles.push_back(p);
				
	//And mark it as active
	p->active = true;
		
	//done!
	return;
}


/*********************************************
	Set up a particle collection object
**********************************************/
ParticleCollection *PSClassic::getCollection(Color col, float size){
	float val = col.sum() + (size * 100000.0f);
	
	ParticleCollection *existing = mParticleCollections[val];
	
	if(existing){
		return existing;
	}
	
	existing = new ParticleCollection();
	existing->fSize = size;
	existing->mColor = col;
	existing->bShown = true;
	existing->mList = 0;
	
	mParticleCollections[val] = existing;
	
	return existing;
}

/*********************************************
	Remove a particle by ID
**********************************************/
void PSClassic::del(ParticleCollection *col, int i){

	//Get a pointer to the particle we're talking about
	Particle *p = col->mParticles[i];
	
	//Remove it from the collection
	col->del(i);
	
	//And mark as inactive
	p->active = false;
	
	//Horrible hack - this means that if an active one gets rendered by
	//glDrawArrays (that doesn't check for p->active), it'll be nowhere to be
	//seen. This situation probably shouldn't happen, but just to be safe...
	p->x = -99999; p->y = -99999; p->z = -99999;
			
	//add to the free list
	mFree.push(p);	
}

/*********************************************
	Remove a bunch of particles
**********************************************/
void PSClassic::delAll(){

	map<float, ParticleCollection *>::const_iterator itr;
	for(itr = mParticleCollections.begin(); 
		itr != mParticleCollections.end(); ++itr){				
		ParticleCollection *collection = itr->second;		
		
		while(collection->mParticles.size() > 0){
			del(collection, 0);
		}
	}
	
}


/*********************************************
	Toggle showing a colour
**********************************************/
void PSClassic::showColor(Color c, bool bShow){

	map<float, ParticleCollection *>::const_iterator itr;
	for(itr = mParticleCollections.begin(); 
		itr != mParticleCollections.end(); ++itr){				
		ParticleCollection *collection = itr->second;		
		
		if(ABS(collection->mColor.sum() - c.sum()) < 0.0001f){
			collection->bShown = bShow;
		}
	}
}

/*********************************************
	Reset internal timestamps
**********************************************/
void PSClassic::doPeriodicCleanup(){
	
	map<float, ParticleCollection *>::const_iterator itr;
	for(itr = mParticleCollections.begin(); 
		itr != mParticleCollections.end(); ++itr){				
		ParticleCollection *collection = itr->second;		
		
		for(int i=0;i<collection->mParticles.size();i++){
			Particle *p = collection->mParticles[i];
			
			p->timestamp -= fTime;			
		}
	}
	
	fTime = 0.0f;
	
	//LOG("Cleanup!\n");
}




/*******************************************************************************
							ParticleCollection
*******************************************************************************/
void ParticleCollection::del(int i){
	Particle *p = mParticles[i];
	
	if(mParticles.size() > 0)	
		mParticles[i] = mParticles[mParticles.size() - 1];
		
	mParticles.pop_back();
}

