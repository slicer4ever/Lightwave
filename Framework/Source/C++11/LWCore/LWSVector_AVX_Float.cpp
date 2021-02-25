#include "LWCore/LWSVector.h"
#include <numeric>
#ifdef __AVX__

LWVector4<float> LWSVector4<float>::AsVec4(void) const {
	alignas(16) LWVector4<float> R;
	_mm_store_ps(&R.x, m_Data);
	return R;
}

LWSVector4<float> LWSVector4<float>::Sign(void) const {
	__m128 c = _mm_cmplt_ps(m_Data, _mm_setzero_ps());
	return _mm_blendv_ps(_mm_set1_ps(1.0f), _mm_set1_ps(-1.0f), c);
}

LWSVector4<float> LWSVector4<float>::Normalize(void) const {
	const float e = std::numeric_limits<float>::epsilon();
	__m128 eps = _mm_set_ps1(e);
	__m128 t = _mm_dp_ps(m_Data, m_Data, 0xFF);
	if (_mm_test_all_ones(_mm_castps_si128(_mm_cmple_ps(t, eps)))) return _mm_set_ps1(0.0f);
	return _mm_div_ps(m_Data, _mm_sqrt_ps(t));
}

LWSVector4<float> LWSVector4<float>::Normalize3(void) const {
	const float e = std::numeric_limits<float>::epsilon();
	__m128 eps = _mm_set_ps1(e);
	__m128 t = _mm_dp_ps(m_Data, m_Data, 0x7F);
	if (_mm_test_all_ones(_mm_castps_si128(_mm_cmple_ps(t, eps)))) return _mm_set_ps1(0.0f);
	return _mm_div_ps(m_Data, _mm_sqrt_ps(t));
}

LWSVector4<float> LWSVector4<float>::Normalize2(void) const {
	const float e = std::numeric_limits<float>::epsilon();
	__m128 eps = _mm_set_ps1(e);
	__m128 t = _mm_dp_ps(m_Data, m_Data, 0x3f);
	if (_mm_test_all_ones(_mm_castps_si128(_mm_cmple_ps(t, eps)))) return _mm_set_ps1(0.0f);
	return _mm_div_ps(m_Data, _mm_sqrt_ps(t));
}

float LWSVector4<float>::Dot(const LWSVector4<float>& O) const {
	return _mm_cvtss_f32(_mm_dp_ps(m_Data, O.m_Data, 0xFF));
}

float LWSVector4<float>::Dot3(const LWSVector4<float>& O) const {
	__m128 t = _mm_dp_ps(m_Data, O.m_Data, 0x7F);
	return _mm_cvtss_f32(_mm_dp_ps(m_Data, O.m_Data, 0x7F));
}

float LWSVector4<float>::Dot2(const LWSVector4<float>& O) const {
	return _mm_cvtss_f32(_mm_dp_ps(m_Data, O.m_Data, 0x3F));
}

LWSVector4<float> LWSVector4<float>::Sum(void) const {
	__m128 t = _mm_hadd_ps(m_Data, m_Data);
	return _mm_hadd_ps(t, t);
}

float LWSVector4<float>::Sum4(void) const {
	__m128 t = _mm_hadd_ps(m_Data, m_Data);
	t = _mm_hadd_ps(t, t);
	return _mm_cvtss_f32(t);
}

float LWSVector4<float>::Sum3(void) const {
	__m128 t = _mm_mul_ps(m_Data, _mm_set_ps(0.0f, 1.0f, 1.0f, 1.0f));
	t = _mm_hadd_ps(t, t);
	return _mm_cvtss_f32(_mm_hadd_ps(t, t));
}

float LWSVector4<float>::Sum2(void) const {
	return _mm_cvtss_f32(_mm_hadd_ps(m_Data, m_Data));
}

float LWSVector4<float>::Min(void) const {
	__m128 A = _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 1, 0, 3));
	__m128 B = _mm_min_ps(m_Data, A);
	A = _mm_permute_ps(B, _MM_SHUFFLE(1, 0, 3, 2));
	return _mm_cvtss_f32(_mm_min_ps(B, A));
}

float LWSVector4<float>::Min3(void) const {
	__m128 A = _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 0, 2));
	__m128 B = _mm_min_ps(m_Data, A);
	A = _mm_permute_ps(B, _MM_SHUFFLE(3, 0, 2, 1));
	return _mm_cvtss_f32(_mm_min_ps(B, A));
}

float LWSVector4<float>::Min2(void) const {
	__m128 A = _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 0, 1));
	return _mm_cvtss_f32(_mm_min_ps(m_Data, A));
}

float LWSVector4<float>::Max(void) const {
	__m128 A = _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 1, 0, 3));
	__m128 B = _mm_max_ps(m_Data, A);
	A = _mm_permute_ps(B, _MM_SHUFFLE(1, 0, 3, 2));
	return _mm_cvtss_f32(_mm_max_ps(B, A));
}

float LWSVector4<float>::Max3(void) const {
	__m128 A = _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 0, 2));
	__m128 B = _mm_max_ps(m_Data, A);
	A = _mm_permute_ps(B, _MM_SHUFFLE(3, 0, 2, 1));
	return _mm_cvtss_f32(_mm_max_ps(B, A));
}

float LWSVector4<float>::Max2(void) const {
	__m128 A = _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 0, 1));
	return _mm_cvtss_f32(_mm_max_ps(m_Data, A));
}

LWSVector4<float> LWSVector4<float>::Min(const LWSVector4<float>& A) const {
	return _mm_min_ps(m_Data, A.m_Data);
}

LWSVector4<float> LWSVector4<float>::Max(const LWSVector4<float>& A) const {
	return _mm_max_ps(m_Data, A.m_Data);
}

LWSVector4<float> LWSVector4<float>::Cross3(const LWSVector4<float>& O) const {
	__m128 A = yzxw().m_Data;
	__m128 B = O.zxyw().m_Data;
	__m128 C = zxyw().m_Data;
	__m128 D = O.yzxw().m_Data;
	return _mm_sub_ps(_mm_mul_ps(A, B), _mm_mul_ps(C, D));
}

void LWSVector4<float>::Orthogonal3(LWSVector4<float> &Right, LWSVector4<float> &Up) const {
	const LWSVector4<float> XAxis = LWSVector4<float>(1.0f, 0.0f, 0.0f, 0.0f);
	const LWSVector4<float> YAxis = LWSVector4<float>(0.0f, 1.0f, 0.0f, 0.0f);
	LWSVector4<float> A = XAxis;
	float d = fabs(Dot3(A));
	if (d > 0.8f) A = YAxis;
	Right = Cross3(A).Normalize3().AAAB(*this);
	Up = Cross3(Right).AAAB(*this);
	return;
};

LWSVector4<float> LWSVector4<float>::Perpindicular2(void) const {
	return _mm_xor_ps(yx().m_Data, _mm_set_ps(0.0f, 0.0f, 0.0f, -0.0f));
}

float LWSVector4<float>::Length(void) const {
	__m128 t = _mm_dp_ps(m_Data, m_Data, 0xFF);
	return _mm_cvtss_f32(_mm_sqrt_ps(t));
}

