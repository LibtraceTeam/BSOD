/* $CVSID$ */ 
#include "stdafx.h"
#include "md2.h"
#include "texture_manager.h"
#include "world.h"
#include "display_manager.h"
#include "reporter.h"
#include "exception.h"


CMD2::CMD2()
{
	curFrameTime = 0.0f;
	curFrame = 0;
	nextFrame = 1;
	curAction = 0;
	t.resize(10000);
}

void CMD2::Update(float diff)
{
	static float ti = 0.10f;
	
	curFrameTime += diff * 10.f; // this 'magic number' affects how fast MD2's are animated -- the higher, the faster
	ti += diff;

	if(curFrameTime > 1.0f) {
		curFrameTime = 0.0f;
		curFrame = nextFrame;
		nextFrame++;

		if(nextFrame > actions[curAction].endFrame) {
			nextFrame = actions[curAction].startFrame;
		}
	}

	/*
	if(ti > (1.0f / 15.0f))
	{
		ti = 0.0f;

		if(v.size() == 0)
		{
			v.resize(12000);
		}

		int nFrame = nextFrame * header.numVertices;
		int cFrame = curFrame * header.numVertices;
		for(unsigned int i = 0; i < vertexIndices.size(); i++) {
			int cInd = cFrame + vertexIndices[i],
				nInd = nFrame + vertexIndices[i];
			int i3 = i*3, i31 = i*3+1, i32 = i*3+2;

			v[i3] = vertices[cInd].x;
			v[i31] = vertices[cInd].y;
			v[i32] = vertices[cInd].z;

			v[i3] += curFrameTime * (-v[i3] + vertices[nInd].x);
			v[i31] += curFrameTime * (-v[i31] + vertices[nInd].y);
			v[i32] += curFrameTime * (-v[i32] + vertices[nInd].z);
		}
	}
	ti = 0.0f;
	*/
}

void CMD2::Draw()
{
	/*
	if(v.size() != 0)
	{


		world.display->BindTexture(tex);
		world.display->DrawTriangles2(&v[0], &t[0], GetNumberTriangles());
		
	}
	
	*/
	//*
	//CReporter::Report(CReporter::R_DEBUG, "Drawing MD2.");
	int cFrame = curFrame * header.numVertices;
	world.display->BindTexture(tex);
	world.display->DrawIndexedTriangles2(
		(float *)&vertices[cFrame], 
		(float *)&texCoords[0], 
		(uint32 *)&vertexIndices[0], 
		GetNumberTriangles());
	//*/
}

bool CMD2::FitInBox(CBox &bbox)
{
	return true;
}

// Temporary!
void CMD2::LoadMD2(string fileName)
{
	//md2_header header; // in the .h
	FILE *md2;

	md2 = fopen(fileName.c_str(), "rb");
	assert(md2);
	fread(&header, sizeof(md2_header), 1, md2);
	
	if(header.magic != ('I' + ('D' << 8) + ('P' << 16) + ('2' << 24))) {
		throw CException("Bad header magic number for md2 file.");
	}
	if(header.version != 8) {
		throw CException("Bad header version for md2 file.");
	}

	ReadSkins(md2, header.offsetSkins, header.numSkins);
	ReadTexCoords(md2, header.offsetTexCoords, header.numTexCoords);
	ReadTriangles(md2, header.offsetTriangles, header.numTriangles);
	ReadFrames(md2, header.offsetFrames, header.numFrames, header.numVertices);

	//*
	vector<Vector2f> ntc;
	ntc.resize(texCoords.size());
	uint32 i;

	for(i = 0; i < textureIndices.size(); i++) {
		uint32 vi = vertexIndices[i];
		uint32 ti = textureIndices[i];

		ntc[vi] = texCoords[ti];
	}

	texCoords = ntc;
	//*/

	/*
	uint32 i;
	for(i = 0; i < textureIndices.size(); i++) {
		t[i*2] = texCoords[textureIndices[i]].x;  
		t[i*2+1] = texCoords[textureIndices[i]].y;
	}
	*/

	/*
	CReporter::Report(CReporter::R_DEBUG, "MD2 size= %d+%d+%d+%d+%d=%d", sizeof(CMD2), vertices.size() * sizeof(Vector3f),
		vertexIndices.size() * 4, textureIndices.size() * 4, texCoords.size() * sizeof(Vector3f),
		sizeof(CMD2) + vertices.size() * sizeof(Vector3f) +	vertexIndices.size() * 4 + textureIndices.size() * 4 +
		texCoords.size() * sizeof(Vector3f)); */

	fclose(md2);
}

