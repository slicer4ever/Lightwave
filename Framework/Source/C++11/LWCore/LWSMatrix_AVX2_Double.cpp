#include "LWCore/LWSMatrix.h"
#include "LWCore/LWSQuaternion.h"
#ifdef __AVX2__

LWMatrix4<double> LWSMatrix4<double>::AsMat4(void) const {
	alignas(64) LWMatrix4<double> R;
	_mm256_store_pd(&R.m_Rows[0].x, m_Row0);
	_mm256_store_pd(&R.m_Rows[1].x, m_Row1);
	_mm256_store_pd(&R.m_Rows[2].x, m_Row2);
	_mm256_store_pd(&R.m_Rows[3].x, m_Row3);
	return R;
}

LWSVector4<double> LWSMatrix4<double>::DecomposeScale(bool doTranspose3x3) const {
	LWSMatrix4<double> v = doTranspose3x3 ? Transpose3x3() : *this;
	__m256d R0 = v.m_Row0;
	__m256d R1 = v.m_Row1;
	__m256d R2 = v.m_Row2;

	//Calculate r0 length.
	R0 = _mm256_mul_pd(R0, R0);
	R0 = _mm256_hadd_pd(R0, R0);
	R0 = _mm256_permute4x64_pd(R0, _MM_SHUFFLE(0, 2, 1, 3));
	R0 = _mm256_hadd_pd(R0, R0);
	R0 = _mm256_sqrt_pd(R0);

	//Calculate r1 length.
	R1 = _mm256_mul_pd(R1, R1);
	R1 = _mm256_hadd_pd(R1, R1);
	R1 = _mm256_permute4x64_pd(R1, _MM_SHUFFLE(0, 2, 1, 3));
	R1 = _mm256_hadd_pd(R1, R1);
	R1 = _mm256_sqrt_pd(R1);

	//Calculate r2 length.
	R2 = _mm256_mul_pd(R2, R2);
	R2 = _mm256_hadd_pd(R2, R2);
	R2 = _mm256_permute4x64_pd(R2, _MM_SHUFFLE(0, 2, 1, 3));
	R2 = _mm256_hadd_pd(R2, R2);
	R2 = _mm256_sqrt_pd(R2);

	__m256d Res = _mm256_blend_pd(R0, R1, 0x2);
	Res = _mm256_blend_pd(Res, R2, 0x4);
	Res = _mm256_blend_pd(Res, _mm256_set1_pd(1.0), 0x8);
	return LWSVector4<double>(Res);
};

void LWSMatrix4<double>::Decompose(LWSVector4<double> &Scale, LWSQuaternion<double> &Rotation, LWSVector4<double> &Translation, bool doTranspose3x3) const {
	LWSMatrix4<double> v = doTranspose3x3 ? Transpose3x3() : *this;
	__m256d R0 = v.m_Row0;
	__m256d R1 = v.m_Row1;
	__m256d R2 = v.m_Row2;
	__m256d R3 = v.m_Row3;

	//Calculate r0 length.
	__m256d sR0 = _mm256_mul_pd(R0, R0);
	sR0 = _mm256_hadd_pd(sR0, sR0);
	sR0 = _mm256_permute4x64_pd(sR0, _MM_SHUFFLE(0, 2, 1, 3));
	sR0 = _mm256_hadd_pd(sR0, sR0);
	sR0 = _mm256_sqrt_pd(sR0);

	//Calculate r1 length.
	__m256d sR1 = _mm256_mul_pd(R1, R1);
	sR1 = _mm256_hadd_pd(sR1, sR1);
	sR1 = _mm256_permute4x64_pd(sR1, _MM_SHUFFLE(0, 2, 1, 3));
	sR1 = _mm256_hadd_pd(sR1, sR1);
	sR1 = _mm256_sqrt_pd(sR1);

	//Calculate r2 length.
	__m256d sR2 = _mm256_mul_pd(R2, R2);
	sR2 = _mm256_hadd_pd(sR2, sR2);
	sR2 = _mm256_permute4x64_pd(sR2, _MM_SHUFFLE(0, 2, 1, 3));
	sR2 = _mm256_hadd_pd(sR2, sR2);
	sR2 = _mm256_sqrt_pd(sR2);

	__m256d S = _mm256_blend_pd(sR0, sR1, 0x2);
	S = _mm256_blend_pd(S, sR2, 0x4);
	S = _mm256_blend_pd(S, _mm256_set1_pd(1.0), 0x8);
	Scale = LWSVector4<double>(S);

	__m256d iScale = _mm256_div_pd(_mm256_set1_pd(1.0), S);
	__m256d xScale = _mm256_permute4x64_pd(iScale, _MM_SHUFFLE(0, 0, 0, 0));
	__m256d yScale = _mm256_permute4x64_pd(iScale, _MM_SHUFFLE(1, 1, 1, 1));
	__m256d zScale = _mm256_permute4x64_pd(iScale, _MM_SHUFFLE(2, 2, 2, 2));

	R0 = _mm256_mul_pd(R0, xScale);
	R1 = _mm256_mul_pd(R1, yScale);
	R2 = _mm256_mul_pd(R2, zScale);
	v = LWSMatrix4<double>(R0, R1, R2, R3);
	Rotation = LWSQuaternion<double>(v);
	Translation = LWSVector4<double>(R3);
	return;
}

LWSMatrix4<double> LWSMatrix4<double>::TransformInverse(void) const {

	__m256d E = _mm256_set1_pd((double)std::numeric_limits<float>::epsilon());
	__m256d One = _mm256_set_pd(0.0, 1.0, 1.0, 1.0);
	//Transpose3x3 matrix.
	LWSMatrix4<double> T3 = Transpose3x3();

	__m256d Sq = _mm256_add_pd(_mm256_add_pd(_mm256_mul_pd(T3.m_Row0, T3.m_Row0), _mm256_mul_pd(T3.m_Row1, T3.m_Row1)), _mm256_mul_pd(T3.m_Row2, T3.m_Row2));
	__m256d rSq = _mm256_div_pd(One, Sq);
	rSq = _mm256_blendv_pd(rSq, One, _mm256_cmp_pd(Sq, E, _CMP_LE_OS));

	__m256d A = _mm256_mul_pd(rSq, T3.m_Row0);
	__m256d B = _mm256_mul_pd(rSq, T3.m_Row1);
	__m256d C = _mm256_mul_pd(rSq, T3.m_Row2);

	__m256d Dx = _mm256_permute4x64_pd(T3.m_Row3, _MM_SHUFFLE(3, 0, 0, 0));
	__m256d Dy = _mm256_permute4x64_pd(T3.m_Row3, _MM_SHUFFLE(3, 1, 1, 1));
	__m256d Dz = _mm256_permute4x64_pd(T3.m_Row3, _MM_SHUFFLE(3, 2, 2, 2));
	__m256d Dr = _mm256_add_pd(_mm256_mul_pd(Dx, A), _mm256_mul_pd(Dy, B));
	Dr = _mm256_add_pd(Dr, _mm256_mul_pd(Dz, C));
	Dr = _mm256_xor_pd(_mm256_set1_pd(-0.0), Dr); //Negate equation.
	Dr = _mm256_blend_pd(Dr, T3.m_Row3, 0x8); //Keep w component of original Row3.
	return LWSMatrix4(A, B, C, Dr);
}

