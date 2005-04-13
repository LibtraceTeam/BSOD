// partflow.cpp
// 


#include "stdafx.h"

#ifdef _WIN32
#include <windows.h>
#endif

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
#define PREALLOC 10 // Number of packets each flow preallocates memory for.

// The following globals are for debugging only
/*int flows_drawn = 0;
int packets_drawn = 0;
int num_particles = 0;
int num_flows = 0;*/

// An actual flow:
CPartFlow::CPartFlow()
: offset(0)//, vertices(20*6), tex_coords(20*6), colours(20*6*4)
{
	//	num_flows++;
	active_flow_ptr = NULL;

	// Reserve some space for 20 packets worth
	vertices.reserve(PREALLOC*6);
	tex_coords.reserve(PREALLOC*6);
	colours.reserve(PREALLOC*6*4);

	sam_count = 0;
	packets = 0;
	gc_count = 0;
	type = 0;

	speed = 1.0f;
}

CPartFlow::~CPartFlow()
{
	//	num_particles -= (const int)vertices.size() / 6 - offset;
	//	num_flows--;
}

void CPartFlow::Draw()
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
		// This is how will our point sprite's size will be modified by distance from the viewer.
		/*float quadratic[] =  { 1.0f, 0.0f, 0.01f };
		glPointParameterfvARB( GL_POINT_DISTANCE_ATTENUATION_ARB, quadratic );
		// The alpha of a point is calculated to allow the fading of points 
		//glPointParameterfARB( GL_POINT_FADE_THRESHOLD_SIZE_ARB, 60.0f );

		//float fAdjustedSize = m_fSize / 4.0f;

		//glPointParameterfARB( GL_POINT_SIZE_MIN_ARB, 1.0f );
		//glPointParameterfARB( GL_POINT_SIZE_MAX_ARB, fAdjustedSize );

		// Specify point sprite texture coordinate replacement mode for each texture unit
		glTexEnvf( GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE );
		
		// Render point sprites:
		glEnable( GL_POINT_SPRITE_ARB );
		//glPointSize( m_fSize );

		// Draw as point sprites:
		// Endpoints:
		d->SetColour( world.partVis->colour_table[colours[0]], world.partVis->colour_table[colours[1]], world.partVis->colour_table[colours[2]], 0.15f );
		glBegin( GL_POINTS );
		{
			// Render each particle...
			/*while( pParticle )
			{
				glVertex3f( pParticle->m_vCurPos.x,
					pParticle->m_vCurPos.y,
					pParticle->m_vCurPos.z );

				pParticle = pParticle->m_pNext;
			}
		}
		glEnd();

		// Flow:
		d->SetColour(1.0f, 1.0f, 1.0f, 1.0f);
		glBegin( GL_POINTS );
		{
			// Render each particle...
			/*while( pParticle )
			{
				glVertex3f( pParticle->m_vCurPos.x,
					pParticle->m_vCurPos.y,
					pParticle->m_vCurPos.z );

				pParticle = pParticle->m_pNext;
			}
		}
		glEnd();

		glDisable( GL_POINT_SPRITE_ARB );*/
	}
	else
	{
		// Draw start and end points
		d->SetColour( world.partVis->colour_table[colours[0]], world.partVis->colour_table[colours[1]], world.partVis->colour_table[colours[2]], 0.15f );
		d->DrawTriangles2( (float *)&endpoint_vertices[0], (float *)&endpoint_tex_coords[0], 4);
		d->SetColour(1.0f, 1.0f, 1.0f, 1.0f);

		// Draw the actual flow
		d->PushMatrix();
		d->Translate(translation);
		d->DrawTriangles2(
			(float *)&vertices[vertex_offset],
			(float *)&tex_coords[vertex_offset],
			(byte *)&colours[vertex_offset*4],
			num_triangles);
		d->PopMatrix();
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
	if( flow_colour[0] == 100 && flow_colour[1] == 0 && flow_colour[2] == 100 ) // TCP
	    tex = CTextureManager::tm.LoadTexture("data/matrix_01.png");
	else if( flow_colour[0] == 0 && flow_colour[1] == 0 && flow_colour[2] == 200 ) // HTTP
	    tex = CTextureManager::tm.LoadTexture("data/matrix_02.png");
	else if( flow_colour[0] == 150 && flow_colour[1] == 150 && flow_colour[2] == 240 ) // HTTPS
	    tex = CTextureManager::tm.LoadTexture("data/matrix_03.png");
	else if( flow_colour[0] == 200 && flow_colour[1] == 0 && flow_colour[2] == 0 ) // MAIL
	    tex = CTextureManager::tm.LoadTexture("data/matrix_04.png");
	else if( flow_colour[0] == 0 && flow_colour[1] == 150 && flow_colour[2] == 0 ) // FTP
	    tex = CTextureManager::tm.LoadTexture("data/matrix_05.png");
	else if( flow_colour[0] == 0 && flow_colour[1] == 250 && flow_colour[2] == 0 ) // VPN
	    tex = CTextureManager::tm.LoadTexture("data/matrix_06.png");
	else if( flow_colour[0] == 200 && flow_colour[1] == 200 && flow_colour[2] == 0 ) // DNS
	    tex = CTextureManager::tm.LoadTexture("data/matrix_07.png");
	else if( flow_colour[0] == 30 && flow_colour[1] == 85 && flow_colour[2] == 30 ) // NTP
	    tex = CTextureManager::tm.LoadTexture("data/matrix_08.png");
	else if( flow_colour[0] == 110 && flow_colour[1] == 110 && flow_colour[2] == 110 ) // SSH
	    tex = CTextureManager::tm.LoadTexture("data/matrix_09.png");
	else if( flow_colour[0] == 150 && flow_colour[1] == 100 && flow_colour[2] == 50 ) // UDP
	    tex = CTextureManager::tm.LoadTexture("data/matrix_10.png");
	else if( flow_colour[0] == 0 && flow_colour[1] == 250 && flow_colour[2] == 200 ) // ICMP
	    tex = CTextureManager::tm.LoadTexture("data/matrix_11.png");
	else if( flow_colour[0] == 240 && flow_colour[1] == 230 && flow_colour[2] == 140 ) // IRC
	    tex = CTextureManager::tm.LoadTexture("data/matrix_12.png");
	else if( flow_colour[0] == 200 && flow_colour[1] == 100 && flow_colour[2] == 0 ) // WINDOWS
	    tex = CTextureManager::tm.LoadTexture("data/matrix_13.png");
	else if( flow_colour[0] == 50 && flow_colour[1] == 150 && flow_colour[2] == 50 ) // P2P
	    tex = CTextureManager::tm.LoadTexture("data/matrix_14.png");
	else
	    tex = CTextureManager::tm.LoadTexture("data/matrix_15.png"); // OTHER
    }
    else
	tex = CTextureManager::tm.LoadTexture(world.partVis->particle_img);
}

