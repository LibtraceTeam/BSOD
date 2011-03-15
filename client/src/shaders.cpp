/* 
 * This file is part of BSOD client
 *
 * Copyright (c) 2011 The University of Waikato, Hamilton, New Zealand.
 *
 * Author: Paul Hunkin
 *
 * Contributors: Shane Alcock
 *
 * All rights reserved.
 *
 * This code has been developed by the University of Waikato WAND research
 * group. For further information please see http://www.wand.net.nz/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $Id$
 */

#include "main.h"

Shader::Shader(){
	mVert = mFrag = 0;
	bIsCompiled = false;
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
	
	//printf("Fragment: %s\n", ff);
	//printf("Vertex: %s\n", vv);
		
	char infobuffer[16385];
	GLsizei infobufferlen = 0;

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

	GLint compiled, linked;
	glGetObjectParameterivARB(mVert, GL_COMPILE_STATUS, &compiled);

	if (!compiled && mVertText != "") {
	
		glGetInfoLogARB(mVert, 16384, &infobufferlen, infobuffer);
		infobuffer[infobufferlen] = 0;	
		ERR("Couldn't compile vertex shader: \n%s\n", infobuffer);
		return false;
	}
	
	glGetObjectParameterivARB(mFrag, GL_COMPILE_STATUS, &compiled);
	if (!compiled && mFragText != "") {
		glGetInfoLogARB(mFrag, 16384, &infobufferlen, infobuffer);
		infobuffer[infobufferlen] = 0;	
		ERR("Couldn't compile fragment shader: \n%s\n", infobuffer);
		return false;
	}
	
	glLinkProgramARB(mProgram);
	
	//glGetProgramiv(mProgram, GL_LINK_STATUS, &linked);
	glGetObjectParameterivARB(mProgram, GL_OBJECT_LINK_STATUS_ARB, &linked);

	if (!linked) {
		glGetInfoLogARB(mProgram, 16384, &infobufferlen, infobuffer);
		infobuffer[infobufferlen] = 0;	
		ERR("Couldn't link shader: \n%s\n", infobuffer);
		return false;
	}
	
	//Log::debug("Compiled shader!\n");
	bIsCompiled = true;
	
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
	
	//LOG("Got location for %s: %d\n", name, location);

	if(location == -1){
		LOG("Error binding resource %s (size %d)\n", name, count);
		return;
	}
	
	if(count == 3){
		//Log::debug("location=%d, %f %f %f\n", location, data[0], data[1], data[2]);
	}

	//LOG("About to call glUniform%df\n", count);
		
	//hacky!
	//Is this really the best the GL API has to offer :|?
	if(count == 1)		glUniform1fARB(location, data[0]);
	else if(count == 2) glUniform2fARB(location, data[0], data[1]);
	else if(count == 3) glUniform3fARB(location, data[0], data[1], data[2]);
	else if(count == 4) glUniform4fARB(location, data[0], data[1], data[2], data[3]);
	else{
		LOG("Bad count %d in bindResource(%s)\n", count, name);
	}

	//LOG("Called\n");
}

void Shader::dispose(){
	if(mFrag) glDeleteObjectARB(mFrag);
	if(mVert) glDeleteObjectARB(mVert);
	glDeleteObjectARB(mProgram); 
	
	LOG("Dispose\n");
}
