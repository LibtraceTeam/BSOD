/* $Header$ $Log$
/* $Header: /home/cvs/wand-general/bsod/client/camera.cpp,v 1.3 2004/02/17 01:59:55 stj2 Exp $ Revision 1.4  2004/02/17 02:05:21  stj2
/* $Header: /home/cvs/wand-general/bsod/client/camera.cpp,v 1.3 2004/02/17 01:59:55 stj2 Exp $ Still trying to get cvs tags correct.
/* $Header: /home/cvs/wand-general/bsod/client/camera.cpp,v 1.3 2004/02/17 01:59:55 stj2 Exp $
 $Header$ Revision 1.3  2004/02/17 01:59:55  stj2
 $Header$ Cvs tags will be correct at one point. Surely.
 $Header$ */ 
#include "stdafx.h"

#include <math.h>
#include "world.h"
#include "display_manager.h"
#include "matrix.h"
#include "camera.h"
#include "reporter.h"

CCamera::CCamera()
{
	SetPosition(Vector3f(0.0f, 0.0f, 0.0f));

	m_Ghost = true;
}

void CCamera::Draw() 
{
}


void CCamera::CalculateFrustumRowMajor(float data[4][4])
{
		// Extract the RIGHT plane.
	frustum[0].normal.x = data[0][3] - data[0][0];
	frustum[0].normal.y = data[1][3] - data[1][0];
	frustum[0].normal.z = data[2][3] - data[2][0];
	frustum[0].d =		  data[3][3] - data[3][0];
	// Normalize the result
	frustum[0].Normalize();

	// Extract the LEFT plane.
	frustum[1].normal.x = data[0][3] + data[0][0];
	frustum[1].normal.y = data[1][3] + data[1][0];
	frustum[1].normal.z = data[2][3] + data[2][0];
	frustum[1].d =		  data[3][3] + data[3][0];
	// Normalize the result
	frustum[1].Normalize();

	// Extract the BOTTOM plane
	frustum[2].normal.x = data[0][3] + data[0][1];
	frustum[2].normal.y = data[1][3] + data[1][1];
	frustum[2].normal.z = data[2][3] + data[2][1];
	frustum[2].d = data[3][3] + data[3][1];
	// Normalize the result
	frustum[2].Normalize();

	// Extract the TOP plane
	frustum[3].normal.x = data[0][3] - data[0][1];
	frustum[3].normal.y = data[1][3] - data[1][1];
	frustum[3].normal.z = data[2][3] - data[2][1];
	frustum[3].d = data[3][3] - data[3][1];
	// Normalize the result
	frustum[3].Normalize();

	// Extract the FAR plane
	frustum[4].normal.x = data[0][3] - data[0][2];
	frustum[4].normal.y = data[1][3] - data[1][2];
	frustum[4].normal.z = data[2][3] - data[2][2];
	frustum[4].d = data[3][3] - data[3][2];
	// Normalize the result
	frustum[4].Normalize(); 

	// Extract the NEAR plane 
	frustum[5].normal.x = data[0][3] + data[0][2];
	frustum[5].normal.y = data[1][3] + data[1][2];
	frustum[5].normal.z = data[2][3] + data[2][2];
	frustum[5].d = data[3][3] + data[3][2];
	// Normalize the result
	frustum[5].Normalize();
}

void CCamera::CalculateFrustumColumnMajor(CMatrix4f &clip)
{
	// Extract the numbers for the RIGHT plane
	frustum[0].normal.x = clip[ 3] - clip[ 0];
	frustum[0].normal.y = clip[ 7] - clip[ 4];
	frustum[0].normal.z = clip[11] - clip[ 8];
	frustum[0].d		= clip[15] - clip[12];

	// Normalize the result
	frustum[0].Normalize();

	// Extract the numbers for the LEFT plane
	frustum[1].normal.x = clip[ 3] + clip[ 0];
	frustum[1].normal.y = clip[ 7] + clip[ 4];
	frustum[1].normal.z = clip[11] + clip[ 8];
	frustum[1].d		= clip[15] + clip[12];

	// Normalize the result
	frustum[1].Normalize();

	// Extract the BOTTOM plane
	frustum[2].normal.x = clip[ 3] + clip[ 1];
	frustum[2].normal.y = clip[ 7] + clip[ 5];
	frustum[2].normal.z = clip[11] + clip[ 9];
	frustum[2].d		= clip[15] + clip[13];

	// Normalize the result
	frustum[2].Normalize();

	// Extract the TOP plane 
	frustum[3].normal.x = clip[ 3] - clip[ 1];
	frustum[3].normal.y = clip[ 7] - clip[ 5];
	frustum[3].normal.z = clip[11] - clip[ 9];
	frustum[3].d		= clip[15] - clip[13];

	// Normalize the result
	frustum[3].Normalize();

	// Extract the FAR plane
	frustum[4].normal.x = clip[ 3] - clip[ 2];
	frustum[4].normal.y = clip[ 7] - clip[ 6];
	frustum[4].normal.z = clip[11] - clip[10];
	frustum[4].d		= clip[15] - clip[14];

	// Normalize the result
	frustum[4].Normalize();

	// Extract the NEAR plane
	frustum[5].normal.x = clip[ 3] + clip[ 2];
	frustum[5].normal.y = clip[ 7] + clip[ 6];
	frustum[5].normal.z = clip[11] + clip[10];
	frustum[5].d		= clip[15] + clip[14];

	// Normalize the result
	frustum[5].Normalize();
}

bool CCamera::IsCubeInFrustum(const Vector3f &centre, float length)
{
	//return true; // TODO: remove this line again - Sam

	int p;
	float x = centre.x, y = centre.y, z = centre.z;

	for( p = 0; p < 6; p++ )
	{
	  if( frustum[p].normal.x * (x - length) + frustum[p].normal.y * (y - length)
			  + frustum[p].normal.z * (z - length) + frustum[p].d > 0 )
		 continue;
	  if( frustum[p].normal.x * (x + length) + frustum[p].normal.y * (y - length)
			  + frustum[p].normal.z * (z - length) + frustum[p].d > 0 )
		 continue;
	  if( frustum[p].normal.x * (x - length) + frustum[p].normal.y * (y + length)
			  + frustum[p].normal.z * (z - length) + frustum[p].d > 0 )
		 continue;
	  if( frustum[p].normal.x * (x + length) + frustum[p].normal.y * (y + length)
			  + frustum[p].normal.z * (z - length) + frustum[p].d > 0 )
		 continue;
	  if( frustum[p].normal.x * (x - length) + frustum[p].normal.y * (y - length)
			  + frustum[p].normal.z * (z + length) + frustum[p].d > 0 )
		 continue;
	  if( frustum[p].normal.x * (x + length) + frustum[p].normal.y * (y - length) 
			  + frustum[p].normal.z * (z + length) + frustum[p].d > 0 )
		 continue;
	  if( frustum[p].normal.x * (x - length) + frustum[p].normal.y * (y + length)
			  + frustum[p].normal.z * (z + length) + frustum[p].d > 0 )
		 continue;
	  if( frustum[p].normal.x * (x + length) + frustum[p].normal.y * (y + length)
			  + frustum[p].normal.z * (z + length) + frustum[p].d > 0 )
		 continue;
	  return false;
	}
	return true;
}

void CCamera::Update(float diff)
{
	CActor::Update(diff);
	
	 
}

