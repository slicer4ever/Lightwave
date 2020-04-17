#ifndef LWSMATRIX_AVX2_FLOAT_H
#define LWSMATRIX_AVX2_FLOAT_H
#include "LWSMatrix.h"
#include <smmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>
#include "LWCore/LWSQuaternion.h"

template<>
struct LWSMatrix4<float> {
	__m256 m_Row01;
	__m256 m_Row23;

	LWMatrix4<float> AsMat4(void) const {
		alignas(64) LWMatrix4<float> R;
		_mm256_store_ps(&R.m_Rows[0].x, m_Row01);
		_mm256_store_ps(&R.m_Rows[2].x, m_Row23);
		return R;
	}
	/*
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
	*/
	/*
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

		LWSVector4<Type> ByAxxx = B.yyyy().ABBB(A.xxxx());
		LWSVector4<Type> ByAyyy = B.yyyy().ABBB(A.yyyy());
		LWSVector4<Type> BzAzzz = B.zzzz().ABBB(A.zzzz());
		LWSVector4<Type> BwAwww = B.wwww().ABBB(A.wwww());
		LWSVector4<Type> BxAxxx = B.xxxx().ABBB(A.xxxx());
		A = (ByAyyy * A2323_A2323_A2313_A2312 - BzAzzz * A1323_A1323_A1313_A1312 + BwAwww * A1223_A1223_A1213_A1212) * MulA * Det;
		B = (ByAxxx * A2323_A2323_A2313_A2312 - BzAzzz * A0323_A0323_A0313_A0312 + BwAwww * A0223_A0223_A0213_A0212) * MulB * Det;
		C = (BxAxxx * A1323_A1323_A1313_A1312 - ByAyyy * A0323_A0323_A0313_A0312 + BwAwww * A0123_A0123_A0113_A0112) * MulA * Det;
		D = (BxAxxx * A1223_A1223_A1213_A1212 - ByAyyy * A0223_A0223_A0213_A0212 + BzAzzz * A0123_A0123_A0113_A0112) * MulB * Det;
		return LWSMatrix4(A, B, C, D);
	}*/
	
	LWSVector4<float> Column(uint32_t Index) {
		//Transpose for column.
		LWSMatrix4<float> T = Transpose();
		__m128 Row;
		if (Index == 0) Row = _mm256_extractf128_ps(T.m_Row01, 0);
		else if(Index==1) Row = _mm256_extractf128_ps(T.m_Row01, 1);
		else if (Index == 2) Row = _mm256_extractf128_ps(T.m_Row23, 0);
		else if (Index == 3) Row = _mm256_extractf128_ps(T.m_Row23, 1);
		return LWSVector4<float>(Row);
	};
	
	LWSMatrix4 Transpose(void) const {
		__m256 A = _mm256_unpacklo_ps(m_Row01, m_Row23);
		__m256 B = _mm256_unpackhi_ps(m_Row01, m_Row23);
		__m256 Row01 = _mm256_permutevar8x32_ps(A, _mm256_set_epi32(7, 3, 6, 2, 5, 1, 4, 0));
		__m256 Row23 = _mm256_permutevar8x32_ps(B, _mm256_set_epi32(7, 3, 6, 2, 5, 1, 4, 0));
		return LWSMatrix4(Row01, Row23);
	}
	
	LWSMatrix4 Transpose3x3(void) const {
		__m256 A = _mm256_unpacklo_ps(m_Row01, m_Row23);
		__m256 B = _mm256_unpackhi_ps(m_Row01, m_Row23);
		__m256 Row01 = _mm256_permutevar8x32_ps(A, _mm256_set_epi32(7, 3, 6, 2, 5, 1, 4, 0));
		__m256 Row23 = _mm256_permutevar8x32_ps(B, _mm256_set_epi32(7, 3, 6, 2, 5, 1, 4, 0));
		Row01 = _mm256_blend_ps(Row01, m_Row01, 0x88);
		Row23 = _mm256_blend_ps(Row23, m_Row23, 0xF8);
		return LWSMatrix4(Row01, Row23);
	}
	
	LWSMatrix4 Transpose2x2(void) const {
		__m256 A = _mm256_unpacklo_ps(m_Row01, m_Row23);
		__m256 Row01 = _mm256_permutevar8x32_ps(A, _mm256_set_epi32(7, 3, 6, 2, 5, 1, 4, 0));
		Row01 = _mm256_blend_ps(Row01, m_Row01, 0xCC);
		return LWSMatrix4(Row01, m_Row23);
	}
	
