/* $Header$ $Log$
/* $Header: /home/cvs/wand-general/bsod/client/player.cpp,v 1.3 2004/02/17 01:59:56 stj2 Exp $ Revision 1.4  2004/02/17 02:05:21  stj2
/* $Header: /home/cvs/wand-general/bsod/client/player.cpp,v 1.3 2004/02/17 01:59:56 stj2 Exp $ Still trying to get cvs tags correct.
/* $Header: /home/cvs/wand-general/bsod/client/player.cpp,v 1.3 2004/02/17 01:59:56 stj2 Exp $
 $Header$ Revision 1.3  2004/02/17 01:59:56  stj2
 $Header$ Cvs tags will be correct at one point. Surely.
 $Header$ */ 
#include "stdafx.h"
#include "world.h"
#include "player.h"
#include "entity_manager.h"
#include "reporter.h"

CPlayer::CPlayer()
	: CActor()
{
	cam = new CCamera();
	AddChild(cam);

	m_Ghost = false;
	invert_mouse = 1;
}

CPlayer::~CPlayer()
{
}

void CPlayer::Update(float diff)
{
	CActor::Update(diff);

    // The number 10 below is the mouse sensitivity
	SetBearing(Vector3f(GetBearing().x + (mpos.x / 10.0f) * invert_mouse,
					    GetBearing().y + (mpos.y / 10.0f),
						GetBearing().z));

	if( GetBearing().x > 90.0f ) 
		SetBearing(Vector3f(90.0f, GetBearing().y, GetBearing().z));
	else if(GetBearing().x < -90.0f) 
		SetBearing(Vector3f(-90.0f, GetBearing().y, GetBearing().z));
}

void CPlayer::Draw()
{

}

void CPlayer::Jump()
{
	//if(this->m_InContact)
	SetVelocity(GetVelocity() + Vector3f(0.0f, 5.0f, 0.0f));
}


void CPlayer::Fire()
{
}
