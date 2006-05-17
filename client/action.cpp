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
// $Id$
#include "stdafx.h"
#include "action.h"
#include "entity_manager.h"
#include "camera.h"
#include "world.h"
#include "system_driver.h"
#include "display_manager.h"
#include "octree.h"
#include "reporter.h"
#include "player.h"
#include "texture_manager.h"
#include "partflow.h"
#include "partvis.h"

#ifdef NO_STLPORT
#include <map>
#endif

#ifdef _WIN32
#pragma warning( disable : 4996 )
#endif
// One thing I forgot about when implementing this are the mouse buttons (and mouse wheel)
// They aren't exactly keys so dont really fit in with 'keycode'.  However, they may as
// well just go in as Keycodes.  It's just a bad name.  And stuff. - Sam

typedef void (CActionHandler::*ActionFunc)();

// For some reason g++ just wont let you have an enum in a hash_map.  Is it because enums
// are constants or something?  I don't know.  But because of g++, we need to use ints in
// our hash_map.
//typedef hash_map<CActionHandler::Keycode, ActionFunc> ActionMap;
typedef map<int, ActionFunc> ActionMap;

ActionMap keyDownMap;
ActionMap keyUpMap;

CActionHandler::CActionHandler()
{
	keyDownMap[ BKC_W ] = &CActionHandler::BeginMovingForward;	
	keyUpMap[ BKC_W ] = &CActionHandler::EndMovingForward;
	
	keyDownMap[ BKC_S ] = &CActionHandler::BeginMovingBackward;	
	keyUpMap[ BKC_S ] = &CActionHandler::EndMovingBackward;
	
	keyDownMap[ BKC_A ] = &CActionHandler::BeginStrafingLeft;	
	keyUpMap[ BKC_A ] = &CActionHandler::EndStrafingLeft;
	
	keyDownMap[ BKC_D ] = &CActionHandler::BeginStrafingRight;	
	keyUpMap[ BKC_D ] = &CActionHandler::EndStrafingRight;

	keyUpMap[ BKC_UP ] = &CActionHandler::TurnUp;
	keyUpMap[ BKC_DOWN ] = &CActionHandler::TurnDown;
	keyUpMap[ BKC_LEFT ] = &CActionHandler::TurnLeft;
	keyUpMap[ BKC_RIGHT ] = &CActionHandler::TurnRight;

	keyUpMap[ BKC_K ] = &CActionHandler::TurnUp;
	keyUpMap[ BKC_I ] = &CActionHandler::TurnDown;
	keyUpMap[ BKC_J ] = &CActionHandler::TurnLeft;
	keyUpMap[ BKC_L ] = &CActionHandler::TurnRight;

	keyDownMap[ BKC_ESCAPE ] = &CActionHandler::Quit;
	//keyDownMap[ BKC_Q ] = &CActionHandler::Quit;

	keyUpMap[ BKC_R ] = &CActionHandler::ToggleWireframe;
	keyUpMap[ BKC_B ] = &CActionHandler::ToggleBackfaceCull;
	//keyUpMap[ BKC_O ] = &CActionHandler::ToggleOctreeBoxes;
	keyDownMap[ BKC_SPACE ] = &CActionHandler::Pause;

	keyDownMap[ BKC_G ] = &CActionHandler::ToggleGhostMode;

	keyDownMap[ BKC_LEFTMOUSEBUT ] = &CActionHandler::Navigate;
	keyUpMap[ BKC_LEFTMOUSEBUT ]   = &CActionHandler::EndNavigate;

	keyDownMap[ BKC_RIGHTMOUSEBUT ] = &CActionHandler::Pick;
	keyUpMap[ BKC_RIGHTMOUSEBUT ]	= &CActionHandler::EndPick;

	keyUpMap[ BKC_M ] = &CActionHandler::Screenshot;
	keyUpMap[ BKC_H ] = &CActionHandler::ToggleDebugDisplay;

	keyUpMap[ BKC_PERIOD ] = &CActionHandler::ToggleFilter;
	keyUpMap[ BKC_COMMA ] = &CActionHandler::ToggleBackFilter;
	keyUpMap[ BKC_SLASH ] = &CActionHandler::ToggleShowDark;
	keyUpMap[ BKC_EQUALS ] = &CActionHandler::Faster;
	keyUpMap[ BKC_MINUS ] = &CActionHandler::Slower;

	keyUpMap[ BKC_MOUSESCROLLUP ]   = &CActionHandler::ZoomIn;
	keyUpMap[ BKC_MOUSESCROLLDOWN ] = &CActionHandler::ZoomOut;

	keyDownMap[ BKC_P ] = &CActionHandler::SingularityPick;
	keyUpMap[ BKC_P ] = &CActionHandler::EndSingularityPick;
	keyUpMap[ BKC_O ] = &CActionHandler::UnlockSingularity;

	lmb_down = false; // Left mouse button not down yet.
	rmb_down = false;
	gui_open = false;
	s_pick = false;
	no_cursor = false;
	screen_saver = false;
}

void CActionHandler::KeyDown(Keycode key)
{
	ActionMap::const_iterator i = keyDownMap.find(key);
	world.partVis->pGui->OnKeyDown( (unsigned short)key ); // Notify GUI of event. (Slightly hax, should really be some sort of flexible event structure in BSOD).
	
	if(i == keyDownMap.end()) {
		// Not found!
	} else {
		(this->*(*i).second)();
	}
}