	float Determinant(void) const {
		//adapted Non-simd version Found from: https://stackoverflow.com/questions/1148309/inverting-a-4x4-matrix
		
		__m128 A = _mm256_extractf128_ps(m_Row01, 0);
		__m128 B = _mm256_extractf128_ps(m_Row01, 1);
		__m128 C = _mm256_extractf128_ps(m_Row23, 0);
		__m128 D = _mm256_extractf128_ps(m_Row23, 1);
		
		__m128 Czzyy = _mm_permute_ps(C, _MM_SHUFFLE(1, 1, 2, 2));
		__m128 Dwwwz = _mm_permute_ps(D, _MM_SHUFFLE(2, 3, 3, 3));
		__m128 Dzzyy = _mm_permute_ps(D, _MM_SHUFFLE(1, 1, 2, 2));
		__m128 Cwwwz = _mm_permute_ps(C, _MM_SHUFFLE(2, 3, 3, 3));

		__m128 Cyxxx = _mm_permute_ps(C, _MM_SHUFFLE(0, 0, 0, 1));
		__m128 Dyxxx = _mm_permute_ps(D, _MM_SHUFFLE(0, 0, 0, 1));

		__m128 A2323_A2323_A1323_A1223 = _mm_add_ps(_mm_mul_ps(Czzyy, Dwwwz), _mm_mul_ps(Cwwwz, Dzzyy));
		__m128 A1323_A0323_A0323_A0223 = _mm_add_ps(_mm_mul_ps(Cyxxx, Dwwwz), _mm_mul_ps(Cwwwz, Dyxxx));
		__m128 A1223_A0223_A0123_A0123 = _mm_add_ps(_mm_mul_ps(Cyxxx, Dzzyy), _mm_mul_ps(Czzyy, Dyxxx));

		__m128 Mul = _mm_set_ps(1.0f, -1.0f, 1.0f, -1.0f);

		__m128 Byxxx = _mm_permute_ps(B, _MM_SHUFFLE(0, 0, 0, 1));
		__m128 Bzzyy = _mm_permute_ps(B, _MM_SHUFFLE(1, 1, 2, 2));
		__m128 Bwwwy = _mm_permute_ps(B, _MM_SHUFFLE(2, 3, 3, 3));

		__m128 PtA = _mm_mul_ps(Byxxx, A2323_A2323_A1323_A1223);
		__m128 PtB = _mm_mul_ps(Bzzyy, A1323_A0323_A0323_A0223);
		__m128 Ptc = _mm_mul_ps(Bwwwy, A1223_A0223_A0123_A0123);
		__m128 r = _mm_mul_ps(_mm_mul_ps(_mm_add_ps(_mm_sub_ps(PtA, PtB), Ptc), A), Mul);
		r = _mm_hadd_ps(r, r);
		r = _mm_hadd_ps(r, r);
		return _mm_cvtss_f32(r);
	}

	LWSMatrix4 &operator = (const LWSMatrix4<float> &Rhs) {
		m_Row01 = Rhs.m_Row01;
		m_Row23 = Rhs.m_Row23;
		return *this;
	}

	LWSMatrix4 &operator+= (const LWSMatrix4<float> &Rhs) {
		m_Row01 = _mm256_add_ps(m_Row01, Rhs.m_Row01);
		m_Row23 = _mm256_add_ps(m_Row23, Rhs.m_Row23);
		return *this;
	}

	LWSMatrix4 &operator-= (const LWSMatrix4<float> &Rhs) {
		m_Row01 = _mm256_sub_ps(m_Row01, Rhs.m_Row01);
		m_Row23 = _mm256_sub_ps(m_Row23, Rhs.m_Row23);
		return *this;
	}