float LWSVector4<float>::Length3(void) const {
	__m128 t = _mm_dp_ps(m_Data, m_Data, 0x7F);
	return _mm_cvtss_f32(_mm_sqrt_ps(t));
}

float LWSVector4<float>::Length2(void) const {
	__m128 t = _mm_dp_ps(m_Data, m_Data, 0x3F);
	return _mm_cvtss_f32(_mm_sqrt_ps(t));
}

float LWSVector4<float>::LengthSquared(void) const {
	return _mm_cvtss_f32(_mm_dp_ps(m_Data, m_Data, 0xFF));
}

float LWSVector4<float>::LengthSquared3(void) const {
	return _mm_cvtss_f32(_mm_dp_ps(m_Data, m_Data, 0x7F));
}

float LWSVector4<float>::LengthSquared2(void) const {
	return _mm_cvtss_f32(_mm_dp_ps(m_Data, m_Data, 0x3F));
}

float LWSVector4<float>::Distance(const LWSVector4<float>& O) const {
	__m128 t = _mm_sub_ps(m_Data, O.m_Data);
	t = _mm_dp_ps(t, t, 0xFF);
	return _mm_cvtss_f32(_mm_sqrt_ps(t));
}

float LWSVector4<float>::Distance3(const LWSVector4<float>& O) const {
	__m128 t = _mm_sub_ps(m_Data, O.m_Data);
	t = _mm_dp_ps(t, t, 0x7F);
	return _mm_cvtss_f32(_mm_sqrt_ps(t));
}

float LWSVector4<float>::Distance2(const LWSVector4<float>& O) const {
	__m128 t = _mm_sub_ps(m_Data, O.m_Data);
	t = _mm_dp_ps(t, t, 0x3f);
	return _mm_cvtss_f32(_mm_sqrt_ps(t));
}

float LWSVector4<float>::DistanceSquared(const LWSVector4<float>& O) const {
	__m128 t = _mm_sub_ps(m_Data, O.m_Data);
	return _mm_cvtss_f32(_mm_dp_ps(t, t, 0xFF));
}

float LWSVector4<float>::DistanceSquared3(const LWSVector4<float>& O) const {
	__m128 t = _mm_sub_ps(m_Data, O.m_Data);
	return _mm_cvtss_f32(_mm_dp_ps(t, t, 0x7F));
}

float LWSVector4<float>::DistanceSquared2(const LWSVector4<float>& O) const {
	__m128 t = _mm_sub_ps(m_Data, O.m_Data);
	return _mm_cvtss_f32(_mm_dp_ps(t, t, 0x3F));
}

LWSVector4<float> LWSVector4<float>::Abs(void) const {
	return _mm_andnot_ps(_mm_set_ps1(-0.0f), m_Data);
}

LWSVector4<float> LWSVector4<float>::Abs3(void) const {
	return _mm_andnot_ps(_mm_set_ps(0.0f, -0.0f, -0.0f, -0.0f), m_Data);
}

LWSVector4<float> LWSVector4<float>::Abs2(void) const {
	return _mm_andnot_ps(_mm_set_ps(0.0f, 0.0f, -0.0f, -0.0f), m_Data);
}

LWSVector4<float> LWSVector4<float>::Blend_Less(const LWSVector4<float> &Rhs, const LWSVector4<float> &Value) const {
	__m128 c = _mm_cmplt_ps(m_Data, Rhs.m_Data);
	return _mm_blendv_ps(m_Data, Value.m_Data, c);
}

LWSVector4<float> LWSVector4<float>::Blend_Less3(const LWSVector4<float> &Rhs, const LWSVector4<float> &Value) const {
	__m128 c = _mm_cmplt_ps(m_Data, Rhs.m_Data);
	return _mm_blend_ps(_mm_blendv_ps(m_Data, Value.m_Data, c), m_Data, 0x8);
}

LWSVector4<float> LWSVector4<float>::Blend_Less2(const LWSVector4<float> &Rhs, const LWSVector4<float> &Value) const {
	__m128 c = _mm_cmplt_ps(m_Data, Rhs.m_Data);
	return _mm_blend_ps(_mm_blendv_ps(m_Data, Value.m_Data, c), m_Data, 0xC);
}

LWSVector4<float> LWSVector4<float>::Blend_LessEqual(const LWSVector4<float> &Rhs, const LWSVector4<float> &Value) const {
	__m128 c = _mm_cmple_ps(m_Data, Rhs.m_Data);
	return _mm_blendv_ps(m_Data, Value.m_Data, c);
}

LWSVector4<float> LWSVector4<float>::Blend_LessEqual3(const LWSVector4<float> &Rhs, const LWSVector4<float> &Value) const {
	__m128 c = _mm_cmple_ps(m_Data, Rhs.m_Data);
	return _mm_blend_ps(_mm_blendv_ps(m_Data, Value.m_Data, c), m_Data, 0x8);
}

LWSVector4<float> LWSVector4<float>::Blend_LessEqual2(const LWSVector4<float> &Rhs, const LWSVector4<float> &Value) const {
	__m128 c = _mm_cmple_ps(m_Data, Rhs.m_Data);
	return _mm_blend_ps(_mm_blendv_ps(m_Data, Value.m_Data, c), m_Data, 0xC);
}

LWSVector4<float> LWSVector4<float>::Blend_Greater(const LWSVector4<float> &Rhs, const LWSVector4<float> &Value) const {
	__m128 c = _mm_cmpgt_ps(m_Data, Rhs.m_Data);
	return _mm_blendv_ps(m_Data, Value.m_Data, c);
}

LWSVector4<float> LWSVector4<float>::Blend_Greater3(const LWSVector4<float> &Rhs, const LWSVector4<float> &Value) const {
	__m128 c = _mm_cmpgt_ps(m_Data, Rhs.m_Data);
	return _mm_blend_ps(_mm_blendv_ps(m_Data, Value.m_Data, c), m_Data, 0x8);
}

LWSVector4<float> LWSVector4<float>::Blend_Greater2(const LWSVector4<float> &Rhs, const LWSVector4<float> &Value) const {
	__m128 c = _mm_cmpgt_ps(m_Data, Rhs.m_Data);
	return _mm_blend_ps(_mm_blendv_ps(m_Data, Value.m_Data, c), m_Data, 0xC);
}

LWSVector4<float> LWSVector4<float>::Blend_GreaterEqual(const LWSVector4<float> &Rhs, const LWSVector4<float> &Value) const {
	__m128 c = _mm_cmpge_ps(m_Data, Rhs.m_Data);
	return _mm_blendv_ps(m_Data, Value.m_Data, c);
}

LWSVector4<float> LWSVector4<float>::Blend_GreaterEqual3(const LWSVector4<float> &Rhs, const LWSVector4<float> &Value) const {
	__m128 c = _mm_cmpge_ps(m_Data, Rhs.m_Data);
	return _mm_blend_ps(_mm_blendv_ps(m_Data, Value.m_Data, c), m_Data, 0x8);
}

