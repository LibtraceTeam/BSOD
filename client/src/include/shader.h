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


/*******************************************************************************
							BSOD2 Client - shader.h						

 	Wrapper for a GLSL shader object. 
*******************************************************************************/
class Shader{
	unsigned int mFrag;
	unsigned int mVert;
	unsigned int mProgram;
	
	string mFragText, mVertText;
	
	bool bIsCompiled;
	
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
	
	bool isCompiled(){return bIsCompiled;}
};

