#include "main.h"

#define MAX_SIZE 10.0f
#define SHADER_FPS (1.0f / 15.0f)

/*********************************************
 Start up the PS/VS extensions, load the shader
**********************************************/
bool PSShaders::init(){

	//First make sure that we have shader support at all
	if (!GLEW_ARB_vertex_program || !GLEW_ARB_fragment_program){
		LOG("No GL_ARB_*_program\n");
		return false;
	}
	
	fTime = 0.0f;
	fUpdateTimer = 0.0f;

	//Now set up the shader
	string vs = "";
	char line[256];	

	//Load the shader text
	std::ifstream vs_file("data/shaders/ps.vert");
	while(!vs_file.eof()){
		vs_file.getline(line, 256);
		vs += string(line) + "\n";
	}

	//Set up the shader object
	if(!mShader.addVertex(vs)) return false;
	//if(!mShader.addFragment(fs)) return false;
	
	if(!mShader.compile()) return false;

	mDisplayList = glGenLists(1);
	
	//This means we can set pointsize in the vertex shader
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
		
	//If we got this far, it's all good.
	
	LOG("Set up PSShaders!\n");
	
	return PSSprites::init();
}



void PSShaders::update(){

	fTime += fTimeScale * App::S()->fParticleSpeedScale;
	fUpdateTimer += fTimeScale;
	
	if(fUpdateTimer < SHADER_FPS){
		return;
	}
	
	float speedScale = App::S()->fParticleSpeedScale * PARTICLE_FPS;

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
			p->life -= fUpdateTimer * App::S()->fParticleSpeedScale;	
			
			//p->timestamp += fUpdateTimer;
										
			//TODO: More particle logic here?
		}
	}
	
	
	fUpdateTimer = 0.0f;
	
}

void PSShaders::render(){


	//Apply the shader
	mShader.bind();
		
	//Set up the shader
	mShader.bindResource("fTime", &fTime, 1);

	fRenderTimer -= fTimeScale;
	if(fRenderTimer < -(SHADER_FPS / 2.0f)){
		glNewList(mDisplayList, GL_COMPILE);
		renderAll();
		glEndList();
		fRenderTimer = (SHADER_FPS / 2.0f);
	}
	glCallList(mDisplayList);
	
	mShader.unbind();
}

void PSShaders::renderAll(){
	
	mTexture->bind();
	
	int count = 0;
	int bad = 0;
	
	//Set GL state
	glEnable(GL_TEXTURE_2D);		
	glDepthMask(GL_FALSE);		
	glEnable(GL_BLEND);							
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);	
		
	//Scaling	
	float scale = setSizeScale();
			
	map<float, ParticleCollection *>::const_iterator itr;
	
	
	glBegin(GL_POINTS);	
	
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
		float thisSize = collection->fSize * scale;
		collection->mColor.bind();
		
		bad++; //Count the number of state changes
		count += list->size(); //Count the number of particles
				
		//Now render all the particles in this list	
		//TODO: Use glDrawArrays!															
		for(int i=0;i<(int)list->size();i++){			
			Particle *p = (*list)[i];		
			glNormal3f(p->vx, p->vy, p->vz);
			glTexCoord2f(p->timestamp, thisSize);
			glVertex3f(p->x, p->y, p->z);
		}		
				
	}	
	
	glEnd();

	//And clean up
	glDisable(GL_BLEND);	
	glDepthMask(GL_TRUE);
	
	iNumActive = count;
}

void PSShaders::shutdown(){
	
	mShader.dispose();
	glDeleteLists(mDisplayList, 1);
	
	PSSprites::shutdown();
}

