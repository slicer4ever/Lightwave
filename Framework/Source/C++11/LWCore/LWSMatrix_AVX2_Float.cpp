#include "LWCore/LWSMatrix.h"
#include "LWCore/LWSQuaternion.h"
#ifdef __AVX2__

LWMatrix4<float> LWSMatrix4<float>::AsMat4(void) const {
	alignas(64) LWMatrix4<float> R;
	_mm256_store_ps(&R.m_Rows[0].x, m_Row01);
	_mm256_store_ps(&R.m_Rows[2].x, m_Row23);
	return R;
}

LWSVector4<float> LWSMatrix4<float>::DecomposeScale(bool doTranspose3x3) const {
	LWSMatrix4<float> v = doTranspose3x3 ? Transpose3x3() : *this;
	__m128 R0 = _mm256_extractf128_ps(v.m_Row01, 0);
	__m128 R1 = _mm256_extractf128_ps(v.m_Row01, 1);
	__m128 R2 = _mm256_extractf128_ps(v.m_Row23, 0);

	//Calculate r0 length.
	R0 = _mm_dp_ps(R0, R0, 0x7F);
	R0 = _mm_sqrt_ps(R0);

	//Calculate r1 length.
	R1 = _mm_dp_ps(R1, R1, 0x7F);
	R1 = _mm_sqrt_ps(R1);

	//Calculate r2 length.
	R2 = _mm_dp_ps(R2, R2, 0x7F);
	R2 = _mm_sqrt_ps(R2);

	__m128 Res = _mm_blend_ps(R0, R1, 0x2);
	Res = _mm_blend_ps(Res, R2, 0x4);
	Res = _mm_blend_ps(Res, _mm_set_ps1(1.0f), 0x8);
	return LWSVector4<float>(Res);
};

void LWSMatrix4<float>::Decompose(LWSVector4<float> &Scale, LWSQuaternion<float> &Rotation, LWSVector4<float> &Translation, bool doTranspose3x3) const {
	LWSMatrix4<float> v = doTranspose3x3 ? Transpose3x3() : *this;
	__m128 R0 = _mm256_extractf128_ps(v.m_Row01, 0);
	__m128 R1 = _mm256_extractf128_ps(v.m_Row01, 1);
	__m128 R2 = _mm256_extractf128_ps(v.m_Row23, 0);
	__m128 R3 = _mm256_extractf128_ps(v.m_Row23, 1);

	//Row0 Length.
	__m128 sR0 = _mm_dp_ps(R0, R0, 0x7F);
	sR0 = _mm_sqrt_ps(sR0);

	//Row1 Length.
	__m128 sR1 = _mm_dp_ps(R1, R1, 0x7F);
	sR1 = _mm_sqrt_ps(sR1);

	__m128 sR2 = _mm_dp_ps(R2, R2, 0x7F);
	sR2 = _mm_sqrt_ps(sR2);

	__m128 S = _mm_blend_ps(sR0, sR1, 0x2);
	S = _mm_blend_ps(S, sR2, 0x4);
	S = _mm_blend_ps(S, _mm_set_ps1(1.0f), 0x8);
	Scale = LWSVector4<float>(S);

	__m128 iScale = _mm_div_ps(_mm_set_ps1(1.0f), S);
	__m128 xScale = _mm_permute_ps(iScale, _MM_SHUFFLE(0, 0, 0, 0));
	__m128 yScale = _mm_permute_ps(iScale, _MM_SHUFFLE(1, 1, 1, 1));
	__m128 zScale = _mm_permute_ps(iScale, _MM_SHUFFLE(2, 2, 2, 2));
	R0 = _mm_mul_ps(R0, xScale);
	R1 = _mm_mul_ps(R1, yScale);
	R2 = _mm_mul_ps(R2, zScale);
	v = LWSMatrix4<float>(LWSVector4<float>(R0), LWSVector4<float>(R1), LWSVector4<float>(R2), LWSVector4<float>(R3));
	Rotation = LWSQuaternion<float>(v);
	Translation = LWSVector4<float>(R3);
	return;
}

LWSMatrix4<float> LWSMatrix4<float>::TransformInverse(void) const {

	__m128 E = _mm_set_ps1(std::numeric_limits<float>::epsilon());
	__m128 One = _mm_set_ps(0.0f, 1.0f, 1.0f, 1.0f);
	//Transpose3x3 matrix.
	LWSMatrix4<float> T3 = Transpose3x3();
	__m128 A = _mm256_extractf128_ps(T3.m_Row01, 0);
	__m128 B = _mm256_extractf128_ps(T3.m_Row01, 1);
	__m128 C = _mm256_extractf128_ps(T3.m_Row23, 0);
	__m128 D = _mm256_extractf128_ps(T3.m_Row23, 1);

	__m128 Sq = _mm_add_ps(_mm_add_ps(_mm_mul_ps(A, A), _mm_mul_ps(B, B)), _mm_mul_ps(C, C));
	__m128 rSq = _mm_div_ps(One, Sq);
	rSq = _mm_blendv_ps(rSq, One, _mm_cmple_ps(Sq, E));

	A = _mm_mul_ps(rSq, A);
	B = _mm_mul_ps(rSq, B);
	C = _mm_mul_ps(rSq, C);
	__m128 Dx = _mm_permute_ps(D, _MM_SHUFFLE(3, 0, 0, 0));
	__m128 Dy = _mm_permute_ps(D, _MM_SHUFFLE(3, 1, 1, 1));
	__m128 Dz = _mm_permute_ps(D, _MM_SHUFFLE(3, 2, 2, 2));
	__m128 Dr = _mm_add_ps(_mm_mul_ps(Dx, A), _mm_mul_ps(Dy, B));
	Dr = _mm_add_ps(Dr, _mm_mul_ps(Dz, C));
	Dr = _mm_xor_ps(_mm_set_ps1(-0.0), Dr); //Negate equation.
	Dr = _mm_blend_ps(Dr, D, 0x8); //Keep w component of original Row3.
	__m256 Row01 = _mm256_set_m128(B, A);
	__m256 Row23 = _mm256_set_m128(Dr, C);
	return LWSMatrix4(Row01, Row23);
}

