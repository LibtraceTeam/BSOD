/* $CVSID$ */ 
#ifndef __SOUND_H__
#define __SOUND_H__

#include "entity.h" // htrmmmmm.... dont this this

class CSoundListener;
class CSoundSource;

class ISoundProvider
{
protected:
	//ISoundProvider();
	
public:

	virtual void Initialise() = 0;
	virtual CSoundSource *CreateSourceFromWav(const string &filename) = 0;
	virtual CSoundListener *CreateSoundListener() = 0;
	
	static ISoundProvider *Create();
};

class CSoundListener : public CEntity
{
public:
	virtual void SetPosition(const Vector3f &pos) = 0;
	virtual void SetVelocity(const Vector3f &vel) = 0;
	virtual void SetBearing(const Vector3f &b) = 0;
};

class CSoundSource : public CEntity
{
public:
	virtual void SetPosition(const Vector3f &pos) = 0;
	virtual void SetVelocity(const Vector3f &vel) = 0;
	virtual void SetBearing(const Vector3f &b) = 0;

	virtual void SetLooping(bool looping) = 0;
	virtual void SetPitch(float pitch) = 0;
	virtual void SetGain(float gain) = 0;
};


void Delete_ISoundProvider(ISoundProvider *toDelete);



#endif

