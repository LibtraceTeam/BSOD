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