LWSVector4<float> LWSVector4<float>::Blend_GreaterEqual2(const LWSVector4<float> &Rhs, const LWSVector4<float> &Value) const {
	__m128 c = _mm_cmpge_ps(m_Data, Rhs.m_Data);
	return _mm_blend_ps(_mm_blendv_ps(m_Data, Value.m_Data, c), m_Data, 0xC);
}

LWSVector4<float> LWSVector4<float>::Blend_Equal(const LWSVector4<float> &Rhs, const LWSVector4<float> &Value) const {
	__m128 e = _mm_set_ps1(std::numeric_limits<float>::epsilon());
	__m128 d = _mm_sub_ps(m_Data, Rhs.m_Data);
	d = _mm_andnot_ps(_mm_set_ps1(-0.0f), d);//get absolute value.
	__m128 c = _mm_cmple_ps(d, e);
	return _mm_blendv_ps(m_Data, Value.m_Data, c);
}

LWSVector4<float> LWSVector4<float>::Blend_Equal3(const LWSVector4<float> &Rhs, const LWSVector4<float> &Value) const {
	__m128 e = _mm_set_ps1(std::numeric_limits<float>::epsilon());
	__m128 d = _mm_sub_ps(m_Data, Rhs.m_Data);
	d = _mm_andnot_ps(_mm_set_ps1(-0.0f), d);//get absolute value.
	__m128 c = _mm_cmple_ps(d, e);
	return _mm_blend_ps(_mm_blendv_ps(m_Data, Value.m_Data, c), m_Data, 0x8);
}

LWSVector4<float> LWSVector4<float>::Blend_Equal2(const LWSVector4<float> &Rhs, const LWSVector4<float> &Value) const {
	__m128 e = _mm_set_ps1(std::numeric_limits<float>::epsilon());
	__m128 d = _mm_sub_ps(m_Data, Rhs.m_Data);
	d = _mm_andnot_ps(_mm_set_ps1(-0.0f), d);//get absolute value.
	__m128 c = _mm_cmple_ps(d, e);
	return _mm_blend_ps(_mm_blendv_ps(m_Data, Value.m_Data, c), m_Data, 0xC);
}

LWSVector4<float> LWSVector4<float>::Blend_NotEqual(const LWSVector4<float> &Rhs, const LWSVector4<float> &Value) const {
	__m128 e = _mm_set_ps1(std::numeric_limits<float>::epsilon());
	__m128 d = _mm_sub_ps(m_Data, Rhs.m_Data);
	d = _mm_andnot_ps(_mm_set_ps1(-0.0f), d);//get absolute value.
	__m128 c = _mm_cmpgt_ps(d, e);
	return _mm_blendv_ps(m_Data, Value.m_Data, c);
}

LWSVector4<float> LWSVector4<float>::Blend_NotEqual3(const LWSVector4<float> &Rhs, const LWSVector4<float> &Value) const {
	__m128 e = _mm_set_ps1(std::numeric_limits<float>::epsilon());
	__m128 d = _mm_sub_ps(m_Data, Rhs.m_Data);
	d = _mm_andnot_ps(_mm_set_ps1(-0.0f), d);//get absolute value.
	__m128 c = _mm_cmpgt_ps(d, e);
	return _mm_blend_ps(_mm_blendv_ps(m_Data, Value.m_Data, c), m_Data, 0x8);
}

LWSVector4<float> LWSVector4<float>::Blend_NotEqual2(const LWSVector4<float> &Rhs, const LWSVector4<float> &Value) const{
	__m128 e = _mm_set_ps1(std::numeric_limits<float>::epsilon());
	__m128 d = _mm_sub_ps(m_Data, Rhs.m_Data);
	d = _mm_andnot_ps(_mm_set_ps1(-0.0f), d);//get absolute value.
	__m128 c = _mm_cmpgt_ps(d, e);
	return _mm_blend_ps(_mm_blendv_ps(m_Data, Value.m_Data, c), m_Data, 0xC);
}

bool LWSVector4<float>::Less3(const LWSVector4<float> &Rhs) const {
	__m128i one = _mm_set1_epi32(-1);
	__m128i t = _mm_castps_si128(_mm_cmplt_ps(m_Data, Rhs.m_Data));
	return _mm_test_all_ones(_mm_blend_epi32(t, one, 0x8));
}

bool LWSVector4<float>::Less2(const LWSVector4<float> &Rhs) const {
	__m128i one = _mm_set1_epi32(-1);
	__m128i t = _mm_castps_si128(_mm_cmplt_ps(m_Data, Rhs.m_Data));
	return _mm_test_all_ones(_mm_blend_epi32(t, one, 0xC));
}

bool LWSVector4<float>::LessEqual3(const LWSVector4<float> &Rhs) const {
	__m128i one = _mm_set1_epi32(-1);
	__m128i t = _mm_castps_si128(_mm_cmple_ps(m_Data, Rhs.m_Data));
	return _mm_test_all_ones(_mm_blend_epi32(t, one, 0x8));
}

bool LWSVector4<float>::LessEqual2(const LWSVector4<float> &Rhs) const {
	__m128i one = _mm_set1_epi32(-1);
	__m128i t = _mm_castps_si128(_mm_cmple_ps(m_Data, Rhs.m_Data));
	return _mm_test_all_ones(_mm_blend_epi32(t, one, 0xC));
}

bool LWSVector4<float>::Greater3(const LWSVector4<float> &Rhs) const {
	__m128i one = _mm_set1_epi32(-1);
	__m128i t = _mm_castps_si128(_mm_cmpgt_ps(m_Data, Rhs.m_Data));
	return _mm_test_all_ones(_mm_blend_epi32(t, one, 0x8));
}

bool LWSVector4<float>::Greater2(const LWSVector4<float> &Rhs) const {
	__m128i one = _mm_set1_epi32(-1);
	__m128i t = _mm_castps_si128(_mm_cmpgt_ps(m_Data, Rhs.m_Data));
	return _mm_test_all_ones(_mm_blend_epi32(t, one, 0xC));
}

bool LWSVector4<float>::GreaterEqual3(const LWSVector4<float> &Rhs) const {
	__m128i one = _mm_set1_epi32(-1);
	__m128i t = _mm_castps_si128(_mm_cmpge_ps(m_Data, Rhs.m_Data));
	return _mm_test_all_ones(_mm_blend_epi32(t, one, 0x8));
}

bool LWSVector4<float>::GreaterEqual2(const LWSVector4<float> &Rhs) const {
	__m128i one = _mm_set1_epi32(-1);
	__m128i t = _mm_castps_si128(_mm_cmpge_ps(m_Data, Rhs.m_Data));
	return _mm_test_all_ones(_mm_blend_epi32(t, one, 0xC));
}

float LWSVector4<float>::operator[](uint32_t i) const {
	return (&x)[i];
}

float &LWSVector4<float>::operator[](uint32_t i) {
	return (&x)[i];
}

LWSVector4<float>& LWSVector4<float>::operator = (const LWSVector4<float>& Rhs) {
	m_Data = Rhs.m_Data;
	return *this;
}

