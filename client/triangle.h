#ifndef __TRIANGLE_H__
#define __TRIANGLE_H__

class CTriangle
{
public:
	CTriangle() {}
	CTriangle(const CTriangle &copy) {
		vertices[0] =	copy.vertices[0];
		vertices[1] = 	copy.vertices[1];
		vertices[2] = 	copy.vertices[2];
		texCoords[0] = 	copy.texCoords[0];
		texCoords[1] = 	copy.texCoords[1];
		texCoords[2] = 	copy.texCoords[2];
		texIndex = 		copy.texIndex;
		normal = 		copy.normal;
	}

	Vector3f	vertices[3];	// 12 * 3 = 36
	Vector2f	texCoords[3];	// 8 * 3 = 24
	uint16		texIndex;		// 2
	Vector3f	normal;			// 12
								// -----------
								// 74 bytes per triangle
};

#endif

