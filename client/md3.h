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
#ifndef _MD3_H_
#define _MD3_H_

/*
struct tFace
{
	int vertIndex[3];			// indicies for the verts that make up this triangle
	int coordIndex[3];			// indicies for the tex coords to texture this face
};*/

struct tMd3Tag {
	char		strName[64];
	Vector3f	vPosition;
	float		rotation[3][3];
};

class t3DObject : public CMesh
{
public:
	//virtual void Draw();

	string strName;						// The name of the object
	//CLoadMD3::tFace *pFaces;				// The faces information of the object
	int numTriangles;
	int numVerts;
	vector<uint32> vertex_indices;
	vector<uint32> tex_indices;

    t3DObject() : CMesh() {  }
	
	virtual int GetNumberTriangles() { return (int)numTriangles; }
	virtual void BuildCollisionInfo();
	virtual void Draw() {}
	virtual uint32 *GetTriangleIndices() { return &vertex_indices[0]; }
};

struct tAnimationInfo
{
    char strName[255];          // This stores the name of the animation (I.E. "TORSO_STAND")
    int startFrame;             // This stores the first frame number for this animation
    int endFrame;               // This stores the last frame number for this animation
    int loopingFrames;          // This stores the looping frames for this animation (not used)
    int framesPerSecond;        // This stores the frames per second that this animation runs

	tAnimationInfo() {
		startFrame = 0;
		endFrame = 0;
		loopingFrames = 0;
		framesPerSecond = 1;
	}
};


class t3DModel 
{
public:
	int numOfObjects;					// The number of objects in the model
	int numOfMaterials;					// The number of materials for the model
	//vector<tMaterialInfo> pMaterials;	// The list of material information (Textures and colors)
	vector< t3DObject > pObject;			// The object list for our model
	
	int numOfTags;						// This stores the number of tags in the model
	t3DModel	**pLinks;				// This stores a list of pointers that are linked to this model
	struct tMd3Tag	*pTags;			// This stores all the tags for the model animations
	
	t3DModel() { memset (this, 0, sizeof(*this)); }
	~t3DModel() {
		pObject.clear();
		if(pTags) delete pTags;
		if(pLinks) delete [] pLinks;
	}

	void BuildCollisionInfo() {
		for(int i = 0; i < (int)pObject.size(); i++) {
			pObject[i].BuildCollisionInfo();
		}
	}

	int numOfAnimations;                // The number of animations in this model
    int currentAnim;                    // The current index into pAnimations list
    int currentFrame;                   // The current frame of the current animation
    int nextFrame;                      // The next frame of animation to interpolate too
    float t;                            // The ratio of 0.0f to 1.0f between each key frame
    float lastTime;                     // This stores the last time that was stored
    vector<tAnimationInfo> pAnimations; // The list of animations
	
};








// **************** CLoadMD3 ****************

class CLoadMD3 
{
public:
	CLoadMD3();
	bool ImportMD3(t3DModel *pModel, string fileName);
	bool LoadSkin(t3DModel *pModel, string strSkin, string directory);

private:
	struct tMd3MeshInfo {
		char	meshID[4];
		char	strName[68];
		int		numMeshFrames;
		int		numSkins;
		int     numVertices;
		int		numTriangles;
		int		triStart;
		int		headerSize;
		int     uvStart;
		int		vertexStart;
		int		meshSize;
	};
	struct tMd3Header { 
		char	fileID[4];
		int		version;
		char	strFile[68];
		int		numFrames;
		int		numTags;
		int		numMeshes;
		int		numMaxSkins;
		int		headerSize;
		int		tagStart;
		int		tagEnd;
		int		fileSize;
	};
	struct tMd3Bone {
		float	mins[3];
		float	maxs[3];
		float	position[3];
		float	scale;
		char	creator[16];
	};
	struct tMd3Triangle {
		signed short vertex[3];
		unsigned char normal[2];
	};
	struct tMd3Face { int vertexIndices[3]; };
	struct tMd3TexCoord { float textureCoord[2]; };
	struct tMd3Skin { char strName[68]; };
private:
	bool IsInString(string strString, string strSubString);
	void ReadMD3Data(t3DModel *pModel);
	void ConvertDataStructures(t3DModel *pModel, tMd3MeshInfo meshHeader);
	void CleanUp();
	
	FILE *m_FilePointer;

	tMd3Header				m_Header;			// The header data

	tMd3Skin				*m_pSkins;			// The skin name data (not used)
	tMd3TexCoord			*m_pTexCoords;		// The texture coordinates
	tMd3Face				*m_pTriangles;		// Face/Triangle data
	tMd3Triangle			*m_pVertices;		// Vertex/UV indices
	tMd3Bone				*m_pBones;			// This stores the bone data (not used)
};

// ******************************************



// ******************* CMD3 ******************

class CMD3 
{
private:

public:
	CMD3();
	virtual ~CMD3();

	bool LoadMD3(string modelName, string color = "default"); // "red" or "blue" also available
	bool LoadWeapon(string weaponName);
	bool LoadAnimations(const char *fileName);

	void LinkModel(t3DModel *pModel, t3DModel *pLink, string strTagName);
	
	void Draw();
	bool FitInBox(CBox &bbox);
	virtual void Update(float diff);
	
	void DrawLink(t3DModel *pModel);
	void RenderModel(t3DModel *pModel);

	void UpdateModel(t3DModel *pModel);

	float model_scale;

	t3DModel upper;
	t3DModel lower;
	t3DModel head;

	void BuildCollisionInfo() {
		upper.BuildCollisionInfo();
		lower.BuildCollisionInfo();
		head.BuildCollisionInfo();
	}


	CCollider			*collider;

};

class CMD3Factory
{
private:
	CMD3 base;
	
public:
	CMD3Factory();
	
	void CreateFactory(const string &fileName);

	void CreateMD3(CMD3 &md3);
};

class CResourceManager
{
private:
	map<const char *, CMD3Factory *> md3s;
	
public:
	void LoadMD3(const char *name, CMD3 &md3);
	
};

// ******************************************

#endif