	LWSMatrix4 &operator*= (const LWSMatrix4<float> &Rhs) {
		__m256 ABxy = _mm256_permutevar8x32_ps(m_Row01, _mm256_set_epi32(5, 5, 5, 5, 0, 0, 0, 0));
		__m256 AByx = _mm256_permutevar8x32_ps(m_Row01, _mm256_set_epi32(4, 4, 4, 4, 1, 1, 1, 1));

		__m256 ABzw = _mm256_permutevar8x32_ps(m_Row01, _mm256_set_epi32(7, 7, 7, 7, 2, 2, 2, 2));
		__m256 ABwz = _mm256_permutevar8x32_ps(m_Row01, _mm256_set_epi32(6, 6, 6, 6, 3, 3, 3, 3));

		__m256 CDxy = _mm256_permutevar8x32_ps(m_Row23, _mm256_set_epi32(5, 5, 5, 5, 0, 0, 0, 0));
		__m256 CDyx = _mm256_permutevar8x32_ps(m_Row23, _mm256_set_epi32(4, 4, 4, 4, 1, 1, 1, 1));

		__m256 CDzw = _mm256_permutevar8x32_ps(m_Row23, _mm256_set_epi32(7, 7, 7, 7, 2, 2, 2, 2));
		__m256 CDwz = _mm256_permutevar8x32_ps(m_Row23, _mm256_set_epi32(6, 6, 6, 6, 3, 3, 3, 3));

		__m256 Row10 = _mm256_permutevar8x32_ps(Rhs.m_Row01, _mm256_set_epi32(3, 2, 1, 0, 7, 6, 5, 4));
		__m256 Row32 = _mm256_permutevar8x32_ps(Rhs.m_Row23, _mm256_set_epi32(3, 2, 1, 0, 7, 6, 5, 4));

		m_Row01 = _mm256_add_ps(_mm256_mul_ps(ABxy, Rhs.m_Row01), _mm256_mul_ps(AByx, Row10));
		m_Row01 = _mm256_add_ps(m_Row01, _mm256_mul_ps(ABzw, Rhs.m_Row23));
		m_Row01 = _mm256_add_ps(m_Row01, _mm256_mul_ps(ABwz, Row32));

		m_Row23 = _mm256_add_ps(_mm256_mul_ps(CDxy, Rhs.m_Row01), _mm256_mul_ps(CDyx, Row10));
		m_Row23 = _mm256_add_ps(m_Row23, _mm256_mul_ps(CDzw, Rhs.m_Row23));
		m_Row23 = _mm256_add_ps(m_Row23, _mm256_mul_ps(CDwz, Row32));
		return *this;
	}

	LWSMatrix4 &operator *=(float Rhs) {
		__m256 r = _mm256_set1_ps(Rhs);
		m_Row01 = _mm256_mul_ps(m_Row01, r);
		m_Row23 = _mm256_mul_ps(m_Row23, r);
		return *this;
	}

	LWSMatrix4 &operator /=(float Rhs) {
		__m256 r = _mm256_set1_ps(Rhs);
		m_Row01 = _mm256_div_ps(m_Row01, r);
		m_Row23 = _mm256_div_ps(m_Row23, r);
		return *this;
	}
	
	bool operator == (const LWSMatrix4<float> &Rhs) const {
		__m256 e = _mm256_set1_ps(std::numeric_limits<float>::epsilon());
		__m256 n0 = _mm256_set1_ps(-0.0f);
		__m256 t01 = _mm256_sub_ps(m_Row01, Rhs.m_Row01);
		__m256 t02 = _mm256_sub_ps(m_Row23, Rhs.m_Row23);
		//Get absolute value of difference:
		t01 = _mm256_andnot_ps(n0, t01);
		t02 = _mm256_andnot_ps(n0, t02);
		//compare le epsilon:
		t01 = _mm256_cmp_ps(t01, e, _CMP_LE_OS);
		t02 = _mm256_cmp_ps(t02, e, _CMP_LE_OS);
		return _mm256_test_all_ones(_mm256_castps_si256(t01)) && _mm256_test_all_ones(_mm256_castps_si256(t02));
	}

	bool operator != (const LWSMatrix4<float> &Rhs) const {
		return !(*this == Rhs);
	}
	
	friend std::ostream &operator<<(std::ostream &o, const LWSMatrix4<float> &M) {
		alignas(32) float V[16];
		_mm256_store_ps(V, M.m_Row01);
		_mm256_store_ps(V + 8, M.m_Row23);
		o << "Matrix4:" << std::endl;
		o << V[0] << " " << V[1] << " " << V[2] << " " << V[3] << std::endl;
		o << V[4] << " " << V[5] << " " << V[6] << " " << V[7] << std::endl;
		o << V[8] << " " << V[9] << " " << V[10] << " " << V[11] << std::endl;
		o << V[12] << " " << V[13] << " " << V[14] << " " << V[15] << std::endl;
		return o;
	}

	friend LWSMatrix4 operator + (const LWSMatrix4<float> &Lhs, const LWSMatrix4<float> &Rhs) {
		__m256 Row01 = _mm256_add_ps(Lhs.m_Row01, Rhs.m_Row01);
		__m256 Row23 = _mm256_add_ps(Lhs.m_Row23, Rhs.m_Row23);
		return LWSMatrix4<float>(Row01, Row23);
	}

