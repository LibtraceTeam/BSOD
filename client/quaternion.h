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
#ifndef __QUATERNION_H__
#define __QUATERNION_H__

class CMatrix4f;
//class Vector3f;

class CQuaternion {
public:
	float x, y, z, w;

public:
	CQuaternion();
	CQuaternion(float x, float y, float z, float w);
	CQuaternion(const CQuaternion &ref);

	CQuaternion operator+(const CQuaternion &ref);
	CQuaternion operator-(const CQuaternion &ref);
	CQuaternion operator*(const CQuaternion &ref);
	CQuaternion operator/(const CQuaternion &ref);
	
	void operator+=(const CQuaternion &ref);
	void operator-=(const CQuaternion &ref);
	void operator*=(const CQuaternion &ref);
	void operator/=(const CQuaternion &ref);

	CQuaternion &operator=(const CQuaternion &ref);

	CQuaternion &Normalize();
	float Length();

	void Scale(float val);
	void Conjugate();
	void Inverse();
	float DotProduct();
	float DotProduct(const CQuaternion &ref);
	
	void Invert();
	void Multiply(const CQuaternion &a, const CQuaternion &b);
	void Ln();
	void LnDif(const CQuaternion &a, const CQuaternion &b);
	void Exp();
	void InnerPoint(const CQuaternion &a, const CQuaternion &b, const CQuaternion &c);

	// Rotate a vector using this quaternion
	void RotateVector(Vector3f &point);

	void GetDirectionVector(Vector3f &result);

	// generates a CMatrix4f from this quaternion
	void CreateMatrix(CMatrix4f &result);
	
	void BuildFromMatrix(const CMatrix4f &matrix);
	void BuildFromMatrix3x3(const float *data);

	void BuildFromEuler(const Vector3f &v);
	void GetEulerAngles(Vector3f &result);

	// Fills the quaternion from the given axis angle of rotation
	void BuildFromAxisAngle(Vector3f axis, float degree);
	void GetAxisAngle(Vector3f &axis, float &degree);

	void Slerp(const CQuaternion &a, const CQuaternion &b, const float time);
};

#endif

