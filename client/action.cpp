/* 
 $Header$ $Log$
 $Header: /home/cvs/wand-general/bsod/client/action.cpp,v 1.3 2004/02/17 01:59:55 stj2 Exp $ Revision 1.4  2004/02/17 02:05:21  stj2
 $Header: /home/cvs/wand-general/bsod/client/action.cpp,v 1.3 2004/02/17 01:59:55 stj2 Exp $ Still trying to get cvs tags correct.
 $Header: /home/cvs/wand-general/bsod/client/action.cpp,v 1.3 2004/02/17 01:59:55 stj2 Exp $
 $Header$ Revision 1.3  2004/02/17 01:59:55  stj2
 $Header$ Cvs tags will be correct at one point. Surely.
 $Header$ 
*/ 
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

#include <hash_map>

// One thing I forgot about when implementing this are the mouse buttons (and mouse wheel)
// They aren't exactly keys so dont really fit in with 'keycode'.  However, they may as
// well just go in as Keycodes.  It's just a bad name.  And stuff. - Sam

typedef void (CActionHandler::*ActionFunc)();

// For some reason g++ just wont let you have an enum in a hash_map.  Is it because enums
// are constants or something?  I don't know.  But because of g++, we need to use ints in
// our hash_map.
//typedef hash_map<CActionHandler::Keycode, ActionFunc> ActionMap;
typedef hash_map<int, ActionFunc> ActionMap;

// Hopefully this is fast enough.  Its a hash-table, so it should be.
// -- It all seems fast enough.  No reason to change certainly. - Sam
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

	keyDownMap[ BKC_ESCAPE ] = &CActionHandler::Quit;
	keyDownMap[ BKC_Q ] = &CActionHandler::Quit;

	keyUpMap[ BKC_R ] = &CActionHandler::ToggleWireframe;
	keyUpMap[ BKC_B ] = &CActionHandler::ToggleBackfaceCull;
	keyUpMap[ BKC_O ] = &CActionHandler::ToggleOctreeBoxes;
	keyDownMap[ BKC_SPACE ] = &CActionHandler::Jump;

	keyDownMap[ BKC_G ] = &CActionHandler::ToggleGhostMode;

	keyDownMap[ BKC_LEFTMOUSEBUT ] = &CActionHandler::Fire;
}

void CActionHandler::KeyDown(Keycode key)
{
	ActionMap::const_iterator i = keyDownMap.find(key);
	
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

void CActionHandler::Jump()
{
	world.entities->GetPlayer()->Jump();
}

void CActionHandler::Fire()
{
	world.entities->GetPlayer()->Fire();
}

void CActionHandler::ToggleGhostMode()
{
	world.entities->GetPlayer()->m_Ghost = !world.entities->GetPlayer()->m_Ghost;
}