LWSMatrix4<double> LWSMatrix4<double>::Inverse(void) const {
	//adapted Non-simd version Found from: https://stackoverflow.com/questions/1148309/inverting-a-4x4-matrix
	__m256d e = _mm256_set1_pd((double)std::numeric_limits<float>::epsilon());
	__m256d Zero = _mm256_setzero_pd();
	__m256d One = _mm256_set1_pd(1.0f);

	__m256d Czzyy = _mm256_permute4x64_pd(m_Row2, _MM_SHUFFLE(1, 1, 2, 2));
	__m256d Dwwwz = _mm256_permute4x64_pd(m_Row3, _MM_SHUFFLE(2, 3, 3, 3));
	__m256d Dzzyy = _mm256_permute4x64_pd(m_Row3, _MM_SHUFFLE(1, 1, 2, 2));
	__m256d Cwwwz = _mm256_permute4x64_pd(m_Row2, _MM_SHUFFLE(2, 3, 3, 3));

	__m256d Cyxxx = _mm256_permute4x64_pd(m_Row2, _MM_SHUFFLE(0, 0, 0, 1));
	__m256d Dyxxx = _mm256_permute4x64_pd(m_Row3, _MM_SHUFFLE(0, 0, 0, 1));

	__m256d Axxxx = _mm256_permute4x64_pd(m_Row0, _MM_SHUFFLE(0, 0, 0, 0));
	__m256d Ayyyy = _mm256_permute4x64_pd(m_Row0, _MM_SHUFFLE(1, 1, 1, 1));
	__m256d Azzzz = _mm256_permute4x64_pd(m_Row0, _MM_SHUFFLE(2, 2, 2, 2));
	__m256d Awwww = _mm256_permute4x64_pd(m_Row0, _MM_SHUFFLE(3, 3, 3, 3));

	__m256d Bxxxx = _mm256_permute4x64_pd(m_Row1, _MM_SHUFFLE(0, 0, 0, 0));
	__m256d Byyyy = _mm256_permute4x64_pd(m_Row1, _MM_SHUFFLE(1, 1, 1, 1));
	__m256d Bzzzz = _mm256_permute4x64_pd(m_Row1, _MM_SHUFFLE(2, 2, 2, 2));
	__m256d Bwwww = _mm256_permute4x64_pd(m_Row1, _MM_SHUFFLE(3, 3, 3, 3));

	__m256d Cxxxx = _mm256_permute4x64_pd(m_Row2, _MM_SHUFFLE(0, 0, 0, 0));
	__m256d Cyyyy = _mm256_permute4x64_pd(m_Row2, _MM_SHUFFLE(1, 1, 1, 1));
	__m256d Czzzz = _mm256_permute4x64_pd(m_Row2, _MM_SHUFFLE(2, 2, 2, 2));
	__m256d Cwwww = _mm256_permute4x64_pd(m_Row2, _MM_SHUFFLE(3, 3, 3, 3));

	__m256d Dxxxx = _mm256_permute4x64_pd(m_Row3, _MM_SHUFFLE(0, 0, 0, 0));
	__m256d Dyyyy = _mm256_permute4x64_pd(m_Row3, _MM_SHUFFLE(1, 1, 1, 1));
	__m256d Dzzzz = _mm256_permute4x64_pd(m_Row3, _MM_SHUFFLE(2, 2, 2, 2));
	__m256d Dwwww = _mm256_permute4x64_pd(m_Row3, _MM_SHUFFLE(3, 3, 3, 3));

	__m256d CzzBzz = _mm256_blend_pd(Czzzz, Bzzzz, 0xC);
	__m256d DwwwCw = _mm256_blend_pd(Dwwww, Cwwww, 0x8);
	__m256d CwwBww = _mm256_blend_pd(Cwwww, Bwwww, 0xC);
	__m256d DzzzCz = _mm256_blend_pd(Dzzzz, Czzzz, 0x8);
	__m256d CyyByy = _mm256_blend_pd(Cyyyy, Byyyy, 0xC);
	__m256d DyyyCy = _mm256_blend_pd(Dyyyy, Cyyyy, 0x8);
	__m256d CxxBxx = _mm256_blend_pd(Cxxxx, Bxxxx, 0xC);
	__m256d DxxxCx = _mm256_blend_pd(Dxxxx, Cxxxx, 0x8);

	__m256d A2323_A2323_A1323_A1223 = _mm256_sub_pd(_mm256_mul_pd(Czzyy, Dwwwz), _mm256_mul_pd(Cwwwz, Dzzyy));
	__m256d A1323_A0323_A0323_A0223 = _mm256_sub_pd(_mm256_mul_pd(Cyxxx, Dwwwz), _mm256_mul_pd(Cwwwz, Dyxxx));
	__m256d A1223_A0223_A0123_A0123 = _mm256_sub_pd(_mm256_mul_pd(Cyxxx, Dzzyy), _mm256_mul_pd(Czzyy, Dyxxx));

	__m256d A2323_A2323_A2313_A2312 = _mm256_sub_pd(_mm256_mul_pd(CzzBzz, DwwwCw), _mm256_mul_pd(CwwBww, DzzzCz));
	__m256d A1323_A1323_A1313_A1312 = _mm256_sub_pd(_mm256_mul_pd(CyyByy, DwwwCw), _mm256_mul_pd(CwwBww, DyyyCy));
	__m256d A1223_A1223_A1213_A1212 = _mm256_sub_pd(_mm256_mul_pd(CyyByy, DzzzCz), _mm256_mul_pd(CzzBzz, DyyyCy));
	__m256d A0323_A0323_A0313_A0312 = _mm256_sub_pd(_mm256_mul_pd(CxxBxx, DwwwCw), _mm256_mul_pd(CwwBww, DxxxCx));
	__m256d A0223_A0223_A0213_A0212 = _mm256_sub_pd(_mm256_mul_pd(CxxBxx, DzzzCz), _mm256_mul_pd(CzzBzz, DxxxCx));
	__m256d A0123_A0123_A0113_A0112 = _mm256_sub_pd(_mm256_mul_pd(CxxBxx, DyyyCy), _mm256_mul_pd(CyyByy, DxxxCx));

	__m256d NegA = _mm256_set_pd(0.0f, -0.0f, 0.0f, -0.0f);
	__m256d NegB = _mm256_set_pd(-0.0f, 0.0f, -0.0f, 0.0f);

	__m256d Byxxx = _mm256_permute4x64_pd(m_Row1, _MM_SHUFFLE(0, 0, 0, 1));
	__m256d Bzzyy = _mm256_permute4x64_pd(m_Row1, _MM_SHUFFLE(1, 1, 2, 2));
	__m256d Bwwwy = _mm256_permute4x64_pd(m_Row1, _MM_SHUFFLE(2, 3, 3, 3));

	__m256d PtA = _mm256_mul_pd(Byxxx, A2323_A2323_A1323_A1223);
	__m256d PtB = _mm256_mul_pd(Bzzyy, A1323_A0323_A0323_A0223);
	__m256d Ptc = _mm256_mul_pd(Bwwwy, A1223_A0223_A0123_A0123);
	__m256d Det = _mm256_xor_pd(_mm256_mul_pd(_mm256_add_pd(_mm256_sub_pd(PtA, PtB), Ptc), m_Row0), NegA);
	Det = _mm256_hadd_pd(Det, Det);
	Det = _mm256_permute4x64_pd(Det, _MM_SHUFFLE(0, 2, 1, 3));
	Det = _mm256_hadd_pd(Det, Det);
	__m256d rDet = _mm256_div_pd(One, Det);
	rDet = _mm256_blendv_pd(rDet, Zero, _mm256_cmp_pd(_mm256_andnot_pd(_mm256_set1_pd(-0.0), Det), e, _CMP_LE_OS));


	__m256d ByAyyy = _mm256_blend_pd(Byyyy, Ayyyy, 0xE);
	__m256d BzAzzz = _mm256_blend_pd(Bzzzz, Azzzz, 0xE);
	__m256d BwAwww = _mm256_blend_pd(Bwwww, Awwww, 0xE);
	__m256d BxAxxx = _mm256_blend_pd(Bxxxx, Axxxx, 0xe);
	__m256d A = _mm256_mul_pd(rDet, _mm256_xor_pd(NegA, _mm256_add_pd(_mm256_sub_pd(_mm256_mul_pd(ByAyyy, A2323_A2323_A2313_A2312), _mm256_mul_pd(BzAzzz, A1323_A1323_A1313_A1312)), _mm256_mul_pd(BwAwww, A1223_A1223_A1213_A1212))));
	//A = (ByAyyy * A2323_A2323_A2313_A2312 - BzAzzz * A1323_A1323_A1313_A1312 + BwAwww * A1223_A1223_A1213_A1212) * MulA * Det;
	__m256d B = _mm256_mul_pd(rDet, _mm256_xor_pd(NegB, _mm256_add_pd(_mm256_sub_pd(_mm256_mul_pd(BxAxxx, A2323_A2323_A2313_A2312), _mm256_mul_pd(BzAzzz, A0323_A0323_A0313_A0312)), _mm256_mul_pd(BwAwww, A0223_A0223_A0213_A0212))));
	//B = (ByAxxx * A2323_A2323_A2313_A2312 - BzAzzz * A0323_A0323_A0313_A0312 + BwAwww * A0223_A0223_A0213_A0212) * MulB * Det;
	__m256d C = _mm256_mul_pd(rDet, _mm256_xor_pd(NegA, _mm256_add_pd(_mm256_sub_pd(_mm256_mul_pd(BxAxxx, A1323_A1323_A1313_A1312), _mm256_mul_pd(ByAyyy, A0323_A0323_A0313_A0312)), _mm256_mul_pd(BwAwww, A0123_A0123_A0113_A0112))));
	//C = (BxAxxx * A1323_A1323_A1313_A1312 - ByAyyy * A0323_A0323_A0313_A0312 + BwAwww * A0123_A0123_A0113_A0112) * MulA * Det;
	__m256d D = _mm256_mul_pd(rDet, _mm256_xor_pd(NegB, _mm256_add_pd(_mm256_sub_pd(_mm256_mul_pd(BxAxxx, A1223_A1223_A1213_A1212), _mm256_mul_pd(ByAyyy, A0223_A0223_A0213_A0212)), _mm256_mul_pd(BzAzzz, A0123_A0123_A0113_A0112))));
	//D = (BxAxxx * A1223_A1223_A1213_A1212 - ByAyyy * A0223_A0223_A0213_A0212 + BzAzzz * A0123_A0123_A0113_A0112) * MulB * Det;
	return LWSMatrix4(A, B, C, D);
}

