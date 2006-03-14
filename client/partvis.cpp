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
//#include <GL/glaux.h>
//#include <GL/glext.h>
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
#include "system_driver.h"
#include "entity_manager.h"
#include "player.h"
#include "action.h"

#include "partflow.h"
#include "partvis.h"

#define MAX_FILTER_STATE 15
// Particle visualisation file


CPartVis::CPartVis( bool mm )
: paused(false)
{
	// Need to create a couple of quads, one with the uni logo, another
	// without the logo.
	//filter_state = -1;
	packetsFrame = 0;
	diff = 0.0f;
	last_gc = world.sys->TimerGetTime();
	show_dark = 0;
	fps = 0.0f;
	global_speed = 1.0f;
	global_alpha = 0.5f; // Default.
	do_gcc = true;
	matrix_mode = mm;
	no_gui = false;
	isHit = false;
	click = false;

	ip[0] = 0;
	ip[1] = 0;
	ip[2] = 0;
	ip[3] = 0;

	pGui = new CGui();

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
		left->tex = CTextureManager::tm.LoadTexture("data/matrix_left.png");
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

	tex_coords.reserve( 6 );
	tex_coords.push_back(Vector2f(1, 1));
	tex_coords.push_back(Vector2f(0, 1));
	tex_coords.push_back(Vector2f(1, 0));

	tex_coords.push_back(Vector2f(0, 1));
	tex_coords.push_back(Vector2f(1, 0));
	tex_coords.push_back(Vector2f(0, 0));
}

// The "container" class implementation
void CPartVis::Draw( bool picking )
{
	//tehMax = 0;
    //FlowMap::const_iterator i = flows.begin();
	IterList::iterator i = active_nodes.begin();
    CDisplayManager *d = world.display;

    // Common state for all flows:
    d->SetBlend(true);
    d->SetBlendMode(CDisplayManager::Transparent2);
    d->SetDepthTest(false);

	int mx = 0, my = 0;
	world.sys->GetMousePos( &mx, &my );

	if( billboard )
	{
		// This is how our point sprite's size will be modified by distance from the viewer.
		// /*
		
		float quadratic[] =  { 1.0f, 0.0f, 0.01f };
		d->SetGLPointParameterfvARB( GL_POINT_DISTANCE_ATTENUATION_ARB, quadratic );

		//------------------------------------------------------------------------+
		glGetFloatv( GL_POINT_SIZE_MAX_ARB, &maxPointSize );

		// Clamp size to 100.0f or the sprites could get a little too big on some  
		// of the newer graphic cards.
		//Log( "MaxSize = %f", maxSize );
		if( maxPointSize > 128.0f )
			maxPointSize = 128.0f;

		glPointSize( maxPointSize );

		// The alpha of a point is calculated to allow the fading of points 
		// instead of shrinking them past a defined threshold size. The threshold 
		// is defined by GL_POINT_FADE_THRESHOLD_SIZE_ARB and is not clamped to 
		// the minimum and maximum point sizes.
		d->SetGLPointParameterfARB( GL_POINT_FADE_THRESHOLD_SIZE_ARB, 60.0f );

		d->SetGLPointParameterfARB( GL_POINT_SIZE_MIN_ARB, 1.0f );
		d->SetGLPointParameterfARB( GL_POINT_SIZE_MAX_ARB, maxPointSize );
		//------------------------------------------------------------------------+

		// Specify point sprite texture coordinate replacement mode for each texture unit
		glTexEnvf( GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE );

		// Render point sprites:
		glEnable( GL_POINT_SPRITE_ARB );
	}

	if( picking )
	{
		GLint viewport[4];

		// Save old projection matrix and reset (identity):
		glMatrixMode( GL_PROJECTION );
		glPushMatrix();
		glLoadIdentity();

		// Set selection buffer:
		glSelectBuffer( SELECT_BUFFER_SIZE, pSelBuff );
		glRenderMode( GL_SELECT );

		// Initialize the OpenGL name stack:
		glInitNames();

		// Set pick matrix:
		glGetIntegerv( GL_VIEWPORT, viewport );
		gluPickMatrix( mx, viewport[3] - my, 15.0, 15.0, viewport );
		gluPerspective(60.0f, (GLfloat)(d->GetWidth())/(GLfloat)(d->GetHeight()), 0.1f, 100.0f);
		glMatrixMode( GL_MODELVIEW );
		glLoadIdentity();
		d->SetCameraTransform( *world.entities->GetPlayer()->GetCamera() );
	}

	for( ; i != active_nodes.end(); ++i )
	{
		(*i)->second->ResetCounter();
		//if( !paused && !picking )
		if( !picking )
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
			/*if( filter_state == -1 )
				(*i)->second->Draw( picking );
			else if( filter_state == (*i)->second->type )
				(*i)->second->Draw( picking );*/
			FlowDescMap::iterator fdmi = fdmap.find((*i)->second->type);
			if( fdmi != fdmap.end() )
			{
				if( fdmi->second->show )
					(*i)->second->Draw( picking );
			}
		}
	}
	if( billboard )
		glDisable( GL_POINT_SPRITE_ARB );
	//BillboardEnd();

	if( picking ) // Done picking.
	{
		int hits;

		// Restore original projection matrix:
		glMatrixMode( GL_PROJECTION );
		glPopMatrix();
		glMatrixMode( GL_MODELVIEW );
		glFlush();
		
		hits = glRenderMode( GL_RENDER );
		//Log( "Hits for click = %d\n", hits );
		if( hits < 1 )
			isHit = false;
		else
			isHit = true;

		if( hits > 0 )
		{
			int choose = pSelBuff[3];
			int depth = pSelBuff[1];

			for( int i=0; i<hits; i++ )
			{
				if( pSelBuff[(i*4)+1] < (GLuint)depth )
				{
					choose = pSelBuff[(i*4)+3];
					depth = pSelBuff[(i*4)+1];
				}
			}

			IPInt2Byte( choose, &ip[0], &ip[1], &ip[2], &ip[3] );

			//Log( "IP chosen: %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3] );
		}

		return;
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

	// Display current toggle state:
	char outstr[256];

	/*if( filter_state == -1 )
		strcpy( outstr, "All packets." );
	else
	{
		FlowDescMap::iterator iter;
		if( (iter = fdmap.find(filter_state) ) != fdmap.end() )
			strcpy( outstr, iter->second->name );
	}*/

	if( show_dark == 0 )
		strcpy( outstr, "Darknetmode: All" );
	else if( show_dark == 1 )
		strcpy( outstr, "Darknetmode: Darknet" );
	else
		strcpy( outstr, "Darknetmode: Sans darknet" );

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

	d->BindTexture(NULL);
	d->SetBlendMode(CDisplayManager::Multiply);
	d->SetBlend(false);
	d->SetColour(1.0f, 1.0f, 1.0f);
	if( matrix_mode )
		d->DrawString2( d->GetWidth() - 75, 5, "MATRIX!");
	/*else
	d->DrawString2( 5, 5, "F1 for help.");*/

	d->BindTexture(NULL);
	d->SetBlendMode(CDisplayManager::Multiply);
	d->SetBlend(false);
	d->SetColour(1.0f, 1.0f, 1.0f);
	d->DrawString2((int)(x+3), (int)(y+1), str);

	if( world.actionHandler->rmb_down && isHit )
	{
		outstr[0] = '\0';
		sprintf( outstr, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3] );

		d->BindTexture(NULL);
		d->SetBlendMode(CDisplayManager::Multiply);
		d->SetBlend(true);
		d->SetColour( 0.0f, 0.0f, 0.3f, 0.5f );
		d->Draw2DQuad( mx, my-25, mx + ((int)(strlen(outstr))*10), my-5 );

		d->SetBlend(false);
		d->SetColour(1.0f, 1.0f, 1.0f);
		d->DrawString2(mx+3, my-23, outstr);
	}

	d->End2D();

	// Lastly draw the GUI:
	if( !no_gui )
		pGui->Draw( 5, 5, mx, my, click );

	click = false;

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

