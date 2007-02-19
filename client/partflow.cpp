// partflow.cpp
// 


#include "stdafx.h"

#ifdef _WIN32
#include <windows.h>
#endif

#include <external/GL/gl.h>
#include <external/GL/glu.h>
#include "external/GL/glext.h"

#include "vector.h"
#include "polygon.h"
#include "world.h"
#include "triangle.h"
#include "display_manager.h"
#include "reporter.h"
#include "entity.h"
#include "texture_manager.h"
#include "exception.h"

#include "partflow.h"
#include "partvis.h"


float CPartFlow::time_to_live = 4.5f;

// The following globals are for debugging only
/*int flows_drawn = 0;
int packets_drawn = 0;
int num_particles = 0;
int num_flows = 0;*/

// An actual flow:
CPartFlow::CPartFlow()
: offset(0)
{
	//	num_flows++;
	//active_flow_ptr = NULL;

	// Reserve some space for 20 packets worth
	vertices.reserve(PREALLOC*6);
	//tex_coords.reserve(PREALLOC*6);
	//colours.reserve(PREALLOC*6*4);
	colour[0] = 0;
	colour[1] = 0;
	colour[2] = 0;

	sam_count = 0;
	packets = 0;
	gc_count = 0;
	type = 0;

	speed = 1.0f;
	is_singularity = false;
}

CPartFlow::~CPartFlow()
{
	//	num_particles -= (const int)vertices.size() / 6 - offset;
	//	num_flows--;
}

void CPartFlow::Draw( bool picking )
{
	if( packets == 0 )
		return;
	CDisplayManager *d = world.display;
	const int vertex_offset = offset * 6;
	const int num_triangles = ((const int)vertices.size() - vertex_offset) / 3;

//	flows_drawn++;

	if(num_triangles == 0)
		return;

//	packets_drawn += num_triangles / 2;

	if(endpoint_vertices.size() == 0)
		CreateEndPoints();

	d->BindTexture(tex);

	if( world.partVis->billboard )
	{
		// Draw as point sprites:
		// Endpoints:
		glPointSize( 20.0f );
		d->SetColour( world.partVis->colour_table[colour[0]], world.partVis->colour_table[colour[1]], world.partVis->colour_table[colour[2]], 0.15f );
		/*d->SetColour( 1.0f, 0.0f, 0.0f, 0.5f ); // DEBUG */
		if( picking )
		{
			// Note: Two separate glBegin()/end calls are needed when picking because you can't name objects inside
			// a glBegin()/end block and the points both have different IPs.
			glPushName( ip1 );
			glBegin( GL_POINTS );
			{
				//glVertex3f( endpoint_vertices[0].x, endpoint_vertices[0].y, endpoint_vertices[0].z );
				glVertex3f( start.x, start.y, start.z );
			}
			glEnd();
			glPopName(); // ???

			glPushName( ip2 );
			glBegin( GL_POINTS );
			{
				//glVertex3f( endpoint_vertices[6].x, endpoint_vertices[6].y, endpoint_vertices[6].z );
				glVertex3f( destination.x, destination.y, destination.z );
			}
			glEnd();
			glPopName(); // ???

			return;
		}
		else
		{
			glBegin( GL_POINTS );
			{
				//glVertex3f( endpoint_vertices[0].x, endpoint_vertices[0].y, endpoint_vertices[0].z );
				//glVertex3f( endpoint_vertices[6].x, endpoint_vertices[6].y, endpoint_vertices[6].z );
				glVertex3f( start.x, start.y, start.z );
				glVertex3f( destination.x, destination.y, destination.z );
			}
			glEnd();
			world.partVis->packetsFrame += 2;
		}

		// Flow:
		//d->SetColour(1.0f, 1.0f, 1.0f, 1.0f);
		
		
		d->SetColour( world.partVis->colour_table[colour[0]], world.partVis->colour_table[colour[1]], world.partVis->colour_table[colour[2]], world.partVis->global_alpha );
		/*if( start.x < destination.x )
			d->SetColour( 1.0f, 0.4f, 0.4f, world.partVis->global_alpha );
		else
			d->SetColour( 0.4f, 0.4f, 1.0f, world.partVis->global_alpha );*/
		
		
		// Draw the actual flow
		glPointSize( world.partVis->maxPointSize );
		d->PushMatrix();
		d->Translate(translation);
		glBegin( GL_POINTS );
		{
			// Render each particle...
			for( int i=vertex_offset; i < (int)(vertices.size()); i+= 6 )
			{
				glVertex3f( (vertices[i]).x, (vertices[i]).y, (vertices[i]).z );
				/*if( start.x < destination.x )
					world.partVis->packetsFrame++;
				else
					world.partVis->packetsFrameIn++;*/
				world.partVis->packetsFrame++;
			}
		}
		glEnd();
		d->PopMatrix();
	}
	else
	{
		// Draw start and end points

		d->SetColour( world.partVis->colour_table[colour[0]], world.partVis->colour_table[colour[1]], world.partVis->colour_table[colour[2]], 0.15f );
		// Start point:
		glPushName( ip1 );
		d->DrawTriangles2( (float *)&endpoint_vertices[0], (float *)&endpoint_tex_coords[0], 2);
		glPopName();
		// End point:
		glPushName( ip2 );
		d->DrawTriangles2( (float *)&endpoint_vertices[6], (float *)&endpoint_tex_coords[6], 2);
		glPopName();

		if( picking )
			return;
		//d->SetColour(1.0f, 1.0f, 1.0f, 1.0f);
		world.partVis->packetsFrame += 2;

		// Check that we have enough texture coordinates in our list:
		while( ((unsigned int)num_triangles * 3) > world.partVis->tex_coords.size() )
		{
			// Grow x2:
			world.partVis->tex_coords.resize( world.partVis->tex_coords.size() * 2 );
			memcpy( &(world.partVis->tex_coords[(world.partVis->tex_coords.size()/2)]), 
				&(world.partVis->tex_coords[0]), 
				world.partVis->tex_coords.size()*sizeof(Vector2f)/2 );
		}

		// Draw the actual flow
		d->PushMatrix();
		d->Translate(translation);
		d->SetColour( world.partVis->colour_table[colour[0]], world.partVis->colour_table[colour[1]], world.partVis->colour_table[colour[2]], world.partVis->global_alpha );
		d->DrawTriangles2(
			(float *)&vertices[vertex_offset],
			(float *)&(world.partVis->tex_coords[0]), //(float *)&(tex_coords[vertex_offset],
			//(byte *)&colours[vertex_offset*4],
			num_triangles);
		d->PopMatrix();
		world.partVis->packetsFrame += (num_triangles/2);
	}
}

