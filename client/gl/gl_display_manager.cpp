#include "../stdafx.h"

#include "gl_display_manager.h"

// Windows OpenGL needs windows.h to be included
#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>

#include <math.h>
#include <stdarg.h>

#include "../world.h"
#include "../octree.h"
#include "../camera.h"
#include "../vfs.h"
#include "../system_driver.h"
#include "../matrix.h"
#include "../bezier.h"
#include "../texture_manager.h"
#include "../entity_manager.h"
#include "../exception.h"
#include "../font.h"

#include "../matrix.h"
#include "../quaternion.h"

#include "../md3.h"					// Temporary

// DevIL Image Library used to open textures
#include "../external/DevIL/include/il/il.h"
#include "../external/DevIL/include/il/ilu.h"

#ifdef _WIN32
#pragma comment( lib, "opengl32.lib" )
#pragma comment( lib, "glu32.lib" )	
#pragma comment( lib, "glaux.lib" )
#endif

// TEMPORARY
#include "../actor.h"

int  adjust = 5;					// speed doodad
CTexture *font_tex = NULL;
char *buf = NULL;

int CGLDisplayManager::triangles_drawn = 0;
int CGLDisplayManager::meshs_drawn = 0;

CFont			default_font;

void CGLDisplayManager::Initialise()		
{
	// Start Of User Initialization

	glClearColor (0.0f, 0.0f, 0.0f, 0.5f);					

	glClearDepth (1.0f); // Depth Buffer Setup
	glDepthFunc (GL_LEQUAL); // The Type Of Depth Testing (Less Or Equal)
	glEnable (GL_DEPTH_TEST); // Enable Depth Testing
	glShadeModel (GL_SMOOTH); // Select Smooth Shading
	glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Set Perspective Calculations To Most Accurate

	GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat mat_shininess[] = { 50.0f };
	GLfloat mat_ambient[] = { 0.7f, 0.7f, 0.7f, 1.0f };
    GLfloat light_position[] = { +16.7f,+2.24f,-42.23f, 1.0f };

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	//glEnable(GL_LIGHTING);
	//glEnable(GL_LIGHT0);

	glEnable(GL_TEXTURE_2D);
	

	// Wireframe? 
//	if(wireframe)
//		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glCullFace(GL_BACK);
	glFrontFace(GL_CW);

	// alpha blending!
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	//glAlphaFunc (GL_GREATER, 0.3f );
	//glEnable ( GL_ALPHA_TEST );

	//glEnable(GL_AUTO_NORMAL);

	// XXX: do we need the following line nowadays??
	//font_tex = CTextureManager::tm.LoadTexture("data/terminal-18.png");

	for(int i = 0; i < 20000; i++)
		tri_indices[i] = i;

	default_font.Load(
			//"font/Times_New_Roman_12");
			//"font/Arial_12");
			//"font/Verdana_Ref_11");
			"font/Courier_New_11");
			//"font/Lucida_Console_11");
	//default_font.SetBold(true);

	Log("OpenGL display driver. Extensions found: %s\n",
			glGetString(GL_EXTENSIONS));
	int tex_units;
	glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &tex_units);
	Log("\tTexture units: %d\n", tex_units);
}

void CGLDisplayManager::DrawSphere(Vector3f offset, float width)
{
	BindTexture(NULL);
	
	SetBlend(true);
	SetBlendMode(Transparent);
	
	glPushMatrix();
	glTranslatef(offset.x, offset.y, offset.z);
//	glBegin (GL_LINES);

	GLUquadricObj* qobj = gluNewQuadric();

	gluSphere(qobj, width, 10, 10);

	gluDeleteQuadric(qobj);
	
//	glEnd();
	glPopMatrix();

	SetBlend(false);
	
}

void CGLDisplayManager::DrawBox(Vector3f offset, float width)
{
	DrawBox(offset, Vector3f(width, width, width));
}

