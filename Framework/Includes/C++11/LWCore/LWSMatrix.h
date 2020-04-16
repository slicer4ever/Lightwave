#ifndef LWSMATRIX_H
#define LWSMATRIX_H
#include <LWCore/LWTypes.h>
#include <LWCore/LWMatrix.h>
#include <LWCore/LWSVector.h>
#include <LWCore/LWSQuaternion.h>
#include <ostream>

/*!< \brief an accelerated simd matrix4 class, non-implemented or disabled sse functions default to a generic class. */
template<class Type>
struct LWSMatrix4 {
	LWSVector4<Type> m_Rows[4];

	LWMatrix4<Type> AsMat4(void) const {
		return LWMatrix4<Type>(m_Rows[0].AsVec4(), m_Rows[1].AsVec4(), m_Rows[2].AsVec4(), m_Rows[3].AsVec4());
	}

	/*! \brief Returns an copied inverse of this matrix. */
	/*
	LWSMatrix4 Inverse(void) const {
		Type D = Determinant();
		
		if (abs(D) <= std::numeric_limits<Type>::epsilon()) return LWSMatrix4();
		D = 1.0f / D;




		Type Ax = m_Rows[1].z*m_Rows[2].w*m_Rows[3].y - m_Rows[1].w*m_Rows[2].z*m_Rows[3].y + m_Rows[1].w*m_Rows[2].y*m_Rows[3].z - m_Rows[1].y*m_Rows[2].w*m_Rows[3].z - m_Rows[1].z*m_Rows[2].y*m_Rows[3].w + m_Rows[1].y*m_Rows[2].z*m_Rows[3].w;
		Type Ay = m_Rows[0].w*m_Rows[2].z*m_Rows[3].y - m_Rows[0].z*m_Rows[2].w*m_Rows[3].y - m_Rows[0].w*m_Rows[2].y*m_Rows[3].z + m_Rows[0].y*m_Rows[2].w*m_Rows[3].z + m_Rows[0].z*m_Rows[2].y*m_Rows[3].w - m_Rows[0].y*m_Rows[2].z*m_Rows[3].w;
		Type Az = m_Rows[0].z*m_Rows[1].w*m_Rows[3].y - m_Rows[0].w*m_Rows[1].z*m_Rows[3].y + m_Rows[0].w*m_Rows[1].y*m_Rows[3].z - m_Rows[0].y*m_Rows[1].w*m_Rows[3].z - m_Rows[0].z*m_Rows[1].y*m_Rows[3].w + m_Rows[0].y*m_Rows[1].z*m_Rows[3].w;
		Type Aw = m_Rows[0].w*m_Rows[1].z*m_Rows[2].y - m_Rows[0].z*m_Rows[1].w*m_Rows[2].y - m_Rows[0].w*m_Rows[1].y*m_Rows[2].z + m_Rows[0].y*m_Rows[1].w*m_Rows[2].z + m_Rows[0].z*m_Rows[1].y*m_Rows[2].w - m_Rows[0].y*m_Rows[1].z*m_Rows[2].w;
		Type Bx = m_Rows[1].w*m_Rows[2].z*m_Rows[3].x - m_Rows[1].z*m_Rows[2].w*m_Rows[3].x - m_Rows[1].w*m_Rows[2].x*m_Rows[3].z + m_Rows[1].x*m_Rows[2].w*m_Rows[3].z + m_Rows[1].z*m_Rows[2].x*m_Rows[3].w - m_Rows[1].x*m_Rows[2].z*m_Rows[3].w;
		Type By = m_Rows[0].z*m_Rows[2].w*m_Rows[3].x - m_Rows[0].w*m_Rows[2].z*m_Rows[3].x + m_Rows[0].w*m_Rows[2].x*m_Rows[3].z - m_Rows[0].x*m_Rows[2].w*m_Rows[3].z - m_Rows[0].z*m_Rows[2].x*m_Rows[3].w + m_Rows[0].x*m_Rows[2].z*m_Rows[3].w;
		Type Bz = m_Rows[0].w*m_Rows[1].z*m_Rows[3].x - m_Rows[0].z*m_Rows[1].w*m_Rows[3].x - m_Rows[0].w*m_Rows[1].x*m_Rows[3].z + m_Rows[0].x*m_Rows[1].w*m_Rows[3].z + m_Rows[0].z*m_Rows[1].x*m_Rows[3].w - m_Rows[0].x*m_Rows[1].z*m_Rows[3].w;
		Type Bw = m_Rows[0].z*m_Rows[1].w*m_Rows[2].x - m_Rows[0].w*m_Rows[1].z*m_Rows[2].x + m_Rows[0].w*m_Rows[1].x*m_Rows[2].z - m_Rows[0].x*m_Rows[1].w*m_Rows[2].z - m_Rows[0].z*m_Rows[1].x*m_Rows[2].w + m_Rows[0].x*m_Rows[1].z*m_Rows[2].w;
		Type Cx = m_Rows[1].y*m_Rows[2].w*m_Rows[3].x - m_Rows[1].w*m_Rows[2].y*m_Rows[3].x + m_Rows[1].w*m_Rows[2].x*m_Rows[3].y - m_Rows[1].x*m_Rows[2].w*m_Rows[3].y - m_Rows[1].y*m_Rows[2].x*m_Rows[3].w + m_Rows[1].x*m_Rows[2].y*m_Rows[3].w;
		Type Cy = m_Rows[0].w*m_Rows[2].y*m_Rows[3].x - m_Rows[0].y*m_Rows[2].w*m_Rows[3].x - m_Rows[0].w*m_Rows[2].x*m_Rows[3].y + m_Rows[0].x*m_Rows[2].w*m_Rows[3].y + m_Rows[0].y*m_Rows[2].x*m_Rows[3].w - m_Rows[0].x*m_Rows[2].y*m_Rows[3].w;
		Type Cz = m_Rows[0].y*m_Rows[1].w*m_Rows[3].x - m_Rows[0].w*m_Rows[1].y*m_Rows[3].x + m_Rows[0].w*m_Rows[1].x*m_Rows[3].y - m_Rows[0].x*m_Rows[1].w*m_Rows[3].y - m_Rows[0].y*m_Rows[1].x*m_Rows[3].w + m_Rows[0].x*m_Rows[1].y*m_Rows[3].w;
		Type Cw = m_Rows[0].w*m_Rows[1].y*m_Rows[2].x - m_Rows[0].y*m_Rows[1].w*m_Rows[2].x - m_Rows[0].w*m_Rows[1].x*m_Rows[2].y + m_Rows[0].x*m_Rows[1].w*m_Rows[2].y + m_Rows[0].y*m_Rows[1].x*m_Rows[2].w - m_Rows[0].x*m_Rows[1].y*m_Rows[2].w;
		Type Dx = m_Rows[1].z*m_Rows[2].y*m_Rows[3].x - m_Rows[1].y*m_Rows[2].z*m_Rows[3].x - m_Rows[1].z*m_Rows[2].x*m_Rows[3].y + m_Rows[1].x*m_Rows[2].z*m_Rows[3].y + m_Rows[1].y*m_Rows[2].x*m_Rows[3].z - m_Rows[1].x*m_Rows[2].y*m_Rows[3].z;
		Type Dy = m_Rows[0].y*m_Rows[2].z*m_Rows[3].x - m_Rows[0].z*m_Rows[2].y*m_Rows[3].x + m_Rows[0].z*m_Rows[2].x*m_Rows[3].y - m_Rows[0].x*m_Rows[2].z*m_Rows[3].y - m_Rows[0].y*m_Rows[2].x*m_Rows[3].z + m_Rows[0].x*m_Rows[2].y*m_Rows[3].z;
		Type Dz = m_Rows[0].z*m_Rows[1].y*m_Rows[3].x - m_Rows[0].y*m_Rows[1].z*m_Rows[3].x - m_Rows[0].z*m_Rows[1].x*m_Rows[3].y + m_Rows[0].x*m_Rows[1].z*m_Rows[3].y + m_Rows[0].y*m_Rows[1].x*m_Rows[3].z - m_Rows[0].x*m_Rows[1].y*m_Rows[3].z;
		Type Dw = m_Rows[0].y*m_Rows[1].z*m_Rows[2].x - m_Rows[0].z*m_Rows[1].y*m_Rows[2].x + m_Rows[0].z*m_Rows[1].x*m_Rows[2].y - m_Rows[0].x*m_Rows[1].z*m_Rows[2].y - m_Rows[0].y*m_Rows[1].x*m_Rows[2].z + m_Rows[0].x*m_Rows[1].y*m_Rows[2].z;
		return LWSMatrix4(LWVector4<Type>(Ax, Ay, Az, Aw), LWVector4<Type>(Bx, By, Bz, Bw), LWVector4<Type>(Cx, Cy, Cz, Cw), LWVector4<Type>(Dx, Dy, Dz, Dw))*D;
	}*/

