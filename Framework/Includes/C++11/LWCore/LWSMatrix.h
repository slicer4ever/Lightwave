<<<<<<< HEAD
#ifndef LWSMATRIX_H
#define LWSMATRIX_H
#include <LWCore/LWTypes.h>
#include <LWCore/LWMatrix.h>
#include <LWCore/LWSVector.h>
#include <ostream>

/*!< \brief an accelerated simd matrix4 class, non-implemented or disabled sse functions default to a generic class. */
template<class Type>
struct LWSMatrix4 {
	LWSVector4<Type> m_Rows[4];

	/*! \brief returns a mat4 version of the SMatrix. */
	LWMatrix4<Type> AsMat4(void) const {
		return LWMatrix4<Type>(m_Rows[0].AsVec4(), m_Rows[1].AsVec4(), m_Rows[2].AsVec4(), m_Rows[3].AsVec4());
	}

	/*! \brief returns the internal components as a array of data. */
	Type *AsArray(void) {
		return m_Rows[0].AsArray();
	}

	/*! \brief returns the internal components as a const array of data. */
	const Type *AsArray(void) const {
		return m_Rows[0].AsArray();
	}

	/*! \brief set's a row*4+column of the matrix to value. */
	LWMatrix4<Type> &sRC(uint32_t Row, uint32_t Column, Type Value) {
		Type *V = AsArray();
		V[Row * 4 + Column] = Value;
		return *this;
	}

	/*!< \brief decomposes 4x4 matrix to get the scalar for each axis.
		 \param Transpose will transpose the 3x3 rotation+scale matrix before calculating the scale/rotation. */
	LWSVector4<Type> DecomposeScale(bool doTranspose3x3) const {
		LWSMatrix4<Type> v = doTranspose3x3 ? Transpose3x3() : *this;
		return LWSVector4<Type>(v.m_Rows[0].Length3(), v.m_Rows[1].Length3(), v.m_Rows[2].Length3(), 1.0f);
	};

	/*!< \brief decomposes 4x4 matrix to get scale, rotation, and translation.
		 \param Transpose will transpose the 3x3 rotation+scale matrix before calculating the scale/rotation.*/
	void Decompose(LWSVector4<Type> &Scale, LWSQuaternion<Type> &Rotation, LWSVector4<Type> &Translation, bool doTranspose3x3) const {
		LWSMatrix4<Type> v = doTranspose3x3 ? Transpose3x3() : *this;
		Scale = LWSVector4<Type>(v.m_Rows[0].Length3(), v.m_Rows[1].Length3(), v.m_Rows[2].Length3(), 1.0f);
		LWSVector4<Type> iScale = 1.0 / Scale;
		v = LWSMatrix4<Type>(v.m_Rows[0] * iScale.xxxx(), v.m_Rows[1] * iScale.yyyy(), v.m_Rows[2] * iScale.zzzz(), v.m_Rows[3]);

		Rotation = LWSQuaternion<Type>(v);
		Translation = v.m_Rows[3];
		return;
	}

	LWSMatrix4 TransformInverse(void) const {
		const Type E = std::numeric_limits<Type>::epsilon();
		//Transpose matrix.
		LWSMatrix4 T3 = Transpose3x3();
		LWSVector4<Type> A = T3.m_Rows[0];
		LWSVector4<Type> B = T3.m_Rows[1];
		LWSVector4<Type> C = T3.m_Rows[2];

		LWSVector4<Type> One = LWSVector4<Type>(1, 1, 1, 0);
		LWSVector4<Type> Sq = A * A + B * B + C * C;
		LWSVector4<Type> rSq = (One / Sq).AAAB(One);
		LWVector4<Type> SqV = Sq.AsVec4();
		if (SqV.x < E) rSq = rSq.BAAA(One);
		if (SqV.y < E) rSq = rSq.ABAA(One);
		if (SqV.z < E) rSq = rSq.AABA(One);
		A *= rSq;
		B *= rSq;
		C *= rSq;

		LWSVector4<Type> Dx = m_Rows[3].xxxw();
		LWSVector4<Type> Dy = m_Rows[3].yyyw();
		LWSVector4<Type> Dz = m_Rows[3].zzzw();
		LWSVector4<Type> D = -(A*Dx + B * Dy + C * Dz);
		return LWSMatrix4(A, B, C, D.AAAB(m_Rows[3]));
	}

