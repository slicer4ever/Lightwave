#include "LWCore/LWSVector.h"
#ifndef LW_NOAVX2

LWVector4<double> LWSVector4<double>::AsVec4(void) const {
	alignas(32) LWVector4<double> R;
	_mm256_store_pd(&R.x, m_Data);
	return R;
}

LWSVector4<double> LWSVector4<double>::Normalize(void) const {
	const double e = std::numeric_limits<double>::epsilon();
	__m256d eps = _mm256_set1_pd(e);
	//Do dot product:
	__m256d t = _mm256_mul_pd(m_Data, m_Data);
	t = _mm256_hadd_pd(t, t); //xy, xy, zw, zw
	t = _mm256_permute4x64_pd(t, _MM_SHUFFLE(0, 2, 1, 3));
	t = _mm256_hadd_pd(t, t);
	if (_mm256_test_all_ones(_mm256_castpd_si256(_mm256_cmp_pd(t, eps, _CMP_LE_OS)))) return _mm256_set1_pd(0.0);
	return _mm256_div_pd(m_Data, _mm256_sqrt_pd(t));
}

LWSVector4<double> LWSVector4<double>::Normalize3(void) const {
	const double e = std::numeric_limits<double>::epsilon();
	__m256d eps = _mm256_set1_pd(e);
	__m256d t = _mm256_mul_pd(m_Data, _mm256_set_pd(0.0, 1.0, 1.0, 1.0));
	t = _mm256_mul_pd(t, t);
	t = _mm256_hadd_pd(t, t);
	t = _mm256_permute4x64_pd(t, _MM_SHUFFLE(0, 2, 1, 3));
	t = _mm256_hadd_pd(t, t);
	if (_mm256_test_all_ones(_mm256_castpd_si256(_mm256_cmp_pd(t, eps, _CMP_LE_OS)))) return _mm256_set1_pd(0.0);
	return _mm256_div_pd(m_Data, _mm256_sqrt_pd(t));
}

LWSVector4<double> LWSVector4<double>::Normalize2(void) const {
	const double e = std::numeric_limits<double>::epsilon();
	__m256d eps = _mm256_set1_pd(e);
	__m256d t = _mm256_mul_pd(m_Data, _mm256_set_pd(0.0, 0.0, 1.0, 1.0));
	t = _mm256_mul_pd(t, t);
	t = _mm256_hadd_pd(t, t);
	if (_mm256_test_all_ones(_mm256_castpd_si256(_mm256_cmp_pd(t, eps, _CMP_LE_OS)))) return _mm256_set1_pd(0.0);
	return _mm256_div_pd(m_Data, _mm256_sqrt_pd(t));
}

double LWSVector4<double>::Dot(const LWSVector4<double>& O) const {
	__m256d t = _mm256_mul_pd(m_Data, O.m_Data);
	t = _mm256_hadd_pd(t, t);
	t = _mm256_permute4x64_pd(t, _MM_SHUFFLE(0, 2, 1, 3));
	t = _mm256_hadd_pd(t, t);
	return _mm256_cvtsd_f64(t);
}

double LWSVector4<double>::Dot3(const LWSVector4<double>& O) const {
	__m256d t = _mm256_set_pd(0.0, 1.0, 1.0, 1.0);
	t = _mm256_mul_pd(_mm256_mul_pd(m_Data, t), _mm256_mul_pd(O.m_Data, t));
	t = _mm256_hadd_pd(t, t);
	t = _mm256_permute4x64_pd(t, _MM_SHUFFLE(0, 2, 1, 3));
	t = _mm256_hadd_pd(t, t);
	return _mm256_cvtsd_f64(t);
}

double LWSVector4<double>::Dot2(const LWSVector4<double>& O) const {
	__m256d t = _mm256_mul_pd(m_Data, O.m_Data);
	return _mm256_cvtsd_f64(_mm256_hadd_pd(t, t));
}

LWSVector4<double> LWSVector4<double>::Sum(void) const {
	__m256d t = _mm256_hadd_pd(m_Data, m_Data);
	t = _mm256_permute4x64_pd(t, _MM_SHUFFLE(0, 2, 1, 3));
	return _mm256_hadd_pd(t, t);
}

double LWSVector4<double>::Sum4(void) const {
	__m256d t = _mm256_hadd_pd(m_Data, m_Data);
	t = _mm256_permute4x64_pd(t, _MM_SHUFFLE(0, 2, 1, 3));
	t = _mm256_hadd_pd(t, t);
	return _mm256_cvtsd_f64(t);
}

double LWSVector4<double>::Sum3(void) const {
	__m256d t = _mm256_mul_pd(m_Data, _mm256_set_pd(0.0, 1.0, 1.0, 1.0));
	t = _mm256_hadd_pd(t, t);
	t = _mm256_permute4x64_pd(t, _MM_SHUFFLE(0, 2, 1, 3));
	t = _mm256_hadd_pd(t, t);
	return _mm256_cvtsd_f64(t);
}

double LWSVector4<double>::Sum2(void) const {
	__m256d t = _mm256_hadd_pd(m_Data, m_Data);
	return _mm256_cvtsd_f64(t);
}

double LWSVector4<double>::Min(void) const {
	__m256d A = _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 1, 0, 3));
	__m256d B = _mm256_min_pd(m_Data, A);
	A = _mm256_permute4x64_pd(B, _MM_SHUFFLE(1, 0, 3, 2));
	return _mm256_cvtsd_f64(_mm256_min_pd(B, A));
}

double LWSVector4<double>::Min3(void) const {
	__m256d A = _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 1, 0, 2));
	__m256d B = _mm256_min_pd(m_Data, A);
	A = _mm256_permute4x64_pd(B, _MM_SHUFFLE(3, 0, 2, 1));
	return _mm256_cvtsd_f64(_mm256_min_pd(B, A));
}

double LWSVector4<double>::Min2(void) const {
	__m256d A = _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 2, 0, 1));
	return _mm256_cvtsd_f64(_mm256_min_pd(m_Data, A));
}

