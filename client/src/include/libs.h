//DEPENDS: SDL, DevIL, SDL_ttf, libconfuse, SDL_net

/*********************************************
		 	stdlib
**********************************************/
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <math.h>
#include <sstream>
#include <map>
#include <dlfcn.h>
#include <iterator>
#include <stack>
#include <cassert>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//libconfuse
#include <confuse.h>

using std::string;
using std::vector;
using std::list;
using std::stack;
using std::map;

typedef unsigned char byte;

//GL-type includes
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <GL/glext.h> //for point_sprites

//SDL
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_net.h>
#include <SDL/SDL_thread.h>
#include <SDL/SDL_mutex.h>

#ifndef CLUSTERGL_COMPAT
	//CG headers
	#include <Cg/cg.h>		
	#include <Cg/cgGL.h>	
#endif


// GL_ARB_point_parameters
extern PFNGLPOINTPARAMETERFARBPROC  glPointParameterfARB;
extern PFNGLPOINTPARAMETERFVARBPROC glPointParameterfvARB;


