#ifndef _MD2_H_
#define _MD2_H_

class CMD2 : public CTriangleMesh
{
private:
	typedef struct 
	{ 
		int magic; 
		int version; 
		int skinWidth; 
		int skinHeight; 
		int frameSize; 
		int numSkins; 
		int numVertices; 
		int numTexCoords; 
		int numTriangles; 
		int numGlCommands; 
		int numFrames; 
		int offsetSkins; 
		int offsetTexCoords; 
		int offsetTriangles; 
		int offsetFrames; 
		int offsetGlCommands; 
		int offsetEnd; 
	} md2_header;

	typedef struct
	{
		byte vertex[3];
		byte lightNormalIndex;
	} md2_tri_vertex;

	typedef struct
	{
		float scale[3];
		float translate[3];
		char name[16];
		md2_tri_vertex vertices[2048];
	} md2_frame;

	typedef struct
	{
		short vertexIndices[3];
		short textureIndices[3];
	} md2_triangle;

	typedef struct
	{
		short s, t;
	} md2_tex_coord;

	typedef struct
	{
		int startFrame;
		int endFrame;
		string name;
	} md2_action;

	vector<int> vertexIndices;
	vector<int> textureIndices;
	vector<float> v;
	vector<float> t;
	vector<md2_action> actions;
	md2_header header;
	/* Derived members:
	vector<Vector3f>	vertices;
	vector<Vector2f>	texCoords;
	CTexture 			*tex;
	*/

	float curFrameTime;
	int curFrame;
	int nextFrame;
	int curAction;
	
	void ReadSkins(FILE *md2, int offset, int number);
	void ReadTexCoords(FILE *md2, int offset, int number);
	void ReadTriangles(FILE *md2, int offset, int number);
	void ReadFrames(FILE *md2, int offset, int number, int num_verts);

	// NOTE NOTE NOTE
	// This implementation of md2 takes a lot of memory.  When checked with the Blood Angel MD2 model
	// it took up 640.75kB of memory.  This does not include any memory that the texture might take
	// up in RAM.

public:
	virtual void Draw();
	virtual bool FitInBox(CBox &bbox);
	virtual void Update(float diff);
	virtual int GetNumberTriangles() { return header.numTriangles; }
	virtual uint32 *GetTriangleIndices() { return (uint32 *)&vertexIndices[0]; }
	void LoadMD2(string fileName);
	CMD2();
};


#endif

