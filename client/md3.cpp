/*
 * Copyright (c) Sam Jansen and Jesse Baker.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * 3. The end-user documentation included with the redistribution, if
 * any, must include the following acknowledgment: "This product includes
 * software developed by Sam Jansen and Jesse Baker
 * (see http://www.wand.net.nz/~stj2/bung)."
 * Alternately, this acknowledgment may appear in the software itself, if
 * and wherever such third-party acknowledgments normally appear.
 *
 * 4. The hosted project names must not be used to endorse or promote
 * products derived from this software without prior written
 * permission. For written permission, please contact sam@wand.net.nz or
 * jtb5@cs.waikato.ac.nz.
 *
 * 5. Products derived from this software may not use the "Bung" name
 * nor may "Bung" appear in their names without prior written
 * permission of Sam Jansen or Jesse Baker.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL COLLABNET OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * */
// $Id$


#include "stdafx.h"
#include "md3.h"
#include "texture_manager.h"
#include "world.h"
#include "display_manager.h"
#include "reporter.h"
#include "exception.h"
#include "misc.h"
#include "config.h"
#include "vfs.h"
#include "matrix.h"
#include "quaternion.h"

#include <fstream>

// HAX: for drawing for now
#include <GL/gl.h>

static const float scale = 0.021f;
static float model_g_scale = 1.0f;

void CResourceManager::LoadMD3(const char *name, CMD3 &md3)
{
	map<const char *, CMD3Factory *>::iterator i = md3s.find(name);
	if(i != md3s.end()) {
		// Looks like it is already loaded.
		(*i).second->CreateMD3(md3);
		return;
	}

	CMD3Factory *fact = new CMD3Factory();
	fact->CreateFactory(name);
	fact->CreateMD3(md3);

	md3s[name] = fact;
}


CMD3Factory::CMD3Factory()
{
}

void CMD3Factory::CreateFactory(const string &fileName)
{
	base.LoadMD3(fileName);
	//base.collider = CCollider::Create();
	//base.BuildCollisionInfo();
}

void CMD3Factory::CreateMD3(CMD3 &md3)
{
	md3 = base;
}


void t3DObject::BuildCollisionInfo() { 
	//collider = CCollider::Create(); 
	//collider->BuildCollisionInfo(((CMesh *)this));
    collider = NULL;
}

CMD3::CMD3()
{
	collider = NULL;
	
	/*m_Head = new t3DModel;
	m_Upper = new t3DModel;
	m_Lower = new t3DModel;
	m_Weapon = new t3DModel;*/
}

CMD3::~CMD3()
{	
}	

bool CMD3::LoadMD3(string modelName, string color)
{
	list< string > models;
	list< string > skins;

	string directory;
	string config_file;
	string model_name;

	//int model_count = 0;
	//int links_count = 0;
	model_scale = 1.0f;

	CLoadMD3 loadMd3;
	
	directory = bsprintf("models/%s",  modelName.c_str());
	
	if( !loadMd3.ImportMD3(&upper, 
			bsprintf("%s/%s", directory.c_str(), "upper.md3")) )
		throw CException("Unable to load upper model!");

	if(!loadMd3.LoadSkin(&upper, 
			bsprintf("%s/%s%s%s", directory.c_str(), "upper_", color.c_str(), ".skin"), 
			directory)) 
		throw CException(bsprintf("Unable to load %s skin!", ""));

	if( !loadMd3.ImportMD3(&lower, 
			bsprintf("%s/%s", directory.c_str(), "lower.md3")) )
		throw CException("Unable to load lower model!");

	if(!loadMd3.LoadSkin(&lower, 
			bsprintf("%s/%s%s%s", directory.c_str(), "lower_", color.c_str(), ".skin"), 
			directory)) 
		throw CException(bsprintf("Unable to load %s skin!", ""));

	if( !loadMd3.ImportMD3(&head, 
			bsprintf("%s/%s", directory.c_str(), "head.md3")) )
		throw CException("Unable to load head model!");

	if(!loadMd3.LoadSkin(&head, 
			bsprintf("%s/%s%s%s", directory.c_str(), "head_", color.c_str(), ".skin"), 
			directory)) 
		throw CException(bsprintf("Unable to load %s skin!", ""));


	LinkModel(&lower, &upper, "tag_torso");
	LinkModel(&upper, &head, "tag_head");


	LoadAnimations( bsprintf("models/%s/animation.cfg", modelName.c_str()).c_str() );


	upper.currentAnim = 12;
	lower.currentAnim = 14;

	upper.currentFrame = upper.pAnimations[upper.currentAnim].startFrame;
	lower.currentFrame = lower.pAnimations[lower.currentAnim].startFrame;

	return true;
}