// dims defines the vector relative to the center of the cubiod that defines 
// the positive x y and z corner
void CGLDisplayManager::DrawBox(Vector3f offset, Vector3f dims)
{
	glPushMatrix();
	glTranslatef(offset.x, offset.y, offset.z);
	glBegin (GL_LINES);							
		glColor3f (1.f, 1.f, 1.f);	
		// bottom face
		glVertex3f( dims.x, -dims.y, -dims.z); glVertex3f( -dims.x, -dims.y, -dims.z);
		glVertex3f( -dims.x,-dims.y, -dims.z); glVertex3f( -dims.x,-dims.y, dims.z);
		glVertex3f( -dims.x, -dims.y, dims.z); glVertex3f( dims.x, -dims.y, dims.z);
		glVertex3f( dims.x, -dims.y, dims.z); glVertex3f( dims.x, -dims.y, -dims.z);
		
		// top face
		glVertex3f( dims.x, dims.y, -dims.z); glVertex3f( -dims.x, dims.y, -dims.z);
		glVertex3f( -dims.x, dims.y, -dims.z); glVertex3f( -dims.x, dims.y, dims.z);
		glVertex3f( -dims.x, dims.y, dims.z); glVertex3f( dims.x, dims.y, dims.z);
		glVertex3f( dims.x, dims.y, dims.z); glVertex3f( dims.x, dims.y, -dims.z);

		// connecting lines
		glVertex3f( dims.x, -dims.y, -dims.z); glVertex3f( dims.x, dims.y, -dims.z);
		glVertex3f( -dims.x,-dims.y, -dims.z);	glVertex3f( -dims.x, dims.y, -dims.z);
		glVertex3f( -dims.x, -dims.y, dims.z); glVertex3f( -dims.x, dims.y, dims.z);
		glVertex3f( dims.x, -dims.y, dims.z); glVertex3f( dims.x, dims.y, dims.z);
	glEnd();
	glPopMatrix();
}

void CGLDisplayManager::DrawActor(CActor *act)
{
//	throw CException("Deprecated DrawActor called in CGLDisplayManager");
	//*
	glPushMatrix();
	glTranslatef(act->GetPosition().x, 
				 act->GetPosition().y - 0.02f, 
				 act->GetPosition().z + 0.05f);

	
	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
	glRotatef(90.0f, 0.0f, 1.0f, 0.0f);

	glRotatef(act->GetBearing().y, 0.0f,  1.0f, 0.0f);
	glRotatef(act->GetBearing().x, 1.0f, 0.0f,  0.0f);
	glRotatef(act->GetBearing().z, 0.0f,  0.0f,  1.0f);
	act->m_model->Draw();
	glPopMatrix(); 
	//*/

	/*
	for(int i = 0; i <= act->num_hits; i++)
	{
		glColor3f((float)i / 3.0f, 0, 0);
		glBegin(GL_LINES);
		CReporter::Report(CReporter::R_DEBUG, "from: %s to %s num_hits=%d", act->orig[i].toString().c_str(),
			act->hits[i].toString().c_str(), act->num_hits);

			glVertex3f( act->orig[i].x, act->orig[i].y, act->orig[i].z );
			glVertex3f( act->orig[i].x + act->hits[i].x, act->orig[i].y + act->hits[i].y, act->orig[i].z + act->hits[i].z );

		glEnd();

		glColor3f(0, 1, 0);
		glBegin(GL_LINES);
			//glVertex3f( act->orig[0].x, act->orig[0].y, act->orig[0].z );
			//glPushMatrix();
			glVertex3f(act->orig[0].x, act->orig[0].y, act->orig[0].z );
			glVertex3f(act->orig[0].x + 0, act->orig[0].y + 1, act->orig[0].z + 0 );
			//glPopMatrix();
		glEnd();
	}
	*/
}

void CGLDisplayManager::DrawTriangleFan(CTriangleFan *fan)
{
	int num_triangles = fan->GetNumberTriangles();
	
	glVertexPointer(3, GL_FLOAT, 0, &fan->vertices[0]);
	glTexCoordPointer(2, GL_FLOAT, 0, &fan->texCoords[0]);
	
	glDrawArrays(GL_TRIANGLE_FAN, 0, num_triangles + 2);
	
	triangles_drawn += num_triangles;
	meshs_drawn++;
}

// Texture management
CDisplayManager::ImageType CGLDisplayManager::RequestSupportedImageType(CDisplayManager::ImageType type)
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
	// BGR -> RGB conversion needed for OpenGL (well, ogl actually has BGR_EXT but we might as well use RGB)
	if(type == B8_G8_R8)
		return R8_G8_B8;
	if(type == B8_G8_R8_A8)
		return R8_G8_B8_A8;
	// Everything else should be fine
	return type;
}

