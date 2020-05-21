#ifndef LWMATRIX_H
#define LWMATRIX_H
#include "LWCore/LWTypes.h"
#include "LWCore/LWVector.h"
#include "LWCore/LWQuaternion.h"
#include <algorithm>
#include <ostream>

/*! \addtogroup LWCore
	@{
*/

/*!	\brief LWMatrix4 is a 4x4 row-major matrix object. */
template<class Type>
struct LWMatrix4{
	LWVector4<Type> m_Rows[4]; /*!< The 4x4 matrix in row-major order. */

	/*!< \brief returns the simd matrix4 of this matrix. */
	LWSMatrix4<Type> AsSMat4(void) const {
		return LWSMatrix4<Type>(m_Rows[0].AsSVec4(), m_Rows[1].AsSVec4(), m_Rows[2].AsSVec4(), m_Rows[3].AsSVec4());
	}

	/*! \brief returns the internal components as a array of data. */
	Type *AsArray(void) {
		return &m_Rows[0].x;
	}

	/*! \brief returns the internal components as a const array of data. */
	const Type *AsArray(void) const {
		return &m_Rows[0].x;
	}

	/*!< \brief returns the inverse of an transformation only matrix, if the matrix is more complex then this function will return incorrect results. */
	LWMatrix4 TransformInverse(void) const {
		const Type E = std::numeric_limits<Type>::epsilon();
		//Transpose matrix.
		LWVector3<Type> A = LWVector3<Type>(m_Rows[0].x, m_Rows[1].x, m_Rows[2].x);
		LWVector3<Type> B = LWVector3<Type>(m_Rows[0].y, m_Rows[1].y, m_Rows[2].y);
		LWVector3<Type> C = LWVector3<Type>(m_Rows[0].z, m_Rows[1].z, m_Rows[2].z);

		LWVector3<Type> sizeSq = A * A + B * B + C * C;
		LWVector3<Type> rsizeSq = (Type)1 / sizeSq;
		if (sizeSq.x < E) rsizeSq.x = 1.0;
		if (sizeSq.y < E) rsizeSq.y = 1.0;
		if (sizeSq.z < E) rsizeSq.z = 1.0;
		A *= rsizeSq;
		B *= rsizeSq;
		C *= rsizeSq;

		LWVector3<Type> D = -LWVector3<Type>(A*m_Rows[3].x + B*m_Rows[3].y + C*m_Rows[3].z);
		return LWMatrix4({ A, 0 }, { B, 0 }, { C, 0 }, { D ,1 });
	}

	/*! \brief writes into Result the transform inverse of the matrix. */
	void TransformInverse(LWMatrix4 &Result) const {
		Result = TransformInverse();
	}

	/*! \brief Returns an copied inverse of this matrix for general matrix's. */
	LWMatrix4 Inverse(void) const{
		//Found from: https://stackoverflow.com/questions/1148309/inverting-a-4x4-matrix
		LWVector4<Type> A = m_Rows[0];
		LWVector4<Type> B = m_Rows[1];
		LWVector4<Type> C = m_Rows[2];
		LWVector4<Type> D = m_Rows[3];

		Type A2323 = C.z * D.w - C.w * D.z; //m.m22 * m.m33 - m.m23 * m.m32;
		Type A1323 = C.y * D.w - C.w * D.y; //m.m21 * m.m33 - m.m23 * m.m31;
		Type A1223 = C.y * D.z - C.z * D.y; //m.m21 * m.m32 - m.m22 * m.m31;
		Type A0323 = C.x * D.w - C.w * D.x; //m.m20 * m.m33 - m.m23 * m.m30;

		Type A0223 = C.x * D.z - C.z * D.x; //m.m20 * m.m32 - m.m22 * m.m30;
		Type A0123 = C.x * D.y - C.y * D.x; //m.m20 * m.m31 - m.m21 * m.m30;
		Type A2313 = B.z * D.w - B.w * D.z; //m.m12 * m.m33 - m.m13 * m.m32;
		Type A1313 = B.y * D.w - B.w * D.y; //m.m11 * m.m33 - m.m13 * m.m31;

		Type A1213 = B.y * D.z - B.z * D.y; //m.m11 * m.m32 - m.m12 * m.m31;
		Type A2312 = B.z * C.w - B.w * C.z; //m.m12 * m.m23 - m.m13 * m.m22;
		Type A1312 = B.y * C.w - B.w * C.y; //m.m11 * m.m23 - m.m13 * m.m21;
		Type A1212 = B.y * C.z - B.z * C.y; //m.m11 * m.m22 - m.m12 * m.m21;

		Type A0313 = B.x * D.w - B.w * D.x; //m.m10 * m.m33 - m.m13 * m.m30;
		Type A0213 = B.x * D.z - B.z * D.x; //m.m10 * m.m32 - m.m12 * m.m30;
		Type A0312 = B.x * C.w - B.w * C.x; //m.m10 * m.m23 - m.m13 * m.m20;
		Type A0212 = B.x * C.z - B.z * C.x; //m.m10 * m.m22 - m.m12 * m.m20;

		Type A0113 = B.x * D.y - B.y * D.x; //m.m10 * m.m31 - m.m11 * m.m30;
		Type A0112 = B.x * C.y - B.y * C.x; //m.m10 * m.m21 - m.m11 * m.m20;

		Type Det = A.x * (B.y * A2323 - B.z * A1323 + B.w * A1223) -
			A.y * (B.x * A2323 - B.z * A0323 + B.w * A0223) +
			A.z * (B.x * A1323 - B.y * A0323 + B.w * A0123) -
			A.w * (B.x * A1223 - B.y * A0223 + B.y * A0123);
		Det = abs(Det) <= std::numeric_limits<Type>::epsilon() ? (Type)0 : (Type)1 / Det;

		
		Type Ax =  B.y * A2323 - B.z * A1323 + B.w * A1223;  // (m.m11 * A2323 - m.m12 * A1323 + m.m13 * A1223)
		Type Ay =-(A.y * A2323 - A.z * A1323 + A.w * A1223); //-(m.m01 * A2323 - m.m02 * A1323 + m.m03 * A1223)
		Type Az =  A.y * A2313 - A.z * A1313 + A.w * A1213;  // (m.m01 * A2313 - m.m02 * A1313 + m.m03 * A1213)
		Type Aw =-(A.y * A2312 - A.z * A1312 + A.w * A1212); //-(m.m01 * A2312 - m.m02 * A1312 + m.m03 * A1212)

		Type Bx =-(B.x * A2323 - B.z * A0323 + B.w * A0223); //-(m.m10 * A2323 - m.m12 * A0323 + m.m13 * A0223)
		Type By =  A.x * A2323 - A.z * A0323 + A.w * A0223;  // (m.m00 * A2323 - m.m02 * A0323 + m.m03 * A0223)
		Type Bz =-(A.x * A2313 - A.z * A0313 + A.w * A0213); //-(m.m00 * A2313 - m.m02 * A0313 + m.m03 * A0213);
		Type Bw =  A.x * A2312 - A.z * A0312 + A.w * A0212;  // (m.m00 * A2312 - m.m02 * A0312 + m.m03 * A0212)

		Type Cx =  B.x * A1323 - B.y * A0323 + B.w * A0123;  // (m.m10 * A1323 - m.m11 * A0323 + m.m13 * A0123)
		Type Cy =-(A.x * A1323 - A.y * A0323 + A.w * A0123); //-(m.m00 * A1323 - m.m01 * A0323 + m.m03 * A0123)
		Type Cz =  A.x * A1313 - A.y * A0313 + A.w * A0113;  // (m.m00 * A1313 - m.m01 * A0313 + m.m03 * A0113),
		Type Cw =-(A.x * A1312 - A.y * A0312 + A.w * A0112); //-(m.m00 * A1312 - m.m01 * A0312 + m.m03 * A0112),
		
		Type Dx =-(B.x * A1223 - B.y * A0223 + B.z * A0123); //-(m.m10 * A1223 - m.m11 * A0223 + m.m12 * A0123)
		Type Dy =  A.x * A1223 - A.y * A0223 + A.z * A0123;  // (m.m00 * A1223 - m.m01 * A0223 + m.m02 * A0123)
		Type Dz =-(A.x * A1213 - A.y * A0213 + A.z * A0113); //-(m.m00 * A1213 - m.m01 * A0213 + m.m02 * A0113)
		Type Dw =  A.x * A1212 - A.y * A0212 + A.z * A0112;  // (m.m00 * A1212 - m.m01 * A0212 + m.m02 * A0112)
		return LWMatrix4({ Ax, Ay, Az, Aw }, { Bx, By, Bz, Bw }, { Cx, Cy, Cz, Cw }, { Dx, Dy, Dz, Dw })*Det;

	}

	/*!< \brief returns the specified column of the matrix. */
	LWVector4<Type> Column(uint32_t Index) {
		Type *Arry = &m_Rows[0].x;
		return LWVector4<Type>(Arry[Index], Arry[Index + 4], Arry[Index + 8], Arry[Index + 12]);
	};

	/*! \brief returns the specified row of the matrix(this function exists for parody to LWSMatrix4's row function.) */
	LWVector4<Type> Row(uint32_t Index) {
		return m_Rows[Index];
	}

	/*! \brief set's a value of the matrix at row*4+Column position, this function exists for parody to LWSMatrix4's sRC function.) */
	LWMatrix4 &sRC(uint32_t Row, uint32_t Column, Type Value) {
		Type *Arry = &m_Rows[0].x;
		Arry[Row * 4 + Column] = Value;
		return *this;
	}

	/*! \brief returns the transpose of the this matrix. */
	LWMatrix4 Transpose(void) const{
		return LWMatrix4(LWVector4<Type>(m_Rows[0].x, m_Rows[1].x, m_Rows[2].x, m_Rows[3].x), LWVector4<Type>(m_Rows[0].y, m_Rows[1].y, m_Rows[2].y, m_Rows[3].y), LWVector4<Type>(m_Rows[0].z, m_Rows[1].z, m_Rows[2].z, m_Rows[3].z), LWVector4<Type>(m_Rows[0].w, m_Rows[1].w, m_Rows[2].w, m_Rows[3].w));
	}

	/*! \brief writes into Result the transpose of this matrix. */
	void Transpose(LWMatrix4 &Result) const {
		Result = Transpose();
		return;
	}

	/*!< \brief returns the upper left 3x3 matrix transposed only. */
	LWMatrix4 Transpose3x3(void) const {
		return LWMatrix4(LWVector4<Type>(m_Rows[0].x, m_Rows[1].x, m_Rows[2].x, m_Rows[0].w),
			LWVector4<Type>(m_Rows[0].y, m_Rows[1].y, m_Rows[2].y, m_Rows[1].w),
			LWVector4<Type>(m_Rows[0].z, m_Rows[1].z, m_Rows[2].z, m_Rows[2].w),
			m_Rows[3]);
	}

	/*! \brief writes into result the upper left 3x3 matrix transpose of this matrix. */
	void Transpose3x3(LWMatrix4 &Result) const {
		Result = Transpose3x3();
		return;
	}

	/*!< \brief returns the upper left 2x2 matrix transposed only. */
	LWMatrix4 Transpose2x2(void) const {
		return LWMatrix4(LWVector4<Type>(m_Rows[0].x, m_Rows[1].x, m_Rows[0].z, m_Rows[0].w),
			LWVector4<Type>(m_Rows[0].y, m_Rows[1].y, m_Rows[1].z, m_Rows[1].w), m_Rows[2], m_Rows[3]);
	}

	/*!< \brief writes into result the upper left 2x3 matrix transpose of this matrix. */
	void Transpose2x2(LWMatrix4 &Result) const {
		Result = Transpose2x2();
		return;
	}
	
	/*!< \brief decomposes 4x4 matrix to get the scalar for each axis. 
		 \param Transpose will transpose the 3x3 rotation+scale matrix before calculating the scale/rotation. */
	LWVector3<Type> DecomposeScale(bool Transpose3x3) const {
		LWVector3<Type> R0 = LWVector3<Type>(m_Rows[0].x, m_Rows[0].y, m_Rows[0].z);
		LWVector3<Type> R1 = LWVector3<Type>(m_Rows[1].x, m_Rows[1].y, m_Rows[1].z);
		LWVector3<Type> R2 = LWVector3<Type>(m_Rows[2].x, m_Rows[2].y, m_Rows[2].z);
		if (Transpose3x3) {
			R0 = LWVector3<Type>(m_Rows[0].x, m_Rows[1].x, m_Rows[2].x);
			R1 = LWVector3<Type>(m_Rows[0].y, m_Rows[1].y, m_Rows[2].y);
			R2 = LWVector3<Type>(m_Rows[0].z, m_Rows[1].z, m_Rows[2].z);
		}
		return LWVector3<Type>(R0.Length(), R1.Length(), R2.Length());
	};