double LWSVector4<double>::Max(void) const {
	__m256d A = _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 1, 0, 3));
	__m256d B = _mm256_max_pd(m_Data, A);
	A = _mm256_permute4x64_pd(B, _MM_SHUFFLE(1, 0, 3, 2));
	return _mm256_cvtsd_f64(_mm256_max_pd(B, A));
}

double LWSVector4<double>::Max3(void) const {
	__m256d A = _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 1, 0, 2));
	__m256d B = _mm256_max_pd(m_Data, A);
	A = _mm256_permute4x64_pd(B, _MM_SHUFFLE(3, 0, 2, 1));
	return _mm256_cvtsd_f64(_mm256_max_pd(B, A));
}

double LWSVector4<double>::Max2(void) const {
	__m256d A = _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 2, 0, 1));
	return _mm256_cvtsd_f64(_mm256_max_pd(m_Data, A));
}

LWSVector4<double> LWSVector4<double>::Min(const LWSVector4<double>& A) const {
	return _mm256_min_pd(m_Data, A.m_Data);
}

LWSVector4<double> LWSVector4<double>::Max(const LWSVector4<double>& A) const {
	return _mm256_max_pd(m_Data, A.m_Data);
}

LWSVector4<double> LWSVector4<double>::Cross3(const LWSVector4<double>& O) const {
	__m256d A = yzxw().m_Data;
	__m256d B = O.zxyw().m_Data;
	__m256d C = zxyw().m_Data;
	__m256d D = O.yzxw().m_Data;
	return _mm256_sub_pd(_mm256_mul_pd(A, B), _mm256_mul_pd(C, D));
}

LWSVector4<double> LWSVector4<double>::Perpindicular2(void) const {
	return _mm256_xor_pd(yx().m_Data, _mm256_set_pd(0.0, 0.0, 0.0, -0.0));
}

double LWSVector4<double>::Length(void) const {
	__m256d t = _mm256_mul_pd(m_Data, m_Data);
	t = _mm256_hadd_pd(t, t);
	t = _mm256_permute4x64_pd(t, _MM_SHUFFLE(0, 2, 1, 3));
	t = _mm256_hadd_pd(t, t);
	return _mm256_cvtsd_f64(_mm256_sqrt_pd(t));
}

double LWSVector4<double>::Length3(void) const {
	__m256d t = _mm256_mul_pd(m_Data, _mm256_set_pd(0.0, 1.0, 1.0, 1.0));
	t = _mm256_mul_pd(t, t);
	t = _mm256_hadd_pd(t, t);
	t = _mm256_permute4x64_pd(t, _MM_SHUFFLE(0, 2, 1, 3));
	t = _mm256_hadd_pd(t, t);
	return _mm256_cvtsd_f64(_mm256_sqrt_pd(t));
}

double LWSVector4<double>::Length2(void) const {
	__m256d t = _mm256_mul_pd(m_Data, m_Data);
	t = _mm256_hadd_pd(t, t);
	return _mm256_cvtsd_f64(_mm256_sqrt_pd(t));
}

double LWSVector4<double>::LengthSquared(void) const {
	__m256d t = _mm256_mul_pd(m_Data, m_Data);
	t = _mm256_hadd_pd(t, t);
	t = _mm256_permute4x64_pd(t, _MM_SHUFFLE(0, 2, 1, 3));
	t = _mm256_hadd_pd(t, t);
	return _mm256_cvtsd_f64(t);
}

double LWSVector4<double>::LengthSquared3(void) const {
	__m256d t = _mm256_mul_pd(m_Data, _mm256_set_pd(0.0, 1.0, 1.0, 1.0));
	t = _mm256_hadd_pd(t, t);
	t = _mm256_permute4x64_pd(t, _MM_SHUFFLE(0, 2, 1, 3));
	t = _mm256_hadd_pd(t, t);
	return _mm256_cvtsd_f64(t);
}

double LWSVector4<double>::LengthSquared2(void) const {
	__m256d t = _mm256_mul_pd(m_Data, m_Data);
	t = _mm256_hadd_pd(t, t);
	return _mm256_cvtsd_f64(t);
}

double LWSVector4<double>::Distance(const LWSVector4<double>& O) const {
	__m256d t = _mm256_sub_pd(m_Data, O.m_Data);
	t = _mm256_mul_pd(t, t);
	t = _mm256_hadd_pd(t, t);
	t = _mm256_permute4x64_pd(t, _MM_SHUFFLE(0, 2, 1, 3));
	t = _mm256_hadd_pd(t, t);
	return _mm256_cvtsd_f64(_mm256_sqrt_pd(t));
}

double LWSVector4<double>::Distance3(const LWSVector4<double>& O) const {
	__m256d t = _mm256_sub_pd(m_Data, O.m_Data);
	t = _mm256_mul_pd(t, _mm256_set_pd(0.0, 1.0, 1.0, 1.0));
	t = _mm256_mul_pd(t, t);
	t = _mm256_hadd_pd(t, t);
	t = _mm256_permute4x64_pd(t, _MM_SHUFFLE(0, 2, 1, 3));
	t = _mm256_hadd_pd(t, t);
	return _mm256_cvtsd_f64(_mm256_sqrt_pd(t));
}

double LWSVector4<double>::Distance2(const LWSVector4<double>& O) const {
	__m256d t = _mm256_sub_pd(m_Data, O.m_Data);
	t = _mm256_mul_pd(t, t);
	t = _mm256_hadd_pd(t, t);
	return _mm256_cvtsd_f64(_mm256_sqrt_pd(t));
}

double LWSVector4<double>::DistanceSquared(const LWSVector4<double>& O) const {
	__m256d t = _mm256_sub_pd(m_Data, O.m_Data);
	t = _mm256_mul_pd(t, t);
	t = _mm256_hadd_pd(t, t);
	t = _mm256_permute4x64_pd(t, _MM_SHUFFLE(0, 2, 1, 3));
	t = _mm256_hadd_pd(t, t);
	return _mm256_cvtsd_f64(t);
}

