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

