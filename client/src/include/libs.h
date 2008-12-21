/*******************************************************************************
							BSOD2 Client - libs.h
							
 This file includes all the common standard libraries that the client uses. It's
 nice to have them all in one place
 
 You'll need: SDL, DevIL, SDL_ttf, libconfuse, SDL_net
*******************************************************************************/


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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <math.h>
#include <dlfcn.h>


/*********************************************
					C stdlib
**********************************************/

//libconfuse
#include <confuse.h>


/*********************************************
					OpenGL
**********************************************/
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>


/*********************************************
				SDL*
**********************************************/
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_net.h>
#include <SDL/SDL_thread.h>
#include <SDL/SDL_mutex.h>


/*********************************************
				GUI
**********************************************/
namespace GUI{
	#include "../../lib/openglui/GUI/GUIUtils.h"
}