bool CMD3::LoadWeapon(string weaponName)
{
	/*string strWeaponModel;	// This stores the file name for the weapon model
	string strWeaponShader;	// This stores the file name for the weapon shader.
	CLoadMD3 loadMd3;		// This object allows us to load the.md3 and .shader file

	if(!modelName.Length() > 0) return false;

	// Concatenate the path and model name together
	sprintf(strWeaponModel, "%s\\%s.md3", strPath, strModel);
	
	// Next we want to load the weapon mesh.  The CModelMD3 class has member
	// variables for the weapon model and all it's sub-objects.  This is of type t3DModel.
	// We pass in a reference to this model in to ImportMD3 to save the data read.
	// This returns a true of false to let us know that the weapon was loaded okay.  The
	// appropriate file name to load is passed in for the last parameter.

	// Load the weapon mesh (*.md3) and make sure it loaded properly
	if(!loadMd3.ImportMD3(&m_Weapon,  strWeaponModel))
	{
		// Display the error message that we couldn't find the weapon MD3 file and return false
		MessageBox(g_hWnd, "Unable to load the WEAPON model!", "Error", MB_OK);
		return false;
	}

	// Unlike the other .MD3 files, a weapon has a .shader file attached with it, not a
	// .skin file.  The shader file has it's own scripting language to describe behaviors
	// of the weapon.  All we care about for this tutorial is it's normal texture maps.
	// There are other texture maps in the shader file that mention the ammo and sphere maps,
	// but we don't care about them for our purposes.  I gutted the shader file to just store
	// the texture maps.  The order these are in the file is very important.  The first
	// texture refers to the first object in the weapon mesh, the second texture refers
	// to the second object in the weapon mesh, and so on.  I didn't want to write a complex
	// .shader loader because there is a TON of things to keep track of.  It's a whole
	// scripting language for goodness sakes! :)  Keep this in mind when downloading new guns.

	// Add the path, file name and .shader extension together to get the file name and path
	sprintf(strWeaponShader, "%s\\%s.shader", strPath, strModel);

	// Load our textures associated with the gun from the weapon shader file
	if(!loadMd3.LoadShader(&m_Weapon, strWeaponShader))
	{
		// Display the error message that we couldn't find the shader file and return false
		MessageBox(g_hWnd, "Unable to load the SHADER file!", "Error", MB_OK);
		return false;
	}

	// We should have the textures needed for each weapon part loaded from the weapon's
	// shader, so let's load them in the given path.
	LoadModelTextures(&m_Weapon, strPath);

	// Just like when we loaded the character mesh files, we need to link the weapon to
	// our character.  The upper body mesh (upper.md3) holds a tag for the weapon.
	// This way, where ever the weapon hand moves, the gun will move with it.

	// Link the weapon to the model's hand that has the weapon tag
	LinkModel(&m_Upper, &m_Weapon, "tag_weapon");
		
	// The weapon loaded okay, so let's return true to reflect this*/
	return true;
}