void CPartFlow::CreateEndPoints()
{
    const float size = 0.3f * world.partVis->global_size;
    const Vector3f offset( -(0.5f * size), (0.5f * size), 0);

    //if(start.x == 10.0f)
    //	size = -size;

    // Create start and end points
	if( world.partVis->billboard )
	{
		endpoint_vertices.push_back(Vector3f(0,0,0)+start+offset);
		endpoint_vertices.push_back(Vector3f(0,0,size)+start+offset);
		endpoint_vertices.push_back(Vector3f(0,size,0)+start+offset);
		endpoint_vertices.push_back(Vector3f(0,0,size)+start+offset);
		endpoint_vertices.push_back(Vector3f(0,size,0)+start+offset);
		endpoint_vertices.push_back(Vector3f(0,size,size)+start+offset);

		endpoint_vertices.push_back(Vector3f(0,0,0)+destination+offset);
		endpoint_vertices.push_back(Vector3f(0,0,size)+destination+offset);
		endpoint_vertices.push_back(Vector3f(0,size,0)+destination+offset);
		endpoint_vertices.push_back(Vector3f(0,0,size)+destination+offset);
		endpoint_vertices.push_back(Vector3f(0,size,0)+destination+offset);
		endpoint_vertices.push_back(Vector3f(0,size,size)+destination+offset);
	}
	else
	{
		endpoint_vertices.push_back(Vector3f(0,0,0)+start+offset);
		endpoint_vertices.push_back(Vector3f(size,0,0)+start+offset);
		endpoint_vertices.push_back(Vector3f(0,size,0)+start+offset);
		endpoint_vertices.push_back(Vector3f(size,0,0)+start+offset);
		endpoint_vertices.push_back(Vector3f(0,size,0)+start+offset);
		endpoint_vertices.push_back(Vector3f(size,size,0)+start+offset);

		endpoint_vertices.push_back(Vector3f(0,0,0)+destination+offset);
		endpoint_vertices.push_back(Vector3f(size,0,0)+destination+offset);
		endpoint_vertices.push_back(Vector3f(0,size,0)+destination+offset);
		endpoint_vertices.push_back(Vector3f(size,0,0)+destination+offset);
		endpoint_vertices.push_back(Vector3f(0,size,0)+destination+offset);
		endpoint_vertices.push_back(Vector3f(size,size,0)+destination+offset);
	}

	if( (destination.x < start.x) || (world.partVis->matrix_mode) )
	{
		endpoint_tex_coords.push_back(Vector2f(1, 1));
		endpoint_tex_coords.push_back(Vector2f(0, 1));
		endpoint_tex_coords.push_back(Vector2f(1, 0));
		endpoint_tex_coords.push_back(Vector2f(0, 1));
		endpoint_tex_coords.push_back(Vector2f(1, 0));
		endpoint_tex_coords.push_back(Vector2f(0, 0));

		endpoint_tex_coords.push_back(Vector2f(1, 1));
		endpoint_tex_coords.push_back(Vector2f(0, 1));
		endpoint_tex_coords.push_back(Vector2f(1, 0));
		endpoint_tex_coords.push_back(Vector2f(0, 1));
		endpoint_tex_coords.push_back(Vector2f(1, 0));
		endpoint_tex_coords.push_back(Vector2f(0, 0));
	}
	else
	{
		endpoint_tex_coords.push_back(Vector2f(0, 0));
		endpoint_tex_coords.push_back(Vector2f(1, 0));
		endpoint_tex_coords.push_back(Vector2f(0, 1));
		endpoint_tex_coords.push_back(Vector2f(1, 0));
		endpoint_tex_coords.push_back(Vector2f(0, 1));
		endpoint_tex_coords.push_back(Vector2f(1, 1));

		endpoint_tex_coords.push_back(Vector2f(0, 0));
		endpoint_tex_coords.push_back(Vector2f(1, 0));
		endpoint_tex_coords.push_back(Vector2f(0, 1));
		endpoint_tex_coords.push_back(Vector2f(1, 0));
		endpoint_tex_coords.push_back(Vector2f(0, 1));
		endpoint_tex_coords.push_back(Vector2f(1, 1));
	}

	// Load texture:
	if( world.partVis->matrix_mode )
	{
		switch( type )
		{
		case 0:
			tex = CTextureManager::tm.LoadTexture( "data/matrix_01.png" );
			break;
		case 1:
			tex = CTextureManager::tm.LoadTexture( "data/matrix_02.png" );
			break;
		case 2:
			tex = CTextureManager::tm.LoadTexture( "data/matrix_03.png" );
			break;
		case 3:
			tex = CTextureManager::tm.LoadTexture( "data/matrix_04.png" );
			break;
		case 4:
			tex = CTextureManager::tm.LoadTexture( "data/matrix_05.png" );
			break;
		case 5:
			tex = CTextureManager::tm.LoadTexture( "data/matrix_06.png" );
			break;
		case 6:
			tex = CTextureManager::tm.LoadTexture( "data/matrix_07.png" );
			break;
		case 7:
			tex = CTextureManager::tm.LoadTexture( "data/matrix_08.png" );
			break;
		case 8:
			tex = CTextureManager::tm.LoadTexture( "data/matrix_09.png" );
			break;
		case 9:
			tex = CTextureManager::tm.LoadTexture( "data/matrix_10.png" );
			break;
		case 10:
			tex = CTextureManager::tm.LoadTexture( "data/matrix_11.png" );
			break;
		case 11:
			tex = CTextureManager::tm.LoadTexture( "data/matrix_12.png" );
			break;
		case 12:
			tex = CTextureManager::tm.LoadTexture( "data/matrix_13.png" );
			break;
		case 13:
			tex = CTextureManager::tm.LoadTexture( "data/matrix_14.png" );
			break;
		default:
			tex = CTextureManager::tm.LoadTexture("data/matrix_15.png");
			break;
		}
	}
	else
		tex = CTextureManager::tm.LoadTexture(world.partVis->particle_img);
}

