/* $Header$ $Log$
/* $Header$ Revision 1.3  2004/02/17 01:59:56  stj2
/* $Header$ Cvs tags will be correct at one point. Surely.
/* $Header$ */ 
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

