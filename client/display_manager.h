/* $CVSID$ */ 
#ifndef __DISPLAY_MANAGER_H__
#define __DISPLAY_MANAGER_H__

struct CTexture;
class CActor;
class CFont;
class CCamera;
class CMatrix;

// XXX: TODO: HAX for now
#include "matrix.h"

class CDisplayManager
{
private:
	bool wireframe;
	bool backfaceCull;

public:
	// member variables
	
	// These should really be handled differently.  But for now, this will do.
	

	///////
	Vector3f cd_verts[3];
	Vector3f contact_pt;
	
	

	enum ImageType {
		R8_G8_B8, // 24 bit RGB
		R8_G8_B8_A8, // 32 bit RGBA
		B8_G8_R8, // 24 bit BGR
		B8_G8_R8_A8, // 32 bit BGRA
		Luminance8 // 8 bit Luminance
	};

	enum DisplayBlendMode {
		Multiply,
		Add,
		Transparent,
		Transparent2
	};
	
	// member functions

	// Initialisation
	virtual void Initialise() = 0;

	// Texture management
	virtual CDisplayManager::ImageType RequestSupportedImageType(CDisplayManager::ImageType type) = 0;
	virtual int	 LoadTexture(void *bytes, CDisplayManager::ImageType type, int width, int height) = 0;
	virtual void UnloadTexture(string texName) = 0;
	virtual void UnloadTexture(CTexture *tex) = 0;
	
	// Primitive drawing
	virtual void DrawSphere(Vector3f offset, float width) = 0;
	virtual void DrawBox(Vector3f offset, float width) = 0;
	virtual void DrawBox(Vector3f offset, Vector3f dims) = 0;
	
	// TEMPORARY
	virtual void DrawActor(CActor *act) = 0;
	virtual void DrawTriangleFan(CTriangleFan *fan) {}

	// New stuff:
	virtual void BeginFrame2() = 0;
	virtual void EndFrame2() = 0;
	virtual void SetCameraTransform(CCamera &cam) = 0;
	virtual void PushMatrix() = 0;
	virtual void Translate(Vector3f position) = 0;
	virtual void MultMatrix(CMatrix4f &mat) = 0;
	virtual void PopMatrix() = 0;
	virtual void Rotate(float amount, Vector3f axis) = 0;
	/** Bind texture tex to be the texture used for subsequent drawing calls. A NULL 
	 * parameter indicates that texturing should be turned off. */
	virtual void BindTexture(CTexture *tex) = 0;
	/** Sets up the colour subsequent primitives are drawn in.  An optional alpha value
	 * allows setting up a default blend value (only used if blending is turned on). */
	virtual void SetColour(float red, float green, float blue, float alpha = 1.0f) = 0;
	/** Sets whether subsequent primitives will be rendered with blending enabled. */
	virtual void SetBlend(bool on) = 0;
	virtual void SetBlendMode(DisplayBlendMode bm) = 0;
	virtual DisplayBlendMode GetBlendMode() = 0;

	// 2D graphics functions
	/** Sets the render state to 2D-mode without depth testing or backface culling.
	 * This mode is suitable for drawing text and user interfaces, for example. */
	virtual void Begin2D() = 0;
	/** Draws string str at location (x,y) on the screen.  This function assumes
	 * Begin2D() has been called before it to setup 2D mode. */
	virtual void DrawString2(int x, int y, const string &str) = 0;
	/** Draws a 2D quad on the screen stretching the currently-bound texture over
	 * the entire image, if a texture is bound.  If no texture is bound it will
	 * draw a flat shaded quad in the current colour.  This function assumes
	 * Begin2D() has been called before it to setup 2D mode. */
	virtual void Draw2DQuad(int left, int top, int right, int bottom) = 0;
	/** Restored the render state (generally back to 3D mode). */
	virtual void End2D() = 0;
	
	// 3D graphics functions
	// 3D mode is the default; there is no needed Begin3D()..End3D() like 2D
	/** Draws triangles in vertex array vertices in order specified by indices.  */
	virtual void DrawIndexedTriangles2(float *vertices, float *tex_coords, 
		unsigned int *indices, int num_triangles) = 0;
	/** Draws triangles in vertex array vertices in sequential order. */
	virtual void DrawTriangles2(float *vertices, float *tex_coords, int num_triangles) = 0;
	/** Draws triangles in vertex array vertices with colours in sequential order. */
	virtual void DrawTriangles2(float *vertices, float *tex_coords, byte *colours, 
		int num_triangles) = 0;
	/** Draws triangles in vertex array vertices in sequential fan order. */
	virtual void DrawTriangleFan2(float *vertices, float *tex_coords, int num_triangles) = 0;
	/** Draws triangles in vertex array vertices in sequential strip order. */
	virtual void DrawTriangleStrip(float *vertices, float *tex_coords, int num_triangles) = 0;
	//virtual void DrawSphere(Vector3f offset, float radius);
	//virtual void DrawBox(Vector3f offset, float ?);

	virtual void DrawString3(const CFont &fnt, const int x, const int y, const string &str) = 0;

	// Other misc functions
	/** Called by the System Driver whenever the user resizes the window. This
	 * func just resets the viewport and matrices for a larger window and updates
	 * the internal width and height. */
	virtual void WindowResized(int new_width, int new_height) = 0;
	virtual int GetWidth() = 0;
	virtual int GetHeight() = 0;

	virtual void SetWireframe(bool w) { wireframe = w; }
	virtual bool GetWireframe() { return wireframe; }
	virtual void SetBackfaceCull(bool b) { backfaceCull = b; }
	virtual bool GetBackfaceCull() { return backfaceCull; }

	virtual int GetNumTrianglesDrawn() = 0;
	virtual int GetNumMeshesDrawn() = 0;
};

typedef CDisplayManager::DisplayBlendMode BlendMode;

#endif

