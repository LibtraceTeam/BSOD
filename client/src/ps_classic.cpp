#include "ps_classic.h"

void PSSprites::renderAll(){
	glColor3f(1,1,1);	
	glEnable(GL_TEXTURE_2D);	
	Texture *tex = App::S()->texGet("particle.bmp");
	if(!tex){
		//LOG("Bad texture in PSSprites!\n");
		//return;
	}else{
		tex->bind();		
	}
	glDepthMask(GL_FALSE);		
	//glDisable(GL_DEPTH_TEST);		
	glEnable(GL_BLEND);							
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);	

	float lastSize = 0.0f;
	float lastColor[3] = {0,0,0};

	glBegin( GL_POINTS );
	
	for(int i=0;i<iNumActive;i++){
	
		Particle *p = mActive[i];
	
		if(!p->active){
			continue;
		}
	
		if(p->size != lastSize){
			glEnd();
			glPointSize(p->size); //hack!	
			//glPointParameterfARB( GL_POINT_SIZE_MAX_ARB, p->size );	
			lastSize = p->size;
			//glColor4f(p->r, p->g, p->b, p->a);			
			glBegin(GL_POINTS);
		}
		
		if(lastColor[0] != p->r || lastColor[1] != p->g || lastColor[2] != p->b){
			glColor4f(p->r, p->g, p->b, p->a);	
			lastColor[0] = p->r;
			lastColor[1] = p->g;
			lastColor[2] = p->b;
		}
		glVertex3f(p->x, p->y, p->z);
	
	}	
	glEnd();		

	glDisable(GL_BLEND);	
	glDepthMask(GL_TRUE);
	//glEnable(GL_DEPTH_TEST);
}

void PSSprites::render(){
	if(App::S()->bCGLCompat){
		renderAll();		
	}else{
		if(bNeedRecompile){	
			glNewList(mDisplayList,GL_COMPILE);
			renderAll();	
			glEndList();	
			bNeedRecompile = false;			
		}	
		glCallList(mDisplayList);		
	}
}

bool PSSprites::init(){

	if(!App::S()->bCGLCompat){

		char *ext = (char*)glGetString( GL_EXTENSIONS );
	
		if( strstr( ext, "GL_ARB_point_parameters" ) == NULL ){
			LOG("GL_ARB_point_parameters extension was not found, falling back to triangles\n");
			return false;
		}else{	
	
			//FIXME: Windows needs wglGetProcAddress, probably
			glPointParameterfARB  = (PFNGLPOINTPARAMETERFARBPROC)glXGetProcAddress((byte *)"glPointParameterfARB");
			glPointParameterfvARB = (PFNGLPOINTPARAMETERFVARBPROC)glXGetProcAddress((byte *)"glPointParameterfvARB");

			if( !glPointParameterfARB || !glPointParameterfvARB ){
				LOG("GL_ARB_point_parameter functions were not found, falling back to triangles\n");
				return false;
			}
		}
		
		LOG("GL_ARB_point_parameters loaded OK\n");


		float maxSize = 0.0f;
		glGetFloatv( GL_POINT_SIZE_MAX_ARB, &maxSize );

		if( maxSize > 5.0f * App::S()->fParticleSizeScale)
			maxSize = 5.0f * App::S()->fParticleSizeScale;
				
		glPointSize( maxSize );
		glPointParameterfARB( GL_POINT_FADE_THRESHOLD_SIZE_ARB, 60.0f );
		glPointParameterfARB( GL_POINT_SIZE_MIN_ARB, 1.0f * App::S()->fParticleSizeScale );
		glPointParameterfARB( GL_POINT_SIZE_MAX_ARB, maxSize );
		glTexEnvf( GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE );
		glEnable( GL_POINT_SPRITE_ARB );

	}

	return PSClassic::init();
}


/*********************************************
		Particle system class
**********************************************/
bool PSClassic::init(){

	while(!mFree.empty()){
		mFree.pop();
	}
		
	for(int i=MAX_PARTICLES - 1;i>=0;i--){
		mParticles[i].active = false;
		mParticles[i].life = 0.0f;
		mParticles[i].index = i;
		
		mFree.push(&mParticles[i]);		
	}
	
	iNumActive = 0;
	
	bNeedRecompile = true;

	if(!App::S()->bCGLCompat)
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

	for(int i=0;i<(int)mActive.size();i++){
		
		Particle *p = mActive[i];
				
		//if(!p->active){
		//	continue;
		//}
		
		if(p->life < 0){
			del(i);
			i--;
			continue;
		}
								
		//Move the particle
		p->x += p->vx;
		p->y += p->vy;
		p->z += p->vz;
		p->life -= PARTICLE_FPS;
		
			
		if(p->life < 0.5f)
			p->a = (p->life * 2.0f);		
		else if(p->a < 1.0f)
			p->a += (PARTICLE_FPS * 2.0f);
				
		//TODO: More particle logic here?
	}
	
	iNumActive = (int)mActive.size();
	
	bNeedRecompile = true;
}

/*********************************************
	Render using standard drawing
**********************************************/
void PSClassic::render(){
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
	
		
		for(int i=0;i<iNumActive;i++){
		
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
}

/*********************************************
	Add a new particle with defaults
**********************************************/
void PSClassic::add(Vector3 pos, Vector3 speed, Color col, float size, float life){

	if(mFree.empty()){
		//LOG("PSClassic: No particles in free list!\n");		
		return;
	}
	
	Particle *p = mFree.top();
	
	if(!p){
		return;
	}
	
	mFree.pop();
			
	//jitter
	float jitter = App::S()->randFloat(0, 1.0f);
	pos = pos + speed * jitter;
		
	p->life = life - jitter;
	
	p->x = pos.x;
	p->y = pos.y;
	p->z = pos.z;
	
	p->vx = speed.x * (float)PARTICLE_FPS;
	p->vy = speed.y * (float)PARTICLE_FPS;
	p->vz = speed.z * (float)PARTICLE_FPS;
		
	p->r = col.r;
	p->g = col.g;
	p->b = col.b;
	
	
	
	//hack!
	if(getType() == PARTICLE_SYSTEM_POINTSPRITES){
		p->size = size * App::S()->fParticleSizeScale * 3.0f;
	}else{
		p->size = size * App::S()->fParticleSizeScale * 0.05f;
	}
	
	mActive.push_back(p);
				
	p->active = true;
		
	return;
}

/*********************************************
	Remove a particle by ID
**********************************************/
void PSClassic::del(int i){
	Particle *p = mActive[i];
	
	mActive[i] = mActive[mActive.size() - 1];
	mActive.pop_back();
		
	p->active = false;
			
	//add to the free list
	mFree.push(p);	
}

void PSClassic::delAll(){
	while(mActive.size() > 0){
		del(0);
	}
}

void PSClassic::delColor(Color c){
	for(int i=0;i<(int)mActive.size();i++){
		if(mActive[i]->r == c.r && 
			mActive[i]->g == c.g && 
			mActive[i]->b == c.b){			
			del(i);
			i--;
		}
	}
}