double LWSVector4<double>::DistanceSquared3(const LWSVector4<double>& O) const {
	__m256d t = _mm256_sub_pd(m_Data, O.m_Data);
	t = _mm256_mul_pd(t, _mm256_set_pd(0.0, 1.0, 1.0, 1.0));
	t = _mm256_mul_pd(t, t);
	t = _mm256_hadd_pd(t, t);
	t = _mm256_permute4x64_pd(t, _MM_SHUFFLE(0, 2, 1, 3));
	t = _mm256_hadd_pd(t, t);
	return _mm256_cvtsd_f64(t);
}

double LWSVector4<double>::DistanceSquared2(const LWSVector4<double>& O) const {
	__m256d t = _mm256_sub_pd(m_Data, O.m_Data);
	t = _mm256_mul_pd(t, t);
	t = _mm256_hadd_pd(t, t);
	return _mm256_cvtsd_f64(t);
}

LWSVector4<double>& LWSVector4<double>::operator = (const LWSVector4<double>& Rhs) {
	m_Data = Rhs.m_Data;
	return *this;
}

LWSVector4<double>& LWSVector4<double>::operator += (const LWSVector4<double>& Rhs) {
	m_Data = _mm256_add_pd(m_Data, Rhs.m_Data);
	return *this;
}

LWSVector4<double>& LWSVector4<double>::operator += (double Rhs) {
	__m256d t = _mm256_set1_pd(Rhs);
	m_Data = _mm256_add_pd(m_Data, t);
	return *this;
}

LWSVector4<double>& LWSVector4<double>::operator -= (const LWSVector4<double>& Rhs) {
	m_Data = _mm256_sub_pd(m_Data, Rhs.m_Data);
	return *this;
}

LWSVector4<double>& LWSVector4<double>::operator -= (double Rhs) {
	__m256d t = _mm256_set1_pd(Rhs);
	m_Data = _mm256_sub_pd(m_Data, t);
	return *this;
}

LWSVector4<double>& LWSVector4<double>::operator *= (const LWSVector4<double>& Rhs) {
	m_Data = _mm256_mul_pd(m_Data, Rhs.m_Data);
	return *this;
}

LWSVector4<double>& LWSVector4<double>::operator *= (double Rhs) {
	__m256d t = _mm256_set1_pd(Rhs);
	m_Data = _mm256_mul_pd(m_Data, t);
	return *this;
}

LWSVector4<double>& LWSVector4<double>::operator /= (const LWSVector4<double>& Rhs) {
	m_Data = _mm256_div_pd(m_Data, Rhs.m_Data);
	return *this;
}

LWSVector4<double>& LWSVector4<double>::operator /= (double Rhs) {
	__m256d t = _mm256_set1_pd(Rhs);
	m_Data = _mm256_div_pd(m_Data, t);
	return *this;
}

LWSVector4<double> operator + (const LWSVector4<double>& Rhs) {
	return Rhs.m_Data;
}

LWSVector4<double> operator - (const LWSVector4<double>& Rhs) {
	__m256d t = _mm256_set1_pd(-0.0f);
	return _mm256_xor_pd(Rhs.m_Data, t);
}

bool LWSVector4<double>::operator == (const LWSVector4<double>& Rhs) const {
	//float episilon is "good" enough for closeness comparison.
	__m256d e = _mm256_set1_pd((double)std::numeric_limits<float>::epsilon());
	__m256d t = _mm256_sub_pd(m_Data, Rhs.m_Data);
	t = _mm256_andnot_pd(_mm256_set1_pd(-0.0), t); //Get absolute value of difference.
	t = _mm256_cmp_pd(t, e, _CMP_LE_OS);
	return _mm256_test_all_ones(_mm256_castpd_si256(t));
}

bool LWSVector4<double>::operator != (const LWSVector4<double>& Rhs) const {
	return !(*this == Rhs);
}

std::ostream& operator<<(std::ostream& o, const LWSVector4<double>& v) {
	alignas(32) double Values[4];
	_mm256_store_pd(Values, v.m_Data);
	o << Values[0] << " " << Values[1] << " " << Values[2] << " " << Values[3];
	return o;
}

LWSVector4<double> operator + (const LWSVector4<double>& Lhs, const LWSVector4<double>& Rhs) {
	return _mm256_add_pd(Lhs.m_Data, Rhs.m_Data);
}

LWSVector4<double> operator + (const LWSVector4<double>& Lhs, double Rhs) {
	__m256d t = _mm256_set1_pd(Rhs);
	return _mm256_add_pd(Lhs.m_Data, t);
}

LWSVector4<double> operator + (double Lhs, const LWSVector4<double>& Rhs) {
	__m256d t = _mm256_set1_pd(Lhs);
	return _mm256_add_pd(t, Rhs.m_Data);
}

LWSVector4<double> operator - (const LWSVector4<double>& Lhs, const LWSVector4<double>& Rhs) {
	return _mm256_sub_pd(Lhs.m_Data, Rhs.m_Data);
}

LWSVector4<double> operator - (const LWSVector4<double>& Lhs, double Rhs) {
	__m256d t = _mm256_set1_pd(Rhs);
	return _mm256_sub_pd(Lhs.m_Data, t);
}

LWSVector4<double> operator - (double Lhs, const LWSVector4<double>& Rhs) {
	__m256d t = _mm256_set1_pd(Lhs);
	return _mm256_sub_pd(t, Rhs.m_Data);
}

LWSVector4<double> operator * (const LWSVector4<double>& Lhs, const LWSVector4<double>& Rhs) {
	return _mm256_mul_pd(Lhs.m_Data, Rhs.m_Data);
}

LWSVector4<double> operator * (const LWSVector4<double>& Lhs, double Rhs) {
	__m256d t = _mm256_set1_pd(Rhs);
	return _mm256_mul_pd(Lhs.m_Data, t);
}

LWSVector4<double> operator * (double Lhs, const LWSVector4<double>& Rhs) {
	__m256d t = _mm256_set1_pd(Lhs);
	return _mm256_mul_pd(t, Rhs.m_Data);
}

LWSVector4<double> operator / (const LWSVector4<double>& Lhs, const LWSVector4<double>& Rhs) {
	return _mm256_div_pd(Lhs.m_Data, Rhs.m_Data);
}