void CPartFlow::Update(float diff)
{
	float percent = diff / time_to_live;
	Vector3f d = destination; // - start;
	d -= start;
	d *= (percent * (speed * world.partVis->global_speed));

	proxy_delta += d;
	if( !world.partVis->paused )
	{
		translation += proxy_delta;
		proxy_delta.x = proxy_delta.y = proxy_delta.z = 0.0f;
	}

	unsigned int vSize = (unsigned int)vertices.size();
	unsigned int lengthV = (unsigned)6*(offset+1);
	while( vSize >= lengthV ) 
	{
		Vector3f e = (vertices[offset*6]); // + translation - jitter[offset];
		e += translation;
		e -= jitter[offset];
		if( (e - start).Length() > length )
		{
			offset++;
			lengthV = (unsigned)6*(offset+1);
			packets--;
		} 
		else 
			break;
	}

	// The number 100 below is just an arbitrary number we decided upon
	// to use. Basically, once you have got to 100 triangles and you have
	// an offset set, it is time to do some garbage collection. Hopefully
	// the usual case is for this to never happen and have all the vertex
	// and so on memory cleaned up when the flow expires.
	// This is very important for efficiencies sake, be careful when
	// changing the value here, if it is too low a lot of memory copies
	// will be taking place.
	// - Sam, 13/2/2004
	//if( (vSize > 6*100) && offset)
	if(offset > vSize/6/3)
	{
		vertices.erase( vertices.begin(), vertices.begin()+(6*offset) );
		//tex_coords.erase( tex_coords.begin(), tex_coords.begin()+(6*offset) );
		//colours.erase( colours.begin(), colours.begin()+(6*4*offset) );
		jitter.erase( jitter.begin(), jitter.begin()+offset );
		offset = 0;
	}
}