	/*!< \brief returns the specified column of the matrix. */
	LWSVector4<Type> Column(uint32_t Index) {
		LWMatrix4 M = AsMat4();
		Type *Arry = &M.m_Rows[0].x;
		return LWSVector4<Type>(Arry[Index], Arry[Index + 4], Arry[Index + 8], Arry[Index + 12]);
	};

	/*! \brief returns the transpose of the this matrix. */
	LWSMatrix4 Transpose(void) const {
		LWMatrix4<Type> M = AsMat4().Transpose();
		return LWSMatrix4(M.m_Rows[0], M.m_Rows[1], M.m_Rows[2], M.m_Rows[3]);
	}

	/*!< \brief returns the upper left 3x3 matrix transposed only. */
	LWSMatrix4 Transpose3x3(void) const {
		LWMatrix4 M = AsMat4().Transpose3x3();
		return LWSMatrix4(M.m_Rows[0], M.m_Rows[1], M.m_Rows[2], M.m_Rows[3]);
	}

	/*!< \brief returns the upper left 2x2 matrix transposed only. */
	LWSMatrix4 Transpose2x2(void) const {
		LWMatrix4 M = AsMat4().Transpose2x2();
		return LWSMatrix4(M.m_Rows[0], M.m_Rows[1], M.m_Rows[2], M.m_Rows[3]);
	}

