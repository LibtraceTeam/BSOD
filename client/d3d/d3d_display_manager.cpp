#include "../stdafx.h"
#include "../display_manager.h"
#include "../exception.h"
#include "../world.h"
#include "../camera.h"
#include "../entity_manager.h"
#include "../texture_manager.h"
#include "../reporter.h"
#include "../matrix.h"
#include "../font.h"

#include <d3dx8.h>
#include <mmsystem.h>
#include "d3d_display_manager.h"

struct BUNGVERTEX
{
	float x, y, z; // position
	float u, v;    // texture coords
};
#define D3DFVF_BUNGVERTEX (D3DFVF_XYZ|D3DFVF_TEX1)

CFont			 def_font;
ID3DXMatrixStack *matrixStack = NULL;

int CD3DDisplayManager::meshs_drawn = 0;
int CD3DDisplayManager::triangles_drawn = 0;

void CD3DDisplayManager::CreateD3D(int n_width, int n_height, bool fullscreen, HWND window)
{
	width = n_width;
	height = n_height;

	// Create the D3D object.
    if( NULL == ( d3d = Direct3DCreate8( D3D_SDK_VERSION ) ) )
        throw CException("D3D creation failed for some weird reason.  Weird weird weird.");

    // Get the current desktop display mode, so we can set up a back
    // buffer of the same format
    D3DDISPLAYMODE d3ddm;
    if( FAILED( d3d->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm ) ) )
        throw CException("Unable to get display adapter mode for D3D!");

    // Set up the structure used to create the D3DDevice. Since we are now
    // using more complex geometry, we will create a device with a zbuffer.
    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp, sizeof(d3dpp) );
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = d3ddm.Format;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	d3dpp.hDeviceWindow = window;

	if(fullscreen)
	{
		d3dpp.BackBufferWidth = width;
		d3dpp.BackBufferHeight = height;
	}

    // Create the D3DDevice
    if( FAILED( d3d->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window,
                                   D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                   &d3dpp, &d3d_device ) ) )
    {
        throw CException("Unable to create D3D device!");
    }
}

// Initialisation
void CD3DDisplayManager::Initialise()
{
    // Turn off D3D lighting
    d3d_device->SetRenderState( D3DRS_LIGHTING, FALSE );

    // Turn on the zbuffer
    d3d_device->SetRenderState( D3DRS_ZENABLE, TRUE );

	// Normal (solid) rendering, not wireframe by default
	SetWireframe(false);

	// Turn off culling
	SetBackfaceCull(true);

	texturingOn = true;

	// Create a vertex buffer for us to use
	HRESULT res = d3d_device->CreateVertexBuffer(
		sizeof(BUNGVERTEX) * 1000, // 1000 vertices limit for our one and only VB for now
		D3DUSAGE_DYNAMIC, // we need dynamic buffers for now
		D3DFVF_BUNGVERTEX,
		D3DPOOL_DEFAULT,
		&vertex_buffer
		);
	
	if(res != D3D_OK)
		throw CException("Error creating vertex buffer.  Damnit.");

	// Create an index buffer for us to use
	res = d3d_device->CreateIndexBuffer(
		1000 * sizeof(int), 
		D3DUSAGE_DYNAMIC, 
		D3DFMT_INDEX32, 
		D3DPOOL_DEFAULT, 
		&index_buffer);

	if(res != D3D_OK)
		throw CException("Error creating index buffer.  Damnit.");

	d3d_device->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR); // D3DTEXF_ANISOTROPIC
	d3d_device->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
	d3d_device->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
		//D3DTEXF_POINT);
		//D3DTEXF_LINEAR);

	def_font.Load("font/Lucida_Console_11");
	
	if(D3D_OK != D3DXCreateMatrixStack(0, &matrixStack))
		throw CException("Bugger, D3D unable to create a matrix stack?");

}

// Texture management
CDisplayManager::ImageType CD3DDisplayManager::RequestSupportedImageType(CDisplayManager::ImageType type)
{
	/*
	enum ImageType {
		R8_G8_B8, // 24 bit RGB
		R8_G8_B8_A8, // 32 bit RGBA
		B8_G8_R8, // 24 bit BGR
		B8_G8_R8_A8, // 32 bit BGRA
		Luminance8 // 8 bit Luminance
	};
	*/
	// RGB -> BGR conversion needed for D3D
	if(type == R8_G8_B8)
		return B8_G8_R8;
	if(type == R8_G8_B8_A8)
		return B8_G8_R8_A8;
	// Everything else should be fine
	return type;
}

