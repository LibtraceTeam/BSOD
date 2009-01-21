/*******************************************************************************
							BSOD2 Client - texture.h

 A very simple GL texture object. Loaded and manipulated by the tex*() funcs
*******************************************************************************/

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

