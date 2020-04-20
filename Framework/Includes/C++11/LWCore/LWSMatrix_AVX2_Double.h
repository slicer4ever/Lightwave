#ifndef LWSMATRIX_AVX2_DOUBLE_H
#define LWSMATRIX_AVX2_DOUBLE_H
#include "LWCore/LWSMatrix.h"
#include <smmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

template<>
struct LWSMatrix4<double> {
	__m256d m_Row0;
	__m256d m_Row1;
	__m256d m_Row2;
	__m256d m_Row3;

	LWMatrix4<double> AsMat4(void) const;

	LWSMatrix4<double> TransformInverse(void) const;

	LWSMatrix4<double> Inverse(void) const;

	LWSVector4<double> Column(uint32_t Index) const;

	LWSVector4<double> Row(uint32_t Index) const;

	LWSMatrix4<double> Transpose(void) const;

	LWSMatrix4<double> Transpose3x3(void) const;

	LWSMatrix4<double> Transpose2x2(void) const;

	double Determinant(void) const;

	LWSMatrix4<double>& operator = (const LWSMatrix4<double>& Rhs);

	LWSMatrix4<double>& operator+= (const LWSMatrix4<double>& Rhs);

	LWSMatrix4<double>& operator-= (const LWSMatrix4<double>& Rhs);

	LWSMatrix4<double>& operator*= (const LWSMatrix4<double>& Rhs);

	LWSMatrix4<double>& operator *=(double Rhs);

	LWSMatrix4<double>& operator /=(double Rhs);

	bool operator == (const LWSMatrix4<double>& Rhs) const;

	bool operator != (const LWSMatrix4<double>& Rhs) const;

	friend std::ostream& operator<<(std::ostream& o, const LWSMatrix4<double>& M);

	friend LWSMatrix4<double> operator + (const LWSMatrix4<double>& Lhs, const LWSMatrix4<double>& Rhs);

	friend LWSMatrix4<double> operator - (const LWSMatrix4<double>& Lhs, const LWSMatrix4<double>& Rhs);

	friend LWSMatrix4<double> operator * (const LWSMatrix4<double>& Lhs, const LWSMatrix4<double>& Rhs);

	friend LWSMatrix4<double> operator * (const LWSMatrix4<double>& Lhs, double Rhs);

	friend LWSMatrix4<double> operator * (double Lhs, const LWSMatrix4<double>& Rhs);

	friend LWSMatrix4<double> operator / (const LWSMatrix4<double>& Lhs, double Rhs);

	friend LWSMatrix4<double> operator / (double Lhs, const LWSMatrix4<double>& Rhs);

	friend LWSVector4<double> operator * (const LWSMatrix4<double>& Lhs, const LWSVector4<double>& Rhs);

	friend LWSVector4<double> operator * (const LWSVector4<double>& Lhs, const LWSMatrix4<double>& Rhs);

	static LWSMatrix4<double> RotationX(double Theta);

	static LWSMatrix4<double> RotationY(double Theta);

	static LWSMatrix4<double> RotationZ(double Theta);

	static LWSMatrix4<double> RotationXY(double ThetaX, double ThetaY);

	static LWSMatrix4<double> RotationXZ(double ThetaX, double ThetaZ);

	static LWSMatrix4<double> RotationYX(double ThetaY, double ThetaX);

	static LWSMatrix4<double> RotationYZ(double ThetaY, double ThetaZ);

	static LWSMatrix4<double> RotationZX(double ThetaZ, double ThetaX);

	static LWSMatrix4<double> RotationZY(double ThetaZ, double ThetaY);

	static LWSMatrix4<double> RotationXYZ(double ThetaX, double ThetaY, double ThetaZ);

	static LWSMatrix4<double> RotationXZY(double ThetaX, double ThetaZ, double ThetaY);

	static LWSMatrix4<double> RotationYXZ(double ThetaY, double ThetaX, double ThetaZ);

	static LWSMatrix4<double> RotationYZX(double ThetaY, double ThetaZ, double ThetaX);

	static LWSMatrix4<double> RotationZXY(double ThetaZ, double ThetaX, double ThetaY);

	static LWSMatrix4<double> RotationZYX(double ThetaZ, double ThetaY, double ThetaX);

	static LWSMatrix4<double> Translation(double x, double y, double z);

	static LWSMatrix4<double> Translation(const LWSVector4<double>& Position);

	static LWSMatrix4<double> Rotation(const LWSVector4<double>& Direction, const LWSVector4<double>& Up);

	static LWSMatrix4<double> Perspective(double FoV, double Aspect, double Near, double Far);

	static LWSMatrix4<double> OrthoDX(double Left, double Right, double Bottom, double Top, double Near, double Far);

	static LWSMatrix4<double> OrthoGL(double Left, double Right, double Bottom, double Top, double Near, double Far);

	static LWSMatrix4<double> Ortho(double Left, double Right, double Bottom, double Top, double Near, double Far);

	static LWSMatrix4<double> Frustum(double Left, double Right, double Bottom, double Top, double Near, double Far);

	static LWSMatrix4<double> LookAt(const LWSVector4<double>& Position, const LWSVector4<double>& Target, const LWSVector4<double>& Up);

	LWSMatrix4(const LWSQuaternion<double>& Q);

	LWSMatrix4(__m256d Row0, __m256d Row1, __m256d Row2, __m256d Row3);

	LWSMatrix4(double xScale = 1.0, double yScale = 1.0, double zScale = 1.0, double wScale = 1.0);

	LWSMatrix4(const LWSVector4<double>& RowA, const LWSVector4<double>& RowB, const LWSVector4<double>& RowC, const LWSVector4<double>& RowD);

	LWSMatrix4(const LWSVector4<double>& Scale, const LWSQuaternion<double>& Rotation, const LWSVector4<double>& Pos);
};


#endif