	/*!< \brief decomposes 4x4 matrix to get scale, rotation, and translation.
		 \param Transpose will transpose the 3x3 rotation+scale matrix before calculating the scale/rotation.*/
	void Decompose(LWVector3<Type> &Scale, LWQuaternion<Type> &Rotation, LWVector3<Type> &Translation, bool Transpose3x3) const {
		LWVector3<Type> R0 = LWVector3<Type>(m_Rows[0].x, m_Rows[0].y, m_Rows[0].z);
		LWVector3<Type> R1 = LWVector3<Type>(m_Rows[1].x, m_Rows[1].y, m_Rows[1].z);
		LWVector3<Type> R2 = LWVector3<Type>(m_Rows[2].x, m_Rows[2].y, m_Rows[2].z);
		if (Transpose3x3) {
			R0 = LWVector3<Type>(m_Rows[0].x, m_Rows[1].x, m_Rows[2].x);
			R1 = LWVector3<Type>(m_Rows[0].y, m_Rows[1].y, m_Rows[2].y);
			R2 = LWVector3<Type>(m_Rows[0].z, m_Rows[1].z, m_Rows[2].z);
		}
		Scale = LWVector3<Type>(R0.Length(), R1.Length(), R2.Length());
		LWVector3<Type> iScale = 1.0 / Scale;
		Rotation = LWQuaternion<Type>(LWMatrix3<Type>(R0*iScale.x, R1*iScale.y, R2*iScale.z));
		Translation = LWVector3<Type>(m_Rows[3].x, m_Rows[3].y, m_Rows[3].z);
		return;
	}

	/*! \brief calculates the determinant of this matrix. */
	Type Determinant(void) const{
		//Found from: https://stackoverflow.com/questions/1148309/inverting-a-4x4-matrix
		LWVector4<Type> A = m_Rows[0];
		LWVector4<Type> B = m_Rows[1];
		LWVector4<Type> C = m_Rows[2];
		LWVector4<Type> D = m_Rows[3];

		Type A2323 = C.z * D.w - C.w * D.z; //m.m22 * m.m33 - m.m23 * m.m32;
		Type A1323 = C.y * D.w - C.w * D.y; //m.m21 * m.m33 - m.m23 * m.m31;
		Type A1223 = C.y * D.z - C.z * D.y; //m.m21 * m.m32 - m.m22 * m.m31;
		Type A0323 = C.x * D.w - C.w * D.x; //m.m20 * m.m33 - m.m23 * m.m30;
		Type A0223 = C.x * D.z - C.z * D.x; //m.m20 * m.m32 - m.m22 * m.m30;
		Type A0123 = C.x * D.y - C.y * D.x; //m.m20 * m.m31 - m.m21 * m.m30;

		return A.x * (B.y * A2323 - B.z * A1323 + B.w * A1223) -
			A.y * (B.x * A2323 - B.z * A0323 + B.w * A0223) +
			A.z * (B.x * A1323 - B.y * A0323 + B.w * A0123) -
			A.w * (B.x * A1223 - B.y * A0223 + B.y * A0123);
	}

	/*! \cond */
	LWMatrix4 &operator = (const LWMatrix4 &Rhs){
		m_Rows[0] = Rhs.m_Rows[0];
		m_Rows[1] = Rhs.m_Rows[1];
		m_Rows[2] = Rhs.m_Rows[2];
		m_Rows[3] = Rhs.m_Rows[3];
		return *this;
	}

	LWMatrix4 &operator+= (const LWMatrix4 &Rhs){
		m_Rows[0] += Rhs.m_Rows[0];
		m_Rows[1] += Rhs.m_Rows[1];
		m_Rows[2] += Rhs.m_Rows[2];
		m_Rows[3] += Rhs.m_Rows[3];
		return *this;
	}

	LWMatrix4 &operator-= (const LWMatrix4 &Rhs){
		m_Rows[0] -= Rhs.m_Rows[0];
		m_Rows[1] -= Rhs.m_Rows[1];
		m_Rows[2] -= Rhs.m_Rows[2];
		m_Rows[3] -= Rhs.m_Rows[3];
		return *this;
	}

	LWMatrix4 &operator*= (const LWMatrix4 &Rhs){
		Type Ax = m_Rows[0].x*Rhs.m_Rows[0].x + m_Rows[0].y*Rhs.m_Rows[1].x + m_Rows[0].z*Rhs.m_Rows[2].x + m_Rows[0].w*Rhs.m_Rows[3].x;
		Type Ay = m_Rows[0].x*Rhs.m_Rows[0].y + m_Rows[0].y*Rhs.m_Rows[1].y + m_Rows[0].z*Rhs.m_Rows[2].y + m_Rows[0].w*Rhs.m_Rows[3].y;
		Type Az = m_Rows[0].x*Rhs.m_Rows[0].z + m_Rows[0].y*Rhs.m_Rows[1].z + m_Rows[0].z*Rhs.m_Rows[2].z + m_Rows[0].w*Rhs.m_Rows[3].z;
		Type Aw = m_Rows[0].x*Rhs.m_Rows[0].w + m_Rows[0].y*Rhs.m_Rows[1].w + m_Rows[0].z*Rhs.m_Rows[2].w + m_Rows[0].w*Rhs.m_Rows[3].w;
		Type Bx = m_Rows[1].x*Rhs.m_Rows[0].x + m_Rows[1].y*Rhs.m_Rows[1].x + m_Rows[1].z*Rhs.m_Rows[2].x + m_Rows[1].w*Rhs.m_Rows[3].x;
		Type By = m_Rows[1].x*Rhs.m_Rows[0].y + m_Rows[1].y*Rhs.m_Rows[1].y + m_Rows[1].z*Rhs.m_Rows[2].y + m_Rows[1].w*Rhs.m_Rows[3].y;
		Type Bz = m_Rows[1].x*Rhs.m_Rows[0].z + m_Rows[1].y*Rhs.m_Rows[1].z + m_Rows[1].z*Rhs.m_Rows[2].z + m_Rows[1].w*Rhs.m_Rows[3].z;
		Type Bw = m_Rows[1].x*Rhs.m_Rows[0].w + m_Rows[1].y*Rhs.m_Rows[1].w + m_Rows[1].z*Rhs.m_Rows[2].w + m_Rows[1].w*Rhs.m_Rows[3].w;
		Type Cx = m_Rows[2].x*Rhs.m_Rows[0].x + m_Rows[2].y*Rhs.m_Rows[1].x + m_Rows[2].z*Rhs.m_Rows[2].x + m_Rows[2].w*Rhs.m_Rows[3].x;
		Type Cy = m_Rows[2].x*Rhs.m_Rows[0].y + m_Rows[2].y*Rhs.m_Rows[1].y + m_Rows[2].z*Rhs.m_Rows[2].y + m_Rows[2].w*Rhs.m_Rows[3].y;
		Type Cz = m_Rows[2].x*Rhs.m_Rows[0].z + m_Rows[2].y*Rhs.m_Rows[1].z + m_Rows[2].z*Rhs.m_Rows[2].z + m_Rows[2].w*Rhs.m_Rows[3].z;
		Type Cw = m_Rows[2].x*Rhs.m_Rows[0].w + m_Rows[2].y*Rhs.m_Rows[1].w + m_Rows[2].z*Rhs.m_Rows[2].w + m_Rows[2].w*Rhs.m_Rows[3].w;
		Type Dx = m_Rows[3].x*Rhs.m_Rows[0].x + m_Rows[3].y*Rhs.m_Rows[1].x + m_Rows[3].z*Rhs.m_Rows[2].x + m_Rows[3].w*Rhs.m_Rows[3].x;
		Type Dy = m_Rows[3].x*Rhs.m_Rows[0].y + m_Rows[3].y*Rhs.m_Rows[1].y + m_Rows[3].z*Rhs.m_Rows[2].y + m_Rows[3].w*Rhs.m_Rows[3].y;
		Type Dz = m_Rows[3].x*Rhs.m_Rows[0].z + m_Rows[3].y*Rhs.m_Rows[1].z + m_Rows[3].z*Rhs.m_Rows[2].z + m_Rows[3].w*Rhs.m_Rows[3].z;
		Type Dw = m_Rows[3].x*Rhs.m_Rows[0].w + m_Rows[3].y*Rhs.m_Rows[1].w + m_Rows[3].z*Rhs.m_Rows[2].w + m_Rows[3].w*Rhs.m_Rows[3].w;
		m_Rows[0] = LWVector4<Type>(Ax, Ay, Az, Aw);
		m_Rows[1] = LWVector4<Type>(Bx, By, Bz, Bw);
		m_Rows[2] = LWVector4<Type>(Cx, Cy, Cz, Cw);
		m_Rows[3] = LWVector4<Type>(Dx, Dy, Dz, Dw);
		return *this;
	}

	LWMatrix4 &operator *=(Type Rhs){
		m_Rows[0] *= Rhs;
		m_Rows[1] *= Rhs;
		m_Rows[2] *= Rhs;
		m_Rows[3] *= Rhs;
		return *this;
	}

	LWMatrix4 &operator /=(Type Rhs){
		m_Rows[0] /= Rhs;
		m_Rows[1] /= Rhs;
		m_Rows[2] /= Rhs;
		m_Rows[3] /= Rhs;
		return *this;
	}

	bool operator == (const LWMatrix4 &Rhs) const{
		return m_Rows[0] == Rhs.m_Rows[0] && m_Rows[1] == Rhs.m_Rows[1] && m_Rows[2] == Rhs.m_Rows[2] && m_Rows[3] == Rhs.m_Rows[3];
	}

	bool operator != (const LWMatrix4 &Rhs) const{
		return !(*this == Rhs);
	}

	friend std::ostream &operator<<(std::ostream &o, const LWMatrix4<Type> &M) {
		o << "Matrix4:" << std::endl;
		o << M.m_Rows[0] << std::endl;
		o << M.m_Rows[1] << std::endl;
		o << M.m_Rows[2] << std::endl;
		o << M.m_Rows[3];
		return o;
	}

	friend LWMatrix4 operator + (const LWMatrix4 &Lhs, const LWMatrix4 &Rhs){
		return LWMatrix4(Lhs.m_Rows[0] + Rhs.m_Rows[0], Lhs.m_Rows[1] + Rhs.m_Rows[1], Lhs.m_Rows[2] + Rhs.m_Rows[2], Lhs.m_Rows[3] + Rhs.m_Rows[3]);
	}

	friend LWMatrix4 operator - (const LWMatrix4 &Lhs, const LWMatrix4 &Rhs){
		return LWMatrix4(Lhs.m_Rows[0] - Rhs.m_Rows[0], Lhs.m_Rows[1] - Rhs.m_Rows[1], Lhs.m_Rows[2] - Rhs.m_Rows[2], Lhs.m_Rows[3] - Rhs.m_Rows[3]);
	}

