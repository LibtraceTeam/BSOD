/* $Header$ $Log$
/* $Header$ Revision 1.3  2004/02/17 01:59:56  stj2
/* $Header$ Cvs tags will be correct at one point. Surely.
/* $Header$ */ 
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