LWSMatrix4<float> LWSMatrix4<float>::Inverse(void) const {
	//adapted Non-simd version Found from: https://stackoverflow.com/questions/1148309/inverting-a-4x4-matrix
	__m128 e = _mm_set_ps1(std::numeric_limits<float>::epsilon());
	__m128 Zero = _mm_setzero_ps();
	__m128 One = _mm_set_ps1(1.0f);

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

	__m128 Axxxx = _mm_permute_ps(A, _MM_SHUFFLE(0, 0, 0, 0));
	__m128 Ayyyy = _mm_permute_ps(A, _MM_SHUFFLE(1, 1, 1, 1));
	__m128 Azzzz = _mm_permute_ps(A, _MM_SHUFFLE(2, 2, 2, 2));
	__m128 Awwww = _mm_permute_ps(A, _MM_SHUFFLE(3, 3, 3, 3));

	__m128 Bxxxx = _mm_permute_ps(B, _MM_SHUFFLE(0, 0, 0, 0));
	__m128 Byyyy = _mm_permute_ps(B, _MM_SHUFFLE(1, 1, 1, 1));
	__m128 Bzzzz = _mm_permute_ps(B, _MM_SHUFFLE(2, 2, 2, 2));
	__m128 Bwwww = _mm_permute_ps(B, _MM_SHUFFLE(3, 3, 3, 3));

	__m128 Cxxxx = _mm_permute_ps(C, _MM_SHUFFLE(0, 0, 0, 0));
	__m128 Cyyyy = _mm_permute_ps(C, _MM_SHUFFLE(1, 1, 1, 1));
	__m128 Czzzz = _mm_permute_ps(C, _MM_SHUFFLE(2, 2, 2, 2));
	__m128 Cwwww = _mm_permute_ps(C, _MM_SHUFFLE(3, 3, 3, 3));

	__m128 Dxxxx = _mm_permute_ps(D, _MM_SHUFFLE(0, 0, 0, 0));
	__m128 Dyyyy = _mm_permute_ps(D, _MM_SHUFFLE(1, 1, 1, 1));
	__m128 Dzzzz = _mm_permute_ps(D, _MM_SHUFFLE(2, 2, 2, 2));
	__m128 Dwwww = _mm_permute_ps(D, _MM_SHUFFLE(3, 3, 3, 3));

	__m128 CzzBzz = _mm_blend_ps(Czzzz, Bzzzz, 0xC);
	__m128 DwwwCw = _mm_blend_ps(Dwwww, Cwwww, 0x8);
	__m128 CwwBww = _mm_blend_ps(Cwwww, Bwwww, 0xC);
	__m128 DzzzCz = _mm_blend_ps(Dzzzz, Czzzz, 0x8);
	__m128 CyyByy = _mm_blend_ps(Cyyyy, Byyyy, 0xC);
	__m128 DyyyCy = _mm_blend_ps(Dyyyy, Cyyyy, 0x8);
	__m128 CxxBxx = _mm_blend_ps(Cxxxx, Bxxxx, 0xC);
	__m128 DxxxCx = _mm_blend_ps(Dxxxx, Cxxxx, 0x8);
	
	__m128 A2323_A2323_A1323_A1223 = _mm_sub_ps(_mm_mul_ps(Czzyy, Dwwwz), _mm_mul_ps(Cwwwz, Dzzyy));
	__m128 A1323_A0323_A0323_A0223 = _mm_sub_ps(_mm_mul_ps(Cyxxx, Dwwwz), _mm_mul_ps(Cwwwz, Dyxxx));
	__m128 A1223_A0223_A0123_A0123 = _mm_sub_ps(_mm_mul_ps(Cyxxx, Dzzyy), _mm_mul_ps(Czzyy, Dyxxx));
	
	__m128 A2323_A2323_A2313_A2312 = _mm_sub_ps(_mm_mul_ps(CzzBzz, DwwwCw), _mm_mul_ps(CwwBww, DzzzCz));
	__m128 A1323_A1323_A1313_A1312 = _mm_sub_ps(_mm_mul_ps(CyyByy, DwwwCw), _mm_mul_ps(CwwBww, DyyyCy));
	__m128 A1223_A1223_A1213_A1212 = _mm_sub_ps(_mm_mul_ps(CyyByy, DzzzCz), _mm_mul_ps(CzzBzz, DyyyCy));
	__m128 A0323_A0323_A0313_A0312 = _mm_sub_ps(_mm_mul_ps(CxxBxx, DwwwCw), _mm_mul_ps(CwwBww, DxxxCx));
	__m128 A0223_A0223_A0213_A0212 = _mm_sub_ps(_mm_mul_ps(CxxBxx, DzzzCz), _mm_mul_ps(CzzBzz, DxxxCx));
	__m128 A0123_A0123_A0113_A0112 = _mm_sub_ps(_mm_mul_ps(CxxBxx, DyyyCy), _mm_mul_ps(CyyByy, DxxxCx));
	
	__m128 NegA = _mm_set_ps(0.0f, -0.0f, 0.0f, -0.0f);
	__m128 NegB = _mm_set_ps(-0.0f, 0.0f, -0.0f, 0.0f);

	__m128 Byxxx = _mm_permute_ps(B, _MM_SHUFFLE(0, 0, 0, 1));
	__m128 Bzzyy = _mm_permute_ps(B, _MM_SHUFFLE(1, 1, 2, 2));
	__m128 Bwwwy = _mm_permute_ps(B, _MM_SHUFFLE(2, 3, 3, 3));

	__m128 PtA = _mm_mul_ps(Byxxx, A2323_A2323_A1323_A1223);
	__m128 PtB = _mm_mul_ps(Bzzyy, A1323_A0323_A0323_A0223);
	__m128 Ptc = _mm_mul_ps(Bwwwy, A1223_A0223_A0123_A0123);
	__m128 Det = _mm_xor_ps(_mm_mul_ps(_mm_add_ps(_mm_sub_ps(PtA, PtB), Ptc), A), NegA);
	Det = _mm_hadd_ps(Det, Det);
	Det = _mm_hadd_ps(Det, Det);
	//std::cout << "Det: " << LWSVector4<float>(Det) << std::endl;
	__m128 rDet = _mm_div_ps(One, Det);
	rDet = _mm_blendv_ps(rDet, Zero, _mm_cmple_ps(_mm_andnot_ps(_mm_set_ps1(-0.0f), Det), e));

	__m128 ByAyyy = _mm_blend_ps(Byyyy, Ayyyy, 0xE);
	__m128 BzAzzz = _mm_blend_ps(Bzzzz, Azzzz, 0xE);
	__m128 BwAwww = _mm_blend_ps(Bwwww, Awwww, 0xE);
	__m128 BxAxxx = _mm_blend_ps(Bxxxx, Axxxx, 0xE);

	A = _mm_mul_ps(rDet, _mm_xor_ps(NegA, _mm_add_ps(_mm_sub_ps(_mm_mul_ps(ByAyyy, A2323_A2323_A2313_A2312), _mm_mul_ps(BzAzzz, A1323_A1323_A1313_A1312)), _mm_mul_ps(BwAwww, A1223_A1223_A1213_A1212))));
	//A = (ByAyyy * A2323_A2323_A2313_A2312 - BzAzzz * A1323_A1323_A1313_A1312 + BwAwww * A1223_A1223_A1213_A1212) * MulA * Det;
	B = _mm_mul_ps(rDet, _mm_xor_ps(NegB, _mm_add_ps(_mm_sub_ps(_mm_mul_ps(BxAxxx, A2323_A2323_A2313_A2312), _mm_mul_ps(BzAzzz, A0323_A0323_A0313_A0312)), _mm_mul_ps(BwAwww, A0223_A0223_A0213_A0212))));
	//B = (ByAxxx * A2323_A2323_A2313_A2312 - BzAzzz * A0323_A0323_A0313_A0312 + BwAwww * A0223_A0223_A0213_A0212) * MulB * Det;
	C = _mm_mul_ps(rDet, _mm_xor_ps(NegA, _mm_add_ps(_mm_sub_ps(_mm_mul_ps(BxAxxx, A1323_A1323_A1313_A1312), _mm_mul_ps(ByAyyy, A0323_A0323_A0313_A0312)), _mm_mul_ps(BwAwww, A0123_A0123_A0113_A0112))));
	//C = (BxAxxx * A1323_A1323_A1313_A1312 - ByAyyy * A0323_A0323_A0313_A0312 + BwAwww * A0123_A0123_A0113_A0112) * MulA * Det;
	D = _mm_mul_ps(rDet, _mm_xor_ps(NegB, _mm_add_ps(_mm_sub_ps(_mm_mul_ps(BxAxxx, A1223_A1223_A1213_A1212), _mm_mul_ps(ByAyyy, A0223_A0223_A0213_A0212)), _mm_mul_ps(BzAzzz, A0123_A0123_A0113_A0112))));
	//D = (BxAxxx * A1223_A1223_A1213_A1212 - ByAyyy * A0223_A0223_A0213_A0212 + BzAzzz * A0123_A0123_A0113_A0112) * MulB * Det;
	__m256 Row01 = _mm256_set_m128(B, A);
	__m256 Row23 = _mm256_set_m128(D, C);
	return LWSMatrix4(Row01, Row23);
}

