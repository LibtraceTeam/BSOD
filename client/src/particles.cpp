#include "main.h"

#define PSINIT(a, b, c) mParticleSystem = new a; \
						if(mParticleSystem->init()){ \
								iParticleMethod = b; \
								LOG("%s (%d)\n", c, b);\
								return; \
						} else{ \
							delete mParticleSystem;\
						}
						

/*********************************************
		Start up the particle system
**********************************************/
void App::initParticleSystem(){

	LOG("ParticleSystemType %d\n", iParticleMethod);
	
	//0 == autodetect
	if(iParticleMethod == PARTICLE_SYSTEM_UNSPECIFIED){
		LOG("Attempting to autodetect the best particle method...");

#ifdef ENABLE_PS_SHADERS
		PSINIT(PSShaders, PARTICLE_SYSTEM_SHADERS, "Shaders");
#endif
		
		PSINIT(PSSprites, PARTICLE_SYSTEM_POINTSPRITES, "PointSprites");
		PSINIT(PSClassic, PARTICLE_SYSTEM_CLASSIC, "Classic");
		
		//We shouldn't get here...
		if(iParticleMethod == PARTICLE_SYSTEM_UNSPECIFIED){
			LOG("Couldn't find any rendering method that works. Bailing out\n");
			utilShutdown(0);
		}
	}
	
	
	//Explicit override 
	else if(iParticleMethod == PARTICLE_SYSTEM_CLASSIC)			mParticleSystem = new PSClassic;
	else if(iParticleMethod == PARTICLE_SYSTEM_POINTSPRITES)	mParticleSystem = new PSSprites;
#ifdef ENABLE_PS_SHADERS
	else if(iParticleMethod == PARTICLE_SYSTEM_SHADERS)			mParticleSystem = new PSShaders;
#endif
	else{
		LOG("Bad particle method %d (must be 0-3)\n", iParticleMethod);
		utilShutdown(0);
	}
		
	if(!mParticleSystem->init()){
		LOG("Couldn't start particle system, try another method!\n");
		utilShutdown(1);
	}
}

