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
#ifndef _COLLIDER_H_
#define _COLLIDER_H_

class CReader;

// Abstract base class for collision detection
class CCollider
{
public:

	// XXX: is this ever used? -Sam
	struct ContactInfo
	{
		Vector3f	position;
		Vector3f	surfaceNormal;
		float		depth;
	};

	// XXX: why do we still have collides here? isn't it all in SphereCollide now? -Sam
	virtual bool Collides(const CCollider *other, const Vector3f &position, 
						  Vector3f &collisionPoint, float &depth) = 0;
	virtual void BuildCollisionInfo(CMesh *mesh) = 0;
	virtual ~CCollider() {}

	// This is here so we dont need to reference Opcode at all outside the one collision
	// detection file.
	static CCollider *Create();
	// Init the collision detection system
	static void Initialise();

	// Write the built info to disk so it doesn't need to be recalculated
	virtual void WriteToDisk(FILE *out) = 0;
	virtual void LoadFromDisk(CReader *r, CMesh *m) = 0;
	
	// Collide sphere with the world.  Returns whether there was a collision.  If there was
	// a collision, it adds contact information into the contact list (contacts).
	virtual bool SphereCollide(const CCollider *other, const Vector3f &position, 
							   const float radius, Vector3f &collisionPoint,
							   float &depth) = 0;
	
	// TODO: implement RayCollide when we find the need for it -Sam
	/*virtual bool RayCollide(const CCollider *other, const Vector3f &start,
							const Vector3f &end, Vector3f &collisionPoint);*/

	virtual void Dump() = 0;
};

/** CDummyCollider is used for objects that are not collided with: eg. a light volume
    or similar.
 */
class CDummyCollider : public CCollider
{
public:
	virtual bool Collides(const CCollider *other, const Vector3f &position, 
						  Vector3f &collisionPoint, float &depth)
	{		return false; 	}
	virtual void BuildCollisionInfo(CMesh *mesh) { }
	virtual ~CDummyCollider() {}

	// Collide sphere with the world.  Returns whether there was a collision.  If there was
	// a collision, it adds contact information into the contact list (contacts).
	virtual bool SphereCollide(const CCollider *other, const Vector3f &position, 
							   const float radius, list<ContactInfo> &contacts)
	{		return false;	}

	virtual void Dump() {}
	
};


#endif