	/*! \brief Returns an copied inverse of this matrix. */
	LWSMatrix4 Inverse(void) const {
		//adapted Non-simd version Found from: https://stackoverflow.com/questions/1148309/inverting-a-4x4-matrix
		LWSVector4<Type> A = m_Rows[0];
		LWSVector4<Type> B = m_Rows[1];
		LWSVector4<Type> C = m_Rows[2];
		LWSVector4<Type> D = m_Rows[3];

		LWSVector4<Type> Czzyy = C.zzyy();
		LWSVector4<Type> Dwwwz = D.wwwz();
		LWSVector4<Type> Dzzyy = D.zzyy();
		LWSVector4<Type> Cwwwz = C.wwwz();

		LWSVector4<Type> Cyxxx = C.yxxx();
		LWSVector4<Type> Dyxxx = D.yxxx();

		LWSVector4<Type> CzzBzz = C.zzzz().AABB(B.zzzz());
		LWSVector4<Type> DwwwCw = D.wwww().AAAB(C.wwww());
		LWSVector4<Type> CwwBww = C.wwww().AABB(B.wwww());
		LWSVector4<Type> DzzzCz = D.zzzz().AAAB(C.zzzz());
		LWSVector4<Type> CyyByy = C.yyyy().AABB(B.yyyy());
		LWSVector4<Type> DyyCyy = D.yyyy().AABB(C.yyyy());
		LWSVector4<Type> DyyyCy = D.yyyy().AAAB(C.yyyy());
		LWSVector4<Type> CxxBxx = C.xxxx().AABB(B.xxxx());
		LWSVector4<Type> DxxxCx = D.xxxx().AAAB(C.xxxx());

		LWSVector4<Type> A2323_A2323_A1323_A1223 = Czzyy * Dwwwz - Cwwwz * Dzzyy;
		LWSVector4<Type> A1323_A0323_A0323_A0223 = Cyxxx * Dwwwz - Cwwwz * Dyxxx;
		LWSVector4<Type> A1223_A0223_A0123_A0123 = Cyxxx * Dzzyy - Czzyy * Dyxxx;

		LWSVector4<Type> A2323_A2323_A2313_A2312 = CzzBzz * DwwwCw - CwwBww * DzzzCz;
		LWSVector4<Type> A1323_A1323_A1313_A1312 = CyyByy * DwwwCw - CwwBww * DyyCyy;
		LWSVector4<Type> A1223_A1223_A1213_A1212 = CyyByy * DzzzCz - CzzBzz * DyyyCy;
		LWSVector4<Type> A0323_A0323_A0313_A0312 = CxxBxx * DwwwCw - CwwBww * DxxxCx;
		LWSVector4<Type> A0223_A0223_A0213_A0212 = CxxBxx * DzzzCz - CzzBzz * DxxxCx;
		LWSVector4<Type> A0123_A0123_A0113_A0112 = CxxBxx * DyyyCy - CyyByy * DxxxCx;
			
		LWSVector4<Type> MulA = LWSVector4<Type>(1, -1, 1, -1);
		LWSVector4<Type> MulB = LWSVector4<Type>(-1, 1, -1, 1);
		LWSVector4<Type> Det = ((A * (B.yxxx() * A2323_A2323_A1323_A1223 - B.zzyy() * A1323_A0323_A0323_A0223 + B.wwwy() * A1223_A0223_A0123_A0123))*MulA).Sum();
		if (Det.x() <= std::numeric_limits<Type>::epsilon()) Det = LWSVector4<Type>(0, 0, 0, 0);
		else Det = (Type)1 / Det;

		LWSVector4<Type> ByAyyy = B.yyyy().ABBB(A.yyyy());
		LWSVector4<Type> BzAzzz = B.zzzz().ABBB(A.zzzz());
		LWSVector4<Type> BwAwww = B.wwww().ABBB(A.wwww());
		LWSVector4<Type> BxAxxx = B.xxxx().ABBB(A.xxxx());
		A = (ByAyyy * A2323_A2323_A2313_A2312 - BzAzzz * A1323_A1323_A1313_A1312 + BwAwww * A1223_A1223_A1213_A1212) * MulA * Det;
		B = (BxAxxx * A2323_A2323_A2313_A2312 - BzAzzz * A0323_A0323_A0313_A0312 + BwAwww * A0223_A0223_A0213_A0212) * MulB * Det;
		C = (BxAxxx * A1323_A1323_A1313_A1312 - ByAyyy * A0323_A0323_A0313_A0312 + BwAwww * A0123_A0123_A0113_A0112) * MulA * Det;
		D = (BxAxxx * A1223_A1223_A1213_A1212 - ByAyyy * A0223_A0223_A0213_A0212 + BzAzzz * A0123_A0123_A0113_A0112) * MulB * Det;
		return LWSMatrix4(A, B, C, D);
	}