	friend LWMatrix4 operator * (const LWMatrix4 &Lhs, const LWMatrix4 &Rhs) {
		Type Ax = Lhs.m_Rows[0].x*Rhs.m_Rows[0].x + Lhs.m_Rows[0].y*Rhs.m_Rows[1].x + Lhs.m_Rows[0].z*Rhs.m_Rows[2].x + Lhs.m_Rows[0].w*Rhs.m_Rows[3].x;
		Type Ay = Lhs.m_Rows[0].x*Rhs.m_Rows[0].y + Lhs.m_Rows[0].y*Rhs.m_Rows[1].y + Lhs.m_Rows[0].z*Rhs.m_Rows[2].y + Lhs.m_Rows[0].w*Rhs.m_Rows[3].y;
		Type Az = Lhs.m_Rows[0].x*Rhs.m_Rows[0].z + Lhs.m_Rows[0].y*Rhs.m_Rows[1].z + Lhs.m_Rows[0].z*Rhs.m_Rows[2].z + Lhs.m_Rows[0].w*Rhs.m_Rows[3].z;
		Type Aw = Lhs.m_Rows[0].x*Rhs.m_Rows[0].w + Lhs.m_Rows[0].y*Rhs.m_Rows[1].w + Lhs.m_Rows[0].z*Rhs.m_Rows[2].w + Lhs.m_Rows[0].w*Rhs.m_Rows[3].w;
		Type Bx = Lhs.m_Rows[1].x*Rhs.m_Rows[0].x + Lhs.m_Rows[1].y*Rhs.m_Rows[1].x + Lhs.m_Rows[1].z*Rhs.m_Rows[2].x + Lhs.m_Rows[1].w*Rhs.m_Rows[3].x;
		Type By = Lhs.m_Rows[1].x*Rhs.m_Rows[0].y + Lhs.m_Rows[1].y*Rhs.m_Rows[1].y + Lhs.m_Rows[1].z*Rhs.m_Rows[2].y + Lhs.m_Rows[1].w*Rhs.m_Rows[3].y;
		Type Bz = Lhs.m_Rows[1].x*Rhs.m_Rows[0].z + Lhs.m_Rows[1].y*Rhs.m_Rows[1].z + Lhs.m_Rows[1].z*Rhs.m_Rows[2].z + Lhs.m_Rows[1].w*Rhs.m_Rows[3].z;
		Type Bw = Lhs.m_Rows[1].x*Rhs.m_Rows[0].w + Lhs.m_Rows[1].y*Rhs.m_Rows[1].w + Lhs.m_Rows[1].z*Rhs.m_Rows[2].w + Lhs.m_Rows[1].w*Rhs.m_Rows[3].w;
		Type Cx = Lhs.m_Rows[2].x*Rhs.m_Rows[0].x + Lhs.m_Rows[2].y*Rhs.m_Rows[1].x + Lhs.m_Rows[2].z*Rhs.m_Rows[2].x + Lhs.m_Rows[2].w*Rhs.m_Rows[3].x;
		Type Cy = Lhs.m_Rows[2].x*Rhs.m_Rows[0].y + Lhs.m_Rows[2].y*Rhs.m_Rows[1].y + Lhs.m_Rows[2].z*Rhs.m_Rows[2].y + Lhs.m_Rows[2].w*Rhs.m_Rows[3].y;
		Type Cz = Lhs.m_Rows[2].x*Rhs.m_Rows[0].z + Lhs.m_Rows[2].y*Rhs.m_Rows[1].z + Lhs.m_Rows[2].z*Rhs.m_Rows[2].z + Lhs.m_Rows[2].w*Rhs.m_Rows[3].z;
		Type Cw = Lhs.m_Rows[2].x*Rhs.m_Rows[0].w + Lhs.m_Rows[2].y*Rhs.m_Rows[1].w + Lhs.m_Rows[2].z*Rhs.m_Rows[2].w + Lhs.m_Rows[2].w*Rhs.m_Rows[3].w;
		Type Dx = Lhs.m_Rows[3].x*Rhs.m_Rows[0].x + Lhs.m_Rows[3].y*Rhs.m_Rows[1].x + Lhs.m_Rows[3].z*Rhs.m_Rows[2].x + Lhs.m_Rows[3].w*Rhs.m_Rows[3].x;
		Type Dy = Lhs.m_Rows[3].x*Rhs.m_Rows[0].y + Lhs.m_Rows[3].y*Rhs.m_Rows[1].y + Lhs.m_Rows[3].z*Rhs.m_Rows[2].y + Lhs.m_Rows[3].w*Rhs.m_Rows[3].y;
		Type Dz = Lhs.m_Rows[3].x*Rhs.m_Rows[0].z + Lhs.m_Rows[3].y*Rhs.m_Rows[1].z + Lhs.m_Rows[3].z*Rhs.m_Rows[2].z + Lhs.m_Rows[3].w*Rhs.m_Rows[3].z;
		Type Dw = Lhs.m_Rows[3].x*Rhs.m_Rows[0].w + Lhs.m_Rows[3].y*Rhs.m_Rows[1].w + Lhs.m_Rows[3].z*Rhs.m_Rows[2].w + Lhs.m_Rows[3].w*Rhs.m_Rows[3].w;
		return LWMatrix4(LWVector4<Type>(Ax, Ay, Az, Aw), LWVector4<Type>(Bx, By, Bz, Bw), LWVector4<Type>(Cx, Cy, Cz, Cw), LWVector4<Type>(Dx, Dy, Dz, Dw));
	}

	friend LWMatrix4 operator * (const LWMatrix4 &Lhs, Type Rhs){
		return LWMatrix4(Lhs.m_Rows[0] * Rhs, Lhs.m_Rows[1] * Rhs, Lhs.m_Rows[2] * Rhs, Lhs.m_Rows[3] * Rhs);
	}

	friend LWMatrix4 operator * (Type Lhs, const LWMatrix4 &Rhs){
		return LWMatrix4(Lhs*Rhs.m_Rows[0], Lhs*Rhs.m_Rows[1], Lhs*Rhs.m_Rows[2], Lhs*Rhs.m_Rows[3]);
	}

	friend LWMatrix4 operator / (const LWMatrix4 &Lhs, Type Rhs){
		return LWMatrix4(Lhs.m_Rows[0] / Rhs, Lhs.m_Rows[1] / Rhs, Lhs.m_Rows[2] / Rhs, Lhs.m_Rows[3] / Rhs);
	}

	friend LWMatrix4 operator / (Type Lhs, const LWMatrix4 &Rhs){
		return LWMatrix4(Lhs / Rhs.m_Rows[0], Lhs / Rhs.m_Rows[1], Lhs / Rhs.m_Rows[2], Lhs / Rhs.m_Rows[3]);
	}

	friend LWVector4<Type> operator * (const LWMatrix4 &Lhs, const LWVector4<Type> &Rhs){
		return LWVector4<Type>(
			Lhs.m_Rows[0].x*Rhs.x + Lhs.m_Rows[1].x*Rhs.y + Lhs.m_Rows[2].x*Rhs.z + Lhs.m_Rows[3].x*Rhs.w,
			Lhs.m_Rows[0].y*Rhs.x + Lhs.m_Rows[1].y*Rhs.y + Lhs.m_Rows[2].y*Rhs.z + Lhs.m_Rows[3].y*Rhs.w,
			Lhs.m_Rows[0].z*Rhs.x + Lhs.m_Rows[1].z*Rhs.y + Lhs.m_Rows[2].z*Rhs.z + Lhs.m_Rows[3].z*Rhs.w,
			Lhs.m_Rows[0].w*Rhs.x + Lhs.m_Rows[1].w*Rhs.y + Lhs.m_Rows[2].w*Rhs.z + Lhs.m_Rows[3].w*Rhs.w);
	}

	friend LWVector4<Type> operator * (const LWVector4<Type> &Lhs, const LWMatrix4 &Rhs){
		return LWVector4<Type>(
			Lhs.x*Rhs.m_Rows[0].x + Lhs.y*Rhs.m_Rows[1].x + Lhs.z*Rhs.m_Rows[2].x + Lhs.w*Rhs.m_Rows[3].x,
			Lhs.x*Rhs.m_Rows[0].y + Lhs.y*Rhs.m_Rows[1].y + Lhs.z*Rhs.m_Rows[2].y + Lhs.w*Rhs.m_Rows[3].y,
			Lhs.x*Rhs.m_Rows[0].z + Lhs.y*Rhs.m_Rows[1].z + Lhs.z*Rhs.m_Rows[2].z + Lhs.w*Rhs.m_Rows[3].z,
			Lhs.x*Rhs.m_Rows[0].w + Lhs.y*Rhs.m_Rows[1].w + Lhs.z*Rhs.m_Rows[2].w + Lhs.w*Rhs.m_Rows[3].w);
	}

	friend LWVector3<Type> operator * (const LWMatrix4 &Lhs, const LWVector3<Type> &Rhs){
		return LWVector3<Type>(
			Lhs.m_Rows[0].x*Rhs.x + Lhs.m_Rows[1].x*Rhs.y + Lhs.m_Rows[2].x*Rhs.z + Lhs.m_Rows[3].x,
			Lhs.m_Rows[0].y*Rhs.x + Lhs.m_Rows[1].y*Rhs.y + Lhs.m_Rows[2].y*Rhs.z + Lhs.m_Rows[3].y,
			Lhs.m_Rows[0].z*Rhs.x + Lhs.m_Rows[1].z*Rhs.y + Lhs.m_Rows[2].z*Rhs.z + Lhs.m_Rows[3].z);
	}

	friend LWVector3<Type> operator * (const LWVector3<Type> &Lhs, const LWMatrix4 &Rhs){
		return LWVector3<Type>(Lhs.x*Rhs.m_Rows[0].x + Lhs.y*Rhs.m_Rows[1].x + Lhs.z*Rhs.m_Rows[2].x + Rhs.m_Rows[3].x,
			Lhs.x*Rhs.m_Rows[0].y + Lhs.y*Rhs.m_Rows[1].y + Lhs.z*Rhs.m_Rows[2].y + Rhs.m_Rows[3].y,
			Lhs.x*Rhs.m_Rows[0].z + Lhs.y*Rhs.m_Rows[1].z + Lhs.z*Rhs.m_Rows[2].z + Rhs.m_Rows[3].z);
	}

	friend LWVector2<Type> operator * (const LWMatrix4 &Lhs, const LWVector2<Type> &Rhs){
		return LWVector2<Type>(Lhs.m_Rows[0].x*Rhs.x + Lhs.m_Rows[1].x*Rhs.y + Lhs.m_Rows[3].x,
			Lhs.m_Rows[0].y*Rhs.x + Lhs.m_Rows[1].y*Rhs.y + Lhs.m_Rows[3].y);
	}

	friend LWVector2<Type> operator * (const LWVector2<Type> &Lhs, const LWMatrix4 &Rhs){
		return LWVector2<Type>(Lhs.x*Rhs.m_Rows[0].x + Lhs.y*Rhs.m_Rows[1].x + Rhs.m_Rows[3].x,
			Lhs.x*Rhs.m_Rows[0].y + Lhs.y*Rhs.m_Rows[1].y + Rhs.m_Rows[3].y);
	}
	/*! \endcond */

	/*! \brief returns a matrix rotated around the x axis.
		\param Theta the angle to use.
		*/
	static LWMatrix4 RotationX(Type Theta){
		Type S = sin(Theta);
		Type C = cos(Theta);
		return LWMatrix4({ 1, 0, 0, 0 },
					 	 { 0, C, -S, 0 },
						 { 0, S, C, 0 },
						 { 0, 0, 0, 1 });
	}

	/*! \brief writes into Result a matrix rotated around the x axis.
		\param Theta the angle to use.
		\param Result the matrix to receive the result.
		*/
	static void RotationX(Type Theta, LWMatrix4 &Result){
		Result = RotationX(Theta);
		return;
	}


	/*! \brief returns a matrix rotated around the y axis.
		\param Theta the angle to use.
		*/
	static LWMatrix4 RotationY(Type Theta){
		Type S = sin(Theta);
		Type C = cos(Theta);
		return LWMatrix4({ C, 0, S, 0 },
						 { 0, 1, 0, 0 },
						 {-S, 0, C, 0 },
						 { 0, 0, 0, 1 });
	}
	/*! \brief writes into Result a matrix rotated around the y axis.
		\param Theta the angle to use.
		\param Result the matrix to receive the result.
		*/
	static void RotationY(Type Theta, LWMatrix4 &Result){
		Result = RotationY(Theta);
		return;
	}

	/*! \brief returns a matrix rotated around the z axis.
		\param Theta the angle to use.
		*/
	static LWMatrix4 RotationZ(Type Theta){
		Type S = sin(Theta);
		Type C = cos(Theta);
		return LWMatrix4({ C, -S, 0, 0 },
						 { S, C, 0, 0 },
						 { 0, 0, 1, 0},
						 { 0, 0, 0, 1});
	}
	/*! \brief returns a matrix rotated around the z axis.
		\param Theta the angle to use.
		\param Result the matrix to receive the result.
		*/
	static void RotationZ(Type Theta, LWMatrix4 &Result){
		Result = RotationZ(Theta);
		return;
	}