bool CMD3::LoadAnimations(const char *fileName)
{
	auto_ptr<CTextReader> reader(new CTextReader(CVFS::LoadFile(fileName)));
	char buf[256];
	int line_length, anim_num = 0, torso_offset = 0, gesture_startFrame = 0;

	do {
		int startFrame =0, numOfFrames = 0, loopingFrames = 0, framesPerSecond = 0;
		tAnimationInfo anim;

		line_length = reader->ReadLine(buf, 255);
		
		// If it doesn't start with a number its not an animation line
		if(!isdigit(buf[0])) 
			continue;

		sscanf(buf, "%d %d %d %d", &startFrame, &numOfFrames, &loopingFrames, &framesPerSecond);

		anim.startFrame = startFrame;
		anim.endFrame = startFrame + numOfFrames;
		anim.loopingFrames = loopingFrames;
		anim.framesPerSecond = framesPerSecond;

		if(anim_num < 6) {
			// In this case we are in the BOTH_ section
			upper.pAnimations.push_back(anim);
			lower.pAnimations.push_back(anim);
		} else if(anim_num < 13) {
			// In this case we are in the TORSO_ section
			if(anim_num == 6) { 
				gesture_startFrame = anim.startFrame;
			}

			upper.pAnimations.push_back(anim);
		} else {
			// Otherwise we are in the LEGS_ section
			if(anim_num == 13) { 
				torso_offset = anim.startFrame - gesture_startFrame;
			}
			
			// Because I found that some config files have the starting frame for the
            // torso and the legs a different number, we need to account for this by finding
            // the starting frame of the first legs animation, then subtracting the starting
            // frame of the first torso animation from it.  For some reason, some exporters
            // might keep counting up, instead of going back down to the next frame after the
            // end frame of the BOTH_DEAD3 anim.  This will make your program crash if so.
            
            // If the torso offset hasn't been set, set it
            /* if(!torsoOffset)
                torsoOffset = 
					animations[LEGS_WALKCR].startFrame - animations[TORSO_GESTURE].startFrame;
			 // Already done		
			 */

            // Minus the offset from the legs animation start and end frame.
            anim.startFrame -= torso_offset;
            anim.endFrame -= torso_offset;

            // Add the animation to the list of leg animations
            lower.pAnimations.push_back(anim);
		}
		
		anim_num++;

		if(anim_num == 25)
			break;

	} while(1);

	upper.numOfAnimations = upper.pAnimations.size();
	lower.numOfAnimations = lower.pAnimations.size();
	head.numOfAnimations = head.pAnimations.size();

	return true;
}

void  CMD3::LinkModel(t3DModel *pModel, t3DModel *pLink, string strTagName)
{
	if(!pModel || !pLink) return;

	for(int i = 0; i < pModel->numOfTags; i++)
	{
		if( pModel->pTags[i].strName == strTagName )
		{
			pModel->pLinks[i] = pLink;
			return;
		}
	}
}

void CMD3::Draw()
{
	world.display->PushMatrix();
	CMatrix4f sam;
	sam.SetToIdentity();
	sam.RotateY(-PI / 2.0f);
	world.display->MultMatrix(sam);
	
	DrawLink(&lower); 
	world.display->PopMatrix();
}

void CMD3::Update(float diff) 
{

	{
		UpdateModel(&lower);
		float elapsedTime = lower.lastTime + diff;
		float animSpeed = lower.pAnimations[lower.currentAnim].framesPerSecond;
		float t = elapsedTime / (1.0f / animSpeed);
		
		if(elapsedTime >= (1.0f / animSpeed)) {
			/*Log("frame %d -> %d (anim:%d)\n", lower.currentFrame, lower.nextFrame,
					lower.currentAnim);*/
			lower.currentFrame = lower.nextFrame;
			lower.lastTime = 0.0f;
		} else
			lower.lastTime = elapsedTime;

		lower.t = t;
	}
	{
		UpdateModel(&upper);
		float elapsedTime = upper.lastTime + diff;
		float animSpeed = upper.pAnimations[upper.currentAnim].framesPerSecond;
		float t = elapsedTime / (1.0f / animSpeed);
		
		if(elapsedTime >= (1.0f / animSpeed)) {
			upper.currentFrame = upper.nextFrame;
			upper.lastTime = 0.0f;
		} else
			upper.lastTime = elapsedTime;

		upper.t = t;
	}

}

