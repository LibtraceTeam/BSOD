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
#ifndef __D3D_DISPLAY_MANAGER_H__
#define __D3D_DISPLAY_MANAGER_H__

struct CTexture;
class CActor;
class CFont;

class CD3DDisplayManager : public CDisplayManager
{
private:
	//bool wireframe;
	//bool backfaceCull;
	int width, height;

	LPDIRECT3D8             d3d; // Used to create the D3DDevice
	LPDIRECT3DDEVICE8       d3d_device; // Our rendering device
	LPDIRECT3DVERTEXBUFFER8 vertex_buffer; // Buffer to hold vertices
	LPDIRECT3DINDEXBUFFER8  index_buffer;
	bool					texturingOn;
	BlendMode				blendMode;

public:
	static int meshs_drawn, triangles_drawn;

	virtual int GetNumTrianglesDrawn() { return triangles_drawn; };
	virtual int GetNumMeshesDrawn() { return meshs_drawn; };
	
	void CreateD3D(int width, int height, bool fullscreen, HWND hwnd);
	

	/*

	enum BlendMode {
		ADDITIVE = 0,
		MULTIPLY = 1
	}; */
	
	// member functions

	// Initialisation
	virtual void Initialise();

	// Texture management
	virtual CDisplayManager::ImageType RequestSupportedImageType(CDisplayManager::ImageType type);
	virtual int	 LoadTexture(void *bytes, CDisplayManager::ImageType type, int width, int height);
	virtual void UnloadTexture(string texName);
	virtual void UnloadTexture(CTexture *tex);
	
	// Primitive drawing
	virtual void DrawSphere(Vector3f offset, float width);
	virtual void DrawBox(Vector3f offset, float width);
	virtual void DrawBox(Vector3f offset, Vector3f dims);
	
	// TEMPORARY
	virtual void DrawActor(CActor *act);

	// New stuff:
	virtual void BeginFrame2();
	virtual void EndFrame2();

	virtual void SetCameraTransform(CCamera &cam);
	virtual void PushMatrix();
	virtual void Translate(Vector3f position);
	virtual void PopMatrix();
	virtual void Rotate(float amount, Vector3f axis);

	/** Bind texture tex to be the texture used for subsequent drawing calls. A NULL 
	 * parameter indicates that texturing should be turned off. */
	virtual void BindTexture(CTexture *tex);
	/** Sets up the colour subsequent primitives are drawn in.  An optional alpha value
	 * allows setting up a default blend value (only used if blending is turned on). */
	virtual void SetColour(float red, float green, float blue, float alpha = 1.0f);
	/** Sets whether subsequent primitives will be rendered with blending enabled. */
	virtual void SetBlend(bool on);
	virtual void SetBlendMode(BlendMode bm);
	virtual BlendMode GetBlendMode();

	// 2D graphics functions
	/** Sets the render state to 2D-mode without depth testing or backface culling.
	 * This mode is suitable for drawing text and user interfaces, for example. */
	virtual void Begin2D();
	/** Draws string str at location (x,y) on the screen.  This function assumes
	 * Begin2D() has been called before it to setup 2D mode. */
	virtual void DrawString2(int x, int y, const string &str);
	/** Draws a 2D quad on the screen stretching the currently-bound texture over
	 * the entire image, if a texture is bound.  If no texture is bound it will
	 * draw a flat shaded quad in the current colour.  This function assumes
	 * Begin2D() has been called before it to setup 2D mode. */
	virtual void Draw2DQuad(int left, int top, int right, int bottom);
	/** Restored the render state (generally back to 3D mode). */
	virtual void End2D();
	
	// 3D graphics functions
	// 3D mode is the default; there is no needed Begin3D()..End3D() like 2D
	/** Draws triangles in vertex array vertices in order specified by indices.  */
	virtual void DrawIndexedTriangles2(float *vertices, float *tex_coords, 
		unsigned int *indices, int num_triangles);
	/** Draws triangles in vertex array vertices in sequential order. */
	virtual void DrawTriangles2(float *vertices, float *tex_coords, int num_triangles);
	/** Draws triangles in vertex array vertices in sequential fan order. */
	virtual void DrawTriangleFan2(float *vertices, float *tex_coords, int num_triangles);
	/** Draws triangles in vertex array vertices in sequential strip order. */
	virtual void DrawTriangleStrip(float *vertices, float *tex_coords, int num_triangles);
	//virtual void DrawSphere(Vector3f offset, float radius);
	//virtual void DrawBox(Vector3f offset, float ?);

	virtual void DrawString3(const CFont &fnt, const int x, const int y, const string &str);

	// Other misc functions
	/** Called by the System Driver whenever the user resizes the window. This
	 * func just resets the viewport and matrices for a larger window and updates
	 * the internal width and height. */
	virtual void WindowResized(int new_width, int new_height);
	virtual int GetWidth();
	virtual int GetHeight();

	virtual void SetWireframe(bool w);
	virtual void SetBackfaceCull(bool b);
};



#endif
