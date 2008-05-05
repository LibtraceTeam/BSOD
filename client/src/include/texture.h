#include "main.h"

#ifndef _TEXTURE_H
#define _TEXTURE_H

/*********************************************
		Texture object
**********************************************/
class Texture{
public:
	int iSizeX;
	int iSizeY;
	GLuint iGLID;
	int iDevilID;
		
	string mFilename;
	
	byte *mData;
	
	void bind(){
		glBindTexture(GL_TEXTURE_2D, iGLID);
	}

};

/*********************************************
		Texture flags
**********************************************/
#define TEXTURE_NO_GL 1

#endif