void CPartFlow::Update(float diff)
{
	float percent = diff / time_to_live;
	Vector3f d = destination - start;
	d *= (percent * (speed * world.partVis->global_speed));

	translation += d;

	unsigned int vSize = (unsigned int)vertices.size();
	unsigned int lengthV = (unsigned)6*(offset+1);
	while( vSize >= lengthV ) 
	{
		Vector3f e = (vertices[offset*6]) + translation - jitter[offset];
		//Vector3f m = destination - start;
		if( (e - start).Length() > length )//m.Length() ) 
		//if( vertices[offset*6].x < destination.x )
		{
			offset++;
			lengthV = (unsigned)6*(offset+1);
//			num_particles--;
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
	if(vSize > 6*100 && offset)
	{
		vertices.erase( vertices.begin(), vertices.begin()+(6*offset) );
		tex_coords.erase( tex_coords.begin(), tex_coords.begin()+(6*offset) );
		colours.erase( colours.begin(), colours.begin()+(6*4*offset) );
		jitter.erase( jitter.begin(), jitter.begin()+offset );
		offset = 0;
	}
}


bool CPartFlow::AddParticle(unsigned char id, byte r, byte g, byte b, unsigned short _size, float speed, bool dark)
{
	//Log("flow %p count %x\n", this, sam_count);
	if(sam_count > 4)
		return( false );
	//sam_count++;
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
	const Vector3f offset( -(0.5f * size), 0, 0);

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
	}
	jitter.push_back(d);



	// -----------------------------------------------------------
	// HAX:
	if( world.partVis->billboard )
	{
		// Triangle 1:
		vertices.push_back(Vector3f(0,0,0)+start+offset-d-translation);
		vertices.push_back(Vector3f(0,0,size)+start+offset-d-translation);
		vertices.push_back(Vector3f(0,size,0)+start+offset-d-translation);

		// Triangle 2:
		vertices.push_back(Vector3f(0,0,size)+start+offset-d-translation);
		vertices.push_back(Vector3f(0,size,0)+start+offset-d-translation);
		vertices.push_back(Vector3f(0,size,size)+start+offset-d-translation);
		// -------------------------------------------------------------
	}
	else
	{
		// Triangle 1:
		vertices.push_back(Vector3f(0,0,0)+start+offset-d-translation);
		vertices.push_back(Vector3f(size,0,0)+start+offset-d-translation);
		vertices.push_back(Vector3f(0,size,0)+start+offset-d-translation);

		// Triangle 2:
		vertices.push_back(Vector3f(size,0,0)+start+offset-d-translation);
		vertices.push_back(Vector3f(0,size,0)+start+offset-d-translation);
		vertices.push_back(Vector3f(size,size,0)+start+offset-d-translation);
	}

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
	}

	// alpha was 80

	colours.push_back(r); colours.push_back(g); colours.push_back(b); 
	colours.push_back(140);
	colours.push_back(r); colours.push_back(g); colours.push_back(b); 
	colours.push_back(140);
	colours.push_back(r); colours.push_back(g); colours.push_back(b); 
	colours.push_back(140);

	colours.push_back(r); colours.push_back(g); colours.push_back(b); 
	colours.push_back(140);
	colours.push_back(r); colours.push_back(g); colours.push_back(b); 
	colours.push_back(140);
	colours.push_back(r); colours.push_back(g); colours.push_back(b); 
	colours.push_back(140);

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
	active_flow_ptr = NULL;

	// Reserve some space for 20 packets worth
	vertices.clear();
	tex_coords.clear();
	colours.clear();
	
	vertices.reserve(PREALLOC*6);
	tex_coords.reserve(PREALLOC*6);
	colours.reserve(PREALLOC*6*4);

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
