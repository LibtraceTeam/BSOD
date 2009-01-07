/*******************************************************************************
							BSOD2 Client - ps_shaders.h
							
 This is a vertex-shader based particle system. It uses VBOs to store the data
*******************************************************************************/


/*********************************************
 A particle system that uses vertex+pixel
 shaders to move most of the work onto the GPU
**********************************************/
class PSShaders : public IParticleSystem{
	int iNumActive;
	float fTime;	
	uint32_t iVBO;	
	Shader mShader;
	
public:
	bool init();	
		
	//Common particle system operations
	void add(Vector3 pos, Vector3 speed, Color col, float size, float life);
	int getType(){return PARTICLE_SYSTEM_SHADERS;}	
	void showColor(Color c, bool bShow);	
	int getActive(){return iNumActive;}		
	void update();		
	void shutdown();
	void render();		
	void delAll(){}	
};

