/* 
 * This file is part of BSOD client
 *
 * Copyright (c) 2011 The University of Waikato, Hamilton, New Zealand.
 *
 * Author: Paul Hunkin
 *
 * Contributors: Shane Alcock
 *
 * All rights reserved.
 *
 * This code has been developed by the University of Waikato WAND research
 * group. For further information please see http://www.wand.net.nz/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $Id$
 */

#include "config.h"
#include "main.h"

//#define ILUT_USE_OPENGL

//DevIL
//#include <IL/il.h>
//#include <IL/ilu.h>
//#include <IL/ilut.h>

#include <png.h>

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
	tex->mFree = false;
	tex->mFilename = "";
	
	mTextures.push_back(tex);
	
	return tex;
}

/*********************************************
		Makes a DevIL image
**********************************************/
#if 0
static ILuint makeImage(){
    ILuint ImageName; 
    ilGenImages(1, &ImageName);    
    return ImageName; 
}
#endif

struct imgBuffer {
	byte *buffer;
	size_t offset;
	size_t buflen;
};

static void readPngBuffer(png_structp pngPtr, png_bytep data, png_size_t len) {
	png_voidp a = png_get_io_ptr(pngPtr);
	struct imgBuffer *imgbuf = (struct imgBuffer *)a;
	
	memcpy((char *)data, imgbuf->buffer + imgbuf->offset, len);
	imgbuf->offset += len;
	if (imgbuf->offset > imgbuf->buflen)
		imgbuf->offset = imgbuf->buflen;
}


/* One of fp and buffer must be NULL ! */
static Texture* loadPngTexture(FILE *fp, byte *buffer, int buflen, int flags) {

	png_byte header[8];
	int bit_depth, color_type;
	png_uint_32 twidth, theight;
	int is_png;
	struct imgBuffer imgbuf;

	Texture *tex = genTexObj();
	
	if(!tex){
		ERR("Couldn't allocate a texture object!\n");
		return NULL;
	}
	
	assert(fp == NULL || buffer == NULL);
	assert(!(fp == NULL && buffer == NULL));

	if (fp) {
		if (fread(header, 1, 8, fp) != 8) {
			LOG("Failed to read PNG header\n");
			return NULL;
		}
		is_png = !png_sig_cmp(header, 0, 8);
	} else {
		is_png = !png_sig_cmp(buffer, 0, 8);
	}
	
	if (!is_png) {
		LOG("Texture must be a .png file!\n");
		return NULL;
	}

	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
			NULL, NULL, NULL);
	if (!png_ptr) {
		LOG("Unable to create PNG read structure\n");
		return NULL;
	}
	
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		LOG("Unable to create info ptr\n");
		png_destroy_read_struct(&png_ptr, (png_infopp) NULL,
				(png_infopp) NULL);
		return NULL;
	}
	
	png_infop end_ptr = png_create_info_struct(png_ptr);
	if (!end_ptr) {
		LOG("Unable to create end ptr\n");
		png_destroy_read_struct(&png_ptr, &info_ptr,
				(png_infopp) NULL);
		return NULL;
	}
	
	if (setjmp(png_jmpbuf(png_ptr))) {
		LOG("Error while reading the PNG\n");
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_ptr);
		return NULL;
	}

	if (fp) {
		png_init_io(png_ptr, fp);
		png_set_sig_bytes(png_ptr, 8);
	} else {

		imgbuf.buffer = buffer;
		imgbuf.offset = 0;
		imgbuf.buflen = buflen;

		png_set_read_fn(png_ptr, &imgbuf, readPngBuffer);
	}

	png_read_info(png_ptr, info_ptr);

	png_get_IHDR(png_ptr, info_ptr, &twidth, &theight, &bit_depth,
			&color_type, NULL, NULL, NULL);

	tex->iSizeX = twidth;
	tex->iSizeY = theight;

	png_read_update_info(png_ptr, info_ptr);

	/*
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
		png_set_tRNS_to_alpha(png_ptr);

    	}
	*/	

	int rowbytes = png_get_rowbytes(png_ptr, info_ptr);
	png_byte *image_data = new png_byte[rowbytes * theight];

	if (!image_data) {
		LOG("Failed to read allocate image data \n");
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_ptr);
		return NULL;
	}

	png_bytep *row_pointers = new png_bytep[theight];
	if (!row_pointers) {
		LOG("Failed to allocate row pointers\n");
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_ptr);
		return NULL;
	}

	
	for (int i = 0; i < theight; ++i) {
		/* Account for images that have been sent over the network
		 * being rendered upside down on Windows clients :( */
		
#ifndef _WINDOWS
		row_pointers[i] = image_data + i * rowbytes;
#else
		if (!fp) {
			row_pointers[i] = image_data + 	
				(theight - 1 - i) * rowbytes;
		}
		else {
			row_pointers[i] = image_data + i * rowbytes;
		}
#endif
	}

	png_read_image(png_ptr, row_pointers);
	bool hasAlpha = false;
	switch(color_type) {
		case PNG_COLOR_TYPE_RGBA:
			hasAlpha = true;
			break;
		case PNG_COLOR_TYPE_RGB:
			hasAlpha = false;
			break;
		default:
			LOG("Unknown color type: %u\n", color_type);
			png_destroy_read_struct(&png_ptr, &info_ptr, &end_ptr);
			return NULL;
	}
			
	

	if (flags != TEXTURE_NO_GL) {
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, 
				hasAlpha ? GL_RGBA : GL_RGB, 
				twidth, theight, 0, 
				hasAlpha ? GL_RGBA : GL_RGB, 
				GL_UNSIGNED_BYTE, 
				(GLvoid *) image_data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
				GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		tex->iGLID = texture;
		LOG("(got id %d)\n", tex->iGLID);	    
	}

	png_destroy_read_struct(&png_ptr, &info_ptr, &end_ptr);
	delete [] row_pointers;

	tex->mFree = false;
	tex->mData = (byte *)image_data;

	return tex;

}

