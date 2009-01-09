#include "main.h"



struct ParticleSort{
    public: bool operator() (Particle *a, Particle *b){
		return (a->r + a->g + a->b) > (b->r + b->g + b->b);
    }
};

#define MAX_SIZE 10.0f


/*******************************************************************************
							PointSprites
*******************************************************************************/
void PSSprites::render(){
	
	//Get the texture first
	Texture *tex = App::S()->texGet("particle.bmp");
	if(!tex){
		LOG("Bad texture in PSSprites!\n");
		return;
	}else{
		tex->bind();		
	}
	
	int count = 0;
	int bad = 0;
	
	//Set GL state
	glEnable(GL_TEXTURE_2D);		
	glDepthMask(GL_FALSE);		
	glEnable(GL_BLEND);							
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);		
	
	
	
	
	float scale = (1.0f/(float)iNumActive) * 150000;	
	scale *= App::S()->fParticleSizeScale;	
	if(scale < 1.0f){ scale = 1.0f; }
		
		
		
	//Set a default size here
	glPointSize(scale); 	
	
	//glEnableClientState(GL_VERTEX_ARRAY);
	//glDisableClientState(GL_COLOR_ARRAY);
	
	//And begin drawing
	glBegin( GL_POINTS );
	
	map<float, vector<Particle *> *>::const_iterator itr;
	
	//Iterate over all the different colors
	for(itr = mColorMap.begin(); itr != mColorMap.end(); ++itr){	
		
		//The list of particles for this color
		vector<Particle *> *list = itr->second;	
		
		//Make sure we've got at least one particle
		if(list->size() == 0) {
			bad++;
			continue; 
		}
				
		Particle *first = (*list)[0];
		
		//Get the color and set it
		Color c = mColorLookup[itr->first];
		
		//Make sure this color is shown, and skip it if not
		if(mColorShown[c.sum()] == false){
			continue;
		}
		
		glColor3f(c.r, c.g, c.b);
				
		//glVertexPointer(3, GL_FLOAT, sizeof(Particle), &first->x);
		//glDrawArrays(GL_POINTS, 0, (int)list->size());
					
		//Go through the entire list	
		
		for(int i=0;i<(int)list->size();i++){			
			Particle *p = (*list)[i];		
			
			/*				
			if(!p->active){
				bad++;
				continue;
			}	
			*/			
			
			glVertex3f(p->x, p->y, p->z);
			count++;
		}	
		
	}
	
	//Finish drawing	
	glEnd();		

	//And clean up
	glDisable(GL_BLEND);	
	glDepthMask(GL_TRUE);
	//glDisableClientState(GL_VERTEX_ARRAY);
	
	iNumActive = count;
}

bool PSSprites::init(){

	if (!glewIsSupported("GL_VERSION_1_4  GL_ARB_point_sprite")){
		ERR("No point sprite support!\n");
	  	return false;
	}

	float maxSize = 0.0f;
	glGetFloatv( GL_POINT_SIZE_MAX_ARB, &maxSize );

	if( maxSize > MAX_SIZE * App::S()->fParticleSizeScale)
		maxSize = MAX_SIZE * App::S()->fParticleSizeScale;
			
	glPointSize( maxSize );
	glPointParameterfARB( GL_POINT_FADE_THRESHOLD_SIZE_ARB, 60.0f );
	glPointParameterfARB( GL_POINT_SIZE_MIN_ARB, 1.0f * App::S()->fParticleSizeScale );
	glPointParameterfARB( GL_POINT_SIZE_MAX_ARB, maxSize );
	glTexEnvf( GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE );
	glEnable( GL_POINT_SPRITE_ARB );

	return PSClassic::init();
}

/*******************************************************************************
							Classic particle system
*******************************************************************************/
bool PSClassic::init(){

	iLastColorChanges = 0;

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
				
	return true; //We assume we can always do this!
}

void PSClassic::shutdown(){
	//clear the stack
	while(!mFree.empty()){
		mFree.pop();
	}
	
	iNumActive = 0;
}

/*********************************************
	Move particles, expire as necessary
**********************************************/
void PSClassic::update(){

	map<float, vector<Particle *> *>::const_iterator itr;
	
	for(itr = mColorMap.begin(); itr != mColorMap.end(); ++itr){	
		vector<Particle *> *list = itr->second;	
		
		for(int i=0;i<(int)list->size();i++){			
			Particle *p = (*list)[i];			
			
			if(!p){
				continue;
			}
			
			if(!p->active){
				continue;
			}	
		
				
			//if(!p->active){
			//	continue;
			//}
		
			if(p->life < 0){
				del(itr->first, i);
				i--;
				continue;
			}
								
			//Move the particle
			p->x += p->vx * PARTICLE_FPS;
			p->y += p->vy * PARTICLE_FPS;
			p->z += p->vz * PARTICLE_FPS;
			p->life -= PARTICLE_FPS;
		
			
			if(p->life < 0.5f)
				p->a = (p->life * 2.0f);		
			else if(p->a < 1.0f)
				p->a += (PARTICLE_FPS * 2.0f);
				
			//TODO: More particle logic here?
		}
	}
	
}

