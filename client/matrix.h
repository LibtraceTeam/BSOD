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
#ifndef __MATRIX_H__
#define __MATRIX_H__

//#include <string.h>

/**
 * A 4x4 floating point matrix class in COLUMN MAJOR order.  It needs column
 * major ordering so it is compatible with OpenGL.  It should seamlessly
 * integrate with OpenGL.
 */
class CMatrix4f
{
private:
	float data[16];

public:
	// Constructors:
	CMatrix4f() { 
		// New behaviour: UNITIALISED when it is created by default!
		// This is desirable behaviour. It is. - Sam
		// SetToIdentity(); 
	}
	CMatrix4f(float *src) { memcpy(data, src, 16 * sizeof(float)); }
	CMatrix4f(CMatrix4f &copy) { memcpy(data, copy.GetData(), 16 * sizeof(float)); }

	// Member functions
	CMatrix4f &ZeroMatrix() { memset(data, 0, 16 * sizeof(float)); return *this; }
	CMatrix4f &SetToIdentity();
	CMatrix4f &Add(CMatrix4f &other);
	CMatrix4f &Multiply(CMatrix4f &other);
	CMatrix4f &Multiply(CMatrix4f &a, CMatrix4f &b); // Multiplies a by b, result in this
	CMatrix4f operator +(CMatrix4f &other);
	CMatrix4f operator *(CMatrix4f &other);
	CMatrix4f &operator=(CMatrix4f &other);
	Vector3f operator*(Vector3f &other);
	void operator*=(CMatrix4f &other);

	CMatrix4f &FromRowMajor(float m[4][4]);
	float &operator[](const int index) { return data[index]; }
	const float &operator[](const int index) const { return data[index]; }
	float *GetData() { return &data[0]; }

	// Misc
	void Dump() const;
	void RotateX(const float angle);
	void RotateY(const float angle);
	void RotateZ(const float angle);

	void Scale(const Vector3f &v);
	void Translate(const Vector3f &v);
	void Transpose();

	void GetEulerAngles(Vector3f &v);
};

#endif