	friend LWSMatrix4 operator - (const LWSMatrix4<float> &Lhs, const LWSMatrix4<float> &Rhs) {
		__m256 Row01 = _mm256_sub_ps(Lhs.m_Row01, Rhs.m_Row01);
		__m256 Row23 = _mm256_sub_ps(Lhs.m_Row23, Rhs.m_Row23);
		return LWSMatrix4<float>(Row01, Row23);
	}

	friend LWSMatrix4 operator * (const LWSMatrix4 &Lhs, const LWSMatrix4 &Rhs) {
		__m256 ABxy = _mm256_permutevar8x32_ps(Lhs.m_Row01, _mm256_set_epi32(5, 5, 5, 5, 0, 0, 0, 0));
		__m256 AByx = _mm256_permutevar8x32_ps(Lhs.m_Row01, _mm256_set_epi32(4, 4, 4, 4, 1, 1, 1, 1));

		__m256 ABzw = _mm256_permutevar8x32_ps(Lhs.m_Row01, _mm256_set_epi32(7, 7, 7, 7, 2, 2, 2, 2));
		__m256 ABwz = _mm256_permutevar8x32_ps(Lhs.m_Row01, _mm256_set_epi32(6, 6, 6, 6, 3, 3, 3, 3));

		__m256 CDxy = _mm256_permutevar8x32_ps(Lhs.m_Row23, _mm256_set_epi32(5, 5, 5, 5, 0, 0, 0, 0));
		__m256 CDyx = _mm256_permutevar8x32_ps(Lhs.m_Row23, _mm256_set_epi32(4, 4, 4, 4, 1, 1, 1, 1));

		__m256 CDzw = _mm256_permutevar8x32_ps(Lhs.m_Row23, _mm256_set_epi32(7, 7, 7, 7, 2, 2, 2, 2));
		__m256 CDwz = _mm256_permutevar8x32_ps(Lhs.m_Row23, _mm256_set_epi32(6, 6, 6, 6, 3, 3, 3, 3));

		__m256 Row10 = _mm256_permutevar8x32_ps(Rhs.m_Row01, _mm256_set_epi32(3, 2, 1, 0, 7, 6, 5, 4));
		__m256 Row32 = _mm256_permutevar8x32_ps(Rhs.m_Row23, _mm256_set_epi32(3, 2, 1, 0, 7, 6, 5, 4));
		
		__m256 Row01 = _mm256_add_ps(_mm256_mul_ps(ABxy, Rhs.m_Row01), _mm256_mul_ps(AByx, Row10));
		Row01 = _mm256_add_ps(Row01, _mm256_mul_ps(ABzw, Rhs.m_Row23));
		Row01 = _mm256_add_ps(Row01, _mm256_mul_ps(ABwz, Row32));

		__m256 Row23 = _mm256_add_ps(_mm256_mul_ps(CDxy, Rhs.m_Row01), _mm256_mul_ps(CDyx, Row10));
		Row23 = _mm256_add_ps(Row23, _mm256_mul_ps(CDzw, Rhs.m_Row23));
		Row23 = _mm256_add_ps(Row23, _mm256_mul_ps(CDwz, Row32));
		return LWSMatrix4(Row01, Row23);
	}

	friend LWSMatrix4 operator * (const LWSMatrix4<float> &Lhs, float Rhs) {
		__m256 r = _mm256_set1_ps(Rhs);
		__m256 Row01 = _mm256_mul_ps(Lhs.m_Row01, r);
		__m256 Row23 = _mm256_mul_ps(Lhs.m_Row23, r);
		return LWSMatrix4(Row01, Row23);
	}

	friend LWSMatrix4 operator * (float Lhs, const LWSMatrix4<float> &Rhs) {
		__m256 r = _mm256_set1_ps(Lhs);
		__m256 Row01 = _mm256_mul_ps(r, Rhs.m_Row01);
		__m256 Row23 = _mm256_mul_ps(r, Rhs.m_Row23);
		return LWSMatrix4(Row01, Row23);
	}

	friend LWSMatrix4 operator / (const LWSMatrix4<float> &Lhs, float Rhs) {
		__m256 r = _mm256_set1_ps(Rhs);
		__m256 Row01 = _mm256_div_ps(Lhs.m_Row01, r);
		__m256 Row23 = _mm256_div_ps(Lhs.m_Row23, r);
		return LWSMatrix4(Row01, Row23);
	}

