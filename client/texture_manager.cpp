/*
Copyright (c) Sam Jansen and Jesse Baker.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

3. The end-user documentation included with the redistribution, if
any, must include the following acknowledgment: "This product includes
software developed by Sam Jansen and Jesse Baker 
(see http://www.wand.net.nz/~stj2/bung)."
Alternately, this acknowledgment may appear in the software itself, if
and wherever such third-party acknowledgments normally appear.

4. The hosted project names must not be used to endorse or promote
products derived from this software without prior written
permission. For written permission, please contact sam@wand.net.nz or
jtb5@cs.waikato.ac.nz.

5. Products derived from this software may not use the "Bung" name
nor may "Bung" appear in their names without prior written
permission of Sam Jansen or Jesse Baker.

THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL COLLABNET OR ITS CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
// $Id$
#include "stdafx.h"
#include "texture_manager.h"
#include "display_manager.h"
#include "world.h"
#include "vfs.h"
#include "exception.h"

#include <math.h>
#include "IL/il.h"
#include "IL/ilu.h"

// Singleton object instantiation
CTextureManager CTextureManager::tm;

static bool IsPowerOfTwo(int num)
{
    for(int i = 0; i < 12; i++)
        if(num == pow(2.0f, i))
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

    if(tex = FindTexture(texName))
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
            height = (int)pow(2.0f, ceil(log((float)height) / log(2.0f)));
        }
        if(!IsPowerOfTwo(width))
        {
            width = (int)pow(2.0f, ceil(log((float)width) / log(2.0f)));
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

void CTextureManager::SaveScreenshot(string fileName)
{
    vector<byte> buf;
    unsigned int w = 0, h = 0;
    ILuint ilId;

    Log("CTextureManager::SaveScreenshot\n");

    // Generate the main image name to use.
    ilGenImages(1, &ilId);
    // Bind this image name.
    ilBindImage(ilId);

    world.display->Screenshot(buf, w, h);

    if(!ilTexImage(w, h, 1, 3, IL_RGB, IL_UNSIGNED_BYTE, &buf[0])) {
        throw CException("ilTexImage error\n");
    }

    ilEnable(IL_FILE_OVERWRITE);

    // XXX: doesn't support VFS atm
    if(!ilSaveImage((char *)fileName.c_str())) {
        throw CException("ilSaveImage error\n");
    }

    ilDeleteImages(1, &ilId);
}