	/*! \brief calculates the determinant of this matrix. */
	/*
	Type Determinant(void) const {
		
		LWSVector4<Type> A = 

		return
			m_Rows[0].w*m_Rows[1].z*m_Rows[2].y*m_Rows[3].x - m_Rows[0].z*m_Rows[1].w*m_Rows[2].y*m_Rows[3].x -
			m_Rows[0].w*m_Rows[1].y*m_Rows[2].z*m_Rows[3].x + m_Rows[0].y*m_Rows[1].w*m_Rows[2].z*m_Rows[3].x +
			m_Rows[0].z*m_Rows[1].y*m_Rows[2].w*m_Rows[3].x - m_Rows[0].y*m_Rows[1].z*m_Rows[2].w*m_Rows[3].x -
			m_Rows[0].w*m_Rows[1].z*m_Rows[2].x*m_Rows[3].y + m_Rows[0].z*m_Rows[1].w*m_Rows[2].x*m_Rows[3].y +
			m_Rows[0].w*m_Rows[1].x*m_Rows[2].z*m_Rows[3].y - m_Rows[0].x*m_Rows[1].w*m_Rows[2].z*m_Rows[3].y -
			m_Rows[0].z*m_Rows[1].x*m_Rows[2].w*m_Rows[3].y + m_Rows[0].x*m_Rows[1].z*m_Rows[2].w*m_Rows[3].y +
			m_Rows[0].w*m_Rows[1].y*m_Rows[2].x*m_Rows[3].z - m_Rows[0].y*m_Rows[1].w*m_Rows[2].x*m_Rows[3].z -
			m_Rows[0].w*m_Rows[1].x*m_Rows[2].y*m_Rows[3].z + m_Rows[0].x*m_Rows[1].w*m_Rows[2].y*m_Rows[3].z +
			m_Rows[0].y*m_Rows[1].x*m_Rows[2].w*m_Rows[3].z - m_Rows[0].x*m_Rows[1].y*m_Rows[2].w*m_Rows[3].z -
			m_Rows[0].z*m_Rows[1].y*m_Rows[2].x*m_Rows[3].w + m_Rows[0].y*m_Rows[1].z*m_Rows[2].x*m_Rows[3].w +
			m_Rows[0].z*m_Rows[1].x*m_Rows[2].y*m_Rows[3].w - m_Rows[0].x*m_Rows[1].z*m_Rows[2].y*m_Rows[3].w -
			m_Rows[0].y*m_Rows[1].x*m_Rows[2].z*m_Rows[3].w + m_Rows[0].x*m_Rows[1].y*m_Rows[2].z*m_Rows[3].w;
	}*/

	/*! \cond */
	LWSMatrix4 &operator = (const LWSMatrix4 &Rhs) {
		m_Rows[0] = Rhs.m_Rows[0];
		m_Rows[1] = Rhs.m_Rows[1];
		m_Rows[2] = Rhs.m_Rows[2];
		m_Rows[3] = Rhs.m_Rows[3];
		return *this;
	}

	LWSMatrix4 &operator+= (const LWSMatrix4 &Rhs) {
		m_Rows[0] += Rhs.m_Rows[0];
		m_Rows[1] += Rhs.m_Rows[1];
		m_Rows[2] += Rhs.m_Rows[2];
		m_Rows[3] += Rhs.m_Rows[3];
		return *this;
	}

	LWSMatrix4 &operator-= (const LWSMatrix4 &Rhs) {
		m_Rows[0] -= Rhs.m_Rows[0];
		m_Rows[1] -= Rhs.m_Rows[1];
		m_Rows[2] -= Rhs.m_Rows[2];
		m_Rows[3] -= Rhs.m_Rows[3];
		return *this;
	}

