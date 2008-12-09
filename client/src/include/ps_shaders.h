/*******************************************************************************
							BSOD2 Client - ps_shaders.h
							
 This is a vertex-shader based particle system. TODO: Rip this apart!
*******************************************************************************/

#ifdef ENABLE_PS_SHADERS

/*********************************************
 A particle system that uses vertex+pixel
 shaders to move most of the work onto the GPU
**********************************************/
class PSShaders : public IParticleSystem{

	int iNumActive;
#ifndef CLUSTERGL_COMPAT
	CGcontext	cgContext;		
	CGprogram	cgProgram;		
	CGprofile	cgVertexProfile;	
	CGparameter	position, color, modelViewMatrix, time;
#endif	
	float fTime;
public:

	bool init();	
		
	//Particle system operations
	void add(Vector3 pos, Vector3 speed, Color col, float size, float life);
	void update();		
	void shutdown();
	void render();		
	
	int getActive(){return iNumActive;}		
	int getType(){return PARTICLE_SYSTEM_SHADERS;}	
	
	void delAll(){}
	void delColor(Color c){}
};

#endif
