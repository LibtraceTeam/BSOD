#include "main.h"

//DevIL
#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>

vector<Texture *> mTextures;


/*********************************************
		Makes an empty texture obj
**********************************************/
static Texture *genTexObj(){

	Texture *tex = new Texture;
	
	if(!tex){
		return NULL;
	}
	
	tex->iSizeX = 0;
	tex->iSizeY = 0;
	tex->iGLID = -1;
	tex->iDevilID = -1;
	tex->mData = NULL;
	tex->mFilename = "";
	
	mTextures.push_back(tex);
	
	return tex;
}

/*********************************************
		Makes a DevIL image
**********************************************/
static ILuint makeImage(){
    ILuint ImageName; 
    ilGenImages(1, &ImageName);    
    return ImageName; 
}

/*********************************************
		Loads an image into DevIL
**********************************************/
Texture *App::texLoad(string name, int flags){

	string path = "data/" + name;	
	
	LOG("Loading '%s'...", path.c_str());

	Texture *tex = genTexObj();
	
	if(!tex){
		ERR("Couldn't allocate a texture object!\n");
		return NULL;
	}
		
	ILuint devilID = makeImage();		
	ilBindImage(devilID);
	
	tex->iDevilID = devilID;	
	tex->mFilename = name;
	
	//Load the data	
	if(!ilLoadImage((char *)path.c_str())){
		ERR("error: %s!\n", path.c_str());
		return NULL;
	}
		
	//ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE);
			
	tex->iSizeX = ilGetInteger(IL_IMAGE_WIDTH);
	tex->iSizeY = ilGetInteger(IL_IMAGE_HEIGHT);
	
	if(flags != TEXTURE_NO_GL){	
		tex->mData = ilGetData();
		tex->iGLID = ilutGLBindTexImage(); //ilutGLBindMipmaps();
		
		LOG("(got id %d)\n", tex->iGLID);	    
	}	
	
	return tex;
}

/*********************************************
	Generates a texture from a buffer
**********************************************/
/*
Texture *App::texGenerate(string name, byte *buffer, int width, int height){

	Texture *tex = genTexObj();
	
	if(!tex){
		ERR("Couldn't allocate a texture object!\n");
		return NULL;
	}
	
	tex->mFilename = name;
	
	GLuint i;

	glGenTextures(1, &i);
	glBindTexture(GL_TEXTURE_2D, i);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer);
	
	tex->iGLID = i;
		
	return tex; 
}
*/

/*********************************************
	Generates a texture from a buffer
**********************************************/
/*
void App::texRegenerate(Texture *t, byte *buffer, int width, int height){
	GLuint i;
	
	i = t->iGLID;
	
	glDeleteTextures(1, &i);

	glGenTextures(1, &i);
	glBindTexture(GL_TEXTURE_2D, i);
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer);

	t->iGLID = i;	
}
*/

/*********************************************
		Starts up the texture sys
**********************************************/
bool App::texInit(){
	
	
	if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION || 
		iluGetInteger(ILU_VERSION_NUM) < ILU_VERSION || 
		ilutGetInteger(ILUT_VERSION_NUM) < ILUT_VERSION) 
	{
		LOG("DevIL version is different...exiting!\n");
		return false;
	}
	
	ilInit();
	ilutRenderer(ILUT_OPENGL);
	
	//preload any common textures here
	mTextures.clear();
	
	string initialTextures[] = {"wm.png", 
								"ticked.png", 
								"unticked.png", 
								"particle.bmp"};
	
	for(int i=0;i<4;i++){
		if(!texLoad(initialTextures[i], 0)){
			return false;
		}
	}
	
	texLoad("banner.png", 0);
		
	LOG("Finished loading initial textures!\n");
	
	return true;

}

/*********************************************
		Cleans up any images we've loaded
**********************************************/
void App::texShutdown(){

	for(int i=0;i<(int)mTextures.size();i++){	
		ILuint id = mTextures[i]->iDevilID;		
		
		ilDeleteImages(1, &id);
						
		LOG("Freed '%s'\n", mTextures[i]->mFilename.c_str());
			
		delete mTextures[i];
	}
}


/*********************************************
		Returns a texture by name
**********************************************/
Texture *App::texGet(string name){

	for(int i=0;i<(int)mTextures.size();i++){
		Texture *t = mTextures[i];		
		if(t->mFilename == name){
			return t;
		}
	}
	return NULL;
}