	LWSMatrix4 &operator*= (const LWSMatrix4 &Rhs) {
		LWSMatrix4 RhsT = Rhs.Transpose();

		LWSVector4<Type> Ax = m_Rows[0].xxxx();
		LWSVector4<Type> Ay = m_Rows[1].yyyy();
		LWSVector4<Type> Az = m_Rows[2].zzzz();
		LWSVector4<Type> Aw = m_Rows[3].wwww();
		LWSVector4<Type> A = Ax * RhsT.m_Rows[0] + Ay * RhsT.m_Rows[1] + Az*RhsT.m_Rows[2] + Aw*RhsT.m_Rows[3];

		LWSVector4<Type> Bx = m_Rows[1].xxxx();
		LWSVector4<Type> By = m_Rows[1].yyyy();
		LWSVector4<Type> Bz = m_Rows[1].zzzz();
		LWSVector4<Type> Bw = m_Rows[2].wwww();
		LWSVector4<Type> B = Bx * RhsT.m_Rows[0] + By * RhsT.m_Rows[1] + Az * RhsT.m_Rows[2] + Aw * RhsT.m_Rows[3];

		LWSVector4<Type> Cx = m_Rows[2].xxxx();
		LWSVector4<Type> Cy = m_Rows[2].yyyy();
		LWSVector4<Type> Cz = m_Rows[2].zzzz();
		LWSVector4<Type> Cw = m_Rows[2].wwww();
		LWSVector4<Type> C = Cx * RhsT.m_Rows[0] + Cy * RhsT.m_Rows[1] + Cz * RhsT.m_Rows[2] + Cw * RhsT.m_Rows[3];

		LWSVector4<Type> Dx = m_Rows[3].xxxx();
		LWSVector4<Type> Dy = m_Rows[3].yyyy();
		LWSVector4<Type> Dz = m_Rows[3].zzzz();
		LWSVector4<Type> Dw = m_Rows[3].wwww();
		LWSVector4<Type> D = Dx * RhsT.m_Rows[0] + Dy * RhsT.m_Rows[1] + Dz * RhsT.m_Rows[2] + Dw * RhsT.m_Rows[3];
		
		m_Rows[0] = A;
		m_Rows[1] = B;
		m_Rows[2] = C;
		m_Rows[3] = D;
		return *this;
	}

	LWSMatrix4 &operator *=(Type Rhs) {
		m_Rows[0] *= Rhs;
		m_Rows[1] *= Rhs;
		m_Rows[2] *= Rhs;
		m_Rows[3] *= Rhs;
		return *this;
	}

	LWSMatrix4 &operator /=(Type Rhs) {
		m_Rows[0] /= Rhs;
		m_Rows[1] /= Rhs;
		m_Rows[2] /= Rhs;
		m_Rows[3] /= Rhs;
		return *this;
	}

	bool operator == (const LWSMatrix4 &Rhs) const {
		return m_Rows[0] == Rhs.m_Rows[0] && m_Rows[1] == Rhs.m_Rows[1] && m_Rows[2] == Rhs.m_Rows[2] && m_Rows[3] == Rhs.m_Rows[3];
	}

	bool operator != (const LWSMatrix4 &Rhs) const {
		return !(*this == Rhs);
	}

	friend std::ostream &operator<<(std::ostream &o, const LWSMatrix4<Type> &M) {
		o << "Matrix4:" << std::endl;
		o << M.m_Rows[0] << std::endl;
		o << M.m_Rows[1] << std::endl;
		o << M.m_Rows[2] << std::endl;
		o << M.m_Rows[3];
		return o;
	}

	friend LWSMatrix4 operator + (const LWSMatrix4 &Lhs, const LWSMatrix4 &Rhs) {
		return LWSMatrix4(Lhs.m_Rows[0] + Rhs.m_Rows[0], Lhs.m_Rows[1] + Rhs.m_Rows[1], Lhs.m_Rows[2] + Rhs.m_Rows[2], Lhs.m_Rows[3] + Rhs.m_Rows[3]);
	}

	friend LWSMatrix4 operator - (const LWSMatrix4 &Lhs, const LWSMatrix4 &Rhs) {
		return LWSMatrix4(Lhs.m_Rows[0] - Rhs.m_Rows[0], Lhs.m_Rows[1] - Rhs.m_Rows[1], Lhs.m_Rows[2] - Rhs.m_Rows[2], Lhs.m_Rows[3] - Rhs.m_Rows[3]);
	}