LWSVector4<double> operator / (const LWSVector4<double>& Lhs, double Rhs) {
	__m256d t = _mm256_set1_pd(Rhs);
	return _mm256_div_pd(Lhs.m_Data, t);
}

LWSVector4<double> operator / (double Lhs, const LWSVector4<double>& Rhs) {
	__m256d t = _mm256_set1_pd(Lhs);
	return _mm256_div_pd(t, Rhs.m_Data);
}

LWSVector4<double> LWSVector4<double>::AAAB(const LWSVector4<double>& B) const {
	return _mm256_blend_pd(m_Data, B.m_Data, 0x8);
}

LWSVector4<double> LWSVector4<double>::AABA(const LWSVector4<double>& B) const {
	return _mm256_blend_pd(m_Data, B.m_Data, 0x4);
}

LWSVector4<double> LWSVector4<double>::AABB(const LWSVector4<double>& B) const {
	return _mm256_blend_pd(m_Data, B.m_Data, 0xC);
}

LWSVector4<double> LWSVector4<double>::ABAA(const LWSVector4<double>& B) const {
	return _mm256_blend_pd(m_Data, B.m_Data, 0x2);
}

LWSVector4<double> LWSVector4<double>::ABAB(const LWSVector4<double>& B) const {
	return _mm256_blend_pd(m_Data, B.m_Data, 0xA);
}

LWSVector4<double> LWSVector4<double>::ABBA(const LWSVector4<double>& B) const {
	return _mm256_blend_pd(m_Data, B.m_Data, 0x6);
}

LWSVector4<double> LWSVector4<double>::ABBB(const LWSVector4<double>& B) const {
	return _mm256_blend_pd(m_Data, B.m_Data, 0xE);
}

LWSVector4<double> LWSVector4<double>::BAAA(const LWSVector4<double>& B) const {
	return _mm256_blend_pd(m_Data, B.m_Data, 0x1);
}

LWSVector4<double> LWSVector4<double>::BAAB(const LWSVector4<double>& B) const {
	return _mm256_blend_pd(m_Data, B.m_Data, 0x9);
}

LWSVector4<double> LWSVector4<double>::BABA(const LWSVector4<double>& B) const {
	return _mm256_blend_pd(m_Data, B.m_Data, 0x5);
}

LWSVector4<double> LWSVector4<double>::BABB(const LWSVector4<double>& B) const {
	return _mm256_blend_pd(m_Data, B.m_Data, 0xD);
}

LWSVector4<double> LWSVector4<double>::BBAA(const LWSVector4<double>& B) const {
	return _mm256_blend_pd(m_Data, B.m_Data, 0x3);
}

LWSVector4<double> LWSVector4<double>::BBAB(const LWSVector4<double>& B) const {
	return _mm256_blend_pd(m_Data, B.m_Data, 0xB);
}

LWSVector4<double> LWSVector4<double>::BBBA(const LWSVector4<double>& B) const {
	return _mm256_blend_pd(m_Data, B.m_Data, 0x7);
}

LWSVector4<double> LWSVector4<double>::xxxx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 0, 0, 0));
}

LWSVector4<double> LWSVector4<double>::xxxy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 0, 0, 0));
}

LWSVector4<double> LWSVector4<double>::xxxz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 0, 0, 0));
}

LWSVector4<double> LWSVector4<double>::xxxw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 0, 0, 0));
}

LWSVector4<double> LWSVector4<double>::xxyx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 1, 0, 0));
}

LWSVector4<double> LWSVector4<double>::xxyy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 1, 0, 0));
}

LWSVector4<double> LWSVector4<double>::xxyz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 1, 0, 0));
}
LWSVector4<double> LWSVector4<double>::xxyw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 1, 0, 0));
}

LWSVector4<double> LWSVector4<double>::xxzx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 2, 0, 0));
}

LWSVector4<double> LWSVector4<double>::xxzy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 2, 0, 0));
}

LWSVector4<double> LWSVector4<double>::xxzz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 2, 0, 0));
}

LWSVector4<double> LWSVector4<double>::xxzw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 2, 0, 0));
}

LWSVector4<double> LWSVector4<double>::xxwx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 3, 0, 0));
}

LWSVector4<double> LWSVector4<double>::xxwy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 3, 0, 0));
}

LWSVector4<double> LWSVector4<double>::xxwz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 3, 0, 0));
}

LWSVector4<double> LWSVector4<double>::xxww(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 3, 0, 0));
}

LWSVector4<double> LWSVector4<double>::xyxx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 0, 1, 0));
}

LWSVector4<double> LWSVector4<double>::xyxy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 0, 1, 0));
}

LWSVector4<double> LWSVector4<double>::xyxz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 0, 1, 0));
}

LWSVector4<double> LWSVector4<double>::xyxw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 0, 1, 0));
}

LWSVector4<double> LWSVector4<double>::xyyx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 1, 1, 0));
}

LWSVector4<double> LWSVector4<double>::xyyy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 1, 1, 0));
}

LWSVector4<double> LWSVector4<double>::xyyz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 1, 1, 0));
}

LWSVector4<double> LWSVector4<double>::xyyw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 1, 1, 0));
}

LWSVector4<double> LWSVector4<double>::xyzx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 2, 1, 0));
}

LWSVector4<double> LWSVector4<double>::xyzy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 2, 1, 0));
}

LWSVector4<double> LWSVector4<double>::xyzz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 2, 1, 0));
}

LWSVector4<double> LWSVector4<double>::xywx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 3, 1, 0));
}

LWSVector4<double> LWSVector4<double>::xywy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 3, 1, 0));
}

LWSVector4<double> LWSVector4<double>::xywz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 3, 1, 0));
}

LWSVector4<double> LWSVector4<double>::xyww(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 3, 1, 0));
}

LWSVector4<double> LWSVector4<double>::xzxx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 0, 2, 0));
}

LWSVector4<double> LWSVector4<double>::xzxy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 0, 2, 0));
}

LWSVector4<double> LWSVector4<double>::xzxz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 0, 2, 0));
}