/*********************************************
	Render using standard drawing
**********************************************/
void PSClassic::render(){

/*
	if(bNeedRecompile){
	
		glNewList(mDisplayList,GL_COMPILE);
			
		glEnable(GL_TEXTURE_2D);		
		App::S()->texGet("particle.bmp")->bind();		
		glDepthMask(GL_FALSE);		
		//glDisable(GL_DEPTH_TEST);		
		glEnable(GL_BLEND);							
		glBlendFunc(GL_SRC_ALPHA,GL_ONE);	
	
		glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);			
		glHint(GL_POINT_SMOOTH_HINT,GL_NICEST);		
	
		map<Color, vector<Particle *> *>::const_iterator itr;
		
		for(itr = mColorMap.begin(); itr != mColorMap.end(); ++itr){
		
			vector<Particle *> *list = itr->second;
		
			for(int i=0;i<list->size();i++){
		
				Particle *p = mActive[i];
		
				//if(!p->active){
				//	continue;
				//}
					
				glColor4f(p->r, p->g, p->b, p->a);
		
				float s = p->size;		
				float x = p->x;
				float y = p->y; 
				float z = p->z;
		
				glBegin(GL_TRIANGLE_STRIP);	
					glTexCoord2d(1,1); glVertex3f(x+s, y+s, z); // Top Right
					glTexCoord2d(0,1); glVertex3f(x-s, y+s, z); // Top Left
					glTexCoord2d(1,0); glVertex3f(x+s, y-s, z); // Bottom Right
					glTexCoord2d(0,0); glVertex3f(x-s, y-s, z); // Bottom Left
				glEnd();
		}
	
		glEnd();
		glDisable(GL_BLEND);	
		glDepthMask(GL_TRUE);
		//glEnable(GL_DEPTH_TEST);
		
		glEndList();	
		bNeedRecompile = false;			
	}
	
	glCallList(mDisplayList);
*/
}

/*********************************************
	Add a new particle with defaults
**********************************************/
void PSClassic::add(Vector3 pos, Vector3 speed, Color col, float size, float life){

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
	float jitter = App::S()->randFloat(0, 1.0f);
	pos = pos + speed * jitter;
		
	p->life = life - jitter;
	
	//Position
	p->x = pos.x;
	p->y = pos.y;
	p->z = pos.z;
	
	//Velocity
	p->vx = speed.x;// * (float)PARTICLE_FPS;
	p->vy = speed.y;// * (float)PARTICLE_FPS;
	p->vz = speed.z;// * (float)PARTICLE_FPS;
		
	//Color
	p->r = col.r;
	p->g = col.g;
	p->b = col.b;
	
	//Nasty nasty hack. 
	if(getType() == PARTICLE_SYSTEM_POINTSPRITES){
		p->size = size * App::S()->fParticleSizeScale * 3.0f;
	}else{
		p->size = size * App::S()->fParticleSizeScale * 0.05f;
	}
	
	//Get the list associated with this color. We batch this way so we don't
	//need to switch colors when rendering
	float c = col.sum();
	vector<Particle *> *mList = mColorMap[c];
	
	//If the list doesn't exist, create it
	if(!mList){
		mColorMap[c] = new vector<Particle *>();
		mList = mColorMap[c];
		mColorLookup[c] = col;
		mColorShown[c] = true; //Show by default
	}
	
	//Add to the list
	mList->push_back(p);
				
	//And mark it as active
	p->active = true;
		
	//done!
	return;
}

/*********************************************
	Remove a particle by ID
**********************************************/
void PSClassic::del(float col, int i){
	vector<Particle *> *mList = mColorMap[col];
	Particle *p = (*mList)[i];
	
	(*mList)[i] = (*mList)[mList->size() - 1];
	mList->pop_back();
		
	p->active = false;
	
	//Horrible hack - this means that if an active one gets rendered by
	//glDrawArrays (that doesn't check for p->active), it'll be nowhere to be
	//seen. This situation probably shouldn't happen, but just to be safe...
	p->x = -99999; p->y = -99999; p->z = -99999;
			
	//add to the free list
	mFree.push(p);	
}

/*********************************************	Remove a bunch of particles
**********************************************/
void PSClassic::delAll(){
	LOG("delAll stub called\n");
}

void PSClassic::showColor(Color c, bool bShow){
	mColorShown[c.sum()] = bShow;
}