LWSVector4<float> LWSMatrix4<float>::Column(uint32_t Index) const {
	//Transpose for column.
	LWSMatrix4<float> T = Transpose();
	__m128 R;
	if (Index == 0) R = _mm256_extractf128_ps(T.m_Row01, 0);
	else if (Index == 1) R = _mm256_extractf128_ps(T.m_Row01, 1);
	else if (Index == 2) R = _mm256_extractf128_ps(T.m_Row23, 0);
	else if (Index == 3) R = _mm256_extractf128_ps(T.m_Row23, 1);
	return LWSVector4<float>(R);
};

LWSMatrix4<float> LWSMatrix4<float>::Transpose(void) const {
	__m256 A = _mm256_unpacklo_ps(m_Row01, m_Row23);
	__m256 B = _mm256_unpackhi_ps(m_Row01, m_Row23);
	__m256 Row01 = _mm256_permutevar8x32_ps(A, _mm256_set_epi32(7, 3, 6, 2, 5, 1, 4, 0));
	__m256 Row23 = _mm256_permutevar8x32_ps(B, _mm256_set_epi32(7, 3, 6, 2, 5, 1, 4, 0));
	return LWSMatrix4(Row01, Row23);
}

LWSMatrix4<float> LWSMatrix4<float>::Transpose3x3(void) const {
	__m256 A = _mm256_unpacklo_ps(m_Row01, m_Row23);
	__m256 B = _mm256_unpackhi_ps(m_Row01, m_Row23);
	__m256 Row01 = _mm256_permutevar8x32_ps(A, _mm256_set_epi32(7, 3, 6, 2, 5, 1, 4, 0));
	__m256 Row23 = _mm256_permutevar8x32_ps(B, _mm256_set_epi32(7, 3, 6, 2, 5, 1, 4, 0));
	Row01 = _mm256_blend_ps(Row01, m_Row01, 0x88);
	Row23 = _mm256_blend_ps(Row23, m_Row23, 0xF8);
	return LWSMatrix4(Row01, Row23);
}

