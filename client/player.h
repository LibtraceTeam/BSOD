/* $CVSID$ */ 
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

