/* $CVSID$ */ 
#include "stdafx.h"
#include "reporter.h"
#include "matrix.h"

CMatrix4f &CMatrix4f::SetToIdentity()
{
	ZeroMatrix();
	data[0] = data[5] = data[10] = data[15] = 1.0f;

	return *this;
}

CMatrix4f &CMatrix4f::Add(CMatrix4f &other)
{
	for(int i = 0; i < 16; i++)
		data[i] += other[i];

	return *this;
}

CMatrix4f &CMatrix4f::Multiply(CMatrix4f &other)
{
	CMatrix4f copy(*this);

	data[ 0] = copy[ 0] * other[ 0] + copy[ 1] * other[ 4] + copy[ 2] * other[ 8] + copy[ 3] * other[12];
	data[ 1] = copy[ 0] * other[ 1] + copy[ 1] * other[ 5] + copy[ 2] * other[ 9] + copy[ 3] * other[13];
	data[ 2] = copy[ 0] * other[ 2] + copy[ 1] * other[ 6] + copy[ 2] * other[10] + copy[ 3] * other[14];
	data[ 3] = copy[ 0] * other[ 3] + copy[ 1] * other[ 7] + copy[ 2] * other[11] + copy[ 3] * other[15];

	data[ 4] = copy[ 4] * other[ 0] + copy[ 5] * other[ 4] + copy[ 6] * other[ 8] + copy[ 7] * other[12];
	data[ 5] = copy[ 4] * other[ 1] + copy[ 5] * other[ 5] + copy[ 6] * other[ 9] + copy[ 7] * other[13];
	data[ 6] = copy[ 4] * other[ 2] + copy[ 5] * other[ 6] + copy[ 6] * other[10] + copy[ 7] * other[14];
	data[ 7] = copy[ 4] * other[ 3] + copy[ 5] * other[ 7] + copy[ 6] * other[11] + copy[ 7] * other[15];

	data[ 8] = copy[ 8] * other[ 0] + copy[ 9] * other[ 4] + copy[10] * other[ 8] + copy[11] * other[12];
	data[ 9] = copy[ 8] * other[ 1] + copy[ 9] * other[ 5] + copy[10] * other[ 9] + copy[11] * other[13];
	data[10] = copy[ 8] * other[ 2] + copy[ 9] * other[ 6] + copy[10] * other[10] + copy[11] * other[14];
	data[11] = copy[ 8] * other[ 3] + copy[ 9] * other[ 7] + copy[10] * other[11] + copy[11] * other[15];

	data[12] = copy[12] * other[ 0] + copy[13] * other[ 4] + copy[14] * other[ 8] + copy[15] * other[12];
	data[13] = copy[12] * other[ 1] + copy[13] * other[ 5] + copy[14] * other[ 9] + copy[15] * other[13];
	data[14] = copy[12] * other[ 2] + copy[13] * other[ 6] + copy[14] * other[10] + copy[15] * other[14];
	data[15] = copy[12] * other[ 3] + copy[13] * other[ 7] + copy[14] * other[11] + copy[15] * other[15];

	return *this;
}

CMatrix4f &CMatrix4f::Multiply(CMatrix4f &a, CMatrix4f &b)
{
	data[ 0] = a[ 0] * b[ 0] + a[ 1] * b[ 4] + a[ 2] * b[ 8] + a[ 3] * b[12];
	data[ 1] = a[ 0] * b[ 1] + a[ 1] * b[ 5] + a[ 2] * b[ 9] + a[ 3] * b[13];
	data[ 2] = a[ 0] * b[ 2] + a[ 1] * b[ 6] + a[ 2] * b[10] + a[ 3] * b[14];
	data[ 3] = a[ 0] * b[ 3] + a[ 1] * b[ 7] + a[ 2] * b[11] + a[ 3] * b[15];

	data[ 4] = a[ 4] * b[ 0] + a[ 5] * b[ 4] + a[ 6] * b[ 8] + a[ 7] * b[12];
	data[ 5] = a[ 4] * b[ 1] + a[ 5] * b[ 5] + a[ 6] * b[ 9] + a[ 7] * b[13];
	data[ 6] = a[ 4] * b[ 2] + a[ 5] * b[ 6] + a[ 6] * b[10] + a[ 7] * b[14];
	data[ 7] = a[ 4] * b[ 3] + a[ 5] * b[ 7] + a[ 6] * b[11] + a[ 7] * b[15];

	data[ 8] = a[ 8] * b[ 0] + a[ 9] * b[ 4] + a[10] * b[ 8] + a[11] * b[12];
	data[ 9] = a[ 8] * b[ 1] + a[ 9] * b[ 5] + a[10] * b[ 9] + a[11] * b[13];
	data[10] = a[ 8] * b[ 2] + a[ 9] * b[ 6] + a[10] * b[10] + a[11] * b[14];
	data[11] = a[ 8] * b[ 3] + a[ 9] * b[ 7] + a[10] * b[11] + a[11] * b[15];

	data[12] = a[12] * b[ 0] + a[13] * b[ 4] + a[14] * b[ 8] + a[15] * b[12];
	data[13] = a[12] * b[ 1] + a[13] * b[ 5] + a[14] * b[ 9] + a[15] * b[13];
	data[14] = a[12] * b[ 2] + a[13] * b[ 6] + a[14] * b[10] + a[15] * b[14];
	data[15] = a[12] * b[ 3] + a[13] * b[ 7] + a[14] * b[11] + a[15] * b[15];

	return *this;
}