	friend LWSMatrix4 operator * (const LWSMatrix4 &Lhs, const LWSMatrix4 &Rhs) {
		LWSMatrix4 RhsT = Rhs.Transpose();

		LWSVector4<Type> Ax = Lhs.m_Rows[0].xxxx();
		LWSVector4<Type> Ay = Lhs.m_Rows[0].yyyy();
		LWSVector4<Type> Az = Lhs.m_Rows[0].zzzz();
		LWSVector4<Type> Aw = Lhs.m_Rows[0].wwww();
		LWSVector4<Type> A = Ax * RhsT.m_Rows[0] + Ay * RhsT.m_Rows[1] + Az * RhsT.m_Rows[2] + Aw * RhsT.m_Rows[3];

		LWSVector4<Type> Bx = Lhs.m_Rows[1].xxxx();
		LWSVector4<Type> By = Lhs.m_Rows[1].yyyy();
		LWSVector4<Type> Bz = Lhs.m_Rows[1].zzzz();
		LWSVector4<Type> Bw = Lhs.m_Rows[1].wwww();
		LWSVector4<Type> B = Bx * RhsT.m_Rows[0] + By * RhsT.m_Rows[1] + Az * RhsT.m_Rows[2] + Aw * RhsT.m_Rows[3];

		LWSVector4<Type> Cx = Lhs.m_Rows[2].xxxx();
		LWSVector4<Type> Cy = Lhs.m_Rows[2].yyyy();
		LWSVector4<Type> Cz = Lhs.m_Rows[2].zzzz();
		LWSVector4<Type> Cw = Lhs.m_Rows[2].wwww();
		LWSVector4<Type> C = Cx * RhsT.m_Rows[0] + Cy * RhsT.m_Rows[1] + Cz * RhsT.m_Rows[2] + Cw * RhsT.m_Rows[3];

		LWSVector4<Type> Dx = Lhs.m_Rows[3].xxxx();
		LWSVector4<Type> Dy = Lhs.m_Rows[3].yyyy();
		LWSVector4<Type> Dz = Lhs.m_Rows[3].zzzz();
		LWSVector4<Type> Dw = Lhs.m_Rows[3].wwww();
		LWSVector4<Type> D = Dx * RhsT.m_Rows[0] + Dy * RhsT.m_Rows[1] + Dz * RhsT.m_Rows[2] + Dw * RhsT.m_Rows[3];
		return { A, B, C, D };
	}

	friend LWSMatrix4 operator * (const LWSMatrix4 &Lhs, Type Rhs) {
		return LWSMatrix4(Lhs.m_Rows[0] * Rhs, Lhs.m_Rows[1] * Rhs, Lhs.m_Rows[2] * Rhs, Lhs.m_Rows[3] * Rhs);
	}

	friend LWSMatrix4 operator * (Type Lhs, const LWSMatrix4 &Rhs) {
		return LWSMatrix4(Lhs*Rhs.m_Rows[0], Lhs*Rhs.m_Rows[1], Lhs*Rhs.m_Rows[2], Lhs*Rhs.m_Rows[3]);
	}

	friend LWSMatrix4 operator / (const LWSMatrix4 &Lhs, Type Rhs) {
		return LWSMatrix4(Lhs.m_Rows[0] / Rhs, Lhs.m_Rows[1] / Rhs, Lhs.m_Rows[2] / Rhs, Lhs.m_Rows[3] / Rhs);
	}

	friend LWSMatrix4 operator / (Type Lhs, const LWSMatrix4 &Rhs) {
		return LWSMatrix4(Lhs / Rhs.m_Rows[0], Lhs / Rhs.m_Rows[1], Lhs / Rhs.m_Rows[2], Lhs / Rhs.m_Rows[3]);
	}

	friend LWSVector4<Type> operator * (const LWSMatrix4 &Lhs, const LWSVector4<Type> &Rhs) {
		LWSVector4<Type> Rx = Rhs.xxxx();
		LWSVector4<Type> Ry = Rhs.yyyy();
		LWSVector4<Type> Rz = Rhs.zzzz();
		LWSVector4<Type> Rw = Rhs.wwww();
		return Lhs.m_Rows[0] * Rx + Lhs.m_Rows[1] * Ry + Lhs.m_Rows[2] * Rz + Lhs.m_Rows[3] * Rw;
	}

	friend LWSVector4<Type> operator * (const LWSVector4<Type> &Lhs, const LWSMatrix4 &Rhs) {
		LWSMatrix4 RhsT = Rhs.Transpose();
		LWSVector4<Type> Lx = Lhs.xxxx();
		LWSVector4<Type> Ly = Lhs.yyyy();
		LWSVector4<Type> Lz = Lhs.zzzz();
		LWSVector4<Type> Lw = Lhs.wwww();
		return Lx * RhsT.m_Rows[0] + Ly * RhsT.m_Rows[1] + Lz * RhsT.m_Rows[2] + Lw * RhsT.m_Rows[3];
	}
	/*! \endcond */

