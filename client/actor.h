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
#ifndef __ACTOR_H__
#define __ACTOR_H__

class CMotion;

#include "entity.h"
#include "octree.h"



class CActor : public CEntity
{
private:

	// Collision Response and Detection functions follow:
	void DoCollision(Vector3f &pos, 
					 Vector3f &new_velocity, 
					 const float diff,
					 int depth = 0);
	bool CheckTree(const COctree::c_node *node, const Vector3f &offset, float width, 
				   const Vector3f &futurePos, Vector3f &collisionPoint);
	bool InNode(const Vector3f &center, const float &width, const Vector3f &futurePos);

public:

	virtual void BeginMovingForward() { movingForward = true; }
	virtual void EndMovingForward() { movingForward = false; }
	virtual void BeginMovingBackward() { movingBackward = true; }
	virtual void EndMovingBackward() { movingBackward = false; }
	virtual void BeginMovingLeft() { movingLeft = true; }
	virtual void EndMovingLeft() { movingLeft = false; }
	virtual void BeginMovingRight() { movingRight = true; }
	virtual void EndMovingRight() { movingRight = false; }


	
	CActor();
	virtual ~CActor();
	
	Vector3f GetBndElpse();	

	bool m_InContact;

	bool Collides(const Vector3f &futurePos, Vector3f &collisionPoint);

	virtual void Draw();
	virtual void Update(float diff);
	virtual void SetGhost(bool value);

	bool CheckTree(const COctree::c_node *node, const Vector3f &offset, float width, 
				   const Vector3f &futurePos, Vector3f &collisionPoint, Vector3f &surfaceNorm);
};

#endif

