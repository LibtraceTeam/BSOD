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
	//TODO: This isn't particularly good. Improve me!
	if(iParticleMethod == PARTICLE_SYSTEM_UNSPECIFIED){
		LOG("Attempting to autodetect the best particle method...\n");

		PSINIT(PSShaders, PARTICLE_SYSTEM_SHADERS, "Shaders");
		PSINIT(PSSprites, PARTICLE_SYSTEM_POINTSPRITES, "PointSprites");
		PSINIT(PSClassic, PARTICLE_SYSTEM_CLASSIC, "Classic");
		
		//We shouldn't get here...
		if(iParticleMethod == PARTICLE_SYSTEM_UNSPECIFIED){
			LOG("Couldn't find any rendering method that works. Bailing out\n");
			return false;
		}
	}
	
	
	//Explicit override 
	else if(iParticleMethod == PARTICLE_SYSTEM_CLASSIC)			
			mParticleSystem = new PSClassic;
	else if(iParticleMethod == PARTICLE_SYSTEM_POINTSPRITES)	
			mParticleSystem = new PSSprites;
	else if(iParticleMethod == PARTICLE_SYSTEM_SHADERS)			
			mParticleSystem = new PSShaders;
	else if(iParticleMethod == PARTICLE_SYSTEM_TEXTURE)			
			mParticleSystem = new PSTexture;
	//else if(iParticleMethod == PARTICLE_SYSTEM_VTF)			
	//		mParticleSystem = new PSVTF;
	else if(iParticleMethod == PARTICLE_SYSTEM_DIRECTIONAL)			
			mParticleSystem = new PSDirectional;
	else{
		LOG("Bad particle method %d (must be 0-5)\n", iParticleMethod);
		return false;
	}
		
	if(!mParticleSystem->init()){
		LOG("Couldn't start particle system, try another method!\n");
		return false;
	}
	
	return true;
}