	/*!< \brief returns the specified column of the matrix. */
	LWSVector4<Type> Column(uint32_t Index) const {
		return Transpose().m_Rows[Index];
	};

	LWSVector4<Type> Row(uint32_t Index) const {
		return m_Rows[Index];
	}

	/*! \brief returns the transpose of the this matrix. */
	LWSMatrix4 Transpose(void) const {
		LWSVector4<Type> A = m_Rows[0];
		LWSVector4<Type> B = m_Rows[1];
		LWSVector4<Type> C = m_Rows[2];
		LWSVector4<Type> D = m_Rows[3];
		LWSVector4<Type> Ay = A.yyyy();
		LWSVector4<Type> Az = A.zzzz();
		LWSVector4<Type> Aw = A.wwww();
		LWSVector4<Type> Bx = B.xxxx();
		LWSVector4<Type> Bz = B.zzzz();
		LWSVector4<Type> Bw = B.wwww();
		LWSVector4<Type> Cx = C.xxxx();
		LWSVector4<Type> Cy = C.yyyy();
		LWSVector4<Type> Cw = C.wwww();
		LWSVector4<Type> Dx = D.xxxx();
		LWSVector4<Type> Dy = D.yyyy();
		LWSVector4<Type> Dz = D.zzzz();

		return LWSMatrix4(A.ABAA(Bx).AABA(Cx).AAAB(Dx),
						  Ay.ABAA(B).AABA(Cy).AAAB(Dy),
						  Az.ABAA(Bz).AABA(C).AAAB(Dz),
						  Aw.ABAA(Bw).AABA(Cw).AAAB(D));
	}

	/*!< \brief returns the upper left 3x3 matrix transposed only. */
	LWSMatrix4 Transpose3x3(void) const {
		LWSVector4<Type> A = m_Rows[0];
		LWSVector4<Type> B = m_Rows[1];
		LWSVector4<Type> C = m_Rows[2];
		LWSVector4<Type> Ay = A.yyyy();
		LWSVector4<Type> Az = A.zzzz();
		LWSVector4<Type> Bx = B.xxxx();
		LWSVector4<Type> Bz = B.zzzz();
		LWSVector4<Type> Cx = C.xxxx();
		LWSVector4<Type> Cy = C.yyyy();

		return LWSMatrix4(A.ABAA(Bx).AABA(Cx),
			Ay.ABAB(B).AABA(Cy),
			Az.ABAA(Bz).AABB(C),
			m_Rows[3]);
	}

	/*!< \brief returns the upper left 2x2 matrix transposed only. */
	LWSMatrix4 Transpose2x2(void) const {
		LWSVector4<Type> A = m_Rows[0];
		LWSVector4<Type> B = m_Rows[1];
		LWSVector4<Type> Ay = A.yyyy();
		LWSVector4<Type> Bx = B.xxxx();
		return LWSMatrix4(A.ABAA(Bx), Ay.ABBB(B), m_Rows[2], m_Rows[3]);
	}
	/*! \brief calculates the determinant of this matrix. */
	
