#ifndef LWSMATRIX_AVX2_FLOAT_H
#define LWSMATRIX_AVX2_FLOAT_H
#include "LWSMatrix.h"
#include <smmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

template<>
struct LWSMatrix4<float> {
	__m256 m_Row01;
	__m256 m_Row23;

	LWMatrix4<float> AsMat4(void) const;
	
	LWSMatrix4<float> TransformInverse(void) const;
	
	LWSMatrix4<float> Inverse(void) const;
	
	LWSVector4<float> Column(uint32_t Index) const;
	
	LWSVector4<float> Row(uint32_t Index) const;

	LWSMatrix4<float> Transpose(void) const;
	
	LWSMatrix4<float> Transpose3x3(void) const;
	
	LWSMatrix4<float> Transpose2x2(void) const;
	
	float Determinant(void) const;

	LWSMatrix4<float>& operator = (const LWSMatrix4<float>& Rhs);

	LWSMatrix4<float>& operator+= (const LWSMatrix4<float>& Rhs);

	LWSMatrix4<float>& operator-= (const LWSMatrix4<float>& Rhs);

	LWSMatrix4<float>& operator*= (const LWSMatrix4<float>& Rhs);

	LWSMatrix4<float>& operator *=(float Rhs);

	LWSMatrix4<float>& operator /=(float Rhs);
	
	bool operator == (const LWSMatrix4<float>& Rhs) const;

	bool operator != (const LWSMatrix4<float>& Rhs) const;
	
	friend std::ostream& operator<<(std::ostream& o, const LWSMatrix4<float>& M);

	friend LWSMatrix4<float> operator + (const LWSMatrix4<float>& Lhs, const LWSMatrix4<float>& Rhs);

	friend LWSMatrix4<float> operator - (const LWSMatrix4<float>& Lhs, const LWSMatrix4<float>& Rhs);

	friend LWSMatrix4<float> operator * (const LWSMatrix4<float>& Lhs, const LWSMatrix4<float>& Rhs);

	friend LWSMatrix4<float> operator * (const LWSMatrix4<float>& Lhs, float Rhs);

	friend LWSMatrix4<float> operator * (float Lhs, const LWSMatrix4<float>& Rhs);

	friend LWSMatrix4<float> operator / (const LWSMatrix4<float>& Lhs, float Rhs);

	friend LWSMatrix4<float> operator / (float Lhs, const LWSMatrix4<float>& Rhs);

	friend LWSVector4<float> operator * (const LWSMatrix4<float>& Lhs, const LWSVector4<float>& Rhs);

	friend LWSVector4<float> operator * (const LWSVector4<float>& Lhs, const LWSMatrix4<float>& Rhs);

	static LWSMatrix4<float> RotationX(float Theta);

	static LWSMatrix4<float> RotationY(float Theta);

	static LWSMatrix4<float> RotationZ(float Theta);

	static LWSMatrix4<float> RotationXY(float ThetaX, float ThetaY);

	static LWSMatrix4<float> RotationXZ(float ThetaX, float ThetaZ);

	static LWSMatrix4<float> RotationYX(float ThetaY, float ThetaX);

	static LWSMatrix4<float> RotationYZ(float ThetaY, float ThetaZ);

	static LWSMatrix4<float> RotationZX(float ThetaZ, float ThetaX);

	static LWSMatrix4<float> RotationZY(float ThetaZ, float ThetaY);

	static LWSMatrix4<float> RotationXYZ(float ThetaX, float ThetaY, float ThetaZ);

	static LWSMatrix4<float> RotationXZY(float ThetaX, float ThetaZ, float ThetaY);

	static LWSMatrix4<float> RotationYXZ(float ThetaY, float ThetaX, float ThetaZ);

	static LWSMatrix4<float> RotationYZX(float ThetaY, float ThetaZ, float ThetaX);

	static LWSMatrix4<float> RotationZXY(float ThetaZ, float ThetaX, float ThetaY);

	static LWSMatrix4<float> RotationZYX(float ThetaZ, float ThetaY, float ThetaX);

	static LWSMatrix4<float> Translation(float x, float y, float z);

	static LWSMatrix4<float> Translation(const LWSVector4<float>& Position);

	static LWSMatrix4<float> Rotation(const LWSVector4<float>& Direction, const LWSVector4<float>& Up);

	static LWSMatrix4<float> Perspective(float FoV, float Aspect, float Near, float Far);

	static LWSMatrix4<float> OrthoDX(float Left, float Right, float Bottom, float Top, float Near, float Far);

	static LWSMatrix4<float> OrthoGL(float Left, float Right, float Bottom, float Top, float Near, float Far);

	static LWSMatrix4<float> Ortho(float Left, float Right, float Bottom, float Top, float Near, float Far);

	static LWSMatrix4<float> Frustum(float Left, float Right, float Bottom, float Top, float Near, float Far);

	static LWSMatrix4<float> LookAt(const LWSVector4<float>& Position, const LWSVector4<float>& Target, const LWSVector4<float>& Up);

	LWSMatrix4(const LWSQuaternion<float>& Q);

	LWSMatrix4(__m256 Row01, __m256 Row23);

	LWSMatrix4(float xScale = 1.0f, float yScale = 1.0f, float zScale = 1.0f, float wScale = 1.0f);

	LWSMatrix4(const LWSVector4<float>& RowA, const LWSVector4<float>& RowB, const LWSVector4<float>& RowC, const LWSVector4<float>& RowD);

	LWSMatrix4(const LWSVector4<float>& Scale, const LWSQuaternion<float>& Rotation, const LWSVector4<float>& Pos);
};

#endif