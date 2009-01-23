#include "main.h"

Shader::Shader(){
	mVert = mFrag = 0;
}

bool Shader::addFragment(string text){

	mFragText = text;
	mFrag = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);	
	return true;	
}

bool Shader::addVertex(string text){

	mVertText = text;
	mVert = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	
	return true;	
}

bool Shader::compile(){

	const char * ff = mFragText.c_str();
	const char * vv = mVertText.c_str();
	
	//Log::info("Fragment: %s\n", ff);
	//Log::info("Vertex: %s\n", vv);
		
	char infobuffer[16385];
	int infobufferlen = 0;

	if(mVertText != ""){
		glShaderSourceARB(mVert, 1, &vv,NULL);
		glCompileShaderARB(mVert);
	}
	
	if(mFragText != ""){
		glShaderSourceARB(mFrag, 1, &ff,NULL);
		glCompileShaderARB(mFrag);
	}	

	mProgram = glCreateProgramObjectARB();
	
	if(mFragText != "") glAttachObjectARB(mProgram, mFrag);
	if(mVertText != "") glAttachObjectARB(mProgram, mVert);
	
	glGetInfoLogARB(mVert, 16384, &infobufferlen, infobuffer);
	infobuffer[infobufferlen] = 0;	
	if(infobufferlen){
		ERR("Couldn't compile vertex shader: \n%s\n", infobuffer);
		return false;
	}
	
	glGetInfoLogARB(mFrag, 16384, &infobufferlen, infobuffer);
	infobuffer[infobufferlen] = 0;	
	if(infobufferlen){
		ERR("Couldn't compile fragment shader: \n%s\n", infobuffer);
		return false;
	}
	
	glLinkProgramARB(mProgram);
	
	glGetInfoLogARB(mProgram, 16384, &infobufferlen, infobuffer);
	infobuffer[infobufferlen] = 0;	
	if(infobufferlen){
		ERR("Couldn't link shader: \n%s\n", infobuffer);
		return false;
	}
	
	//Log::debug("Compiled shader!\n");
	
	return true;		
}

void Shader::bind(){
	glUseProgramObjectARB(mProgram);
	
	//Log::debug("Bound shader\n");
}

void Shader::unbind(){
	glUseProgramObjectARB(0);
	
	//Log::debug("unbound shader\n");
}

void Shader::bindResource(const char *name, int id){
	int location = glGetUniformLocationARB(mProgram, name);
	
	if(location == -1){
		LOG("Error binding resource %s to id %d\n", name, id);
		return;
	}
	
	glUniform1iARB(location, id);
	
}

void Shader::bindResource(const char *name, float *data, int count){
	int location = glGetUniformLocationARB(mProgram, name);
	
	if(location == -1){
		LOG("Error binding resource %s (size %d)\n", name, count);
		return;
	}
	
	if(count == 3){
		//Log::debug("location=%d, %f %f %f\n", location, data[0], data[1], data[2]);
	}
		
	//hacky!
	//Is this really the best the GL API has to offer :|?
	if(count == 1)		glUniform1f(location, data[0]);
	else if(count == 2) glUniform2f(location, data[0], data[1]);
	else if(count == 3) glUniform3f(location, data[0], data[1], data[2]);
	else if(count == 4) glUniform4f(location, data[0], data[1], data[2], data[3]);
	else{
		LOG("Bad count %d in bindResource(%s)\n", count, name);
	}		
}

void Shader::dispose(){
	if(mFrag) glDeleteShader(mFrag);
	if(mVert) glDeleteShader(mVert);
	glDeleteProgram(mProgram); 
	
	LOG("Dispose\n");
}