LWSVector4<double> LWSMatrix4<double>::Column(uint32_t Index) const {
	//Transpose for column.
	LWSMatrix4<double> T = Transpose();
	return T[Index];
};

LWSMatrix4<double> LWSMatrix4<double>::Transpose(void) const {

	//0x 1x 0z 1z
	__m256d A = _mm256_unpacklo_pd(m_Row0, m_Row1);
	//0y 1y 0w 1w
	__m256d B = _mm256_unpackhi_pd(m_Row0, m_Row1);
	//2x 3x 2y 3z
	__m256d C = _mm256_unpacklo_pd(m_Row2, m_Row3);
	//2y 3y 2w 3w
	__m256d D = _mm256_unpackhi_pd(m_Row2, m_Row3);

	__m256d Ar = _mm256_permute4x64_pd(A, _MM_SHUFFLE(1, 0, 3, 2));
	__m256d Br = _mm256_permute4x64_pd(B, _MM_SHUFFLE(1, 0, 3, 2));
	__m256d Cr = _mm256_permute4x64_pd(C, _MM_SHUFFLE(1, 0, 3, 2));
	__m256d Dr = _mm256_permute4x64_pd(D, _MM_SHUFFLE(1, 0, 3, 2));

	__m256d Row0 = _mm256_blend_pd(A, Cr, 0xC);
	__m256d Row1 = _mm256_blend_pd(B, Dr, 0xC);
	__m256d Row2 = _mm256_blend_pd(Ar, C, 0xC);
	__m256d Row3 = _mm256_blend_pd(Br, D, 0xC);
	return LWSMatrix4<double>(Row0, Row1, Row2, Row3);
}

LWSMatrix4<double> LWSMatrix4<double>::Transpose3x3(void) const {

	//0x 1x 0z 1z
	__m256d A = _mm256_unpacklo_pd(m_Row0, m_Row1);
	//0y 1y 0w 1w
	__m256d B = _mm256_unpackhi_pd(m_Row0, m_Row1);
	//2x 3x 2z 3z
	__m256d C = _mm256_unpacklo_pd(m_Row2, m_Row3);
	//2y 3y 2w 3w
	__m256d D = _mm256_unpackhi_pd(m_Row2, m_Row3);

	__m256d Ar = _mm256_permute4x64_pd(A, _MM_SHUFFLE(1, 0, 3, 2));
	__m256d Br = _mm256_permute4x64_pd(B, _MM_SHUFFLE(1, 0, 3, 2));
	__m256d Cr = _mm256_permute4x64_pd(C, _MM_SHUFFLE(1, 0, 3, 2));
	__m256d Dr = _mm256_permute4x64_pd(D, _MM_SHUFFLE(1, 0, 3, 2));

	__m256d Row0 = _mm256_blend_pd(_mm256_blend_pd(A, Cr, 0xC), m_Row0, 0x8);
	__m256d Row1 = _mm256_blend_pd(_mm256_blend_pd(B, Dr, 0xC), m_Row1, 0x8);
	__m256d Row2 = _mm256_blend_pd(_mm256_blend_pd(Ar, C, 0xC), m_Row2, 0x8);
	
	return LWSMatrix4<double>(Row0, Row1, Row2, m_Row3);
}

LWSMatrix4<double> LWSMatrix4<double>::Transpose2x2(void) const {
	//0x 1x | 0z 1z
	__m256d A = _mm256_unpacklo_pd(m_Row0, m_Row1);
	//0y 1y | 0w 1w
	__m256d B = _mm256_unpackhi_pd(m_Row0, m_Row1);

	__m256d Row0 = _mm256_blend_pd(A, m_Row0, 0xC);
	__m256d Row1 = _mm256_blend_pd(B, m_Row1, 0xC);
	return LWSMatrix4<double>(Row0, Row1, m_Row2, m_Row3);
}