LWSVector4<float>& LWSVector4<float>::operator += (const LWSVector4<float>& Rhs) {
	m_Data = _mm_add_ps(m_Data, Rhs.m_Data);
	return *this;
}

LWSVector4<float>& LWSVector4<float>::operator += (float Rhs) {
	__m128 t = _mm_set_ps1(Rhs);
	m_Data = _mm_add_ps(m_Data, t);
	return *this;
}

LWSVector4<float>& LWSVector4<float>::operator -= (const LWSVector4<float>& Rhs) {
	m_Data = _mm_sub_ps(m_Data, Rhs.m_Data);
	return *this;
}

LWSVector4<float>& LWSVector4<float>::operator -= (float Rhs) {
	__m128 t = _mm_set_ps1(Rhs);
	m_Data = _mm_sub_ps(m_Data, t);
	return *this;
}

LWSVector4<float>& LWSVector4<float>::operator *= (const LWSVector4<float>& Rhs) {
	m_Data = _mm_mul_ps(m_Data, Rhs.m_Data);
	return *this;
}

LWSVector4<float>& LWSVector4<float>::operator *= (float Rhs) {
	__m128 t = _mm_set_ps1(Rhs);
	m_Data = _mm_mul_ps(m_Data, t);
	return *this;
}

LWSVector4<float>& LWSVector4<float>::operator /= (const LWSVector4<float>& Rhs) {
	m_Data = _mm_div_ps(m_Data, Rhs.m_Data);
	return *this;
}

LWSVector4<float>& LWSVector4<float>::operator /= (float Rhs) {
	__m128 t = _mm_set_ps1(Rhs);
	m_Data = _mm_div_ps(m_Data, t);
	return *this;
}

LWSVector4<float> operator + (const LWSVector4<float>& Rhs) {
	return Rhs.m_Data;
}

LWSVector4<float> operator - (const LWSVector4<float>& Rhs) {
	__m128 t = _mm_set_ps1(-0.0f);
	return _mm_xor_ps(Rhs.m_Data, t);
}

bool LWSVector4<float>::operator > (const LWSVector4<float> &Rhs) const {
	return _mm_test_all_ones(_mm_castps_si128(_mm_cmpgt_ps(m_Data, Rhs.m_Data)));
}

bool LWSVector4<float>::operator >= (const LWSVector4<float> &Rhs) const {
	return _mm_test_all_ones(_mm_castps_si128(_mm_cmpge_ps(m_Data, Rhs.m_Data)));
}

bool LWSVector4<float>::operator < (const LWSVector4<float> &Rhs) const {
	return _mm_test_all_ones(_mm_castps_si128(_mm_cmplt_ps(m_Data, Rhs.m_Data)));
}

bool LWSVector4<float>::operator <= (const LWSVector4<float> &Rhs) const {
	return _mm_test_all_ones(_mm_castps_si128(_mm_cmple_ps(m_Data, Rhs.m_Data)));
}

bool LWSVector4<float>::operator == (const LWSVector4<float>& Rhs) const {
	__m128 e = _mm_set_ps1(std::numeric_limits<float>::epsilon());
	__m128 t = _mm_sub_ps(m_Data, Rhs.m_Data);
	t = _mm_andnot_ps(_mm_set_ps1(-0.0), t); //Get absolute value of difference.
	t = _mm_cmple_ps(t, e);
	return _mm_test_all_ones(_mm_castps_si128(t));
}

bool LWSVector4<float>::operator != (const LWSVector4<float>& Rhs) const {
	return !(*this == Rhs);
}

std::ostream& operator<<(std::ostream& o, const LWSVector4<float>& v) {
	alignas(16) float Values[4];
	_mm_store_ps(Values, v.m_Data);
	o << Values[0] << " " << Values[1] << " " << Values[2] << " " << Values[3];
	return o;
}

LWSVector4<float> operator + (const LWSVector4<float>& Lhs, const LWSVector4<float>& Rhs) {
	return _mm_add_ps(Lhs.m_Data, Rhs.m_Data);
}

LWSVector4<float> operator + (const LWSVector4<float>& Lhs, float Rhs) {
	__m128 t = _mm_set_ps1(Rhs);
	return _mm_add_ps(Lhs.m_Data, t);
}

LWSVector4<float> operator + (float Lhs, const LWSVector4<float>& Rhs) {
	__m128 t = _mm_set_ps1(Lhs);
	return _mm_add_ps(t, Rhs.m_Data);
}

LWSVector4<float> operator - (const LWSVector4<float>& Lhs, const LWSVector4<float>& Rhs) {
	return _mm_sub_ps(Lhs.m_Data, Rhs.m_Data);
}

LWSVector4<float> operator - (const LWSVector4<float>& Lhs, float Rhs) {
	__m128 t = _mm_set_ps1(Rhs);
	return _mm_sub_ps(Lhs.m_Data, t);
}

LWSVector4<float> operator - (float Lhs, const LWSVector4<float>& Rhs) {
	__m128 t = _mm_set_ps1(Lhs);
	return _mm_sub_ps(t, Rhs.m_Data);
}

LWSVector4<float> operator * (const LWSVector4<float>& Lhs, const LWSVector4<float>& Rhs) {
	return _mm_mul_ps(Lhs.m_Data, Rhs.m_Data);
}

LWSVector4<float> operator * (const LWSVector4<float>& Lhs, float Rhs) {
	__m128 t = _mm_set_ps1(Rhs);
	return _mm_mul_ps(Lhs.m_Data, t);
}

LWSVector4<float> operator * (float Lhs, const LWSVector4<float>& Rhs) {
	__m128 t = _mm_set_ps1(Lhs);
	return _mm_mul_ps(t, Rhs.m_Data);
}

LWSVector4<float> operator / (const LWSVector4<float>& Lhs, const LWSVector4<float>& Rhs) {
	return _mm_div_ps(Lhs.m_Data, Rhs.m_Data);
}

LWSVector4<float> operator / (const LWSVector4<float>& Lhs, float Rhs) {
	__m128 t = _mm_set_ps1(Rhs);
	return _mm_div_ps(Lhs.m_Data, t);
}

LWSVector4<float> operator / (float Lhs, const LWSVector4<float>& Rhs) {
	__m128 t = _mm_set_ps1(Lhs);
	return _mm_div_ps(t, Rhs.m_Data);
}

LWSVector4<float> LWSVector4<float>::AAAB(const LWSVector4<float>& B) const {
	return _mm_blend_ps(m_Data, B.m_Data, 0x8);
}

LWSVector4<float> LWSVector4<float>::AABA(const LWSVector4<float>& B) const {
	return _mm_blend_ps(m_Data, B.m_Data, 0x4);
}

LWSVector4<float> LWSVector4<float>::AABB(const LWSVector4<float>& B) const {
	return _mm_blend_ps(m_Data, B.m_Data, 0xC);
}

LWSVector4<float> LWSVector4<float>::ABAA(const LWSVector4<float>& B) const {
	return _mm_blend_ps(m_Data, B.m_Data, 0x2);
}