LWSMatrix4<float> LWSMatrix4<float>::Transpose2x2(void) const {
	__m256 A = _mm256_unpacklo_ps(m_Row01, m_Row23);
	__m256 Row01 = _mm256_permutevar8x32_ps(A, _mm256_set_epi32(7, 3, 6, 2, 5, 1, 4, 0));
	Row01 = _mm256_blend_ps(Row01, m_Row01, 0xCC);
	return LWSMatrix4(Row01, m_Row23);
}

float LWSMatrix4<float>::Determinant(void) const {
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

	__m128 Byxxx = _mm_permute_ps(B, _MM_SHUFFLE(0, 0, 0, 1));
	__m128 Bzzyy = _mm_permute_ps(B, _MM_SHUFFLE(1, 1, 2, 2));
	__m128 Bwwwy = _mm_permute_ps(B, _MM_SHUFFLE(2, 3, 3, 3));

	__m128 A2323_A2323_A1323_A1223 = _mm_sub_ps(_mm_mul_ps(Czzyy, Dwwwz), _mm_mul_ps(Cwwwz, Dzzyy));
	__m128 A1323_A0323_A0323_A0223 = _mm_sub_ps(_mm_mul_ps(Cyxxx, Dwwwz), _mm_mul_ps(Cwwwz, Dyxxx));
	__m128 A1223_A0223_A0123_A0123 = _mm_sub_ps(_mm_mul_ps(Cyxxx, Dzzyy), _mm_mul_ps(Czzyy, Dyxxx));

	__m128 Neg = _mm_set_ps(-0.0f, 0.0f, -0.0f, 0.0f);

	__m128 PtA = _mm_mul_ps(Byxxx, A2323_A2323_A1323_A1223);
	__m128 PtB = _mm_mul_ps(Bzzyy, A1323_A0323_A0323_A0223);
	__m128 Ptc = _mm_mul_ps(Bwwwy, A1223_A0223_A0123_A0123);
	__m128 r = _mm_xor_ps(_mm_mul_ps(_mm_add_ps(_mm_sub_ps(PtA, PtB), Ptc), A), Neg);
	r = _mm_hadd_ps(r, r);
	r = _mm_hadd_ps(r, r);
	return _mm_cvtss_f32(r);
}

LWSMatrix4<float>& LWSMatrix4<float>::operator = (const LWSMatrix4<float>& Rhs) {
	m_Row01 = Rhs.m_Row01;
	m_Row23 = Rhs.m_Row23;
	return *this;
}

LWSMatrix4<float>& LWSMatrix4<float>::operator+= (const LWSMatrix4<float>& Rhs) {
	m_Row01 = _mm256_add_ps(m_Row01, Rhs.m_Row01);
	m_Row23 = _mm256_add_ps(m_Row23, Rhs.m_Row23);
	return *this;
}

LWSMatrix4<float>& LWSMatrix4<float>::operator-= (const LWSMatrix4<float>& Rhs) {
	m_Row01 = _mm256_sub_ps(m_Row01, Rhs.m_Row01);
	m_Row23 = _mm256_sub_ps(m_Row23, Rhs.m_Row23);
	return *this;
}

