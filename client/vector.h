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
// Vector3f.h
// by Sam Jansen, 21/1/2002

#ifndef __VECTOR3F_H__
#define __VECTOR3F_H__

enum PointType { PLANE_FRONT, PLANE_BACKSIDE, ON_PLANE };

template<class T>
class Vector3
{
public:
	T x, y, z;

	Vector3();
	Vector3(const Vector3 &copy);
	Vector3(const T c_x, const T c_y, const T c_z);
	Vector3(const T *v);
	
	// Constant operations: These do not change the vector in question:
	T		 Dot(const Vector3 &dot) const;
	Vector3  Cross(const Vector3 &other) const;
	T		 Length() const;
	string   toString() const;

	Vector3  operator-(const Vector3 &sub) const;
	Vector3  operator+(const Vector3 &add) const;
	Vector3  operator*(const T mul) const;
	Vector3  operator/(const T div) const;
	Vector3  operator-() const;
	bool	 operator==(const Vector3 &eq) const;
	bool	 operator!=(const Vector3 &eq) const;

	// Non-constant operations: These do change the vector in question:
	void	 Normalize();
	Vector3  &Add(const Vector3 &add);
	
	Vector3  &operator=(const Vector3 &eq);
	Vector3  &operator+=(const Vector3 &add);
	Vector3  &operator-=(const Vector3 &add);
	Vector3  &operator/=(const T div);
	Vector3  &operator*=(const T mul);

	// Operators for when the left-hand operator is of another type:
	friend Vector3 operator*(const T f, const Vector3 &v);

protected:

};

typedef Vector3<float>	Vector3f;
typedef Vector3<double>	Vector3d;

class CPlane
{
public:
	CPlane(float A, float B, float C, float D) {
		normal = Vector3f(A, B, C);
		d = D;
	}
	CPlane() { d = 0; }

	Vector3f normal; // x, y, z make up the A. B and C components
	float d; // Distance from origin: D component of plane equation

	CPlane &Normalize() { d /= normal.Length(); normal.Normalize(); return *this; }
};

/* There is currently no guarantee that m_min will actually be the min and
   m_max will be the max.  There should really be a function to fix that for
   us when we know it might not be the case. */
class CBox
{
public:
	Vector3f m_min, m_max;

	CBox() { }
	CBox(Vector3f _min, Vector3f _max) : m_min(_min), m_max(_max) { }

	float Width() { return (float)fabs(m_max.x - m_min.x); }
	float Height() { return (float)fabs(m_max.y - m_min.y); }
	float Length() { return (float)fabs(m_max.z - m_min.z); }

};

class Vector2f
{
public:
	float x, y;

	Vector2f();
	Vector2f(const Vector2f &copy);
	Vector2f(float c_x, float c_y);

	Vector2f &Add(Vector2f &add);

	Vector2f operator-(const Vector2f &sub);
	Vector2f operator+(const Vector2f &add);
	Vector2f operator*(float mul);
	Vector2f &operator=(const Vector2f &eq);
};

// ray intersections. All return -1.0 if no intersection, otherwise the distance along the 
// ray where the (first) intersection takes place
float intersectRayPlane(Vector3f rOrigin, Vector3f rVector, Vector3f pOrigin, Vector3f pNormal); 
float intersectRaySphere(Vector3f rO, Vector3f rV, Vector3f sO, float sR);

// Distance to line of triangle
Vector3f closestPointOnLine(Vector3f& a, Vector3f& b, Vector3f& p);
Vector3f closestPointOnTriangle(Vector3f a, Vector3f b, Vector3f c, Vector3f p);

// point inclusion
bool CheckPointInTriangle(Vector3f point ,Vector3f a, Vector3f b, Vector3f c);
bool CheckPointInSphere(Vector3f point, Vector3f sO, float sR);

// Normal generation
Vector3f tangentPlaneNormalOfEllipsoid(Vector3f point,Vector3f eO, Vector3f eR);

// Point classification
PointType classifyPoint(Vector3f point, Vector3f pO, Vector3f pN);

bool GetIntersection(Vector3f n1, 
					  Vector3f n2, 
					  Vector3f n3, 
					  float d1, 
					  float d2, 
					  float d3,
					  Vector3f &p);

void TriangleNormal(const Vector3f &p1, const Vector3f &p2, const Vector3f &p3, Vector3f *normal);

#endif

