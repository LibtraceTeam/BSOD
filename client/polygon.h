#ifndef __POLYGON_H__
#define __POLYGON_H__

struct CTexture;
class CBox;
class CCollider;

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
	CCollider			*collider;

	CMesh() { tex = NULL; collider = NULL; }
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

