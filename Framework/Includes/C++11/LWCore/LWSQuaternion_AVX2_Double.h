#ifndef LWSQUATERNION_AVX2_DOUBLE_H
#define LWSQUATERNION_AVX2_DOUBLE_H
#include "LWCore/LWSQuaternion.h"
#include <smmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

template<>
struct alignas(__m256d) LWSQuaternion<double> {
	union {
		__m256d m_Data;
		struct {
			double x, y, z, w;
		};
	};

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

	double operator[](uint32_t i) const;

	double &operator[](uint32_t i);

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

	friend LWSQuaternion<double> operator * (double Lhs, const LWSQuaternion<double> &Rhs);

	friend LWSQuaternion<double> operator + (double Lhs, const LWSQuaternion<double> &Rhs);

	friend LWSQuaternion<double> operator - (double Lhs, const LWSQuaternion<double> &Rhs);

	friend LWSQuaternion<double> operator / (double Lhs, const LWSQuaternion<double> &Rhs);

	LWSQuaternion(__m256d Data);

	LWSQuaternion(const LWQuaternion<double> &Q);

	LWSQuaternion(double vw, double vx, double vy, double vz);

	LWSQuaternion(const LWSMatrix4<double>& Mat);

	LWSQuaternion();
};
#endif