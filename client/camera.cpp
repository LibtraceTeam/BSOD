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