int	 CD3DDisplayManager::LoadTexture(void *bytes, CDisplayManager::ImageType type, int width, int height)
{
	LPDIRECT3DTEXTURE8 tex;
	LPDIRECT3DSURFACE8 surface;
	RECT source_rect = { 0, 0, width, height };
	HRESULT res;
	D3DFORMAT format;
	
	switch(type)
	{
	case B8_G8_R8:		format = D3DFMT_R8G8B8; break;
	case B8_G8_R8_A8:	format = D3DFMT_A8R8G8B8; break;
	case Luminance8:	format = D3DFMT_L8; break;
	default:	throw CException("Unsupported image format in CD3DDisplayManager::LoadTexture.");
	};

	res = D3DXCreateTexture(
		d3d_device,
		width,
		height,
		0, // Mipmap levels -- 0 is default or something like that
		0, // Render target
		format,
		D3DPOOL_MANAGED,
		&tex);

	if(res != D3D_OK)
	{
		switch(res)
		{
		case D3DERR_NOTAVAILABLE : throw CException("D3D texture create error: D3DERR_NOTAVAILABLE.");
		case D3DERR_OUTOFVIDEOMEMORY :throw CException("D3D texture create error: D3DERR_OUTOFVIDEOMEMORY.");
		case D3DERR_INVALIDCALL :throw CException("D3D texture create error: D3DERR_OUTOFVIDEOMEMORY.");
		case E_OUTOFMEMORY :throw CException("D3D texture create error: D3DERR_OUTOFVIDEOMEMORY.");
		default: throw CException("Unknown error when D3D creating texture.");
		};
	}

	if( tex->GetSurfaceLevel(0, &surface) != D3D_OK )
		throw CException("CD3DDisplayManager::LoadTexture : Unable to get surface level 0 for some reason...");

	res = D3DXLoadSurfaceFromMemory(
		surface,
		NULL,
		NULL,
		bytes,
		D3DFMT_R8G8B8,
		width * 3,
		NULL,
		&source_rect,
		D3DX_DEFAULT,//D3DX_FILTER_NONE,
		0
		);

	if(res != D3D_OK)
	{
		switch(res)
		{
		case D3DERR_INVALIDCALL : throw CException("D3DXLoadSurfaceFromMemory: D3DERR_INVALIDCALL");
		case D3DXERR_INVALIDDATA :throw CException("D3DXLoadSurfaceFromMemory: D3DXERR_INVALIDDATA");
		default: throw CException("D3DXLoadSurfaceFromMemory: Unknown error");
		};
	}

	// Create mipmaps!
	res = D3DXFilterTexture(tex,
		NULL,
		0,
		D3DX_FILTER_LINEAR); // D3DX_FILTER_BOX D3DX_FILTER_TRIANGLE D3DX_FILTER_POINT 

	if(res != D3D_OK)
	{
		switch(res)
		{
		case D3DERR_INVALIDCALL: throw CException("D3DXFilterTexture: D3DERR_INVALIDCALL");
		case D3DXERR_INVALIDDATA: throw CException("D3DXFilterTexture: D3DXERR_INVALIDDATA");
		default: throw CException("D3DXFilterTexture: Unknown error");
		};
	}

	return (int)tex; // haX0r really, but oh well... it works
}

void CD3DDisplayManager::UnloadTexture(string texName)
{
}

void CD3DDisplayManager::UnloadTexture(CTexture *tex)
{
}
	
	// Primitive drawing
void CD3DDisplayManager::DrawSphere(Vector3f offset, float width)
{
}

void CD3DDisplayManager::DrawBox(Vector3f offset, float width)
{
}

void CD3DDisplayManager::DrawBox(Vector3f offset, Vector3f dims)
{
}
	
	// TEMPORARY
void CD3DDisplayManager::DrawActor(CActor *act)
{
}

	// New stuff:
