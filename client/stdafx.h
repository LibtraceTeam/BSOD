/*
Copyright (c) Sam Jansen and Jesse Baker.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

3. The end-user documentation included with the redistribution, if
any, must include the following acknowledgment: "This product includes
software developed by Sam Jansen and Jesse Baker 
(see http://www.wand.net.nz/~stj2/bung)."
Alternately, this acknowledgment may appear in the software itself, if
and wherever such third-party acknowledgments normally appear.

4. The hosted project names must not be used to endorse or promote
products derived from this software without prior written
permission. For written permission, please contact sam@wand.net.nz or
jtb5@cs.waikato.ac.nz.

5. Products derived from this software may not use the "Bung" name
nor may "Bung" appear in their names without prior written
permission of Sam Jansen or Jesse Baker.

THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL COLLABNET OR ITS CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
// $Id$
#ifndef __STDAFX_H__
#define __STDAFX_H__

#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

#include <vector>
#include <list>
#include <string>
#include <map>

#ifdef _WIN32
#pragma warning(disable:4786)
#endif

#if defined(DEBUG) || defined(_DEBUG)
#define _D(x)	x
#else
#define _D(x)
#endif

#ifndef ASSERT
#if defined(DEBUG) || defined(_DEBUG)
#define ASSERT(x) assert(x)
#else
#define ASSERT(x)
#endif
#endif

// Define this symbol to produce a demo ready to run on any computer.
// At the time of writing, this just changes the image it reads on startup.
#define DEMO 

// Include this, compile and run.  Any asserts that fail are bad memory manegement by
// BuNg.
// Once it runs OK, quit and wait for some time for the memory manager to create its
// debug log.  Once it has it should show all the memory leaks and such.
// Also, make sure USE_MEM_MANAGER is defined.  If it is defined and the header file is
// not included there will still be a performance hit, but it isn't too bad (and often
// still worth it as bad pointers will have values like 0xfeedface).
//#define USE_MEM_MANAGER
//#include "external/memory/mmgr.h"

#define PI 3.141592654f
#define EPSILON 1e-12
#define DEG_TO_RAD PI/180.0f
#define RAD_TO_DEG 180.0f/PI

#ifdef WIN32
typedef unsigned __int16	uint16;
typedef unsigned __int32	uint32;
typedef __int16				int16;
typedef __int32				int32;
typedef unsigned char		byte;
#else // Linux has no __int32
typedef unsigned short		uint16;
typedef unsigned int		uint32;
typedef short				int16;
typedef int					int32;
typedef unsigned char		byte;
#endif

using namespace std;

#include "vector.h"
#include "triangle.h"
#include "polygon.h"



#endif