void CActionHandler::KeyUp(Keycode key)
{
	ActionMap::iterator i = keyUpMap.find(key);
	
	if(i == keyUpMap.end()) {
		// Not found!
	} else {
		(this->*(*i).second)();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
// Possible actions follow
void CActionHandler::BeginMovingForward()
{
	world.entities->GetPlayer()->BeginMovingForward();
}

void CActionHandler::EndMovingForward()
{
	world.entities->GetPlayer()->EndMovingForward();
}

void CActionHandler::BeginMovingBackward()
{
	world.entities->GetPlayer()->BeginMovingBackward();
}

void CActionHandler::EndMovingBackward()
{
	world.entities->GetPlayer()->EndMovingBackward();
}

void CActionHandler::Quit()
{
	world.sys->Quit();
}

void CActionHandler::BeginStrafingLeft()
{
	world.entities->GetPlayer()->BeginMovingLeft();
}

void CActionHandler::EndStrafingLeft()
{
	world.entities->GetPlayer()->EndMovingLeft();
}

void CActionHandler::BeginStrafingRight()
{
	world.entities->GetPlayer()->BeginMovingRight();
}

void CActionHandler::EndStrafingRight()
{
	world.entities->GetPlayer()->EndMovingRight();
}

void CActionHandler::ToggleWireframe()
{
	world.display->SetWireframe(!world.display->GetWireframe());
}

void CActionHandler::ToggleBackfaceCull()
{
	world.display->SetBackfaceCull(!world.display->GetBackfaceCull());
}

void CActionHandler::ToggleOctreeBoxes()
{
	world.tree->drawOctreeBoxes = !world.tree->drawOctreeBoxes;
}

void CActionHandler::Pause()
{
    world.partVis->TogglePaused();
}

void CActionHandler::Fire()
{
	world.entities->GetPlayer()->Fire();
}

void CActionHandler::ToggleGhostMode()
{
	world.entities->GetPlayer()->m_Ghost = !world.entities->GetPlayer()->m_Ghost;
}

void CActionHandler::Screenshot()
{
    char buffer[1024];
    static unsigned int ss_num = 0;
#ifdef _WIN32
	_snprintf
#else
	snprintf
#endif
		(buffer,sizeof(buffer),"screenshot-%u-%u.png",
            (unsigned int)time(NULL), ++ss_num);
    CTextureManager::tm.SaveScreenshot(buffer);
}

void CActionHandler::ToggleDebugDisplay()
{
    world.debug_display = !world.debug_display;
}

void CActionHandler::TurnDown()
{
	//if( world.actionHandler->screen_saver )
	//	world.sys->Quit();

	if( gui_open )
		return;

		/*SetBearing(Vector3f(GetBearing().x + (mpos.x / 10.0f) * invert_mouse,
					    GetBearing().y + (mpos.y / 10.0f),
						GetBearing().z));*/
	Vector3f b(world.entities->GetPlayer()->GetBearing());
	world.entities->GetPlayer()->SetBearing(
		Vector3f(b.x-3, b.y, b.z));
}

void CActionHandler::TurnUp()
{
	//if( world.actionHandler->screen_saver )
	//	world.sys->Quit();

	if( gui_open )
		return;

	Vector3f b(world.entities->GetPlayer()->GetBearing());
	world.entities->GetPlayer()->SetBearing(
		Vector3f(b.x+3, b.y, b.z));
}

void CActionHandler::TurnLeft()
{
	//if( world.actionHandler->screen_saver )
	//	world.sys->Quit();

	if( gui_open )
		return;

	Vector3f b(world.entities->GetPlayer()->GetBearing());
	world.entities->GetPlayer()->SetBearing(
		Vector3f(b.x, b.y+3, b.z));
}

void CActionHandler::TurnRight()
{
	//if( world.actionHandler->screen_saver )
	//	world.sys->Quit();

	if( gui_open )
		return;

	Vector3f b(world.entities->GetPlayer()->GetBearing());
	world.entities->GetPlayer()->SetBearing(
		Vector3f(b.x, b.y-3, b.z));
}

void CActionHandler::ToggleFilter()
{
	world.partVis->ToggleFilter();
}

void CActionHandler::ToggleBackFilter()
{
	world.partVis->ToggleBackFilter();
}

void CActionHandler::ToggleShowDark()
{
	world.partVis->ToggleShowDark();
}

void CActionHandler::Faster()
{
	world.partVis->ChangeSpeed( true );
}

void CActionHandler::Slower()
{
	world.partVis->ChangeSpeed( false );
}

void CActionHandler::Navigate()
{
	lmb_down = true;
}

void CActionHandler::EndNavigate()
{
	lmb_down = false;
	world.partVis->click = true;
}

void CActionHandler::Pick()
{
	rmb_down = true;
}

void CActionHandler::EndPick()
{
	rmb_down = false;
}

void CActionHandler::ZoomIn()
{
}

void CActionHandler::ZoomOut()
{
}

void CActionHandler::SingularityPick()
{
	s_pick = true;
}

void CActionHandler::EndSingularityPick()
{
	s_pick = false;
}

void CActionHandler::UnlockSingularity()
{
	world.partVis->singularity = false;
}