double LWSMatrix4<double>::Determinant(void) const {
	//adapted Non-simd version Found from: https://stackoverflow.com/questions/1148309/inverting-a-4x4-matrix

	__m256d Czzyy = _mm256_permute4x64_pd(m_Row2, _MM_SHUFFLE(1, 1, 2, 2));
	__m256d Dwwwz = _mm256_permute4x64_pd(m_Row3, _MM_SHUFFLE(2, 3, 3, 3));
	__m256d Dzzyy = _mm256_permute4x64_pd(m_Row3, _MM_SHUFFLE(1, 1, 2, 2));
	__m256d Cwwwz = _mm256_permute4x64_pd(m_Row2, _MM_SHUFFLE(2, 3, 3, 3));

	__m256d Cyxxx = _mm256_permute4x64_pd(m_Row2, _MM_SHUFFLE(0, 0, 0, 1));
	__m256d Dyxxx = _mm256_permute4x64_pd(m_Row3, _MM_SHUFFLE(0, 0, 0, 1));

	__m256d Byxxx = _mm256_permute4x64_pd(m_Row1, _MM_SHUFFLE(0, 0, 0, 1));
	__m256d Bzzyy = _mm256_permute4x64_pd(m_Row1, _MM_SHUFFLE(1, 1, 2, 2));
	__m256d Bwwwy = _mm256_permute4x64_pd(m_Row1, _MM_SHUFFLE(2, 3, 3, 3));

	__m256d A2323_A2323_A1323_A1223 = _mm256_sub_pd(_mm256_mul_pd(Czzyy, Dwwwz), _mm256_mul_pd(Cwwwz, Dzzyy));
	__m256d A1323_A0323_A0323_A0223 = _mm256_sub_pd(_mm256_mul_pd(Cyxxx, Dwwwz), _mm256_mul_pd(Cwwwz, Dyxxx));
	__m256d A1223_A0223_A0123_A0123 = _mm256_sub_pd(_mm256_mul_pd(Cyxxx, Dzzyy), _mm256_mul_pd(Czzyy, Dyxxx));

	__m256d Neg = _mm256_set_pd(-0.0, 0.0, -0.0, 0.0);

	__m256d PtA = _mm256_mul_pd(Byxxx, A2323_A2323_A1323_A1223);
	__m256d PtB = _mm256_mul_pd(Bzzyy, A1323_A0323_A0323_A0223);
	__m256d Ptc = _mm256_mul_pd(Bwwwy, A1223_A0223_A0123_A0123);
	__m256d r = _mm256_xor_pd(_mm256_mul_pd(_mm256_add_pd(_mm256_sub_pd(PtA, PtB), Ptc), m_Row0), Neg);
	r = _mm256_hadd_pd(r, r);
	r = _mm256_permute4x64_pd(r, _MM_SHUFFLE(0, 2, 1, 3));
	r = _mm256_hadd_pd(r, r);
	return _mm256_cvtsd_f64(r);
}

LWSMatrix4<double>& LWSMatrix4<double>::operator = (const LWSMatrix4<double>& Rhs) {
	m_Row0 = Rhs.m_Row0;
	m_Row1 = Rhs.m_Row1;
	m_Row2 = Rhs.m_Row2;
	m_Row3 = Rhs.m_Row3;
	return *this;
}

LWSMatrix4<double>& LWSMatrix4<double>::operator+= (const LWSMatrix4<double>& Rhs) {
	m_Row0 = _mm256_add_pd(m_Row0, Rhs.m_Row0);
	m_Row1 = _mm256_add_pd(m_Row1, Rhs.m_Row1);
	m_Row2 = _mm256_add_pd(m_Row2, Rhs.m_Row2);
	m_Row3 = _mm256_add_pd(m_Row3, Rhs.m_Row3);
	return *this;
}

LWSMatrix4<double>& LWSMatrix4<double>::operator-= (const LWSMatrix4<double>& Rhs) {
	m_Row0 = _mm256_sub_pd(m_Row0, Rhs.m_Row0);
	m_Row1 = _mm256_sub_pd(m_Row1, Rhs.m_Row1);
	m_Row2 = _mm256_sub_pd(m_Row2, Rhs.m_Row2);
	m_Row3 = _mm256_sub_pd(m_Row3, Rhs.m_Row3);
	return *this;
}

LWSMatrix4<double>& LWSMatrix4<double>::operator*= (const LWSMatrix4<double>& Rhs) {
	__m256d Ax = _mm256_permute4x64_pd(m_Row0, _MM_SHUFFLE(0, 0, 0, 0));
	__m256d Ay = _mm256_permute4x64_pd(m_Row0, _MM_SHUFFLE(1, 1, 1, 1));
	__m256d Az = _mm256_permute4x64_pd(m_Row0, _MM_SHUFFLE(2, 2, 2, 2));
	__m256d Aw = _mm256_permute4x64_pd(m_Row0, _MM_SHUFFLE(3, 3, 3, 3));

	__m256d Bx = _mm256_permute4x64_pd(m_Row1, _MM_SHUFFLE(0, 0, 0, 0));
	__m256d By = _mm256_permute4x64_pd(m_Row1, _MM_SHUFFLE(1, 1, 1, 1));
	__m256d Bz = _mm256_permute4x64_pd(m_Row1, _MM_SHUFFLE(2, 2, 2, 2));
	__m256d Bw = _mm256_permute4x64_pd(m_Row1, _MM_SHUFFLE(3, 3, 3, 3));

	__m256d Cx = _mm256_permute4x64_pd(m_Row2, _MM_SHUFFLE(0, 0, 0, 0));
	__m256d Cy = _mm256_permute4x64_pd(m_Row2, _MM_SHUFFLE(1, 1, 1, 1));
	__m256d Cz = _mm256_permute4x64_pd(m_Row2, _MM_SHUFFLE(2, 2, 2, 2));
	__m256d Cw = _mm256_permute4x64_pd(m_Row2, _MM_SHUFFLE(3, 3, 3, 3));

	__m256d Dx = _mm256_permute4x64_pd(m_Row3, _MM_SHUFFLE(0, 0, 0, 0));
	__m256d Dy = _mm256_permute4x64_pd(m_Row3, _MM_SHUFFLE(1, 1, 1, 1));
	__m256d Dz = _mm256_permute4x64_pd(m_Row3, _MM_SHUFFLE(2, 2, 2, 2));
	__m256d Dw = _mm256_permute4x64_pd(m_Row3, _MM_SHUFFLE(3, 3, 3, 3));
	
	m_Row0 = _mm256_add_pd(_mm256_add_pd(_mm256_add_pd(_mm256_mul_pd(Ax, Rhs.m_Row0), _mm256_mul_pd(Ay, Rhs.m_Row1)), _mm256_mul_pd(Az, Rhs.m_Row2)), _mm256_mul_pd(Aw, Rhs.m_Row3));
	
	m_Row1 = _mm256_add_pd(_mm256_add_pd(_mm256_add_pd(_mm256_mul_pd(Bx, Rhs.m_Row0), _mm256_mul_pd(By, Rhs.m_Row1)), _mm256_mul_pd(Bz, Rhs.m_Row2)), _mm256_mul_pd(Bw, Rhs.m_Row3));
	m_Row2 = _mm256_add_pd(_mm256_add_pd(_mm256_add_pd(_mm256_mul_pd(Cx, Rhs.m_Row0), _mm256_mul_pd(Cy, Rhs.m_Row1)), _mm256_mul_pd(Cz, Rhs.m_Row2)), _mm256_mul_pd(Cw, Rhs.m_Row3));
	m_Row3 = _mm256_add_pd(_mm256_add_pd(_mm256_add_pd(_mm256_mul_pd(Dx, Rhs.m_Row0), _mm256_mul_pd(Dy, Rhs.m_Row1)), _mm256_mul_pd(Dz, Rhs.m_Row2)), _mm256_mul_pd(Dw, Rhs.m_Row3));

	return *this;
}

LWSMatrix4<double>& LWSMatrix4<double>::operator *=(double Rhs) {
	__m256d r = _mm256_set1_pd(Rhs);
	m_Row0 = _mm256_mul_pd(m_Row0, r);
	m_Row1 = _mm256_mul_pd(m_Row1, r);
	m_Row2 = _mm256_mul_pd(m_Row2, r);
	m_Row3 = _mm256_mul_pd(m_Row3, r);
	return *this;
}

LWSMatrix4<double>& LWSMatrix4<double>::operator /=(double Rhs) {
	__m256d r = _mm256_set1_pd(Rhs);
	m_Row0 = _mm256_div_pd(m_Row0, r);
	m_Row1 = _mm256_div_pd(m_Row1, r);
	m_Row2 = _mm256_div_pd(m_Row2, r);
	m_Row3 = _mm256_div_pd(m_Row3, r);
	return *this;
}

