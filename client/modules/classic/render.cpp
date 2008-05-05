#include "module_interface.h"
#include "classic.h"

/*********************************************
	Draws a plane. It used to be a slab...
**********************************************/
void ClassicModule::utilSlab(float x, float y, float z){

	x/=2; y/=2; z/=2;
		
	glEnable(GL_TEXTURE_2D);

	glBegin(GL_QUADS);
	// Front Face
		glNormal3f( 0.0f, 0.0f, 1.0f);					// Normal Pointing Towards Viewer
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-x, -y,  z);	// Point 1 (Front)
		glTexCoord2f(1.0f, 0.0f); glVertex3f( x, -y,  z);	// Point 2 (Front)
		glTexCoord2f(1.0f, -1.0f); glVertex3f( x,  y,  z);	// Point 3 (Front)
		glTexCoord2f(0.0f, -1.0f); glVertex3f(-x,  y,  z);	// Point 4 (Front)		
	glEnd();								// Done Drawing Quads

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);
	glColor3f(0.25f, 0.25f, 0.25f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	glBegin(GL_QUADS);
	// Front Face
		glNormal3f( 0.0f, 0.0f, 1.0f);					// Normal Pointing Towards Viewer
		glVertex3f(-x, -y,  z);	// Point 1 (Front)
		glVertex3f( x, -y,  z);	// Point 2 (Front)
		glVertex3f( x,  y,  z);	// Point 3 (Front)
		glVertex3f(-x,  y,  z);	// Point 4 (Front)		
	glEnd();								// Done Drawing Quads
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);

	
}
