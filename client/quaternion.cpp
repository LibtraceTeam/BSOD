/* $Header$ $Log$
/* $Header: /home/cvs/wand-general/bsod/client/quaternion.cpp,v 1.3 2004/02/17 01:59:56 stj2 Exp $ Revision 1.4  2004/02/17 02:05:21  stj2
/* $Header: /home/cvs/wand-general/bsod/client/quaternion.cpp,v 1.3 2004/02/17 01:59:56 stj2 Exp $ Still trying to get cvs tags correct.
/* $Header: /home/cvs/wand-general/bsod/client/quaternion.cpp,v 1.3 2004/02/17 01:59:56 stj2 Exp $
 $Header$ Revision 1.3  2004/02/17 01:59:56  stj2
 $Header$ Cvs tags will be correct at one point. Surely.
 $Header$ */ 
#include "stdafx.h"

#include "matrix.h"
#include "quaternion.h"

// Quaternion class by Jesse Baker - we use SLERPS
// float x, y, z, w;

// Updated by Sam for newer interface 19-3-2003

CQuaternion::CQuaternion()
{
	// NOTE: we no longer set anything in the constructor. Be aware of
	// this.
	//x = y = z = 0.0f;
	//w = 1.0f;
}

CQuaternion::CQuaternion(float X, float Y, float Z, float W)
{
	x = X; y = Y; z = Z; w = W;
}

CQuaternion::CQuaternion(const CQuaternion &ref)
{
	*this = ref;
}

CQuaternion CQuaternion::operator*(const CQuaternion &ref)
{
	return CQuaternion(
		w*ref.x + x*ref.w + y*ref.z - z*ref.y,                          
		w*ref.y + y*ref.w + z*ref.x - x*ref.z,
		w*ref.z + z*ref.w + x*ref.y - y*ref.x,
		w*ref.w - x*ref.x - y*ref.y - z*ref.z );
}

void CQuaternion::operator*=(const CQuaternion &ref)
{
	float X = w*ref.x + x*ref.w + y*ref.z - z*ref.y;
	float Y = w*ref.y + y*ref.w + z*ref.x - x*ref.z;
	float Z = w*ref.z + z*ref.w + x*ref.y - y*ref.x;
	float W = w*ref.w - x*ref.x - y*ref.y - z*ref.z;

	x = X; y = Y; z = Z; w = W;
}

CQuaternion CQuaternion::operator/(const CQuaternion &ref)
{
	CQuaternion inv = ref;
	inv.Inverse();
	inv *= (*this);

	return inv;
}

void CQuaternion::operator/=(const CQuaternion &ref)
{
	CQuaternion inv = ref;
	inv.Inverse();
	*this *= inv;
}

CQuaternion CQuaternion::operator+(const CQuaternion &ref)
{
	return CQuaternion(x+ref.x, y+ref.y, z+ref.z, w+ref.w);
}

CQuaternion CQuaternion::operator-(const CQuaternion &ref)
{
	return CQuaternion(x-ref.x, y-ref.y, z-ref.z, w-ref.w);
}

void CQuaternion::operator+=(const CQuaternion &ref)
{
	x+=ref.x; y+=ref.y; z+=ref.z; w+=ref.w;
}
void CQuaternion::operator-=(const CQuaternion &ref)
{
	x-=ref.x; y-=ref.y; z-=ref.z; w-=ref.w;
}

CQuaternion &CQuaternion::operator=(const CQuaternion &ref)
{
	x = ref.x;
	y = ref.y;
	z = ref.z;
	w = ref.w;
	return *this;	
}

CQuaternion &CQuaternion::Normalize()
{
	float a = DotProduct();
	float b = (float) (1.0f / sqrt(a));
	x*=b;
	y*=b;
	z*=b;
	w*=b;
	return *this;
}

float CQuaternion::Length()
{
	return sqrtf(DotProduct());
}

