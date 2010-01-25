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