int CGLDisplayManager::LoadTexture(void *bytes, CDisplayManager::ImageType type, int width, int height)
{
	int glId = -1, glType = -1, glByteType = -1, glComponents = 3;
	// Note that width and height should be powers of two!

	switch(type)
	{
	case R8_G8_B8:		glType = GL_RGB;  glByteType = GL_UNSIGNED_BYTE; glComponents = 3; break;
	case R8_G8_B8_A8:	glType = GL_RGBA; glByteType = GL_UNSIGNED_BYTE; glComponents = 4; break;
	case Luminance8:	glType = GL_LUMINANCE; glByteType = GL_UNSIGNED_BYTE; glComponents = 1; break;
	default: throw CException("Unsupported image byte type in CGLDisplayManager::LoadTexture!");
	}

	// On to the OpenGL: Generate a new texture ID
	glGenTextures(1, (GLuint *)&glId);
	// Bind the new ID
	glBindTexture(GL_TEXTURE_2D, glId);
	// Send the texture to the video card
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
	// Go the mipmaps!
	gluBuild2DMipmaps(
		GL_TEXTURE_2D,
		glComponents,
		width,
		height,
		glType,
		glByteType,
		bytes
	);
	

	// We better check if opengl had a problem with loading the
	// texture or not.
	int err = glGetError();
	if(err != GL_NO_ERROR)
	{
		switch(err) {
		case GL_NO_ERROR : throw CException("OpenGL error in LoadTexture: No error has been "
				"recorded. The value of this symbolic constant is guaranteed to be zero.");
		case GL_INVALID_ENUM : throw CException("OpenGL error in LoadTexture: An unacceptable "
				"value is specified for an enumerated argument. The offending function is "
				"ignored, having no side effect other than to set the error flag.");
		case GL_INVALID_VALUE : throw CException("OpenGL error in LoadTexture: A numeric "
				"argument is out of range. The offending function is ignored, having no "
				"side effect other than to set the error flag.");
		case GL_INVALID_OPERATION : throw CException("OpenGL error in LoadTexture: The "
				"specified operation is not allowed in the current state. The offending "
				"function is ignored, having no side effect other than to set the error flag.");
		case GL_STACK_OVERFLOW : throw CException("OpenGL error in LoadTexture: This function "
				"would cause a stack overflow. The offending function is ignored, having no "
				"side effect other than to set the error flag.");
		case GL_STACK_UNDERFLOW : throw CException("OpenGL error in LoadTexture: This function "
				"would cause a stack underflow. The offending function is ignored, having no "
				"side effect other than to set the error flag.");
		case GL_OUT_OF_MEMORY : throw CException("OpenGL error in LoadTexture: There is not "
				"enough memory left to execute the function. The state of OpenGL is undefined, "
				"except for the state of the error flags, after this error is recorded.");
		};
		
		//throw CException("OpenGL failed in glTexImage2D/gluBuild2DMipmaps.");
	}

	return glId;
}

void CGLDisplayManager::UnloadTexture(CTexture *tex)
{
	if(!tex)
		return;

	glDeleteTextures(1, (const unsigned int *)&tex->id);
}