LWSVector4<double> LWSVector4<double>::xzxw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 0, 2, 0));
}

LWSVector4<double> LWSVector4<double>::xzyx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 1, 2, 0));
}

LWSVector4<double> LWSVector4<double>::xzyy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 1, 2, 0));
}

LWSVector4<double> LWSVector4<double>::xzyz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 1, 2, 0));
}

LWSVector4<double> LWSVector4<double>::xzyw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 1, 2, 0));
}

LWSVector4<double> LWSVector4<double>::xzzx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 2, 2, 0));
}

LWSVector4<double> LWSVector4<double>::xzzy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 2, 2, 0));
}

LWSVector4<double> LWSVector4<double>::xzzz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 2, 2, 0));
}

LWSVector4<double> LWSVector4<double>::xzzw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 2, 2, 0));
}

LWSVector4<double> LWSVector4<double>::xzwx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 3, 2, 0));
}

LWSVector4<double> LWSVector4<double>::xzwy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 3, 2, 0));
}

LWSVector4<double> LWSVector4<double>::xzwz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 3, 2, 0));
}

LWSVector4<double> LWSVector4<double>::xzww(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 3, 2, 0));
}

LWSVector4<double> LWSVector4<double>::xwxx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 0, 3, 0));
}

LWSVector4<double> LWSVector4<double>::xwxy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 0, 3, 0));
}

LWSVector4<double> LWSVector4<double>::xwxz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 0, 3, 0));
}

LWSVector4<double> LWSVector4<double>::xwxw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 0, 3, 0));
}

LWSVector4<double> LWSVector4<double>::xwyx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 1, 3, 0));
}

LWSVector4<double> LWSVector4<double>::xwyy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 1, 3, 0));
}

LWSVector4<double> LWSVector4<double>::xwyz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 1, 3, 0));
}

LWSVector4<double> LWSVector4<double>::xwyw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 1, 3, 0));
}

LWSVector4<double> LWSVector4<double>::xwzx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 2, 3, 0));
}

LWSVector4<double> LWSVector4<double>::xwzy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 2, 3, 0));
}

LWSVector4<double> LWSVector4<double>::xwzz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 2, 3, 0));
}

LWSVector4<double> LWSVector4<double>::xwzw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 2, 3, 0));
}

LWSVector4<double> LWSVector4<double>::xwwx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 3, 3, 0));
}

LWSVector4<double> LWSVector4<double>::xwwy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 3, 3, 0));
}

LWSVector4<double> LWSVector4<double>::xwwz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 3, 3, 0));
}

LWSVector4<double> LWSVector4<double>::xwww(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 3, 3, 0));
}

LWSVector4<double> LWSVector4<double>::yxxx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 0, 0, 1));
}

LWSVector4<double> LWSVector4<double>::yxxy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 0, 0, 1));
}

LWSVector4<double> LWSVector4<double>::yxxz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 0, 0, 1));
}

LWSVector4<double> LWSVector4<double>::yxxw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 0, 0, 1));
}

LWSVector4<double> LWSVector4<double>::yxyx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 1, 0, 1));
}

LWSVector4<double> LWSVector4<double>::yxyy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 1, 0, 1));
}

LWSVector4<double> LWSVector4<double>::yxyz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 1, 0, 1));
}

LWSVector4<double> LWSVector4<double>::yxyw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 1, 0, 1));
}

LWSVector4<double> LWSVector4<double>::yxzx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 2, 0, 1));
}

LWSVector4<double> LWSVector4<double>::yxzy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 2, 0, 1));
}

LWSVector4<double> LWSVector4<double>::yxzz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 2, 0, 1));
}

LWSVector4<double> LWSVector4<double>::yxzw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 2, 0, 1));
}

LWSVector4<double> LWSVector4<double>::yxwx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 3, 0, 1));
}

LWSVector4<double> LWSVector4<double>::yxwy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 3, 0, 1));
}

LWSVector4<double> LWSVector4<double>::yxwz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 3, 0, 1));
}

LWSVector4<double> LWSVector4<double>::yxww(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 3, 0, 1));
}

LWSVector4<double> LWSVector4<double>::yyxx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 0, 1, 1));
}

LWSVector4<double> LWSVector4<double>::yyxy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 0, 1, 1));
}

LWSVector4<double> LWSVector4<double>::yyxz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 0, 1, 1));
}

LWSVector4<double> LWSVector4<double>::yyxw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 0, 1, 1));
}

LWSVector4<double> LWSVector4<double>::yyyx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 1, 1, 1));
}

LWSVector4<double> LWSVector4<double>::yyyy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 1, 1, 1));
}

LWSVector4<double> LWSVector4<double>::yyyz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 1, 1, 1));
}

LWSVector4<double> LWSVector4<double>::yyyw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 1, 1, 1));
}

LWSVector4<double> LWSVector4<double>::yyzx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 2, 1, 1));
}

LWSVector4<double> LWSVector4<double>::yyzy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 2, 1, 1));
}

LWSVector4<double> LWSVector4<double>::yyzz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 2, 1, 1));
}

LWSVector4<double> LWSVector4<double>::yyzw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 2, 1, 1));
}

LWSVector4<double> LWSVector4<double>::yywx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 3, 1, 1));
}

LWSVector4<double> LWSVector4<double>::yywy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 3, 1, 1));
}

LWSVector4<double> LWSVector4<double>::yywz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 3, 1, 1));
}

LWSVector4<double> LWSVector4<double>::yyww(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 3, 1, 1));
}

LWSVector4<double> LWSVector4<double>::yzxx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 0, 2, 1));
}

LWSVector4<double> LWSVector4<double>::yzxy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 0, 2, 1));
}

LWSVector4<double> LWSVector4<double>::yzxz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 0, 2, 1));
}

LWSVector4<double> LWSVector4<double>::yzxw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 0, 2, 1));
}

LWSVector4<double> LWSVector4<double>::yzyx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 1, 2, 1));
}

LWSVector4<double> LWSVector4<double>::yzyy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 1, 2, 1));
}

