#include "main.h"


#define MAX_SIZE 10.0f

/*********************************************
		  Set up the framebuffer
**********************************************/
bool PSTexture::init(){

	LOG("Starting PSTexture!\n");
	/*
	if (!GLEW_EXT_framebuffer_object){
		LOG("No GLEW_EXT_framebuffer_object\n");
		return false;
	}*/
	
	height = App::S()->iScreenY / 2;
	width = App::S()->iScreenX / 2;
		
	//Set up the initial frame buffer object
	glGenFramebuffersEXT(1, &fbo);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);

	//Next we need to apply a depth component
	//TODO: Because we're rendering particles that have GL_DEPTH_TEST disabled,
	//do we actually need this?
	/*
	glGenRenderbuffersEXT(1, &depthbuffer);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthbuffer);
	
	//Allocate the memory on GPU
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, 
						  GL_DEPTH_COMPONENT, 
							width, height );
	
	//And connect up the FBO and depth buffer							
	glFramebufferRenderbufferEXT(	GL_FRAMEBUFFER_EXT, 
									GL_DEPTH_ATTACHMENT_EXT, 
									GL_RENDERBUFFER_EXT, 
									depthbuffer);
*/
	//Add the texture to render to
	glGenTextures(1, &img);
	glBindTexture(GL_TEXTURE_2D, img);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmapEXT(GL_TEXTURE_2D);

		
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8,  
					width, height, 0, GL_RGB, 
					GL_UNSIGNED_BYTE, NULL);
	
	//Connect up the FBO and texture
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, 
								GL_COLOR_ATTACHMENT0_EXT, 
								GL_TEXTURE_2D, img, 0);

	//OK. Now check that we did all the above correctly
	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);

	if(status != GL_FRAMEBUFFER_COMPLETE_EXT){
		LOG("Couldn't create FBO: Error code %d (0x%x)\n", status, status);
		return false;
	}
	
	LOG("Made the FBO of size %d/%d\n", width, height);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	
	return PSSprites::init();
}


/*********************************************
 	Render the particles to the texture 
**********************************************/
void PSTexture::render(){

	//Set up the render target
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	glPushAttrib(GL_VIEWPORT_BIT);
	glViewport(0,0,width, height);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	float ar = (float)App::S()->iScreenX / (float)App::S()->iScreenY;

	gluPerspective(70.0f,ar,0.5f,1000.0f);
	
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glDisable(GL_DEPTH_TEST);
	
	//Apply the transformation to screen space
	App::S()->camLook();
	
	//Render with point sprites as normal
	//renderAll();
	PSSprites::render();
	//PSClassic::render();

	//Now we have the texture. Unbind everything and go back to normal
	glPopAttrib();
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}


/*********************************************
 	Our internal particle rendering
**********************************************/
void PSTexture::renderAll(){

	//Set GL state	
	glDisable( GL_POINT_SPRITE_ARB );
	glPointSize(2.0f);
	//glBegin(GL_POINTS);	
	float scale = setSizeScale() * 0.25f;
			
	map<float, ParticleCollection *>::const_iterator itr;
	
	int count = 0;
	
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
		glPointSize(collection->fSize * scale);
		glBegin(GL_POINTS);
		
		//Set the state for this collection
		count += list->size(); //Count the number of particles
				
		//Now render all the particles in this list	
		//TODO: Use glDrawArrays!															
		for(int i=0;i<(int)list->size();i++){			
			Particle *p = (*list)[i];		
			glVertex3f(p->x, p->y, p->z);
		}	
		
		glEnd();
	
	}	

	//glEnd();
	
	iNumActive = count;
}

float PSTexture::setSizeScale(){
	float scale = (1.0f/(float)iNumActive) * 150000;	
	if(scale < 5.0f){ scale = 5.0f; }
	else if(scale > MAX_SIZE){	
		scale = MAX_SIZE;
	}	
	scale *= App::S()->fParticleSizeScale;	
	scale *= 0.5f; //The render target is half the size
				
	//Set a default size here
	glPointSize(scale); 	
	glPointParameterfARB( GL_POINT_SIZE_MIN_ARB, App::S()->fParticleSizeScale );
	glPointParameterfARB( GL_POINT_SIZE_MAX_ARB, fMaxSize );
	
	return scale;
}



/*********************************************
 	Render the texture to the screen 
**********************************************/
void PSTexture::render2D(){

	//OK. Now we need to draw a plane with the texture on, 
	//covering the entire screen. We draw it several times with additive
	//blending. This means it adds up to look better
	
	glBindTexture(GL_TEXTURE_2D, img);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glColor4f(0.5, 0.5, 0.5, 0.5);
	
	float h = App::S()->iScreenY;
	float w = App::S()->iScreenX;
	int iterations = 2;
	
	for(int x=0;x<iterations;x++){
		for(int y=0;y<iterations;y++){
	
			glBegin(GL_QUADS);
				glTexCoord2f(0,0); glVertex2f(x,y);
				glTexCoord2f(1,0); glVertex2f(x+w,y);
				glTexCoord2f(1,-1); glVertex2f(x+w,h+y);
				glTexCoord2f(0,-1); glVertex2f(x,h+y);
			glEnd();
		}
	
	}
	
}


/*********************************************
	Cleanups
**********************************************/
void PSTexture::shutdown(){
	glDeleteFramebuffersEXT(1, &fbo);
	glDeleteTextures(1, &img);
	
	PSSprites::shutdown();
}
