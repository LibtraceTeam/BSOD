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


/*******************************************************************************
							BSOD2 Client - ps_shaders.h
							
				This is a vertex-shader based particle system. 
*******************************************************************************/

/*********************************************
 A particle system that uses vertex+pixel
 shaders to move most of the work onto the GPU
**********************************************/
class PSShaders : public PSSprites{
protected:			
	Shader mShader;
	float fUpdateTimer;
	float fRenderTimer;
	//uint32_t mDisplayList;	
		
	void renderAll();
	
	void updateCollection(ParticleCollection *collection);
	
public:
	bool init();	
			
	//Common particle system operations	
	void update();		
	void shutdown();
	void render();		

	int getType(){return PARTICLE_SYSTEM_SHADERS;}
};

/*********************************************
 Another shader based particle system. This
 uses VTF (Vertex Texture Fetch) to lower the
 upload bottleneck. 
**********************************************/
class PSVTF : public PSSprites{
protected:			
	Shader mShader;
	float fUpdateTimer;
	float fRenderTimer;
	//uint32_t mDisplayList;	
		
	void renderAll();
	
	void updateCollection(ParticleCollection *collection);
	
public:
	bool init();	
			
	//Common particle system operations	
	void update();		
	void shutdown();
	void render();		

	int getType(){return PARTICLE_SYSTEM_VTF;}
};


/*********************************************
 A particle system that does rotation to make
 fish look nice. No, really. Blame phrogs.
**********************************************/
class PSDirectional : public PSShaders{
public:
	bool init();	
		
	int getType(){return PARTICLE_SYSTEM_DIRECTIONAL;}
};

