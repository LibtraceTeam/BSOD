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
	
	//Windows and Linux don't agree on which way images should be pointing.
	//Perry: grab a rusty spoon, and decapitate yourself, it's just easier.
#ifndef _WINDOWS
	iluFlipImage();
#endif

		
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

Texture *App::texGenerate(string name, byte *buffer, int buflen){
	
	LOG("Loading '%s' from buffer...", name.c_str());

	Texture *tex = genTexObj();
	
	if(!tex){
		ERR("Couldn't allocate a texture object!\n");
		return NULL;
	}
		
	ILuint devilID = makeImage();		
	ilBindImage(devilID);
	
	tex->iDevilID = devilID;	
	tex->mFilename = name;
	
	//Load the data	from the buf.
	if(!ilLoadL(0, buffer, buflen)){
		ERR("error!\n");
		return NULL;
	}
					
	tex->iSizeX = ilGetInteger(IL_IMAGE_WIDTH);
	tex->iSizeY = ilGetInteger(IL_IMAGE_HEIGHT);
	
	tex->mData = ilGetData();
	tex->iGLID = ilutGLBindTexImage(); //ilutGLBindMipmaps();
	
	LOG("(got id %d)\n", tex->iGLID);	    
	
	return tex;
}

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
	
	mLeftTex = mRightTex = NULL;
	
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
	
	if(mLeftTexName != ""){
		mLeftTex = texLoad(mLeftTexName, 0);
		if(!mLeftTex){
			return false;
		}
	}
	
	if(mRightTexName != ""){
		mRightTex = texLoad(mRightTexName, 0);
		if(!mRightTex){
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
		
		glDeleteTextures(1, &mTextures[i]->iGLID);
						
		LOG("Freed '%s'\n", mTextures[i]->mFilename.c_str());
			
		delete mTextures[i];
	}
	
	ilShutDown();
}

/*********************************************
		Delete a specific texture
**********************************************/
void App::texDelete(Texture *tex){
	ILuint id =tex->iDevilID;			
	ilDeleteImages(1, &id);	
	glDeleteTextures(1, &tex->iGLID);
	
	for(int i=0;i<mTextures.size();i++){
		if(mTextures[i] == tex){
			mTextures[i] = mTextures[mTextures.size() - 1];
			mTextures.pop_back();
			break;
		}
	}
					
	LOG("Freed '%s'\n", tex->mFilename.c_str());
	
	delete tex;
}


/*********************************************
		Returns a texture by name
**********************************************/
Texture *App::texGet(string name){

	for(int i=0;i<(int)mTextures.size();i++){
		Texture *t = mTextures[i];		
		if(t && t->mFilename == name){
			return t;
		}
	}
	return NULL;
}


