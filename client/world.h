/* $Header$ $Log$
/* $Header$ Revision 1.3  2004/02/17 01:59:56  stj2
/* $Header$ Cvs tags will be correct at one point. Surely.
/* $Header$ */ 
#ifndef __WORLD_H__
#define __WORLD_H__

//class CCamera;
class COctree;
class CDisplayManager;
class CEntityManager;
class CSystemDriver;
class CVFS;
class CActor;
class CActionHandler;
class CPhysicsHandler;
class CNetDriver;
class CConfig;
class CResourceManager;
class CPartVis;

class ISoundProvider;

class CWorld
{
public:
	CWorld();
	~CWorld();

	CSystemDriver	*sys;
	COctree			*tree;
	CDisplayManager	*display;
	CEntityManager	*entities;
	CActionHandler	*actionHandler;
	CPhysicsHandler	*physicsHandler;
	CNetDriver		*netDriver;
	CConfig			*config;
	CResourceManager *resources;
	CPartVis	*partVis;
	ISoundProvider	*soundProvider;
	
	void Draw();
	void Update(float diff);
	void Cleanup();
};

extern CWorld world;

#endif

