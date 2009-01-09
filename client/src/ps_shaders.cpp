#include "main.h"

#define MAX_SIZE 10.0f
#define SHADER_FPS (1.0f / 5.0f) //5fps

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

	mDisplayList = glGenLists(1);
		
	//If we got this far, it's all good.
	
	LOG("Set up PSShaders!\n");
	
	return PSSprites::init();
}



void PSShaders::update(){

	fTime += fTimeScale;
	fUpdateTimer += fTimeScale;
	
	if(fUpdateTimer < SHADER_FPS){
		return;
	}
			
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
			//p->x += p->vx * fUpdateTimer;
			//p->y += p->vy * fUpdateTimer;
			//p->z += p->vz * fUpdateTimer;
			p->life -= fUpdateTimer;
		
			/*
			if(p->life < 0.5f)
				p->a = (p->life * 2.0f);		
			else if(p->a < 1.0f)
				p->a += (fUpdateTimer * 2.0f);
			*/
			
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
				
		//Go through the entire list			
		for(int i=0;i<(int)list->size();i++){			
			Particle *p = (*list)[i];		
			glNormal3f(p->vx, p->vy, p->vz);
			glTexCoord2f(p->timestamp, 0.0f);
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

void PSShaders::shutdown(){
	
	PSSprites::shutdown();
}


void PSShaders::add(Vector3 pos, Vector3 speed, Color col, float size, float life){

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
	p->timestamp = fTime;
	
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
	
	//Nasty nasty hack. 
	if(getType() != PARTICLE_SYSTEM_CLASSIC){
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
