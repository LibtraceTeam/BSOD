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
// Vector3f.cpp
// Created 21/1/2002 by Sam Jansen

#include "stdafx.h"
#include <math.h>

#include "vector.h"

// Forces template instantiation with MSVC.
// See the discussion at:
// http://www.flipcode.com/cgi-bin/msg.cgi?showThread=00001144&forum=general&id=-1
// for why this needs to be done.
// TODO: check this works for G++
template Vector3<float>;
template Vector3<double>;

// Global operators:
template<class T>
Vector3<T> operator*(const T f, const Vector3<T> &v)
{
	return v * f;
}

// Member operators and functions:
template<class T>
Vector3<T> Vector3<T>::operator-() const
{
	return Vector3<T>() - *this;
}

template<class T>
Vector3<T>::Vector3()
{
	x = y = z = 0.0f;
}

template<class T>
Vector3<T>::Vector3(const Vector3<T> &copy)
{
	x = copy.x;
	y = copy.y;
	z = copy.z;
}

template<class T>
Vector3<T>::Vector3(const T c_x, const T c_y, const T c_z)
{
	x = c_x;
	y = c_y;
	z = c_z;
}

template<class T>
Vector3<T>::Vector3(const T *v)
{
	x = v[0];
	y = v[1];
	z = v[2];
}

template<class T>
Vector3<T> &Vector3<T>::Add(const Vector3<T> &add)
{
	x += add.x;
	y += add.y;
	z += add.z;
	
	return *this;
}

template<class T>
Vector3<T> Vector3<T>::operator *(const T mul) const
{
	Vector3<T> copy(*this);
	
	copy.x *= mul;
	copy.y *= mul;
	copy.z *= mul;
	
	return copy;
}

template<class T>
Vector3<T> Vector3<T>::operator /(const T div) const
{
	Vector3<T> copy(*this);
	
	copy.x /= div;
	copy.y /= div;
	copy.z /= div;
	
	return copy;
}

template<class T>
Vector3<T> &Vector3<T>::operator+=(const Vector3<T> &add)
{
	x += add.x;
	y += add.y;
	z += add.z;
	
	return *this;
}

template<class T>
Vector3<T> &Vector3<T>::operator-=(const Vector3<T> &add)
{
	x -= add.x;
	y -= add.y;
	z -= add.z;
	
	return *this;
}


template<class T>
Vector3<T> &Vector3<T>::operator/=(const T div)
{
	x /= div;
	y /= div;
	z /= div;
	
	return *this;
}

template<class T>
Vector3<T> &Vector3<T>::operator*=(const T mul)
{
	x *= mul;
	y *= mul;
	z *= mul;
	
	return *this;
}

template<class T>
T Vector3<T>::Dot(const Vector3<T> &dot) const
{
	return ( x * dot.x + y * dot.y + z * dot.z );
}

template<class T>
Vector3<T> Vector3<T>::Cross(const Vector3<T> &other) const
{
	return Vector3<T>(
		y * other.z - z * other.y,
		z * other.x - x * other.z,
		x * other.y - y * other.x);
}

template<class T>
T Vector3<T>::Length() const
{
	return (float)(sqrt(x * x + y * y + z * z));
}

template<class T>
void Vector3<T>::Normalize()
{
	T len = Length();
	ASSERT(len != 0.0f);
	x /= len;
	y /= len;
	z /= len;
}

template<class T>
string Vector3<T>::toString() const
{
	char buf[3072];
	sprintf(buf, "(%#+3.3f,%#+3.3f,%#+3.3f)", x, y, z);
	buf[3071] = '\0';
	return buf;
}

template<class T>
Vector3<T> Vector3<T>::operator-(const Vector3<T> &sub) const
{
	Vector3<T> copy(*this);
	
	copy.x -= sub.x;
	copy.y -= sub.y;
	copy.z -= sub.z;
	
	return copy;
}

template<class T>
Vector3<T> Vector3<T>::operator+(const Vector3<T> &add) const
{
	Vector3<T> copy(*this); 
	
	copy.x += add.x;
	copy.y += add.y;
	copy.z += add.z;
	
	return copy;
}

template<class T>
Vector3<T> &Vector3<T>::operator=(const Vector3<T> &eq)
{
	x = eq.x;
	y = eq.y;
	z = eq.z;
	
	return *this;
}

template<class T>
bool Vector3<T>::operator==(const Vector3<T> &eq) const
{
	return 
		(x == eq.x &&
		y == eq.y &&
		z == eq.z);
}

template<class T>
bool Vector3<T>::operator!=(const Vector3<T> &eq) const
{
	return !(*this == eq);
}