LWSVector4<float> LWSVector4<float>::ABAB(const LWSVector4<float>& B) const {
	return _mm_blend_ps(m_Data, B.m_Data, 0xA);
}

LWSVector4<float> LWSVector4<float>::ABBA(const LWSVector4<float>& B) const {
	return _mm_blend_ps(m_Data, B.m_Data, 0x6);
}

LWSVector4<float> LWSVector4<float>::ABBB(const LWSVector4<float>& B) const {
	return _mm_blend_ps(m_Data, B.m_Data, 0xE);
}

LWSVector4<float> LWSVector4<float>::BAAA(const LWSVector4<float>& B) const {
	return _mm_blend_ps(m_Data, B.m_Data, 0x1);
}

LWSVector4<float> LWSVector4<float>::BAAB(const LWSVector4<float>& B) const {
	return _mm_blend_ps(m_Data, B.m_Data, 0x9);
}

LWSVector4<float> LWSVector4<float>::BABA(const LWSVector4<float>& B) const {
	return _mm_blend_ps(m_Data, B.m_Data, 0x5);
}

LWSVector4<float> LWSVector4<float>::BABB(const LWSVector4<float>& B) const {
	return _mm_blend_ps(m_Data, B.m_Data, 0xD);
}

LWSVector4<float> LWSVector4<float>::BBAA(const LWSVector4<float>& B) const {
	return _mm_blend_ps(m_Data, B.m_Data, 0x3);
}

LWSVector4<float> LWSVector4<float>::BBAB(const LWSVector4<float>& B) const {
	return _mm_blend_ps(m_Data, B.m_Data, 0xB);
}

LWSVector4<float> LWSVector4<float>::BBBA(const LWSVector4<float>& B) const {
	return _mm_blend_ps(m_Data, B.m_Data, 0x7);
}

LWSVector4<float> LWSVector4<float>::xxxx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 0, 0, 0));
}

LWSVector4<float> LWSVector4<float>::xxxy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 0, 0, 0));
}

LWSVector4<float> LWSVector4<float>::xxxz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 0, 0, 0));
}

LWSVector4<float> LWSVector4<float>::xxxw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 0, 0));
}

LWSVector4<float> LWSVector4<float>::xxyx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 1, 0, 0));
}

LWSVector4<float> LWSVector4<float>::xxyy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 1, 0, 0));
}

LWSVector4<float> LWSVector4<float>::xxyz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 1, 0, 0));
}
LWSVector4<float> LWSVector4<float>::xxyw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 0, 0));
}

LWSVector4<float> LWSVector4<float>::xxzx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 2, 0, 0));
}

LWSVector4<float> LWSVector4<float>::xxzy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 2, 0, 0));
}

LWSVector4<float> LWSVector4<float>::xxzz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 2, 0, 0));
}

LWSVector4<float> LWSVector4<float>::xxzw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 0, 0));
}

LWSVector4<float> LWSVector4<float>::xxwx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 3, 0, 0));
}

LWSVector4<float> LWSVector4<float>::xxwy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 3, 0, 0));
}

LWSVector4<float> LWSVector4<float>::xxwz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 3, 0, 0));
}

LWSVector4<float> LWSVector4<float>::xxww(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 3, 0, 0));
}

LWSVector4<float> LWSVector4<float>::xyxx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 0, 1, 0));
}

LWSVector4<float> LWSVector4<float>::xyxy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 0, 1, 0));
}

LWSVector4<float> LWSVector4<float>::xyxz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 0, 1, 0));
}

LWSVector4<float> LWSVector4<float>::xyxw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 1, 0));
}

LWSVector4<float> LWSVector4<float>::xyyx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 1, 1, 0));
}

LWSVector4<float> LWSVector4<float>::xyyy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 1, 1, 0));
}

LWSVector4<float> LWSVector4<float>::xyyz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 1, 1, 0));
}

LWSVector4<float> LWSVector4<float>::xyyw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 1, 0));
}

LWSVector4<float> LWSVector4<float>::xyzx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 2, 1, 0));
}

LWSVector4<float> LWSVector4<float>::xyzy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 2, 1, 0));
}

LWSVector4<float> LWSVector4<float>::xyzz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 2, 1, 0));
}

LWSVector4<float> LWSVector4<float>::xyz0(void) const {
	LWSVector4<float> v = m_Data;
	v.w = 0.0f;
	return v;
}

LWSVector4<float> LWSVector4<float>::xyz1(void) const {
	LWSVector4<float> v = m_Data;
	v.w = 1.0f;
	return v;
}


LWSVector4<float> LWSVector4<float>::xywx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 3, 1, 0));
}

LWSVector4<float> LWSVector4<float>::xywy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 3, 1, 0));
}

LWSVector4<float> LWSVector4<float>::xywz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 3, 1, 0));
}

LWSVector4<float> LWSVector4<float>::xyww(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 3, 1, 0));
}

LWSVector4<float> LWSVector4<float>::xzxx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 0, 2, 0));
}

LWSVector4<float> LWSVector4<float>::xzxy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 0, 2, 0));
}

LWSVector4<float> LWSVector4<float>::xzxz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 0, 2, 0));
}

LWSVector4<float> LWSVector4<float>::xzxw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 2, 0));
}

LWSVector4<float> LWSVector4<float>::xzyx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 1, 2, 0));
}

LWSVector4<float> LWSVector4<float>::xzyy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 1, 2, 0));
}

LWSVector4<float> LWSVector4<float>::xzyz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 1, 2, 0));
}

LWSVector4<float> LWSVector4<float>::xzyw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 2, 0));
}

LWSVector4<float> LWSVector4<float>::xzzx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 2, 2, 0));
}

LWSVector4<float> LWSVector4<float>::xzzy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 2, 2, 0));
}

LWSVector4<float> LWSVector4<float>::xzzz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 2, 2, 0));
}

LWSVector4<float> LWSVector4<float>::xzzw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 2, 0));
}

LWSVector4<float> LWSVector4<float>::xzwx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 3, 2, 0));
}

LWSVector4<float> LWSVector4<float>::xzwy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 3, 2, 0));
}

LWSVector4<float> LWSVector4<float>::xzwz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 3, 2, 0));
}

LWSVector4<float> LWSVector4<float>::xzww(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 3, 2, 0));
}

LWSVector4<float> LWSVector4<float>::xwxx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 0, 3, 0));
}

LWSVector4<float> LWSVector4<float>::xwxy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 0, 3, 0));
}

LWSVector4<float> LWSVector4<float>::xwxz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 0, 3, 0));
}

LWSVector4<float> LWSVector4<float>::xwxw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 3, 0));
}

LWSVector4<float> LWSVector4<float>::xwyx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 1, 3, 0));
}

LWSVector4<float> LWSVector4<float>::xwyy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 1, 3, 0));
}

LWSVector4<float> LWSVector4<float>::xwyz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 1, 3, 0));
}

LWSVector4<float> LWSVector4<float>::xwyw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 3, 0));
}

LWSVector4<float> LWSVector4<float>::xwzx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 2, 3, 0));
}