	/*! \brief returns a matrix rotated both around the x and y axis in that order.
		\param ThetaX the x-axis angle.
		\param ThetaY the y-axis angle.
		*/
	static LWMatrix4 RotationXY(Type ThetaX, Type ThetaY){
		return RotationX(ThetaX)*RotationY(ThetaY);
	}

	/*! \brief writes into result a matrix rotated around the x and y axis in that order.
		\param ThetaX the x-axis angle.
		\param ThetaY the y-axis angle.
		\param Result the matrix to receive the result.
		*/
	static void RotationXY(Type ThetaX, Type ThetaY, LWMatrix4 &Result){
		Result = RotationXY(ThetaX, ThetaY);
		return;
	}

	/*! \brief returns a matrix rotated both around the x and z axis.
		\param ThetaX the x-axis angle.
		\param ThetaZ the z-axis angle.
		*/
	static LWMatrix4 RotationXZ(Type ThetaX, Type ThetaZ){
		return RotationX(ThetaX)*RotationZ(ThetaZ);
	}

	/*! \brief writes into result a matrix rotated around the x and z axis in that order.
		\param ThetaX the x-axis angle.
		\param ThetaZ the z-axis angle.
		\param Result the matrix to receive the result.
		*/
	static void RotationXZ(Type ThetaX, Type ThetaZ, LWMatrix4 &Result){
		Result = RotationXZ(ThetaX, ThetaZ);
		return;
	}

	/*! \brief returns a matrix rotated both around the y and x axis in that order.
		\param ThetaY the y-axis angle.
		\param ThetaX the x-axis angle.
		*/
	static LWMatrix4 RotationYX(Type ThetaY, Type ThetaX){
		return RotationY(ThetaY)*RotationX(ThetaX);
	}

	/*! \brief writes into result a matrix rotated both around the y and x axis in that order.
		\param ThetaY the y-axis angle.
		\param ThetaX the z-axis angle.
		\param Result the matrix to receive the result.
		*/
	static void RotationYX(Type ThetaY, Type ThetaX, LWMatrix4 &Result){
		Result = RotationYX(ThetaY, ThetaX);
		return;
	}

	/*! \brief returns a matrix rotated both around the y and z axis in that order.
		\param ThetaY the y-axis angle.
		\param ThetaZ the z-axis angle.
		*/
	static LWMatrix4 RotationYZ(Type ThetaY, Type ThetaZ){
		return RotationY(ThetaY)*RotationZ(ThetaZ);
	}

	/*! \brief writes into result a matrix rotated both around the y and z axis in that order.
		\param ThetaY the y-axis angle.
		\param ThetaZ the z-axis angle.
		\param Result the matrix to receive the result.
		*/
	static void RotationYZ(Type ThetaY, Type ThetaZ, LWMatrix4 &Result){
		Result = RotationYZ(ThetaY, ThetaZ);
		return;
	}

	/*! \brief returns a matrix rotated both around the z and x axis in that order.
		\param ThetaZ the z-axis angle.
		\param ThetaX the x-axis angle.
		*/

	static LWMatrix4 RotationZX(Type ThetaZ, Type ThetaX){
		return RotationZ(ThetaZ)*RotationX(ThetaX);
	}

	/*! \brief writes into result a matrix rotated both around the z and x axis in that order.
		\param ThetaZ the z-axis angle.
		\param ThetaX the x-axis angle.
		\param Result the matrix to receive the result.
		*/
	static void RotationZX(Type ThetaZ, Type ThetaX, LWMatrix4 &Result){
		Result = RotationZX(ThetaZ, ThetaX);
		return;
	}

	/*! \brief returns a matrix rotated both around the z and y axis in that order.
		\param ThetaZ the z-axis angle.
		\param ThetaY the y-axis angle.
		*/
	static LWMatrix4 RotationZY(Type ThetaZ, Type ThetaY){
		return RotationZ(ThetaZ)*RotationY(ThetaY);
	}

	/*! \brief writes into result a matrix rotated both around the z and y axis in that order.
		\param ThetaZ the z-axis angle.
		\param ThetaY the y-axis angle.
		\param Result the matrix to receive the result.
		*/
	static void RotationZY(Type ThetaZ, Type ThetaY, LWMatrix4 &Result){
		Result = RotationZY(ThetaZ, ThetaY);
		return;
	}

	/*! \brief returns a matrix rotated around the x, y, and z axis in that order.
		\param ThetaX the x-axis angle.
		\param ThetaY the y-axis angle.
		\param ThetaZ the z-axis angle.
		*/
	static LWMatrix4 RotationXYZ(Type ThetaX, Type ThetaY, Type ThetaZ){
		return RotationX(ThetaX)*RotationY(ThetaY)*RotationZ(ThetaZ);
	}

	/*! \brief writes into result a matrix rotated around the x, y, and z axis in that order.
		\param ThetaX the x-axis angle.
		\param ThetaY the y-axis angle.
		\param ThetaZ the z-axis angle.
		\param Result the matrix to receive the result.
		*/
	static void RotationXYZ(Type ThetaX, Type ThetaY, Type ThetaZ, LWMatrix4 &Result){
		Result = RotationXYZ(ThetaX, ThetaY, ThetaZ);
		return;
	}

	/*! \brief returns a matrix rotated around the x, z, and y axis in that order.
		\param ThetaX the x-axis angle.
		\param ThetaZ the z-axis angle.
		\param ThetaY the y-axis angle.
		*/
	static LWMatrix4 RotationXZY(Type ThetaX, Type ThetaZ, Type ThetaY){
		return RotationX(ThetaX)*RotationZ(ThetaZ)*RotationY(ThetaY);
	}

	/*! \brief writes into result a matrix rotated around the x, z, and y axis in that order.
		\param ThetaX the x-axis angle.
		\param ThetaZ the z-axis angle.
		\param ThetaY the y-axis angle.
		\param Result the matrix to receive the result.
		*/
	static void RotationXZY(Type ThetaX, Type ThetaZ, Type ThetaY, LWMatrix4 &Result){
		Result = RotationXZY(ThetaX, ThetaZ, ThetaY);
		return;
	}

	/*! \brief returns a matrix rotated around the y, x, and z axis in that order.
		\param ThetaY the y-axis angle.
		\param ThetaX the x-axis angle.
		\param ThetaZ the z-axis angle.
		*/
	static LWMatrix4 RotationYXZ(Type ThetaY, Type ThetaX, Type ThetaZ){
		return RotationY(ThetaY)*RotationX(ThetaX)*RotationZ(ThetaZ);
	}

	/*! \brief writes into result a matrix rotated around the y, x, and z axis in that order.
		\param ThetaY the y-axis angle.
		\param ThetaX the x-axis angle.
		\param ThetaZ the z-axis angle.
		\param Result the matrix to receive the result.
		*/
	static void RotationYXZ(Type ThetaY, Type ThetaX, Type ThetaZ, LWMatrix4 &Result){
		Result = RotationYXZ(ThetaY, ThetaX, ThetaZ);
		return;
	}

	/*! \brief returns a matrix rotated around the y, z, and x axis in that order.
		\param ThetaY the y-axis angle.
		\param ThetaZ the z-axis angle.
		\param ThetaX the x-axis angle.
		*/
	static LWMatrix4 RotationYZX(Type ThetaY, Type ThetaZ, Type ThetaX){
		return RotationY(ThetaY)*RotationZ(ThetaZ)*RotationX(ThetaX);
	}

	/*! \brief writes into result a matrix rotated around the y, z, and x axis in that order.
		\param ThetaY the y-axis angle.
		\param ThetaZ the z-axis angle.
		\param ThetaX the x-axis angle.
		\param Result the matrix to receive the result.
		*/
	static void RotationYZX(Type ThetaY, Type ThetaZ, Type ThetaX, LWMatrix4 &Result){
		Result = RotationYZX(ThetaY, ThetaZ, ThetaX);
		return;
	}

	/*! \brief returns a matrix rotated around the z, x, and y axis in that order.
		\param ThetaZ the z-axis angle.
		\param ThetaX the x-axis angle.
		\param ThetaY the y-axis angle.
		*/
	static LWMatrix4 RotationZXY(Type ThetaZ, Type ThetaX, Type ThetaY){
		return RotationZ(ThetaZ)*RotationX(ThetaX)*RotationY(ThetaY);
	}

	/*! \brief writes into result a matrix rotated around the z, x, and y axis in that order.
		\param ThetaZ the z-axis angle.
		\param ThetaX the x-axis angle.
		\param ThetaY the y-axis angle.
		\param Result the matrix to receive the result.
		*/
	static void RotationZXY(Type ThetaZ, Type ThetaX, Type ThetaY, LWMatrix4 &Result){
		Result = RotationZXY(ThetaZ, ThetaX, ThetaY);
		return;
	}

	/*! \brief returns a matrix rotated around the z, y, and x axis in that order.
		\param ThetaZ the z-axis angle.
		\param ThetaY the y-axis angle.
		\param ThetaX the x-axis angle.
		*/
	static LWMatrix4 RotationZYX(Type ThetaZ, Type ThetaY, Type ThetaX){
		return RotationZ(ThetaZ)*RotationY(ThetaY)*RotationX(ThetaX);
	}

	/*! \brief writes into result a matrix rotated around the z, y, and x axis in that order.
		\param ThetaZ the z-axis angle.
		\param ThetaY the y-axis angle.
		\param ThetaX the x-axis angle.
		\param Result the matrix to receive the result.
		*/
	static void RotationZYX(Type ThetaZ, Type ThetaY, Type ThetaX, LWMatrix4 &Result){
		Result = RotationZYX(ThetaZ, ThetaY, ThetaX);
		return;
	}

	/*! \brief returns a matrix translated along the x, y, and z axis.
		\param x the x axis to translate along.
		\param y the y axis to translate along.
		\param z the z axis to translate along.
		*/
	static LWMatrix4 Translation(Type x, Type y, Type z){
		return Translation({ x, y, z });
	}

	/*! \brief writes into result a matrix translated along the x, y, and z axis.
		\param x the x axis to translate along.
		\param y the y axis to translate along.
		\param z the z axis to translate along.
		\param Result the matrix to receive the result.
		*/
	static void Translation(Type x, Type y, Type z, LWMatrix4 &Result){
		Result = Translation(x, y, z);
	}

	/*! \brief returns a matrix translated along the x, y, and z axis in the position vector3.
		\param Position the translation to apply.
	*/
	static LWMatrix4 Translation(const LWVector3<Type> &Position){
		return LWMatrix4({ 1, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, 1, 0 }, { Position, 1 });
	}


	/*! \brief writes into a matrix translated along the x, y, and z axis in the position vector3.
		\param Position the translation to apply.
		\param Result the matrix to receive the result.
	*/

	static void Translation(const LWVector3<Type> &Position, LWMatrix4 &Result){
		Result = Translation(Position);
		return;
	}

	/*!< \brief constructs a rotation matrix from the directional vector which is relative to the supplied up vector.
		 \param Direction the normalized directional vector.
		 \param Up the normalized up vector.
	*/
	static LWMatrix4 Rotation(const LWVector3<Type> &Direction, const LWVector3<Type> &Up) {
		LWVector3<Type> xAxis = Up.Cross(Direction);
		LWVector3<Type> yAxis = Direction.Cross(xAxis);
		return LWMatrix4<Type>(
		{ xAxis.x, xAxis.y, xAxis.z, 0.0f },
		{ yAxis.x, yAxis.y, yAxis.z, 0.0f },
		{ Direction.x, Direction.y, Direction.z, 0.0f },
		{ 0.0f, 0.0f, 0.0f, 1.0f });
	}

	/*!< \brief constructs a rotation matrix from the directional vector which is relative to the supplied up vector. stores the new matrix into result.
		 \param Direction the normalized directional vector.
		 \param Up the normalized up vector.
		 \param Result the matrix to receive the result.
	*/
	static void Rotation(const LWVector3<Type> &Direction, const LWVector3<Type> &Up, LWMatrix4<Type> &Result) {
		Result = Rotation(Direction, Up);
		return;
	}

