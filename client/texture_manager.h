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
#ifndef __TEXTURE_MANAGER_H__
#define __TEXTURE_MANAGER_H__

class CVFS;

struct CTexture
{
	CTexture() : id(0), width(0), height(0), orig_width(0), orig_height(0) 
	{}

	string			name;
	int				id;
	uint32			width, height;
	uint32			orig_width, orig_height;
};

/**
 * Singleton class that manages textures.  It doesn't do too much at the moment:
 * allows loading of textures; caches them so we don't load the same texture more
 * than once, and provides an interface to unload textures (it really just removed
 * the texture from the list and calls the display manager to do the rest).
 */
class CTextureManager
{
private:
	// Fuck I'm stupid.  - Sam
	// TODO: Make this actually keep tabs on font textures :)
	// Actually, this is a somewhat more complicated procedure if we want to make
	// it generic. Ideally we want a resource cache of 'named' resources in the
	// system. So like font textures, gui textures, maybe sounds, etc.
public:
	static CTextureManager tm;
	list<CTexture *>	textures;

	CTextureManager();
	virtual ~CTextureManager();

	CTexture *LoadTexture(string texName);
	void UnloadTexture(string texName);
	void UnloadTexture(CTexture *tex);
	CTexture *FindTexture(string texName);
	void SaveScreenshot(string fileName);
};


#endif

