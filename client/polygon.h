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
#ifndef __POLYGON_H__
#define __POLYGON_H__

struct CTexture;
class CBox;

class CMesh
{
public:
	/* vertices and texCoords are both stored in vectors for the same reasons.
	 * 1. They will be stored contiguously in an array.  It is the equivalent of
	 *    float vertices[num_vertices * 3] in many ways.  This way the array is
	 *    dynamic and offers us all the benefits of C++.  Having the vertices and
	 *    tex coords in an array means they can easily be drawn with vertex
	 *    pointers and the like.
	 * 2. vectors are somewhat more efficient at storing these than lists.  An
	 *    STL list means probably an extra 8 bytes (prev/next pointers) for each
	 *    vertex or tex coord.  This was the old, wasteful way of doing it.
	 */
	vector<Vector3f>	vertices;
	vector<Vector2f>	texCoords;
	CTexture 			*tex;

	CMesh() { tex = NULL; }
	virtual ~CMesh();

	virtual void Draw() = 0;
	virtual bool FitInBox(CBox &bbox);
	virtual int GetNumberTriangles() = 0;
	virtual uint32 *GetTriangleIndices() = 0;
	virtual void Dump();


	// Index stuff for use with indexed drawing and collision detection
	static uint32 mesh_indices[2000];
	static uint32 fan_indices[2000];
	static uint32 strip_indices[2000];
	// The following MUST be called at startup somewhere. Otherwise you get
	// stupid annoying crash bugs when you try and do stuff like build
	// colliders.
	static void InitIndices();
};

class CTriangleFan : public CMesh
{
public:
	virtual void Draw();
	virtual int GetNumberTriangles() { return (int)vertices.size() - 2; }
	virtual uint32 *GetTriangleIndices();
};

class CTriangleMesh : public CMesh
{
public:
	virtual void Draw();
	virtual int GetNumberTriangles() { return (int)vertices.size() / 3; }
	virtual uint32 *GetTriangleIndices();
};

#endif