void CD3DDisplayManager::BeginFrame2()
{
	// Clear the backbuffer and the zbuffer
    d3d_device->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
                         D3DCOLOR_XRGB(12,12,43), 1.0f, 0 );

    // Begin the scene
    d3d_device->BeginScene();

	COctree::nodes_drawn = 0;
	triangles_drawn = 0;
	meshs_drawn = 0;
}

void CD3DDisplayManager::EndFrame2()
{
	// End the scene
    d3d_device->EndScene();

    // Present the backbuffer contents to the display
    d3d_device->Present( NULL, NULL, NULL, NULL );
}

void CD3DDisplayManager::SetCameraTransform(CCamera &cam)
{
   // For our world matrix, we will just leave it as the identity
    D3DXMATRIX matWorld;
    D3DXMatrixIdentity( &matWorld );
    d3d_device->SetTransform( D3DTS_WORLD, &matWorld );

    D3DXMATRIX matView;
	D3DXMATRIX xRot, yRot, rot, trans;

	D3DXMatrixRotationX(&xRot, -cam.GetBearing().x / 180.0f * D3DX_PI);
	D3DXMatrixRotationY(&yRot, cam.GetBearing().y / 180.0f * D3DX_PI);
	D3DXMatrixMultiply(&rot, &yRot, &xRot);

	D3DXMatrixTranslation(&trans, 
						  -cam.GetPosition().x, 
						  -cam.GetPosition().y, 
						  cam.GetPosition().z); // Z reversed for D3D coord system
	D3DXMatrixMultiply(&matView, &trans, &rot);

	matView._31 = -matView._31;
	matView._32 = -matView._32;
	matView._33 = -matView._33;
	matView._34 = -matView._34;

    d3d_device->SetTransform( D3DTS_VIEW, &matView );

    // For the projection matrix, we set up a perspective transform (which
    // transforms geometry from 3D view space to 2D viewport space, with
    // a perspective divide making objects smaller in the distance). To build
    // a perpsective transform, we need the field of view (1/4 pi is common),
    // the aspect ratio, and the near and far clipping planes (which define at
    // what distances geometry should be no longer be rendered).
	// BuNg uses 60 degrees in the OGL renderer = 1/3 PI so we will use that
    D3DXMATRIX matProj;
    D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/3.0f, (float)width / (float)height, 0.1f, 100.0f );
    d3d_device->SetTransform( D3DTS_PROJECTION, &matProj );

	// Calculate the frustum
	D3DXMATRIX frus;
	D3DXMatrixMultiply(&frus, &matView, &matProj);

	cam.CalculateFrustumRowMajor(frus.m);

}

void CD3DDisplayManager::BindTexture(CTexture *tex)
{
	if(tex)
	{
		if(!texturingOn) 
		{
			d3d_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			texturingOn = true;

			d3d_device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);	//alpha from texture
		}

		d3d_device->SetTexture( 0, (IDirect3DBaseTexture8 *)tex->id );
	}
	else
	{
		d3d_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
		d3d_device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);	//alpha from diffuse colour
		texturingOn = false;
	}

}

void CD3DDisplayManager::SetColour(float red, float green, float blue, float alpha/* = 1.0f*/)
{
	d3d_device->SetRenderState(D3DRS_AMBIENT,RGB(red,green,blue));
}

void CD3DDisplayManager::SetBlend(bool on)
{
	d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE, on);
}

BlendMode CD3DDisplayManager::GetBlendMode()
{
	return blendMode;
}

void CD3DDisplayManager::SetBlendMode(BlendMode bm)
{
	blendMode = bm;

	switch(bm) {
		case BlendMode::Multiply:
			d3d_device->SetRenderState(D3DRS_SRCBLEND,			D3DBLEND_ONE);
			d3d_device->SetRenderState(D3DRS_DESTBLEND,			D3DBLEND_ZERO);
			break;
		case BlendMode::Add:
			d3d_device->SetRenderState(D3DRS_SRCBLEND,			D3DBLEND_ONE);
			d3d_device->SetRenderState(D3DRS_DESTBLEND,			D3DBLEND_ONE);
			break;
		case BlendMode::Transparent:
			d3d_device->SetRenderState(D3DRS_SRCBLEND,			D3DBLEND_SRCALPHA);
			d3d_device->SetRenderState(D3DRS_DESTBLEND,			D3DBLEND_INVSRCALPHA);
			break;
	};
}

