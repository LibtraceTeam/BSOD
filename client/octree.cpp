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

#include "octree.h"
#include "world.h"
#include "display_manager.h"
#include "camera.h"
#include "bezier.h"

#include <time.h>
#include <stdio.h>

#include "collider.h"

int COctree::nodes_drawn = 0;

//octree C_NODE class controller
COctree::c_node::c_node()
{
	int x,y,z;

	for(x = 0; x < 2; x++)
		for(y = 0; y < 2; y++)
			for(z = 0; z < 2; z++)
				this->m_children[x][y][z] = NULL;
}

COctree::c_node::~c_node()
{
	int x,y,z;

	for(x = 0; x < 2; x++)
		for(y = 0; y < 2; y++)
			for(z = 0; z < 2; z++)
					delete m_children[x][y][z];

	list<CMesh *>::iterator i = m_objs.begin();
	for(; i != m_objs.end(); ++i)
	{
		delete (*i);
	}
}

//octree superclass constructor
COctree::COctree() 
{
	// TODO these constants can fuck off.
	// set depth to 8
	m_depth = 8;
	m_width = 50.0f;

	m_origin.x = 0.0f;
	m_origin.y = 0.0f;
	m_origin.z = 0.0f;

	drawOctreeBoxes = false;	
}

void COctree::AddMesh(CMesh *ent)
{
	// If no collision info is present, we build it now
	if(ent->collider == NULL) {
		//ent->collider = CCollider::Create();
		//ent->collider->BuildCollisionInfo(ent);
	}
	AddMesh(&m_TopNode, m_origin, m_width, ent, 0);
}

COctree::~COctree()
{
	// TODO kill all nodes and objects conatined within world -- done automatically already?
}

bool COctree::FitInNode(Vector3f centre, float width, CMesh *ins)
{
	bool fit = true;
	CBox bbox( Vector3f(centre.x - width, centre.y - width, centre.z - width),
			   Vector3f(centre.x + width, centre.y + width, centre.z + width) );

	vector<Vector3f>::iterator i = ins->vertices.begin();
	for(; i != ins->vertices.end(); ++i) {
		if(!ins->FitInBox(bbox)) {
			fit = false;
			break;
		}
	}

	return fit;
}

// Inserts the object into the COctree
void COctree::AddMesh(c_node *node, Vector3f offset, float width, CMesh *ins, int recursion)
{
	if(recursion > 20)
		return;

	// check children
	int x,y,z;
	width /= 2;
	for(x=0;x<2;x++) {
		for(y=0;y<2;y++) {
			for(z=0;z<2;z++) {
				if(FitInNode(Vector3f((x==0)?offset.x - width:offset.x + width, 
									(y==0)?offset.y - width:offset.y + width,
									(z==0)?offset.z - width:offset.z + width),
							 width, ins)) {
					// create new node
					if(node->m_children[x][y][z] == NULL)
						node->m_children[x][y][z] = new c_node();
					AddMesh(node->m_children[x][y][z],
							  Vector3f((x==0)?offset.x - width:offset.x + width, 
									 (y==0)?offset.y - width:offset.y + width,
									 (z==0)?offset.z - width:offset.z + width),
							  width, ins, recursion + 1);
					return;
				}
			}
		}
	}
	node->m_objs.push_back(ins);
}

void COctree::InitBBox(float width, Vector3f centre)
{
	m_width = width;
	this->m_origin = centre;
}

// Handle the drawing of the COctree and objects within
void COctree::Draw(CCamera &cam)
{
	// draw the main box
	DrawNode(&m_TopNode, m_origin, m_width, cam, 0);
}

void COctree::DrawBox(Vector3f offset, float width)
{
	world.display->DrawBox(offset, width);
}

void COctree::DrawNode(c_node *node, const Vector3f &offset, float width, CCamera &cam, int recursion)
{
	// Frustum culling
	if(!cam.IsCubeInFrustum(offset, width)) {
		return;
	}

	if(drawOctreeBoxes)
		DrawBox(offset, width);

	// Draw node  
	list<CMesh *>::iterator i;
	for(i = node->m_objs.begin(); i != node->m_objs.end(); ++i)
	{
		(*i)->Draw();
	}
	nodes_drawn++;

//	if(recursion == 5)
//		return;

	// Draw children
	int x,y,z;
	width /= 2.0f;
	for(x = 0; x< 2; x++) 
		for(y = 0; y < 2; y++) 
			for(z = 0; z < 2; z++) 
				if(node->m_children[x][y][z] != NULL) 
					DrawNode(node->m_children[x][y][z],
						Vector3f(	(x==0)?offset.x - width : offset.x + width,
								(y==0)?offset.y - width : offset.y + width,
								(z==0)?offset.z - width : offset.z + width),
						width, cam, recursion + 1);

}

