/* $Header$ $Log$
/* $Header$ Revision 1.3  2004/02/17 01:59:55  stj2
/* $Header$ Cvs tags will be correct at one point. Surely.
/* $Header$ */ 
#include "stdafx.h"

#include "entity.h"

#include "quaternion.h"
#include "reporter.h"

#include <math.h>

static float acceleration = 1.0f;

CEntity::CEntity()
{
	id = -2;
	position = Vector3f(0,0,0);
	bearing = Vector3f(0,0,0);
	rotation_vel = Vector3f(0,0,0);
	//rotation = CQuaternion();
	velocity = Vector3f(0,0,0);

	// Moving stuff:
	movingForward = false;
	movingBackward = false;
	movingLeft = false;
	movingRight = false;
	// By default entities are not affected by gravity
	m_Ghost = true;
}

CEntity::~CEntity()
{
	list<CEntity *>::iterator iter = children.begin();
	for(; iter != children.end(); ++iter) 
		delete *iter;
}

void CEntity::Draw() {}

#define DO_CHILDREN(func) { list<CEntity *>::iterator iter = children.begin(); \
							for(; iter != children.end(); ++iter) (*iter)->func; }

void CEntity::SetPosition(const Vector3f &pos)
{
	position = pos;

	DO_CHILDREN(SetPosition(pos));
}

void CEntity::SetVelocity(const Vector3f &vel)
{
	velocity = vel;

	DO_CHILDREN(SetVelocity(vel));
}

void CEntity::SetRotationalVelocity(const Vector3f &v)
{
	rotation_vel = v;

	DO_CHILDREN(SetRotationalVelocity(v));
}

void CEntity::SetBearing(const Vector3f &b)
{
	bearing = b;

	DO_CHILDREN(SetBearing(b));
}

void CEntity::Update(float diff)
{
    add = Vector3f();

	if(movingForward)
		MoveForward();
	if(movingBackward)
		MoveBackward();
	if(movingLeft)
		MoveLeft();
	if(movingRight)
		MoveRight();

	if(add.Length() > 0.001f) add.Normalize();

	// Add the vector calculated from moving to our velocity
	SetVelocity(add);
	
	DO_CHILDREN(Update(diff));
}

#undef DO_CHILDREN

void CEntity::MoveForward()
{
	add.z -= cosf(GetBearing().x * DEG_TO_RAD) * cosf(GetBearing().y * DEG_TO_RAD) * acceleration;
	add.x -= cosf(GetBearing().x * DEG_TO_RAD) * sinf(GetBearing().y * DEG_TO_RAD) * acceleration;
	// For more player like movement, we don't allow Y-movement (no flying in other words)
	if(m_Ghost) add.y -= sinf(GetBearing().x * DEG_TO_RAD) * acceleration;
}

void CEntity::MoveBackward()
{
	add.z += cosf(GetBearing().x * DEG_TO_RAD) * cosf(GetBearing().y * DEG_TO_RAD) * acceleration;
	add.x += cosf(GetBearing().x * DEG_TO_RAD) * sinf(GetBearing().y * DEG_TO_RAD) * acceleration;
	// For more player like movement, we don't allow Y-movement (no flying in other words)
	if(m_Ghost) add.y += sinf(GetBearing().x * DEG_TO_RAD) * acceleration;
}

void CEntity::MoveLeft()
{
	add.x += sinf((GetBearing().y - 90.0f)*DEG_TO_RAD) * acceleration;
	add.z += cosf((GetBearing().y - 90.0f)*DEG_TO_RAD) * acceleration;
}

void CEntity::MoveRight()
{
	add.x += sinf((GetBearing().y + 90.0f)*DEG_TO_RAD) * acceleration;	
	add.z += cosf((GetBearing().y + 90.0f)*DEG_TO_RAD) * acceleration;
}


// calculates a bearing given an absolute position to look at
void CEntity::LookAt(const Vector3f &pos)
{
	// we need to figure out the vector from our current position to
	// pos.
	Vector3f rel = pos - position;

	// We are only interested in angles, not magnitude
	Vector3f relzx = rel;
	Vector3f bear(GetBearing());
	relzx.y = 0;
	relzx.Normalize();
	rel.Normalize();

	Vector3f x_axis(0.0f, 0.0f, -1.0f), 
			 y_axis(0.0f, -1.0f, 0.0f);

	float addx = (relzx.x > 0)?-1.0f:1.0f;
	
	// bearing.x - pitch
	// bearing.y - heading

	
	bear.x = -90 + (float)acos((double)rel.Dot(y_axis)) * (180.0f/PI);
	bear.y = addx * (float)acos((double)relzx.Dot(x_axis)) * (180.0f/PI);

	SetBearing(bear);
	
}

float CEntity::GetSpeed() const
{
	return velocity.Length();
}