void CD3DDisplayManager::Begin2D()
{
	D3DXMATRIX ortho, view, proj, iden;

	d3d_device->GetTransform(D3DTS_VIEW, &view);
	d3d_device->GetTransform(D3DTS_PROJECTION, &proj);

	matrixStack->LoadMatrix(&view);
	matrixStack->Push();
	matrixStack->LoadMatrix(&proj);

	D3DXMatrixOrthoOffCenterLH(
		&ortho,
		0.0f,
		(float)GetWidth(),
		(float)GetHeight(),
		0.0f,
		0.0f,
		1.0f); 
	d3d_device->SetTransform( D3DTS_VIEW, &ortho );

	D3DXMatrixIdentity(&iden);
	d3d_device->SetTransform( D3DTS_PROJECTION, &iden );

	d3d_device->SetRenderState( D3DRS_ZENABLE, FALSE );
}

void CD3DDisplayManager::DrawString2(int x, int y, const string &str)
{
	DrawString3(def_font, x, y, str);
}

void CD3DDisplayManager::Draw2DQuad(int left, int top, int right, int bottom)
{
	bool bc = GetBackfaceCull();
	SetBackfaceCull(false);

	// Fill the vertex buffer. We are setting the tu and tv texture
    // coordinates, which range from 0.0 to 1.0
    BUNGVERTEX* pVertices;
    if( FAILED( vertex_buffer->Lock( 0, 0, (BYTE**)&pVertices, 0 ) ) )
        throw CException("Bitch");

    pVertices[0].x = (float)left;
	pVertices[0].y = (float)top;
	pVertices[0].z = 0.0f;
    pVertices[0].u = 0.0f;
    pVertices[0].v = 0.0f;

	pVertices[1].x = (float)left;
	pVertices[1].y = (float)bottom;
	pVertices[1].z = 0.0f;
    pVertices[1].u = 0.0f;
    pVertices[1].v = 1.0f;

	pVertices[2].x = (float)right;
	pVertices[2].y = (float)bottom;
	pVertices[2].z = 0.0f;
    pVertices[2].u = 1.0f;
    pVertices[2].v = 1.0f;

	pVertices[3].x = (float)right;
	pVertices[3].y = (float)top;
	pVertices[3].z = 0.0f;
    pVertices[3].u = 1.0f;
    pVertices[3].v = 0.0f;

    vertex_buffer->Unlock();

	// Render the vertex buffer contents
    d3d_device->SetStreamSource( 0, vertex_buffer, sizeof(BUNGVERTEX) );
    d3d_device->SetVertexShader( D3DFVF_BUNGVERTEX );
    d3d_device->DrawPrimitive( D3DPT_TRIANGLEFAN, 0, 2 );

	SetBackfaceCull(bc);
}

	/** Restored the render state (generally back to 3D mode). */
void CD3DDisplayManager::End2D()
{
	d3d_device->SetTransform( D3DTS_PROJECTION, matrixStack->GetTop() );
	matrixStack->Pop();

	d3d_device->SetTransform( D3DTS_VIEW, matrixStack->GetTop() );

	d3d_device->SetRenderState( D3DRS_ZENABLE, TRUE );
}
	
void CD3DDisplayManager::DrawIndexedTriangles2(float *vertices, float *tex_coords, 
		unsigned int *indices, int num_triangles)
{
	unsigned int numVerts = num_triangles * 3;
	
	UINT *pIndices;
	BUNGVERTEX* pVertices;

	if( FAILED( index_buffer->Lock( 0, 0, (BYTE **)&pIndices, 0) ) )
		throw CException("Index buffer lock failed");
	if( FAILED( vertex_buffer->Lock( 0, 0, (BYTE**)&pVertices, 0 ) ) )
        throw CException("Bitch");

	for(DWORD i = 0; i < numVerts; i++) {
		pIndices[i] = indices[i];

		// The below is pretty hax, but oh well
		pVertices[pIndices[i]].x = vertices[pIndices[i] * 3];
		pVertices[pIndices[i]].y = vertices[pIndices[i] * 3 + 1];
		pVertices[pIndices[i]].z = vertices[pIndices[i] * 3 + 2];
        pVertices[pIndices[i]].u = tex_coords[pIndices[i] * 2];
        pVertices[pIndices[i]].v = tex_coords[pIndices[i] * 2 + 1];
	}

	index_buffer->Unlock();
	vertex_buffer->Unlock();


	d3d_device->SetStreamSource( 0, vertex_buffer, sizeof(BUNGVERTEX) );
    
	d3d_device->SetIndices(	index_buffer, 0 );

	d3d_device->SetVertexShader( D3DFVF_BUNGVERTEX );

	d3d_device->DrawIndexedPrimitive(
		D3DPT_TRIANGLELIST,
		0,
		num_triangles * 3,
		0,
		num_triangles
		);

	meshs_drawn++;
	triangles_drawn += num_triangles;
}