bool CPartFlow::AddParticle(unsigned char id, byte r, byte g, byte b, unsigned short _size, float speed, bool dark)
{
	if(sam_count > 4)
		return( false );
	sam_count++;
	type = id;

	// -------------------------------------------------------------
	Vector3f d;
	if( world.partVis->jitter )
	{
		float percent = world.partVis->diff / time_to_live;
		d = destination - start;
		d *= (percent * (speed * world.partVis->global_speed));
		float v_jitter = (float)rand()/RAND_MAX;
		d *= v_jitter;		
		//d.z = 0.0f;
	}
	// --------------------------------------------------------------


	float size;

	if(_size <= 512)
		size = 0.6f * world.partVis->global_size;
	else if(_size < 1024)
		size = 0.8f * world.partVis->global_size;
	else
		size = 1.0f * world.partVis->global_size;

	//const Vector3f offset( -(0.5f * size), (0.5f * size), 0);
	
	//const Vector3f offset( -(0.5f * size), 0, 0);
	//const Vector3f offset(0,0.25f,0); // This is pretty random to fix positioning (but doesn't do a good job anyway). FIX-IT!
	const Vector3f offset(0.0f,0.0f,0.0f); // This is pretty random to fix positioning (but doesn't do a good job anyway). FIX-IT!
	
	/*
	 // WHAT DOES THIS DO?? XXX
	if( vertices.size() > 0 )
	{
		//if( vertices.back().Length() > (Vector3f(size,size,0)+start-d-translation).Length() )
		if( destination.x > start.x )
		{
			if( vertices.back().x < (Vector3f(size,size,0)+start-d-translation).x )
				return( false );
		}
		else if( vertices.back().x > (Vector3f(size,size,0)+start-d-translation).x )
			return( false );
	}*/
	jitter.push_back(d);

	//Log("flow %p count %x\n", this, sam_count);


	// -----------------------------------------------------------
	// HAX:
	if( world.partVis->billboard )
	{
		// Triangle 1:
		Vector3f t1c1(0,0,0);
		t1c1 += start;
		t1c1 += offset;
		t1c1 -= d;
		t1c1 -= translation;
		t1c1 -= proxy_delta;
		vertices.push_back(t1c1);
		Vector3f t1c2(0,0,size);
		t1c2 += start;
		t1c2 += offset;
		t1c2 -= d;
		t1c2 -= translation;
		t1c2 -= proxy_delta;
		vertices.push_back(t1c2);
		Vector3f t1c3(0,size,0);
		t1c3 += start;
		t1c3 += offset;
		t1c3 -= d;
		t1c3 -= translation;
		t1c3 -= proxy_delta;
		vertices.push_back(t1c3);

		// Triangle 2:
		Vector3f t2c1(0,0,size);
		t2c1 += start;
		t2c1 += offset;
		t2c1 -= d;
		t2c1 -= translation;
		t2c1 -= proxy_delta;
		vertices.push_back(t2c1);
		Vector3f t2c2(0,size,0);
		t2c2 += start;
		t2c2 += offset;
		t2c2 -= d;
		t2c2 -= translation;
		t2c2 -= proxy_delta;
		vertices.push_back(t2c2);
		Vector3f t2c3(0,size,size);
		t2c3 += start;
		t2c3 += offset;
		t2c3 -= d;
		t2c3 -= translation;
		t2c3 -= proxy_delta;
		vertices.push_back(t2c3);
		// -------------------------------------------------------------
	}
	else
	{
		// Triangle 1:
		vertices.push_back(Vector3f(0,0,0)+start+offset-d-translation-proxy_delta);
		vertices.push_back(Vector3f(size,0,0)+start+offset-d-translation-proxy_delta);
		vertices.push_back(Vector3f(0,size,0)+start+offset-d-translation-proxy_delta);

		// Triangle 2:
		vertices.push_back(Vector3f(size,0,0)+start+offset-d-translation-proxy_delta);
		vertices.push_back(Vector3f(0,size,0)+start+offset-d-translation-proxy_delta);
		vertices.push_back(Vector3f(size,size,0)+start+offset-d-translation-proxy_delta);
	}

/*	ORIGINAL CODE (pre-optimisation). Stupid operators creating loads of new objects 'n stuff.******
	if( world.partVis->billboard )
	{
		// Triangle 1:
		vertices.push_back(Vector3f(0,0,0)+start+offset-d-translation-proxy_delta);
		vertices.push_back(Vector3f(0,0,size)+start+offset-d-translation-proxy_delta);
		vertices.push_back(Vector3f(0,size,0)+start+offset-d-translation-proxy_delta);

		// Triangle 2:
		vertices.push_back(Vector3f(0,0,size)+start+offset-d-translation-proxy_delta);
		vertices.push_back(Vector3f(0,size,0)+start+offset-d-translation-proxy_delta);
		vertices.push_back(Vector3f(0,size,size)+start+offset-d-translation-proxy_delta);
		// -------------------------------------------------------------
	}
	else
	{
		// Triangle 1:
		vertices.push_back(Vector3f(0,0,0)+start+offset-d-translation-proxy_delta);
		vertices.push_back(Vector3f(size,0,0)+start+offset-d-translation-proxy_delta);
		vertices.push_back(Vector3f(0,size,0)+start+offset-d-translation-proxy_delta);

		// Triangle 2:
		vertices.push_back(Vector3f(size,0,0)+start+offset-d-translation-proxy_delta);
		vertices.push_back(Vector3f(0,size,0)+start+offset-d-translation-proxy_delta);
		vertices.push_back(Vector3f(size,size,0)+start+offset-d-translation-proxy_delta);
	}
	[END OF THAT FOO]*****************************************************************************/


/*
	if( (destination.x < start.x) || (world.partVis->matrix_mode) )
	{
		tex_coords.push_back(Vector2f(1, 1));
		tex_coords.push_back(Vector2f(0, 1));
		tex_coords.push_back(Vector2f(1, 0));

		tex_coords.push_back(Vector2f(0, 1));
		tex_coords.push_back(Vector2f(1, 0));
		tex_coords.push_back(Vector2f(0, 0));
	}
	else
	{
		tex_coords.push_back(Vector2f(0, 0));
		tex_coords.push_back(Vector2f(1, 0));
		tex_coords.push_back(Vector2f(0, 1));

		tex_coords.push_back(Vector2f(1, 0));
		tex_coords.push_back(Vector2f(0, 1));
		tex_coords.push_back(Vector2f(1, 1));
	} */

	colour[0] = r;
	colour[1] = g;
	colour[2] = b;

//	num_particles++;
	packets++;
	this->dark = dark;
	if( speed > 0.0f )
	{
		this->speed = speed;
	}

	return( true );
}

void CPartFlow::ReInitialize()
{
	//	num_flows++;
	//active_flow_ptr = NULL;

	// Reserve some space for 20 packets worth
	vertices.clear();
	//tex_coords.clear();
	//colours.clear();
	
	vertices.reserve(PREALLOC*6);
	//tex_coords.reserve(PREALLOC*6);
	//colours.reserve(PREALLOC*6*4);

	translation.x = translation.y = translation.z = 0.0f;
	start.x = start.y = start.z = 0.0f;
	destination.x = destination.y = destination.z = 0.0f;

	sam_count = 0;
	packets = 0;
	offset = 0;

	speed = 1.0f;

	jitter.clear();
	endpoint_vertices.clear();
	endpoint_tex_coords.clear();
}
