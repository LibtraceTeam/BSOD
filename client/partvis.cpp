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
	filter_state = 0;
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

	// Alternative draw:
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

				// DEBUG
				//if( (*i)->second->active_flow_ptr == NULL )
				//	exit(-99);
				// END_DEBUG

				active_nodes.erase( (*i)->second->active_flow_ptr );
				//(*i)->second->active_flow_ptr = NULL;
				i = tmp;
				continue;
			}
		}

		/*if( matrix_mode )
		{
			(*i)->second->Draw();
		}
		else */
		if( (show_dark == 0) || ( (show_dark == 1) && (*i)->second->dark) || ( (show_dark == 2) && !(*i)->second->dark) ) 
		{
			switch( filter_state ) 
			{
			case 0: // Display all packets:
				(*i)->second->Draw();
				break;
			case 1: // Display only packets with RTT data:
				if( (*i)->second->speed != 1.0f )
					(*i)->second->Draw();
				break;
			case 2: // TCP
				if( (*i)->second->flow_colour[0] == 100 && (*i)->second->flow_colour[1] == 0 && (*i)->second->flow_colour[2] == 100 )
					(*i)->second->Draw();
				break;
			case 3: // HTTP
				if( (*i)->second->flow_colour[0] == 0 && (*i)->second->flow_colour[1] == 0 && (*i)->second->flow_colour[2] == 200 )
					(*i)->second->Draw();
				break;
			case 4: // HTTPS
				if( (*i)->second->flow_colour[0] == 150 && (*i)->second->flow_colour[1] == 150 && (*i)->second->flow_colour[2] == 240 )
					(*i)->second->Draw();
				break;
			case 5: // MAIL
				if( (*i)->second->flow_colour[0] == 200 && (*i)->second->flow_colour[1] == 0 && (*i)->second->flow_colour[2] == 0 )
					(*i)->second->Draw();
				break;
			case 6: // FTP
				if( (*i)->second->flow_colour[0] == 0 && (*i)->second->flow_colour[1] == 150 && (*i)->second->flow_colour[2] == 0 )
					(*i)->second->Draw();
				break;
			case 7: // VPN
				if( (*i)->second->flow_colour[0] == 0 && (*i)->second->flow_colour[1] == 250 && (*i)->second->flow_colour[2] == 0 )
					(*i)->second->Draw();
				break;
			case 8: // DNS
				if( (*i)->second->flow_colour[0] == 200 && (*i)->second->flow_colour[1] == 200 && (*i)->second->flow_colour[2] == 0 )
					(*i)->second->Draw();
				break;
			case 9: // NTP
				if( (*i)->second->flow_colour[0] == 30 && (*i)->second->flow_colour[1] == 85 && (*i)->second->flow_colour[2] == 30 )
					(*i)->second->Draw();
				break;
			case 10: // SSH
				if( (*i)->second->flow_colour[0] == 110 && (*i)->second->flow_colour[1] == 110 && (*i)->second->flow_colour[2] == 110 )
					(*i)->second->Draw();
				break;
			case 11: // UDP
				if( (*i)->second->flow_colour[0] == 150 && (*i)->second->flow_colour[1] == 100 && (*i)->second->flow_colour[2] == 50 )
					(*i)->second->Draw();
				break;
			case 12: // ICMP
				if( (*i)->second->flow_colour[0] == 0 && (*i)->second->flow_colour[1] == 250 && (*i)->second->flow_colour[2] == 200 )
					(*i)->second->Draw();
				break;
			case 13: // IRC
				if( (*i)->second->flow_colour[0] == 240 && (*i)->second->flow_colour[1] == 230 && (*i)->second->flow_colour[2] == 140 )
					(*i)->second->Draw();
				break;
			case 14: // Windows
				if( (*i)->second->flow_colour[0] == 200 && (*i)->second->flow_colour[1] == 100 && (*i)->second->flow_colour[2] == 0 )
					(*i)->second->Draw();
				break;
			case 15: // P2P
				if( (*i)->second->flow_colour[0] == 50 && (*i)->second->flow_colour[1] == 150 && (*i)->second->flow_colour[2] == 50 )
					(*i)->second->Draw();
				break;
			case 16: // Other
				if( (*i)->second->flow_colour[0] == 255 && (*i)->second->flow_colour[1] == 192 && (*i)->second->flow_colour[2] == 203 )
					(*i)->second->Draw();
				break;
			default: // All packets are shown:
				(*i)->second->Draw();
			}
		}
	}
	//BillboardEnd();
    
    // Draw the flows
	/*for( ; i != flows.end(); ++i )
	{
		i->second->ResetCounter();
		if( !paused )
		{
			i->second->Update(diff);
			if( i->second->packets == 0 )
				RemoveActiveFlow( i );
		}

		if( (show_dark == 0) || ( (show_dark == 1) && i->second->dark) || ( (show_dark == 2) && !i->second->dark) ) 
		{
			switch( filter_state ) 
			{
			case 0: // Display all packets:
				i->second->Draw();
				break;
			case 1: // Display only packets with RTT data:
				if( i->second->speed != 1.0f )
					i->second->Draw();
				break;
			case 2: // TCP
				if( i->second->colours[0] == 100 && i->second->colours[1] == 0 && i->second->colours[2] == 100 )
					i->second->Draw();
				break;
			case 3: // HTTP
				if( i->second->colours[0] == 0 && i->second->colours[1] == 0 && i->second->colours[2] == 200 )
					i->second->Draw();
				break;
			case 4: // HTTPS
				if( i->second->colours[0] == 150 && i->second->colours[1] == 150 && i->second->colours[2] == 240 )
					i->second->Draw();
				break;
			case 5: // MAIL
				if( i->second->colours[0] == 200 && i->second->colours[1] == 0 && i->second->colours[2] == 0 )
					i->second->Draw();
				break;
			case 6: // FTP
				if( i->second->colours[0] == 0 && i->second->colours[1] == 150 && i->second->colours[2] == 0 )
					i->second->Draw();
				break;
			case 7: // VPN
				if( i->second->colours[0] == 0 && i->second->colours[1] == 250 && i->second->colours[2] == 0 )
					i->second->Draw();
				break;
			case 8: // DNS
				if( i->second->colours[0] == 200 && i->second->colours[1] == 200 && i->second->colours[2] == 0 )
					i->second->Draw();
				break;
			case 9: // NTP
				if( i->second->colours[0] == 30 && i->second->colours[1] == 85 && i->second->colours[2] == 30 )
					i->second->Draw();
				break;
			case 10: // SSH
				if( i->second->colours[0] == 110 && i->second->colours[1] == 110 && i->second->colours[2] == 110 )
					i->second->Draw();
				break;
			case 11: // UDP
				if( i->second->colours[0] == 150 && i->second->colours[1] == 100 && i->second->colours[2] == 50 )
					i->second->Draw();
				break;
			case 12: // ICMP
				if( i->second->colours[0] == 0 && i->second->colours[1] == 250 && i->second->colours[2] == 200 )
					i->second->Draw();
				break;
			case 13: // IRC
				if( i->second->colours[0] == 240 && i->second->colours[1] == 230 && i->second->colours[2] == 140 )
					i->second->Draw();
				break;
			case 14: // Windows
				if( i->second->colours[0] == 200 && i->second->colours[1] == 100 && i->second->colours[2] == 0 )
					i->second->Draw();
				break;
			case 15: // P2P
				if( i->second->colours[0] == 50 && i->second->colours[1] == 150 && i->second->colours[2] == 50 )
					i->second->Draw();
				break;
			case 16: // Other
				if( i->second->colours[0] == 255 && i->second->colours[1] == 192 && i->second->colours[2] == 203 )
					i->second->Draw();
				break;
			default: // All packets are shown:
				i->second->Draw();
			}
		}
	}*/
    /*for(; i != flows.end(); ++i) 
	{
		i->second->Draw();
    }*/

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
	switch( filter_state ) 
	{
	case 0: // Display all packets:
		strcpy( outstr, "All packets." );
		break;
	case 1: // Display only packets with RTT data:
		strcpy( outstr, "Flows with RTT data." );	
		break;
	case 2: // TCP
		strcpy( outstr, "TCP." );
		break;
	case 3: // HTTP
		strcpy( outstr, "HTTP." );
		break;
	case 4: // HTTPS
		strcpy( outstr, "HTTPS." );
		break;
	case 5: // MAIL
		strcpy( outstr, "Mail." );
		break;
	case 6: // FTP
		strcpy( outstr, "FTP." );
		break;
	case 7: // VPN
		strcpy( outstr, "VPN." );
		break;
	case 8: // DNS
		strcpy( outstr, "DNS." );
		break;
	case 9: // NTP
		strcpy( outstr, "NTP." );
		break;
	case 10: // SSH
		strcpy( outstr, "SSH." );
		break;
	case 11: // UDP
		strcpy( outstr, "UDP." );
		break;
	case 12: // ICMP
		strcpy( outstr, "ICMP." );
		break;
	case 13: // IRC
		strcpy( outstr, "IRC." );
		break;
	case 14: // Windows
		strcpy( outstr, "Windows." );
		break;
	case 15: // P2P
		strcpy( outstr, "P2P." );
		break;
	case 16: // Other
		strcpy( outstr, "Other." );
		break;
	default: // All packets are shown:
		strcpy( outstr, "All packets." );
	}

	if( show_dark == 0 )
		strcat( outstr, " - All" );
	else if( show_dark == 1 )
		strcat( outstr, " - Darknet" );
	else
		strcat( outstr, " - Sans darknet" );

	float w = 330, h = 40;
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
		if( (world.sys->TimerGetTime() - last_gc) > 360000.0f )
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
		//flow->CreateEndPoints();
		/*if( matrix_mode )
		{
			// Pick random matrix code character (this is per flow only atm)
			// Might be able to update to be per char or maybe based on flow
			// type (HTTP/UDP/TCP/etc...)
			int select = rand()%9;
			switch( select ) 
			{
			case 0:
				flow->tex = CTextureManager::tm.LoadTexture("data/matrix_01.png");
				break;
			case 1:
				flow->tex = CTextureManager::tm.LoadTexture("data/matrix_02.png");
				break;
			case 2:
				flow->tex = CTextureManager::tm.LoadTexture("data/matrix_03.png");
				break;
			case 3:
				flow->tex = CTextureManager::tm.LoadTexture("data/matrix_04.png");
				break;
			case 4:
				flow->tex = CTextureManager::tm.LoadTexture("data/matrix_05.png");
				break;
			case 5:
				flow->tex = CTextureManager::tm.LoadTexture("data/matrix_06.png");
				break;
			case 6:
				flow->tex = CTextureManager::tm.LoadTexture("data/matrix_07.png");
				break;
			case 7:
				flow->tex = CTextureManager::tm.LoadTexture("data/matrix_08.png");
				break;
			case 8:
				flow->tex = CTextureManager::tm.LoadTexture("data/matrix_09.png");
				break;
			case 9:
				flow->tex = CTextureManager::tm.LoadTexture("data/matrix_10.png");
				break;
			default:
				flow->tex = CTextureManager::tm.LoadTexture("data/matrix_10.png");
				break;
			}
		}
		else
			flow->tex = CTextureManager::tm.LoadTexture(particle_img); */
		flows.insert(FlowMap::value_type(flow_id, flow));
	/*} else {
		// Found
		Log("UpdateFlow called on flow that already exits (flow=%d)\n",
			flow_id);
    }*/
}