/*********************************************
		Loads an image into DevIL
**********************************************/
Texture *App::texLoad(string name, int flags){

#ifdef _WINDOWS
	string path = "data\\" + name;
#else
	string path = "data/" + name;	
#endif
	
	LOG("Loading '%s'...", path.c_str());
	FILE *fp = fopen(path.c_str(), "rb");
	if (!fp) {
		LOG("Failed to open file\n");
		return NULL;
	}

	Texture *tex = loadPngTexture(fp, NULL, 0, flags);
	if (tex)
		tex->mFilename = name;
	fclose(fp);
	return tex;

#if 0

	ILuint devilID = makeImage();		
	ilBindImage(devilID);
	
	tex->iDevilID = devilID;	
	tex->mFilename = name;
	
	//Load the data	
	if(!ilLoadImage((char *)path.c_str())){
		int err = ilGetError();
		ERR("error: %s, %d!\n", path.c_str(), err);
		return NULL;
	}
	
	//Windows and Linux don't agree on which way images should be pointing.
	//Perry: grab a rusty spoon, and decapitate yourself, it's just easier.
#ifndef _WINDOWS
	iluFlipImage();
#endif

	//OK, so we've flipped it dependent on platform already. Now if
	//necessary we flip *again*, to observe what the user wants. 
	if(bTexFlip){
		iluFlipImage();
	}

		
	ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
			
	tex->iSizeX = ilGetInteger(IL_IMAGE_WIDTH);
	tex->iSizeY = ilGetInteger(IL_IMAGE_HEIGHT);
	
	if(flags != TEXTURE_NO_GL){	
		tex->mFree = true;
		tex->mData = new byte[tex->iSizeX * tex->iSizeY * 4];
		ilCopyPixels(0, 0, 0, tex->iSizeX, tex->iSizeY, 1, IL_RGBA,
				IL_UNSIGNED_BYTE, tex->mData);
		
		tex->iGLID = ilutGLBindTexImage(); //ilutGLBindMipmaps();
		
		LOG("(got id %d)\n", tex->iGLID);	    
	}	
	
	return tex;
#endif
}

/*********************************************
	Generates a texture from a buffer
**********************************************/

Texture *App::texGenerate(string name, byte *buffer, int buflen){
	
	LOG("Loading '%s' from buffer...", name.c_str());

	Texture *tex = loadPngTexture(NULL, buffer, buflen, 0);
	if (tex)
		tex->mFilename = name;
	return tex;
#if 0



	ILuint devilID = makeImage();		
	ilBindImage(devilID);
	
	tex->iDevilID = devilID;	
	tex->mFilename = name;
	
	//Load the data	from the buf.
	if(!ilLoadL(0, buffer, buflen)){
		ERR("error!\n");
		return NULL;
	}
	
#ifndef _WINDOWS
	iluFlipImage();
#endif
	
	if(bTexFlip){
		iluFlipImage();
	}
					
	tex->iSizeX = ilGetInteger(IL_IMAGE_WIDTH);
	tex->iSizeY = ilGetInteger(IL_IMAGE_HEIGHT);
	
	tex->mData = ilGetData();
	tex->iGLID = ilutGLBindTexImage(); //ilutGLBindMipmaps();
	
#endif
}

/*********************************************
		Starts up the texture sys
**********************************************/
bool App::texInit(){
	
#if 0	
	if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION || 
		iluGetInteger(ILU_VERSION_NUM) < ILU_VERSION || 
		ilutGetInteger(ILUT_VERSION_NUM) < ILUT_VERSION) 
	{
		LOG("DevIL version is different...exiting!\n");
		return false;
	}
	
	ilInit();
	ilutRenderer(ILUT_OPENGL);
#endif

	mLeftTex = mRightTex = NULL;
	
	//preload any common textures here
	mTextures.clear();
	
	/*
	string initialTextures[] = {"particle.bmp"};
	
	for(int i=0;i<1;i++){
		if(!texLoad(initialTextures[i], 0)){
			return false;
		}
	}
	*/

	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	
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
	
	mParticleTex = texLoad(mParticleTexName, 0);
	
	if(!mParticleTex){
		LOG("Couldn't load particle texture '%s'\n", mParticleTexName.c_str());
		return false;
	}
	
	texLoad("banner.png", 0);
	
#ifdef ENABLE_CGL_COMPAT
	//under CGL we want a mouse cursor
	texLoad("mouse.png", 0); 
#endif
		
	LOG("Finished loading initial textures!\n");
	
	return true;

}

/*********************************************
		Cleans up any images we've loaded
**********************************************/
void App::texShutdown(){

	for(int i=0;i<(int)mTextures.size();i++){	
#if 0
		ILuint id = mTextures[i]->iDevilID;		
		
		ilDeleteImages(1, &id);
#endif		
		glDeleteTextures(1, &mTextures[i]->iGLID);
						
		LOG("Freed '%s'\n", mTextures[i]->mFilename.c_str());
			
		delete mTextures[i];
	}
	
	//ilShutDown();
}

/*********************************************
		Delete a specific texture
**********************************************/
void App::texDelete(Texture *tex){
	//ILuint id =tex->iDevilID;			
	//ilDeleteImages(1, &id);	
	glDeleteTextures(1, &tex->iGLID);
	
	for(int i=0;i<mTextures.size();i++){
		if(mTextures[i] == tex){
			if (tex->mData && tex->mFree)
				free(tex->mData);
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