	/*! \brief returns a matrix rotated around the x axis.
		\param Theta the angle to use.
		*/
	static LWSMatrix4 RotationX(Type Theta) {
		Type S = sin(Theta);
		Type C = cos(Theta);
		return LWSMatrix4({ 1, 0, 0, 0 },
			{ 0, C, -S, 0 },
			{ 0, S, C, 0 },
			{ 0, 0, 0, 1 });
	}


	/*! \brief returns a matrix rotated around the y axis.
		\param Theta the angle to use.
		*/
	static LWSMatrix4 RotationY(Type Theta) {
		Type S = sin(Theta);
		Type C = cos(Theta);
		return LWSMatrix4({ C, 0, S, 0 },
			{ 0, 1, 0, 0 },
			{ -S, 0, C, 0 },
			{ 0, 0, 0, 1 });
	}

	/*! \brief returns a matrix rotated around the z axis.
		\param Theta the angle to use.
		*/
	static LWSMatrix4 RotationZ(Type Theta) {
		Type S = sin(Theta);
		Type C = cos(Theta);
		return LWSMatrix4({ C, -S, 0, 0 },
			{ S, C, 0, 0 },
			{ 0, 0, 1, 0 },
			{ 0, 0, 0, 1 });
	}

	/*! \brief returns a matrix rotated both around the x and y axis in that order.
		\param ThetaX the x-axis angle.
		\param ThetaY the y-axis angle.
		*/
	static LWSMatrix4 RotationXY(Type ThetaX, Type ThetaY) {
		return RotationX(ThetaX)*RotationY(ThetaY);
	}

	/*! \brief returns a matrix rotated both around the x and z axis.
		\param ThetaX the x-axis angle.
		\param ThetaZ the z-axis angle.
		*/
	static LWSMatrix4 RotationXZ(Type ThetaX, Type ThetaZ) {
		return RotationX(ThetaX)*RotationZ(ThetaZ);
	}

	/*! \brief returns a matrix rotated both around the y and x axis in that order.
		\param ThetaY the y-axis angle.
		\param ThetaX the x-axis angle.
		*/
	static LWSMatrix4 RotationYX(Type ThetaY, Type ThetaX) {
		return RotationY(ThetaY)*RotationX(ThetaX);
	}

	/*! \brief returns a matrix rotated both around the y and z axis in that order.
		\param ThetaY the y-axis angle.
		\param ThetaZ the z-axis angle.
		*/
	static LWSMatrix4 RotationYZ(Type ThetaY, Type ThetaZ) {
		return RotationY(ThetaY)*RotationZ(ThetaZ);
	}

	/*! \brief returns a matrix rotated both around the z and x axis in that order.
		\param ThetaZ the z-axis angle.
		\param ThetaX the x-axis angle.
		*/

	static LWSMatrix4 RotationZX(Type ThetaZ, Type ThetaX) {
		return RotationZ(ThetaZ)*RotationX(ThetaX);
	}

	/*! \brief returns a matrix rotated both around the z and y axis in that order.
		\param ThetaZ the z-axis angle.
		\param ThetaY the y-axis angle.
		*/
	static LWSMatrix4 RotationZY(Type ThetaZ, Type ThetaY) {
		return RotationZ(ThetaZ)*RotationY(ThetaY);
	}

	/*! \brief returns a matrix rotated around the x, y, and z axis in that order.
		\param ThetaX the x-axis angle.
		\param ThetaY the y-axis angle.
		\param ThetaZ the z-axis angle.
		*/
	static LWSMatrix4 RotationXYZ(Type ThetaX, Type ThetaY, Type ThetaZ) {
		return RotationX(ThetaX)*RotationY(ThetaY)*RotationZ(ThetaZ);
	}

	/*! \brief returns a matrix rotated around the x, z, and y axis in that order.
		\param ThetaX the x-axis angle.
		\param ThetaZ the z-axis angle.
		\param ThetaY the y-axis angle.
		*/
	static LWSMatrix4 RotationXZY(Type ThetaX, Type ThetaZ, Type ThetaY) {
		return RotationX(ThetaX)*RotationZ(ThetaZ)*RotationY(ThetaY);
	}

	/*! \brief returns a matrix rotated around the y, x, and z axis in that order.
		\param ThetaY the y-axis angle.
		\param ThetaX the x-axis angle.
		\param ThetaZ the z-axis angle.
		*/
	static LWSMatrix4 RotationYXZ(Type ThetaY, Type ThetaX, Type ThetaZ) {
		return RotationY(ThetaY)*RotationX(ThetaX)*RotationZ(ThetaZ);
	}

