/*******************************************************************************
							BSOD2 Client - shader.h						

 	Wrapper for a GLSL shader object. 
*******************************************************************************/
class Shader{
	unsigned int mFrag;
	unsigned int mVert;
	unsigned int mProgram;
	
	string mFragText, mVertText;
	
public:
	Shader();

	bool addFragment(string text);
	bool addVertex(string text);
	bool compile();
	void dispose();
		
	void bind();
	void unbind();
	
	void bindResource(const char *name, int id);
	void bindResource(const char *name, float *data, int count);
	
	unsigned int getProgram(){return mProgram;}
};

