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