	friend LWSMatrix4 operator / (float Lhs, const LWSMatrix4<float> &Rhs) {
		__m256 r = _mm256_set1_ps(Lhs);
		__m256 Row01 = _mm256_div_ps(r, Rhs.m_Row01);
		__m256 Row23 = _mm256_div_ps(r, Rhs.m_Row23);
		return LWSMatrix4(Row01, Row23);
	}

	friend LWSVector4<float> operator * (const LWSMatrix4<float> &Lhs, const LWSVector4<float> &Rhs) {
		__m256 Rxy = _mm256_set_m128(_mm_permute_ps(Rhs.m_Data, _MM_SHUFFLE(1, 1, 1, 1)), _mm_permute_ps(Rhs.m_Data, _MM_SHUFFLE(0, 0, 0, 0)));
		__m256 Rzw = _mm256_set_m128(_mm_permute_ps(Rhs.m_Data, _MM_SHUFFLE(3, 3, 3, 3)), _mm_permute_ps(Rhs.m_Data, _MM_SHUFFLE(2, 2, 2, 2)));

		/*
		LWSVector4<Type> Rx = Rhs.xxxx();
		LWSVector4<Type> Ry = Rhs.yyyy();
		LWSVector4<Type> Rz = Rhs.zzzz();
		LWSVector4<Type> Rw = Rhs.wwww();
		return Lhs.m_Rows[0] * Rx + Lhs.m_Rows[1] * Ry + Lhs.m_Rows[2] * Rz + Lhs.m_Rows[3] * Rw;
		*/
		alignas(32) float v[8];
		__m256 r = _mm256_mul_ps(Lhs.m_Row01, Rxy);
		r = _mm256_add_ps(r, _mm256_mul_ps(Lhs.m_Row23, Rzw));
		r = _mm256_add_ps(r, _mm256_permutevar8x32_ps(r, _mm256_set_epi32(3, 2, 1, 0, 7, 6, 5, 4)));
		_mm256_store_ps(v, r);
		std::cout << v[0] << " " << v[1] << " " << v[2] << " " << v[3] << " | " << v[4] << " " << v[5] << " " << v[6] << " " << v[7] << std::endl;

		return LWSVector4<float>(_mm256_extractf128_ps(r, 0));
	}

	friend LWSVector4<float> operator * (const LWSVector4<float> &Lhs, const LWSMatrix4<float> &Rhs) {
		__m256 Lxy = _mm256_set_m128(_mm_permute_ps(Lhs.m_Data, _MM_SHUFFLE(1, 1, 1, 1)), _mm_permute_ps(Lhs.m_Data, _MM_SHUFFLE(0, 0, 0, 0)));
		__m256 Lzw = _mm256_set_m128(_mm_permute_ps(Lhs.m_Data, _MM_SHUFFLE(3, 3, 3, 3)), _mm_permute_ps(Lhs.m_Data, _MM_SHUFFLE(2, 2, 2, 2)));
		__m256 r = _mm256_mul_ps(Rhs.m_Row01, Lxy);
		r = _mm256_add_ps(r, _mm256_mul_ps(Rhs.m_Row23, Lzw));
		r = _mm256_hadd_ps(r, r);
		return LWSVector4<float>(_mm256_extractf128_ps(r, 0));
	}

	static LWSMatrix4 RotationX(float Theta) {
		float S = sinf(Theta);
		float C = cosf(Theta);
		__m256 Row01 = _mm256_set_ps(0.0f, -S, C, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);
		__m256 Row23 = _mm256_set_ps(1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, C, S, 0.0f);
		return LWSMatrix4(Row01, Row23);
	}

	static LWSMatrix4 RotationY(float Theta) {
		float S = sinf(Theta);
		float C = cosf(Theta);
		__m256 Row01 = _mm256_set_ps(0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, S, 0.0f, C);
		__m256 Row23 = _mm256_set_ps(1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, C, 0.0f,-S);
		return LWSMatrix4(Row01, Row23);
	}

	static LWSMatrix4 RotationZ(float Theta) {
		float S = sinf(Theta);
		float C = cosf(Theta);
		__m256 Row01 = _mm256_set_ps(0.0f, 0.0f, C, S,
			0.0f, 0.0f, -S, C);
		__m256 Row23 = _mm256_set_ps(1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f);
		return LWSMatrix4(Row01, Row23);
	}

	static LWSMatrix4 RotationXY(float ThetaX, float ThetaY) {
		return RotationX(ThetaX)*RotationY(ThetaY);
	}

	static LWSMatrix4 RotationXZ(float ThetaX, float ThetaZ) {
		return RotationX(ThetaX)*RotationZ(ThetaZ);
	}