	/*! \brief returns a camera's perspective matrix.
		\param FoV the Field of View angle that is multiplied by the aspect.
		\param Aspect the ratio of the views width/height.
		\param Near the near plane of the camera.
		\param Far the far plane of the camera.
		*/
	static LWMatrix4 Perspective(Type FoV, Type Aspect, Type Near, Type Far){
		Type F = 1 / tan(FoV / 2);
		return LWMatrix4({ F / Aspect, 0, 0,                            0 }, 
						 { 0,          F, 0,                            0 }, 
						 { 0,          0, (Far + Near) / (Near - Far), -1 }, 
						 { 0,          0, (Type)2.0*Far*Near / (Near - Far),  0 });
	}

	/*! \brief writes into result a camera's perspective matrix.
		\param FoV the Field of View angle that is multipled by the aspect.
		\param Aspect the ratio of the views width/height.
		\param Near the near plane of the camera.
		\param Far the far plane of the camera.
		\param Result the matrix to receive the result.
		*/
	static void Pespective(Type FoV, Type Aspect, Type Near, Type Far, LWMatrix4 &Result){
		Result = Perspective(FoV, Aspect, Near, Far);
		return;
	}

	/*! \brief returns a camera's boxed orthographic matrix for d3d11+metal NDC's where z range is 0-1.
		\param Left the left side of the camera's view.
		\param Right the right side of the camera's view.
		\param Bottom the bottom side of the camera's view.
		\param Top the top side of the camera's view.
		\param Near the camera's near plane.
		\param Far the camera's far plane.
	*/
	static LWMatrix4 OrthoDX(Type Left, Type Right, Type Bottom, Type Top, Type Near, Type Far) {
		Type sDepth = 1 / (Far - Near);
		return LWMatrix4({ 2 / (Right - Left), 0, 0, 0 },
			{ 0, 2 / (Top - Bottom), 0, 0 },
			{ 0, 0, -sDepth, 0 },
			{ -(Right + Left) / (Right - Left), -(Top + Bottom) / (Top - Bottom), Near*sDepth, 1 });
	}

	/*! \brief writes into result a camera's boxed orthographic matrix for d3d11+metal NDC's(z range of 0-1).
		\param Left the left side of the camera's view.
		\param Right the right side of the camera's view.
		\param Bottom the bottom side of the camera's view.
		\param Top the top side of the camera's view.
		\param Near the camera's near plane.
		\param Far the camera's far plane.
		\param Result the matrix to receive the result.
		*/
	static void OrthoDX(Type Left, Type Right, Type Bottom, Type Top, Type Near, Type Far, LWMatrix4 &Result) {
		Result = OrthoDX(Left, Right, Bottom, Top, Near, Far);
		return;
	}


	/*! \brief returns a camera's boxed orthographic matrix for openGL NDC's(z range of -1-1).
		\param Left the left side of the camera's view.
		\param Right the right side of the camera's view.
		\param Bottom the bottom side of the camera's view.
		\param Top the top side of the camera's view.
		\param Near the camera's near plane.
		\param Far the camera's far plane.
		*/

	static LWMatrix4 OrthoGL(Type Left, Type Right, Type Bottom, Type Top, Type Near, Type Far){
		return LWMatrix4({ 2 / (Right - Left), 0, 0, 0 },
						 { 0, 2 / (Top - Bottom), 0, 0 },
						 { 0, 0, -2/(Far-Near), 0 },
						 { -(Right + Left) / (Right - Left), -(Top + Bottom) / (Top - Bottom), -(Near+Far)/(Far-Near), 1 });
	}

	/*! \brief writes into result a camera's boxed orthographic matrix for openGL NDC's(z range of -1-1).
		\param Left the left side of the camera's view.
		\param Right the right side of the camera's view.
		\param Bottom the bottom side of the camera's view.
		\param Top the top side of the camera's view.
		\param Near the camera's near plane.
		\param Far the camera's far plane.
		\param Result the matrix to receive the result.
		*/
	static void OrthoGL(Type Left, Type Right, Type Bottom, Type Top, Type Near, Type Far, LWMatrix4 &Result){
		Result = OrthoGL(Left, Right, Bottom, Top, Near, Far);
		return;
	}

	/*! \brief writes into result a camera's boxed orthographic matrix depending on if LWMatrix4_UseDXOrtho is set to true or not
		\param Left the left side of the camera's view.
		\param Right the right side of the camera's view.
		\param Bottom the bottom side of the camera's view.
		\param Top the top side of the camera's view.
		\param Near the camera's near plane.
		\param Far the camera's far plane.
		\param Result the matrix to receive the result.
		\note the value of LWMatrix4_UseDXOrtho is changed when a video driver is created, if for some odd reason multiple video drivers are being used concurrently, you should not rely on this function.
		*/
	static void Ortho(Type Left, Type Right, Type Bottom, Type Top, Type Near, Type Far, LWMatrix4 &Result) {
		Result = Ortho(Left, Right, Bottom, Top, Near, Far);
		return;
	}

	/*! \brief returns a camera's boxed orthographic matrix depending on if LWMatrix4_UseDXOrtho is set to true or not
.
		\param Left the left side of the camera's view.
		\param Right the right side of the camera's view.
		\param Bottom the bottom side of the camera's view.
		\param Top the top side of the camera's view.
		\param Near the camera's near plane.
		\param Far the camera's far plane.
		\note the value of LWMatrix4_UseDXOrtho is changed when a video driver is created, if for some odd reason multiple video drivers are being used concurrently, you should not rely on this function.
	*/

	static LWMatrix4 Ortho(Type Left, Type Right, Type Bottom, Type Top, Type Near, Type Far) {
		if (LWMatrix4_UseDXOrtho) return OrthoDX(Left, Right, Bottom, Top, Near, Far);
		return OrthoGL(Left, Right, Bottom, Top, Near, Far);
	}

	/*! \brief returns a frustum camera matrix.
		\param Left the left side of the camera's view.
		\param Right the right side of the camera's view.
		\param Bottom the bottom side of the camera's view.
		\param Top the top side of the camera's view.
		\param Near the camera's near plane.
		\param Far the camera's far plane.
		*/
	static LWMatrix4 Frustum(Type Left, Type Right, Type Bottom, Type Top, Type Near, Type Far){
		return LWMatrix4({ 2 * Near / (Right - Left), 0, (Right + Left) / (Right - Left), 0 },
						 { 0, 2 * Near / (Top - Bottom), (Top + Bottom) / (Top - Bottom), 0 },
						 { 0, 0, -(Far + Near) / (Far - Near), -1 },
						 { 0, 0, -2 * Far*Near / (Far - Near), 0 });
	}
	/*! \brief writes into result a frustum camera matrix.
		\param Left the left side of the camera's view.
		\param Right the right side of the camera's view.
		\param Bottom the bottom side of the camera's view.
		\param Top the top side of the camera's view.
		\param Near the camera's near plane.
		\param Far the camera's far plane.
		\param Result the matrix to receive the result.
		*/
	static void Frustum(Type Left, Type Right, Type Bottom, Type Top, Type Near, Type Far, LWMatrix4 &Result){
		Result = Frustum(Left, Right, Bottom, Top, Near, Far);
		return;
	}

	/*! \brief generates a view matrix for looking at a specific target.
		\param Position the position of the viewing agent.
		\param Target the target to look at.
		\param Up the up matrix to work off of.
		*/
	static LWMatrix4 LookAt(const LWVector3<Type> &Position, const LWVector3<Type> &Target, const LWVector3<Type> &Up){
		LWVector3<Type> Fwrd = (Target - Position).Normalize();
		LWVector3<Type> Rght = Fwrd.Cross(Up).Normalize();
		LWVector3<Type> U = Rght.Cross(Fwrd);
		return LWMatrix4({ Rght, 0 }, { U, 0 }, { -Fwrd, 0 }, { Position, 1 });
	}

	/*! \brief writes into result a view matrix for looking at a specific target.
		\param Position the position of the viewing agent.
		\param Target the target to look at.
		\param Up the up matrix to work off of.
		\param Result the matrix to receive the result.
	*/
	static void LookAt(const LWVector3<Type> &Position, const LWVector3<Type> &Target, const LWVector3<Type> &Up, LWMatrix4 &Result){
		Result = LookAt(Position, Target, Up);
		return;
	}

	/*!< \brief constructs a 4x4 rotational matrix from the specified quaternion. */
	LWMatrix4(const LWQuaternion<Type> &Q) {
		Type xx = Q.x*Q.x;
		Type xy = Q.x*Q.y;
		Type xz = Q.x*Q.z;
		Type xw = Q.x*Q.w;
		Type yy = Q.y*Q.y;
		Type yz = Q.y*Q.z;
		Type yw = Q.y*Q.w;

		Type zz = Q.z*Q.z;
		Type zw = Q.z*Q.w;

		m_Rows[0] = { (Type)(1 - 2 * (yy + zz)), (Type)(2 * (xy - zw)), (Type)(2 * (xz + yw)), (Type)0 };
		m_Rows[1] = { (Type)2 * (xy + zw), (Type)(1 - 2 * (xx + zz)), (Type)(2 * (yz - xw)), 0 };
		m_Rows[2] = { (Type)(2 * (xz - yw)), (Type)(2 * (yz + xw)), (Type)(1 - 2 * (xx + yy)), (Type)0 };
		m_Rows[3] = { (Type)0, (Type)0, (Type)0, (Type)1 };

	}

	/*! \brief constructs a 4x4 matrix with each row scaled by the respective scales.
	*/
	LWMatrix4(Type xScale = 1, Type yScale = 1, Type zScale = 1, Type wScale = 1){
		m_Rows[0] = { xScale, 0, 0, 0 };
		m_Rows[1] = { 0, yScale, 0, 0 };
		m_Rows[2] = { 0, 0, zScale, 0 };
		m_Rows[3] = { 0, 0, 0, wScale };
	}
	
	/*! \brief constructs a 4x4 matrix where each row is set by the applications. */
	LWMatrix4(const LWVector4<Type> &RowA, const LWVector4<Type> &RowB, const LWVector4<Type> &RowC, const LWVector4<Type> &RowD){
		m_Rows[0] = RowA;
		m_Rows[1] = RowB;
		m_Rows[2] = RowC;
		m_Rows[3] = RowD;
	}

	/*!< \brief constructs a 4x4 matrix from Scale, Rotation, Position components. */
	LWMatrix4(const LWVector3<Type> &Scale, const LWQuaternion<Type> &Rotation, const LWVector3<Type> &Pos) {
		const LWQuaternion<Type> &Q = Rotation;
		Type xx = Q.x*Q.x;
		Type xy = Q.x*Q.y;
		Type xz = Q.x*Q.z;
		Type xw = Q.x*Q.w;
		Type yy = Q.y*Q.y;
		Type yz = Q.y*Q.z;
		Type yw = Q.y*Q.w;

		Type zz = Q.z*Q.z;
		Type zw = Q.z*Q.w;

		m_Rows[0] = { (Type)(1 - 2 * (yy + zz)), (Type)(2 * (xy - zw)), (Type)(2 * (xz + yw)), (Type)0 }*Scale.x;
		m_Rows[1] = { (Type)2 * (xy + zw), (Type)(1 - 2 * (xx + zz)), (Type)(2 * (yz - xw)), 0 }*Scale.y;
		m_Rows[2] = { (Type)(2 * (xz - yw)), (Type)(2 * (yz + xw)), (Type)(1 - 2 * (xx + yy)), (Type)0 }*Scale.z;
		m_Rows[3] = { (Type)Pos.x, (Type)Pos.y, (Type)Pos.z, (Type)1 };
	};
};

/*! \brief LWMatrix3 is a 3x3 row-major matrix object.
*/

template<class Type>
struct LWMatrix3{
	LWVector3<Type> m_Rows[3]; /*!< The 4x4 matrix in row-major order. */


	/*!< \brief returns the specified column of the matrix. */
	LWVector3<Type> Column(uint32_t Index) {
		Type *Arry = &m_Rows[0].x;
		return LWVector3<Type>(Arry[Index], Arry[Index + 3], Arry[Index + 6]);
	};

	/*! \brief returns the specified row of the matrix(this function exists for parody to LWSMatrix4's row function.) */
	LWVector3<Type> Row(uint32_t Index) {
		return m_Rows[Index];
	}

	/*! \brief set's a value of the matrix at row*3+Column position, this function exists for parody to LWSMatrix4's sRC function.) */
	LWMatrix3 &sRC(uint32_t Row, uint32_t Column, Type Value) {
		Type *Arry = &m_Rows[0].x;
		Arry[Row * 3 + Column] = Value;
		return *this;
	}

