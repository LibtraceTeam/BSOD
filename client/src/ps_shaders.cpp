#include "main.h"

GLfloat vertices[] = 
{
	0.0f, -0.8f, 0.0f,
	-0.8f, 0.8f, 0.0f,
	0.8f, 0.8f, 0.0f
};

GLubyte colors[] = 
{
	255, 0, 0,
	0, 255, 0,
	0, 0, 255
};


#define MAX_SIZE 15.0f


//CPU-side records of particle data
//TODO: We should be smarter about how we cache and upload this!
Vector3 mPos[MAX_PARTICLES];
Vector3 mNormals[MAX_PARTICLES];

#define BUFFER_OFFSET(i) (void*)(0 + (i))

/*********************************************
 Start up the PS/VS extensions, load the shader
**********************************************/
bool PSShaders::init(){

	//First make sure that we have shader support at all
	if (!GLEW_ARB_vertex_program || !GLEW_ARB_fragment_program){
		LOG("No GL_ARB_*_program\n");
		return false;
	}
	
	//We also use VBOs. Probably if we have shaders we have VBOs, but better
	//safe than sorry
	if(!GLEW_ARB_vertex_buffer_object){
		LOG("No GL_ARB_vbo\n");
		return false;
	}
	
	iNumActive = 0;
	
	LOG("%d\n", sizeof(Vector3));
	
	for(int i=0;i<MAX_PARTICLES;i++){
		mPos[i] = Vector3(0,0,0);
		mNormals[i] = Vector3(1,0,0);
	}


	//Set up the VBO first
	//glEnableClientState( GL_COLOR_ARRAY );
	glEnableClientState( GL_VERTEX_ARRAY );
	
	//Generate the VBO object
	glGenBuffers( 1, &iVBO );
	glBindBuffer( GL_ARRAY_BUFFER, iVBO );
	
	//And allocated its memory
	glBufferData( GL_ARRAY_BUFFER,					
					sizeof(mPos) + sizeof(mNormals),
					NULL,									
					GL_DYNAMIC_DRAW );
		
	//Send a bunch of 0's across
	glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, 0, sizeof(mPos), &mPos[0].x);
	glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, sizeof(mPos), sizeof(mNormals), &mNormals[0].x);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//Done!
						


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
	
	
	
	
	//And finally, set up point sprites
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



	
	//If we got this far, it's all good.
	
	LOG("Set up PSShaders!\n");
	
	return true;
}

void PSShaders::render(){

	if(iNumActive == 0){
		return;
	}
	
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
	glColor3f(1,1,1);		
		
	float scale = (1.0f/(float)iNumActive) * 150000;	
	scale *= App::S()->fParticleSizeScale;	
	if(scale < 1.0f){ scale = 1.0f; }
			
	//Set a default size here
	glPointSize(scale); 	
	
	
	
	
	
	mShader.bind();
	
	float params[3] = {fTime, fTime, 0.0f};
	
	mShader.bindResource("fTime", params, 3);
	glBindBufferARB( GL_ARRAY_BUFFER, iVBO );
	
	glVertexPointer( 3, GL_FLOAT, 0, 0 );
	glNormalPointer( GL_FLOAT, 0, (void *)sizeof(mPos) );	
	
	glDrawArrays( GL_POINTS, 0, iNumActive );
	
	glBindBuffer( GL_ARRAY_BUFFER, 0);
	
	mShader.unbind();
	
	
	glDisable(GL_BLEND);	
	glDepthMask(GL_TRUE);
	
}	
	
	
void PSShaders::add(Vector3 pos, Vector3 speed, Color col, float size, float life){
	
	if(iNumActive >= MAX_PARTICLES){
		return;
	}
	
	mPos[iNumActive] = pos;
	mNormals[iNumActive] = Vector3(0,1,0); //speed.normalized();
	
	iNumActive++;
}


void PSShaders::update(){
	
	fTime += PARTICLE_FPS * 10.0f;
	
	//LOG("%f\n", fTime);
	
	//LOG("Update!\n");

	glBindBuffer( GL_ARRAY_BUFFER, iVBO );
					
	glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, 0, sizeof(Vector3) * iNumActive, &mPos[0].x);							
	
	glBindBuffer( GL_ARRAY_BUFFER, 0);
	

}

void PSShaders::shutdown(){
	glDeleteBuffers(1, &iVBO);
}


void PSShaders::showColor(Color c, bool b){

}