LWSVector4<double> LWSVector4<double>::yzyz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 1, 2, 1));
}

LWSVector4<double> LWSVector4<double>::yzyw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 1, 2, 1));
}

LWSVector4<double> LWSVector4<double>::yzzx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 2, 2, 1));
}

LWSVector4<double> LWSVector4<double>::yzzy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 2, 2, 1));
}

LWSVector4<double> LWSVector4<double>::yzzz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 2, 2, 1));
}

LWSVector4<double> LWSVector4<double>::yzzw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 2, 2, 1));
}

LWSVector4<double> LWSVector4<double>::yzwx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 3, 2, 1));
}

LWSVector4<double> LWSVector4<double>::yzwy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 3, 2, 1));
}

LWSVector4<double> LWSVector4<double>::yzwz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 3, 2, 1));
}

LWSVector4<double> LWSVector4<double>::yzww(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 3, 2, 1));
}

LWSVector4<double> LWSVector4<double>::ywxx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 0, 3, 1));
}

LWSVector4<double> LWSVector4<double>::ywxy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 0, 3, 1));
}

LWSVector4<double> LWSVector4<double>::ywxz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 0, 3, 1));
}

LWSVector4<double> LWSVector4<double>::ywxw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 0, 3, 1));
}

LWSVector4<double> LWSVector4<double>::ywyx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 1, 3, 1));
}

LWSVector4<double> LWSVector4<double>::ywyy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 1, 3, 1));
}

LWSVector4<double> LWSVector4<double>::ywyz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 1, 3, 1));
}

LWSVector4<double> LWSVector4<double>::ywyw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 1, 3, 1));
}

LWSVector4<double> LWSVector4<double>::ywzx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 2, 3, 1));
}

LWSVector4<double> LWSVector4<double>::ywzy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 2, 3, 1));
}

LWSVector4<double> LWSVector4<double>::ywzz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 2, 3, 1));
}

LWSVector4<double> LWSVector4<double>::ywzw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 2, 3, 1));
}

LWSVector4<double> LWSVector4<double>::ywwx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 3, 3, 1));
}

LWSVector4<double> LWSVector4<double>::ywwy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 3, 3, 1));
}

LWSVector4<double> LWSVector4<double>::ywwz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 3, 3, 1));
}

LWSVector4<double> LWSVector4<double>::ywww(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 3, 3, 1));
}

LWSVector4<double> LWSVector4<double>::zxxx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 0, 0, 2));
}

LWSVector4<double> LWSVector4<double>::zxxy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 0, 0, 2));
}

LWSVector4<double> LWSVector4<double>::zxxz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 0, 0, 2));
}

LWSVector4<double> LWSVector4<double>::zxxw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 0, 0, 2));
}

LWSVector4<double> LWSVector4<double>::zxyx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 1, 0, 2));
}

LWSVector4<double> LWSVector4<double>::zxyy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 1, 0, 2));
}

LWSVector4<double> LWSVector4<double>::zxyz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 1, 0, 2));
}

LWSVector4<double> LWSVector4<double>::zxyw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 1, 0, 2));
}

LWSVector4<double> LWSVector4<double>::zxzx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 2, 0, 2));
}

LWSVector4<double> LWSVector4<double>::zxzy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 2, 0, 2));
}

LWSVector4<double> LWSVector4<double>::zxzz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 2, 0, 2));
}

LWSVector4<double> LWSVector4<double>::zxzw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 2, 0, 2));
}

LWSVector4<double> LWSVector4<double>::zxwx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 3, 0, 2));
}

LWSVector4<double> LWSVector4<double>::zxwy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 3, 0, 2));
}

LWSVector4<double> LWSVector4<double>::zxwz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 3, 0, 2));
}

LWSVector4<double> LWSVector4<double>::zxww(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 3, 0, 2));
}

LWSVector4<double> LWSVector4<double>::zyxx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 0, 1, 2));
}

LWSVector4<double> LWSVector4<double>::zyxy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 0, 1, 2));
}

LWSVector4<double> LWSVector4<double>::zyxz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 0, 1, 2));
}

LWSVector4<double> LWSVector4<double>::zyxw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 0, 1, 2));
}

LWSVector4<double> LWSVector4<double>::zyyx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 1, 1, 2));
}

LWSVector4<double> LWSVector4<double>::zyyy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 1, 1, 2));
}

LWSVector4<double> LWSVector4<double>::zyyz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 1, 1, 2));
}

LWSVector4<double> LWSVector4<double>::zyyw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 1, 1, 2));
}

LWSVector4<double> LWSVector4<double>::zyzx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 2, 1, 2));
}

LWSVector4<double> LWSVector4<double>::zyzy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 2, 1, 2));
}

LWSVector4<double> LWSVector4<double>::zyzz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 2, 1, 2));
}

LWSVector4<double> LWSVector4<double>::zyzw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 2, 1, 2));
}

LWSVector4<double> LWSVector4<double>::zywx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 3, 1, 2));
}

LWSVector4<double> LWSVector4<double>::zywy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 3, 1, 2));
}

LWSVector4<double> LWSVector4<double>::zywz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 3, 1, 2));
}

LWSVector4<double> LWSVector4<double>::zyww(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 3, 1, 2));
}

LWSVector4<double> LWSVector4<double>::zzxx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 0, 2, 2));
}

LWSVector4<double> LWSVector4<double>::zzxy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 0, 2, 2));
}

LWSVector4<double> LWSVector4<double>::zzxz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 0, 2, 2));
}

LWSVector4<double> LWSVector4<double>::zzxw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 0, 2, 2));
}

LWSVector4<double> LWSVector4<double>::zzyx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 1, 2, 2));
}

LWSVector4<double> LWSVector4<double>::zzyy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 1, 2, 2));
}

LWSVector4<double> LWSVector4<double>::zzyz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 1, 2, 2));
}

LWSVector4<double> LWSVector4<double>::zzyw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 1, 2, 2));
}

LWSVector4<double> LWSVector4<double>::zzzx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 2, 2, 2));
}

LWSVector4<double> LWSVector4<double>::zzzy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 2, 2, 2));
}

