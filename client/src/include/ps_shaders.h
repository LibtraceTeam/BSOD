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
	float fTime;
	float fUpdateTimer;
	float fRenderTimer;
	
	void renderAll();
	
	uint32_t mDisplayList;
	
public:
	bool init();	
	
	void add(Vector3 pos, Vector3 speed, Color col, float size, float life);
		
	//Common particle system operations	
	void update();		
	void shutdown();
	void render();		
};