LWSVector4<float> LWSVector4<float>::xwzy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 2, 3, 0));
}

LWSVector4<float> LWSVector4<float>::xwzz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 2, 3, 0));
}

LWSVector4<float> LWSVector4<float>::xwzw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 3, 0));
}

LWSVector4<float> LWSVector4<float>::xwwx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 3, 3, 0));
}

LWSVector4<float> LWSVector4<float>::xwwy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 3, 3, 0));
}

LWSVector4<float> LWSVector4<float>::xwwz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 3, 3, 0));
}

LWSVector4<float> LWSVector4<float>::xwww(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 3, 3, 0));
}

LWSVector4<float> LWSVector4<float>::yxxx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 0, 0, 1));
}

LWSVector4<float> LWSVector4<float>::yxxy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 0, 0, 1));
}

LWSVector4<float> LWSVector4<float>::yxxz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 0, 0, 1));
}

LWSVector4<float> LWSVector4<float>::yxxw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 0, 1));
}

LWSVector4<float> LWSVector4<float>::yxyx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 1, 0, 1));
}

LWSVector4<float> LWSVector4<float>::yxyy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 1, 0, 1));
}

LWSVector4<float> LWSVector4<float>::yxyz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 1, 0, 1));
}

LWSVector4<float> LWSVector4<float>::yxyw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 0, 1));
}

LWSVector4<float> LWSVector4<float>::yxzx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 2, 0, 1));
}

LWSVector4<float> LWSVector4<float>::yxzy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 2, 0, 1));
}

LWSVector4<float> LWSVector4<float>::yxzz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 2, 0, 1));
}

LWSVector4<float> LWSVector4<float>::yxzw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 0, 1));
}

LWSVector4<float> LWSVector4<float>::yxwx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 3, 0, 1));
}

LWSVector4<float> LWSVector4<float>::yxwy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 3, 0, 1));
}

LWSVector4<float> LWSVector4<float>::yxwz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 3, 0, 1));
}

LWSVector4<float> LWSVector4<float>::yxww(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 3, 0, 1));
}

LWSVector4<float> LWSVector4<float>::yyxx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 0, 1, 1));
}

LWSVector4<float> LWSVector4<float>::yyxy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 0, 1, 1));
}

LWSVector4<float> LWSVector4<float>::yyxz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 0, 1, 1));
}

LWSVector4<float> LWSVector4<float>::yyxw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 1, 1));
}

LWSVector4<float> LWSVector4<float>::yyyx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 1, 1, 1));
}

LWSVector4<float> LWSVector4<float>::yyyy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 1, 1, 1));
}

LWSVector4<float> LWSVector4<float>::yyyz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 1, 1, 1));
}

LWSVector4<float> LWSVector4<float>::yyyw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 1, 1));
}

LWSVector4<float> LWSVector4<float>::yyzx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 2, 1, 1));
}

LWSVector4<float> LWSVector4<float>::yyzy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 2, 1, 1));
}

LWSVector4<float> LWSVector4<float>::yyzz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 2, 1, 1));
}

LWSVector4<float> LWSVector4<float>::yyzw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 1, 1));
}

LWSVector4<float> LWSVector4<float>::yywx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 3, 1, 1));
}

LWSVector4<float> LWSVector4<float>::yywy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 3, 1, 1));
}

LWSVector4<float> LWSVector4<float>::yywz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 3, 1, 1));
}

LWSVector4<float> LWSVector4<float>::yyww(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 3, 1, 1));
}

LWSVector4<float> LWSVector4<float>::yzxx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 0, 2, 1));
}

LWSVector4<float> LWSVector4<float>::yzxy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 0, 2, 1));
}

LWSVector4<float> LWSVector4<float>::yzxz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 0, 2, 1));
}

LWSVector4<float> LWSVector4<float>::yzxw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 2, 1));
}

LWSVector4<float> LWSVector4<float>::yzyx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 1, 2, 1));
}

LWSVector4<float> LWSVector4<float>::yzyy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 1, 2, 1));
}

LWSVector4<float> LWSVector4<float>::yzyz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 1, 2, 1));
}

LWSVector4<float> LWSVector4<float>::yzyw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 2, 1));
}

LWSVector4<float> LWSVector4<float>::yzzx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 2, 2, 1));
}

LWSVector4<float> LWSVector4<float>::yzzy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 2, 2, 1));
}

LWSVector4<float> LWSVector4<float>::yzzz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 2, 2, 1));
}

LWSVector4<float> LWSVector4<float>::yzzw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 2, 1));
}

LWSVector4<float> LWSVector4<float>::yzwx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 3, 2, 1));
}

LWSVector4<float> LWSVector4<float>::yzwy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 3, 2, 1));
}

LWSVector4<float> LWSVector4<float>::yzwz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 3, 2, 1));
}

LWSVector4<float> LWSVector4<float>::yzww(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 3, 2, 1));
}

LWSVector4<float> LWSVector4<float>::ywxx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 0, 3, 1));
}

LWSVector4<float> LWSVector4<float>::ywxy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 0, 3, 1));
}

LWSVector4<float> LWSVector4<float>::ywxz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 0, 3, 1));
}

LWSVector4<float> LWSVector4<float>::ywxw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 3, 1));
}

LWSVector4<float> LWSVector4<float>::ywyx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 1, 3, 1));
}

LWSVector4<float> LWSVector4<float>::ywyy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 1, 3, 1));
}

LWSVector4<float> LWSVector4<float>::ywyz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 1, 3, 1));
}

LWSVector4<float> LWSVector4<float>::ywyw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 3, 1));
}

LWSVector4<float> LWSVector4<float>::ywzx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 2, 3, 1));
}

LWSVector4<float> LWSVector4<float>::ywzy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 2, 3, 1));
}

LWSVector4<float> LWSVector4<float>::ywzz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 2, 3, 1));
}

LWSVector4<float> LWSVector4<float>::ywzw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 3, 1));
}

LWSVector4<float> LWSVector4<float>::ywwx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 3, 3, 1));
}

LWSVector4<float> LWSVector4<float>::ywwy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 3, 3, 1));
}

LWSVector4<float> LWSVector4<float>::ywwz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 3, 3, 1));
}

LWSVector4<float> LWSVector4<float>::ywww(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 3, 3, 1));
}

LWSVector4<float> LWSVector4<float>::zxxx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 0, 0, 2));
}

LWSVector4<float> LWSVector4<float>::zxxy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 0, 0, 2));
}

LWSVector4<float> LWSVector4<float>::zxxz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 0, 0, 2));
}

LWSVector4<float> LWSVector4<float>::zxxw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 0, 2));
}

LWSVector4<float> LWSVector4<float>::zxyx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 1, 0, 2));
}

LWSVector4<float> LWSVector4<float>::zxyy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 1, 0, 2));
}

LWSVector4<float> LWSVector4<float>::zxyz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 1, 0, 2));
}

LWSVector4<float> LWSVector4<float>::zxyw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 0, 2));
}

LWSVector4<float> LWSVector4<float>::zxzx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 2, 0, 2));
}

