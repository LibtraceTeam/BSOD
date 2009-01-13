#include "main.h"

#define PSINIT(a, b, c) mParticleSystem = new a; \
						if(mParticleSystem->init()){ \
								iParticleMethod = b; \
								LOG("Selected %s (%d)\n", c, b);\
								return true; \
						} else{ \
							delete mParticleSystem;\
						}
						

/*********************************************
		Start up the particle system
**********************************************/
bool App::initParticleSystem(){

	//LOG("ParticleSystemType %d\n", iParticleMethod);
	
	//0 == autodetect
	if(iParticleMethod == PARTICLE_SYSTEM_UNSPECIFIED){
		LOG("Attempting to autodetect the best particle method...\n");

		//PSINIT(PSShaders, PARTICLE_SYSTEM_SHADERS, "Shaders");
		PSINIT(PSSprites, PARTICLE_SYSTEM_POINTSPRITES, "PointSprites");
		PSINIT(PSClassic, PARTICLE_SYSTEM_CLASSIC, "Classic");
		
		//We shouldn't get here...
		if(iParticleMethod == PARTICLE_SYSTEM_UNSPECIFIED){
			LOG("Couldn't find any rendering method that works. Bailing out\n");
			return false;
		}
	}
	
	
	//Explicit override 
	else if(iParticleMethod == PARTICLE_SYSTEM_CLASSIC)			mParticleSystem = new PSClassic;
	else if(iParticleMethod == PARTICLE_SYSTEM_POINTSPRITES)	mParticleSystem = new PSSprites;
	else if(iParticleMethod == PARTICLE_SYSTEM_SHADERS)			mParticleSystem = new PSShaders;
	else if(iParticleMethod == PARTICLE_SYSTEM_TEXTURE)			mParticleSystem = new PSTexture;
	else{
		LOG("Bad particle method %d (must be 0-3)\n", iParticleMethod);
		return false;
	}
		
	if(!mParticleSystem->init()){
		LOG("Couldn't start particle system, try another method!\n");
		return false;
	}
	
	return true;
}