void CMD3::DrawLink(t3DModel *pModel)
{
	CQuaternion qQuat, qNextQuat, qInterpolatedQuat;
	float *pMatrix, *pNextMatrix;
	CMatrix4f finalMatrix;
	
	RenderModel(pModel);
	for(int i = 0; i < pModel->numOfTags; i++)
	{
		t3DModel *pLink = pModel->pLinks[i];
		if(pLink) {			
			// To find the current translation position for this frame of animation, we times
            // the currentFrame by the number of tags, then add i.  This is similar to how
            // the vertex key frames are interpolated.
            Vector3f vPosition = pModel->pTags[pModel->currentFrame * 
				pModel->numOfTags + i].vPosition;

            // Grab the next key frame translation position
            Vector3f vNextPosition = pModel->pTags[pModel->nextFrame * 
				pModel->numOfTags + i].vPosition;
        
			// By using the equation: p(t) = p0 + t(p1 - p0), with a time t,
            // we create a new translation position that is closer to the next key frame.
            vPosition.x = vPosition.x + pModel->t * (vNextPosition.x - vPosition.x),
            vPosition.y = vPosition.y + pModel->t * (vNextPosition.y - vPosition.y),
            vPosition.z = vPosition.z + pModel->t * (vNextPosition.z - vPosition.z); 	
			
			// Now comes the more complex interpolation.  Just like the translation, we
            // want to store the current and next key frame rotation matrix, then interpolate
            // between the 2.

            // Get a pointer to the start of the 3x3 rotation matrix for the current frame
            pMatrix = &pModel->pTags[pModel->currentFrame * pModel->numOfTags + i].rotation[0][0];

            // Get a pointer to the start of the 3x3 rotation matrix for the next frame
            pNextMatrix = &pModel->pTags[pModel->nextFrame * pModel->numOfTags + i].rotation[0][0];

			// Now that we have 2 1D arrays that store the matrices, let's interpolate them

            // Convert the current and next key frame 3x3 matrix into a quaternion
            qQuat.BuildFromMatrix3x3(pMatrix);
            qNextQuat.BuildFromMatrix3x3(pNextMatrix);

            // Using spherical linear interpolation, we find the interpolated quaternion
            qInterpolatedQuat.Slerp(qQuat, qNextQuat, pModel->t);

			// Here we convert the interpolated quaternion into a 4x4 matrix
            qInterpolatedQuat.CreateMatrix( finalMatrix );
            
            // To cut out the need for 2 matrix calls, we can just slip the translation
            // into the same matrix that holds the rotation.  That is what index 12-14 holds.
            finalMatrix[12] = vPosition.x;
            finalMatrix[13] = vPosition.y;
            finalMatrix[14] = vPosition.z;

			world.display->PushMatrix();
			world.display->MultMatrix(finalMatrix);
			DrawLink(pLink);
			world.display->PopMatrix();
			
			/*
			// This was the old (working but static) code   
			Vector3f vPosition = pModel->pTags[i].vPosition;
			world.display->PushMatrix();
			world.display->Translate(vPosition);
			DrawLink(pLink);
			world.display->PopMatrix();
			*/
		}
	}

}