void CPartVis::UpdatePacket(unsigned int flow_id, uint32 timestamp, byte r,
	byte g, byte b, unsigned short size, float speed, bool dark)
{
    FlowMap::iterator i = flows.find(flow_id);

	if(i == flows.end()) {
		return;//Log("Adding packet to non-existant flow %d\n", flow_id);
	} else {
		CPartFlow *flow = i->second;

		//flow->Update_ServerTime(timestamp);
		// XXX: at this point we could use the time to make sure the 
		// particle is in the correct position; if we have lost some amount
		// of time it might not make sense to have this particle start at
		// the start position. To make this useful we would really need
		// time more accurate than second accuracy, though.

		//flow->MoveParticles();
		
		// Log( "Flow speedz: %f", speed );
		// Log( "R = %c G = %c B = %c", r, g, b );
		flow->flow_colour[0] = r;
		flow->flow_colour[1] = g;
		flow->flow_colour[2] = b;
		if( matrix_mode )
		{
			r = b = 36;
			g = 132;
		}

		if( flow->AddParticle(r, g, b, size, speed, dark) && (flow->packets == 1) )
		{
			flow->active_flow_ptr = AddActiveFlow( i );
		}
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
    /*FlowMap::iterator i = flows.begin();
    for(; i != flows.end(); ++i)
	{
		//if( (int)(i->second->vertices.size()) == 0 )
		//	RemoveFlow( i->first );
		//else
			i->second->ResetCounter();
	}*/
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
	if( filter_state > MAX_FILTER_STATE )
		filter_state = 0;
}

void CPartVis::ToggleBackFilter()
{
	filter_state--;
	if( filter_state < 0 )
		filter_state = MAX_FILTER_STATE;
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

	ret->ReInitialize();
	return( ret );
}

void CPartVis::FreePartFlow( CPartFlow *flow )
{
	flow->gc_count = 0;
	partflow_pool.push_front( flow );
}

void CPartVis::GCPartFlows()
{

	// Garbage collect:
	if( partflow_pool.size() > 10 )
	{
		FlowList::iterator i = partflow_pool.begin();
		FlowList::iterator last;
		CPartFlow *tmp;
		for( ; i != partflow_pool.end(); )
		{
			if( (*i)->gc_count == 2 )
			{
				tmp = (*i);
				last = i;
				i++;
				partflow_pool.erase( last );
				delete tmp;
				continue;
			}
			else
			{
				(*i)->gc_count++;
			}
			i++;
		}
	}

	// If there are oer than 10 items in the pool clean some out:
	/*if( partflow_pool.size() > 10 )
	{
		CPartFlow *ptr;
		for( int i=0; i<10; i++ )
		{
			ptr = partflow_pool.begin();
			partflow_pool.pop_front();
			delete ptr;
		}
	}*/
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