void CD3DDisplayManager::DrawTriangles2(float *vertices, float *tex_coords, int num_triangles)
{
	
}

void CD3DDisplayManager::DrawTriangleFan2(float *vertices, float *tex_coords, int num_triangles)
{
	unsigned int numVerts = num_triangles + 2;
	// Fill the vertex buffer. We are setting the tu and tv texture
    // coordinates, which range from 0.0 to 1.0
    BUNGVERTEX* pVertices;
    if( FAILED( vertex_buffer->Lock( 0, 0, (BYTE**)&pVertices, 0 ) ) )
        throw CException("Bitch");
    for( DWORD i=0; i < numVerts; i++ )
    {
        pVertices[i].x = vertices[i * 3];
		pVertices[i].y = vertices[i * 3 + 1];
		pVertices[i].z = vertices[i * 3 + 2];
        pVertices[i].u = tex_coords[i * 2];
        pVertices[i].v = tex_coords[i * 2 + 1];
    }
    vertex_buffer->Unlock();

	// Render the vertex buffer contents
    d3d_device->SetStreamSource( 0, vertex_buffer, sizeof(BUNGVERTEX) );
    d3d_device->SetVertexShader( D3DFVF_BUNGVERTEX );
    d3d_device->DrawPrimitive( D3DPT_TRIANGLEFAN, 0, num_triangles );

	meshs_drawn++;
	triangles_drawn += num_triangles;
}

void CD3DDisplayManager::DrawTriangleStrip(float *vertices, float *tex_coords, int num_triangles)
{
	unsigned int numVerts = num_triangles + 2;
	// Fill the vertex buffer. We are setting the tu and tv texture
    // coordinates, which range from 0.0 to 1.0
    BUNGVERTEX* pVertices;
    if( FAILED( vertex_buffer->Lock( 0, 0, (BYTE**)&pVertices, 0 ) ) )
        throw CException("Bitch");
    for( DWORD i=0; i < numVerts; i++ )
    {
        pVertices[i].x = vertices[i * 3];
		pVertices[i].y = vertices[i * 3 + 1];
		pVertices[i].z = vertices[i * 3 + 2];
        pVertices[i].u = tex_coords[i * 2];
        pVertices[i].v = tex_coords[i * 2 + 1];
    }
    vertex_buffer->Unlock();

	// Render the vertex buffer contents
    d3d_device->SetStreamSource( 0, vertex_buffer, sizeof(BUNGVERTEX) );
    d3d_device->SetVertexShader( D3DFVF_BUNGVERTEX );
    d3d_device->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, num_triangles );

	meshs_drawn++;
	triangles_drawn += num_triangles;
}