void CMD2::ReadSkins(FILE *md2, int offset, int number)
{
	assert(md2);
	char name[64];

	fseek(md2, offset, SEEK_SET);
	
	for(int i = 0; i < number; i++) {
		fread(name, 64, 1, md2);
	}

	try {
		/* TODO:
		 * We need some better way of doing this.  Player md2's don't have a
		 * skin associated with them.  Generally just one of the skins in the
		 * md2's directory is picked by the player.  We don't have a nice way
		 * of checking what is in a directory (especially in an archive) yet,
		 * unfortunately.   Just another thing that needs work...
		 */
		if(number > 0)
			tex = CTextureManager::tm.LoadTexture(name);
		else
			tex = CTextureManager::tm.LoadTexture("md2s/yohko/Yohko.pcx");
	}
	catch(string err) {
		tex = NULL;
	}
}

void CMD2::ReadTexCoords(FILE *md2, int offset, int number)
{
	vector<md2_tex_coord> md2_tex_coords;

	assert(md2);

	fseek(md2, offset, SEEK_SET);

	md2_tex_coords.resize(number);
	texCoords.reserve(number);
	
	fread(&md2_tex_coords[0], sizeof(md2_tex_coord), number, md2);

	for(int i = 0; i < number; i++) {
		texCoords.push_back(Vector2f(
			(float)md2_tex_coords[i].s / (float)header.skinWidth,
			(float)md2_tex_coords[i].t / (float)header.skinHeight));
	}
}

void CMD2::ReadTriangles(FILE *md2, int offset, int number)
{
	vector<md2_triangle> md2_triangles;
	assert(md2);
	int i;

	fseek(md2, offset, SEEK_SET);

	md2_triangles.resize(number);

	fread(&md2_triangles[0], sizeof(md2_triangle), number, md2);
	
	for(i = 0; i < number; i++) {
		vertexIndices.push_back( md2_triangles[i].vertexIndices[0] );
		vertexIndices.push_back( md2_triangles[i].vertexIndices[1] );
		vertexIndices.push_back( md2_triangles[i].vertexIndices[2] );
		textureIndices.push_back( md2_triangles[i].textureIndices[0] );
		textureIndices.push_back( md2_triangles[i].textureIndices[1] );
		textureIndices.push_back( md2_triangles[i].textureIndices[2] );
	}
}

void CMD2::ReadFrames(FILE *md2, int offset, int number, int num_verts)
{
	vector<md2_frame> md2_frames;
	int i;
	string lastName;

	assert(md2);
	
	fseek(md2, offset, SEEK_SET);

	md2_frames.resize(number);

	for(i = 0; i < number; i++) {
		fread(&md2_frames[i], 
			sizeof(md2_frame) 
			- sizeof(md2_tri_vertex) * 2048
			+ sizeof(md2_tri_vertex) * (num_verts), 
			1, md2);

		// Convert vertices to proper order and size
		for(int v = 0; v < num_verts; v++) {
			Vector3f vert;
			vert.x = md2_frames[i].vertices[v].vertex[0] * md2_frames[i].scale[0] +	md2_frames[i].translate[0];
			vert.y = md2_frames[i].vertices[v].vertex[2] * md2_frames[i].scale[2] + md2_frames[i].translate[2];
			vert.z = -(md2_frames[i].vertices[v].vertex[1] * md2_frames[i].scale[1] +	md2_frames[i].translate[1]);
			vert *= 1.0f / 45.0f; // This should correspond to scale in BuNgMap

			vertices.push_back(vert);
		}
	}

	// Now: create actions
	/* This pretty much assumes actions are ordered like: 'stand01, stand02, stand03'.  This seems to always
	 * be the case.  A valid MD2 file might actually have 'stand02, stand01, stand04' or something and expect
	 * to work.  In BuNg, this wont work at all, but all MD2's inspected so far have the frames in order in
	 * the actions.
	 */
	for(i = 0; i < number; i++) {
		//CReporter::Report(CReporter::R_DEBUG, "Action name='%s', actions.size=%d", md2_frames[i].name, actions.size());
		bool next = true;
		unsigned int f = 0;
		if(lastName.length() != strlen(md2_frames[i].name)) next = false;
		else {
			while(f < lastName.length() && f < strlen(md2_frames[i].name)) {
				if((lastName[f] != md2_frames[i].name[f]) && isalpha(lastName[f])) {
					next = false;
					break;
				}
				f++;
			}
		}

		if(next) {
			actions.back().endFrame++;
		}
		else {
			// Time to create a new action!
			md2_action action;

			action.startFrame = action.endFrame = i;
			action.name = lastName = string(md2_frames[i].name);

			actions.push_back(action);
		}
	}

	nextFrame = actions[curAction].startFrame;
}

