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

