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
#include "stdafx.h"

#include "vector.h"
#include "polygon.h"
#include "world.h"
#include "triangle.h"
#include "display_manager.h"
#include "collider.h"
#include "reporter.h"
#include "bezier.h"

uint32 CMesh::fan_indices[2000];
uint32 CMesh::strip_indices[2000];
uint32 CMesh::mesh_indices[2000];

CMesh::~CMesh()
{ 
	if(collider) delete collider; 
}

void CMesh::Dump()
{
	/*vector<Vector3f>	vertices;
	vector<Vector2f>	texCoords;
	//vector<Vector3f>	normals;
	CTexture 			*tex;
	CCollider			*collider;*/

	unsigned int i;

	Log("Vertices:{");
	for(i = 0; i < vertices.size(); i++) {
		Log("%s,", vertices[i].toString().c_str());
		if(i % 5 == 0) Log("\n");
	}
	Log("}\n");
}

void CTriangleFan::Draw()
{
	world.display->BindTexture(tex);
	world.display->DrawTriangleFan(this);
	/*
		(float *)&vertices[0],
		(float *)&texCoords[0],
		GetNumberTriangles());*/
}

void CTriangleMesh::Draw()
{
	world.display->BindTexture(tex);
	/*
	world.display->DrawTriangles2(
		(float *)&vertices[0],
		(float *)&texCoords[0],
		GetNumberTriangles());
	*/
	world.display->DrawIndexedTriangles2(
		(float *)&vertices[0],
		(float *)&texCoords[0],
		GetTriangleIndices(), GetNumberTriangles()
		);
}

void CMesh::InitIndices() {
	uint32 fan_counter = 1;
	uint32 strip_counter = 0;
	int i;

	for(i = 0; i < 2000; i++) {
		mesh_indices[i] = i;

		if(i % 3 == 0)
			fan_indices[i] = 0;
		else if(i % 3 == 1)
			fan_indices[i] = fan_counter++;
		else
			fan_indices[i] = fan_counter;

		strip_indices[i] = strip_counter++;

		if(i % 3 == 2)
			strip_counter -= 2;
	}

	/*for(i = 0; i < 2000; i += 6) {
		uint32 temp[3];

		memcpy(temp, &strip_indices[i + 3], sizeof(uint32) * 3);
		strip_indices[i + 3] = temp[2];
		strip_indices[i + 4] = temp[1];
		strip_indices[i + 5] = temp[0];
	}*/

	CBezier::InitIndices();
}

uint32 *CTriangleMesh::GetTriangleIndices() 
{
	return mesh_indices;
}

uint32 *CTriangleFan::GetTriangleIndices() 
{
	return fan_indices;
}

bool CMesh::FitInBox(CBox &bbox)
{
	vector<Vector3f>::iterator i = vertices.begin();

	for(; i != vertices.end(); ++i) {
		if(	   (*i).x < bbox.m_min.x || (*i).x > bbox.m_max.x
			|| (*i).y < bbox.m_min.y || (*i).y > bbox.m_max.y
			|| (*i).z < bbox.m_min.z || (*i).z > bbox.m_max.z ) {

			return false;
		}
	}

	return true;
}
