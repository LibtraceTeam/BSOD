#include "stdafx.h"

#include "vector.h"
#include "polygon.h"
#include "world.h"
#include "triangle.h"
#include "display_manager.h"
#include "reporter.h"
#include "entity.h"
#include "texture_manager.h"
#include "octree.h"

#include "partvis.h"

// Particle visualisation file
float CPartFlow::time_to_live = 4.5f; // in seconds

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

    if(num_triangles > 0) {
	d->PushMatrix();
	d->Translate(translation);

	d->BindTexture(tex);
	d->SetBlend(true);
	d->SetBlendMode(CDisplayManager::Transparent2);

	packets_drawn += num_triangles / 2;

	d->DrawTriangles2(
	    (float *)&vertices[vertex_offset],
	    (float *)&tex_coords[vertex_offset],
	    (byte *)&colours[vertex_offset*4],
	    num_triangles
			);
	
	d->SetBlend(false);
	d->PopMatrix();
    } 
    /*Log("Drawing flow with %d vertices with offset %d (%d)\n", 
	    vertices.size(),offset,
	    (vertices.size() - offset*6)/3    
	    );*/
}

void CPartFlow::Update(float diff)
{
    float percent = diff / time_to_live;

    translation += (destination - start) * percent;

    while(vertices.size() >= (unsigned)6*(offset+1)) {
	vector<Vector3f>::iterator i = &vertices[offset*6];
	Vector3f e = (*i) + translation;
	Vector3f m = destination - start;
	if( (e - start).Length() > m.Length() ) {
	    offset++;
	    num_particles--;
	} else {
	    break;
	}
    }

    if(vertices.size() > 6*100 && offset) {
	 vertices.erase( vertices.begin(), &vertices[6*offset] );
	 tex_coords.erase( tex_coords.begin(), &tex_coords[6*offset] );
	 colours.erase( colours.begin(), &colours[6*4*offset] );
	 offset = 0;
    }
}

void CPartFlow::MoveParticles()
{
    vector<Vector3f>::iterator i = &vertices[offset*6];
    for(; i != vertices.end(); ++i) {
	(*i) += translation;
    }
    translation = Vector3f();
}

void CPartFlow::AddParticle(byte r, byte g, byte b, unsigned short size)
{
    vertices.push_back(Vector3f(0,0,0)+start);
    vertices.push_back(Vector3f(1,0,0)+start);
    vertices.push_back(Vector3f(0,1,0)+start);

    vertices.push_back(Vector3f(1,0,0)+start);
    vertices.push_back(Vector3f(0,1,0)+start);
    vertices.push_back(Vector3f(1,1,0)+start);

    tex_coords.push_back(Vector2f(0, 0));
    tex_coords.push_back(Vector2f(1, 0));
    tex_coords.push_back(Vector2f(0, 1));

    tex_coords.push_back(Vector2f(1, 0));
    tex_coords.push_back(Vector2f(0, 1));
    tex_coords.push_back(Vector2f(1, 1));

    colours.push_back(r); colours.push_back(g); colours.push_back(b); colours.push_back(80);
    colours.push_back(r); colours.push_back(g); colours.push_back(b); colours.push_back(80);
    colours.push_back(r); colours.push_back(g); colours.push_back(b); colours.push_back(80);

    colours.push_back(r); colours.push_back(g); colours.push_back(b); colours.push_back(80);
    colours.push_back(r); colours.push_back(g); colours.push_back(b); colours.push_back(80);
    colours.push_back(r); colours.push_back(g); colours.push_back(b); colours.push_back(80);

    num_particles++;
}

CPartFlow::CPartFlow()
 	: offset(0)
{
    num_flows++;
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
    //Log("CPartVis::Draw() ... flows.size()=%d\n", flows.size());
    for(; i != flows.end(); ++i) {
	//Log("Drawing flow id=%d\n", i->first);
	i->second->Draw();
    }

    CDisplayManager *d = world.display;
    d->SetBlend(true);
    d->SetBlendMode(CDisplayManager::Transparent2);
    d->SetColour(1.0f, 1.0f, 1.0f, 0.35f);
    left->Draw();
    right->Draw();
    d->SetColour(1.0f, 1.0f, 1.0f);
    d->SetBlend(false);

    /*Log("flows_drawn:%d num_flows:%d packets_drawn:%d num_packets:%d\n",
	    flows_drawn, num_flows, packets_drawn, num_particles);*/

    flows_drawn = packets_drawn = 0;
}

void CPartVis::Update(float diff)
{
    FlowMap::const_iterator i = flows.begin();
    for(; i != flows.end(); ++i) {
	i->second->Update(diff);
    }
}

CPartVis::CPartVis()
{
    left = new CTriangleFan();
    left->vertices.push_back(Vector3f(-10, 10, -10));	left->texCoords.push_back(Vector2f(0, 0));
    left->vertices.push_back(Vector3f(-10, 10, 10));	left->texCoords.push_back(Vector2f(1, 0));
    left->vertices.push_back(Vector3f(-10, -10, 10));	left->texCoords.push_back(Vector2f(1, 1));
    left->vertices.push_back(Vector3f(-10, -10, -10));	left->texCoords.push_back(Vector2f(0, 1));
    left->tex = CTextureManager::tm.LoadTexture("data/uni-logo.png");

    right = new CTriangleFan();
    right->vertices.push_back(Vector3f(10, 10, -10));	right->texCoords.push_back(Vector2f(0, 0));
    right->vertices.push_back(Vector3f(10, 10, 10));	right->texCoords.push_back(Vector2f(1, 0));
    right->vertices.push_back(Vector3f(10, -10, 10));	right->texCoords.push_back(Vector2f(1, 1));
    right->vertices.push_back(Vector3f(10, -10, -10));	right->texCoords.push_back(Vector2f(0, 1));
    right->tex = CTextureManager::tm.LoadTexture("data/right.png");
}

void CPartVis::UpdateFlow(unsigned int flow_id, Vector3f v1, Vector3f v2)
{
    FlowMap::const_iterator i = flows.find(flow_id);

    if(i == flows.end()) {
	CPartFlow *flow = new CPartFlow();
	flow->start = v1;
	flow->destination = v2;
	flow->tex = CTextureManager::tm.LoadTexture("data/particle.png");
	flows[flow_id] = flow; // XXX

	//Log("Flows.size()=%d", flows.size());
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
	
	flow->MoveParticles();
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
	//Log("Removing flow: %d\n", id);
        delete i->second;
	flows.erase(i);
    }
}
