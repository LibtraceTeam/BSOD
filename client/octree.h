/* $Header$ $Log$
/* $Header$ Revision 1.3  2004/02/17 01:59:56  stj2
/* $Header$ Cvs tags will be correct at one point. Surely.
/* $Header$ */ 
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

