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

