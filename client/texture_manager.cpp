#include "stdafx.h"
#include "texture_manager.h"
#include "display_manager.h"
#include "world.h"
#include "vfs.h"
#include "exception.h"

#include <math.h>
#include "external/DevIL/include/il/il.h"
#include "external/DevIL/include/il/ilu.h"

// Singleton object instantiation
CTextureManager CTextureManager::tm;

static bool IsPowerOfTwo(int num)
{
	for(int i = 0; i < 12; i++)
		if(num == pow(2, i))
			return true;

	return false;
}

CTextureManager::CTextureManager() 
{
	// Initialise DevIL il - low level part of the lib
	ilInit();
	// Initialise DevIL ilu - mid level part of the lib
	iluInit();

	ilEnable(IL_CONV_PAL);
}

CTextureManager::~CTextureManager() {
	for(list<CTexture *>::iterator i = textures.begin(); i != textures.end();  ++i)
		delete *i;
}

CTexture *CTextureManager::LoadTexture(string texName)
{
	unsigned int glId, ilId;
	int width, height, orig_width, orig_height;
	char *buf;
	auto_ptr<CReader> reader;
	CTexture *tex = NULL;
	int ilType = IL_BGR;
	CDisplayManager::ImageType type = CDisplayManager::R8_G8_B8;
	// TODO: fix this: OpenGL really wants RGB and D3D wanta BGR
		//IL_RGB; // We might as well always use RGBA - on NVidia cards, textures are always stored in 32bit-per-pixel
	
	if( (tex = FindTexture(texName)) )
		return tex;
	
	// Generate the main image name to use.
	ilGenImages(1, &ilId);
	// Bind this image name.
	ilBindImage(ilId);

	// Load image into memory and process
	reader = auto_ptr<CReader>(CVFS::LoadFile(texName));
	buf = new char [reader->GetLength()];
	reader->Read(buf, reader->GetLength());

	if(! ilLoadL(IL_TYPE_UNKNOWN, buf, reader->GetLength()) )
	{
		Log("Error %d\n", ilGetError());
		throw CException("OpenIL unable to determine image format!");
	}

	type = world.display->RequestSupportedImageType(type);

	// Convert image to RGB using unsigned bytes
	ilEnable(IL_FORMAT_SET);
	ilEnable(IL_TYPE_SET);
	
	switch(type)
	{
	case CDisplayManager::R8_G8_B8:		ilType = IL_RGB; break;
	case CDisplayManager::R8_G8_B8_A8:	ilType = IL_RGBA; break;
	case CDisplayManager::B8_G8_R8:		ilType = IL_BGR; break;
	case CDisplayManager::B8_G8_R8_A8:	ilType = IL_BGRA; break;
	case CDisplayManager::Luminance8:	ilType = IL_LUMINANCE; break;
	};
	ilConvertImage(ilType, IL_UNSIGNED_BYTE);

	height = orig_height = ilGetInteger(IL_IMAGE_HEIGHT);
	width = orig_width = ilGetInteger(IL_IMAGE_WIDTH);

	if( !IsPowerOfTwo(height) || !IsPowerOfTwo(width) )
	{
		if(!IsPowerOfTwo(height))
		{
			height = (int)pow(2, floor(log(height) / log(2.0)));
		}
		if(!IsPowerOfTwo(width))
		{
			width = (int)pow(2, floor(log(width) / log(2.0)));
		}

		iluScale(width, height, ilGetInteger(IL_IMAGE_DEPTH));
	}

	glId = world.display->LoadTexture(ilGetData(), type, width, height);

	ilDeleteImages(1, &ilId);

	CTexture *newTex = new CTexture;
	newTex->name = texName;
	newTex->id = glId;
	newTex->width = width;
	newTex->height = height;
	newTex->orig_width = orig_width;
	newTex->orig_height = orig_height;

	textures.push_back(newTex);

	delete [] buf;

	return newTex;
}

void CTextureManager::UnloadTexture(string texName)
{
	UnloadTexture(FindTexture(texName));
}

void CTextureManager::UnloadTexture(CTexture *tex)
{
	if(!tex)
		return;

	world.display->UnloadTexture(tex);

	// XXX: This is somewhat bad.  Makes unloading textures O(n) where I'm sure it
	// could be better with a bit of thought.  Does this even matter? - Sam
	list<CTexture *>::iterator i;
	for(i = textures.begin(); i != textures.end(); ++i)
	{
		if(*i == tex)
		{
			delete *i;
			textures.erase(i);
			break;
		}
	}
}

CTexture *CTextureManager::FindTexture(string texName)
{
	list<CTexture *>::iterator i = textures.begin();
	for(; i != textures.end(); ++i)
	{
		// XXX: Should this be case sensitive?
		if((*i)->name.compare(texName) == 0)
		{
			return (*i);
		}
	}

	return NULL;
}
