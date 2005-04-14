/*
 * This file is part of bsod-client
 *
 * Copyright (c) 2004 The University of Waikato, Hamilton, New Zealand.
 * Authors: Sam Jansen
 *	    Sebastian Dusterwald
 *          
 * All rights reserved.
 *
 * This code has been developed by the University of Waikato WAND 
 * research group. For further information please see http://www.wand.net.nz/
 *
 * bsod-client includes software developed by Sam Jansen and Jesse Baker 
 * (see http://www.wand.net.nz/~stj2/bung).
 */

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

// $Header$
#include "stdafx.h"

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>

#include "vector.h"
#include "polygon.h"
#include "world.h"
#include "triangle.h"
#include "display_manager.h"
#include "reporter.h"
#include "entity.h"
#include "texture_manager.h"
#include "exception.h"
#include "system_driver.h"

#include "partflow.h"
#include "partvis.h"

#define MAX_FILTER_STATE 15
// Particle visualisation file


CPartVis::CPartVis( bool mm )
: paused(false)
{
	// Need to create a couple of quads, one with the uni logo, another
	// without the logo.
	filter_state = -1;
	packetsFrame = 0;
	diff = 0.0f;
	last_gc = world.sys->TimerGetTime();
	show_dark = 0;
	fps = 0.0f;
	show_help = false;
	global_speed = 1.0f;
	do_gcc = true;
	matrix_mode = mm;

	// Build colour lookup table:
	for( int i=0; i<256; i++ )
		colour_table[i] = (float)i/255.0f;

	left = new CTriangleFan();
	left->vertices.push_back(Vector3f(-10, 10, -10));	
	left->vertices.push_back(Vector3f(-10, 10, 10));	
	left->vertices.push_back(Vector3f(-10, -10, 10));	
	left->vertices.push_back(Vector3f(-10, -10, -10));	

	left->texCoords.push_back(Vector2f(1, 0));
	left->texCoords.push_back(Vector2f(0, 0));
	left->texCoords.push_back(Vector2f(0, 1));
	left->texCoords.push_back(Vector2f(1, 1));

	if( matrix_mode )
		left->tex = CTextureManager::tm.LoadTexture("data/matrix_left_.png");
	else
		left->tex = CTextureManager::tm.LoadTexture("data/left.png");

	right = new CTriangleFan();
	right->vertices.push_back(Vector3f(10, 10, -10));	
	right->texCoords.push_back(Vector2f(0, 0));
	right->vertices.push_back(Vector3f(10, 10, 10));	
	right->texCoords.push_back(Vector2f(1, 0));
	right->vertices.push_back(Vector3f(10, -10, 10));	
	right->texCoords.push_back(Vector2f(1, 1));
	right->vertices.push_back(Vector3f(10, -10, -10));	
	right->texCoords.push_back(Vector2f(0, 1));
	if( matrix_mode )
		right->tex = CTextureManager::tm.LoadTexture("data/matrix_right.png");
	else
		right->tex = CTextureManager::tm.LoadTexture("data/right.png");

	wandlogo = CTextureManager::tm.LoadTexture( "data/wand.png" );
	helptex = CTextureManager::tm.LoadTexture( "data/help.png" );
}

