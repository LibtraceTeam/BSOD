/* $CVSID$ */ 
#ifndef __BEZIER_H__
#define __BEZIER_H__

class CBezier : public CMesh {
private:
	Vector3f EvalBezier3D(float t, Vector3f &cp1, Vector3f &cp2, Vector3f &cp3);
	Vector2f EvalBezier2D(float t, Vector2f &cp1, Vector2f &cp2, Vector2f &cp3);

	static vector<uint32> indices;

public:
	vector<Vector3f> m_cpts;
	vector<Vector2f> m_tex_cpts;
	
	int m_x; // width
	int m_y; // height
	static int detail;
	
	CBezier() { m_x = m_y = 0; }

	virtual void Tesselate();
	virtual void Draw();
	virtual bool FitInBox(CBox &bbox);
	virtual uint32 *GetTriangleIndices();
	virtual int GetNumberTriangles();

	static void InitIndices();
};

#endif

