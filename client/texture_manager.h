/* $Header$ $Log$
/* $Header$ Revision 1.3  2004/02/17 01:59:56  stj2
/* $Header$ Cvs tags will be correct at one point. Surely.
/* $Header$ */ 
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
};


#endif

