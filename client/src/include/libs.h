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
							BSOD2 Client - libs.h
							
 This file includes all the common standard libraries that the client uses. It's
 nice to have them all in one place
 
 You'll need: SDL, DevIL, SDL_ttf, libconfuse, SDL_net, CEGUI, libGLEW
*******************************************************************************/


/*********************************************
					CEGUI
**********************************************/
#ifdef ENABLE_GUI
	#ifdef __APPLE__
		#include <CoreFoundation/CoreFoundation.h>
		#include <CEGUIBase/CEGUI.h>
	#else
		#ifdef _WINDOWS
			#include <CEGUI.h>
		#else
			#include <CEGUI/CEGUI.h>
		#endif
	#endif
#endif

/*********************************************
					STL
**********************************************/
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <sstream>
#include <map>
#include <iterator>
#include <stack>
#include <cassert>
#include <limits>
#include <algorithm>
#include <fstream>
#include <cctype>


using std::string;
using std::vector;
using std::list;
using std::stack;
using std::map;

/*********************************************
					C stdlib
**********************************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef _WINDOWS
#include <Winsock2.h>
#include <time.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

//libconfuse
#include <confuse.h>


/*********************************************
					OpenGL
**********************************************/
#ifdef ENABLE_CGL_COMPAT
	#define GL_GLEXT_PROTOTYPES
#else
	#include <GL/glew.h>
#endif

#ifdef __APPLE__
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
	#include <OpenGL/glext.h>
#else

	#include <GL/gl.h>
	#include <GL/glu.h>
#endif

/*********************************************
				SDL*
**********************************************/
#ifdef _WINDOWS
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_net.h>
#else
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_net.h>
#include <SDL/SDL_thread.h>
#include <SDL/SDL_mutex.h>
#endif


/* PowerPCs with Altivec seem to replace rather important keywords such as
 * "bool" and "vector" :( */
#ifdef __APPLE__
#ifdef bool
#undef bool
#endif
#ifdef vector
#undef vector
#endif
#endif