bool LWSMatrix4<double>::operator == (const LWSMatrix4<double>& Rhs) const {
	__m256d e = _mm256_set1_pd((float)std::numeric_limits<float>::epsilon());
	__m256d n0 = _mm256_set1_pd(-0.0f);
	__m256d t0 = _mm256_sub_pd(m_Row0, Rhs.m_Row0);
	__m256d t1 = _mm256_sub_pd(m_Row1, Rhs.m_Row1);
	__m256d t2 = _mm256_sub_pd(m_Row2, Rhs.m_Row2);
	__m256d t3 = _mm256_sub_pd(m_Row3, Rhs.m_Row3);
	//Get absolute value of difference:

	t0 = _mm256_andnot_pd(n0, t0);
	t1 = _mm256_andnot_pd(n0, t1);
	t2 = _mm256_andnot_pd(n0, t2);
	t3 = _mm256_andnot_pd(n0, t3);
	//compare le epsilon:
	t0 = _mm256_cmp_pd(t0, e, _CMP_LE_OS);
	t1 = _mm256_cmp_pd(t1, e, _CMP_LE_OS);
	t2 = _mm256_cmp_pd(t2, e, _CMP_LE_OS);
	t3 = _mm256_cmp_pd(t3, e, _CMP_LE_OS);
	return _mm256_test_all_ones(_mm256_castpd_si256(t0)) && _mm256_test_all_ones(_mm256_castpd_si256(t1)) && _mm256_test_all_ones(_mm256_castpd_si256(t2)) && _mm256_test_all_ones(_mm256_castpd_si256(t3));
}

bool LWSMatrix4<double>::operator != (const LWSMatrix4<double>& Rhs) const {
	return !(*this == Rhs);
}

LWSVector4<double> LWSMatrix4<double>::operator[](uint32_t i) const {
	return m_Rows[i];
}

LWSVector4<double> &LWSMatrix4<double>::operator[](uint32_t i) {
	return m_Rows[i];
}

std::ostream& operator<<(std::ostream& o, const LWSMatrix4<double>& M) {
	alignas(64) double V[16];
	_mm256_store_pd(V, M.m_Row0);
	_mm256_store_pd(V + 4, M.m_Row1);
	_mm256_store_pd(V + 8, M.m_Row2);
	_mm256_store_pd(V + 12, M.m_Row3);
	o << "Matrix4:" << std::endl;
	o << V[0] << " " << V[1] << " " << V[2] << " " << V[3] << std::endl;
	o << V[4] << " " << V[5] << " " << V[6] << " " << V[7] << std::endl;
	o << V[8] << " " << V[9] << " " << V[10] << " " << V[11] << std::endl;
	o << V[12] << " " << V[13] << " " << V[14] << " " << V[15] << std::endl;
	return o;
}

LWSMatrix4<double> operator + (const LWSMatrix4<double>& Lhs, const LWSMatrix4<double>& Rhs) {
	return LWSMatrix4<double>(_mm256_add_pd(Lhs.m_Row0, Rhs.m_Row0), _mm256_add_pd(Lhs.m_Row1, Rhs.m_Row1), _mm256_add_pd(Lhs.m_Row2, Rhs.m_Row2), _mm256_add_pd(Lhs.m_Row3, Rhs.m_Row3));
}

LWSMatrix4<double> operator - (const LWSMatrix4<double>& Lhs, const LWSMatrix4<double>& Rhs) {
	return LWSMatrix4<double>(_mm256_sub_pd(Lhs.m_Row0, Rhs.m_Row0), _mm256_sub_pd(Lhs.m_Row1, Rhs.m_Row1), _mm256_sub_pd(Lhs.m_Row2, Rhs.m_Row2), _mm256_sub_pd(Lhs.m_Row3, Rhs.m_Row3));
}

LWSMatrix4<double> operator * (const LWSMatrix4<double>& Lhs, const LWSMatrix4<double>& Rhs) {
	__m256d Ax = _mm256_permute4x64_pd(Lhs.m_Row0, _MM_SHUFFLE(0, 0, 0, 0));
	__m256d Ay = _mm256_permute4x64_pd(Lhs.m_Row0, _MM_SHUFFLE(1, 1, 1, 1));
	__m256d Az = _mm256_permute4x64_pd(Lhs.m_Row0, _MM_SHUFFLE(2, 2, 2, 2));
	__m256d Aw = _mm256_permute4x64_pd(Lhs.m_Row0, _MM_SHUFFLE(3, 3, 3, 3));

	__m256d Bx = _mm256_permute4x64_pd(Lhs.m_Row1, _MM_SHUFFLE(0, 0, 0, 0));
	__m256d By = _mm256_permute4x64_pd(Lhs.m_Row1, _MM_SHUFFLE(1, 1, 1, 1));
	__m256d Bz = _mm256_permute4x64_pd(Lhs.m_Row1, _MM_SHUFFLE(2, 2, 2, 2));
	__m256d Bw = _mm256_permute4x64_pd(Lhs.m_Row1, _MM_SHUFFLE(3, 3, 3, 3));

	__m256d Cx = _mm256_permute4x64_pd(Lhs.m_Row2, _MM_SHUFFLE(0, 0, 0, 0));
	__m256d Cy = _mm256_permute4x64_pd(Lhs.m_Row2, _MM_SHUFFLE(1, 1, 1, 1));
	__m256d Cz = _mm256_permute4x64_pd(Lhs.m_Row2, _MM_SHUFFLE(2, 2, 2, 2));
	__m256d Cw = _mm256_permute4x64_pd(Lhs.m_Row2, _MM_SHUFFLE(3, 3, 3, 3));
	
	__m256d Dx = _mm256_permute4x64_pd(Lhs.m_Row3, _MM_SHUFFLE(0, 0, 0, 0));
	__m256d Dy = _mm256_permute4x64_pd(Lhs.m_Row3, _MM_SHUFFLE(1, 1, 1, 1));
	__m256d Dz = _mm256_permute4x64_pd(Lhs.m_Row3, _MM_SHUFFLE(2, 2, 2, 2));
	__m256d Dw = _mm256_permute4x64_pd(Lhs.m_Row3, _MM_SHUFFLE(3, 3, 3, 3));

	__m256d Row0 = _mm256_add_pd(_mm256_add_pd(_mm256_add_pd(_mm256_mul_pd(Ax, Rhs.m_Row0), _mm256_mul_pd(Ay, Rhs.m_Row1)), _mm256_mul_pd(Az, Rhs.m_Row2)), _mm256_mul_pd(Aw, Rhs.m_Row3));
	__m256d Row1 = _mm256_add_pd(_mm256_add_pd(_mm256_add_pd(_mm256_mul_pd(Bx, Rhs.m_Row0), _mm256_mul_pd(By, Rhs.m_Row1)), _mm256_mul_pd(Bz, Rhs.m_Row2)), _mm256_mul_pd(Bw, Rhs.m_Row3));
	__m256d Row2 = _mm256_add_pd(_mm256_add_pd(_mm256_add_pd(_mm256_mul_pd(Cx, Rhs.m_Row0), _mm256_mul_pd(Cy, Rhs.m_Row1)), _mm256_mul_pd(Cz, Rhs.m_Row2)), _mm256_mul_pd(Cw, Rhs.m_Row3));
	__m256d Row3 = _mm256_add_pd(_mm256_add_pd(_mm256_add_pd(_mm256_mul_pd(Dx, Rhs.m_Row0), _mm256_mul_pd(Dy, Rhs.m_Row1)), _mm256_mul_pd(Dz, Rhs.m_Row2)), _mm256_mul_pd(Dw, Rhs.m_Row3));

	return LWSMatrix4<double>(Row0, Row1, Row2, Row3);
}

