/*******************************************************************************
							BSOD2 Client - ps_interface.h
							
 This file specifices the interface that particle systems implement. Yay for
 abstraction :)
*******************************************************************************/

//Returned by IParticleSystem::getType()
#define PARTICLE_SYSTEM_UNSPECIFIED 	0
#define PARTICLE_SYSTEM_CLASSIC	 		1
#define PARTICLE_SYSTEM_POINTSPRITES 	2
#define PARTICLE_SYSTEM_SHADERS 		3

/*********************************************
		Particle system interface
**********************************************/
class IParticleSystem{
public:	
	virtual ~IParticleSystem(){}
	
	//init/terminate
	virtual bool init()=0; //If false, we should try an 'easier' type
	virtual void shutdown()=0;
	
	//Called automatically each frame
	virtual void update()=0;
	virtual void render()=0;
		
	//Add a particle with these attributes
	virtual void add(Vector3 pos, Vector3 speed, Color col, 
					float size, float life)=0;
					
	//Clear everything
	virtual void delAll()=0;
	
	//Delete particles with a specific colour
	virtual void delColor(Color c)=0;
	
	//stats
	virtual int getActive(){return 0;}
	virtual int getType(){return PARTICLE_SYSTEM_UNSPECIFIED;}
};