LWSMatrix4<float>& LWSMatrix4<float>::operator*= (const LWSMatrix4<float>& Rhs) {
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

LWSVector4<float> LWSMatrix4<float>::operator[](uint32_t i) const {
	return m_Rows[i];
}

LWSVector4<float> &LWSMatrix4<float>::operator[](uint32_t i) {
	return m_Rows[i];
}

LWSMatrix4<float>& LWSMatrix4<float>::operator *=(float Rhs) {
	__m256 r = _mm256_set1_ps(Rhs);
	m_Row01 = _mm256_mul_ps(m_Row01, r);
	m_Row23 = _mm256_mul_ps(m_Row23, r);
	return *this;
}

LWSMatrix4<float>& LWSMatrix4<float>::operator /=(float Rhs) {
	__m256 r = _mm256_set1_ps(Rhs);
	m_Row01 = _mm256_div_ps(m_Row01, r);
	m_Row23 = _mm256_div_ps(m_Row23, r);
	return *this;
}

bool LWSMatrix4<float>::operator == (const LWSMatrix4<float>& Rhs) const {
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

bool LWSMatrix4<float>::operator != (const LWSMatrix4<float>& Rhs) const {
	return !(*this == Rhs);
}

std::ostream& operator<<(std::ostream& o, const LWSMatrix4<float>& M) {
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

LWSMatrix4<float> operator + (const LWSMatrix4<float>& Lhs, const LWSMatrix4<float>& Rhs) {
	__m256 Row01 = _mm256_add_ps(Lhs.m_Row01, Rhs.m_Row01);
	__m256 Row23 = _mm256_add_ps(Lhs.m_Row23, Rhs.m_Row23);
	return LWSMatrix4<float>(Row01, Row23);
}

LWSMatrix4<float> operator - (const LWSMatrix4<float>& Lhs, const LWSMatrix4<float>& Rhs) {
	__m256 Row01 = _mm256_sub_ps(Lhs.m_Row01, Rhs.m_Row01);
	__m256 Row23 = _mm256_sub_ps(Lhs.m_Row23, Rhs.m_Row23);
	return LWSMatrix4<float>(Row01, Row23);
}

LWSMatrix4<float> operator * (const LWSMatrix4<float>& Lhs, const LWSMatrix4<float>& Rhs) {
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
	return LWSMatrix4<float>(Row01, Row23);
}

LWSMatrix4<float> operator * (const LWSMatrix4<float>& Lhs, float Rhs) {
	__m256 r = _mm256_set1_ps(Rhs);
	__m256 Row01 = _mm256_mul_ps(Lhs.m_Row01, r);
	__m256 Row23 = _mm256_mul_ps(Lhs.m_Row23, r);
	return LWSMatrix4<float>(Row01, Row23);
}

LWSMatrix4<float> operator * (float Lhs, const LWSMatrix4<float>& Rhs) {
	__m256 r = _mm256_set1_ps(Lhs);
	__m256 Row01 = _mm256_mul_ps(r, Rhs.m_Row01);
	__m256 Row23 = _mm256_mul_ps(r, Rhs.m_Row23);
	return LWSMatrix4<float>(Row01, Row23);
}

LWSMatrix4<float> operator / (const LWSMatrix4<float>& Lhs, float Rhs) {
	__m256 r = _mm256_set1_ps(Rhs);
	__m256 Row01 = _mm256_div_ps(Lhs.m_Row01, r);
	__m256 Row23 = _mm256_div_ps(Lhs.m_Row23, r);
	return LWSMatrix4<float>(Row01, Row23);
}

LWSMatrix4<float> operator / (float Lhs, const LWSMatrix4<float>& Rhs) {
	__m256 r = _mm256_set1_ps(Lhs);
	__m256 Row01 = _mm256_div_ps(r, Rhs.m_Row01);
	__m256 Row23 = _mm256_div_ps(r, Rhs.m_Row23);
	return LWSMatrix4<float>(Row01, Row23);
}

LWSVector4<float> operator * (const LWSMatrix4<float>& Lhs, const LWSVector4<float>& Rhs) {
	__m256 Rxy = _mm256_set_m128(_mm_permute_ps(Rhs.m_Data, _MM_SHUFFLE(1, 1, 1, 1)), _mm_permute_ps(Rhs.m_Data, _MM_SHUFFLE(0, 0, 0, 0)));
	__m256 Rzw = _mm256_set_m128(_mm_permute_ps(Rhs.m_Data, _MM_SHUFFLE(3, 3, 3, 3)), _mm_permute_ps(Rhs.m_Data, _MM_SHUFFLE(2, 2, 2, 2)));

	__m256 A = _mm256_mul_ps(Lhs.m_Row01, Rxy);
	A = _mm256_add_ps(A, _mm256_permutevar8x32_ps(A, _mm256_set_epi32(3, 2, 1, 0, 7, 6, 5, 4)));
	__m256 B = _mm256_mul_ps(Lhs.m_Row23, Rzw);
	B = _mm256_add_ps(B, _mm256_permutevar8x32_ps(B, _mm256_set_epi32(3, 2, 1, 0, 7, 6, 5, 4)));
	A = _mm256_add_ps(A, B);
	return LWSVector4<float>(_mm256_extractf128_ps(A, 0));
}

LWSVector4<float> operator * (const LWSVector4<float>& Lhs, const LWSMatrix4<float>& Rhs) {
	__m256 Lxy = _mm256_set_m128(_mm_permute_ps(Lhs.m_Data, _MM_SHUFFLE(1, 1, 1, 1)), _mm_permute_ps(Lhs.m_Data, _MM_SHUFFLE(0, 0, 0, 0)));
	__m256 Lzw = _mm256_set_m128(_mm_permute_ps(Lhs.m_Data, _MM_SHUFFLE(3, 3, 3, 3)), _mm_permute_ps(Lhs.m_Data, _MM_SHUFFLE(2, 2, 2, 2)));
	__m256 r = _mm256_mul_ps(Lxy, Rhs.m_Row01);
	r = _mm256_add_ps(r, _mm256_mul_ps(Lzw, Rhs.m_Row23));
	r = _mm256_add_ps(r, _mm256_permutevar8x32_ps(r, _mm256_set_epi32(3, 2, 1, 0, 7, 6, 5, 4)));
	return LWSVector4<float>(_mm256_extractf128_ps(r, 0));
}

LWSMatrix4<float> LWSMatrix4<float>::FromEuler(float Pitch, float Yaw, float Roll) {
	float c1 = cosf(Yaw);
	float c2 = cosf(Pitch);
	float c3 = cosf(Roll);
	float s1 = sinf(Yaw);
	float s2 = sinf(Pitch);
	float s3 = sinf(Roll);
	float s1s2 = s1 * s2;
	return LWSMatrix4<float>({ c1 * c2, s1 * s3 - c1 * s2 * c3, c1 * s2 * s3 + s1 * c3, 0.0 },
		{ s2, c2 * c3, -c2 * s3, 0.0 },
		{ -s1 * c2, s1s2 * c3 + c1 * s3, -s1s2 * s3 + c1 * c3, 0.0 },
		{ 0.0, 0.0, 0.0, 1.0 });
}

LWSMatrix4<float> LWSMatrix4<float>::FromEuler(const LWVector3<float> &Euler) {
	return FromEuler(Euler.x, Euler.y, Euler.z);
}

LWVector3<float> LWSMatrix4<float>::ToEuler(void) const {
	const float e = std::numeric_limits<float>::epsilon();
	if (m_Rows[1].x > 1.0f - e) {
		return LWVector3<float>(LW_PI_2, atan2f(m_Rows[0].z, m_Rows[2].z), 0.0);
	} else if (m_Rows[1].x < -1.0f + e) {
		return LWVector3<float>(-LW_PI_2, atan2f(m_Rows[0].z, m_Rows[2].z), 0.0);
	}
	return LWVector3<float>(asinf(m_Rows[1].x), atan2f(-m_Rows[2].x, m_Rows[0].x), atan2f(-m_Rows[1].z, m_Rows[1].y));
}

LWSMatrix4<float> LWSMatrix4<float>::RotationX(float Theta) {
	float S = sinf(Theta);
	float C = cosf(Theta);
	__m256 Row01 = _mm256_set_ps(0.0f, -S, C, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
	__m256 Row23 = _mm256_set_ps(1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, C, S, 0.0f);
	return LWSMatrix4<float>(Row01, Row23);
}

LWSMatrix4<float> LWSMatrix4<float>::RotationY(float Theta) {
	float S = sinf(Theta);
	float C = cosf(Theta);
	__m256 Row01 = _mm256_set_ps(0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, S, 0.0f, C);
	__m256 Row23 = _mm256_set_ps(1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, C, 0.0f, -S);
	return LWSMatrix4<float>(Row01, Row23);
}

LWSMatrix4<float> LWSMatrix4<float>::RotationZ(float Theta) {
	float S = sinf(Theta);
	float C = cosf(Theta);
	__m256 Row01 = _mm256_set_ps(0.0f, 0.0f, C, S,
		0.0f, 0.0f, -S, C);
	__m256 Row23 = _mm256_set_ps(1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f);
	return LWSMatrix4<float>(Row01, Row23);
}

LWSMatrix4<float> LWSMatrix4<float>::RotationXY(float ThetaX, float ThetaY) {
	return RotationX(ThetaX) * RotationY(ThetaY);
}

LWSMatrix4<float> LWSMatrix4<float>::RotationXZ(float ThetaX, float ThetaZ) {
	return RotationX(ThetaX) * RotationZ(ThetaZ);
}

LWSMatrix4<float> LWSMatrix4<float>::RotationYX(float ThetaY, float ThetaX) {
	return RotationY(ThetaY) * RotationX(ThetaX);
}

LWSMatrix4<float> LWSMatrix4<float>::RotationYZ(float ThetaY, float ThetaZ) {
	return RotationY(ThetaY) * RotationZ(ThetaZ);
}

LWSMatrix4<float> LWSMatrix4<float>::RotationZX(float ThetaZ, float ThetaX) {
	return RotationZ(ThetaZ) * RotationX(ThetaX);
}

LWSMatrix4<float> LWSMatrix4<float>::RotationZY(float ThetaZ, float ThetaY) {
	return RotationZ(ThetaZ) * RotationY(ThetaY);
}

LWSMatrix4<float> LWSMatrix4<float>::RotationXYZ(float ThetaX, float ThetaY, float ThetaZ) {
	return RotationX(ThetaX) * RotationY(ThetaY) * RotationZ(ThetaZ);
}

LWSMatrix4<float> LWSMatrix4<float>::RotationXZY(float ThetaX, float ThetaZ, float ThetaY) {
	return RotationX(ThetaX) * RotationZ(ThetaZ) * RotationY(ThetaY);
}

LWSMatrix4<float> LWSMatrix4<float>::RotationYXZ(float ThetaY, float ThetaX, float ThetaZ) {
	return RotationY(ThetaY) * RotationX(ThetaX) * RotationZ(ThetaZ);
}

LWSMatrix4<float> LWSMatrix4<float>::RotationYZX(float ThetaY, float ThetaZ, float ThetaX) {
	return RotationY(ThetaY) * RotationZ(ThetaZ) * RotationX(ThetaX);
}

LWSMatrix4<float> LWSMatrix4<float>::RotationZXY(float ThetaZ, float ThetaX, float ThetaY) {
	return RotationZ(ThetaZ) * RotationX(ThetaX) * RotationY(ThetaY);
}

LWSMatrix4<float> LWSMatrix4<float>::RotationZYX(float ThetaZ, float ThetaY, float ThetaX) {
	return RotationZ(ThetaZ) * RotationY(ThetaY) * RotationX(ThetaX);
}

LWSMatrix4<float> LWSMatrix4<float>::Translation(float x, float y, float z) {
	__m256 Row01 = _mm256_set_ps(0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
	__m256 Row23 = _mm256_set_ps(1.0f, z, y, x,
		0.0f, 1.0f, 0.0f, 0.0f);
	return LWSMatrix4(Row01, Row23);
}

LWSMatrix4<float> LWSMatrix4<float>::Translation(const LWSVector4<float>& Position) {
	__m128 C = _mm_set_ps(0.0f, 1.0f, 0.0f, 0.0f);
	__m256 Row01 = _mm256_set_ps(0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
	__m256 Row23 = _mm256_set_m128(_mm_blend_ps(Position.m_Data, _mm_set_ps1(1.0f), 0x8), C);
	return LWSMatrix4(Row01, Row23);
}

LWSMatrix4<float> LWSMatrix4<float>::Rotation(const LWSVector4<float>& Direction, const LWSVector4<float>& Up) {
	LWSVector4<float> xAxis = Up.Cross3(Direction);
	LWSVector4<float> yAxis = Direction.Cross3(xAxis);
	__m128 D = _mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f);
	__m256 Row01 = _mm256_set_m128(yAxis.m_Data, xAxis.m_Data);
	__m256 Row23 = _mm256_set_m128(D, Direction.m_Data);
	return LWSMatrix4(Row01, Row23);
}

LWSMatrix4<float> LWSMatrix4<float>::Perspective(float FoV, float Aspect, float Near, float Far) {
	float F = 1.0f / tanf(FoV * 0.5f);
	__m256 Row01 = _mm256_set_ps(0.0f, 0.0f, F, 0.0f,
		0.0f, 0.0f, 0.0f, F / Aspect);
	__m256 Row23 = _mm256_set_ps(0.0f, 2.0f * Far * Near / (Near - Far), 0.0f, 0.0f,
		-1.0f, (Far + Near) / (Near - Far), 0.0f, 0.0f);
	return LWSMatrix4(Row01, Row23);
}

LWSMatrix4<float> LWSMatrix4<float>::OrthoDX(float Left, float Right, float Bottom, float Top, float Near, float Far) {
	float sDepth = 1.0f / (Far - Near);
	__m256 Row01 = _mm256_set_ps(0.0f, 0.0f, 2.0f / (Top - Bottom), 0.0f,
		0.0f, 0.0f, 0.0f, 2.0f / (Right - Left));
	__m256 Row23 = _mm256_set_ps(1.0f, Near * sDepth, -(Top + Bottom) / (Top - Bottom), -(Right + Left) / (Right - Left),
		0.0f, -sDepth, 0.0f, 0.0f);
	return LWSMatrix4(Row01, Row23);
}

LWSMatrix4<float> LWSMatrix4<float>::OrthoGL(float Left, float Right, float Bottom, float Top, float Near, float Far) {
	__m256 Row01 = _mm256_set_ps(0.0f, 0.0f, 2.0f / (Top - Bottom), 0.0f,
		0.0f, 0.0f, 0.0f, 2.0f / (Right - Left));
	__m256 Row23 = _mm256_set_ps(1.0f, -(Near + Far) / (Far - Near), -(Top + Bottom) / (Top - Bottom), -(Right + Left) / (Right - Left),
		0.0f, -2.0f / (Far - Near), 0.0f, 0.0f);
	return LWSMatrix4(Row01, Row23);
}

LWSMatrix4<float> LWSMatrix4<float>::Ortho(float Left, float Right, float Bottom, float Top, float Near, float Far) {
	if (LWMatrix4_UseDXOrtho) return OrthoDX(Left, Right, Bottom, Top, Near, Far);
	return OrthoGL(Left, Right, Bottom, Top, Near, Far);
}

LWSMatrix4<float> LWSMatrix4<float>::Frustum(float Left, float Right, float Bottom, float Top, float Near, float Far) {
	__m256 Row01 = _mm256_set_ps(0.0f, (Top + Bottom) / (Top - Bottom), 2.0f * Near / (Top - Bottom), 0.0f,
		0.0f, (Right + Left) / (Right - Left), 0.0f, 2.0f * Near / (Right - Left));

	__m256 Row23 = _mm256_set_ps(0.0f, -2.0f * Far * Near / (Far - Near), 0.0f, 0.0f,
		-1.0f, -(Far + Near) / (Far - Near), 0.0f, 0.0f);
	return LWSMatrix4(Row01, Row23);
}

LWSMatrix4<float> LWSMatrix4<float>::LookAt(const LWSVector4<float>& Position, const LWSVector4<float>& Target, const LWSVector4<float>& Up) {
	LWSVector4<float> Fwrd = (Target - Position).Normalize3();
	LWSVector4<float> Rght = Fwrd.Cross3(Up).Normalize3();
	LWSVector4<float> U = Rght.Cross3(Fwrd);
	return LWSMatrix4(Rght, U, -Fwrd, Position);
}

LWSMatrix4<float>::LWSMatrix4(const LWSQuaternion<float>& Q) {
	__m128 vq = Q.m_Data;

	__m128 yxxx = _mm_permute_ps(vq, _MM_SHUFFLE(0, 0, 0, 1));
	__m128 zzyx = _mm_permute_ps(vq, _MM_SHUFFLE(0, 1, 2, 2));
	__m128 xxyx = _mm_permute_ps(vq, _MM_SHUFFLE(0, 1, 0, 0));
	__m128 zyzx = _mm_permute_ps(vq, _MM_SHUFFLE(0, 2, 1, 2));
	__m128 yzxx = _mm_permute_ps(vq, _MM_SHUFFLE(0, 0, 2, 1));
	__m128 wwwx = _mm_permute_ps(vq, _MM_SHUFFLE(0, 3, 3, 3));


	__m128 yy_xx_xx_xx = _mm_mul_ps(yxxx, yxxx);
	__m128 zz_zz_yy_xx = _mm_mul_ps(zzyx, zzyx);

	__m128 xz_xy_yz_xx = _mm_mul_ps(xxyx, zyzx);
	__m128 yw_zw_xw_xx = _mm_mul_ps(yzxx, wwwx);

	__m128 One = _mm_set_ps(0.0f, 1.0f, 1.0f, 1.0f);
	__m128 Two = _mm_set_ps(0.0f, 2.0f, 2.0f, 2.0f);

	__m128 A = _mm_sub_ps(One, _mm_mul_ps(Two, _mm_add_ps(yy_xx_xx_xx, zz_zz_yy_xx)));
	__m128 B = _mm_mul_ps(Two, _mm_add_ps(xz_xy_yz_xx, yw_zw_xw_xx));
	__m128 C = _mm_mul_ps(Two, _mm_sub_ps(xz_xy_yz_xx, yw_zw_xw_xx));

	__m128 Bxxxx = _mm_permute_ps(B, _MM_SHUFFLE(0, 0, 0, 0));
	__m128 Byyww = _mm_permute_ps(B, _MM_SHUFFLE(3, 3, 1, 1));
	__m128 Bzzzz = _mm_permute_ps(B, _MM_SHUFFLE(2, 2, 2, 2));

	__m128 Row0 = _mm_blend_ps(_mm_blend_ps(A, C, 0x2), Bxxxx, 0x4);
	__m128 Row1 = _mm_blend_ps(_mm_blend_ps(Byyww, A, 0x2), C, 0x4);
	__m128 Row2 = _mm_blend_ps(_mm_blend_ps(C, Bzzzz, 0x2), A, 0x4);

	__m128 Row3 = _mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f);

	m_Row01 = _mm256_set_m128(Row1, Row0);
	m_Row23 = _mm256_set_m128(Row3, Row2);
}

LWSMatrix4<float>::LWSMatrix4(const LWMatrix4<float> &M) {
	__m128 R0 = _mm_set_ps(M.m_Rows[0].w, M.m_Rows[0].z, M.m_Rows[0].y, M.m_Rows[0].x);
	__m128 R1 = _mm_set_ps(M.m_Rows[1].w, M.m_Rows[1].z, M.m_Rows[1].y, M.m_Rows[1].x);
	__m128 R2 = _mm_set_ps(M.m_Rows[2].w, M.m_Rows[2].z, M.m_Rows[2].y, M.m_Rows[2].x);
	__m128 R3 = _mm_set_ps(M.m_Rows[3].w, M.m_Rows[3].z, M.m_Rows[3].y, M.m_Rows[3].x);
	m_Row01 = _mm256_set_m128(R1, R0);
	m_Row23 = _mm256_set_m128(R3, R2);
}

LWSMatrix4<float>::LWSMatrix4(__m256 Row01, __m256 Row23) : m_Row01(Row01), m_Row23(Row23) {}

LWSMatrix4<float>::LWSMatrix4(float xScale, float yScale, float zScale, float wScale) : m_Row01(_mm256_set_ps(0.0f, 0.0f, yScale, 0.0f, 0.0f, 0.0f, 0.0f, xScale)), m_Row23(_mm256_set_ps(wScale, 0.0f, 0.0f, 0.0f, 0.0f, zScale, 0.0f, 0.0f)) {}

LWSMatrix4<float>::LWSMatrix4(const LWSVector4<float>& RowA, const LWSVector4<float>& RowB, const LWSVector4<float>& RowC, const LWSVector4<float>& RowD) : m_Row01(_mm256_set_m128(RowB.m_Data, RowA.m_Data)), m_Row23(_mm256_set_m128(RowD.m_Data, RowC.m_Data)) {}

LWSMatrix4<float>::LWSMatrix4(const LWSVector4<float>& Scale, const LWSQuaternion<float>& Rotation, const LWSVector4<float>& Pos) {
	__m128 vq = Rotation.m_Data;

	__m128 yxxx = _mm_permute_ps(vq, _MM_SHUFFLE(0, 0, 0, 1));
	__m128 zzyx = _mm_permute_ps(vq, _MM_SHUFFLE(0, 1, 2, 2));
	__m128 xxyx = _mm_permute_ps(vq, _MM_SHUFFLE(0, 1, 0, 0));
	__m128 zyzx = _mm_permute_ps(vq, _MM_SHUFFLE(0, 2, 1, 2));
	__m128 yzxx = _mm_permute_ps(vq, _MM_SHUFFLE(0, 0, 2, 1));
	__m128 wwwx = _mm_permute_ps(vq, _MM_SHUFFLE(0, 3, 3, 3));

	__m128 yy_xx_xx_xx = _mm_mul_ps(yxxx, yxxx);
	__m128 zz_zz_yy_xx = _mm_mul_ps(zzyx, zzyx);

	__m128 xz_xy_yz_xx = _mm_mul_ps(xxyx, zyzx);
	__m128 yw_zw_xw_xx = _mm_mul_ps(yzxx, wwwx);

	__m128 One = _mm_set_ps(0.0f, 1.0f, 1.0f, 1.0f);
	__m128 Two = _mm_set_ps(0.0f, 2.0f, 2.0f, 2.0f);

	__m128 A = _mm_sub_ps(One, _mm_mul_ps(Two, _mm_add_ps(yy_xx_xx_xx, zz_zz_yy_xx)));
	__m128 B = _mm_mul_ps(Two, _mm_add_ps(xz_xy_yz_xx, yw_zw_xw_xx));
	__m128 C = _mm_mul_ps(Two, _mm_sub_ps(xz_xy_yz_xx, yw_zw_xw_xx));

	__m128 Bxxxx = _mm_permute_ps(B, _MM_SHUFFLE(0, 0, 0, 0));
	__m128 Byyww = _mm_permute_ps(B, _MM_SHUFFLE(3, 3, 1, 1));
	__m128 Bzzzz = _mm_permute_ps(B, _MM_SHUFFLE(2, 2, 2, 2));

	__m128 Row0 = _mm_mul_ps(_mm_blend_ps(_mm_blend_ps(A, C, 0x2), Bxxxx, 0x4), Scale.xxxw().m_Data);
	__m128 Row1 = _mm_mul_ps(_mm_blend_ps(_mm_blend_ps(Byyww, A, 0x2), C, 0x4), Scale.yyyw().m_Data);
	__m128 Row2 = _mm_mul_ps(_mm_blend_ps(_mm_blend_ps(C, Bzzzz, 0x2), A, 0x4), Scale.zzzw().m_Data);
	__m128 Row3 = Pos.m_Data;


	m_Row01 = _mm256_set_m128(Row1, Row0);
	m_Row23 = _mm256_set_m128(Row3, Row2);
};

#endif