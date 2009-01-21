/*******************************************************************************
							BSOD2 Client - ps_texture.h
*******************************************************************************/

class PSTexture : public PSSprites{

	//For the frame buffer object rendering
	GLuint fbo;
	GLuint depthbuffer;
	GLuint img;
	
	int height, width;
	
	void renderAll();
	float setSizeScale();
public:
	bool init();
	void render();
	void render2D();
	void shutdown();
	int getType(){return PARTICLE_SYSTEM_TEXTURE;}
};
