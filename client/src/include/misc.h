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
							BSOD2 Client - misc.h
							
 Basically anything weird that didn't fit got lumped into here :)
*******************************************************************************/


/*********************************************
	 	Other #defines and stuff
**********************************************/
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#define ABS(a)	   (((a) < 0) ? -(a) : (a))

#ifndef PI
	#define PI 3.14159265
#endif

const float infinity = std::numeric_limits<float>::infinity();




/*********************************************
			 	Typedefs
**********************************************/
typedef unsigned char byte;






/*********************************************
	 	Simple Color object
**********************************************/
class Color{
public:
	float r, g, b;
	
	Color();
	Color(float _r, float _g, float _b){
		r = _r; g=_g; b=_b;
		
		//hack!
		if(r > 1.0f || g > 1.0f || b > 1.0f){
			r /= 255.0f;
			g /= 255.0f;
			b /= 255.0f;
		}
	}
	
	void copy(Color *c);
	
	float sum(){
		return (r * 1) + (g * 10) + (b * 100);
	}	
	
	//The same formatting that CEGUI uses
	string toString(){
		byte rb = (byte)(r * 255);
		byte gb = (byte)(g * 255);
		byte bb = (byte)(b * 255);
		
		char buf[32];
		sprintf(buf, "FF%s%X%s%X%s%X", 	rb < 0xF ? "0" : "", rb, 
										gb < 0xF ? "0" : "", gb, 
										bb < 0xF ? "0" : "", bb);
		
		return string(buf);
	}
	
	void bind(){
		glColor3f(r,g,b);
	}
	
	
};

struct ColorSort{
    public: bool operator() (Color *a, Color *b){
		return a->sum() > b->sum();
    }
};

#define glError() { \
        GLenum err = glGetError(); \
        while (err != GL_NO_ERROR) { \
                fprintf(stderr, "glError: %s caught at %s:%u\n", (char *)gluErrorString(err), __FILE__, __LINE__); \
                err = glGetError(); \
        } \
}
