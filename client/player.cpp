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
