#ifndef LWSQUATERNION_AVX_FLOAT_H
#define LWSQUATERNION_AVX_FLOAT_H
#include "LWCore/LWSQuaternion.h"
#include <smmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

template<>
struct alignas(__m128) LWSQuaternion<float> {
	union {
		__m128 m_Data;
		struct {
			float x, y, z, w;
		};
	};

	static LWSQuaternion<float> FromEuler(float Pitch, float Yaw, float Roll);

	static LWSQuaternion<float> FromEuler(const LWVector3<float>& Euler);

	static LWSQuaternion<float> FromAxis(float xAxis, float yAxis, float zAxis, float Theta);

	static LWSQuaternion<float> FromAxis(const LWVector4<float>& AxisAngle);

	static LWSQuaternion<float> SLERP(const LWSQuaternion<float>& A, const LWSQuaternion<float>& B, float t);

	static LWSQuaternion<float> NLERP(const LWSQuaternion<float>& A, const LWSQuaternion<float>& B, float t);

	LWQuaternion<float> AsQuaternion(void) const;

	LWSVector4<float> AsSVec4(void) const;

	LWSVector4<float> ToEuler(void) const;

	LWSQuaternion Normalize(void) const;

	float LengthSq(void) const;

	float Length(void) const;

	float Dot(const LWSQuaternion& O) const;

	LWSQuaternion<float> Conjugate(void) const;

	LWSQuaternion<float> Inverse(void) const;

	LWSVector4<float> RotatePoint(const LWSVector4<float> Pnt) const;

	float operator[](uint32_t i) const;

	float &operator[](uint32_t i);

	bool operator == (const LWSQuaternion<float>& Rhs) const;

	bool operator != (const LWSQuaternion<float>& Rhs) const;

	LWSQuaternion<float> operator+(const LWSQuaternion<float>& Rhs) const;

	LWSQuaternion<float> operator-(const LWSQuaternion<float>& Rhs) const;

	LWSQuaternion<float> operator*(const LWSQuaternion<float>& Rhs) const;

	LWSQuaternion<float> operator*(float Rhs) const;

	friend std::ostream& operator<<(std::ostream& o, const LWSQuaternion<float>& q);

	LWSQuaternion<float>& operator*=(const LWSQuaternion<float>& Rhs);

	LWSQuaternion<float>& operator*=(float Rhs);

	LWSQuaternion<float>& operator+=(const LWSQuaternion<float>& Rhs);

	LWSQuaternion<float>& operator-=(const LWSQuaternion<float>& Rhs);

	LWSQuaternion<float>& operator=(const LWSQuaternion<float>& Rhs);

	LWSQuaternion<float> operator-() const;

	friend LWSQuaternion<float> operator * (float Lhs, const LWSQuaternion<float> &Rhs);

	friend LWSQuaternion<float> operator + (float Lhs, const LWSQuaternion<float> &Rhs);

	friend LWSQuaternion<float> operator - (float Lhs, const LWSQuaternion<float> &Rhs);

	friend LWSQuaternion<float> operator / (float Lhs, const LWSQuaternion<float> &Rhs);

	LWSQuaternion(__m128 Data);

	LWSQuaternion(const LWQuaternion<float> &Q);

	LWSQuaternion(float vw, float vx, float vy, float vz);
	
	LWSQuaternion(const LWSMatrix4<float>& Mat);

	LWSQuaternion();
};
#endif