void CQuaternion::Scale(float val)
{
	x *= val, 
	y *= val, 
	z *= val, 
	w *= val;
}

void CQuaternion::Conjugate()
{
	x = -x, 
	y = -y, 
	z = -z;
	// w stays the same
}

void CQuaternion::Inverse()
{
	Conjugate();
	Scale(1.0f / DotProduct());
}

float CQuaternion::DotProduct()
{
	return (x * x) + (y * y) + (z * z) + (w * w);
}

float CQuaternion::DotProduct(const CQuaternion &ref)
{
	return (x * ref.x) + (y * ref.y) + (z * ref.z) + (w * ref.w);
}

void CQuaternion::Invert(void)
{
	float len = Length();

	if(fabsf(len) < EPSILON) {
		x = y = z = 0.0f;
		w = 1.0f;
	} else {  
		len = 1.0f / len;
		x = -x * len;
		y = -y * len;
		z = -z * len;
		w =  w * len;
	}
}

void CQuaternion::Multiply(const CQuaternion &a, const CQuaternion &b)
{
	x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
	y = a.w * b.y + a.y * b.w + a.z * b.x - a.x * b.z;
	z = a.w * b.z + a.z * b.w + a.x * b.y - a.y * b.x;
	w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
}

void CQuaternion::Ln(void)
{
	float t;

	float sqr = (float) sqrtf(x*x + y*y + z*z);
	float o = (float) atan2f(sqr, w);

	if(fabsf(sqr) < EPSILON) t = 0.0f;
	else t = o / sqr;

	x *= t;
	y *= t;
	z *= t;
	w = 0.0f;
}

void CQuaternion::LnDif(const CQuaternion &a, const CQuaternion &b)
{
	CQuaternion qu(a);

	qu.Invert();
	Multiply(qu, b);
	Ln();
}

void CQuaternion::Exp(void)
{
	float sin_o;

	float sqr = sqrtf(x*x + y*y + z*z);

	if(fabsf(sqr) < EPSILON) sin_o = 1.0f;
	else sin_o = sinf(sqr)/sqr;

	x *= sin_o;
	y *= sin_o;
	z *= sin_o;
	w = cosf(sqr);
}

void CQuaternion::InnerPoint(const CQuaternion &a, const CQuaternion &b, 
						     const CQuaternion &c)
{
	CQuaternion dc, da, d;

	dc.LnDif(b, c);
	da.LnDif(b, a);

    d.x = -1.0f / 4.0f * (dc.x + da.x);
	d.y = -1.0f / 4.0f * (dc.y + da.y);
	d.z = -1.0f / 4.0f * (dc.z + da.z);
	d.w = -1.0f / 4.0f * (dc.w + da.w);
	
	d.Exp();
	Multiply(b, d);
}

void CQuaternion::RotateVector(Vector3f &point)
{
	CQuaternion point_v(point.x, point.y, point.z, 0.0f);
	CQuaternion inv(*this);
	CQuaternion point_m;
	
	inv.Inverse();
	
	point_m = (*this) * point_v * inv;
  
	point.x = point_m.x;
	point.y = point_m.y;
	point.z = point_m.z;
}

void CQuaternion::GetDirectionVector(Vector3f &result)
{
	Normalize();
	
	result.x = 2.0f * ( x * z - w * y);
	result.y = 2.0f * ( y * z + w * x);
	result.z = 1.0f - 2.0f * ( x * x + y * y);
}

// generates a CMatrix4f from this quaternion
void CQuaternion::CreateMatrix(CMatrix4f &result)
{
	result[0]  = 1.0f - 2.0f * ( y * y + z * z ); 
	result[1]  = 2.0f * (x * y + z * w);
	result[2]  = 2.0f * (x * z - y * w);
	result[3]  = 0.0f;  

	result[4]  = 2.0f * ( x * y - z * w );  
	result[5]  = 1.0f - 2.0f * ( x * x + z * z ); 
	result[6]  = 2.0f * (z * y + x * w );  
	result[7]  = 0.0f;  

	result[8]  = 2.0f * ( x * z + y * w );
	result[9]  = 2.0f * ( y * z - x * w );
	result[10] = 1.0f - 2.0f * ( x * x + y * y );  
	result[11] = 0.0f;  

	result[12] = 0;  
	result[13] = 0;  
	result[14] = 0;  
	result[15] = 1.0f;
}