	static LWSMatrix4 RotationYX(float ThetaY, float ThetaX) {
		return RotationY(ThetaY)*RotationX(ThetaX);
	}

	static LWSMatrix4 RotationYZ(float ThetaY, float ThetaZ) {
		return RotationY(ThetaY)*RotationZ(ThetaZ);
	}

	static LWSMatrix4 RotationZX(float ThetaZ, float ThetaX) {
		return RotationZ(ThetaZ)*RotationX(ThetaX);
	}

	static LWSMatrix4 RotationZY(float ThetaZ, float ThetaY) {
		return RotationZ(ThetaZ)*RotationY(ThetaY);
	}

	static LWSMatrix4 RotationXYZ(float ThetaX, float ThetaY, float ThetaZ) {
		return RotationX(ThetaX)*RotationY(ThetaY)*RotationZ(ThetaZ);
	}

	static LWSMatrix4 RotationXZY(float ThetaX, float ThetaZ, float ThetaY) {
		return RotationX(ThetaX)*RotationZ(ThetaZ)*RotationY(ThetaY);
	}

	static LWSMatrix4 RotationYXZ(float ThetaY, float ThetaX, float ThetaZ) {
		return RotationY(ThetaY)*RotationX(ThetaX)*RotationZ(ThetaZ);
	}

	static LWSMatrix4 RotationYZX(float ThetaY, float ThetaZ, float ThetaX) {
		return RotationY(ThetaY)*RotationZ(ThetaZ)*RotationX(ThetaX);
	}

	static LWSMatrix4 RotationZXY(float ThetaZ, float ThetaX, float ThetaY) {
		return RotationZ(ThetaZ)*RotationX(ThetaX)*RotationY(ThetaY);
	}

	static LWSMatrix4 RotationZYX(float ThetaZ, float ThetaY, float ThetaX) {
		return RotationZ(ThetaZ)*RotationY(ThetaY)*RotationX(ThetaX);
	}

	static LWSMatrix4 Translation(float x, float y, float z) {
		__m256 Row01 = _mm256_set_ps(1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f);
		__m256 Row23 = _mm256_set_ps(0.0f, 0.0f, 1.0f, 0.0f,
			x, y, z, 1.0f);
		return LWSMatrix4(Row01, Row23);
	}

	static LWSMatrix4 Translation(const LWSVector4<float> &Position) {
		__m128 C = _mm_set_ps(0.0f, 0.0f, 1.0f, 0.0f);
		__m256 Row01 = _mm256_set_ps(1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f);
		__m256 Row23 = _mm256_set_m128(Position.m_Data, C);
		return LWSMatrix4(Row01, Row23);
	}

	static LWSMatrix4 Rotation(const LWSVector4<float> &Direction, const LWSVector4<float> &Up) {
		LWSVector4<float> xAxis = Up.Cross3(Direction);
		LWSVector4<float> yAxis = Direction.Cross3(xAxis);
		__m128 D = _mm_set_ps(0.0f, 0.0f, 0.0f, 1.0f);
		__m256 Row01 = _mm256_set_m128(yAxis.m_Data, xAxis.m_Data);
		__m256 Row23 = _mm256_set_m128(D, Direction.m_Data);
		return LWSMatrix4(Row01, Row23);
	}

	static LWSMatrix4 Perspective(float FoV, float Aspect, float Near, float Far) {
		float F = 1.0f / tanf(FoV *0.5f);
		__m256 Row01 = _mm256_set_ps(F / Aspect, 0.0f, 0.0f, 0.0f,
			0.0f, F, 0.0f, 0.0f);
		__m256 Row23 = _mm256_set_ps(0.0f, 0.0f, (Far + Near) / (Near - Far), -1.0f,
			0.0f, 0.0f, 2.0f*Far*Near / (Near - Far), 0.0f);
		return LWSMatrix4(Row01, Row23);
	}

	static LWSMatrix4 OrthoDX(float Left, float Right, float Bottom, float Top, float Near, float Far) {
		float sDepth = 1.0f / (Far - Near);
		__m256 Row01 = _mm256_set_ps(2.0f / (Right - Left), 0.0f, 0.0f, 0.0f,
			0.0f, 2.0f / (Top - Bottom), 0.0f, 0.0f);
		__m256 Row23 = _mm256_set_ps(0.0f, 0.0f, -sDepth, 0.0f,
			-(Right + Left) / (Right - Left), -(Top + Bottom) / (Top - Bottom), Near*sDepth, 1.0f);
		return LWSMatrix4(Row01, Row23);
	}

