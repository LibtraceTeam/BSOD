// $Header$
#include "stdafx.h"

#include "vector.h"
#include "polygon.h"
#include "world.h"
#include "triangle.h"
#include "display_manager.h"
#include "reporter.h"
#include "entity.h"
#include "texture_manager.h"
#include "exception.h"

#include "partvis.h"

// Particle visualisation file
float CPartFlow::time_to_live = 4.5f; // in seconds

// The following globals are for debugging only
int flows_drawn = 0;
int packets_drawn = 0;
int num_particles = 0;
int num_flows = 0;

// An actual flow:
void CPartFlow::Draw()
{
    CDisplayManager *d = world.display;
    const int vertex_offset = offset * 6;
    const int num_triangles = (vertices.size() - vertex_offset) / 3;

    flows_drawn++;

    if(num_triangles == 0)
	return;

    packets_drawn += num_triangles / 2;

    d->BindTexture(tex);

    if(endpoint_vertices.size() == 0)
	CreateEndPoints();

    // Draw start and end points
    //d->SetColour(colours[0], colours[1], colours[2], (translation - start).Length() / (destination - start).Length());
    byte c = (byte)((destination - start).Length() - (vertices.back() + translation).Length());
    d->SetColour(colours[0] / 255.0f, colours[1] / 255.0f, colours[2] / 255.0f, 0.15f);
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
    d *= percent;

    translation += d;

    while(vertices.size() >= (unsigned)6*(offset+1)) {
	Vector3f e = (vertices[offset*6]) + translation;
	Vector3f m = destination - start;
	if( (e - start).Length() > m.Length() ) {
	    offset++;
	    num_particles--;
	} else {
	    break;
	}
    }

    // The number 100 below is just an abitrary number we decided upon
    // to use. Basically, once you have got to 100 triangles and you have
    // an offset set, it is time to do some garbage collection. Hopefully
    // the usual case is for this to never happen and have all the vertex
    // and so on memory cleaned up when the flow expires.
    // This is very important for efficiencies sake, be careful when
    // changing the value here, if it is too low a lot of memory copies
    // will be taking place.
    // - Sam, 13/2/2004
    if(vertices.size() > 6*100 && offset) {
	 vertices.erase( vertices.begin(), vertices.begin()+(6*offset) );
	 tex_coords.erase( tex_coords.begin(), tex_coords.begin()+(6*offset) );
	 colours.erase( colours.begin(), colours.begin()+(6*4*offset) );
	 offset = 0;
    }
}

// deprecated!
void CPartFlow::MoveParticles()
{
    throw CException("MoveParticles is deprecated.");
    vector<Vector3f>::iterator i = vertices.begin()+(offset*6);
    for(; i != vertices.end(); ++i) {
	(*i) += translation;
    }
    translation = Vector3f();
}

void CPartFlow::AddParticle(byte r, byte g, byte b, unsigned short _size)
{
	//Log("flow %p count %x\n", this, sam_count);
    sam_count++;
    if(sam_count > 5)
		return;

    float size;
    // size = 1; // Old operation

    if(_size <= 512)
	    size = 0.6f; // was 0.8f
    else if(_size < 1024)
	    size = 0.8f; // was 1.0f
    else
	    size = 0.9f; // was 1.2f
    
    vertices.push_back(Vector3f(0,0,0)+start-translation);
    vertices.push_back(Vector3f(size,0,0)+start-translation);
    vertices.push_back(Vector3f(0,size,0)+start-translation);

    vertices.push_back(Vector3f(size,0,0)+start-translation);
    vertices.push_back(Vector3f(0,size,0)+start-translation);
    vertices.push_back(Vector3f(size,size,0)+start-translation);

    tex_coords.push_back(Vector2f(0, 0));
    tex_coords.push_back(Vector2f(1, 0));
    tex_coords.push_back(Vector2f(0, 1));

    tex_coords.push_back(Vector2f(1, 0));
    tex_coords.push_back(Vector2f(0, 1));
    tex_coords.push_back(Vector2f(1, 1));

	//r = g = b = 255;

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

    num_particles++;
}

CPartFlow::CPartFlow()
 	: offset(0)
{
    num_flows++;

    // Reserve some space for 20 packets worth
    vertices.reserve(20*6);
    tex_coords.reserve(20*6);
    colours.reserve(20*6*4);

	sam_count = 0;
}

CPartFlow::~CPartFlow()
{
    num_particles -= vertices.size() / 6 - offset;
    num_flows--;
}

