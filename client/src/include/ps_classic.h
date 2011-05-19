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
							BSOD2 Client - ps_classic.h
							
 This contains the 'classic' type of particle systems - a very simple one that
 renders each particle as triangles, and a slightly less simple one that uses
 pointsprites. Because these two are very similar, they share everything but
 their init() and render() methods.
*******************************************************************************/

#include "libs.h"

/*********************************************
		Represents one particle
**********************************************/
class Particle{
public:
	float x,y,z; //position in 3-space relative to the origin of the parent
	float life; //how long left till it dies
	float r,g,b,a; //color
	float vx, vy, vz; //speed to move per second
	float size;	
	float timestamp; //Used by shaders - the current time at creation 
	
	unsigned int index; //index into the parent particle array
	
	bool active; //if it's alive or not
	bool rendered; //if it's been rendered once
};


/*********************************************
	Represents a group of particles that 
	share common colours and size
**********************************************/
class ParticleCollection{
public:
	vector<Particle *> mParticles;
	float fSize;
	Color mColor;
	bool bShown;
	uint32_t mList; 
	
	//For sorting
	float sum(){
		return mColor.sum() + (fSize * 100000);
	}	
	void del(int i);
};


/*********************************************
		A triangle-based particle system. 
**********************************************/
class PSClassic : public IParticleSystem{
protected:
	int iNumActive;
	int iLastColorChanges;
	float fTime;
	
	Texture *mTexture;
	
	//particle lists
	Particle mParticles[MAX_PARTICLES];	
	stack<Particle *> mFree; 
	
	//The collection of renderable particles. 
	map<float, ParticleCollection *> mParticleCollections;	
	ParticleCollection *getCollection(Color col, float size);
	
	//Display list
	GLuint mDisplayList;
	bool bNeedRecompile;	
	
	//Other util
	void del(ParticleCollection *col, int i);
	virtual float setSizeScale();
public:

	//overridden by PSSprites and PSShaders
	virtual bool init();
	virtual void render();	
	virtual void update();	
	virtual int getType(){return PARTICLE_SYSTEM_CLASSIC;}
	virtual void add(Vector3 pos, Vector3 speed, Color col, 
						float size, float life);
						
	//Other common particle system operations
	void showColor(Color c, bool bShow);	
	int getActive(){return iNumActive;}		
	void shutdown();		
	void delAll();	
	virtual void doPeriodicCleanup();

	virtual ~PSClassic(){}
};


/*********************************************
		A point-sprite particle system
**********************************************/
class PSSprites : public PSClassic{
protected:
	virtual float setSizeScale();
	float fMaxSize;
public:
	bool init();
	void render();
	virtual int getType(){return PARTICLE_SYSTEM_POINTSPRITES;}

	virtual ~PSSprites(){}
};

