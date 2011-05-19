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
							BSOD2 Client - texture.h

 A very simple GL texture object. Loaded and manipulated by the tex*() funcs
*******************************************************************************/

/*********************************************
		Texture object
**********************************************/
class Texture{
public:
	int iSizeX;
	int iSizeY;
	GLuint iGLID;
	int iDevilID;
		
	string mFilename;
	
	byte *mData;
	bool mFree;

	void bind(){
		glBindTexture(GL_TEXTURE_2D, iGLID);
	}

};

/*********************************************
		Texture flags
**********************************************/
#define TEXTURE_NO_GL 1

