#ifndef __ACTOR_H__
#define __ACTOR_H__

class CMD3;
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


	
	CMD3		*m_model; 

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