void CGLDisplayManager::UnloadTexture(string texName)
{
	UnloadTexture(CTextureManager::tm.FindTexture(texName));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW INTERFACE IMPLEMENTED BELOW:

void CGLDisplayManager::BindTexture(CTexture *tex)
{
	if(tex == NULL) {
		if(textureOn) {
			textureOn = false;
			glDisable(GL_TEXTURE_2D);
		}
		return;
	}
	else if(!textureOn) {
		textureOn = true;
		glEnable(GL_TEXTURE_2D);
	}
	glBindTexture(GL_TEXTURE_2D, tex->id);
}

void CGLDisplayManager::SetColour(float red, float green, float blue, float alpha/* = 1.0f*/)
{
	glColor4f(red, green, blue, alpha);
}

void CGLDisplayManager::SetBlend(bool on)
{
	if(on) {
		if(!blendOn) {
			glEnable(GL_BLEND);
			blendOn = true;
		}
	} else {
		if(blendOn) {
			glDisable(GL_BLEND);
			blendOn = false;
		}
	}
}

void CGLDisplayManager::SetBlendMode(BlendMode bm)
{
	blendMode = bm;

	switch(bm) {
		case CDisplayManager::Multiply:
			glBlendFunc(GL_ONE, GL_ZERO);
			break;
		case CDisplayManager::Add:
			glBlendFunc(GL_ONE, GL_ONE);
			break;
		case CDisplayManager::Transparent:
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
		case CDisplayManager::Transparent2:
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			break;
	};
}

BlendMode CGLDisplayManager::GetBlendMode()
{
	return blendMode;
}

void CGLDisplayManager::BeginFrame2()
{
	// Clear Screen And Depth Buffer
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	COctree::nodes_drawn = 0;
	CGLDisplayManager::triangles_drawn = 0;
	CGLDisplayManager::meshs_drawn = 0;
}

void CGLDisplayManager::SetCameraTransform(CCamera &cam)
{
	// Reset The Modelview Matrix
	glLoadIdentity ();

	// Move and rotate to the camera position
	//*
	glRotatef(cam.GetBearing().x, 1.0f, 0.0f, 0.0f);
	glRotatef(cam.GetBearing().y, 0.0f, -1.0f, 0.0f);
	glRotatef(cam.GetBearing().z, 0.0f, 0.0f, 1.0f);
	
    glTranslatef(-cam.GetPosition().x, 
				 -cam.GetPosition().y, 
				 -cam.GetPosition().z);
				 
	// Calculate frustum via combining projection and modelview matrices
	// and extracting planes
	CMatrix4f proj, modl, f;

	glGetFloatv( GL_PROJECTION_MATRIX, proj.GetData() );
	glGetFloatv( GL_MODELVIEW_MATRIX, modl.GetData() );

	f.Multiply(modl, proj);

	cam.CalculateFrustumColumnMajor(f);
}

void CGLDisplayManager::EndFrame2()
{
	// Flush The GL Rendering Pipeline
	glFlush();

	world.sys->ForceWindowDraw();
}

void CGLDisplayManager::Begin2D()
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	
	//GLint vPort[4];
    //glGetIntegerv(GL_VIEWPORT, vPort);
    
	// Note: we want screen co-ordinates, so the top left should be 0,0
	glOrtho(0,		// left
		GetWidth(),	// right 
		GetHeight(),// bottom
		0,			// top
		-1,			// znear
		1);			// zfar

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glPushAttrib(GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
}

void CGLDisplayManager::DrawString3(const CFont &fnt, const int x, const int y, const string &str)
{
	int len = (int)str.length(), i, top, left, count = x, height, width, bottom, right;
	float topf, leftf, rightf, bottomf;
	bool wf = GetWireframe();
	float tcs[8];
	int verts[8];
	BlendMode bm = GetBlendMode();

	if(wf)
		SetWireframe(false);

	BindTexture(fnt.tex);
	SetBlend(true);
	SetBlendMode(CDisplayManager::Add);
	
	glVertexPointer(2, GL_INT, 0, verts);
	glTexCoordPointer(2, GL_FLOAT, 0, tcs);

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
		bottomf	= (float)(top + height) / (float)fnt.tex->height;
		rightf	= (float)(left + width) / (float)fnt.tex->width;

		bottom = y + height;
		right = count + width;

		tcs[0]		= leftf;	tcs[1]		= topf;
		verts[0]	= count;	verts[1]	= y;
		
		tcs[2]		= rightf;	tcs[3]		= topf;
		verts[2]	= right;	verts[3]	= y;
		
		tcs[4]		= rightf;	tcs[5]		= bottomf;
		verts[4]	= right;	verts[5]	= bottom;

		tcs[6]		= leftf;	tcs[7]		= bottomf;
		verts[6]	= count;	verts[7]	= bottom;
		
		// Using glDrawElements is MUCH faster than glVertex2i and glTexCoord2f.
		//glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, tri_indices);
		glDrawArrays(GL_QUADS, 0, 4);

		count += width;
	}


	SetBlendMode(bm);
	SetBlend(false);
	if(wf)
		SetWireframe(true);
}

void CGLDisplayManager::PushMatrix() 
{	
	glPushMatrix(); 
}

void CGLDisplayManager::Translate(Vector3f position) 
{ 
	glTranslatef(position.x, position.y, position.z); 
}

void CGLDisplayManager::MultMatrix(CMatrix4f &mat) 
{ 
	glMultMatrixf( mat.GetData() );
}

void CGLDisplayManager::PopMatrix() 
{ 
	glPopMatrix(); 
}

void CGLDisplayManager::Rotate(float amount, Vector3f axis) 
{ 
	glRotatef(amount, axis.x, axis.y, axis.z); 
}

void CGLDisplayManager::DrawString2(int x, int y, const string &str)
{
	DrawString3(default_font, x, y, str);
	return;

	int len = (int)str.length();
	float char_width = 10, char_height = 22, char_spacing = 0;
	float tx = (float)char_width / 256.0f, ty = (float)char_height / 128.0f, scale = 0.8f;
	bool wf = GetWireframe();

	if(wf)
		SetWireframe(false);

	char_width *= scale;
	char_height *= scale;

	BindTexture(font_tex);

	SetColour(1.0f, 1.0f, 1.0f, 1.0f);
	SetBlend(true);

	for(int i = 0; i < len; i++)
	{
		glBegin(GL_QUADS);
			int ch_x, ch_y;

			ch_x = str[i] - ' ';
			ch_y = ch_x / 24;
			ch_x %= 24;

			glTexCoord2f(ch_x * tx, ch_y * ty);		
			glVertex2i(x + 0, y + 0); 
			
			glTexCoord2f(ch_x * tx, ch_y * ty + (ty * 0.9f));		
			glVertex2i(x + 0, (int)(y + char_height));
			
			glTexCoord2f(ch_x * tx + tx, ch_y * ty + (ty * 0.9f));	
			glVertex2i((int)(x + char_width), (int)(y + char_height));
			
			glTexCoord2f(ch_x * tx + tx, ch_y * ty);	
			glVertex2i((int)(x + char_width), y);

			x += (int)(char_width + char_spacing);

		glEnd();
	}

	SetBlend(false);

	if(wf)
		SetWireframe(true);
}