// The "container" class implementation
void CPartVis::Draw()
{
    FlowMap::const_iterator i = flows.begin();
    CDisplayManager *d = world.display;

    // Common state for all flows:
    d->SetBlend(true);
    d->SetBlendMode(CDisplayManager::Transparent2);
    d->SetDepthTest(false);
    
    // Draw the flows
    for(; i != flows.end(); ++i) {
	i->second->Draw();
    }

    d->SetDepthTest(true);

    // Draw the left, then the right sides of the display.
    d->SetBlend(true);
    d->SetBlendMode(CDisplayManager::Transparent2);
    d->SetColour(1.0f, 1.0f, 1.0f, 0.35f);
    left->Draw();
    right->Draw();
    d->SetColour(1.0f, 1.0f, 1.0f);
    d->SetBlend(false);

    flows_drawn = packets_drawn = 0;
}

void CPartVis::Update(float diff)
{
    FlowMap::const_iterator i = flows.begin();

    if(paused)
	return;
    
    for(; i != flows.end(); ++i) {
	i->second->Update(diff);
    }
}

CPartVis::CPartVis()
    : paused(false)
{
    // Need to create a couple of quads, one with the uni logo, another
    // without the logo.
    left = new CTriangleFan();
    left->vertices.push_back(Vector3f(-10, 10, -10));	
    left->vertices.push_back(Vector3f(-10, 10, 10));	
    left->vertices.push_back(Vector3f(-10, -10, 10));	
    left->vertices.push_back(Vector3f(-10, -10, -10));	

    left->texCoords.push_back(Vector2f(1, 0));
    left->texCoords.push_back(Vector2f(0, 0));
    left->texCoords.push_back(Vector2f(0, 1));
    left->texCoords.push_back(Vector2f(1, 1));
    
    left->tex = CTextureManager::tm.LoadTexture("data/uni-logo.png");

    right = new CTriangleFan();
    right->vertices.push_back(Vector3f(10, 10, -10));	
    right->texCoords.push_back(Vector2f(0, 0));
    right->vertices.push_back(Vector3f(10, 10, 10));	
    right->texCoords.push_back(Vector2f(1, 0));
    right->vertices.push_back(Vector3f(10, -10, 10));	
    right->texCoords.push_back(Vector2f(1, 1));
    right->vertices.push_back(Vector3f(10, -10, -10));	
    right->texCoords.push_back(Vector2f(0, 1));
    right->tex = CTextureManager::tm.LoadTexture("data/earth.png");
}

void CPartVis::UpdateFlow(unsigned int flow_id, Vector3f v1, Vector3f v2)
{
    FlowMap::const_iterator i = flows.find(flow_id);

    if(i == flows.end()) {
	// If the flow does not already exist (it shouldn't at this point)
		CPartFlow *flow = new CPartFlow();
		flow->start = v1;
		flow->destination = v2;
		flow->tex = CTextureManager::tm.LoadTexture("data/particle.png");
		flows.insert(FlowMap::value_type(flow_id, flow));
	} else {
		// Found
		Log("UpdateFlow called on flow that already exits (flow=%d)\n",
			flow_id);
    }
}

void CPartVis::UpdatePacket(unsigned int flow_id, uint32 timestamp, byte r,
	byte g, byte b, unsigned short size)
{
    FlowMap::const_iterator i = flows.find(flow_id);

	if(i == flows.end()) {
		Log("Adding packet to non-existant flow %d\n", flow_id);
	} else {
		CPartFlow *flow = i->second;

		//flow->Update_ServerTime(timestamp);
		// XXX: at this point we could use the time to make sure the 
		// particle is in the correct position; if we have lost some amount
		// of time it might not make sense to have this particle start at
		// the start position. To make this useful we would really need
		// time more accurate than second accuracy, though.

		//flow->MoveParticles();
		flow->AddParticle(r, g, b, size);
	}

    last_timestamp = timestamp;
}

void CPartVis::RemoveFlow(unsigned int id)
{
    FlowMap::iterator i = flows.find(id);

    if(i == flows.end()) {
        Log("Removing non-existant flow %d\n", id);
    } else {
        delete i->second;
	flows.erase(i);
    }
}

void CPartVis::BeginUpdate()
{
    FlowMap::iterator i = flows.begin();
    for(; i != flows.end(); ++i)
	i->second->ResetCounter();
}

void CPartVis::EndUpdate()
{
}

void CPartVis::TogglePaused()
{
    paused = !paused;
}