void CMD3::RenderModel(t3DModel *pModel)
{
	if(pModel->pObject.size() <= 0) 
		return;

	for(int i = 0; i < (signed)pModel->pObject.size(); i++) {
#if 0		
		pModel->pObject[i].Draw();	
#else		
		world.display->BindTexture(pModel->pObject[i].tex);
		//int num_tris = pModel->pObject[i].numTriangles;
		int currentIndex = pModel->currentFrame * pModel->pObject[i].numVerts;
		int nextIndex = pModel->nextFrame * pModel->pObject[i].numVerts;

		glBegin(GL_TRIANGLES);
		
		for(int j = 0; j < pModel->pObject[i].numTriangles; j++) {

			for(int whichVertex = 0; whichVertex < 3; whichVertex++) {
				int index = pModel->pObject[i].vertex_indices[j * 3 + whichVertex];
				
				Vector3f p1 = pModel->pObject[i].vertices[currentIndex + index];
				Vector3f p2 = pModel->pObject[i].vertices[nextIndex + index];

				{
					glTexCoord2f(pModel->pObject[i].texCoords[ index ].x, 
								 pModel->pObject[i].texCoords[ index ].y);
					
					glVertex3f(p1.x + pModel->t * (p2.x - p1.x),
							   p1.y + pModel->t * (p2.y - p1.y),
							   p1.z + pModel->t * (p2.z - p1.z));
				}
			}
			
			
			/*Vector3f p1 = pModel->pObject[i].vertices[currentIndex + index];
			Vector3f p2 = pModel->pObject[i].vertices[nextIndex + index];

			{
				glTexCoord2f(pModel->pObject[i].texCoords[ index ].x, 
							 pModel->pObject[i].texCoords[ index ].y);
				
				glVertex3f(p1.x + pModel->t * (p2.x - p1.x),
						   p1.y + pModel->t * (p2.y - p1.y),
						   p1.z + pModel->t * (p2.z - p1.z));
			}*/
		}

		glEnd();
#endif
	}
}

void CMD3::UpdateModel(t3DModel *pModel)
{
    // Initialize a start and end frame, for models with no animation
    int startFrame = 0;
    int endFrame   = 1;

    // This function is used to keep track of the current and next frames of animation
    // for each model, depending on the current animation.  Some models down have animations,
    // so there won't be any change.

    // Here we grab the current animation that we are on from our model's animation list
    tAnimationInfo *pAnim = &(pModel->pAnimations[pModel->currentAnim]);

    // If there is any animations for this model
    if(pModel->numOfAnimations)
    {
        // Set the starting and end frame from for the current animation
        startFrame = pAnim->startFrame;
        endFrame   = pAnim->endFrame;
    }
    
    // This gives us the next frame we are going to.  We mod the current frame plus
    // 1 by the current animations end frame to make sure the next frame is valid.
    pModel->nextFrame = (pModel->currentFrame + 1) % endFrame;

    // If the next frame is zero, that means that we need to start the animation over.
    // To do this, we set nextFrame to the starting frame of this animation.
    if(pModel->nextFrame == 0) 
        pModel->nextFrame =  startFrame;

}

//////////////////////////  BELOW IS THE LOADER CLASS //////////////////////////////

CLoadMD3::CLoadMD3()
{
	// Here we initialize our structures to 0
	memset(&m_Header, 0, sizeof(tMd3Header));

	// Set the pointers to null
	m_pSkins=NULL;
	m_pTexCoords=NULL;
	m_pTriangles=NULL;
	m_pBones=NULL;
}

bool CLoadMD3::ImportMD3(t3DModel *pModel, string strFileName)
{
	char strMessage[255] = {0};

	m_FilePointer = fopen(strFileName.c_str(), "rb");

	// Make sure we have a valid file pointer (we found the file)
	if(!m_FilePointer) 
	{
		// Display an error message and don't load anything if no file was found
		sprintf(strMessage, "Unable to find the file: %s!", strFileName.c_str());
		
		throw CException(strMessage);
	}
	
	// Now that we know the file was found and it's all cool, let's read in
	// the header of the file.  If it has the correct 4 character ID and version number,
	// we can continue to load the rest of the data, otherwise we need to print an error.

	// Read the header data and store it in our m_Header member variable
	fread(&m_Header, 1, sizeof(tMd3Header), m_FilePointer);

	// Get the 4 character ID
	char *ID = m_Header.fileID;

	// The ID MUST equal "IDP3" and the version MUST be 15, or else it isn't a valid
	// .MD3 file.  This is just the numbers ID Software chose.

	// Make sure the ID == IDP3 and the version is this crazy number '15' or else it's a bad egg
	if((ID[0] != 'I' || ID[1] != 'D' || ID[2] != 'P' || ID[3] != '3') || m_Header.version != 15)
	{
		// Display an error message for bad file format, then stop loading
		sprintf(strMessage, "Invalid file format (Version not 15): %s!", strFileName.c_str());
		
		throw CException(strMessage);
	}
	ReadMD3Data(pModel);
	CleanUp();

	return true;
}

