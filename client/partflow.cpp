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

	// Reserve some space for 20 packets worth
	vertices.reserve(20*6);
	tex_coords.reserve(20*6);
	colours.reserve(20*6*4);

	sam_count = 0;

	speed = 1.0f;
}

CPartFlow::~CPartFlow()
{
	//	num_particles -= (const int)vertices.size() / 6 - offset;
	//	num_flows--;
}

void CPartFlow::Draw()
{
	CDisplayManager *d = world.display;
	const int vertex_offset = offset * 6;
	const int num_triangles = ((const int)vertices.size() - vertex_offset) / 3;

//	flows_drawn++;

	if(num_triangles == 0)
		return;

//	packets_drawn += num_triangles / 2;

	d->BindTexture(tex);

	if(endpoint_vertices.size() == 0)
		CreateEndPoints();

	// Draw start and end points
	//d->SetColour(colours[0], colours[1], colours[2], (translation - start).Length() / (destination - start).Length());
	//byte c = (byte)((destination - start).Length() - (vertices.back() + translation).Length());
	//d->SetColour(colours[0] / 255.0f, colours[1] / 255.0f, colours[2] / 255.0f, 0.15f);
	d->SetColour( world.partVis->colour_table[colours[0]], world.partVis->colour_table[colours[1]], world.partVis->colour_table[colours[2]], 0.15f );
	d->DrawTriangles2(
		(float *)&endpoint_vertices[0],
		(float *)&endpoint_tex_coords[0],
		4);
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

void CPartFlow::CreateEndPoints()
{
	const float size = 0.3f;
	const Vector3f offset(0, 0.15f, 0);

	//if(start.x == 10.0f)
	//	size = -size;

	// Create start and end points
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

void CPartFlow::Update(float diff)
{
	float percent = diff / time_to_live;
	Vector3f d = destination - start;
	d *= (percent * speed);

	translation += d;

	unsigned int vSize = (unsigned int)vertices.size();
	unsigned int lengthV = (unsigned)6*(offset+1);
	while( vSize >= lengthV ) 
	{
		Vector3f e = (vertices[offset*6]) + translation - jitter[offset];
		//Vector3f m = destination - start;
		if( (e - start).Length() > length )//m.Length() ) 
		{
			offset++;
			lengthV = (unsigned)6*(offset+1);
//			num_particles--;
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


void CPartFlow::AddParticle(byte r, byte g, byte b, unsigned short _size, float speed, bool dark)
{
	//Log("flow %p count %x\n", this, sam_count);
	if(sam_count > 4)
		return;
	sam_count++;

	// -------------------------------------------------------------
	float percent = world.partVis->diff / time_to_live;
	Vector3f d = destination - start;
	d *= (percent * speed);
	float v_jitter = (float)rand()/RAND_MAX;
	d *= v_jitter;
	//d.z = 0.0f;
	
	// --------------------------------------------------------------

	float size;

	if(_size <= 512)
		size = 0.6f; // was 0.8f
	else if(_size < 1024)
		size = 0.8f; // was 1.0f
	else
		size = 1.0f; // was 1.2f

	if( vertices.size() > 0 )
	{
		//if( vertices.back().Length() > (Vector3f(size,size,0)+start-d-translation).Length() )
		if( vertices.back().x < (Vector3f(size,size,0)+start-d-translation).x )
		{
			return;
		}
	}
	jitter.push_back(d);

	// Triangle 1:
	vertices.push_back(Vector3f(0,0,0)+start-d-translation);
	vertices.push_back(Vector3f(size,0,0)+start-d-translation);
	vertices.push_back(Vector3f(0,size,0)+start-d-translation);

	// Triangle 2:
	vertices.push_back(Vector3f(size,0,0)+start-d-translation);
	vertices.push_back(Vector3f(0,size,0)+start-d-translation);
	vertices.push_back(Vector3f(size,size,0)+start-d-translation);

	tex_coords.push_back(Vector2f(0, 0));
	tex_coords.push_back(Vector2f(1, 0));
	tex_coords.push_back(Vector2f(0, 1));

	tex_coords.push_back(Vector2f(1, 0));
	tex_coords.push_back(Vector2f(0, 1));
	tex_coords.push_back(Vector2f(1, 1));

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
	this->dark = dark;
	if( speed > 0.0f )
	{
		this->speed = speed;
	}
}