	Type Determinant(void) const {
		//adapted Non-simd version Found from: https://stackoverflow.com/questions/1148309/inverting-a-4x4-matrix
		LWSVector4<Type> A = m_Rows[0];
		LWSVector4<Type> B = m_Rows[1];
		LWSVector4<Type> C = m_Rows[2];
		LWSVector4<Type> D = m_Rows[3];

		LWSVector4<Type> Czzyy = C.zzyy();
		LWSVector4<Type> Dwwwz = D.wwwz();
		LWSVector4<Type> Dzzyy = D.zzyy();
		LWSVector4<Type> Cwwwz = C.wwwz();

		LWSVector4<Type> Cyxxx = C.yxxx();
		LWSVector4<Type> Dyxxx = D.yxxx();

		LWSVector4<Type> A2323_A2323_A1323_A1223 = Czzyy * Dwwwz - Cwwwz * Dzzyy;
		LWSVector4<Type> A1323_A0323_A0323_A0223 = Cyxxx * Dwwwz - Cwwwz * Dyxxx;
		LWSVector4<Type> A1223_A0223_A0123_A0123 = Cyxxx * Dzzyy - Czzyy * Dyxxx;
		
		LWSVector4<Type> Mul = LWSVector4<Type>(1, -1, 1, -1);
		return ((A * (B.yxxx() * A2323_A2323_A1323_A1223 - B.zzyy() * A1323_A0323_A0323_A0223 + B.wwwy() * A1223_A0223_A0123_A0123))*Mul).Sum4();
	}

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
		LWSVector4<Type> Ax = m_Rows[0].xxxx();
		LWSVector4<Type> Ay = m_Rows[0].yyyy();
		LWSVector4<Type> Az = m_Rows[0].zzzz();
		LWSVector4<Type> Aw = m_Rows[0].wwww();
		LWSVector4<Type> A = Ax * Rhs.m_Rows[0] + Ay * Rhs.m_Rows[1] + Az*Rhs.m_Rows[2] + Aw*Rhs.m_Rows[3];

		LWSVector4<Type> Bx = m_Rows[1].xxxx();
		LWSVector4<Type> By = m_Rows[1].yyyy();
		LWSVector4<Type> Bz = m_Rows[1].zzzz();
		LWSVector4<Type> Bw = m_Rows[1].wwww();
		LWSVector4<Type> B = Bx * Rhs.m_Rows[0] + By * Rhs.m_Rows[1] + Bz * Rhs.m_Rows[2] + Bw * Rhs.m_Rows[3];

		LWSVector4<Type> Cx = m_Rows[2].xxxx();
		LWSVector4<Type> Cy = m_Rows[2].yyyy();
		LWSVector4<Type> Cz = m_Rows[2].zzzz();
		LWSVector4<Type> Cw = m_Rows[2].wwww();
		LWSVector4<Type> C = Cx * Rhs.m_Rows[0] + Cy * Rhs.m_Rows[1] + Cz * Rhs.m_Rows[2] + Cw * Rhs.m_Rows[3];

		LWSVector4<Type> Dx = m_Rows[3].xxxx();
		LWSVector4<Type> Dy = m_Rows[3].yyyy();
		LWSVector4<Type> Dz = m_Rows[3].zzzz();
		LWSVector4<Type> Dw = m_Rows[3].wwww();
		LWSVector4<Type> D = Dx * Rhs.m_Rows[0] + Dy * Rhs.m_Rows[1] + Dz * Rhs.m_Rows[2] + Dw * Rhs.m_Rows[3];
		
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
		LWSVector4<Type> Ax = Lhs.m_Rows[0].xxxx();
		LWSVector4<Type> Ay = Lhs.m_Rows[0].yyyy();
		LWSVector4<Type> Az = Lhs.m_Rows[0].zzzz();
		LWSVector4<Type> Aw = Lhs.m_Rows[0].wwww();
		LWSVector4<Type> A = Ax * Rhs.m_Rows[0] + Ay * Rhs.m_Rows[1] + Az * Rhs.m_Rows[2] + Aw * Rhs.m_Rows[3];

		LWSVector4<Type> Bx = Lhs.m_Rows[1].xxxx();
		LWSVector4<Type> By = Lhs.m_Rows[1].yyyy();
		LWSVector4<Type> Bz = Lhs.m_Rows[1].zzzz();
		LWSVector4<Type> Bw = Lhs.m_Rows[1].wwww();
		LWSVector4<Type> B = Bx * Rhs.m_Rows[0] + By * Rhs.m_Rows[1] + Bz * Rhs.m_Rows[2] + Bw * Rhs.m_Rows[3];

		LWSVector4<Type> Cx = Lhs.m_Rows[2].xxxx();
		LWSVector4<Type> Cy = Lhs.m_Rows[2].yyyy();
		LWSVector4<Type> Cz = Lhs.m_Rows[2].zzzz();
		LWSVector4<Type> Cw = Lhs.m_Rows[2].wwww();
		LWSVector4<Type> C = Cx * Rhs.m_Rows[0] + Cy * Rhs.m_Rows[1] + Cz * Rhs.m_Rows[2] + Cw * Rhs.m_Rows[3];

