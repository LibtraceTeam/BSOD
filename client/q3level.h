/* $CVSID$ */ 
class Q3LevelLoader
{
public:
	void LoadLevel(string fileName, COctree &tree);

private:
	struct q3_header
	{
		char	magic[4];
		int		version;
		struct q3_direntry
		{
			int offset;
			int length; // Always a multiple of 4, apparently.
		}		lumps[17];
	};

	struct q3_plane
	{
		float normal[3];
		float dist;
	};

	struct q3_node
	{
		int plane;
		int children[2];
		int mins[3];
		int maxs[3];
	};

	struct q3_leaf
	{
		int cluster;
		int area;
		int mins[3];
		int maxs[3];
		int leafface;
		int num_leaffaces;
		int leafbrush;
		int num_leafbrushes;
	};

	struct q3_leafface
	{
		int face;
	};

	struct q3_leafbrush
	{
		int brush;
	};

	struct q3_model
	{
		float mins[3];
		float maxs[3];
		int face;
		int n_faces;
		int brush;
		int num_brushes;
	};

	struct q3_brush
	{
		int brushside;
		int n_brushsides;
		int texture;
	};

	struct q3_brushsides
	{
		int plane;
		int texture;
	};

	struct q3_vertex
	{
		float position[3];//  Vertex position.  
		float texcoord[2][2];  //Vertex texture coordinates. 0=surface, 1=lightmap.  
		float normal[3];//  Vertex normal.  
		unsigned char color[4];//  Vertex color. RGBA.  
	};

	struct q3_meshvert
	{
		int offset; //Vertex index offset, relative to first vertex of corresponding face
	};

	struct q3_face
	{
		int texture;//  Texture index.  
		int effect;//  Index into lump 12 (Effects), or -1.  
		int type;//  Face type. 1=polygon, 2=patch, 3=mesh, 4=billboard  
		int vertex;//  Index of first vertex.  
		int n_vertexes;//  Number of vertices.  
		int meshvert;//  Index of first meshvert.  
		int n_meshverts;//  Number of meshverts.  
		int lm_index;//  Lightmap index.  
		int lm_start[2];//  Corner of this face's lightmap image in lightmap.  
		int lm_size[2];//  Size of this face's lightmap image in lightmap.  
		float lm_origin[3];//  World space origin of lightmap.  
		float lm_vecs[2][3];//  World space lightmap s and t unit vectors.  
		float normal[3];//  Surface normal.  
		int size[2];//  Patch dimensions.  
	};

	struct q3_lightmap
	{
		unsigned char map[128][128][3];
	};

	struct q3_lightvol
	{
		unsigned char ambient[3];
		unsigned char directional[3];
		unsigned char dir[3];
	};

	struct q3_visdata
	{
		int num_vects;
		int size_vects;
		unsigned char *vecs;
	};

	struct q3_texture
	{
		char name[64];
		int flags;
		int contents;
	};

	enum
	{
		/*
		Index  Lump Name  Description  
		0 Entities  Game-related object descriptions.  
		1 Textures  Surface descriptions.  
		2 Planes  Planes used by map geometry.  
		3 Nodes  BSP tree nodes.  
		4 Leafs  BSP tree leaves.  
		5 Leaffaces  Lists of face indices, one list per leaf.  
		6 Leafbrushes  Lists of brush indices, one list per leaf.  
		7 Models  Descriptions of rigid world geometry in map.  
		8 Brushes  Convex polyhedra used to describe solid space.  
		9 Brushsides  Brush surfaces.  
		10 Vertexes  Vertices used to describe faces.  
		11 Meshverts  Lists of offsets, one list per mesh.  
		12 Effects  List of special map effects.  
		13 Faces  Surface geometry.  
		14 Lightmaps  Packed lightmap data.  
		15 Lightvols  Local illumination data.  
		16 Visdata  Cluster-cluster visibility data.  
		*/
		LumpEntities = 0,
		LumpTextures = 1,
		LumpPlanes = 2,
		LumpNodes = 3,
		LumpLeafs = 4,
		LumpLeaffaces = 5,
		LumpLeafbrushes = 6,
		LumpModels = 7,
		LumpBrushes = 8,
		LumpBrushsides = 9,
		LumpVertices = 10,
		LumpMeshverts = 11,
		LumpEffects = 12,
		LumpFaces = 13,
		LumpLightmaps = 14,
		LumpLightvols = 15,
		LumpVisdata = 16
	};

	// The quake 3 file info:
	q3_header header;
	q3_brush *brushes;			int num_brushes;
	q3_brushsides *brushsides;	int num_brushsides;
	q3_face *faces;				int num_faces;
	q3_leaf *leaves;			int num_leaves;
	q3_leafbrush *leafbrushes;	int num_leafbrushes;
	q3_leafface *leaffaces;		int num_leaffaces;
	q3_lightmap *lightmaps;		int num_lightmaps;
	q3_lightvol *lightvols;		int num_lightvols;
	q3_meshvert *meshverts;		int num_meshverts;
	q3_model *models;			int num_models;
	q3_node *nodes;				int num_nodes;
	q3_plane *planes;			int num_planes;
	q3_vertex *vertices;		int num_vertices;
	q3_visdata *visdata;		int num_visdata;
	q3_texture *textures;		int num_textures;

	// The file we will open
	FILE *map;
	
	// Util functions:
	void CreateLumps();
	void ReadLumps();
	void DeleteLumps();
	int LoadTexture(string fileName); // Returns the texture index and binds the texture in opengl
};	