	static LWSMatrix4 OrthoGL(float Left, float Right, float Bottom, float Top, float Near, float Far) {
		__m256 Row01 = _mm256_set_ps(2.0f / (Right - Left), 0.0f, 0.0f, 0.0f,
			0.0f, 2.0f / (Top - Bottom), 0.0f, 0.0f);
		__m256 Row23 = _mm256_set_ps(0.0f, 0.0f, -2.0f / (Far - Near), 0.0f,
			-(Right + Left) / (Right - Left), -(Top + Bottom) / (Top - Bottom), -(Near + Far) / (Far - Near), 1.0f);
		return LWSMatrix4(Row01, Row23);
	}

	static LWSMatrix4 Ortho(float Left, float Right, float Bottom, float Top, float Near, float Far) {
		if (LWMatrix4_UseDXOrtho) return OrthoDX(Left, Right, Bottom, Top, Near, Far);
		return OrthoGL(Left, Right, Bottom, Top, Near, Far);
	}

	static LWSMatrix4 Frustum(float Left, float Right, float Bottom, float Top, float Near, float Far) {
		__m256 Row01 = _mm256_set_ps(2.0f*Near / (Right - Left), 0.0f, (Right + Left) / (Right - Left), 0.0f,
			0.0f, 2.0f*Near / (Top - Bottom), (Top + Bottom) / (Top - Bottom), 0.0f);
		__m256 Row23 = _mm256_set_ps(0.0f, 0.0f, -(Far + Near) / (Far - Near), -1.0f,
			0.0f, 0.0f, -2.0f*Far*Near / (Far - Near), 0.0f);
		return LWSMatrix4(Row01, Row23);
	}

	static LWSMatrix4 LookAt(const LWSVector4<float> &Position, const LWSVector4<float> &Target, const LWSVector4<float> &Up) {
		LWSVector4<float> Fwrd = (Target - Position).Normalize3();
		LWSVector4<float> Rght = Fwrd.Cross3(Up).Normalize3();
		LWSVector4<float> U = Rght.Cross3(Fwrd);
		return LWSMatrix4(Rght, U, Fwrd, Position);
	}

	LWSMatrix4(const LWSQuaternion<float> &Q) {
		__m128 vq = Q.m_Data;

		__m128 yxxx = _mm_permute_ps(vq, _MM_SHUFFLE(0, 0, 0, 1));
		__m128 zzyx = _mm_permute_ps(vq, _MM_SHUFFLE(0, 1, 2, 2));
		__m128 xxyx = _mm_permute_ps(vq, _MM_SHUFFLE(0, 0, 1, 0));
		__m128 zyzx = _mm_permute_ps(vq, _MM_SHUFFLE(0, 2, 1, 2));
		__m128 yzxx = _mm_permute_ps(vq, _MM_SHUFFLE(0, 0, 2, 1));
		__m128 wwwx = _mm_permute_ps(vq, _MM_SHUFFLE(0, 3, 3, 3));

		__m128 yy_xx_xx_xx = _mm_mul_ps(yxxx, yxxx);
		__m128 zz_zz_yy_xx = _mm_mul_ps(zzyx, zzyx);

		__m128 xz_xy_yz_xx = _mm_mul_ps(xxyx, zyzx);
		__m128 yw_zw_xw_xx = _mm_mul_ps(yzxx, wwwx);

		__m128 One = _mm_set_ps(1.0f, 1.0f, 1.0f, 0.0f);
		__m128 Two = _mm_set_ps(2.0f, 2.0f, 2.0f, 0.0f);

		__m128 A = _mm_sub_ps(One, _mm_mul_ps(Two, _mm_add_ps(yy_xx_xx_xx, zz_zz_yy_xx)));
		__m128 B = _mm_mul_ps(Two, _mm_add_ps(xz_xy_yz_xx, yw_zw_xw_xx));
		__m128 C = _mm_mul_ps(Two, _mm_sub_ps(xz_xy_yz_xx, yw_zw_xw_xx));

		__m128 Bxxxx = _mm_permute_ps(B, _MM_SHUFFLE(0, 0, 0, 0));
		__m128 Byyww = _mm_permute_ps(B, _MM_SHUFFLE(3, 3, 1, 1));
		__m128 Czzzz = _mm_permute_ps(C, _MM_SHUFFLE(2, 2, 2, 2));

		__m128 Row0 = _mm_blend_ps(_mm_blend_ps(A, C, 0x2), Bxxxx, 0x4);
		__m128 Row1 = _mm_blend_ps(_mm_blend_ps(Byyww, A, 0x2), C, 0x4);
		__m128 Row2 = _mm_blend_ps(_mm_blend_ps(C, Czzzz, 0x2), A, 0x4);
		__m128 Row3 = _mm_set_ps(0.0f, 0.0f, 0.0f, 1.0f);
		m_Row01 = _mm256_set_m128(Row1, Row0);
		m_Row23 = _mm256_set_m128(Row3, Row2);
	}

