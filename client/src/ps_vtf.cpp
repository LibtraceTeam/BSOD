#include "main.h"

#define MAX_SIZE 10.0f
#define SHADER_FPS (1.0f / 10.0f) //The rate at which we push to the GPU

static map<float, ParticleCollection *>::const_iterator mCurrentCollection;
	
/*********************************************
 	Make sure we have VTF and shaders
**********************************************/
bool PSVTF::init(){

	//First make sure that we have shader support at all
	if (!GLEW_ARB_vertex_program || !GLEW_ARB_fragment_program){
		LOG("No GL_ARB_*_program\n");
		return false;
	}
	
	//And make sure we have at least one VTF texture unit
	int num = 0;
	glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS_ARB, &num);
	
	LOG("We have %d VTF units!\n", num);
	
	if(num <= 0){
		return false;
	}
	
	
		
	fUpdateTimer = 0.0f;

	//Now set up the shader
	string vs = "";
	char line[256];	

	//Load the shader text
	std::ifstream vs_file("data/shaders/ps_vtf.vert");
	
	if(vs_file.fail()){
		return false;
	}
	
	while(!vs_file.eof()){
		vs_file.getline(line, 256);
		vs += string(line) + "\n";
	}

	//Set up the shader object
	if(!mShader.addVertex(vs)) return false;
	//if(!mShader.addFragment(fs)) return false;
	
	if(!mShader.compile()) return false;
	
	//This means we can set pointsize in the vertex shader
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

	mParticleCollections.clear();

	//If we got this far, it's all good.	
	LOG("Set up PSVTF!\n");
	
	//Now set up the lower levels
	if(!PSSprites::init()){
		return false;
	}
	
	//Hack! We need to have at least one particle collection,
	//so add a particle here. This only appeared to be a problem
	//on W7. Oh well. 
	add(Vector3(0,0,0), Vector3(0,0,0), Color(0,0,0), 1.0f, 100.0f);

	//We can now make an iterator
	mCurrentCollection = mParticleCollections.begin();	
	
	return true;
}



/*********************************************
 Update the particles life, expire as necessary
**********************************************/
void PSVTF::update(){

	fTime += fTimeScale * App::S()->fParticleSpeedScale;
	
	if(mParticleCollections.size() == 0){
		return;
	}
		
	if(mCurrentCollection == mParticleCollections.end()){
		return;
	}
		
	updateCollection(mCurrentCollection->second);	
}

/*********************************************
	Render the display list as necessary
**********************************************/
void PSVTF::render(){

	if(mParticleCollections.size() == 0){
		return;
	}

	//Apply the shader
	mShader.bind();
	
	mTexture->bind();
			
	//Set up the shader
	float planeDist = App::S()->mFlowMgr->getPlaneDistance() / 2;
	
	mShader.bindResource("fTime", &fTime, 1);
	mShader.bindResource("fSlabOffset", &planeDist, 1);
		
	iNumActive = 0;
	
	//Now go through and call all the various display lists
	for(map<float, ParticleCollection *>::const_iterator itr = 
		mParticleCollections.begin(); 
		itr != mParticleCollections.end(); ++itr){	
		ParticleCollection *collection = itr->second;		
		
		if(collection->bShown){
			glCallList(collection->mList);
			iNumActive += collection->mParticles.size();
		}	
	}
	
	//LOG("%f\n", fTime);
	
	mShader.unbind();
}

/*********************************************
  Update an individual collection
**********************************************/
void PSVTF::updateCollection(ParticleCollection *collection){

	if(!collection){	
		return;
	}
		
	vector<Particle *> *list = &collection->mParticles;
	
	for(int i=0;i<(int)list->size();i++){			
		Particle *p = (*list)[i];			
		
		if(p->life < 0){
			del(collection, i);
			i--;
			continue;
		}
										
		//Move the particle
		p->life -= fTimeScale * 
					App::S()->fParticleSpeedScale * 
					mParticleCollections.size();	
		
		//p->timestamp += fUpdateTimer;
									
		//TODO: More particle logic here?
	}
}


/*********************************************
  Render points with the right normal + tex
**********************************************/
void PSVTF::renderAll(){

	mCurrentCollection++;	
	if(mCurrentCollection == mParticleCollections.end()){
		mCurrentCollection = mParticleCollections.begin();
	}
	
	ParticleCollection *collection = mCurrentCollection->second;		
	if(collection->mList == 0){
		collection->mList = glGenLists(1);
	}	
	glNewList(collection->mList, GL_COMPILE);
		
	//This may be hidden by colour or size
	if(!collection->bShown){
		return;
	}
	
	vector<Particle *> *list = &collection->mParticles;
	
	//Make sure we've got at least one particle
	if(list->size() == 0) {
		return; 
	}
	
	//LOG("Rendered %d\n", collection->mList); 
	
	int count = 0;
	int bad = 0;
	
	//Set GL state
	glEnable(GL_TEXTURE_2D);		
	glDepthMask(GL_FALSE);		
	glEnable(GL_BLEND);							
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glBlendFunc(GL_ONE, GL_ONE);	
		
	//Scaling	
	float scale = setSizeScale();							
	
	glBegin(GL_POINTS);		
	
	//Set the state for this collection
	float thisSize = collection->fSize * scale;
	collection->mColor.bind();
					
	//Now render all the particles in this list	
	//TODO: Use glDrawArrays!															
	for(int i=0;i<(int)list->size();i++){			
		Particle *p = (*list)[i];		
		
		//Normal = velocity
		glNormal3f(p->vx, p->vy, p->vz);
		
		//TexCoord.x = timestamp offset
		//TexCoord.y = size 
		glTexCoord2f(p->timestamp, thisSize);
		
		//Create the point
		glVertex3f(p->x, p->y, p->z);
	}					
	
	glEnd();

	//And clean up
	glDisable(GL_BLEND);	
	glDepthMask(GL_TRUE);
	
}


/*********************************************
	Kill the shader and the list
**********************************************/
void PSVTF::shutdown(){
	
	mShader.dispose();
	//glDeleteLists(mDisplayList, 1);
	
	for(map<float, ParticleCollection *>::const_iterator itr = 
		mParticleCollections.begin(); 
		itr != mParticleCollections.end(); ++itr){	
		ParticleCollection *collection = itr->second;		
		
		if(collection->bShown){
			glDeleteLists(collection->mList, 1);
		}	
	}
	
	PSSprites::shutdown();
}