	/*! \brief Returns an copied inverse of this matrix. */
	LWMatrix3 Inverse(void) const{
		Type D = Determinant();
		if (abs(D) <= std::numeric_limits<Type>::epsilon()) return LWMatrix3();
		LWMatrix3 Result;
		Result.m_Rows[0] = LWVector3<Type>(m_Rows[1].y*m_Rows[2].z - m_Rows[1].z*m_Rows[2].x, m_Rows[0].z*m_Rows[2].y - m_Rows[0].y*m_Rows[2].z, m_Rows[0].y*m_Rows[1].z - m_Rows[0].z*m_Rows[1].y)*D;
		Result.m_Rows[1] = LWVector3<Type>(m_Rows[1].z*m_Rows[2].x - m_Rows[1].x*m_Rows[2].z, m_Rows[0].x*m_Rows[2].z - m_Rows[0].z*m_Rows[2].x, m_Rows[0].z*m_Rows[1].x - m_Rows[0].x*m_Rows[1].z)*D;
		Result.m_Rows[2] = LWVector3<Type>(m_Rows[1].x*m_Rows[2].y - m_Rows[1].y*m_Rows[2].x, m_Rows[0].y*m_Rows[2].x - m_Rows[0].x*m_Rows[2].y, m_Rows[0].x*m_Rows[1].y - m_Rows[0].y*m_Rows[1].x)*D;
		return Result;
	}

	/*! \brief writes into Result the inverse of the matrix. */
	void Inverse(LWMatrix3 &Result) const{
		Result = Inverse();
		return;
	}

	/*! \brief returns the transpose of the this matrix. */
	LWMatrix3 Transpose(void) const{
		return LWMatrix3({ m_Rows[0].x, m_Rows[1].x, m_Rows[2].x }, { m_Rows[0].y, m_Rows[1].y, m_Rows[2].y }, { m_Rows[0].z, m_Rows[1].z, m_Rows[2].z });
	}

	/*! \brief writes into Result the transpose of this matrix. */
	void Transpose(LWMatrix3 &Result) const{
		Result = Transpose();
		return;
	}

	/*!< \brief returns the upper left 2x2 matrix transposed only. */
	LWMatrix3 Transpose2x2(void) const {
		return LWMatrix3(LWVector3<Type>(m_Rows[0].x, m_Rows[1].x, m_Rows[0].z),
			LWVector3<Type>(m_Rows[0].y, m_Rows[1].y, m_Rows[1].z));
	}

	/*!< \brief writes into result the upper left 2x3 matrix transpose of this matrix. */
	void Transpose2x2(LWMatrix3 &Result) const {
		Result = Transpose2x2();
		return;
	}

	/*! \brief calculates the determinant of this matrix. */
	Type Determinant(void) const{
		return m_Rows[0].x*m_Rows[1].y*m_Rows[2].z + m_Rows[0].y*m_Rows[1].z*m_Rows[2].x - m_Rows[0].x*m_Rows[1].z*m_Rows[2].y - m_Rows[0].y*m_Rows[1].x*m_Rows[2].z - m_Rows[0].z*m_Rows[1].y*m_Rows[2].x;
	}

	/*! \cond */
	LWMatrix3 &operator = (const LWMatrix3 &Rhs){
		m_Rows[0] = Rhs.m_Rows[0];
		m_Rows[1] = Rhs.m_Rows[1];
		m_Rows[2] = Rhs.m_Rows[2];
		return *this;
	}

	LWMatrix3 &operator+= (const LWMatrix3 &Rhs){
		m_Rows[0] += Rhs.m_Rows[0];
		m_Rows[1] += Rhs.m_Rows[1];
		m_Rows[2] += Rhs.m_Rows[2];
		return *this;
	}

	LWMatrix3 &operator-= (const LWMatrix3 &Rhs){
		m_Rows[0] -= Rhs.m_Rows[0];
		m_Rows[1] -= Rhs.m_Rows[1];
		m_Rows[2] -= Rhs.m_Rows[2];
		return *this;
	}

	LWMatrix3 &operator*= (const LWMatrix3 &Rhs){
		Type Ax = m_Rows[0].x*Rhs.m_Rows[0].x + m_Rows[0].y*Rhs.m_Rows[1].x + m_Rows[0].z*Rhs.m_Rows[2].x;
		Type Ay = m_Rows[0].x*Rhs.m_Rows[0].y + m_Rows[0].y*Rhs.m_Rows[1].y + m_Rows[0].z*Rhs.m_Rows[2].y;
		Type Az = m_Rows[0].x*Rhs.m_Rows[0].z + m_Rows[0].y*Rhs.m_Rows[1].z + m_Rows[0].z*Rhs.m_Rows[2].z;
		Type Bx = m_Rows[1].x*Rhs.m_Rows[0].x + m_Rows[1].y*Rhs.m_Rows[1].x + m_Rows[1].z*Rhs.m_Rows[2].x;
		Type By = m_Rows[1].x*Rhs.m_Rows[0].y + m_Rows[1].y*Rhs.m_Rows[1].y + m_Rows[1].z*Rhs.m_Rows[2].y;
		Type Bz = m_Rows[1].x*Rhs.m_Rows[0].z + m_Rows[1].y*Rhs.m_Rows[1].z + m_Rows[1].z*Rhs.m_Rows[2].z;
		Type Cx = m_Rows[2].x*Rhs.m_Rows[0].x + m_Rows[2].y*Rhs.m_Rows[1].x + m_Rows[2].z*Rhs.m_Rows[2].x;
		Type Cy = m_Rows[2].x*Rhs.m_Rows[0].y + m_Rows[2].y*Rhs.m_Rows[1].y + m_Rows[2].z*Rhs.m_Rows[2].y;
		Type Cz = m_Rows[2].x*Rhs.m_Rows[0].z + m_Rows[2].y*Rhs.m_Rows[1].z + m_Rows[2].z*Rhs.m_Rows[2].z;
		m_Rows[0] = { Ax, Ay, Az };
		m_Rows[1] = { Bx, By, Bz };
		m_Rows[2] = { Cx, Cy, Cz };
		return *this;
	}

	LWMatrix3 &operator *=(Type Rhs){
		m_Rows[0] *= Rhs;
		m_Rows[1] *= Rhs;
		m_Rows[2] *= Rhs;
		return *this;
	}

	LWMatrix3 &operator /=(Type Rhs){
		m_Rows[0] /= Rhs;
		m_Rows[1] /= Rhs;
		m_Rows[2] /= Rhs;
		return *this;
	}

	bool operator == (const LWMatrix3 &Rhs) const{
		return m_Rows[0] == Rhs.m_Rows[0] && m_Rows[1] == Rhs.m_Rows[1] && m_Rows[2] == Rhs.m_Rows[2];
	}

	bool operator != (const LWMatrix3 &Rhs) const{
		return !(*this == Rhs);
	}

	friend std::ostream &operator<<(std::ostream &o, const LWMatrix3<Type> &M) {
		o << "Matrix3:" << std::endl;
		o << M.m_Rows[0] << std::endl;
		o << M.m_Rows[1] << std::endl;
		o << M.m_Rows[2];
		return o;
	}

	friend LWMatrix3 operator + (const LWMatrix3 &Lhs, const LWMatrix3 &Rhs){
		return LWMatrix3(Lhs.m_Rows[0] + Rhs.m_Rows[0], Lhs.m_Rows[1] + Rhs.m_Rows[1], Lhs.m_Rows[2] + Rhs.m_Rows[2]);
	}

	friend LWMatrix3 operator - (const LWMatrix3 &Lhs, const LWMatrix3 &Rhs){
		return LWMatrix3(Lhs.m_Rows[0] - Rhs.m_Rows[0], Lhs.m_Rows[1] - Rhs.m_Rows[1], Lhs.m_Rows[2] - Rhs.m_Rows[2]);
	}

	friend LWMatrix3 operator * (const LWMatrix3 &Lhs, const LWMatrix3 &Rhs){
		Type Ax = Lhs.m_Rows[0].x*Rhs.m_Rows[0].x + Lhs.m_Rows[0].y*Rhs.m_Rows[1].x + Lhs.m_Rows[0].z*Rhs.m_Rows[2].x;
		Type Ay = Lhs.m_Rows[0].x*Rhs.m_Rows[0].y + Lhs.m_Rows[0].y*Rhs.m_Rows[1].y + Lhs.m_Rows[0].z*Rhs.m_Rows[2].y;
		Type Az = Lhs.m_Rows[0].x*Rhs.m_Rows[0].z + Lhs.m_Rows[0].y*Rhs.m_Rows[1].z + Lhs.m_Rows[0].z*Rhs.m_Rows[2].z;
		Type Bx = Lhs.m_Rows[1].x*Rhs.m_Rows[0].x + Lhs.m_Rows[1].y*Rhs.m_Rows[1].x + Lhs.m_Rows[1].z*Rhs.m_Rows[2].x;
		Type By = Lhs.m_Rows[1].x*Rhs.m_Rows[0].y + Lhs.m_Rows[1].y*Rhs.m_Rows[1].y + Lhs.m_Rows[1].z*Rhs.m_Rows[2].y;
		Type Bz = Lhs.m_Rows[1].x*Rhs.m_Rows[0].z + Lhs.m_Rows[1].y*Rhs.m_Rows[1].z + Lhs.m_Rows[1].z*Rhs.m_Rows[2].z;
		Type Cx = Lhs.m_Rows[2].x*Rhs.m_Rows[0].x + Lhs.m_Rows[2].y*Rhs.m_Rows[1].x + Lhs.m_Rows[2].z*Rhs.m_Rows[2].x;
		Type Cy = Lhs.m_Rows[2].x*Rhs.m_Rows[0].y + Lhs.m_Rows[2].y*Rhs.m_Rows[1].y + Lhs.m_Rows[2].z*Rhs.m_Rows[2].y;
		Type Cz = Lhs.m_Rows[2].x*Rhs.m_Rows[0].z + Lhs.m_Rows[2].y*Rhs.m_Rows[1].z + Lhs.m_Rows[2].z*Rhs.m_Rows[2].z;
		return LWMatrix3({ Ax, Ay, Az }, { Bx, By, Bz }, { Cx, Cy, Cz });
	}

	friend LWMatrix3 operator * (const LWMatrix3 &Lhs, Type Rhs){
		return LWMatrix3(Lhs.m_Rows[0] * Rhs, Lhs.m_Rows[1] * Rhs, Lhs.m_Rows[2] * Rhs);
	}

	friend LWMatrix3 operator * (Type Lhs, const LWMatrix3 &Rhs){
		return LWMatrix3(Lhs*Rhs.m_Rows[0], Lhs*Rhs.m_Rows[1], Lhs*Rhs.m_Rows[2]);
	}

	friend LWMatrix3 operator / (const LWMatrix3 &Lhs, Type Rhs){
		return LWMatrix3(Lhs.m_Rows[0] / Rhs, Lhs.m_Rows[1] / Rhs, Lhs.m_Rows[2] / Rhs);
	}

	friend LWMatrix3 operator / (Type Lhs, const LWMatrix3 &Rhs){
		return LWMatrix3(Lhs / Rhs.m_Rows[0], Lhs / Rhs.m_Rows[1], Lhs / Rhs.m_Rows[2]);
	}

	friend LWVector3<Type> operator * (const LWMatrix3 &Lhs, const LWVector3<Type> &Rhs){
		return LWVector3<Type>(Lhs.m_Rows[0].x*Rhs.x + Lhs.m_Rows[1].x*Rhs.y + Lhs.m_Rows[2].x*Rhs.z,
			Lhs.m_Rows[0].y*Rhs.x + Lhs.m_Rows[1].y*Rhs.y + Lhs.m_Rows[2].y*Rhs.y,
			Lhs.m_Rows[0].z*Rhs.x + Lhs.m_Rows[1].z*Rhs.y + Lhs.m_Rows[2].z*Rhs.z);
	}

	friend LWVector3<Type> operator * (const LWVector3<Type> &Lhs, const LWMatrix3 &Rhs){
		return LWVector3<Type>(Rhs.m_Rows[0].x*Lhs.x + Rhs.m_Rows[1].x*Lhs.y + Rhs.m_Rows[2].x*Lhs.z,
			Rhs.m_Rows[0].y*Lhs.x + Rhs.m_Rows[1].y*Lhs.y + Rhs.m_Rows[2].y*Lhs.z,
			Rhs.m_Rows[0].z*Lhs.x + Rhs.m_Rows[1].z*Lhs.y + Rhs.m_Rows[2].z*Lhs.z);
	}