LWSVector4<double> LWSVector4<double>::zzzz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 2, 2, 2));
}

LWSVector4<double> LWSVector4<double>::zzzw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 2, 2, 2));
}

LWSVector4<double> LWSVector4<double>::zzwx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 3, 2, 2));
}

LWSVector4<double> LWSVector4<double>::zzwy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 3, 2, 2));
}

LWSVector4<double> LWSVector4<double>::zzwz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 3, 2, 2));
}

LWSVector4<double> LWSVector4<double>::zzww(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 3, 2, 2));
}

LWSVector4<double> LWSVector4<double>::zwxx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 0, 3, 2));
}

LWSVector4<double> LWSVector4<double>::zwxy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 0, 3, 2));
}

LWSVector4<double> LWSVector4<double>::zwxz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 0, 3, 2));
}

LWSVector4<double> LWSVector4<double>::zwxw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 0, 3, 2));
}

LWSVector4<double> LWSVector4<double>::zwyx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 1, 3, 2));
}

LWSVector4<double> LWSVector4<double>::zwyy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 1, 3, 2));
}

LWSVector4<double> LWSVector4<double>::zwyz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 1, 3, 2));
}

LWSVector4<double> LWSVector4<double>::zwyw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 1, 3, 2));
}

LWSVector4<double> LWSVector4<double>::zwzx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 2, 3, 2));
}

LWSVector4<double> LWSVector4<double>::zwzy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 2, 3, 2));
}

LWSVector4<double> LWSVector4<double>::zwzz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 2, 3, 2));
}

LWSVector4<double> LWSVector4<double>::zwzw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 2, 3, 2));
}

LWSVector4<double> LWSVector4<double>::zwwx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 3, 3, 2));
}

LWSVector4<double> LWSVector4<double>::zwwy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 3, 3, 2));
}

LWSVector4<double> LWSVector4<double>::zwwz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 3, 3, 2));
}

LWSVector4<double> LWSVector4<double>::zwww(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 3, 3, 2));
}

LWSVector4<double> LWSVector4<double>::wxxx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 0, 0, 3));
}

LWSVector4<double> LWSVector4<double>::wxxy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 0, 0, 3));
}

LWSVector4<double> LWSVector4<double>::wxxz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 0, 0, 3));
}

LWSVector4<double> LWSVector4<double>::wxxw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 0, 0, 3));
}

LWSVector4<double> LWSVector4<double>::wxyx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 1, 0, 3));
}

LWSVector4<double> LWSVector4<double>::wxyy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 1, 0, 3));
}

LWSVector4<double> LWSVector4<double>::wxyz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 1, 0, 3));
}

LWSVector4<double> LWSVector4<double>::wxyw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 1, 0, 3));
}

LWSVector4<double> LWSVector4<double>::wxzx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 2, 0, 3));
}

LWSVector4<double> LWSVector4<double>::wxzy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 2, 0, 3));
}

LWSVector4<double> LWSVector4<double>::wxzz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 2, 0, 3));
}

LWSVector4<double> LWSVector4<double>::wxzw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 2, 0, 3));
}

LWSVector4<double> LWSVector4<double>::wxwx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 3, 0, 3));
}

LWSVector4<double> LWSVector4<double>::wxwy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 3, 0, 3));
}

LWSVector4<double> LWSVector4<double>::wxwz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 3, 0, 3));
}

LWSVector4<double> LWSVector4<double>::wxww(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 3, 0, 3));
}

LWSVector4<double> LWSVector4<double>::wyxx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 0, 1, 3));
}

LWSVector4<double> LWSVector4<double>::wyxy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 0, 1, 3));
}

LWSVector4<double> LWSVector4<double>::wyxz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 0, 1, 3));
}

LWSVector4<double> LWSVector4<double>::wyxw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 0, 1, 3));
}

LWSVector4<double> LWSVector4<double>::wyyx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 1, 1, 3));
}

LWSVector4<double> LWSVector4<double>::wyyy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 1, 1, 3));
}

LWSVector4<double> LWSVector4<double>::wyyz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 1, 1, 3));
}

LWSVector4<double> LWSVector4<double>::wyyw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 1, 1, 3));
}

LWSVector4<double> LWSVector4<double>::wyzx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 2, 1, 3));
}

LWSVector4<double> LWSVector4<double>::wyzy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 2, 1, 3));
}

LWSVector4<double> LWSVector4<double>::wyzz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 2, 1, 3));
}

LWSVector4<double> LWSVector4<double>::wyzw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 2, 1, 3));
}

LWSVector4<double> LWSVector4<double>::wywx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 3, 1, 3));
}

LWSVector4<double> LWSVector4<double>::wywy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 3, 1, 3));
}

LWSVector4<double> LWSVector4<double>::wywz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 3, 1, 3));
}

LWSVector4<double> LWSVector4<double>::wyww(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 3, 1, 3));
}

LWSVector4<double> LWSVector4<double>::wzxx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 0, 2, 3));
}

LWSVector4<double> LWSVector4<double>::wzxy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 0, 2, 3));
}

LWSVector4<double> LWSVector4<double>::wzxz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 0, 2, 3));
}

LWSVector4<double> LWSVector4<double>::wzxw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 0, 2, 3));
}

LWSVector4<double> LWSVector4<double>::wzyx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 1, 2, 3));
}

LWSVector4<double> LWSVector4<double>::wzyy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 1, 2, 3));
}

LWSVector4<double> LWSVector4<double>::wzyz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 1, 2, 3));
}

LWSVector4<double> LWSVector4<double>::wzyw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 1, 2, 3));
}

LWSVector4<double> LWSVector4<double>::wzzx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 2, 2, 3));
}

LWSVector4<double> LWSVector4<double>::wzzy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 2, 2, 3));
}

LWSVector4<double> LWSVector4<double>::wzzz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 2, 2, 3));
}

LWSVector4<double> LWSVector4<double>::wzzw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 2, 2, 3));
}

LWSVector4<double> LWSVector4<double>::wzwx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 3, 2, 3));
}

