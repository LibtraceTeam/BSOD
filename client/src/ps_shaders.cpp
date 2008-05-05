#include "ps_shaders.h"

#define NUM_VERTS 100
	
typedef struct{
	float x, y, z;
}VERT;

typedef struct{
	float r, g, b;
}COL;

VERT mVerts[NUM_VERTS];
VERT mNorms[NUM_VERTS];
COL mCol[NUM_VERTS];

GLuint mVertexBuffer;
GLuint mNormalBuffer;
GLuint mColorBuffer;


//VBO pointers
PFNGLGENBUFFERSARBPROC glGenBuffersARB = NULL;
PFNGLBINDBUFFERARBPROC glBindBufferARB = NULL;
PFNGLBUFFERDATAARBPROC glBufferDataARB = NULL;	
PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB = NULL;	

/*********************************************
 Start up the PS/VS extensions, load the shader
**********************************************/
bool PSShaders::init(){

	cgContext = cgCreateContext();	

	if(!cgContext){
		LOG("Failed to init CG!\n");
		return false;
	}

	cgVertexProfile = cgGLGetLatestProfile(CG_GL_VERTEX);	

	if(cgVertexProfile == CG_PROFILE_UNKNOWN){
		LOG("Couldn't get CG Vertex profile\n");
		return false;
	}
	
	cgGLSetOptimalOptions(cgVertexProfile);	

	//Compile our shader
	cgProgram = cgCreateProgramFromFile(cgContext, CG_SOURCE, "./data/particle.cg", cgVertexProfile, "main", 0);

	if(!cgProgram){	
		CGerror err = cgGetError();
		LOG("CG: %s \n", cgGetErrorString(err));
		return false;
	}

	//If we get here, CG works and our program is OK.
	cgGLLoadProgram(cgProgram);

	//Get handles
	position		= cgGetNamedParameter(cgProgram, "IN.position");
	color			= cgGetNamedParameter(cgProgram, "IN.color");
	modelViewMatrix	= cgGetNamedParameter(cgProgram, "ModelViewProj");
	//time			= cgGetNamedParameter(cgProgram, "IN.time");
	
	iNumActive = 0;
	
	//Build up the test array
	for(int i=0;i<NUM_VERTS;i++){
		mVerts[i].x = -15;
		mVerts[i].y = App::S()->randFloat(-10, 10);
		mVerts[i].z = App::S()->randFloat(-10, 10);
	}
	
	for(int i=0;i<NUM_VERTS;i++){
		mNorms[i].x = 15;
		mNorms[i].y = App::S()->randFloat(-10, 10);
		mNorms[i].z = App::S()->randFloat(-10, 10);
		
		mCol[i].r = App::S()->randFloat();
		mCol[i].g = App::S()->randFloat();
		mCol[i].b = App::S()->randFloat();
	}
	
	//Get the VBO extensions
	glGenBuffersARB = (PFNGLGENBUFFERSARBPROC) glXGetProcAddress((byte *)"glGenBuffersARB");
	glBindBufferARB = (PFNGLBINDBUFFERARBPROC) glXGetProcAddress((byte *)"glBindBufferARB");
	glBufferDataARB = (PFNGLBUFFERDATAARBPROC) glXGetProcAddress((byte *)"glBufferDataARB");
	glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC) glXGetProcAddress((byte *)"glDeleteBuffersARB");
	
	//Generate the vertex data
	glGenBuffersARB( 1, &mVertexBuffer );			
	glBindBufferARB( GL_ARRAY_BUFFER_ARB, mVertexBuffer );	
	glBufferDataARB( GL_ARRAY_BUFFER_ARB, NUM_VERTS * 3 * sizeof(float), mVerts, GL_STATIC_DRAW_ARB );
	
	//Generate the vertex data
	glGenBuffersARB( 1, &mNormalBuffer );			
	glBindBufferARB( GL_ARRAY_BUFFER_ARB, mNormalBuffer );	
	glBufferDataARB( GL_ARRAY_BUFFER_ARB, NUM_VERTS * 3 * sizeof(float), mNorms, GL_STATIC_DRAW_ARB );
		
	glGenBuffersARB( 1, &mColorBuffer );			
	glBindBufferARB( GL_ARRAY_BUFFER_ARB, mColorBuffer );	
	glBufferDataARB( GL_ARRAY_BUFFER_ARB, NUM_VERTS * 3 * sizeof(float), mCol, GL_STATIC_DRAW_ARB );
	
	
	//Start up POINT_SPRITE
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
	
	iNumActive = NUM_VERTS;
	
	return true;
}

void PSShaders::render(){

	glColor3f(1,1,1);

	cgGLSetStateMatrixParameter(modelViewMatrix, CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);
	cgGLEnableProfile(cgVertexProfile);
	cgGLBindProgram(cgProgram);
	//cgGLSetParameter3f(time, fTime, 0, 0);		
	//cgGLSetParameter4f(color, 0.5f, 1.0f, 0.5f, 1.0f);
	
	glTexCoord2f(fTime / 20.0f, 0.0f);
	
	glEnable(GL_TEXTURE_2D);		
	App::S()->texGet("particle.bmp")->bind();		
	glDepthMask(GL_FALSE);		
	//glDisable(GL_DEPTH_TEST);		
	glEnable(GL_BLEND);							
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);	
	glPointSize(5.0f);
	
	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	
	glBindBufferARB( GL_ARRAY_BUFFER_ARB, mVertexBuffer );
	glVertexPointer( 3, GL_FLOAT, 0, (char *) NULL );	
	
	glBindBufferARB( GL_ARRAY_BUFFER_ARB, mNormalBuffer );
	glNormalPointer( GL_FLOAT, 0, (char *) NULL );	
	
	glBindBufferARB( GL_ARRAY_BUFFER_ARB, mColorBuffer );
	glColorPointer( 3, GL_FLOAT, 0, (char *) NULL );	
	
	glDrawArrays( GL_POINTS, 0, NUM_VERTS );
	
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState( GL_VERTEX_ARRAY );		
	
	cgGLUnbindProgram(cgVertexProfile);
	cgGLDisableProfile(cgVertexProfile);
	
}	
	
	
void PSShaders::add(Vector3 pos, Vector3 speed, Color col, float size, float life){
	
}


void PSShaders::update(){
	fTime += PARTICLE_FPS;
}

void PSShaders::shutdown(){
	cgDestroyContext(cgContext);
}
	
