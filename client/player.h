/* $Header$ $Log$
/* $Header$ Revision 1.3  2004/02/17 01:59:56  stj2
/* $Header$ Cvs tags will be correct at one point. Surely.
/* $Header$ */ 
#ifndef __PLAYER_H__
#define __PLAYER_H__


#include "camera.h"

class CPlayer : public CActor
{
protected:
	CCamera *cam;
	
public:
	CPlayer();
	virtual ~CPlayer();

	Vector2f	mpos;
	int			invert_mouse;

	virtual void Update(float diff);
	virtual void Draw();
	virtual void Jump();

	virtual void Fire();

	CCamera *GetCamera() { return cam; }
};

#endif