	friend LWVector2<Type> operator * (const LWMatrix3 &Lhs, const LWVector2<Type> &Rhs){
		return LWVector2<Type>(Lhs.m_Rows[0].x*Rhs.x + Lhs.m_Rows[1].x*Rhs.y,
			Lhs.m_Rows[0].y*Rhs.x + Lhs.m_Rows[1].y*Rhs.y);
	}

	friend LWVector2<Type> operator * (const LWVector2<Type> &Lhs, const LWMatrix3 &Rhs){
		return LWVector2<Type>(Rhs.m_Rows[0].x*Lhs.x + Rhs.m_Rows[1].x*Lhs.y,
			Rhs.m_Rows[0].y*Lhs.x + Rhs.m_Rows[1].y*Lhs.y);
	}
	/*! \endcond */

	/*! \brief returns a matrix rotated around the x axis.
	\param Theta the angle to use.
	*/
	static LWMatrix3 RotationX(Type Theta){
		Type C = cos(Theta);
		Type S = sin(Theta);
		return LWMatrix3({ 1, 0, 0 },
		{ 0, C, -S },
		{ 0, S, C });
	}
	/*! \brief writes into Result a matrix rotated around the x axis.
	\param Theta the angle to use.
	\param Result the matrix to receive the result.
	*/
	static void RotationX(Type Theta, LWMatrix3 &Result){
		Result = RotationX(Theta);
		return;
	}


	/*! \brief returns a matrix rotated around the y axis.
	\param Theta the angle to use.
	*/
	static LWMatrix3 RotationY(Type Theta){
		Type C = cos(Theta);
		Type S = sin(Theta);
		return LWMatrix3({ C, 0, S },
		{ 0, 1, 0 },
		{ -S, 0, C });
	}
	/*! \brief writes into Result a matrix rotated around the y axis.
	\param Theta the angle to use.
	\param Result the matrix to receive the result.
	*/
	static void RotationY(Type Theta, LWMatrix3 &Result){
		Result = RotationY(Theta);
		return;
	}

	/*! \brief returns a matrix rotated around the z axis.
	\param Theta the angle to use.
	*/
	static LWMatrix3 RotationZ(Type Theta){
		Type C = cos(Theta);
		Type S = sin(Theta);
		return LWMatrix3({ C, -S, 0 },
		{ S, C, 0 },
		{ 0, 0, 1 });
	}

	/*! \brief returns a matrix rotated around the z axis.
	\param Theta the angle to use.
	\param Result the matrix to receive the result.
	*/
	static void RotationZ(Type Theta, LWMatrix3 &Result){
		Result = RotationZ(Theta);
		return;
	}

	/*! \brief returns a matrix rotated both around the x and y axis in that order.
	\param ThetaX the x-axis angle.
	\param ThetaY the y-axis angle.
	*/
	static LWMatrix3 RotationXY(Type ThetaX, Type ThetaY){
		return RotationX(ThetaX)*RotationY(ThetaY);
	}

	/*! \brief writes into result a matrix rotated around the x and y axis in that order.
	\param ThetaX the x-axis angle.
	\param ThetaY the y-axis angle.
	\param Result the matrix to receive the result.
	*/
	static void RotationXY(Type ThetaX, Type ThetaY, LWMatrix3 &Result){
		Result = RotationXY(ThetaX, ThetaY);
		return;
	}

	/*! \brief returns a matrix rotated both around the x and z axis.
	\param ThetaX the x-axis angle.
	\param ThetaZ the z-axis angle.
	*/
	static LWMatrix3 RotationXZ(Type ThetaX, Type ThetaZ){
		return RotationX(ThetaX)*RotationZ(ThetaZ);
	}

	/*! \brief writes into result a matrix rotated around the x and z axis in that order.
	\param ThetaX the x-axis angle.
	\param ThetaZ the z-axis angle.
	\param Result the matrix to receive the result.
	*/
	static void RotationXZ(Type ThetaX, Type ThetaZ, LWMatrix3 &Result){
		Result = RotationXZ(ThetaX, ThetaZ);
		return;
	}

	/*! \brief returns a matrix rotated both around the y and x axis in that order.
	\param ThetaY the y-axis angle.
	\param ThetaX the x-axis angle.
	*/
	static LWMatrix3 RotationYX(Type ThetaY, Type ThetaX){
		return RotationY(ThetaY)*RotationX(ThetaX);
	}

	/*! \brief writes into result a matrix rotated both around the y and x axis in that order.
	\param ThetaY the y-axis angle.
	\param ThetaX the z-axis angle.
	\param Result the matrix to receive the result.
	*/
	static void RotationYX(Type ThetaY, Type ThetaX, LWMatrix3 &Result){
		Result = RotationYX(ThetaY, ThetaX);
		return;
	}

	/*! \brief returns a matrix rotated both around the y and z axis in that order.
	\param ThetaY the y-axis angle.
	\param ThetaZ the z-axis angle.
	*/
	static LWMatrix3 RotationYZ(Type ThetaY, Type ThetaZ){
		return RotationY(ThetaY)*RotationZ(ThetaZ);
	}

	/*! \brief writes into result a matrix rotated both around the y and z axis in that order.
	\param ThetaY the y-axis angle.
	\param ThetaZ the z-axis angle.
	\param Result the matrix to receive the result.
	*/
	static void RotationYZ(Type ThetaY, Type ThetaZ, LWMatrix3 &Result){
		Result = RotationYZ(ThetaY, ThetaZ);
		return;
	}

	/*! \brief returns a matrix rotated both around the z and x axis in that order.
	\param ThetaZ the z-axis angle.
	\param ThetaX the x-axis angle.
	*/

	static LWMatrix3 RotationZX(Type ThetaZ, Type ThetaX){
		return RotationZ(ThetaZ)*RotationX(ThetaX);
	}

	/*! \brief writes into result a matrix rotated both around the z and x axis in that order.
	\param ThetaZ the z-axis angle.
	\param ThetaX the x-axis angle.
	\param Result the matrix to receive the result.
	*/
	static void RotationZX(Type ThetaZ, Type ThetaX, LWMatrix3 &Result){
		Result = RotationZX(ThetaZ, ThetaX);
		return;
	}

	/*! \brief returns a matrix rotated both around the z and y axis in that order.
	\param ThetaZ the z-axis angle.
	\param ThetaY the y-axis angle.
	*/
	static LWMatrix3 RotationZY(Type ThetaZ, Type ThetaY){
		return RotationZ(ThetaZ)*RotationY(ThetaY);
	}

	/*! \brief writes into result a matrix rotated both around the z and y axis in that order.
	\param ThetaZ the z-axis angle.
	\param ThetaY the y-axis angle.
	\param Result the matrix to receive the result.
	*/
	static void RotationZY(Type ThetaZ, Type ThetaY, LWMatrix3 &Result){
		Result = RotationZY(ThetaZ, ThetaY);
		return;
	}

	/*! \brief returns a matrix rotated around the x, y, and z axis in that order.
	\param ThetaX the x-axis angle.
	\param ThetaY the y-axis angle.
	\param ThetaZ the z-axis angle.
	*/
	static LWMatrix3 RotationXYZ(Type ThetaX, Type ThetaY, Type ThetaZ){
		return RotationX(ThetaX)*RotationY(ThetaY)*RotationZ(ThetaZ);
	}

	/*! \brief writes into result a matrix rotated around the x, y, and z axis in that order.
	\param ThetaX the x-axis angle.
	\param ThetaY the y-axis angle.
	\param ThetaZ the z-axis angle.
	\param Result the matrix to receive the result.
	*/
	static void RotationXYZ(Type ThetaX, Type ThetaY, Type ThetaZ, LWMatrix3 &Result){
		Result = RotationXYZ(ThetaX, ThetaY, ThetaZ);
		return;
	}

	/*! \brief returns a matrix rotated around the x, z, and y axis in that order.
	\param ThetaX the x-axis angle.
	\param ThetaZ the z-axis angle.
	\param ThetaY the y-axis angle.
	*/
	static LWMatrix3 RotationXZY(Type ThetaX, Type ThetaZ, Type ThetaY){
		return RotationX(ThetaX)*RotationZ(ThetaZ)*RotationY(ThetaY);
	}

	/*! \brief writes into result a matrix rotated around the x, z, and y axis in that order.
	\param ThetaX the x-axis angle.
	\param ThetaZ the z-axis angle.
	\param ThetaY the y-axis angle.
	\param Result the matrix to receive the result.
	*/
	static void RotationXZY(Type ThetaX, Type ThetaZ, Type ThetaY, LWMatrix3 &Result){
		Result = RotationXZY(ThetaX, ThetaZ, ThetaY);
		return;
	}

	/*! \brief returns a matrix rotated around the y, x, and z axis in that order.
	\param ThetaY the y-axis angle.
	\param ThetaX the x-axis angle.
	\param ThetaZ the z-axis angle.
	*/
	static LWMatrix3 RotationYXZ(Type ThetaY, Type ThetaX, Type ThetaZ){
		return RotationY(ThetaY)*RotationX(ThetaX)*RotationZ(ThetaZ);
	}

	/*! \brief writes into result a matrix rotated around the y, x, and z axis in that order.
	\param ThetaY the y-axis angle.
	\param ThetaX the x-axis angle.
	\param ThetaZ the z-axis angle.
	\param Result the matrix to receive the result.
	*/
	static void RotationYXZ(Type ThetaY, Type ThetaX, Type ThetaZ, LWMatrix3 &Result){
		Result = RotationYXZ(ThetaY, ThetaX, ThetaZ);
		return;
	}

	/*! \brief returns a matrix rotated around the y, z, and x axis in that order.
	\param ThetaY the y-axis angle.
	\param ThetaZ the z-axis angle.
	\param ThetaX the x-axis angle.
	*/
	static LWMatrix3 RotationYZX(Type ThetaY, Type ThetaZ, Type ThetaX){
		return RotationY(ThetaY)*RotationZ(ThetaZ)*RotationX(ThetaX);
	}

	/*! \brief writes into result a matrix rotated around the y, z, and x axis in that order.
	\param ThetaY the y-axis angle.
	\param ThetaZ the z-axis angle.
	\param ThetaX the x-axis angle.
	\param Result the matrix to receive the result.
	*/
	static void RotationYZX(Type ThetaY, Type ThetaZ, Type ThetaX, LWMatrix3 &Result){
		Result = RotationYZX(ThetaY, ThetaZ, ThetaX);
		return;
	}

	/*! \brief returns a matrix rotated around the z, x, and y axis in that order.
	\param ThetaZ the z-axis angle.
	\param ThetaX the x-axis angle.
	\param ThetaY the y-axis angle.
	*/
	static LWMatrix3 RotationZXY(Type ThetaZ, Type ThetaX, Type ThetaY){
		return RotationZ(ThetaZ)*RotationX(ThetaX)*RotationY(ThetaY);
	}

	/*! \brief writes into result a matrix rotated around the z, x, and y axis in that order.
	\param ThetaZ the z-axis angle.
	\param ThetaX the x-axis angle.
	\param ThetaY the y-axis angle.
	\param Result the matrix to receive the result.
	*/
	static void RotationZXY(Type ThetaZ, Type ThetaX, Type ThetaY, LWMatrix3 &Result){
		Result = RotationZXY(ThetaZ, ThetaX, ThetaY);
		return;
	}

	/*! \brief returns a matrix rotated around the z, y, and x axis in that order.
	\param ThetaZ the z-axis angle.
	\param ThetaY the y-axis angle.
	\param ThetaX the x-axis angle.
	*/
	static LWMatrix3 RotationZYX(Type ThetaZ, Type ThetaY, Type ThetaX){
		return RotationZ(ThetaZ)*RotationY(ThetaY)*RotationX(ThetaX);
	}

	/*! \brief writes into result a matrix rotated around the z, y, and x axis in that order.
	\param ThetaZ the z-axis angle.
	\param ThetaY the y-axis angle.
	\param ThetaX the x-axis angle.
	\param Result the matrix to receive the result.
	*/
	static void RotationZYX(Type ThetaZ, Type ThetaY, Type ThetaX, LWMatrix3 &Result){
		Result = RotationZYX(ThetaZ, ThetaY, ThetaX);
	}

