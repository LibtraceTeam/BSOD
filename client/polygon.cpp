/* $Header$ $Log$
/* $Header: /home/cvs/wand-general/bsod/client/polygon.cpp,v 1.3 2004/02/17 01:59:56 stj2 Exp $ Revision 1.4  2004/02/17 02:05:21  stj2
/* $Header: /home/cvs/wand-general/bsod/client/polygon.cpp,v 1.3 2004/02/17 01:59:56 stj2 Exp $ Still trying to get cvs tags correct.
/* $Header: /home/cvs/wand-general/bsod/client/polygon.cpp,v 1.3 2004/02/17 01:59:56 stj2 Exp $
 $Header$ Revision 1.3  2004/02/17 01:59:56  stj2
 $Header$ Cvs tags will be correct at one point. Surely.
 $Header$ */ 
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