LWSMatrix4<double> operator * (const LWSMatrix4<double>& Lhs, double Rhs) {
	__m256d r = _mm256_set1_pd(Rhs);
	return LWSMatrix4<double>(_mm256_mul_pd(Lhs.m_Row0, r), _mm256_mul_pd(Lhs.m_Row1, r), _mm256_mul_pd(Lhs.m_Row2, r), _mm256_mul_pd(Lhs.m_Row3, r));
}

LWSMatrix4<double> operator * (double Lhs, const LWSMatrix4<double>& Rhs) {
	__m256d r = _mm256_set1_pd(Lhs);
	return LWSMatrix4<double>(_mm256_mul_pd(r, Rhs.m_Row0), _mm256_mul_pd(r, Rhs.m_Row1), _mm256_mul_pd(r, Rhs.m_Row2), _mm256_mul_pd(r, Rhs.m_Row3));
}

LWSMatrix4<double> operator / (const LWSMatrix4<double>& Lhs, double Rhs) {
	__m256d r = _mm256_set1_pd(Rhs);
	return LWSMatrix4<double>(_mm256_div_pd(Lhs.m_Row0, r), _mm256_div_pd(Lhs.m_Row1, r), _mm256_div_pd(Lhs.m_Row2, r), _mm256_div_pd(Lhs.m_Row3, r));
}

LWSMatrix4<double> operator / (double Lhs, const LWSMatrix4<double>& Rhs) {
	__m256d r = _mm256_set1_pd(Lhs);
	return LWSMatrix4<double>(_mm256_div_pd(r, Rhs.m_Row0), _mm256_div_pd(r, Rhs.m_Row1), _mm256_div_pd(r, Rhs.m_Row2), _mm256_div_pd(r, Rhs.m_Row3));
}

LWSVector4<double> operator * (const LWSMatrix4<double>& Lhs, const LWSVector4<double>& Rhs) {
	__m256d Rx = _mm256_permute4x64_pd(Rhs.m_Data, _MM_SHUFFLE(0, 0, 0, 0));
	__m256d Ry = _mm256_permute4x64_pd(Rhs.m_Data, _MM_SHUFFLE(1, 1, 1, 1));
	__m256d Rz = _mm256_permute4x64_pd(Rhs.m_Data, _MM_SHUFFLE(2, 2, 2, 2));
	__m256d Rw = _mm256_permute4x64_pd(Rhs.m_Data, _MM_SHUFFLE(3, 3, 3, 3));

	__m256d r = _mm256_mul_pd(Lhs.m_Row0, Rx);
	r = _mm256_add_pd(r, _mm256_mul_pd(Lhs.m_Row1, Ry));
	r = _mm256_add_pd(r, _mm256_mul_pd(Lhs.m_Row2, Rz));
	r = _mm256_add_pd(r, _mm256_mul_pd(Lhs.m_Row3, Rw));
	return LWSVector4<double>(r);
}

LWSVector4<double> operator * (const LWSVector4<double>& Lhs, const LWSMatrix4<double>& Rhs) {
	__m256d Lx = _mm256_permute4x64_pd(Lhs.m_Data, _MM_SHUFFLE(0, 0, 0, 0));
	__m256d Ly = _mm256_permute4x64_pd(Lhs.m_Data, _MM_SHUFFLE(1, 1, 1, 1));
	__m256d Lz = _mm256_permute4x64_pd(Lhs.m_Data, _MM_SHUFFLE(2, 2, 2, 2));
	__m256d Lw = _mm256_permute4x64_pd(Lhs.m_Data, _MM_SHUFFLE(3, 3, 3, 3));
	__m256d r = _mm256_mul_pd(Lx, Rhs.m_Row0);
	r = _mm256_add_pd(r, _mm256_mul_pd(Ly, Rhs.m_Row1));
	r = _mm256_add_pd(r, _mm256_mul_pd(Lz, Rhs.m_Row2));
	r = _mm256_add_pd(r, _mm256_mul_pd(Lw, Rhs.m_Row3));
	return LWSVector4<double>(r);
}

LWSMatrix4<double> LWSMatrix4<double>::FromEuler(double Pitch, double Yaw, double Roll) {
	double c1 = cos(Yaw);
	double c2 = cos(Pitch);
	double c3 = cos(Roll);
	double s1 = sin(Yaw);
	double s2 = sin(Pitch);
	double s3 = sin(Roll);
	double s1s2 = s1 * s2;
	return LWSMatrix4<double>(
		LWSVector4<double>(c1 * c2, s1 * s3 - c1 * s2 * c3, c1 * s2 * s3 + s1 * c3, 0.0),
		LWSVector4<double>(s2, c2 * c3, -c2 * s3, 0.0),
		LWSVector4<double>(-s1 * c2, s1s2 * c3 + c1 * s3, -s1s2 * s3 + c1 * c3, 0.0),
		LWSVector4<double>(0.0, 0.0, 0.0, 1.0)
		);
}

LWSMatrix4<double> LWSMatrix4<double>::FromEuler(const LWVector3<double> &Euler) {
	return FromEuler(Euler.x, Euler.y, Euler.z);
}

LWVector3<double> LWSMatrix4<double>::ToEuler(void) const {
	const double e = std::numeric_limits<double>::epsilon();
	if (m_Rows[1].x > 1.0f - e) {
		return LWVector3<double>(LW_PI_2, atan2(m_Rows[0].z, m_Rows[2].z), 0.0);
	} else if (m_Rows[1].x < -1.0f + e) {
		return LWVector3<double>(-LW_PI_2, atan2(m_Rows[0].z, m_Rows[2].z), 0.0);
	}
	return LWVector3<double>(asin(m_Rows[1].x), atan2(-m_Rows[2].x, m_Rows[0].x), atan2(-m_Rows[1].z, m_Rows[1].y));
}

LWSMatrix4<double> LWSMatrix4<double>::RotationX(double Theta) {
	double S = sin(Theta);
	double C = cos(Theta);
	return LWSMatrix4<double>(_mm256_set_pd(0.0,0.0, 0.0, 1.0), _mm256_set_pd(0.0, -S, C, 0.0), _mm256_set_pd(0.0, C, S, 0.0), _mm256_set_pd(1.0, 0.0, 0.0, 0.0));
}

LWSMatrix4<double> LWSMatrix4<double>::RotationY(double Theta) {
	double S = sin(Theta);
	double C = cos(Theta);
	return LWSMatrix4<double>(_mm256_set_pd(0.0, S, 0.0, C), _mm256_set_pd(0.0, 0.0, 1.0, 0.0), _mm256_set_pd(0.0, C, 0.0, -S), _mm256_set_pd(1.0, 0.0, 0.0, 0.0));
}

LWSMatrix4<double> LWSMatrix4<double>::RotationZ(double Theta) {
	double S = sin(Theta);
	double C = cos(Theta);
	return LWSMatrix4<double>(_mm256_set_pd(0.0, 0.0, -S, C), _mm256_set_pd(0.0, 0.0, C, S), _mm256_set_pd(0.0, 1.0, 0.0, 0.0), _mm256_set_pd(1.0, 0.0, 0.0, 0.0));
}

LWSMatrix4<double> LWSMatrix4<double>::RotationXY(double ThetaX, double ThetaY) {
	return RotationX(ThetaX) * RotationY(ThetaY);
}