	/*! \brief returns a matrix rotated around the y, z, and x axis in that order.
		\param ThetaY the y-axis angle.
		\param ThetaZ the z-axis angle.
		\param ThetaX the x-axis angle.
		*/
	static LWSMatrix4 RotationYZX(Type ThetaY, Type ThetaZ, Type ThetaX) {
		return RotationY(ThetaY)*RotationZ(ThetaZ)*RotationX(ThetaX);
	}

	/*! \brief returns a matrix rotated around the z, x, and y axis in that order.
		\param ThetaZ the z-axis angle.
		\param ThetaX the x-axis angle.
		\param ThetaY the y-axis angle.
		*/
	static LWSMatrix4 RotationZXY(Type ThetaZ, Type ThetaX, Type ThetaY) {
		return RotationZ(ThetaZ)*RotationX(ThetaX)*RotationY(ThetaY);
	}

	/*! \brief returns a matrix rotated around the z, y, and x axis in that order.
		\param ThetaZ the z-axis angle.
		\param ThetaY the y-axis angle.
		\param ThetaX the x-axis angle.
		*/
	static LWSMatrix4 RotationZYX(Type ThetaZ, Type ThetaY, Type ThetaX) {
		return RotationZ(ThetaZ)*RotationY(ThetaY)*RotationX(ThetaX);
	}

	/*! \brief returns a matrix translated along the x, y, and z axis.
		\param x the x axis to translate along.
		\param y the y axis to translate along.
		\param z the z axis to translate along.
		*/
	static LWSMatrix4 Translation(Type x, Type y, Type z) {
		return Translation({ x, y, z, 1 });
	}

	/*! \brief returns a matrix translated along the x, y, and z axis in the position vector3.
		\param Position the translation to apply.
	*/
	static LWSMatrix4 Translation(const LWSVector4<Type> &Position) {
		return LWSMatrix4({ 1, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, 1, 0 }, Position);
	}

	/*!< \brief constructs a rotation matrix from the directional vector which is relative to the supplied up vector.
		 \param Direction the normalized directional vector.
		 \param Up the normalized up vector.
		 \note the direction and up w component should both be 0 for a correct matrix output.
	*/
	static LWSMatrix4 Rotation(const LWSVector4<Type> &Direction, const LWSVector4<Type> &Up) {
		LWSVector4<Type> xAxis = Up.Cross3(Direction);
		LWSVector4<Type> yAxis = Direction.Cross3(xAxis);
		return LWSMatrix4<Type>(xAxis, yAxis, Direction, { 0, 0, 0, 1 });
	}

	/*! \brief returns a camera's perspective matrix.
		\param FoV the Field of View angle that is multiplied by the aspect.
		\param Aspect the ratio of the views width/height.
		\param Near the near plane of the camera.
		\param Far the far plane of the camera.
		*/
	static LWSMatrix4 Perspective(Type FoV, Type Aspect, Type Near, Type Far) {
		Type F = 1 / tan(FoV / 2);
		return LWSMatrix4({ F / Aspect, 0, 0,                            0 },
			{ 0,          F, 0,                            0 },
			{ 0,          0, (Far + Near) / (Near - Far), -1 },
			{ 0,          0, (Type)2.0*Far*Near / (Near - Far),  0 });
	}

	/*! \brief returns a camera's boxed orthographic matrix for d3d11+metal NDC's where z range is 0-1.
		\param Left the left side of the camera's view.
		\param Right the right side of the camera's view.
		\param Bottom the bottom side of the camera's view.
		\param Top the top side of the camera's view.
		\param Near the camera's near plane.
		\param Far the camera's far plane.
	*/
	static LWSMatrix4 OrthoDX(Type Left, Type Right, Type Bottom, Type Top, Type Near, Type Far) {
		Type sDepth = 1 / (Far - Near);
		return LWSMatrix4({ 2 / (Right - Left), 0, 0, 0 },
			{ 0, 2 / (Top - Bottom), 0, 0 },
			{ 0, 0, -sDepth, 0 },
			{ -(Right + Left) / (Right - Left), -(Top + Bottom) / (Top - Bottom), Near*sDepth, 1 });
	}


	/*! \brief returns a camera's boxed orthographic matrix for openGL NDC's(z range of -1-1).
		\param Left the left side of the camera's view.
		\param Right the right side of the camera's view.
		\param Bottom the bottom side of the camera's view.
		\param Top the top side of the camera's view.
		\param Near the camera's near plane.
		\param Far the camera's far plane.
		*/