LWSVector4<float> LWSVector4<float>::zxzy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 2, 0, 2));
}

LWSVector4<float> LWSVector4<float>::zxzz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 2, 0, 2));
}

LWSVector4<float> LWSVector4<float>::zxzw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 0, 2));
}

LWSVector4<float> LWSVector4<float>::zxwx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 3, 0, 2));
}

LWSVector4<float> LWSVector4<float>::zxwy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 3, 0, 2));
}

LWSVector4<float> LWSVector4<float>::zxwz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 3, 0, 2));
}

LWSVector4<float> LWSVector4<float>::zxww(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 3, 0, 2));
}

LWSVector4<float> LWSVector4<float>::zyxx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 0, 1, 2));
}

LWSVector4<float> LWSVector4<float>::zyxy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 0, 1, 2));
}

LWSVector4<float> LWSVector4<float>::zyxz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 0, 1, 2));
}

LWSVector4<float> LWSVector4<float>::zyxw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 1, 2));
}

LWSVector4<float> LWSVector4<float>::zyyx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 1, 1, 2));
}

LWSVector4<float> LWSVector4<float>::zyyy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 1, 1, 2));
}

LWSVector4<float> LWSVector4<float>::zyyz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 1, 1, 2));
}

LWSVector4<float> LWSVector4<float>::zyyw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 1, 2));
}

LWSVector4<float> LWSVector4<float>::zyzx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 2, 1, 2));
}

LWSVector4<float> LWSVector4<float>::zyzy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 2, 1, 2));
}

LWSVector4<float> LWSVector4<float>::zyzz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 2, 1, 2));
}

LWSVector4<float> LWSVector4<float>::zyzw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 1, 2));
}

LWSVector4<float> LWSVector4<float>::zywx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 3, 1, 2));
}

LWSVector4<float> LWSVector4<float>::zywy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 3, 1, 2));
}

LWSVector4<float> LWSVector4<float>::zywz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 3, 1, 2));
}

LWSVector4<float> LWSVector4<float>::zyww(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 3, 1, 2));
}

LWSVector4<float> LWSVector4<float>::zzxx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 0, 2, 2));
}

LWSVector4<float> LWSVector4<float>::zzxy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 0, 2, 2));
}

LWSVector4<float> LWSVector4<float>::zzxz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 0, 2, 2));
}

LWSVector4<float> LWSVector4<float>::zzxw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 2, 2));
}

LWSVector4<float> LWSVector4<float>::zzyx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 1, 2, 2));
}

LWSVector4<float> LWSVector4<float>::zzyy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 1, 2, 2));
}

LWSVector4<float> LWSVector4<float>::zzyz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 1, 2, 2));
}

LWSVector4<float> LWSVector4<float>::zzyw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 2, 2));
}

LWSVector4<float> LWSVector4<float>::zzzx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 2, 2, 2));
}

LWSVector4<float> LWSVector4<float>::zzzy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 2, 2, 2));
}

LWSVector4<float> LWSVector4<float>::zzzz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 2, 2, 2));
}

LWSVector4<float> LWSVector4<float>::zzzw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 2, 2));
}

LWSVector4<float> LWSVector4<float>::zzwx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 3, 2, 2));
}

LWSVector4<float> LWSVector4<float>::zzwy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 3, 2, 2));
}

LWSVector4<float> LWSVector4<float>::zzwz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 3, 2, 2));
}

LWSVector4<float> LWSVector4<float>::zzww(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 3, 2, 2));
}

LWSVector4<float> LWSVector4<float>::zwxx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 0, 3, 2));
}

LWSVector4<float> LWSVector4<float>::zwxy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 0, 3, 2));
}

LWSVector4<float> LWSVector4<float>::zwxz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 0, 3, 2));
}

LWSVector4<float> LWSVector4<float>::zwxw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 3, 2));
}

LWSVector4<float> LWSVector4<float>::zwyx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 1, 3, 2));
}

LWSVector4<float> LWSVector4<float>::zwyy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 1, 3, 2));
}

LWSVector4<float> LWSVector4<float>::zwyz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 1, 3, 2));
}

LWSVector4<float> LWSVector4<float>::zwyw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 3, 2));
}

LWSVector4<float> LWSVector4<float>::zwzx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 2, 3, 2));
}

LWSVector4<float> LWSVector4<float>::zwzy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 2, 3, 2));
}

LWSVector4<float> LWSVector4<float>::zwzz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 2, 3, 2));
}

LWSVector4<float> LWSVector4<float>::zwzw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 3, 2));
}

LWSVector4<float> LWSVector4<float>::zwwx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 3, 3, 2));
}

LWSVector4<float> LWSVector4<float>::zwwy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 3, 3, 2));
}

LWSVector4<float> LWSVector4<float>::zwwz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 3, 3, 2));
}

LWSVector4<float> LWSVector4<float>::zwww(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 3, 3, 2));
}

LWSVector4<float> LWSVector4<float>::wxxx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 0, 0, 3));
}

LWSVector4<float> LWSVector4<float>::wxxy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 0, 0, 3));
}

LWSVector4<float> LWSVector4<float>::wxxz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 0, 0, 3));
}

LWSVector4<float> LWSVector4<float>::wxxw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 0, 3));
}

LWSVector4<float> LWSVector4<float>::wxyx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 1, 0, 3));
}

LWSVector4<float> LWSVector4<float>::wxyy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 1, 0, 3));
}

LWSVector4<float> LWSVector4<float>::wxyz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 1, 0, 3));
}

LWSVector4<float> LWSVector4<float>::wxyw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 0, 3));
}

LWSVector4<float> LWSVector4<float>::wxzx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 2, 0, 3));
}

LWSVector4<float> LWSVector4<float>::wxzy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 2, 0, 3));
}

LWSVector4<float> LWSVector4<float>::wxzz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 2, 0, 3));
}

LWSVector4<float> LWSVector4<float>::wxzw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 0, 3));
}

LWSVector4<float> LWSVector4<float>::wxwx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 3, 0, 3));
}

LWSVector4<float> LWSVector4<float>::wxwy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 3, 0, 3));
}

LWSVector4<float> LWSVector4<float>::wxwz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 3, 0, 3));
}

LWSVector4<float> LWSVector4<float>::wxww(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 3, 0, 3));
}

LWSVector4<float> LWSVector4<float>::wyxx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 0, 1, 3));
}

LWSVector4<float> LWSVector4<float>::wyxy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 0, 1, 3));
}

LWSVector4<float> LWSVector4<float>::wyxz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 0, 1, 3));
}

LWSVector4<float> LWSVector4<float>::wyxw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 1, 3));
}

LWSVector4<float> LWSVector4<float>::wyyx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 1, 1, 3));
}

LWSVector4<float> LWSVector4<float>::wyyy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 1, 1, 3));
}

LWSVector4<float> LWSVector4<float>::wyyz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 1, 1, 3));
}

LWSVector4<float> LWSVector4<float>::wyyw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 1, 3));
}

LWSVector4<float> LWSVector4<float>::wyzx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 2, 1, 3));
}