void CLoadMD3::ReadMD3Data(t3DModel *pModel)
{
	int i = 0;

	m_pBones = new tMd3Bone [m_Header.numFrames];
	fread(m_pBones, sizeof(tMd3Bone), m_Header.numFrames, m_FilePointer);

//	delete [] m_pBones;

	pModel->pTags = new tMd3Tag [m_Header.numFrames * m_Header.numTags];
	fread(pModel->pTags, sizeof(tMd3Tag), m_Header.numFrames * m_Header.numTags, m_FilePointer);

	pModel->numOfTags = m_Header.numTags;
	
	pModel->pLinks = new t3DModel*[m_Header.numTags];
	
	for (i = 0; i < m_Header.numFrames * m_Header.numTags; i++) 
		pModel->pTags[i].vPosition *= scale;

	for (i = 0; i < m_Header.numTags; i++)
		pModel->pLinks[i] = NULL;

	long meshOffset = ftell(m_FilePointer);

	tMd3MeshInfo meshHeader;

	// Go through all of the sub-objects in this mesh
	for (i = 0; i < m_Header.numMeshes; i++)
	{
		// Seek to the start of this mesh and read in it's header
		fseek(m_FilePointer, meshOffset, SEEK_SET);
		fread(&meshHeader, sizeof(tMd3MeshInfo), 1, m_FilePointer);

		// Here we allocate all of our memory from the header's information
		m_pSkins     = new tMd3Skin [meshHeader.numSkins];
		m_pTexCoords = new tMd3TexCoord [meshHeader.numVertices];
		m_pTriangles = new tMd3Face [meshHeader.numTriangles];
		m_pVertices  = new tMd3Triangle [meshHeader.numVertices * meshHeader.numMeshFrames];

		// Read in the skin information
		fread(m_pSkins, sizeof(tMd3Skin), meshHeader.numSkins, m_FilePointer);
		
		// Seek to the start of the triangle/face data, then read it in
		fseek(m_FilePointer, meshOffset + meshHeader.triStart, SEEK_SET);
		fread(m_pTriangles, sizeof(tMd3Face), meshHeader.numTriangles, m_FilePointer);

		// Seek to the start of the UV coordinate data, then read it in
		fseek(m_FilePointer, meshOffset + meshHeader.uvStart, SEEK_SET);
		fread(m_pTexCoords, sizeof(tMd3TexCoord), meshHeader.numVertices, m_FilePointer);

		// Seek to the start of the vertex/face index information, then read it in.
		fseek(m_FilePointer, meshOffset + meshHeader.vertexStart, SEEK_SET);
		fread(m_pVertices, sizeof(tMd3Triangle), meshHeader.numMeshFrames * meshHeader.numVertices, m_FilePointer);

		// Now that we have the data loaded into the Quake3 structures, let's convert them to
		// our data types like t3DModel and t3DObject.  That way the rest of our model loading
		// code will be mostly the same as the other model loading tutorials.
		ConvertDataStructures(pModel, meshHeader);

		// Free all the memory for this mesh since we just converted it to our structures
		delete [] m_pSkins;    
		delete [] m_pTexCoords;
		delete [] m_pTriangles;
		delete [] m_pVertices;   
//		delete [] m_pBones;

		// Increase the offset into the file
		meshOffset += meshHeader.meshSize;
	}
	//pModel->pTagsvPosition
}