Vector2f::Vector2f()
{
	x = y = 0.0f;
}

Vector2f::Vector2f(const Vector2f &copy)
{
	x = copy.x;
	y = copy.y;
}

Vector2f::Vector2f(float c_x, float c_y)
{
	x = c_x;
	y = c_y;
}

Vector2f &Vector2f::Add(Vector2f &add)
{
	x += add.x;
	y += add.y;
	
	return *this;
}

Vector2f Vector2f::operator-(const Vector2f &sub)
{
	Vector2f copy(*this);
	
	copy.x -= sub.x;
	copy.y -= sub.y;
	
	return copy;
}

Vector2f Vector2f::operator+(const Vector2f &add)
{
	Vector2f copy(*this);
	
	copy.x += add.x;
	copy.y += add.y;
	
	return copy;
}

Vector2f Vector2f::operator *(float mul)
{
	Vector2f copy(*this);
	
	copy.x *= mul;
	copy.y *= mul;
	
	return copy;
}

Vector2f &Vector2f::operator=(const Vector2f &eq)
{
	x = eq.x;
	y = eq.y;
	
	return *this;
}


// ----------------------------------------------------------------------
// Name  : intersectRayPlane()
// Input : rOrigin - origin of ray in world space
//         rVector - vector describing direction of ray in world space
//         pOrigin - Origin of plane 
//         pNormal - Normal to plane
// Notes : Normalized directional vectors expected
// Return: distance to plane in world units, -1 if no intersection.
// -----------------------------------------------------------------------  
float intersectRayPlane(Vector3f rOrigin, Vector3f rVector, Vector3f pOrigin, Vector3f pNormal) {
	
	float d = - pNormal.Dot(pOrigin);
	
	float numer = pNormal.Dot(rOrigin) + d;
	float denom = pNormal.Dot(rVector);
	
	
	if (denom == 0)  // normal is orthogonal to vector, cant intersect
		return (-1.0f);
	
	return -(numer / denom);	
}


// ----------------------------------------------------------------------
// Name  : intersectRaySphere()
// Input : rO - origin of ray in world space
//         rV - vector describing direction of ray in world space
//         sO - Origin of sphere 
//         sR - radius of sphere
// Notes : Normalized directional vectors expected
// Return: distance to sphere in world units, -1 if no intersection.
// -----------------------------------------------------------------------  

float intersectRaySphere(Vector3f rO, Vector3f rV, Vector3f sO, float sR) {
	
	Vector3f Q = sO-rO;
	
	float c = Q.Length();
	float v = Q.Dot(rV);
	float d = sR*sR - (c*c - v*v);
	
	// If there was no intersection, return -1
	if (d < 0.0) return (-1.0f);
	
	// Return the distance to the [first] intersecting point
	return (float)(v - sqrt(d));
}




// ----------------------------------------------------------------------
// Name  : CheckPointInTriangle()
// Input : point - point we wish to check for inclusion
//         a - first vertex in triangle
//         b - second vertex in triangle 
//         c - third vertex in triangle
// Notes : Triangle should be defined in clockwise order a,b,c
// Return: TRUE if point is in triangle, FALSE if not.
// -----------------------------------------------------------------------  

bool CheckPointInTriangle(Vector3f point, Vector3f a, Vector3f b, Vector3f c) {
	
	float total_angles = 0.0f;
	
	// make the 3 vectors
	Vector3f v1 = point-a;
	Vector3f v2 = point-b;
	Vector3f v3 = point-c;
	
	v1.Normalize();
	v2.Normalize();
	v3.Normalize();
	
	total_angles += (float)acos(v1.Dot(v2));   
	total_angles += (float)acos(v2.Dot(v3));
	total_angles += (float)acos(v3.Dot(v1)); 
	
	if (fabs(total_angles-2*PI) <= 0.005)
		return (true);
	
	return(false);
}



// ----------------------------------------------------------------------
// Name  : closestPointOnLine()
// Input : a - first end of line segment
//         b - second end of line segment
//         p - point we wish to find closest point on line from 
// Notes : Helper function for closestPointOnTriangle()
// Return: closest point on line segment
// -----------------------------------------------------------------------  