CMatrix4f CMatrix4f::operator *(CMatrix4f &other)
{
	return CMatrix4f(*this).Multiply(other);
}

CMatrix4f CMatrix4f::operator +(CMatrix4f &other)
{
	return CMatrix4f(*this).Add(other);
}

CMatrix4f &CMatrix4f::FromRowMajor(float m[4][4])
{
	// Maybe this should really be called FromD3DMatrix or some shit.
	/*
	data[0] = m[0][0];
	data[1] = m[1][0];
	data[2] = m[2][0];
	data[3] = m[3][0];

	data[4] = m[0][1];
	data[5] = m[1][1];
	data[6] = m[2][1];
	data[7] = m[3][1];

	data[8] = m[0][2];
	data[9] = m[1][2];
	data[10] = m[2][2];
	data[11] = m[3][2];

	data[12] = m[0][3];
	data[13] = m[1][3];
	data[14] = m[2][3];
	data[15] = m[3][3];
	*/

	return *this;
}

void CMatrix4f::Dump() const
{
	Log("Matrix{ [%1.3f,%1.3f,%1.3f]\n        [%1.3f,%1.3f,%1.3f]\n        [%1.3f,%1.3f,%1.3f]\n        [%1.3f,%1.3f,%1.3f] }\n",
		data[0], data[4], data[8], data[12],
		data[1], data[5], data[9], data[13],
		data[2], data[6], data[10], data[14],
		data[3], data[7], data[11], data[15]
		);
}

CMatrix4f &CMatrix4f::operator=(CMatrix4f &other)
{
	for(int i = 0; i < 16; i++)
		data[i] = other[i];
	return *this;
}

void CMatrix4f::operator*=(CMatrix4f &other)
{
	this->Multiply(other);
}

Vector3f CMatrix4f::operator*(Vector3f &other)
{
	return Vector3f(
		other.x * data[0] + 
		other.y * data[1] + 
		other.z * data[2] + 
		data[3],
		other.x * data[4] + 
		other.y * data[5] + 
		other.z * data[6] + 
		data[7],
		other.x * data[8] + 
		other.y * data[9] + 
		other.z * data[9] + 
		data[11]);
}

void CMatrix4f::RotateX(const float angle)
{
	const float cos_th = (float) cosf(angle);
	const float sin_th = (float) sinf(angle);

	data[5] = cos_th;	data[6]  = sin_th;	
	data[9] = -sin_th;	data[10] = cos_th;
}

void CMatrix4f::RotateY(const float angle)
{
	const float cos_th = (float) cosf(angle);
	const float sin_th = (float) sinf(angle);

	data[0] = cos_th;	data[2] = -sin_th;
	data[8] = sin_th;	data[10] = cos_th;
}

void CMatrix4f::RotateZ(const float angle)
{
	const float cos_th = (float) cosf(angle);
	const float sin_th = (float) sinf(angle);

	data[0] = cos_th;	data[1] = sin_th;
	data[4] = -sin_th;	data[5] = cos_th;
}

void CMatrix4f::Scale(const Vector3f &v)
{
	data[0] = v.x;
	data[5] = v.y;
	data[10] = v.z;
}

void CMatrix4f::Translate(const Vector3f &v)
{
	data[12] += data[0]*v.x + data[4]*v.y + data[8]*v.z;
	data[13] += data[1]*v.x + data[5]*v.y + data[9]*v.z;
	data[14] += data[2]*v.x + data[6]*v.y + data[10]*v.z;
	data[15] += data[3]*v.x + data[7]*v.y + data[11]*v.z;
}

void CMatrix4f::Transpose()
{
	swap(data[1], data[4]);
    swap(data[2], data[8]);
    swap(data[3], data[12]);
    swap(data[6], data[9]);
    swap(data[7], data[13]);
    swap(data[11], data[14]);
    swap(data[6], data[9]);
}

void CMatrix4f::GetEulerAngles(Vector3f &v)
{
	// TODO: check this function for correctness and use of doubles
	double x_th, y_th, z_th;

	y_th = atan2(data[8], 
			sqrt(data[9] * data[9] + 
				data[10] * data[10]));

	double cos_y_th = cos(y_th);

	if(fabs(cos_y_th) > 1e12) {

		z_th = atan2(-data[4] / cos_y_th, data[0] / cos_y_th);
		x_th = atan2(-data[9] / cos_y_th, data[10] / cos_y_th);

	} else if(fabs((PI/2) - y_th) < 1e12) {

		x_th = atan2(data[1] , data[5]);
		y_th = PI/2;
		z_th = 0.0;
	} else {

		x_th = atan2(-data[1], data[5]);
		y_th = -(PI/2);
		z_th = 0.0;
	}
	

	v.x = (float) ( -x_th * (180.0f/PI) );
	v.y = (float) ( -y_th * (180.0f/PI) );
	v.z = (float) ( -z_th * (180.0f/PI) );
}