// The "container" class implementation
void CPartVis::Draw()
{
    //FlowMap::const_iterator i = flows.begin();
	IterList::iterator i = active_nodes.begin();
    CDisplayManager *d = world.display;

    // Common state for all flows:
    d->SetBlend(true);
    d->SetBlendMode(CDisplayManager::Transparent2);
    d->SetDepthTest(false);

	//BillboardBegin();
	for( ; i != active_nodes.end(); ++i )
	{
		(*i)->second->ResetCounter();
		if( !paused )
		{
			(*i)->second->Update(diff);
			if( (*i)->second->packets == 0 )
			{
				IterList::iterator tmp = i;
				tmp--;

				active_nodes.erase( (*i)->second->active_flow_ptr );
				//(*i)->second->active_flow_ptr = NULL;
				i = tmp;
				continue;
			}
		}

		if( (show_dark == 0) || ( (show_dark == 1) && (*i)->second->dark) || ( (show_dark == 2) && !(*i)->second->dark) ) 
		{
			if( filter_state == -1 )
				(*i)->second->Draw();
			else if( filter_state == (*i)->second->type )
				(*i)->second->Draw();
		}
	}
	//BillboardEnd();

    d->SetDepthTest(true);

    // Draw the left, then the right sides of the display.
    d->SetBlend(true);
    d->SetBlendMode(CDisplayManager::Transparent2);
    d->SetColour(1.0f, 1.0f, 1.0f, 0.35f);
    left->Draw();
    right->Draw();
    d->SetColour(1.0f, 1.0f, 1.0f);
    d->SetBlend(false);

	// Display current toggle state:
	char outstr[256];

	if( filter_state == -1 )
		strcpy( outstr, "All packets." );
	else
	{
		FlowDescMap::iterator iter;
		if( (iter = fdmap.find(filter_state) ) != fdmap.end() )
			strcpy( outstr, iter->second->name );
	}

	if( show_dark == 0 )
		strcat( outstr, " - All" );
	else if( show_dark == 1 )
		strcat( outstr, " - Darknet" );
	else
		strcat( outstr, " - Sans darknet" );

	float w = 360, h = 40;
	float x = 0, y = d->GetHeight() - h;
	string str;

	str = outstr;

	d->Begin2D();
	d->SetColour(0.4f, 0.8f, 0.9f, 0.75f);
	d->BindTexture(NULL);
	d->SetBlend(true);
	d->SetBlendMode(CDisplayManager::Transparent);
	d->Draw2DQuad((int)x, (int)y, (int)(x + w), (int)(y + h));
	d->BindTexture(wandlogo);
	d->SetColour(1.0f, 1.0f, 1.0f, 0.8f);
	d->Draw2DQuad( d->GetWidth() - 160, d->GetHeight() - 65, d->GetWidth(), d->GetHeight() );
	if( show_help )
	{
		d->BindTexture( helptex );
		d->Draw2DQuad( 0, 0, 516, 480 );
	}
	else
	{
		d->BindTexture(NULL);
		d->SetBlendMode(CDisplayManager::Multiply);
		d->SetBlend(false);
		d->SetColour(1.0f, 1.0f, 1.0f);
		if( matrix_mode )
			d->DrawString2( 5, 5, "MATRIX!");
		else
			d->DrawString2( 5, 5, "F1 for help.");
	}
	d->BindTexture(NULL);
	d->SetBlendMode(CDisplayManager::Multiply);
	d->SetBlend(false);
	d->SetColour(1.0f, 1.0f, 1.0f);
	d->DrawString2((int)(x+3), (int)(y+1), str);
	
	d->End2D();

    //flows_drawn = packets_drawn = 0;
}

void CPartVis::Update(float diff)
{
	if(paused)
		return;

	this->diff = diff;

    /*FlowMap::const_iterator i = flows.begin();
    for(; i != flows.end(); ++i) {
	i->second->Update(diff);
    }*/
	if( do_gcc )
	{
		if( (world.sys->TimerGetTime() - last_gc) > 180000.0f ) // 5 minutes
		{
			last_gc = world.sys->TimerGetTime();
			GCPartFlows(); 
		}
	}
}

void CPartVis::UpdateFlow(unsigned int flow_id, Vector3f v1, Vector3f v2)
{
    //FlowMap::const_iterator i = flows.find(flow_id);

    //if(i == flows.end()) {
		//if( fps < 30.0f )//flows.size() > 10000 )
		//	return;
		//flows.size();
	// If the flow does not already exist (it shouldn't at this point)
		//CPartFlow *flow = new CPartFlow();
		CPartFlow *flow = AllocPartFlow();
		flow->start = v1;// - Vector3f((global_size*0.5f), 0.0f, 0.0f);
		flow->destination = v2;// - Vector3f((global_size*0.5f), 0.0f, 0.0f);
		flow->length = (v2 - v1).Length();

		flows.insert(FlowMap::value_type(flow_id, flow));
	/*} else {
		// Found
		Log("UpdateFlow called on flow that already exits (flow=%d)\n",
			flow_id);
    }*/
}

void CPartVis::UpdatePacket(unsigned int flow_id, uint32 timestamp, byte id_num, 
							unsigned short size, float speed, bool dark)
{
    FlowMap::iterator i = flows.find(flow_id);

	if(i == flows.end()) {
		return;//Log("Adding packet to non-existant flow %d\n", flow_id);
	} else {
		CPartFlow *flow = i->second;

		// XXX: at this point we could use the time to make sure the 
		// particle is in the correct position; if we have lost some amount
		// of time it might not make sense to have this particle start at
		// the start position. To make this useful we would really need
		// time more accurate than second accuracy, though.

		/*flow->flow_colour[0] = r;
		flow->flow_colour[1] = g;
		flow->flow_colour[2] = b;
		if( matrix_mode )
		{
			r = b = 36;
			g = 132;
		}*/

		FlowDescMap::iterator iter;
		if( (iter = fdmap.find(id_num)) != fdmap.end() )
		{
			byte r, g, b;
			if( matrix_mode )
			{
				r = b = 36;
				g = 132;
			}
			else
			{
				r = iter->second->colour[0];
				g = iter->second->colour[1];
				b = iter->second->colour[2];
			}
			if( flow->AddParticle(id_num, r, g, b, size, speed, dark) && (flow->packets == 1) )
			{
				flow->active_flow_ptr = AddActiveFlow( i );
			}
		}
		else
			Log( "Received packet that had no descriptor!" );
	}

    last_timestamp = timestamp;
}