void CLoadMD3::ConvertDataStructures(t3DModel *pModel, tMd3MeshInfo meshHeader)
{
	int i = 0;
	pModel->numOfObjects++;
	t3DObject currentMesh;
	currentMesh.strName = meshHeader.strName;

	currentMesh.numTriangles = meshHeader.numTriangles;
	currentMesh.numVerts = meshHeader.numVertices;

	/*for(i = 0; i < meshHeader.numTriangles * meshHeader.numMeshFrames; i++)
	{
		for(int a= 0; a < 3; a++) {
			Vector3f vert3f(
				m_pVertices[m_pTriangles[i%meshHeader.numTriangles].vertexIndices[a]].vertex[0] / (64.0f / scale) * model_g_scale,
				m_pVertices[m_pTriangles[i%meshHeader.numTriangles].vertexIndices[a]].vertex[1] / (64.0f / scale) * model_g_scale,
				m_pVertices[m_pTriangles[i%meshHeader.numTriangles].vertexIndices[a]].vertex[2] / (64.0f / scale) * model_g_scale);
			currentMesh.vertices.push_back(vert3f);
		}
	}*/

	for(i = 0; i < meshHeader.numVertices * meshHeader.numMeshFrames; i++) {
		Vector3f vert(
			m_pVertices[i].vertex[0] / (64.0f / scale) * model_g_scale,
			m_pVertices[i].vertex[1] / (64.0f / scale) * model_g_scale,
			m_pVertices[i].vertex[2] / (64.0f / scale) * model_g_scale
			);
		currentMesh.vertices.push_back(vert);
	}

	for(i = 0; i < meshHeader.numVertices; i++) {
		Vector2f vert2f(
			m_pTexCoords[i].textureCoord[0],
			-m_pTexCoords[i].textureCoord[1]
			);
		currentMesh.texCoords.push_back(vert2f);
	}

	// Go through all of the face data and assign it over to OUR structure
    for(i = 0; i < meshHeader.numTriangles; i++)
    {
        // Assign the vertex indices to our face data
        //currentMesh.pFaces[i].vertIndex[0] = m_pTriangles[i].vertexIndices[0];
        //currentMesh.pFaces[i].vertIndex[1] = m_pTriangles[i].vertexIndices[1];
        //currentMesh.pFaces[i].vertIndex[2] = m_pTriangles[i].vertexIndices[2];
		currentMesh.vertex_indices.push_back(m_pTriangles[i].vertexIndices[0]);
		currentMesh.vertex_indices.push_back(m_pTriangles[i].vertexIndices[1]);
		currentMesh.vertex_indices.push_back(m_pTriangles[i].vertexIndices[2]);

        // Assign the texture coord indices to our face data (same as the vertex indices)
        //currentMesh.pFaces[i].coordIndex[0] = m_pTriangles[i].vertexIndices[0];
        //currentMesh.pFaces[i].coordIndex[1] = m_pTriangles[i].vertexIndices[1];
        //currentMesh.pFaces[i].coordIndex[2] = m_pTriangles[i].vertexIndices[2];
		currentMesh.tex_indices.push_back(m_pTriangles[i].vertexIndices[0]);
		currentMesh.tex_indices.push_back(m_pTriangles[i].vertexIndices[1]);
		currentMesh.tex_indices.push_back(m_pTriangles[i].vertexIndices[2]);
    }


	// Here we add the current object to our list object list
	pModel->pObject.push_back(currentMesh);
}

bool CLoadMD3::IsInString(string strString, string strSubString)
{
	if(strString.length() <= 0 || strSubString.length() <= 0) return false;
	string::size_type index = strString.find(strSubString);
	if((int)index >= (int)0 && (int)index < (int)strString.length())
		return true;
	return false;
}