void CGLDisplayManager::Draw2DQuad(int left, int top, int right, int bottom)
{
	//* This method should be faster than the one below it:
	int vertices[] = { left, top, left, bottom, right, bottom, right, top };
	static int tex_coords[] = { 0, 0, 0, 1, 1, 1, 1, 0 };

	glVertexPointer(2, GL_INT, 0, vertices);
	glTexCoordPointer(2, GL_INT, 0, tex_coords);
	glDrawArrays(GL_QUADS, 0, 4);
}

void CGLDisplayManager::End2D()
{
	glPopAttrib();
	
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void CGLDisplayManager::DrawIndexedTriangles2(float *vertices, float *tex_coords, 
											  unsigned int *indices, int num_triangles)
{
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glTexCoordPointer(2, GL_FLOAT, 0, tex_coords);

	glDrawElements(GL_TRIANGLES, num_triangles * 3, GL_UNSIGNED_INT, indices);
	
	triangles_drawn += num_triangles;
	meshs_drawn++;
}

void CGLDisplayManager::DrawTriangles2(float *vertices, float *tex_coords, int num_triangles)
{
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glTexCoordPointer(2, GL_FLOAT, 0, tex_coords);

	glDrawArrays(GL_TRIANGLES, 0, num_triangles * 3);

	triangles_drawn += num_triangles;
	meshs_drawn++;
}

void CGLDisplayManager::DrawTriangles2(float *vertices, float *tex_coords, byte *colours, 
		int num_triangles)
{
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glTexCoordPointer(2, GL_FLOAT, 0, tex_coords);
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, colours);

	glEnableClientState(GL_COLOR_ARRAY);
	glDrawArrays(GL_TRIANGLES, 0, num_triangles * 3);
	glDisableClientState(GL_COLOR_ARRAY);

	triangles_drawn += num_triangles;
	meshs_drawn++;
}

void CGLDisplayManager::DrawTriangleFan2(float *vertices, float *tex_coords, int num_triangles)
{
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glTexCoordPointer(2, GL_FLOAT, 0, tex_coords);

	glDrawArrays(GL_TRIANGLE_FAN, 0, num_triangles + 2);
	
	triangles_drawn += num_triangles;
	meshs_drawn++;
}

void CGLDisplayManager::DrawTriangleStrip(float *vertices, float *tex_coords, int num_triangles)
{
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glTexCoordPointer(2, GL_FLOAT, 0, tex_coords);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, num_triangles + 2);
	
	triangles_drawn += num_triangles;
	meshs_drawn++;
}

void CGLDisplayManager::SetDepthTest(bool on)
{
    if(on)
	glEnable(GL_DEPTH_TEST);
    else
	glDisable(GL_DEPTH_TEST);
}

void CGLDisplayManager::WindowResized(int new_width, int new_height)
{
	// Reset The Current Viewport
	glViewport(0, 0, (GLsizei)(new_width), (GLsizei)(new_height));
	// Select The Projection Matrix
	glMatrixMode(GL_PROJECTION);										
	// Reset The Projection Matrix
	glLoadIdentity();													
	// Calculate The Aspect Ratio Of The Window
	gluPerspective(60.0f, (GLfloat)(width)/(GLfloat)(height), 0.1f, 100.0f);		
	// Select The Modelview Matrix
	glMatrixMode(GL_MODELVIEW);
	// Reset The Modelview Matrix
	glLoadIdentity();

	width = new_width;
	height = new_height;
}

void CGLDisplayManager::SetWireframe(bool w)
{
	CDisplayManager::SetWireframe(w);

	if(w)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void CGLDisplayManager::SetBackfaceCull(bool b)
{
	CDisplayManager::SetBackfaceCull(b);

	if(b)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);
}

void CGLDisplayManager::Screenshot(vector<byte> &buf, uint32 &w, uint32 &h)
{
    int ViewPort[4];
    
    glGetIntegerv(GL_VIEWPORT, ViewPort);

    w = ViewPort[2];
    h = ViewPort[3];
    
    buf.resize(w * h * 3);

    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, buf.begin());
}
