/* $CVSID$ */ 
#ifndef __CAMERA_H__
#define __CAMERA_H__


#include "actor.h"
class CMatrix4f;



class CCamera : public CActor
{
protected:
	
public:
	CCamera();

	char		buffer[256];

	float xpos; // ???
	float zpos; // ???
	float ypos; // ???
	CPlane frustum[6];

	virtual void Draw();

	virtual void CalculateFrustumColumnMajor(CMatrix4f &m);
	virtual void CalculateFrustumRowMajor(float data[4][4]);
	virtual bool IsCubeInFrustum(const Vector3f &centre, float length);

	virtual void Update(float diff);
};

#endif