bool CLoadMD3::LoadSkin(t3DModel *pModel, string strSkin, string directory)
{
	// Make sure valid data was passed in
	if(!pModel || !(strSkin.length() > 0)) return false;

	ifstream fin(strSkin.c_str());

	// Make sure the file was opened
	if(fin.fail())
	{
		// Display the error message and return false
		throw CException("Unable to load skin");
	}

	// These 2 variables are for reading in each line from the file, then storing
	// the index of where the bitmap name starts after the last '/' character.
	string strLine = "";
	int textureNameStart = 0;

	// Go through every line in the .skin file
	while(getline(fin, strLine))
	{
		// Loop through all of our objects to test if their name is in this line
		for(int i = 0; i < pModel->numOfObjects; i++)
		{
			// Check if the name of this object appears in this line from the skin file
			if( IsInString(strLine, pModel->pObject[i].strName) )			
			{			
				// To abstract the texture name, we loop through the string, starting
				// at the end of it until we find a '/' character, then save that index + 1.
				for(int j = (int)strLine.length() - 1; j > 0; j--)
				{
					// If this character is a '/', save the index + 1
					if(strLine[j] == '/')
					{
						// Save the index + 1 (the start of the texture name) and break
						textureNameStart = j + 1;
						break;
					}	
				}
				try {
					string image(&strLine[textureNameStart]);
					string fn = bsprintf("%s/%s", directory.c_str(), image.c_str());

					// Stupid stupid stupid.. hax to get rid of the \r shit that stuffs linux loading up
					if(fn[ fn.size() - 1 ] == '\r') {
						fn[ fn.size() - 1 ] = '\0';
					}
					
					pModel->pObject[i].tex = CTextureManager::tm.LoadTexture(fn);

				}
				catch(string err) {
					pModel->pObject[i].tex = NULL;
				}
			}
		}
	}

	// Close the file and return a success
	fin.close();
	return true;
}


void CLoadMD3::CleanUp()
{
	fclose(m_FilePointer);						
}

///////////////////////////////// LOAD SHADER \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This loads the basic shader texture info associated with the weapon model
/////
///////////////////////////////// LOAD SHADER \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/*
bool CLoadMD3::LoadShader(t3DModel *pModel, LPSTR strShader)
{
	// Make sure valid data was passed in
	if(!pModel || !strShader) return false;

	// This function is used to load the .shader file that is associated with
	// the weapon model.  Instead of having a .skin file, weapons use a .shader file
	// because it has it's own scripting language to describe the behavior of the
	// weapon.  There is also many other factors like environment map and sphere map
	// textures, among other things.  Since I am not trying to replicate Quake, I
	// just care about the weapon's texture.  I went through each of the blocks
	// in the shader file and deleted everything except the texture name (of course
	// I changed the .tga files to .bmp for our purposes).  All this file now includes
	// is a texture name on each line.  No parsing needs to be done.  It is important
	// to keep in mind that the order of which these texture are stored in the file
	// is in the same order each sub-object is loaded in the .md3 file.  For instance,
	// the first texture name on the first line of the shader is the texture for
	// the main gun object that is loaded, the second texture is for the second sub-object
	// loaded, and so on. I just want to make sure that you understand that I hacked
	// up the .shader file so I didn't have to parse through a whole language.  This is
	// not a normal .shader file that we are loading.  I only kept the relevant parts.

	// Open the shader file
	ifstream fin(strShader);

	// Make sure the file was opened
	if(fin.fail())
	{
		// Display the error message and return false
		MessageBox(NULL, "Unable to load shader!", "Error", MB_OK);
		return false;
	}

	// These variables are used to read in a line at a time from the file, and also
	// to store the current line being read so that we can use that as an index for the 
	// textures, in relation to the index of the sub-object loaded in from the weapon model.
	string strLine = "";
	int currentIndex = 0;
	
	// Go through and read in every line of text from the file
	while(getline(fin, strLine))
	{
		// Create a local material info structure
		tMaterialInfo texture;

		// Copy the name of the file into our texture file name variable
		strcpy(texture.strFile, strLine.c_str());
				
		// The tile or scale for the UV's is 1 to 1 
		texture.uTile = texture.uTile = 1;

		// Store the material ID for this object and set the texture boolean to true
		pModel->pObject[currentIndex].materialID = pModel->numOfMaterials;
		pModel->pObject[currentIndex].bHasTexture = true;

		// Here we increase the number of materials for the model
		pModel->numOfMaterials++;

		// Add the local material info structure to our model's material list
		pModel->pMaterials.push_back(texture);

		// Here we increase the material index for the next texture (if any)
		currentIndex++;
	}

	// Close the file and return a success
	fin.close();
	return true;
}*/