void CD3DDisplayManager::DrawString3(const CFont &fnt, const int x, const int y, const string &str)
{
	int len = str.length(), i, top, left, count = x, height, width, bottom, right;
	float topf, leftf, rightf, bottomf;
	bool wf = GetWireframe(), bc = GetBackfaceCull();
	BlendMode bm = GetBlendMode();

	SetWireframe(false);
	SetBackfaceCull(false);

	SetBlendMode(BlendMode::Add);
	BindTexture(fnt.tex);
	SetBlend(true);

	BUNGVERTEX* pVertices;
    if( FAILED( vertex_buffer->Lock( 0, 0, (BYTE**)&pVertices, 0 ) ) )
        throw CException("Bitch");

	for(i = 0; i < len; i++)
	{
		char c = str[i];
		
		// We don't support ASCII characters out of this range. Remember the infamous 'corrupted drawing'
		// '\n' bug?
		// - Sam
		if(c < '!' || c > '~')
			c = ' ';

		fnt.GetGlyph(c, top, left, width, height);

		topf	= (float)top / (float)fnt.tex->height;
		leftf	= (float)left / (float)fnt.tex->width;
		bottomf	= (float)(top + height) / ((float)fnt.tex->height - 1);
		rightf	= (float)(left + width) / ((float)fnt.tex->width - 1);

		bottom = y + height;
		right = count + width;

		/*
		tcs[0]		= leftf;	tcs[1]		= topf;
		verts[0]	= count;	verts[1]	= y;
		
		tcs[2]		= rightf;	tcs[3]		= topf;
		verts[2]	= right;	verts[3]	= y;
		
		tcs[4]		= rightf;	tcs[5]		= bottomf;
		verts[4]	= right;	verts[5]	= bottom;

		tcs[6]		= leftf;	tcs[7]		= bottomf;
		verts[6]	= count;	verts[7]	= bottom;
		*/

		// Left bottom
		pVertices[i * 6].x = pVertices[i * 6 + 5].x = (float)count;
		pVertices[i * 6].y = pVertices[i * 6 + 5].y = (float)bottom;
		pVertices[i * 6].z = pVertices[i * 6 + 5].z = 0.0f;
		pVertices[i * 6].u = pVertices[i * 6 + 5].u = leftf;
		pVertices[i * 6].v = pVertices[i * 6 + 5].v = bottomf;
		
		// Left top
		pVertices[i * 6 + 1].x = (float)count;
		pVertices[i * 6 + 1].y = (float)y;
		pVertices[i * 6 + 1].z = 0.0f;
		pVertices[i * 6 + 1].u = leftf;
		pVertices[i * 6 + 1].v = topf;

		// Right top
		pVertices[i * 6 + 2].x = pVertices[i * 6 + 3].x = (float)right;
		pVertices[i * 6 + 2].y = pVertices[i * 6 + 3].y = (float)y;
		pVertices[i * 6 + 2].z = pVertices[i * 6 + 3].z = 0;
		pVertices[i * 6 + 2].u = pVertices[i * 6 + 3].u = rightf;
		pVertices[i * 6 + 2].v = pVertices[i * 6 + 3].v = topf;

		// Right bottom
		pVertices[i * 6 + 4].x = (float)right;
		pVertices[i * 6 + 4].y = (float)bottom;
		pVertices[i * 6 + 4].z = 0.0f;
		pVertices[i * 6 + 4].u = rightf;
		pVertices[i * 6 + 4].v = bottomf;

		count += width;
	}
    vertex_buffer->Unlock();

	// Render the vertex buffer contents
    d3d_device->SetStreamSource( 0, vertex_buffer, sizeof(BUNGVERTEX) );
    d3d_device->SetVertexShader( D3DFVF_BUNGVERTEX );
    d3d_device->DrawPrimitive( D3DPT_TRIANGLELIST, 0, len * 2 );


	SetBlendMode(bm);
	SetBlend(false);
	SetWireframe(wf);
	SetBackfaceCull(bc);
}

	// Other misc functions
	/** Called by the System Driver whenever the user resizes the window. This
	 * func just resets the viewport and matrices for a larger window and updates
	 * the internal width and height. */
void CD3DDisplayManager::WindowResized(int new_width, int new_height)
{
	width = new_width;
	height = new_height;

	// TODO: resize the d3d stuff here like in the GL display manager.
}

int CD3DDisplayManager::GetWidth()
{
	return width;
}

int CD3DDisplayManager::GetHeight()
{
	return height;
}

void CD3DDisplayManager::PushMatrix()
{
}

void CD3DDisplayManager::Translate(Vector3f position)
{
}

void CD3DDisplayManager::PopMatrix()
{
}

void CD3DDisplayManager::Rotate(float amount, Vector3f axis)
{
}

void CD3DDisplayManager::SetWireframe(bool w)
{
	CDisplayManager::SetWireframe(w);

	if(w)
		d3d_device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	else
		d3d_device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
}

void CD3DDisplayManager::SetBackfaceCull(bool b)
{
	CDisplayManager::SetBackfaceCull(b);

	if(b)
		d3d_device->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW ); 
	else
		d3d_device->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE);
}