Vector3f closestPointOnLine(Vector3f& a, Vector3f& b, Vector3f& p) {
	
	// Determine t (the length of the vector from ‘a’ to ‘p’)
	Vector3f c = p-a;
	Vector3f V = b-a; 
	
	float d = V.Length();
	
	V.Normalize();  
	float t = V.Dot(c);
	
	
	// Check to see if ‘t’ is beyond the extents of the line segment
	if (t < 0.0f) return (a);
	if (t > d) return (b);
	
	
	// Return the point between ‘a’ and ‘b’
	//set length of V to t. V is normalized so this is easy
	V.x = V.x * t;
	V.y = V.y * t;
	V.z = V.z * t;
	
	return (a+V);	
}




// ----------------------------------------------------------------------
// Name  : closestPointOnTriangle()
// Input : a - first vertex in triangle
//         b - second vertex in triangle 
//         c - third vertex in triangle
//         p - point we wish to find closest point on triangle from 
// Notes : 
// Return: closest point on line triangle edge
// -----------------------------------------------------------------------  

Vector3f closestPointOnTriangle(Vector3f a, Vector3f b, Vector3f c, Vector3f p) {
	
	Vector3f Rab = closestPointOnLine(a, b, p);
	Vector3f Rbc = closestPointOnLine(b, c, p);
	Vector3f Rca = closestPointOnLine(c, a, p);
    
	float dAB = (p-Rab).Length();
	float dBC = (p-Rbc).Length();
	float dCA = (p-Rca).Length();
	
	
	float min = dAB;
	Vector3f result = Rab;
	
	if (dBC < min) {
		min = dBC;
		result = Rbc;
    }
	
	if (dCA < min)
		result = Rca;
	
    
	return (result);	
}



// ----------------------------------------------------------------------
// Name  : CheckPointInTriangle()
// Input : point - point we wish to check for inclusion
//         sO - Origin of sphere
//         sR - radius of sphere 
// Notes : 
// Return: TRUE if point is in sphere, FALSE if not.
// -----------------------------------------------------------------------  

bool CheckPointInSphere(Vector3f point, Vector3f sO, float sR) {
	
	float d = (point-sO).Length();
	
	if(d<= sR) return true;
	return false;	
}




// ----------------------------------------------------------------------
// Name  : tangentPlaneNormalOfEllipsoid()
// Input : point - point we wish to compute normal at 
//         eO - Origin of ellipsoid
//         eR - radius vector of ellipsoid 
// Notes : 
// Return: a unit normal vector to the tangent plane of the ellipsoid in the point.
// -----------------------------------------------------------------------  
Vector3f tangentPlaneNormalOfEllipsoid(Vector3f point, Vector3f eO, Vector3f eR) {
	
	Vector3f p = point - eO;
	
	float a2 = eR.x * eR.x;
	float b2 = eR.y * eR.y;
	float c2 = eR.z * eR.z;
	
	
	Vector3f res;
	res.x = p.x / a2;
	res.y = p.y / b2;
	res.z = p.z / c2;
	
	res.Normalize();	
	return (res);	
}



// ----------------------------------------------------------------------
// Name  : classifyPoint()
// Input : point - point we wish to classify 
//         pO - Origin of plane
//         pN - Normal to plane 
// Notes : 
// Return: One of 3 classification codes
// -----------------------------------------------------------------------  

PointType classifyPoint(Vector3f point, Vector3f pO, Vector3f pN) {
	
	Vector3f dir = pO - point;
	float d = dir.Dot(pN);
	
	if (d<-0.001f)
		return PLANE_FRONT;	
	else
		if (d>0.001f)
			return PLANE_BACKSIDE;	
		
		return ON_PLANE;	
}



// Has problems with g++.  Not used at the moment anyway!
bool GetIntersection(Vector3f n1, 
					 Vector3f n2, 
					 Vector3f n3, 
					 float d1, 
					 float d2, 
					 float d3,
					 Vector3f &p)
{
	/*
	float denom = n1.Dot ( n2.Cross ( n3 ) );
	
	if ( denom == 0 )
	{
		return false;
	}
	p = -d1 * ( n2.Cross ( n3 ) )  - d2 * ( n3.Cross ( n1 ) )  - d3 * ( n1.Cross ( n2 ) );
	p /= denom; */
	
	return true;
}


// Sam's function to calculate normals for arbitrary triangles
// p1 - triangle point 1
// p2 - triangle point 2
// p3 - triangle point 3
// normal - output of function
// returns the normal -- note that it is NOT normalised (length prob. doesn't equal 1)
void TriangleNormal(const Vector3f &p1, const Vector3f &p2, const Vector3f &p3, Vector3f *normal)
{
	Vector3f u, v, r;

	ASSERT(normal);

	u = p2 - p1;
	v = p3 - p1;

	r = u.Cross(v);

	normal->x = r.x;
	normal->y = r.y;
	normal->z = r.z;
}