LWSMatrix4<double> LWSMatrix4<double>::RotationXZ(double ThetaX, double ThetaZ) {
	return RotationX(ThetaX) * RotationZ(ThetaZ);
}

LWSMatrix4<double> LWSMatrix4<double>::RotationYX(double ThetaY, double ThetaX) {
	return RotationY(ThetaY) * RotationX(ThetaX);
}

LWSMatrix4<double> LWSMatrix4<double>::RotationYZ(double ThetaY, double ThetaZ) {
	return RotationY(ThetaY) * RotationZ(ThetaZ);
}

LWSMatrix4<double> LWSMatrix4<double>::RotationZX(double ThetaZ, double ThetaX) {
	return RotationZ(ThetaZ) * RotationX(ThetaX);
}

LWSMatrix4<double> LWSMatrix4<double>::RotationZY(double ThetaZ, double ThetaY) {
	return RotationZ(ThetaZ) * RotationY(ThetaY);
}

LWSMatrix4<double> LWSMatrix4<double>::RotationXYZ(double ThetaX, double ThetaY, double ThetaZ) {
	return RotationX(ThetaX) * RotationY(ThetaY) * RotationZ(ThetaZ);
}

LWSMatrix4<double> LWSMatrix4<double>::RotationXZY(double ThetaX, double ThetaZ, double ThetaY) {
	return RotationX(ThetaX) * RotationZ(ThetaZ) * RotationY(ThetaY);
}

LWSMatrix4<double> LWSMatrix4<double>::RotationYXZ(double ThetaY, double ThetaX, double ThetaZ) {
	return RotationY(ThetaY) * RotationX(ThetaX) * RotationZ(ThetaZ);
}

LWSMatrix4<double> LWSMatrix4<double>::RotationYZX(double ThetaY, double ThetaZ, double ThetaX) {
	return RotationY(ThetaY) * RotationZ(ThetaZ) * RotationX(ThetaX);
}

LWSMatrix4<double> LWSMatrix4<double>::RotationZXY(double ThetaZ, double ThetaX, double ThetaY) {
	return RotationZ(ThetaZ) * RotationX(ThetaX) * RotationY(ThetaY);
}

LWSMatrix4<double> LWSMatrix4<double>::RotationZYX(double ThetaZ, double ThetaY, double ThetaX) {
	return RotationZ(ThetaZ) * RotationY(ThetaY) * RotationX(ThetaX);
}

LWSMatrix4<double> LWSMatrix4<double>::Translation(double x, double y, double z) {
	return LWSMatrix4<double>(_mm256_set_pd(0.0, 0.0, 0.0, 1.0), _mm256_set_pd(0.0, 0.0, 1.0, 0.0), _mm256_set_pd(0.0, 1.0, 0.0, 0.0), _mm256_set_pd(1.0, z, y, x));
}

LWSMatrix4<double> LWSMatrix4<double>::Translation(const LWSVector4<double>& Position) {
	return LWSMatrix4<double>(_mm256_set_pd(0.0, 0.0, 0.0, 1.0), _mm256_set_pd(0.0, 0.0, 1.0, 0.0), _mm256_set_pd(0.0, 1.0, 0.0, 0.0), _mm256_blend_pd(Position.m_Data, _mm256_set1_pd(1.0), 0x8));
}

LWSMatrix4<double> LWSMatrix4<double>::Rotation(const LWSVector4<double>& Direction, const LWSVector4<double>& Up) {
	LWSVector4<double> xAxis = Up.Cross3(Direction);
	LWSVector4<double> yAxis = Direction.Cross3(xAxis);
	return LWSMatrix4<double>(xAxis.m_Data, yAxis.m_Data, Direction.m_Data, _mm256_set_pd(1.0, 0.0, 0.0, 0.0));
}

LWSMatrix4<double> LWSMatrix4<double>::Perspective(double FoV, double Aspect, double Near, double Far) {
	double F = 1.0 / tan(FoV * 0.5);
	__m256d Row0 = _mm256_set_pd(0.0, 0.0, 0.0, F / Aspect);
	__m256d Row1 = _mm256_set_pd(0.0, 0.0, F, 0.0);
	__m256d Row2 = _mm256_set_pd(-1.0, (Far + Near) / (Near - Far), 0.0, 0.0);
	__m256d Row3 = _mm256_set_pd(0.0, 2.0 * Far * Near / (Near - Far), 0.0, 0.0);
		
	return LWSMatrix4<double>(Row0, Row1, Row2, Row3);
}

LWSMatrix4<double> LWSMatrix4<double>::OrthoDX(double Left, double Right, double Bottom, double Top, double Near, double Far) {
	double sDepth = 1.0f / (Far - Near);
	__m256d Row0 = _mm256_set_pd(0.0, 0.0, 0.0, 2.0 / (Right - Left));
	__m256d Row1 = _mm256_set_pd(0.0, 0.0, 2.0 / (Top - Bottom), 0.0);
	__m256d Row2 = _mm256_set_pd(0.0, -sDepth, 0.0, 0.0);
	__m256d Row3 = _mm256_set_pd(1.0, Near * sDepth, -(Top + Bottom) / (Top - Bottom), -(Right + Left) / (Right - Left));
	return LWSMatrix4<double>(Row0, Row1, Row2, Row3);
}

LWSMatrix4<double> LWSMatrix4<double>::OrthoGL(double Left, double Right, double Bottom, double Top, double Near, double Far) {
	__m256d Row0 = _mm256_set_pd(0.0, 0.0, 0.0, 2.0 / (Right - Left));
	__m256d Row1 = _mm256_set_pd(0.0, 0.0, 2.0 / (Top - Bottom), 0.0);
	__m256d Row2 = _mm256_set_pd(0.0, -2.0 / (Far - Near), 0.0, 0.0);
	__m256d Row3 = _mm256_set_pd(1.0, -(Near + Far) / (Far - Near), -(Top + Bottom) / (Top - Bottom), -(Right + Left) / (Right - Left));
	return LWSMatrix4<double>(Row0, Row1, Row2, Row3);
}

LWSMatrix4<double> LWSMatrix4<double>::Ortho(double Left, double Right, double Bottom, double Top, double Near, double Far) {
	if (LWMatrix4_UseDXOrtho) return OrthoDX(Left, Right, Bottom, Top, Near, Far);
	return OrthoGL(Left, Right, Bottom, Top, Near, Far);
}

LWSMatrix4<double> LWSMatrix4<double>::Frustum(double Left, double Right, double Bottom, double Top, double Near, double Far) {
	__m256d Row0 = _mm256_set_pd(0.0, (Right + Left) / (Right - Left), 0.0, 2.0 * Near / (Right - Left));
	__m256d Row1 = _mm256_set_pd(0.0, (Top + Bottom) / (Top - Bottom), 2.0 * Near / (Top - Bottom), 0.0);
	__m256d Row2 = _mm256_set_pd(-1.0, -(Far + Near) / (Far - Near), 0.0, 0.0);
	__m256d Row3 = _mm256_set_pd(0.0f, -2.0 * Far * Near / (Far - Near), 0.0, 0.0);
		
	return LWSMatrix4<double>(Row0, Row1, Row2, Row3);
}

LWSMatrix4<double> LWSMatrix4<double>::LookAt(const LWSVector4<double>& Position, const LWSVector4<double>& Target, const LWSVector4<double>& Up) {
	LWSVector4<double> Fwrd = (Target - Position).Normalize3();
	LWSVector4<double> Rght = Fwrd.Cross3(Up).Normalize3();
	LWSVector4<double> U = Rght.Cross3(Fwrd);
	return LWSMatrix4<double>(Rght, U, -Fwrd, Position);
}

