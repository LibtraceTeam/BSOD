/* $CVSID$ */ 
#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "quaternion.h"

class CEntity
{
	// TODO: make all members private: not protected. Everything should be done by accessor functions and stuff
private:
	Vector3f position;
	Vector3f bearing; // pitch and heading
	Vector3f velocity;
	Vector3f rotation_vel;
	list<CEntity *> children;

protected:
	bool  movingForward;
	bool  movingBackward;
	bool  movingLeft;
	bool  movingRight;

	virtual void MoveForward();
	virtual void MoveBackward();
	virtual void MoveLeft();
	virtual void MoveRight();

	Vector3f add;

public:
//	CQuaternion rotation;

	int		 id;
	
	// TODO: make m_Ghost not public and stuff
	bool m_Ghost;

	CEntity();
	virtual ~CEntity();

	// camera-like methods
	virtual const Vector3f &GetPosition() const { return position; }
	virtual const Vector3f &GetBearing() const { return bearing; }
	virtual const Vector3f &GetVelocity() const { return velocity; }
	virtual const Vector3f &GetRotationalVelocity() const { 
		return rotation_vel; };

	virtual void SetPosition(const Vector3f &pos);
	virtual void SetVelocity(const Vector3f &vel);
	virtual void SetBearing(const Vector3f &b);
	virtual void SetRotationalVelocity(const Vector3f &v);

	virtual float GetSpeed() const;

	virtual void Draw();
	virtual void LookAt(const Vector3f &pos); // calculates a bearing given an absolute position to look at
	virtual void AddChild(CEntity *e) { children.push_back(e); }

	// TODO: make virtual functions to change/get velocity as well (this is/will be neeed
	// in some derived classes).

	// All chaning in bearing and such should be via virtual functions as well. This allows
	// derived classes to actually be useful...
	
	virtual void Update(float diff);
};

#endif // __ENTITY_H__