	/*!< \brief constructs a rotation matrix from the directional vector which is relative to the supplied up vector.
	\param Direction the normalized directional vector.
	\param Up the normalized up vector.
	*/
	static LWMatrix3 Rotation(const LWVector3<Type> &Direction, const LWVector3<Type> &Up) {
		LWVector3<Type> xAxis = Up.Cross(Direction);
		LWVector3<Type> yAxis = Direction.Cross(xAxis);
		return LWMatrix3<Type>(
		{ xAxis.x, xAxis.y, xAxis.z },
		{ yAxis.x, yAxis.y, yAxis.z },
		{ Direction.x, Direction.y, Direction.z});
	}

	/*!< \brief constructs a rotation matrix from the directional vector which is relative to the supplied up vector. stores the new matrix into result.
	\param Direction the normalized directional vector.
	\param Up the normalized up vector.
	\param Result the matrix to receive the result.
	*/
	static void Rotation(const LWVector3<Type> &Direction, const LWVector3<Type> &Up, LWMatrix4<Type> &Result) {
		Result = Rotation(Direction, Up);
		return;
	}

	/*! \brief returns a matrix translated along the x, and y axis.
	\param x the x axis to translate along.
	\param y the y axis to translate along.
	*/
	static LWMatrix3 Translation(Type x, Type y){
		return LWMatrix3({ 1, 0, 0 }, { 0, 1, 0 }, { x, y, 1 });
	}

	/*! \brief writes into result a matrix translated along the x, and y axis.
	\param x the x axis to translate along.
	\param y the y axis to translate along.
	\param Result the matrix to receive the result.
	*/
	static void Translation(Type x, Type y, LWMatrix3 &Result){
		Result = Translation(x, y);
		return;
	}
	
	/*!< \brief constructs a 3x3 rotational matrix from the specified quaternion. */
	LWMatrix3(const LWQuaternion<Type> &Q) {
		Type xx = Q.x*Q.x;
		Type xy = Q.x*Q.y;
		Type xz = Q.x*Q.z;
		Type xw = Q.x*Q.w;
		Type yy = Q.y*Q.y;
		Type yz = Q.y*Q.z;
		Type yw = Q.y*Q.w;

		Type zz = Q.z*Q.z;
		Type zw = Q.z*Q.w;

		m_Rows[0] = LWVector3<Type>((Type)(1 - 2 * (yy + zz)), (Type)(2 * (xy - zw)), (Type)(2 * (xz + yw)));
		m_Rows[1] = LWVector3<Type>((Type)(2 * (xy + zw)), (Type)(1 - 2 * (xx + zz)), (Type)2 * (yz - xw));
		m_Rows[2] = LWVector3<Type>((Type)(2 * (xz - yw)), (Type)(2 * (yz + xw)), (Type)(1 - 2 * (xx + yy)));
		
	}

	/*! \brief construs a 3x3 matrix where each row is scaled by their respective scales.*/
	LWMatrix3(Type xScale = 1, Type yScale = 1, Type zScale = 1){
		m_Rows[0] = { xScale, 0, 0 };
		m_Rows[1] = { 0, yScale, 0 };
		m_Rows[2] = { 0, 0, zScale };
	}
	
	/*! \brief constructs a 3x3 matrix where the matrix is set by the application. */
	LWMatrix3(const LWVector3<Type> &RowA, const LWVector3<Type> &RowB, const LWVector3<Type> &RowC){
		m_Rows[0] = RowA;
		m_Rows[1] = RowB;
		m_Rows[2] = RowC;
	}
};

/*! \brief LWMatrix2 is a 2x2 row-major matrix object.
*/
template<class Type>
struct LWMatrix2{
	LWVector2<Type> m_Rows[2]; /*!< The 4x4 matrix in row-major order. */


	/*!< \brief returns the specified column of the matrix. */
	LWVector2<Type> Column(uint32_t Index) {
		Type *Arry = &m_Rows[0].x;
		return LWVector2<Type>(Arry[Index], Arry[Index + 2]);
	};

	/*! \brief returns the specified row of the matrix(this function exists for parody to LWSMatrix4's row function.) */
	LWVector2<Type> Row(uint32_t Index) {
		return m_Rows[Index];
	}

	/*! \brief set's a value of the matrix at row*2+Column position, this function exists for parody to LWSMatrix4's sRC function.) */
	LWMatrix2 &sRC(uint32_t Row, uint32_t Column, Type Value) {
		Type *Arry = &m_Rows[0].x;
		Arry[Row * 2 + Column] = Value;
		return *this;
	}


	/*! \brief Returns an copied inverse of this matrix. */
	LWMatrix2 Inverse(void) const{
		Type D = Determinant();
		if (abs(D) <= std::numeric_limits<Type>::epsilon()) return LWMatrix2();
		return LWMatrix2(LWVector2<Type>(m_Rows[1].y, m_Rows[0].y)*D, LWVector2<Type>(m_Rows[1].x, m_Rows[0].x)*D );
	}

	/*! \brief writes into Result the inverse of the matrix. */
	void Inverse(LWMatrix2 &Result) const{
		Result = Inverse();
		return;
	}

	/*! \brief returns the transpose of the this matrix. */
	LWMatrix2 Transpose(void) const{
		return LWMatrix2({ m_Rows[0].x, m_Rows[1].x }, { m_Rows[0].y, m_Rows[1].y });
	}

	/*! \brief writes into Result the transpose of this matrix. */
	void Transpose(LWMatrix2 &Result) const{
		Result = Transpose();
	}

	/*! \brief calculates the determinant of this matrix. */
	Type Determinant(void) const{
		return m_Rows[0].x*m_Rows[1].y - m_Rows[0].y*m_Rows[1].x;
	}

	/*! \cond */
	LWMatrix2 &operator = (const LWMatrix2 &Rhs){
		m_Rows[0] = Rhs.m_Rows[0];
		m_Rows[1] = Rhs.m_Rows[1];
		return *this;
	}

	LWMatrix2 &operator+= (const LWMatrix2 &Rhs){
		m_Rows[0] += Rhs.m_Rows[0];
		m_Rows[1] += Rhs.m_Rows[1];
		return *this;
	}

	LWMatrix2 &operator-= (const LWMatrix2 &Rhs){
		m_Rows[0] -= Rhs.m_Rows[0];
		m_Rows[1] -= Rhs.m_Rows[1];
		return *this;
	}

	LWMatrix2 &operator*= (const LWMatrix2 &Rhs){
		Type Ax = m_Rows[0].x*Rhs.m_Rows[0].x + m_Rows[0].y*Rhs.m_Rows[1].x;
		Type Ay = m_Rows[0].x*Rhs.m_Rows[0].y + m_Rows[0].y*Rhs.m_Rows[1].y;
		Type Bx = m_Rows[1].x*Rhs.m_Rows[0].x + m_Rows[1].y*Rhs.m_Rows[1].x;
		Type By = m_Rows[1].x*Rhs.m_Rows[0].y + m_Rows[1].y*Rhs.m_Rows[1].y;
		m_Rows[0] = { Ax, Ay };
		m_Rows[1] = { Bx, By };
		return *this;
	}

	LWMatrix2 &operator *=(Type Rhs){
		m_Rows[0] *= Rhs;
		m_Rows[1] *= Rhs;
		return *this;
	}

	LWMatrix2 &operator /=(Type Rhs){
		m_Rows[0] /= Rhs;
		m_Rows[1] /= Rhs;
		return *this;
	}

	bool operator == (const LWMatrix2 &Rhs) const{
		return (m_Rows[0] == Rhs.m_Rows[0] && m_Rows[1] == Rhs.m_Rows[1]);
	}

	bool operator != (const LWMatrix2 &Rhs) const{
		return !(*this == Rhs);
	}

	friend std::ostream &operator<<(std::ostream &o, const LWMatrix2<Type> &M) {
		o << "Matrix2:" << std::endl;
		o << M.m_Rows[0] << std::endl;
		o << M.m_Rows[1];
		return o;
	}

	friend LWMatrix2 operator + (const LWMatrix2 &Lhs, const LWMatrix2 &Rhs){
		return LWMatrix2({ Lhs.m_Rows[0] + Rhs.m_Rows[0] },
						 { Lhs.m_Rows[1] + Rhs.m_Rows[1] });
	}

	friend LWMatrix2 operator - (const LWMatrix2 &Lhs, const LWMatrix2 &Rhs){
		return LWMatrix2({ Lhs.m_Rows[0] - Rhs.m_Rows[0] },
						 { Lhs.m_Rows[1] - Rhs.m_Rows[1] });
	}

	friend LWMatrix2 operator * (const LWMatrix2 &Lhs, const LWMatrix2 &Rhs){
		Type Ax = Lhs.m_Rows[0].x*Rhs.m_Rows[0].x + Lhs.m_Rows[0].y*Rhs.m_Rows[1].x;
		Type Ay = Lhs.m_Rows[0].x*Rhs.m_Rows[0].y + Lhs.m_Rows[0].y*Rhs.m_Rows[1].y;
		Type Bx = Lhs.m_Rows[1].x*Rhs.m_Rows[0].x + Lhs.m_Rows[1].y*Rhs.m_Rows[1].x;
		Type By = Lhs.m_Rows[1].x*Rhs.m_Rows[0].y + Lhs.m_Rows[1].y*Rhs.m_Rows[1].y;
		return LWMatrix2({ Ax, Ay }, { Bx, By });
	}

	friend LWMatrix2 operator * (const LWMatrix2 &Lhs, Type Rhs){
		return LWMatrix2({ Lhs.m_Rows[0] * Rhs }, { Lhs.m_Rows[1] * Rhs });
	}

	friend LWMatrix2 operator * (Type Lhs, const LWMatrix2 &Rhs){
		return LWMatrix2({ Lhs*Rhs.m_Rows[0] }, { Lhs*Rhs.m_Rows[1] });
	}

	friend LWMatrix2 operator / (const LWMatrix2 &Lhs, Type Rhs){
		return LWMatrix2({ Lhs.m_Rows[0] / Rhs }, { Lhs.m_Rows[1] / Rhs });
	}

	friend LWMatrix2 operator / (Type Lhs, const LWMatrix2 &Rhs){
		return LWMatrix2({ Lhs / Rhs.m_Rows[0] }, { Lhs / Rhs.m_Rows[1] });
	}

	friend LWVector2<Type> operator * (const LWMatrix2 &Lhs, const LWVector2<Type> &Rhs){
		return LWVector2<Type>(Lhs.m_Rows[0].x*Rhs.x + Lhs.m_Rows[1].x*Rhs.y, Lhs.m_Rows[0].y*Rhs.x + Lhs.m_Rows[1].y*Rhs.y);
	}

	friend LWVector2<Type> operator * (const LWVector2<Type> &Lhs, const LWMatrix2 &Rhs){
		return LWVector2<Type>(Rhs.m_Rows[0].x*Lhs.x + Rhs.m_Rows[1].x*Lhs.y, Rhs.m_Rows[0].y*Lhs.x + Rhs.m_Rows[1].y*Lhs.y);
	}
	/*! \endcond */

	/*! \brief returns a rotated matrix.
		\param Theta the angle to use.
	*/
	static LWMatrix2 Rotation(Type Theta){
		Type C = cos(Theta);
		Type S = sin(Theta);
		return LWMatrix2({ C, -S }, { S, C });
	}

	/*! \brief writes into Result a rotated matrix.
		\param Theta the angle to use.
		\param Result the matrix to receive the result.
	*/
	static void Rotation(Type Theta, LWMatrix2 &Result){
		Result = Rotation(Theta);
		return;
	}

	/*! \brief constructs a 2x2 matrix where each row is scaled by their respective scales.*/
	LWMatrix2(Type xScale = 1, Type yScale = 1){
		m_Rows[0] = { xScale, 0 };
		m_Rows[1] = { 0, yScale };
	}

	/*! \brief constructs a 2x2 matrix where the matrix is set by the application. */
	LWMatrix2(const LWVector2<Type> &RowA, const LWVector2<Type> &RowB){
		m_Rows[0] = RowA;
		m_Rows[1] = RowB;
	}
};
/*! @} */
#endif