LWSMatrix4<double>::LWSMatrix4(const LWSQuaternion<double>& Q) {
	__m256d vq = Q.m_Data;

	__m256d yxxx = _mm256_permute4x64_pd(vq, _MM_SHUFFLE(0, 0, 0, 1));
	__m256d zzyx = _mm256_permute4x64_pd(vq, _MM_SHUFFLE(0, 1, 2, 2));
	__m256d xxyx = _mm256_permute4x64_pd(vq, _MM_SHUFFLE(0, 1, 0, 0));
	__m256d zyzx = _mm256_permute4x64_pd(vq, _MM_SHUFFLE(0, 2, 1, 2));
	__m256d yzxx = _mm256_permute4x64_pd(vq, _MM_SHUFFLE(0, 0, 2, 1));
	__m256d wwwx = _mm256_permute4x64_pd(vq, _MM_SHUFFLE(0, 3, 3, 3));


	__m256d yy_xx_xx_xx = _mm256_mul_pd(yxxx, yxxx);
	__m256d zz_zz_yy_xx = _mm256_mul_pd(zzyx, zzyx);

	__m256d xz_xy_yz_xx = _mm256_mul_pd(xxyx, zyzx);
	__m256d yw_zw_xw_xx = _mm256_mul_pd(yzxx, wwwx);

	__m256d One = _mm256_set_pd(0.0, 1.0, 1.0, 1.0);
	__m256d Two = _mm256_set_pd(0.0, 2.0, 2.0, 2.0);

	__m256d A = _mm256_sub_pd(One, _mm256_mul_pd(Two, _mm256_add_pd(yy_xx_xx_xx, zz_zz_yy_xx)));
	__m256d B = _mm256_mul_pd(Two, _mm256_add_pd(xz_xy_yz_xx, yw_zw_xw_xx));
	__m256d C = _mm256_mul_pd(Two, _mm256_sub_pd(xz_xy_yz_xx, yw_zw_xw_xx));

	__m256d Bxxxx = _mm256_permute4x64_pd(B, _MM_SHUFFLE(0, 0, 0, 0));
	__m256d Byyww = _mm256_permute4x64_pd(B, _MM_SHUFFLE(3, 3, 1, 1));
	__m256d Bzzzz = _mm256_permute4x64_pd(B, _MM_SHUFFLE(2, 2, 2, 2));

	m_Row0 = _mm256_blend_pd(_mm256_blend_pd(A, C, 0x2), Bxxxx, 0x4);
	m_Row1 = _mm256_blend_pd(_mm256_blend_pd(Byyww, A, 0x2), C, 0x4);
	m_Row2 = _mm256_blend_pd(_mm256_blend_pd(C, Bzzzz, 0x2), A, 0x4);
	m_Row3 = _mm256_set_pd(1.0, 0.0, 0.0, 0.0);
}

LWSMatrix4<double>::LWSMatrix4(const LWMatrix4<double> &M) {
	m_Row0 = _mm256_set_pd(M.m_Rows[0].w, M.m_Rows[0].z, M.m_Rows[0].y, M.m_Rows[0].x);
	m_Row1 = _mm256_set_pd(M.m_Rows[1].w, M.m_Rows[1].z, M.m_Rows[1].y, M.m_Rows[1].x);
	m_Row2 = _mm256_set_pd(M.m_Rows[2].w, M.m_Rows[2].z, M.m_Rows[2].y, M.m_Rows[2].x);
	m_Row3 = _mm256_set_pd(M.m_Rows[3].w, M.m_Rows[3].z, M.m_Rows[3].y, M.m_Rows[3].x);
}

LWSMatrix4<double>::LWSMatrix4(__m256d Row0, __m256d Row1, __m256d Row2, __m256d Row3) : m_Row0(Row0), m_Row1(Row1), m_Row2(Row2), m_Row3(Row3) {}

LWSMatrix4<double>::LWSMatrix4(double xScale, double yScale, double zScale, double wScale) : m_Row0(_mm256_set_pd(0.0, 0.0, 0.0, xScale)), m_Row1(_mm256_set_pd(0.0, 0.0, yScale, 0.0)), m_Row2(_mm256_set_pd(0.0, zScale, 0.0, 0.0)), m_Row3(_mm256_set_pd(wScale, 0.0, 0.0, 0.0)) {}

LWSMatrix4<double>::LWSMatrix4(const LWSVector4<double>& RowA, const LWSVector4<double>& RowB, const LWSVector4<double>& RowC, const LWSVector4<double>& RowD) : m_Row0(RowA.m_Data), m_Row1(RowB.m_Data), m_Row2(RowC.m_Data), m_Row3(RowD.m_Data) {}

LWSMatrix4<double>::LWSMatrix4(const LWSVector4<double>& Scale, const LWSQuaternion<double>& Rotation, const LWSVector4<double>& Pos) {
	__m256d vq = Rotation.m_Data;

	__m256d yxxx = _mm256_permute4x64_pd(vq, _MM_SHUFFLE(0, 0, 0, 1));
	__m256d zzyx = _mm256_permute4x64_pd(vq, _MM_SHUFFLE(0, 1, 2, 2));
	__m256d xxyx = _mm256_permute4x64_pd(vq, _MM_SHUFFLE(0, 1, 0, 0));
	__m256d zyzx = _mm256_permute4x64_pd(vq, _MM_SHUFFLE(0, 2, 1, 2));
	__m256d yzxx = _mm256_permute4x64_pd(vq, _MM_SHUFFLE(0, 0, 2, 1));
	__m256d wwwx = _mm256_permute4x64_pd(vq, _MM_SHUFFLE(0, 3, 3, 3));


	__m256d yy_xx_xx_xx = _mm256_mul_pd(yxxx, yxxx);
	__m256d zz_zz_yy_xx = _mm256_mul_pd(zzyx, zzyx);

	__m256d xz_xy_yz_xx = _mm256_mul_pd(xxyx, zyzx);
	__m256d yw_zw_xw_xx = _mm256_mul_pd(yzxx, wwwx);

	__m256d One = _mm256_set_pd(0.0, 1.0, 1.0, 1.0);
	__m256d Two = _mm256_set_pd(0.0, 2.0, 2.0, 2.0);

	__m256d A = _mm256_sub_pd(One, _mm256_mul_pd(Two, _mm256_add_pd(yy_xx_xx_xx, zz_zz_yy_xx)));
	__m256d B = _mm256_mul_pd(Two, _mm256_add_pd(xz_xy_yz_xx, yw_zw_xw_xx));
	__m256d C = _mm256_mul_pd(Two, _mm256_sub_pd(xz_xy_yz_xx, yw_zw_xw_xx));

	__m256d Bxxxx = _mm256_permute4x64_pd(B, _MM_SHUFFLE(0, 0, 0, 0));
	__m256d Byyww = _mm256_permute4x64_pd(B, _MM_SHUFFLE(3, 3, 1, 1));
	__m256d Bzzzz = _mm256_permute4x64_pd(B, _MM_SHUFFLE(2, 2, 2, 2));

	m_Row0 = _mm256_mul_pd(_mm256_blend_pd(_mm256_blend_pd(A, C, 0x2), Bxxxx, 0x4), Scale.xxxw().m_Data);
	m_Row1 = _mm256_mul_pd(_mm256_blend_pd(_mm256_blend_pd(Byyww, A, 0x2), C, 0x4), Scale.yyyw().m_Data);
	m_Row2 = _mm256_mul_pd(_mm256_blend_pd(_mm256_blend_pd(C, Bzzzz, 0x2), A, 0x4), Scale.zzzw().m_Data);
	m_Row3 = Pos.m_Data;
};

#endif