	LWSMatrix4(__m256 Row01, __m256 Row23) : m_Row01(Row01), m_Row23(Row23) {}

	LWSMatrix4(float xScale = 1, float yScale = 1, float zScale = 1, float wScale = 1) : m_Row01(_mm256_set_ps(0.0f, 0.0f, yScale, 0.0f, 0.0f, 0.0f, 0.0f, xScale)), m_Row23(_mm256_set_ps(wScale, 0.0f, 0.0f, 0.0f, 0.0f, zScale, 0.0f, 0.0f)) {}

	LWSMatrix4(const LWSVector4<float> &RowA, const LWSVector4<float> &RowB, const LWSVector4<float> &RowC, const LWSVector4<float> &RowD) : m_Row01(_mm256_set_m128(RowB.m_Data, RowA.m_Data)), m_Row23(_mm256_set_m128(RowD.m_Data, RowC.m_Data)) {}

	
	LWSMatrix4(const LWSVector4<float> &Scale, const LWSQuaternion<float> &Rotation, const LWSVector4<float> &Pos) {
		__m128 vq = Rotation.m_Data;

		__m128 yxxx = _mm_permute_ps(vq, _MM_SHUFFLE(0, 0, 0, 1));
		__m128 zzyx = _mm_permute_ps(vq, _MM_SHUFFLE(0, 1, 2, 2));
		__m128 xxyx = _mm_permute_ps(vq, _MM_SHUFFLE(0, 0, 1, 0));
		__m128 zyzx = _mm_permute_ps(vq, _MM_SHUFFLE(0, 2, 1, 2));
		__m128 yzxx = _mm_permute_ps(vq, _MM_SHUFFLE(0, 0, 2, 1));
		__m128 wwwx = _mm_permute_ps(vq, _MM_SHUFFLE(0, 3, 3, 3));

		__m128 yy_xx_xx_xx = _mm_mul_ps(yxxx, yxxx);
		__m128 zz_zz_yy_xx = _mm_mul_ps(zzyx, zzyx);

		__m128 xz_xy_yz_xx = _mm_mul_ps(xxyx, zyzx);
		__m128 yw_zw_xw_xx = _mm_mul_ps(yzxx, wwwx);

		__m128 One = _mm_set_ps(1.0f, 1.0f, 1.0f, 0.0f);
		__m128 Two = _mm_set_ps(2.0f, 2.0f, 2.0f, 0.0f);

		__m128 A = _mm_sub_ps(One, _mm_mul_ps(Two, _mm_add_ps(yy_xx_xx_xx, zz_zz_yy_xx)));
		__m128 B = _mm_mul_ps(Two, _mm_add_ps(xz_xy_yz_xx, yw_zw_xw_xx));
		__m128 C = _mm_mul_ps(Two, _mm_sub_ps(xz_xy_yz_xx, yw_zw_xw_xx));

		__m128 Bxxxx = _mm_permute_ps(B, _MM_SHUFFLE(0, 0, 0, 0));
		__m128 Byyww = _mm_permute_ps(B, _MM_SHUFFLE(3, 3, 1, 1));
		__m128 Czzzz = _mm_permute_ps(C, _MM_SHUFFLE(2, 2, 2, 2));

		__m128 Row0 = _mm_mul_ps(_mm_blend_ps(_mm_blend_ps(A, C, 0x2), Bxxxx, 0x4), Scale.xxxw().m_Data);
		__m128 Row1 = _mm_mul_ps(_mm_blend_ps(_mm_blend_ps(Byyww, A, 0x2), C, 0x4), Scale.yyyw().m_Data);
		__m128 Row2 = _mm_mul_ps(_mm_blend_ps(_mm_blend_ps(C, Czzzz, 0x2), A, 0x4), Scale.zzzw().m_Data);
		__m128 Row3 = Pos.m_Data;
		m_Row01 = _mm256_set_m128(Row1, Row0);
		m_Row23 = _mm256_set_m128(Row3, Row2);
	};
};

#endif