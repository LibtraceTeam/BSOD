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
#ifndef _octree_H_
#define _octree_H_

// Container of worlds - objects...

class CCamera;
class CBezier;

class COctree {
public:
	int		m_depth;	// Maximum depth of tree
	Vector3f m_origin;	// Position of tree centre
	float	m_width;	// octree size (length or height) 
	bool	m_outline;	// Draw wireframe of tree
	


public:
	static int nodes_drawn;

	class c_node {
	public:
		// personal information
		list<CMesh *> m_objs;
		// size of node can be calculated from depth and positio in tree;

		// children
		c_node *m_children[2][2][2];
		//                 x  y  z
		c_node();
		~c_node();
	} m_TopNode;

	COctree();
	~COctree();	

	void AddMesh(CMesh *ent);
	
	bool FitInNode(Vector3f offset, float width, CMesh *ins);
	bool COctree::FitInNode(Vector3f centre, float width, CBezier *ins);

	void Draw(CCamera &cam);
	void DrawBox(Vector3f offset, float width);
	void DrawNode(c_node *temp, const Vector3f &offset, float width, CCamera &cam, int recursion);
	void InitBBox(float width, Vector3f centre);

	bool drawOctreeBoxes;

private:
	void AddMesh(c_node *node, Vector3f offset, float width, CMesh *ins, int recursion);
};

#endif