void CQuaternion::BuildFromMatrix3x3(const float *data)
{
	CMatrix4f matrix;

	matrix.SetToIdentity();

	matrix[0] = data[0]; matrix[1] = data[1]; matrix[2] = data[2];
	matrix[4] = data[3]; matrix[5] = data[4]; matrix[6] = data[5];
	matrix[8] = data[6]; matrix[9] = data[7]; matrix[10] = data[8];
	
	BuildFromMatrix(matrix);
}

void CQuaternion::BuildFromMatrix(const CMatrix4f &matrix)
{
	float copy[3], sqr;

	int next_pos[3] = {1, 2, 0};

	float diag = matrix[0] + matrix[5] + matrix[10] + 1; /// +1 added - Sam

	// Check if matrix diagonal is +ve
	if(diag > 0.0f) {
		sqr = (float) sqrtf(diag) * 2.0f;
		//w = sqr / 2.0f;
		//sqr = 0.5f / sqr;
		x = (matrix[9] - matrix[6]) / sqr;
		y = (matrix[2] - matrix[8]) / sqr;
		z = (matrix[4] - matrix[1]) / sqr;
		w = 0.25f * sqr;
	} else {
		/*
		int i = 0;
		if(matrix[5] > matrix[0])
			i = 1;
		if(matrix[10] > matrix[(i * 4) + i])
			i = 2;
		
		int j = next_pos[i];
		int k = next_pos[j];

		sqr = (float) matrix[(i * 4) + i] - (matrix[(j * 4) + j] + matrix[(k * 4) + k]);
		sqr = (float) sqrtf(sqr + 1.0f);

		copy[i] = sqr / 2.0f;

		if(sqr != 0.0f) sqr = 0.5f / sqr;

		w = (matrix[(j * 4) + k] - matrix[(k * 4) + j]) * sqr;
		copy[j] = (matrix[(i * 4) + j] + matrix[(j * 4) + i]) * sqr;
		copy[k] = (matrix[(i * 4) + k] + matrix[(k * 4) + i]) * sqr;

		x = copy[0];
		y = copy[1];
		z = copy[2];
		*/
		// If the first element of the diagonal is the greatest value
        if ( matrix[0] > matrix[5] && matrix[0] > matrix[10] )  
        {   
            // Find the sqr according to the first element, and double that value
            sqr  = (float)sqrtf( 1.0f + matrix[0] - matrix[5] - matrix[10] ) * 2.0f;

            // Calculate the x, y, x and w of the quaternion through the respective equation
            x = 0.25f * sqr;
            y = (matrix[4] + matrix[1] ) / sqr;
            z = (matrix[2] + matrix[8] ) / sqr;
            w = (matrix[9] - matrix[6] ) / sqr; 
        } 
        // Else if the second element of the diagonal is the greatest value
        else if ( matrix[5] > matrix[10] ) 
        {
            // Find the sqr according to the second element, and double that value
            sqr  = (float)sqrtf( 1.0f + matrix[5] - matrix[0] - matrix[10] ) * 2.0f;
            
            // Calculate the x, y, x and w of the quaternion through the respective equation
            x = (matrix[4] + matrix[1] ) / sqr;
            y = 0.25f * sqr;
            z = (matrix[9] + matrix[6] ) / sqr;
            w = (matrix[2] - matrix[8] ) / sqr;
        } 
        // Else the third element of the diagonal is the greatest value
        else 
        {   
            // Find the sqr according to the third element, and double that value
            sqr  = (float)sqrtf( 1.0f + matrix[10] - matrix[0] - matrix[5] ) * 2.0f;

            // Calculate the x, y, x and w of the quaternion through the respective equation
            x = (matrix[2] + matrix[8] ) / sqr;
            y = (matrix[9] + matrix[6] ) / sqr;
            z = 0.25f * sqr;
            w = (matrix[4] - matrix[1] ) / sqr;
        }
	
	}
}

