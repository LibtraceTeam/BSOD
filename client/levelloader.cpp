#include "stdafx.h"

#include "octree.h"
#include "xml_parse.h"
#include "bezier.h"
#include "levelloader.h"
#include "vfs.h"
#include "world.h"
#include "display_manager.h"
#include "texture_manager.h"
#include "misc.h"
#include "reporter.h"
#include "exception.h"
#include "loading_screen.h"

#include <math.h>

void CLevelLoader::LoadLevel(string fileName, COctree &tree, IMessageCallback &screen)
{
	auto_ptr<CReader> reader( CVFS::LoadFile(fileName) );
	int i;
	
	reader->Read((char *)&bheader, sizeof(bung_header));
	
	if(strncmp(bheader.magic, "BuNg", 4) != 0) {
		string err = "Bad magic numbers in level file: '";
		err += bheader.magic[0] + bheader.magic[1] + bheader.magic[2]
			+ bheader.magic[3];
		err += "' instead of 'BuNg'";

		throw err;
	}

	// Header stuff: level bounding box
	Vector3f min_v(bheader.lev_bbox_min[0], bheader.lev_bbox_min[1], bheader.lev_bbox_min[2]), 
		max_v(bheader.lev_bbox_max[0], bheader.lev_bbox_max[1], bheader.lev_bbox_max[2]);
	Vector3f offset = (min_v + max_v) / 2;
	float length;
		
	if(max_v.x - min_v.x > max_v.y - min_v.y)
		length = max_v.x - min_v.x;
	else
		length = max_v.y - min_v.y;

	if(length < max_v.z - min_v.z)
		length = max_v.z - min_v.z;

	tree.InitBBox(length, offset);

	// Load textures:
	if((int)bheader.block_nums[Textures] > 0) {
		screen.AddMessage("    Loading textures...");
		reader->Seek(bheader.block_indices[Textures]);
	}
	for(i = 0; i < (int)bheader.block_nums[Textures]; i++) {
		bung_texture tex;
		reader->Read((char *)&tex, sizeof(bung_texture));
		string texname = string("data/textures.barc/") + string(tex.name);
		//CReporter::Report(CReporter::DEBUG, "Loading texture of name: %s.", texname.c_str());

		try {
			if(strlen(tex.name) > 0)
				tex_ids.push_back( CTextureManager::tm.LoadTexture(texname) );
			else
				tex_ids.push_back( NULL );
		}
		catch(string err)
		{
			tex_ids.push_back( NULL );
		}
		catch(CException e)
		{
			tex_ids.push_back( NULL );
		}
	}

	// Load Beziers
	if((int)bheader.block_nums[Beziers] > 0) {
		screen.AddMessage("    Loading bezier curves...");
		reader->Seek(bheader.block_indices[Beziers]);
	}

	for(i = 0; i < (int)bheader.block_nums[Beziers]; i++) {
		bung_bezier bezier;
		int v, n;

		reader->Read((char *)&bezier, sizeof(bung_bezier));

		for(n = 0; n < bezier.number; n++) {
			CBezier *bez = new CBezier();
			bez->m_x = bez->m_y = 3;
			bez->tex = tex_ids[ bezier.tex_id ];

			for(v = 0; v < 3 * 3; v++) {
				Vector3f cpoint;
				reader->Read((char *)&cpoint, sizeof(float) * 3);

				bez->m_cpts.push_back(cpoint);
			}
			for(v = 0; v < 3 * 3; v++) {
				Vector2f tpoint;
				reader->Read((char *)&tpoint, sizeof(float) * 2);
				
				bez->m_tex_cpts.push_back(tpoint);
			}
			/*
			for(v = 0; v < bezier.width * bezier.height; v++) {
				Vector3f normal;
				reader->Read((char *)&normal, sizeof(float) * 3);

				bez->normals.push_back(normal);
			}
			*/

			
			bez->Tesselate();
//			bez->collider = CCollider::Create();
//			bez->collider->LoadFromDisk(reader.get(), bez);
			
			//delete bez->collider; bez->collider = NULL;

			tree.AddMesh(bez);
			
		}

#if 0
		// At this point we need to do some magic if the bezier
		// is larger than 3x3.
		int nx, ny;
		nx = bez->m_x / 2;
		ny = bez->m_y / 2;

		for(int y = 0; y < ny; y++) {
			for(int x = 0; x < nx; x++) {
				CBezier *bez2 = new CBezier();
				bez2->m_x = bez2->m_y = 3;
				bez2->tex = bez->tex;

				int start_x, end_x;
				int start_y, end_y;
				
				// Thanks to Jesse for the next 4 lines: they made large bezier
				// patches work perfectly!
				start_x = x * 2;
				end_x = (x + 1) * 2;
				start_y = y * 2;
				end_y = (y + 1) * 2;

				vector<Vector3f>::iterator cp = bez->m_cpts.begin(); // contol point(char *)&tex_coord, sizeof(float) * 2);
				vector<Vector2f>::iterator tp = bez->m_tex_cpts.begin(); // texture point
//				vector<Vector3f>::iterator np = bez->normals.begin(); // normal
				for(int c_y = 0; c_y < bez->m_y; c_y++) {
					for(int c_x = 0; c_x < bez->m_x; c_x++) {
						if(c_x >= start_x && c_x <= end_x && c_y >= start_y && c_y <= end_y) {
							bez2->m_cpts.push_back( *cp );
							bez2->m_tex_cpts.push_back( *tp );
//							bez2->normals.push_back( *np );
						}

						++cp;
						++tp;
//						++np;
					}
				}
				
				bez2->Tesselate();
				tree.AddMesh(bez2);
			}
		}
		delete bez;
#endif
	}

	// Load TriFans
	if((int)bheader.block_nums[TriFans] > 0) {
		screen.AddMessage("    Loading polygons...");
		reader->Seek(bheader.block_indices[TriFans]);
	}
	for(i = 0; i < (int)bheader.block_nums[TriFans]; i++) {
		bung_trifan trifan;
		int v;
		CTriangleFan *fan = new CTriangleFan();

		//CReporter::Report(CReporter::DEBUG, "Loading polygon/trifan %d of %d.", i, (int)bheader.block_nums[TriFans]);

		reader->Read((char *)&trifan, sizeof(bung_trifan));
		
		fan->tex = tex_ids[ trifan.tex_id ];

		for(v = 0; v < trifan.no_verts; v++) {
			Vector3f vert;
			reader->Read((char *)&vert, sizeof(float) * 3);
			fan->vertices.push_back(vert);
		}
		for(v = 0; v < trifan.no_verts; v++) {
			Vector2f tex_coord;
			reader->Read((char *)&tex_coord, sizeof(float) * 2);
			fan->texCoords.push_back(tex_coord);
		}
		// BEGIN: HAX
		for(v = 0; v < trifan.no_verts; v++) {
			Vector2f lm_coord;
			reader->Read((char *)&lm_coord, sizeof(float) * 2);
//			fan->lmCoords.push_back(lm_coord);
		}

		if(trifan.lightmap_id >= 0) {
//			fan->lmTex = CTextureManager::tm.LoadTexture(
//				bsprintf("bungmap/level_creation/lightmaps/lightmap_%03d.png", trifan.lightmap_id)
//				);
			//Log("Lightmap at %x.\n", fan->lmTex);
		} else {
//			fan->lmTex = NULL;
			//Log("No lightmap texture (%d)\n", trifan.lightmap_id);
		}

		// END: HAX
		/*
		for(v = 0; v < trifan.no_verts; v++) {
			Vector3f normal;
			reader->Read((char *)&normal, sizeof(float) * 3);
			fan->normals.push_back(normal);
		}
		*/

//		fan->collider = CCollider::Create();
//		fan->collider->LoadFromDisk(reader.get(), fan);

		//delete fan->collider; fan->collider = NULL;

		tree.AddMesh(fan);
	}

	// Load Meshes
	if((int)bheader.block_nums[Meshes] > 0) {
		screen.AddMessage("    Loading meshes...");
		reader->Seek(bheader.block_indices[Meshes]);
	}
	for(i = 0; i < (int)bheader.block_nums[Meshes]; i++) {
		bung_simple_mesh mesh;
		CTriangleMesh *trimesh = new CTriangleMesh();
		int v;
		
		reader->Read((char *)&mesh, sizeof(bung_simple_mesh));

		trimesh->tex = tex_ids[ mesh.tex_id ];

		for(v = 0; v < mesh.no_verts; v++) {
			Vector3f vert;
			reader->Read((char *)&vert, sizeof(float) * 3);
			trimesh->vertices.push_back(vert);
		}
		for(v = 0; v < mesh.no_verts; v++) {
			Vector2f tex_coord;
			reader->Read((char *)&tex_coord, sizeof(float) * 2);
			trimesh->texCoords.push_back(tex_coord);
		}
		/*
		for(v = 0; v < mesh.no_verts; v++) {
			Vector3f normal;
			reader->Read((char *)&normal, sizeof(float) * 3);
			trimesh->normals.push_back(normal);
		}
		*/

//		trimesh->collider = CCollider::Create();
//		trimesh->collider->LoadFromDisk(reader.get(), trimesh);
		
		// HAX
		//delete trimesh->collider; trimesh->collider = NULL;
		// end HAX

		tree.AddMesh(trimesh);
	}
}


