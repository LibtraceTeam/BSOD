/* $CVSID$ */ 
#ifndef __LEVELLOADER_H__
#define __LEVELLOADER_H__

class CBezier;
class IMessageCallback;

class CLevelLoader
{
public:
	void LoadLevel(string fileName, COctree &tree, IMessageCallback &screen);

private:
	
	// The below should be in sync with the same in q3level.h
	struct bung_header {
		char	magic[4];
		float	lev_bbox_min[3]; // min vector for level bounding box
		float	lev_bbox_max[3]; // max vector for level bounding box
		uint32	block_indices[30]; // 30 just incase we need more in the future.  4GB limit on file size :)
		uint32	block_nums[30]; // number of whatevers in each block
	};
	enum bung_blocks {
		Textures = 0, TriFans, Beziers, Meshes
	};
	struct bung_texture {
		char	name[128];
	};
	struct bung_bezier {
		int		tex_id;
		int		number;
		// Followed by number times:
		//     float[3] * 3 * 3 (control points)
		//     and then float[2] * 3 * 3 (texture coords)
	};
	/*struct bung_trifan {
		int		tex_id;
		int		no_verts;
		// Followed by float[3] * no_verts (vertices)
		// and then float[2] * no_verts (texture coords)
	};*/

	struct bung_trifan {
		int		tex_id;
		int		lightmap_id;
		int		no_verts;
		// Followed by float[3] * no_verts (vertices)
		// and then float[2] * no_verts (texture coords)
		// and then float[2] * no_verts (lightmap coords)
		// NOT: and then float[3] * width * height (normals)
		// collision info
	};

	
	struct bung_simple_mesh {
		int		tex_id;
		int		no_verts;
		// Followed by float[3] * no_verts (vertices)
		// and then float[2] * no_verts (texture coords)
	};
	bung_header bheader;
	vector<CTexture *> tex_ids;
};

#endif

