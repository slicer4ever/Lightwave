#ifndef LWSQUATERNION_AVX2_DOUBLE_H
#define LWSQUATERNION_AVX2_DOUBLE_H
#include "LWCore/LWSQuaternion.h"
#include <smmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

template<>
struct LWSQuaternion<double> {
	__m256d m_Data;

	static LWSQuaternion<double> FromEuler(double Pitch, double Yaw, double Roll);

	static LWSQuaternion<double> FromEuler(const LWVector3<double>& Euler);

	static LWSQuaternion<double> FromAxis(double xAxis, double yAxis, double zAxis, double Theta);

	static LWSQuaternion<double> FromAxis(const LWVector4<double>& AxisAngle);

	static LWSQuaternion<double> SLERP(const LWSQuaternion<double>& A, const LWSQuaternion<double>& B, double t);

	static LWSQuaternion<double> NLERP(const LWSQuaternion<double>& A, const LWSQuaternion<double>& B, double t);

	LWQuaternion<double> AsQuaternion(void) const;

	LWSVector4<double> AsSVec4(void) const;

	LWSVector4<double> ToEuler(void) const;

	LWSQuaternion Normalize(void) const;

	double LengthSq(void) const;

	double Length(void) const;

	double Dot(const LWSQuaternion& O) const;

	LWSQuaternion<double> Conjugate(void) const;

	LWSQuaternion<double> Inverse(void) const;

	LWSVector4<double> RotatePoint(const LWSVector4<double> Pnt) const;

	bool operator == (const LWSQuaternion<double>& Rhs) const;

	bool operator != (const LWSQuaternion<double>& Rhs) const;

	LWSQuaternion<double> operator+(const LWSQuaternion<double>& Rhs) const;

	LWSQuaternion<double> operator-(const LWSQuaternion<double>& Rhs) const;

	LWSQuaternion<double> operator*(const LWSQuaternion<double>& Rhs) const;

	LWSQuaternion<double> operator*(double Rhs) const;

	friend std::ostream& operator<<(std::ostream& o, const LWSQuaternion<double>& q);

	LWSQuaternion<double>& operator*=(const LWSQuaternion<double>& Rhs);

	LWSQuaternion<double>& operator*=(double Rhs);

	LWSQuaternion<double>& operator+=(const LWSQuaternion<double>& Rhs);

	LWSQuaternion<double>& operator-=(const LWSQuaternion<double>& Rhs);

	LWSQuaternion<double>& operator=(const LWSQuaternion<double>& Rhs);

	LWSQuaternion<double> operator-() const;

	double x(void) const;

	double y(void) const;

	double z(void) const;

	double w(void) const;

	LWSQuaternion(__m256d Data);

	LWSQuaternion(double vw, double vx, double vy, double vz);

	LWSQuaternion(const LWSMatrix4<double>& Mat);

	LWSQuaternion();
};
#endif