void CQuaternion::BuildFromEuler(const Vector3f &v)
{
	// Doubles are used here intentionally for precisions sake
	double half_ex = (DEG_TO_RAD * v.x) / 2.0f;
	double half_ey = (DEG_TO_RAD * v.y) / 2.0f;
	double half_ez = (DEG_TO_RAD * v.z) / 2.0f;

	double cos_rol = cos(half_ex);
	double cos_pit = cos(half_ey);
	double cos_yaw = cos(half_ez);

	double sin_rol = sin(half_ex);
	double sin_pit = sin(half_ey);
	double sin_yaw = sin(half_ez);

	x = float(sin_rol * cos_pit * cos_yaw - cos_rol * sin_pit * sin_yaw);
	y = float(cos_rol * sin_pit * sin_yaw + sin_rol * cos_pit * sin_yaw);
	z = float(cos_rol * cos_pit * sin_yaw - sin_rol * sin_pit * cos_yaw);
	w = float(cos_rol * cos_pit * cos_yaw + sin_rol * sin_pit * sin_yaw);

	Normalize();
}

void CQuaternion::GetEulerAngles(Vector3f &result)
{
	CMatrix4f mat;
	CreateMatrix(mat);
	mat.GetEulerAngles(result);
}

// Fills the quaternion from the given axis angle of rotation
void CQuaternion::BuildFromAxisAngle(Vector3f axis, float degree)
{
	// degree is in degrees :P
	// Calculating sin( theta / 2)
	float angle = sinf( (degree * (PI/180.0f)) / 2.0f );
		
	// Calcualte the w value by cos( theta / 2 )
	w = cosf( degree / 2.0f ); // degree not angle (Jesse fix Jan 20, 2004)

	// Calculate the x, y and z of the quaternion
	x = axis.x * angle;
	y = axis.y * angle;
	z = axis.z * angle;
}

void CQuaternion::GetAxisAngle(Vector3f &axis, float &degree)
{
	float length = (x * x) + (y * y) + (z * z);

	if(length < EPSILON) {
		axis.x = 1.0f;
		axis.y = 0.0f;
		axis.z = 0.0f;
		degree = 0.0f;
	} else {
		float inv = 1.0f /  sqrtf(length);
		axis.x = x * inv;
		axis.y = y * inv;
		axis.z = z * inv;
		degree = 2.0f * acosf(w);
	}
}

void CQuaternion::Slerp(const CQuaternion &a, const CQuaternion &b, const float time)
{
	float o, cos_o, sin_o;
	float s1, s2;
	CQuaternion final;

	cos_o = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;

	if(cos_o < 0.0f) {
		cos_o = -cos_o; 
		final = CQuaternion(-b.x, -b.y, -b.z, -b.w);
	} else 
		final = CQuaternion(b.x, b.y, b.z, b.w);
	
	if((1.0f - cos_o) > EPSILON) { // Captian SLERP
		o = acosf(cos_o);
		sin_o = sinf(o);
		s1 = sinf((1.0f - time) * o) / sin_o;
		s2 = sinf(time * o) / sin_o;
	} else { // linearly interpolate between the two (m4d hax0r)
		s1 = 1.0f - time;
		s2 = time;
	}

	x = (s1 * a.x) + (s2 * final.x); // Bugfix here: b -> final  -- Sam
	y = (s1 * a.y) + (s2 * final.y);
	z = (s1 * a.z) + (s2 * final.z);
	w = (s1 * a.w) + (s2 * final.w);
}

