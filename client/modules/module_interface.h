/*********************************************
 Should be #included by BSOD rendering modules
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

using std::string;
using std::vector;
using std::list;

//GL-type includes
#include <GL/gl.h>
#include <GL/glu.h>

//For any magic we may need to do in the app headers...
#define MODULE

//Application headers
#include "main.h"

#define EXPORT extern "C"
#define LOG printf("MODULE: "); printf

extern App *mApp;