void CPartVis::UpdateFlow(unsigned int flow_id, Vector3f v1, Vector3f v2, uint32 ip1, uint32 ip2 )
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

		if( flow->start.x < flow->destination.x )
		{
			flow->ip1 = ip1;
			flow->ip2 = ip2;
		}
		else
		{
			flow->ip1 = ip2;
			flow->ip2 = ip1;
		}

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
	if( paused ) // Don't add packets when paused.
		return;

    FlowMap::iterator i = flows.find(flow_id);

	if(i == flows.end()) 
	{
		return;//Log("Adding packet to non-existant flow %d\n", flow_id);
	} 
	else 
	{
		CPartFlow *flow = i->second;

		// XXX: at this point we could use the time to make sure the 
		// particle is in the correct position; if we have lost some amount
		// of time it might not make sense to have this particle start at
		// the start position. To make this useful we would really need
		// time more accurate than second accuracy, though.

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
	if( paused )
	{
		flows_to_remove.push_back( id );
		return;
	}

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

	// if not paused, remove all flows stored in list.
	if( !paused ) // Unpaused, so remove any flows we marked for removal.
	{
		for( FlowIDList::iterator i = flows_to_remove.begin(); i != flows_to_remove.end(); i++ )
			RemoveFlow( *i );

		flows_to_remove.clear();
	}
}

void CPartVis::ToggleFilter()
{
	/*filter_state++;
	FlowDescMap::iterator iter = fdmap.find( filter_state );
	if( iter == fdmap.end() )
		filter_state = -1;
	if( (iter->second->colour[0] == 0) && (iter->second->colour[1] == 0) && (iter->second->colour[2] == 0) )
		filter_state = -1;*/
}

void CPartVis::ToggleBackFilter()
{
	/*filter_state--;
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
		
	}*/
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