		LWSVector4<Type> Dx = Lhs.m_Rows[3].xxxx();
		LWSVector4<Type> Dy = Lhs.m_Rows[3].yyyy();
		LWSVector4<Type> Dz = Lhs.m_Rows[3].zzzz();
		LWSVector4<Type> Dw = Lhs.m_Rows[3].wwww();
		LWSVector4<Type> D = Dx * Rhs.m_Rows[0] + Dy * Rhs.m_Rows[1] + Dz * Rhs.m_Rows[2] + Dw * Rhs.m_Rows[3];
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
		return LWSMatrix4({ 1, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, 1, 0 }, Position.AAAB(LWSVector4<Type>(1)));
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
		return LWSMatrix4(Rght, U, -Fwrd, Position);
	}

	/*! \brief copys the LWMatrix4 to LWSMatrix4. */
	LWSMatrix4(const LWMatrix4<Type> &M) {
		m_Rows[0] = LWSVector4<Type>(M.m_Rows[0]);
		m_Rows[1] = LWSVector4<Type>(M.m_Rows[1]);
		m_Rows[2] = LWSVector4<Type>(M.m_Rows[2]);
		m_Rows[3] = LWSVector4<Type>(M.m_Rows[3]);
	}

	/*!< \brief constructs a 4x4 rotational matrix from the specified quaternion. */

	LWSMatrix4(const LWSQuaternion<Type> &Q) {
		LWSVector4<Type> VQ = Q.AsSVec4();
		
		LWSVector4<Type> yy_xx_xx_xx = VQ.yxxx()*VQ.yxxx();
		LWSVector4<Type> zz_zz_yy_xx = VQ.zzyx()*VQ.zzyx();

		LWSVector4<Type> xz_xy_yz_xx = VQ.xxyx()*VQ.zyzx();
		LWSVector4<Type> yw_zw_xw_xx = VQ.yzxx()*VQ.wwwx();
		
		LWSVector4<Type> One = LWSVector4<Type>(1, 1, 1, 0);
		LWSVector4<Type> Two = LWSVector4<Type>(2, 2, 2, 0);

		LWSVector4<Type> A = One - Two*(yy_xx_xx_xx + zz_zz_yy_xx);
		LWSVector4<Type> B = Two * (xz_xy_yz_xx + yw_zw_xw_xx);
		LWSVector4<Type> C = Two * (xz_xy_yz_xx - yw_zw_xw_xx);

		m_Rows[0] = (A.ABAA(C)).AABA(B.xxxx());
		m_Rows[1] = (B.yyww().ABAA(A)).AABA(C);
		m_Rows[2] = (C.ABAA(B.zzzz())).AABA(A);
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
	
	/*!< \brief constructs a 4x4 matrix from Scale, Rotation, Position components(Position should have 1 as it's w component). */
	LWSMatrix4(const LWSVector4<Type> &Scale, const LWSQuaternion<Type> &Rotation, const LWSVector4<Type> &Pos) {
		LWSVector4<Type> VQ = Rotation.AsSVec4();

		LWSVector4<Type> yy_xx_xx_xx = VQ.yxxx()*VQ.yxxx();
		LWSVector4<Type> zz_zz_yy_xx = VQ.zzyx()*VQ.zzyx();

		LWSVector4<Type> xz_xy_yz_xx = VQ.xxyx()*VQ.zyzx();
		LWSVector4<Type> yw_zw_xw_xx = VQ.yzxx()*VQ.wwwx();

		LWSVector4<Type> One = LWSVector4<Type>(1, 1, 1, 0);
		LWSVector4<Type> Two = LWSVector4<Type>(2, 2, 2, 0);

		LWSVector4<Type> A = One - Two * (yy_xx_xx_xx + zz_zz_yy_xx);
		LWSVector4<Type> B = Two * (xz_xy_yz_xx + yw_zw_xw_xx);
		LWSVector4<Type> C = Two * (xz_xy_yz_xx - yw_zw_xw_xx);

		m_Rows[0] = ((A.ABAA(C)).AABA(B.xxxx()))*Scale.xxxw();
		m_Rows[1] = ((B.yyww().ABAA(A)).AABA(C))*Scale.yyyw();
		m_Rows[2] = ((C.ABAA(B.zzzz())).AABA(A))*Scale.zzzw();
		m_Rows[3] = Pos;
	};
};

#ifndef LW_NOAVX2
#include "LWCore/LWSMatrix_AVX2_Float.h"
#include "LWCore/LWSMatrix_AVX2_Double.h"
#endif

=======
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

>>>>>>> Added initial inroads for vulkan support(this is far from complete and not usable yet).
#endif