	static LWSMatrix4 OrthoGL(Type Left, Type Right, Type Bottom, Type Top, Type Near, Type Far) {
		return LWSMatrix4({ 2 / (Right - Left), 0, 0, 0 },
			{ 0, 2 / (Top - Bottom), 0, 0 },
			{ 0, 0, -2 / (Far - Near), 0 },
			{ -(Right + Left) / (Right - Left), -(Top + Bottom) / (Top - Bottom), -(Near + Far) / (Far - Near), 1 });
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

	static LWSMatrix4 Ortho(Type Left, Type Right, Type Bottom, Type Top, Type Near, Type Far) {
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
	static LWSMatrix4 Frustum(Type Left, Type Right, Type Bottom, Type Top, Type Near, Type Far) {
		return LWSMatrix4({ 2 * Near / (Right - Left), 0, (Right + Left) / (Right - Left), 0 },
			{ 0, 2 * Near / (Top - Bottom), (Top + Bottom) / (Top - Bottom), 0 },
			{ 0, 0, -(Far + Near) / (Far - Near), -1 },
			{ 0, 0, -2 * Far*Near / (Far - Near), 0 });
	}

	/*! \brief generates a view matrix for looking at a specific target.
		\param Position the position of the viewing agent.
		\param Target the target to look at.
		\param Up the up matrix to work off of.
		\note Target+Position w component should both be set to 1, Up's w paramter should be set to 0.
		*/
	static LWSMatrix4 LookAt(const LWSVector4<Type> &Position, const LWSVector4<Type> &Target, const LWSVector4<Type> &Up) {
		LWSVector4<Type> Fwrd = (Target - Position).Normalize3();
		LWSVector4<Type> Rght = Fwrd.Cross3(Up).Normalize3();
		LWSVector4<Type> U = Rght.Cross3(Fwrd);
		return LWSMatrix4(Rght, U, Fwrd, Position);
	}

	/*!< \brief constructs a 4x4 rotational matrix from the specified quaternion. */
	
	LWSMatrix4(const LWQuaternion<Type> &Q) {
		LWSVector4<Type> VQ = Q.AsVec4();
		
		LWSVector4<Type> yy_xx_xx_xx = VQ.yxxx()*VQ.yxxx();
		LWSVector4<Type> zz_zz_yy_xx = VQ.zzyx()*VQ.zzyx();

		LWSVector4<Type> xz_xy_yz_xx = VQ.xxyx()*VQ.zyzx();
		LWSVector4<Type> yw_zw_xw_xx = VQ.yzxx()*VQ.wwwx();
		
		LWSVector4<Type> One = LWSVector4<Type>(1, 1, 1, 0);
		LWSVector4<Type> Two = LWSVector4<Type>(2, 2, 2, 0);

		LWSVector4<Type> A = One - Two*(yy_xx_xx_xx + zz_zz_yy_xx);
		LWSVector4<Type> B = Two * (xz_xy_yz_xx + yw_zw_xw_xx);
		LWSVector4<Type> C = Two * (xz_xy_yz_xx - yw_zw_xw_xx);

		m_Rows[0] = A.xwww() + C.wyww() + B.wwxw();
		m_Rows[1] = B.ywww() + A.wyww() + C.wwzw();
		m_Rows[2] = C.xwww() + B.wzww() + A.wwzw();
		m_Rows[3] = LWSVector4<Type>(0, 0, 0, 1);

	}
	
	/*! \brief constructs a 4x4 matrix with each row scaled by the respective scales.
	*/
	LWSMatrix4(Type xScale = 1, Type yScale = 1, Type zScale = 1, Type wScale = 1) {
		m_Rows[0] = { xScale, 0, 0, 0 };
		m_Rows[1] = { 0, yScale, 0, 0 };
		m_Rows[2] = { 0, 0, zScale, 0 };
		m_Rows[3] = { 0, 0, 0, wScale };
	}

	/*! \brief constructs a 4x4 matrix where each row is set by the applications. */
	LWSMatrix4(const LWSVector4<Type> &RowA, const LWSVector4<Type> &RowB, const LWSVector4<Type> &RowC, const LWSVector4<Type> &RowD) {
		m_Rows[0] = RowA;
		m_Rows[1] = RowB;
		m_Rows[2] = RowC;
		m_Rows[3] = RowD;
	}

	/*!< \brief constructs a 4x4 matrix from Scale, Rotation, Position components. */
	/*
	LWSMatrix4(const LWSVector4<Type> &Scale, const LWQuaternion<Type> &Rotation, const LWVector3<Type> &Pos) {
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
	};*/
};

#ifndef LW_NOAVX2
#include "LWCore/LWSMatrix_AVX2_Float.h"
#endif

#endif