void CPartVis::RemoveFlow(unsigned int id)
{
    FlowMap::iterator i = flows.find(id);

    if(i == flows.end()) 
	{
        return;//Log("Removing non-existant flow %d\n", id);
    } 
	else 
	{
		//RemoveActiveFlow( i );
		if( i->second->packets != 0 )
			active_nodes.erase( i->second->active_flow_ptr );
        //delete i->second;
		FreePartFlow( i->second );
		flows.erase(i);
    }
}

void CPartVis::BeginUpdate()
{
	packetsFrame = 0;
}

void CPartVis::EndUpdate()
{
}

void CPartVis::TogglePaused()
{
    paused = !paused;
}

void CPartVis::ToggleFilter()
{
	filter_state++;
	FlowDescMap::iterator iter = fdmap.find( filter_state );
	if( iter == fdmap.end() )
		filter_state = -1;
	if( (iter->second->colour[0] == 0) && (iter->second->colour[1] == 0) && (iter->second->colour[2] == 0) )
		filter_state = -1;
}

void CPartVis::ToggleBackFilter()
{
	filter_state--;
	if( filter_state < -1 )
	{
		for( int i=0; i<256; i++ )
		{
			FlowDescMap::iterator iter = fdmap.find( i );
			if( iter == fdmap.end() )
			{
				filter_state = i-1;
				return;
			}
			if( (iter->second->colour[0] == 0) && (iter->second->colour[1] == 0) && (iter->second->colour[2] == 0) )
			{
				filter_state = i-1; // Should maybe cache this value!
				return;
			}
		}
		
	}
}

void CPartVis::ToggleShowDark()
{
	show_dark++;// = !show_dark;
	if( show_dark > 2 )
		show_dark = 0;
}

int CPartVis::NumFlows()
{
	return( (int)flows.size() );
}

void CPartVis::ToggleHelp()
{
	show_help = !show_help;
}

void CPartVis::ChangeSpeed( bool faster )
{
	if( faster )
	{
		if( global_speed >= 1.0f )
		{
			global_speed += 1.0f;
			if( global_speed > 5.0f )
				global_speed = 5.0f;
			return;
		}
		global_speed += 0.1f;
		return;
	}

	if( global_speed < 2.0f )
	{
		global_speed -= 0.1f;
		if( global_speed < 0.5f )
			global_speed = 0.5f;
		return;
	}
	global_speed -= 1.0f;
}

void CPartVis::KillAll()
{
	active_nodes.clear();

	FlowMap::iterator i = flows.begin();
	
	for( ; i != flows.end(); i++ )
	{
		FreePartFlow( i->second );
	}

	flows.clear();
	//partflow_pool.clear();
}

IterList::iterator CPartVis::AddActiveFlow( const FlowMap::iterator i )
{
	//active_nodes.push_back( i );
	//return( active_nodes.end()-- );
	active_nodes.push_front( i );
	return( active_nodes.begin() );
}

void CPartVis::RemoveActiveFlow( const FlowMap::iterator i )
{
	active_nodes.remove( i );
}

CPartFlow *CPartVis::AllocPartFlow()
{
	if( partflow_pool.empty() )
		return( new CPartFlow() );
	
	CPartFlow *ret = *(partflow_pool.begin());
	partflow_pool.pop_front();

	//ret->ReInitialize();
	return( ret );
}

void CPartVis::FreePartFlow( CPartFlow *flow )
{
	if( flow->vertices.capacity() > (PREALLOC*6) )
	{
		// The flow has grown so discard it to prevent infinitely growing vectors
		delete flow;
		flow = NULL;
		return;
	}

	flow->gc_count = 0;
	flow->ReInitialize();
	partflow_pool.push_front( flow );
}

void CPartVis::GCPartFlows()
{

	// Garbage collect:
	if( partflow_pool.size() > 10 )
	{
		//Log( "Begin garbage collection. %d flows in free list", partflow_pool.size() );
		FlowList::iterator i = partflow_pool.begin();
		FlowList::iterator last;
		CPartFlow *tmp;
		for( ; i != partflow_pool.end(); )
		{
			if( (*i)->gc_count == 1 )
			{
				tmp = (*i);
				last = i;
				i++;
				partflow_pool.erase( last );
				delete tmp;
				//continue;
			}
			else
			{
				(*i)->gc_count++;
				i++;
			}
			//i++;
		}
		//Log( "End garbage collection. %d flows in free list", partflow_pool.size() );
	}
}

void CPartVis::BillboardBegin()
{
	/*float modelview[16];

	// Save current view:
	glPushMatrix();

	// Get current view:
	glGetFloatv( GL_MODELVIEW_MATRIX, modelview );

	// Undo all rotations:
	for( int i=0; i<3; i++ )
	{
		for( int j=0; j<3; j++ )
		{
			if( i==j )
				modelview[i*4+j] = 1.0;
			else
				modelview[i*4*j] = 0.0;
		}
	}
	glLoadMatrixf( modelview );*/
}

void CPartVis::BillboardEnd()
{
	//glPopMatrix();
}