LWSVector4<float> LWSVector4<float>::wyzy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 2, 1, 3));
}

LWSVector4<float> LWSVector4<float>::wyzz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 2, 1, 3));
}

LWSVector4<float> LWSVector4<float>::wyzw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 1, 3));
}

LWSVector4<float> LWSVector4<float>::wywx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 3, 1, 3));
}

LWSVector4<float> LWSVector4<float>::wywy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 3, 1, 3));
}

LWSVector4<float> LWSVector4<float>::wywz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 3, 1, 3));
}

LWSVector4<float> LWSVector4<float>::wyww(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 3, 1, 3));
}

LWSVector4<float> LWSVector4<float>::wzxx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 0, 2, 3));
}

LWSVector4<float> LWSVector4<float>::wzxy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 0, 2, 3));
}

LWSVector4<float> LWSVector4<float>::wzxz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 0, 2, 3));
}

LWSVector4<float> LWSVector4<float>::wzxw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 2, 3));
}

LWSVector4<float> LWSVector4<float>::wzyx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 1, 2, 3));
}

LWSVector4<float> LWSVector4<float>::wzyy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 1, 2, 3));
}

LWSVector4<float> LWSVector4<float>::wzyz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 1, 2, 3));
}

LWSVector4<float> LWSVector4<float>::wzyw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 2, 3));
}

LWSVector4<float> LWSVector4<float>::wzzx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 2, 2, 3));
}

LWSVector4<float> LWSVector4<float>::wzzy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 2, 2, 3));
}

LWSVector4<float> LWSVector4<float>::wzzz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 2, 2, 3));
}

LWSVector4<float> LWSVector4<float>::wzzw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 2, 3));
}

LWSVector4<float> LWSVector4<float>::wzwx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 3, 2, 3));
}

LWSVector4<float> LWSVector4<float>::wzwy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 3, 2, 3));
}

LWSVector4<float> LWSVector4<float>::wzwz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 3, 2, 3));
}

LWSVector4<float> LWSVector4<float>::wzww(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 3, 2, 3));
}

LWSVector4<float> LWSVector4<float>::wwxx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 0, 3, 3));
}

LWSVector4<float> LWSVector4<float>::wwxy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 0, 3, 3));
}

LWSVector4<float> LWSVector4<float>::wwxz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 0, 3, 3));
}

LWSVector4<float> LWSVector4<float>::wwxw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 3, 3));
}

LWSVector4<float> LWSVector4<float>::wwyx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 1, 3, 3));
}

LWSVector4<float> LWSVector4<float>::wwyy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 1, 3, 3));
}

LWSVector4<float> LWSVector4<float>::wwyz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 1, 3, 3));
}

LWSVector4<float> LWSVector4<float>::wwyw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 3, 3));
}

LWSVector4<float> LWSVector4<float>::wwzx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 2, 3, 3));
}

LWSVector4<float> LWSVector4<float>::wwzy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 2, 3, 3));
}

LWSVector4<float> LWSVector4<float>::wwzz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 2, 3, 3));
}

LWSVector4<float> LWSVector4<float>::wwzw(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 3, 3));
}

LWSVector4<float> LWSVector4<float>::wwwx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 3, 3, 3));
}

LWSVector4<float> LWSVector4<float>::wwwy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 3, 3, 3));
}

LWSVector4<float> LWSVector4<float>::wwwz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 3, 3, 3));
}

LWSVector4<float> LWSVector4<float>::wwww(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 3, 3, 3));
}

LWSVector4<float> LWSVector4<float>::xxx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 0, 0));
}

LWSVector4<float> LWSVector4<float>::xxy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 0, 0));
}

LWSVector4<float> LWSVector4<float>::xxz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 0, 0));
}

LWSVector4<float> LWSVector4<float>::xyx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 1, 0));
}

LWSVector4<float> LWSVector4<float>::xyy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 1, 0));
}

LWSVector4<float> LWSVector4<float>::xyz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 1, 0));
}

LWSVector4<float> LWSVector4<float>::xzx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 2, 0));
}

LWSVector4<float> LWSVector4<float>::xzy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 2, 0));
}

LWSVector4<float> LWSVector4<float>::xzz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 2, 0));
}

LWSVector4<float> LWSVector4<float>::yxx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 0, 1));
}

LWSVector4<float> LWSVector4<float>::yxy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 0, 1));
}

LWSVector4<float> LWSVector4<float>::yxz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 0, 1));
}

LWSVector4<float> LWSVector4<float>::yyx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 1, 1));
}

LWSVector4<float> LWSVector4<float>::yyy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 1, 1));
}

LWSVector4<float> LWSVector4<float>::yyz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 1, 1));
}

LWSVector4<float> LWSVector4<float>::yzx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 2, 1));
}

LWSVector4<float> LWSVector4<float>::yzy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 2, 1));
}

LWSVector4<float> LWSVector4<float>::yzz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 2, 1));
}

LWSVector4<float> LWSVector4<float>::zxx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 0, 2));
}

LWSVector4<float> LWSVector4<float>::zxy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 0, 2));
}

LWSVector4<float> LWSVector4<float>::zxz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 0, 2));
}

LWSVector4<float> LWSVector4<float>::zyx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 1, 2));
}

LWSVector4<float> LWSVector4<float>::zyy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 1, 2));
}

LWSVector4<float> LWSVector4<float>::zyz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 1, 2));
}

LWSVector4<float> LWSVector4<float>::zzx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 2, 2));
}

LWSVector4<float> LWSVector4<float>::zzy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 2, 2));
}

LWSVector4<float> LWSVector4<float>::zzz(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 2, 2));
}

LWSVector4<float> LWSVector4<float>::xx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 0, 0));
}

LWSVector4<float> LWSVector4<float>::xy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 1, 0));
}

LWSVector4<float> LWSVector4<float>::yx(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 0, 1));
}

LWSVector4<float> LWSVector4<float>::yy(void) const {
	return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 1, 1));
}

LWSVector4<float>::LWSVector4(__m128 Data) : m_Data(Data) {}

LWSVector4<float>::LWSVector4(const LWVector4<float>& vxyzw) : m_Data(_mm_set_ps(vxyzw.w, vxyzw.z, vxyzw.y, vxyzw.x)) {}

LWSVector4<float>::LWSVector4(const LWVector3<float>& vxyz, float vw) : m_Data(_mm_set_ps(vw, vxyz.z, vxyz.y, vxyz.x)) {}

LWSVector4<float>::LWSVector4(const LWVector2<float>& vxy, const LWVector2<float>& vzw) : m_Data(_mm_set_ps(vzw.y, vzw.x, vxy.y, vxy.x)) {}

LWSVector4<float>::LWSVector4(const LWVector2<float>& vxy, float vz, float vw) : m_Data(_mm_set_ps(vw, vz, vxy.y, vxy.x)) {}

LWSVector4<float>::LWSVector4(float vx, float vy, float vz, float vw) : m_Data(_mm_set_ps(vw, vz, vy, vx)) {}

LWSVector4<float>::LWSVector4(float f) : m_Data(_mm_set_ps1(f)) {}

#endif