LWSVector4<double> LWSVector4<double>::wzwy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 3, 2, 3));
}

LWSVector4<double> LWSVector4<double>::wzwz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 3, 2, 3));
}

LWSVector4<double> LWSVector4<double>::wzww(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 3, 2, 3));
}

LWSVector4<double> LWSVector4<double>::wwxx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 0, 3, 3));
}

LWSVector4<double> LWSVector4<double>::wwxy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 0, 3, 3));
}

LWSVector4<double> LWSVector4<double>::wwxz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 0, 3, 3));
}

LWSVector4<double> LWSVector4<double>::wwxw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 0, 3, 3));
}

LWSVector4<double> LWSVector4<double>::wwyx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 1, 3, 3));
}

LWSVector4<double> LWSVector4<double>::wwyy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 1, 3, 3));
}

LWSVector4<double> LWSVector4<double>::wwyz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 1, 3, 3));
}

LWSVector4<double> LWSVector4<double>::wwyw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 1, 3, 3));
}

LWSVector4<double> LWSVector4<double>::wwzx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 2, 3, 3));
}

LWSVector4<double> LWSVector4<double>::wwzy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 2, 3, 3));
}

LWSVector4<double> LWSVector4<double>::wwzz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 2, 3, 3));
}

LWSVector4<double> LWSVector4<double>::wwzw(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 2, 3, 3));
}

LWSVector4<double> LWSVector4<double>::wwwx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 3, 3, 3));
}

LWSVector4<double> LWSVector4<double>::wwwy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 3, 3, 3));
}

LWSVector4<double> LWSVector4<double>::wwwz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 3, 3, 3));
}

LWSVector4<double> LWSVector4<double>::wwww(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 3, 3, 3));
}

LWSVector4<double> LWSVector4<double>::xxx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 0, 0, 0));
}

LWSVector4<double> LWSVector4<double>::xxy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 1, 0, 0));
}

LWSVector4<double> LWSVector4<double>::xxz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 2, 0, 0));
}

LWSVector4<double> LWSVector4<double>::xyx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 0, 1, 0));
}

LWSVector4<double> LWSVector4<double>::xyy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 1, 1, 0));
}

LWSVector4<double> LWSVector4<double>::xyz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 2, 1, 0));
}

LWSVector4<double> LWSVector4<double>::xzx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 0, 2, 0));
}

LWSVector4<double> LWSVector4<double>::xzy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 1, 2, 0));
}

LWSVector4<double> LWSVector4<double>::xzz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 2, 2, 0));
}

LWSVector4<double> LWSVector4<double>::yxx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 0, 0, 1));
}

LWSVector4<double> LWSVector4<double>::yxy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 1, 0, 1));
}

LWSVector4<double> LWSVector4<double>::yxz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 2, 0, 1));
}

LWSVector4<double> LWSVector4<double>::yyx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 0, 1, 1));
}

LWSVector4<double> LWSVector4<double>::yyy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 1, 1, 1));
}

LWSVector4<double> LWSVector4<double>::yyz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 2, 1, 1));
}

LWSVector4<double> LWSVector4<double>::yzx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 0, 2, 1));
}

LWSVector4<double> LWSVector4<double>::yzy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 1, 2, 1));
}

LWSVector4<double> LWSVector4<double>::yzz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 2, 2, 1));
}

LWSVector4<double> LWSVector4<double>::zxx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 0, 0, 2));
}

LWSVector4<double> LWSVector4<double>::zxy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 1, 0, 2));
}

LWSVector4<double> LWSVector4<double>::zxz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 2, 0, 2));
}

LWSVector4<double> LWSVector4<double>::zyx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 0, 1, 2));
}

LWSVector4<double> LWSVector4<double>::zyy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 1, 1, 2));
}

LWSVector4<double> LWSVector4<double>::zyz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 2, 1, 2));
}

LWSVector4<double> LWSVector4<double>::zzx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 0, 2, 2));
}

LWSVector4<double> LWSVector4<double>::zzy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 1, 2, 2));
}

LWSVector4<double> LWSVector4<double>::zzz(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 2, 2, 2));
}

LWSVector4<double> LWSVector4<double>::xx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 2, 0, 0));
}

LWSVector4<double> LWSVector4<double>::xy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 2, 1, 0));
}

LWSVector4<double> LWSVector4<double>::yx(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 2, 0, 1));
}

LWSVector4<double> LWSVector4<double>::yy(void) const {
	return _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 2, 1, 1));
}

double LWSVector4<double>::x(void) const {
	return _mm256_cvtsd_f64(xxxx().m_Data);
}

double LWSVector4<double>::y(void) const {
	return _mm256_cvtsd_f64(yyyy().m_Data);
}

double LWSVector4<double>::z(void) const {
	return _mm256_cvtsd_f64(zzzz().m_Data);
}

double LWSVector4<double>::w(void) const {
	return _mm256_cvtsd_f64(wwww().m_Data);
}

LWSVector4<double>::LWSVector4(const __m256d Data) : m_Data(Data) {}

LWSVector4<double>::LWSVector4(const LWVector4<double>& vxyzw) : m_Data(_mm256_set_pd(vxyzw.w, vxyzw.z, vxyzw.y, vxyzw.x)) {}

LWSVector4<double>::LWSVector4(const LWVector3<double>& vxyz, double vw) : m_Data(_mm256_set_pd(vw, vxyz.z, vxyz.y, vxyz.x)) {}

LWSVector4<double>::LWSVector4(const LWVector2<double>& vxy, const LWVector2<double>& vzw) : m_Data(_mm256_set_pd(vzw.y, vzw.x, vxy.y, vxy.x)) {}

LWSVector4<double>::LWSVector4(const LWVector2<double>& vxy, double vz, double vw) : m_Data(_mm256_set_pd(vw, vz, vxy.y, vxy.x)) {}

LWSVector4<double>::LWSVector4(double vx, double vy, double vz, double vw) : m_Data(_mm256_set_pd(vw, vz, vy, vx)) {}

LWSVector4<double>::LWSVector4(double f) : m_Data(_mm256_set_pd(f, f, f, f)) {}

#endif