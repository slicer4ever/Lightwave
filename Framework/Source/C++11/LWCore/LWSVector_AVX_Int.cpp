#include "LWCore/LWSVector.h"
#ifndef LW_NOAVX

LWVector4<int32_t> LWSVector4<int32_t>::AsVec4(void) const {
	alignas(16) LWVector4<int32_t> R;
	_mm_store_si128((__m128i*) & R.x, m_Data);
	return R;
}

int32_t *LWSVector4<int32_t>::AsArray(void) {
	return (int32_t*)&m_Data;
}

const int32_t *LWSVector4<int32_t>::AsArray(void) const {
	return (int32_t*)&m_Data;
}

LWSVector4<int32_t> &LWSVector4<int32_t>::sX(int32_t Value) {
	((int32_t*)&m_Data)[0] = Value;
	return *this;
}

LWSVector4<int32_t> &LWSVector4<int32_t>::sY(int32_t Value) {
	((int32_t*)&m_Data)[1] = Value;
	return *this;
}

LWSVector4<int32_t> &LWSVector4<int32_t>::sZ(int32_t Value) {
	((int32_t*)&m_Data)[2] = Value;
	return *this;
}

LWSVector4<int32_t> &LWSVector4<int32_t>::sW(int32_t Value) {
	((int32_t*)&m_Data)[3] = Value;
	return *this;
}

LWSVector4<int32_t> LWSVector4<int32_t>::Normalize(void) const {
	__m128i e = _mm_set1_epi32(1);
	__m128i t = _mm_mullo_epi32(m_Data, m_Data);
	t = _mm_hadd_epi32(t, t);
	t = _mm_hadd_epi32(t, t);
	if (_mm_test_all_ones(_mm_cmplt_epi32(t, e))) return _mm_set1_epi32(0);
	alignas(16) int32_t vi[4];
	alignas(16) float vf[4];
	_mm_store_si128((__m128i*)vi, t);
	__m128 tf = _mm_set_ps((float)vi[0], (float)vi[1], (float)vi[2], (float)vi[3]);
	tf = _mm_div_ps(tf, _mm_sqrt_ps(tf));
	_mm_store_ps(vf, tf);
	return _mm_set_epi32((int32_t)vf[0], (int32_t)vf[1], (int32_t)vf[2], (int32_t)vf[3]);
}

LWSVector4<int32_t> LWSVector4<int32_t>::Normalize3(void) const {
	__m128i e = _mm_set1_epi32(1);
	__m128i t = _mm_mullo_epi32(m_Data, _mm_set_epi32(0, 1, 1, 1));
	t = _mm_mullo_epi32(t, t);
	t = _mm_hadd_epi32(t, t);
	t = _mm_hadd_epi32(t, t);
	if (_mm_test_all_ones(_mm_cmplt_epi32(t, e))) return _mm_set1_epi32(0);
	alignas(16) int32_t vi[4];
	alignas(16) float vf[4];
	_mm_store_si128((__m128i*)vi, t);
	__m128 tf = _mm_set_ps((float)vi[0], (float)vi[1], (float)vi[2], (float)vi[3]);
	tf = _mm_div_ps(tf, _mm_sqrt_ps(tf));
	_mm_store_ps(vf, tf);
	return _mm_blend_epi32(_mm_set_epi32((int32_t)vf[0], (int32_t)vf[1], (int32_t)vf[2], (int32_t)vf[3]), m_Data, 0x8);

}

LWSVector4<int32_t> LWSVector4<int32_t>::Normalize2(void) const {
	__m128i e = _mm_set1_epi32(1);
	__m128i t = _mm_mullo_epi32(m_Data, _mm_set_epi32(0, 0, 1, 1));
	t = _mm_mullo_epi32(t, t);
	t = _mm_hadd_epi32(t, t);
	t = _mm_hadd_epi32(t, t);
	if (_mm_test_all_ones(_mm_cmplt_epi32(t, e))) return _mm_set1_epi32(0);
	alignas(16) int32_t vi[4];
	alignas(16) float vf[4];
	_mm_store_si128((__m128i*)vi, t);
	__m128 tf = _mm_set_ps((float)vi[0], (float)vi[1], (float)vi[2], (float)vi[3]);
	tf = _mm_div_ps(tf, _mm_sqrt_ps(tf));
	_mm_store_ps(vf, tf);
	return _mm_blend_epi32(_mm_set_epi32((int32_t)vf[0], (int32_t)vf[1], (int32_t)vf[2], (int32_t)vf[3]), m_Data, 0xC);

}

int32_t LWSVector4<int32_t>::Min(void) const {
	__m128i A = _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 1, 0, 3));
	__m128i B = _mm_min_epi32(m_Data, A);
	A = _mm_shuffle_epi32(B, _MM_SHUFFLE(1, 0, 3, 2));
	return _mm_cvtsi128_si32(_mm_min_epi32(B, A));
}

int32_t LWSVector4<int32_t>::Min3(void) const {
	__m128i A = _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 0, 2));
	__m128i B = _mm_min_epi32(m_Data, A);
	A = _mm_shuffle_epi32(B, _MM_SHUFFLE(3, 0, 2, 1));
	return _mm_cvtsi128_si32(_mm_min_epi32(B, A));
}

int32_t LWSVector4<int32_t>::Min2(void) const {
	__m128i A = _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 0, 1));
	return _mm_cvtsi128_si32(_mm_min_epi32(m_Data, A));
}

int32_t LWSVector4<int32_t>::Dot(const LWSVector4<int32_t>& O) const {
	__m128i t = _mm_mul_epi32(m_Data, O.m_Data);
	t = _mm_hadd_epi32(t, t);
	t = _mm_hadd_epi32(t, t);
	return _mm_cvtsi128_si32(t);
}

int32_t LWSVector4<int32_t>::Dot3(const LWSVector4<int32_t>& O) const {
	__m128i t = _mm_mul_epi32(m_Data, O.m_Data);
	t = _mm_mul_epi32(t, _mm_set_epi32(1, 1, 1, 0));
	t = _mm_hadd_epi32(t, t);
	t = _mm_hadd_epi32(t, t);
	return _mm_cvtsi128_si32(t);
}

int32_t LWSVector4<int32_t>::Dot2(const LWSVector4<int32_t>& O) const {
	__m128i t = _mm_mul_epi32(m_Data, O.m_Data);
	t = _mm_hadd_epi32(t, t);
	return _mm_cvtsi128_si32(t);
}

LWSVector4<int32_t> LWSVector4<int32_t>::Sum(void) const {
	__m128i t = _mm_hadd_epi32(m_Data, m_Data);
	return _mm_hadd_epi32(t, t);
}

int32_t LWSVector4<int32_t>::Sum4(void) const {
	__m128i t = _mm_hadd_epi32(m_Data, m_Data);
	return _mm_cvtsi128_si32(_mm_hadd_epi32(t, t));
}

int32_t LWSVector4<int32_t>::Sum3(void) const {
	__m128i t = _mm_mullo_epi32(m_Data, _mm_set_epi32(0, 1, 1, 1));
	t = _mm_hadd_epi32(t, t);
	return _mm_cvtsi128_si32(_mm_hadd_epi32(t, t));
}

int32_t LWSVector4<int32_t>::Sum2(void) const {
	return _mm_cvtsi128_si32(_mm_hadd_epi32(m_Data, m_Data));
}

int32_t LWSVector4<int32_t>::Max(void) const {
	__m128i A = _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 1, 0, 3));
	__m128i B = _mm_max_epi32(m_Data, A);
	A = _mm_shuffle_epi32(B, _MM_SHUFFLE(1, 0, 3, 2));
	return _mm_cvtsi128_si32(_mm_max_epi32(B, A));
}

int32_t LWSVector4<int32_t>::Max3(void) const {
	__m128i A = _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 0, 2));
	__m128i B = _mm_max_epi32(m_Data, A);
	A = _mm_shuffle_epi32(B, _MM_SHUFFLE(3, 0, 2, 1));
	return _mm_cvtsi128_si32(_mm_max_epi32(B, A));
}

int32_t LWSVector4<int32_t>::Max2(void) const {
	__m128i A = _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 0, 1));
	return _mm_cvtsi128_si32(_mm_max_epi32(m_Data, A));
}

LWSVector4<int32_t> LWSVector4<int32_t>::Min(const LWSVector4<int32_t>& A) const {
	return _mm_min_epi32(m_Data, A.m_Data);
}

LWSVector4<int32_t> LWSVector4<int32_t>::Max(const LWSVector4<int32_t>& A) const {
	return _mm_max_epi32(m_Data, A.m_Data);
}

LWSVector4<int32_t> LWSVector4<int32_t>::Cross3(const LWSVector4<int32_t>& O) const {
	__m128i A = yzxw().m_Data;
	__m128i B = O.zxyw().m_Data;
	__m128i C = zxyw().m_Data;
	__m128i D = O.yzxw().m_Data;
	return _mm_sub_epi32(_mm_mullo_epi32(A, B), _mm_mullo_epi32(C, D));
}

void LWSVector4<int32_t>::Orthogonal3(LWSVector4<int32_t> &Right, LWSVector4<int32_t> &Up) const {
	const LWSVector4<int32_t> XAxis = LWSVector4<int32_t>(1, 0, 0, 0);
	const LWSVector4<int32_t> YAxis = LWSVector4<int32_t>(0, 1, 0, 0);
	LWSVector4<int32_t> A = XAxis;
	int32_t d = abs(Dot3(A));
	if (d > 1) A = YAxis;
	Right = Cross3(A).Normalize3().AAAB(*this);
	Up = Cross3(Right).AAAB(*this);
	return;
};

LWSVector4<int32_t> LWSVector4<int32_t>::Perpindicular2(void) const {
	return _mm_xor_si128(yx().m_Data, _mm_set_epi32(0, 0, 0, -0));
}

int32_t LWSVector4<int32_t>::Length(void) const {
	const __m128i e = _mm_set1_epi32(1);
	__m128i t = _mm_mullo_epi32(m_Data, m_Data);
	t = _mm_hadd_epi32(t, t);
	t = _mm_hadd_epi32(t, t);
	if (_mm_test_all_ones(_mm_cmplt_epi32(t, e))) return 0;
	alignas(16) int32_t vi[4];
	_mm_store_si128((__m128i*)vi, t);
	__m128 mf = _mm_set_ps((float)vi[0], (float)vi[1], (float)vi[2], (float)vi[3]);
	return (int32_t)_mm_cvtss_f32(_mm_sqrt_ps(mf));
}

int32_t LWSVector4<int32_t>::Length3(void) const {
	const __m128i e = _mm_set1_epi32(1);
	__m128i t = _mm_mullo_epi32(m_Data, _mm_set_epi32(0, 1, 1, 1));
	t = _mm_mullo_epi32(t, t);
	t = _mm_hadd_epi32(t, t);
	t = _mm_hadd_epi32(t, t);
	if (_mm_test_all_ones(_mm_cmplt_epi32(t, e))) return 0;
	alignas(16) int32_t vi[4];
	_mm_store_si128((__m128i*)vi, t);
	__m128 mf = _mm_set_ps((float)vi[0], (float)vi[1], (float)vi[2], (float)vi[3]);
	return (int32_t)_mm_cvtss_f32(_mm_sqrt_ps(mf));

}

int32_t LWSVector4<int32_t>::Length2(void) const {
	const __m128i e = _mm_set1_epi32(1);
	__m128i t = _mm_mullo_epi32(m_Data, _mm_set_epi32(0, 0, 1, 1));
	t = _mm_mullo_epi32(t, t);
	t = _mm_hadd_epi32(t, t);
	t = _mm_hadd_epi32(t, t);
	if (_mm_test_all_ones(_mm_cmplt_epi32(t, e))) return 0;
	alignas(16) int32_t vi[4];
	_mm_store_si128((__m128i*)vi, t);
	__m128 mf = _mm_set_ps((float)vi[0], (float)vi[1], (float)vi[2], (float)vi[3]);
	return (int32_t)_mm_cvtss_f32(_mm_sqrt_ps(mf));
}

int32_t LWSVector4<int32_t>::LengthSquared(void) const {
	__m128i Sq = _mm_mullo_epi32(m_Data, m_Data);
	Sq = _mm_hadd_epi32(Sq, Sq);
	Sq = _mm_hadd_epi32(Sq, Sq);
	return _mm_cvtsi128_si32(Sq);
}

int32_t LWSVector4<int32_t>::LengthSquared3(void) const {
	__m128i t = _mm_mullo_epi32(m_Data, _mm_set_epi32(0, 1, 1, 1));
	__m128i Sq = _mm_mullo_epi32(t, t);
	Sq = _mm_hadd_epi32(Sq, Sq);
	Sq = _mm_hadd_epi32(Sq, Sq);
	return _mm_cvtsi128_si32(Sq);
}

int32_t LWSVector4<int32_t>::LengthSquared2(void) const {
	__m128i t = _mm_mullo_epi32(m_Data, _mm_set_epi32(0, 0, 1, 1));
	__m128i Sq = _mm_mullo_epi32(t, t);
	Sq = _mm_hadd_epi32(Sq, Sq);
	Sq = _mm_hadd_epi32(Sq, Sq);
	return _mm_cvtsi128_si32(Sq);
}

int32_t LWSVector4<int32_t>::Distance(const LWSVector4<int32_t>& O) const {
	const __m128i e = _mm_set1_epi32(1);
	alignas(16) int32_t vi[4];
	__m128i ti = _mm_sub_epi32(m_Data, O.m_Data);
	ti = _mm_mullo_epi32(ti, ti);
	ti = _mm_hadd_epi32(ti, ti);
	ti = _mm_hadd_epi32(ti, ti);
	if(_mm_test_all_ones(_mm_cmplt_epi32(ti, e))) return 0;
	_mm_store_si128((__m128i*)vi, ti);
	__m128 t = _mm_set_ps((float)vi[0], (float)vi[1], (float)vi[2], (float)vi[3]);
	return (int32_t)_mm_cvtss_f32(_mm_sqrt_ps(t));
}

int32_t LWSVector4<int32_t>::Distance3(const LWSVector4<int32_t>& O) const {
	const __m128i e = _mm_set1_epi32(1);
	alignas(16) int32_t vi[4];
	__m128i ti = _mm_sub_epi32(m_Data, O.m_Data);
	ti = _mm_mullo_epi32(ti, _mm_set_epi32(0, 1, 1, 1));
	ti = _mm_mullo_epi32(ti, ti);
	ti = _mm_hadd_epi32(ti, ti);
	ti = _mm_hadd_epi32(ti, ti);
	if (_mm_test_all_ones(_mm_cmplt_epi32(ti, e))) return 0;
	_mm_store_si128((__m128i*)vi, ti);
	__m128 t = _mm_set_ps((float)vi[0], (float)vi[1], (float)vi[2], (float)vi[3]);
	return (int32_t)_mm_cvtss_f32(_mm_sqrt_ps(t));
}

int32_t LWSVector4<int32_t>::Distance2(const LWSVector4<int32_t>& O) const {
	const __m128i e = _mm_set1_epi32(1);
	alignas(16) int32_t vi[4];
	__m128i ti = _mm_sub_epi32(m_Data, O.m_Data);
	ti = _mm_mullo_epi32(ti, _mm_set_epi32(0, 0, 1, 1));
	ti = _mm_mullo_epi32(ti, ti);
	ti = _mm_hadd_epi32(ti, ti);
	ti = _mm_hadd_epi32(ti, ti);
	if (_mm_test_all_ones(_mm_cmplt_epi32(ti, e))) return 0;
	_mm_store_si128((__m128i*)vi, ti);
	__m128 t = _mm_set_ps((float)vi[0], (float)vi[1], (float)vi[2], (float)vi[3]);
	return (int32_t)_mm_cvtss_f32(_mm_sqrt_ps(t));
}

int32_t LWSVector4<int32_t>::DistanceSquared(const LWSVector4<int32_t>& O) const {
	const __m128i e = _mm_set1_epi32(1);
	__m128i t = _mm_sub_epi32(m_Data, O.m_Data);
	t = _mm_mullo_epi32(t, t);
	t = _mm_hadd_epi32(t, t);
	t = _mm_hadd_epi32(t, t);
	return _mm_cvtsi128_si32(t);
}

int32_t LWSVector4<int32_t>::DistanceSquared3(const LWSVector4<int32_t>& O) const {
	const __m128i e = _mm_set1_epi32(1);
	__m128i t = _mm_sub_epi32(m_Data, O.m_Data);
	t = _mm_mullo_epi32(t, _mm_set_epi32(0, 1, 1, 1));
	t = _mm_mullo_epi32(t, t);
	t = _mm_hadd_epi32(t, t);
	t = _mm_hadd_epi32(t, t);
	return _mm_cvtsi128_si32(t);
}

int32_t LWSVector4<int32_t>::DistanceSquared2(const LWSVector4<int32_t>& O) const {
	const __m128i e = _mm_set1_epi32(1);
	__m128i t = _mm_sub_epi32(m_Data, O.m_Data);
	t = _mm_mullo_epi32(t, _mm_set_epi32(0, 0, 1, 1));
	t = _mm_mullo_epi32(t, t);
	t = _mm_hadd_epi32(t, t);
	t = _mm_hadd_epi32(t, t);
	return _mm_cvtsi128_si32(t);
}

LWSVector4<int32_t> LWSVector4<int32_t>::Abs(void) const {
	return _mm_abs_epi32(m_Data);
}

LWSVector4<int32_t> LWSVector4<int32_t>::Abs3(void) const {
	return _mm_blend_epi32(_mm_abs_epi32(m_Data), m_Data, 0x8);
}

LWSVector4<int32_t> LWSVector4<int32_t>::Abs2(void) const {
	return _mm_blend_epi32(_mm_abs_epi32(m_Data), m_Data, 0xC);
}

LWSVector4<int32_t> LWSVector4<int32_t>::Blend_Less(const LWSVector4<int32_t> &Rhs, const LWSVector4<int32_t> &Value) const {
	__m128i c = _mm_cmplt_epi32(m_Data, Rhs.m_Data);
	return _mm_blendv_epi8(m_Data, Value.m_Data, c);
}

LWSVector4<int32_t> LWSVector4<int32_t>::Blend_Less3(const LWSVector4<int32_t> &Rhs, const LWSVector4<int32_t> &Value) const {
	__m128i c = _mm_cmplt_epi32(m_Data, Rhs.m_Data);
	return _mm_blend_epi32(_mm_blendv_epi8(m_Data, Value.m_Data, c), m_Data, 0x8);
}

LWSVector4<int32_t> LWSVector4<int32_t>::Blend_Less2(const LWSVector4<int32_t> &Rhs, const LWSVector4<int32_t> &Value) const {
	__m128i c = _mm_cmplt_epi32(m_Data, Rhs.m_Data);
	return _mm_blend_epi32(_mm_blendv_epi8(m_Data, Value.m_Data, c), m_Data, 0xC);
}

LWSVector4<int32_t> LWSVector4<int32_t>::Blend_LessEqual(const LWSVector4<int32_t> &Rhs, const LWSVector4<int32_t> &Value) const {
	__m128i c = _mm_or_si128(_mm_cmplt_epi32(m_Data, Rhs.m_Data), _mm_cmpeq_epi32(m_Data, Rhs.m_Data));
	return _mm_blendv_epi8(m_Data, Value.m_Data, c);
}

LWSVector4<int32_t> LWSVector4<int32_t>::Blend_LessEqual3(const LWSVector4<int32_t> &Rhs, const LWSVector4<int32_t> &Value) const {
	__m128i c = _mm_or_si128(_mm_cmplt_epi32(m_Data, Rhs.m_Data), _mm_cmpeq_epi32(m_Data, Rhs.m_Data));
	return _mm_blend_epi32(_mm_blendv_epi8(m_Data, Value.m_Data, c), m_Data, 0x8);
}

LWSVector4<int32_t> LWSVector4<int32_t>::Blend_LessEqual2(const LWSVector4<int32_t> &Rhs, const LWSVector4<int32_t> &Value) const {
	__m128i c = _mm_or_si128(_mm_cmplt_epi32(m_Data, Rhs.m_Data), _mm_cmpeq_epi32(m_Data, Rhs.m_Data));
	return _mm_blend_epi32(_mm_blendv_epi8(m_Data, Value.m_Data, c), m_Data, 0xC);
}

LWSVector4<int32_t> LWSVector4<int32_t>::Blend_Greater(const LWSVector4<int32_t> &Rhs, const LWSVector4<int32_t> &Value) const {
	__m128i c = _mm_cmpgt_epi32(m_Data, Rhs.m_Data);
	return _mm_blendv_epi8(m_Data, Value.m_Data, c);
}

LWSVector4<int32_t> LWSVector4<int32_t>::Blend_Greater3(const LWSVector4<int32_t> &Rhs, const LWSVector4<int32_t> &Value) const {
	__m128i c = _mm_cmpgt_epi32(m_Data, Rhs.m_Data);
	return _mm_blend_epi32(_mm_blendv_epi8(m_Data, Value.m_Data, c), m_Data, 0x8);
}

LWSVector4<int32_t> LWSVector4<int32_t>::Blend_Greater2(const LWSVector4<int32_t> &Rhs, const LWSVector4<int32_t> &Value) const {
	__m128i c = _mm_cmpgt_epi32(m_Data, Rhs.m_Data);
	return _mm_blend_epi32(_mm_blendv_epi8(m_Data, Value.m_Data, c), m_Data, 0xC);
}

LWSVector4<int32_t> LWSVector4<int32_t>::Blend_GreaterEqual(const LWSVector4<int32_t> &Rhs, const LWSVector4<int32_t> &Value) const {
	__m128i c = _mm_or_si128(_mm_cmpgt_epi32(m_Data, Rhs.m_Data), _mm_cmpeq_epi32(m_Data, Rhs.m_Data));
	return _mm_blendv_epi8(m_Data, Value.m_Data, c);
}

LWSVector4<int32_t> LWSVector4<int32_t>::Blend_GreaterEqual3(const LWSVector4<int32_t> &Rhs, const LWSVector4<int32_t> &Value) const {
	__m128i c = _mm_or_si128(_mm_cmpgt_epi32(m_Data, Rhs.m_Data), _mm_cmpeq_epi32(m_Data, Rhs.m_Data));
	return _mm_blend_epi32(_mm_blendv_epi8(m_Data, Value.m_Data, c), m_Data, 0x8);
}

LWSVector4<int32_t> LWSVector4<int32_t>::Blend_GreaterEqual2(const LWSVector4<int32_t> &Rhs, const LWSVector4<int32_t> &Value) const {
	__m128i c = _mm_or_si128(_mm_cmpgt_epi32(m_Data, Rhs.m_Data), _mm_cmpeq_epi32(m_Data, Rhs.m_Data));
	return _mm_blend_epi32(_mm_blendv_epi8(m_Data, Value.m_Data, c), m_Data, 0xC);
}

LWSVector4<int32_t> LWSVector4<int32_t>::Blend_Equal(const LWSVector4<int32_t> &Rhs, const LWSVector4<int32_t> &Value) const {
	__m128i c = _mm_cmpeq_epi32(m_Data, Rhs.m_Data);
	return _mm_blendv_epi8(m_Data, Value.m_Data, c);
}

LWSVector4<int32_t> LWSVector4<int32_t>::Blend_Equal3(const LWSVector4<int32_t> &Rhs, const LWSVector4<int32_t> &Value) const {
	__m128i c = _mm_cmpeq_epi32(m_Data, Rhs.m_Data);
	return _mm_blend_epi32(_mm_blendv_epi8(m_Data, Value.m_Data, c), m_Data, 0x8);
}

LWSVector4<int32_t> LWSVector4<int32_t>::Blend_Equal2(const LWSVector4<int32_t> &Rhs, const LWSVector4<int32_t> &Value) const {
	__m128i c = _mm_cmpeq_epi32(m_Data, Rhs.m_Data);
	return _mm_blend_epi32(_mm_blendv_epi8(m_Data, Value.m_Data, c), m_Data, 0xC);
}

LWSVector4<int32_t> LWSVector4<int32_t>::Blend_NotEqual(const LWSVector4<int32_t> &Rhs, const LWSVector4<int32_t> &Value) const {
	__m128i c = _mm_cmpeq_epi32(m_Data, Rhs.m_Data);
	c = _mm_xor_si128(c, _mm_set1_epi32(-1));
	return _mm_blendv_epi8(m_Data, Value.m_Data, c);
}

/*! \brief compares x, y, and z component, if component is != rhs(use's int32_t epsilon for comparison) than stores value's component, otherwise keeps current component. */
LWSVector4<int32_t> LWSVector4<int32_t>::Blend_NotEqual3(const LWSVector4<int32_t> &Rhs, const LWSVector4<int32_t> &Value) const {
	__m128i c = _mm_cmpeq_epi32(m_Data, Rhs.m_Data);
	c = _mm_xor_si128(c, _mm_set1_epi32(-1));
	return _mm_blend_epi32(_mm_blendv_epi8(m_Data, Value.m_Data, c), m_Data, 0x8);
}

LWSVector4<int32_t> LWSVector4<int32_t>::Blend_NotEqual2(const LWSVector4<int32_t> &Rhs, const LWSVector4<int32_t> &Value) const {
	__m128i c = _mm_cmpeq_epi32(m_Data, Rhs.m_Data);
	c = _mm_xor_si128(c, _mm_set1_epi32(-1));
	return _mm_blend_epi32(_mm_blendv_epi8(m_Data, Value.m_Data, c), m_Data, 0xC);
}

bool LWSVector4<int32_t>::Less3(const LWSVector4<int32_t> &Rhs) const {
	__m128i one = _mm_set1_epi32(-1);
	__m128i t = _mm_cmplt_epi32(m_Data, Rhs.m_Data);
	return _mm_test_all_ones(_mm_blend_epi32(t, one, 0x8));
}

bool LWSVector4<int32_t>::Less2(const LWSVector4<int32_t> &Rhs) const {
	__m128i one = _mm_set1_epi32(-1);
	__m128i t = _mm_cmplt_epi32(m_Data, Rhs.m_Data);
	return _mm_test_all_ones(_mm_blend_epi32(t, one, 0xC));
}

bool LWSVector4<int32_t>::LessEqual3(const LWSVector4<int32_t> &Rhs) const {
	__m128i one = _mm_set1_epi32(-1);
	__m128i t = _mm_or_si128(_mm_cmplt_epi32(m_Data, Rhs.m_Data), _mm_cmpeq_epi32(m_Data, Rhs.m_Data));
	return _mm_test_all_ones(_mm_blend_epi32(t, one, 0x8));
}

bool LWSVector4<int32_t>::LessEqual2(const LWSVector4<int32_t> &Rhs) const {
	__m128i one = _mm_set1_epi32(-1);
	__m128i t = _mm_or_si128(_mm_cmplt_epi32(m_Data, Rhs.m_Data), _mm_cmpeq_epi32(m_Data, Rhs.m_Data));
	return _mm_test_all_ones(_mm_blend_epi32(t, one, 0xC));
}

bool LWSVector4<int32_t>::Greater3(const LWSVector4<int32_t> &Rhs) const {
	__m128i one = _mm_set1_epi32(-1);
	__m128i t = _mm_cmpgt_epi32(m_Data, Rhs.m_Data);
	return _mm_test_all_ones(_mm_blend_epi32(t, one, 0x8));
}

bool LWSVector4<int32_t>::Greater2(const LWSVector4<int32_t> &Rhs) const {
	__m128i one = _mm_set1_epi32(-1);
	__m128i t = _mm_cmpgt_epi32(m_Data, Rhs.m_Data);
	return _mm_test_all_ones(_mm_blend_epi32(t, one, 0xC));
}

bool LWSVector4<int32_t>::GreaterEqual3(const LWSVector4<int32_t> &Rhs) const {
	__m128i one = _mm_set1_epi32(-1);
	__m128i t = _mm_or_si128(_mm_cmpgt_epi32(m_Data, Rhs.m_Data), _mm_cmpeq_epi32(m_Data, Rhs.m_Data));
	return _mm_test_all_ones(_mm_blend_epi32(t, one, 0x8));
}

bool LWSVector4<int32_t>::GreaterEqual2(const LWSVector4<int32_t> &Rhs) const {
	__m128i one = _mm_set1_epi32(-1);
	__m128i t = _mm_or_si128(_mm_cmpgt_epi32(m_Data, Rhs.m_Data), _mm_cmpeq_epi32(m_Data, Rhs.m_Data));
	return _mm_test_all_ones(_mm_blend_epi32(t, one, 0xC));
}

LWSVector4<int32_t>& LWSVector4<int32_t>::operator = (const LWSVector4<int32_t>& Rhs) {
	m_Data = Rhs.m_Data;
	return *this;
}

LWSVector4<int32_t>& LWSVector4<int32_t>::operator += (const LWSVector4<int32_t>& Rhs) {
	m_Data = _mm_add_epi32(m_Data, Rhs.m_Data);
	return *this;
}

LWSVector4<int32_t>& LWSVector4<int32_t>::operator += (int32_t Rhs) {
	__m128i t = _mm_set1_epi32(Rhs);
	m_Data = _mm_add_epi32(m_Data, t);
	return *this;
}

LWSVector4<int32_t>& LWSVector4<int32_t>::operator -= (const LWSVector4<int32_t>& Rhs) {
	m_Data = _mm_sub_epi32(m_Data, Rhs.m_Data);
	return *this;
}

LWSVector4<int32_t>& LWSVector4<int32_t>::operator -= (int32_t Rhs) {
	__m128i t = _mm_set1_epi32(Rhs);
	m_Data = _mm_sub_epi32(m_Data, t);
	return *this;
}

LWSVector4<int32_t>& LWSVector4<int32_t>::operator *= (const LWSVector4<int32_t>& Rhs) {
	m_Data = _mm_mullo_epi32(m_Data, Rhs.m_Data);
	return *this;
}

LWSVector4<int32_t>& LWSVector4<int32_t>::operator *= (int32_t Rhs) {
	__m128i t = _mm_set1_epi32(Rhs);
	m_Data = _mm_mullo_epi32(m_Data, t);
	return *this;
}

LWSVector4<int32_t>& LWSVector4<int32_t>::operator /= (const LWSVector4<int32_t>& Rhs) {
	//No integer division, cast data to float, divide, cast back to m128i
	int32_t *L = AsArray();
	const int32_t *R = Rhs.AsArray();
	L[0] /= R[0];
	L[1] /= R[1];
	L[2] /= R[2];
	L[3] /= R[3];
	return *this;
}

LWSVector4<int32_t>& LWSVector4<int32_t>::operator /= (int32_t Rhs) {
	//No integer division, cast data to float, divide, cast back to m128i
	int32_t *L = AsArray();
	L[0] /= Rhs;
	L[1] /= Rhs;
	L[2] /= Rhs;
	L[3] /= Rhs;
	return *this;
}

LWSVector4<int32_t> operator + (const LWSVector4<int32_t>& Rhs) {
	return Rhs.m_Data;
}

LWSVector4<int32_t> operator - (const LWSVector4<int32_t>& Rhs) {
	return _mm_sign_epi32(Rhs.m_Data, _mm_set1_epi32(-1));
}

bool LWSVector4<int32_t>::operator > (const LWSVector4<int32_t> &Rhs) const {
	return _mm_test_all_ones(_mm_cmpgt_epi32(m_Data, Rhs.m_Data));
}

bool LWSVector4<int32_t>::operator >= (const LWSVector4<int32_t> &Rhs) const {
	return _mm_test_all_ones(_mm_cmpgt_epi32(m_Data, Rhs.m_Data)) || _mm_test_all_ones(_mm_cmpeq_epi32(m_Data, Rhs.m_Data));
}

bool LWSVector4<int32_t>::operator < (const LWSVector4<int32_t> &Rhs) const {
	return _mm_test_all_ones(_mm_cmplt_epi32(m_Data, Rhs.m_Data));
}

bool LWSVector4<int32_t>::operator <= (const LWSVector4<int32_t> &Rhs) const {
	return _mm_test_all_ones(_mm_cmplt_epi32(m_Data, Rhs.m_Data)) || _mm_test_all_ones(_mm_cmpeq_epi32(m_Data, Rhs.m_Data));
}

bool LWSVector4<int32_t>::operator == (const LWSVector4<int32_t>& Rhs) const {
	return _mm_test_all_ones(_mm_cmpeq_epi32(m_Data, Rhs.m_Data));
}

bool LWSVector4<int32_t>::operator != (const LWSVector4<int32_t>& Rhs) const {
	return !(*this == Rhs);
}

std::ostream& operator<<(std::ostream& o, const LWSVector4<int32_t>& v) {
	alignas(16) int32_t Values[4];
	_mm_store_si128((__m128i*)Values, v.m_Data);
	o << Values[0] << " " << Values[1] << " " << Values[2] << " " << Values[3];
	return o;
}

LWSVector4<int32_t> operator + (const LWSVector4<int32_t>& Lhs, const LWSVector4<int32_t>& Rhs) {
	return _mm_add_epi32(Lhs.m_Data, Rhs.m_Data);
}

LWSVector4<int32_t> operator + (const LWSVector4<int32_t>& Lhs, int32_t Rhs) {
	__m128i t = _mm_set1_epi32(Rhs);
	return _mm_add_epi32(Lhs.m_Data, t);
}

LWSVector4<int32_t> operator + (int32_t Lhs, const LWSVector4<int32_t>& Rhs) {
	__m128i t = _mm_set1_epi32(Lhs);
	return _mm_add_epi32(t, Rhs.m_Data);
}

LWSVector4<int32_t> operator - (const LWSVector4<int32_t>& Lhs, const LWSVector4<int32_t>& Rhs) {
	return _mm_sub_epi32(Lhs.m_Data, Rhs.m_Data);
}

LWSVector4<int32_t> operator - (const LWSVector4<int32_t>& Lhs, int32_t Rhs) {
	__m128i t = _mm_set1_epi32(Rhs);
	return _mm_sub_epi32(Lhs.m_Data, t);
}

LWSVector4<int32_t> operator - (int32_t Lhs, const LWSVector4<int32_t>& Rhs) {
	__m128i t = _mm_set1_epi32(Lhs);
	return _mm_sub_epi32(t, Rhs.m_Data);
}

LWSVector4<int32_t> operator * (const LWSVector4<int32_t>& Lhs, const LWSVector4<int32_t>& Rhs) {
	return _mm_mullo_epi32(Lhs.m_Data, Rhs.m_Data);
}

LWSVector4<int32_t> operator * (const LWSVector4<int32_t>& Lhs, int32_t Rhs) {
	__m128i t = _mm_set1_epi32(Rhs);
	return _mm_mullo_epi32(Lhs.m_Data, t);
}

LWSVector4<int32_t> operator * (int32_t Lhs, const LWSVector4<int32_t>& Rhs) {
	__m128i t = _mm_set1_epi32(Lhs);
	return _mm_mullo_epi32(t, Rhs.m_Data);
}

LWSVector4<int32_t> operator / (const LWSVector4<int32_t>& Lhs, const LWSVector4<int32_t>& Rhs) {
	const int32_t *L = Lhs.AsArray();
	const int32_t *R = Rhs.AsArray();
	return LWSVector4<int32_t>(L[0] / R[0], L[1] / R[1], L[2] / R[2], L[3] / R[3]);
}

LWSVector4<int32_t> operator / (const LWSVector4<int32_t>& Lhs, int32_t Rhs) {
	const int32_t *L = Lhs.AsArray();
	return LWSVector4<int32_t>(L[0] / Rhs, L[1] / Rhs, L[2] / Rhs, L[3] / Rhs);
}

LWSVector4<int32_t> operator / (int32_t Lhs, const LWSVector4<int32_t>& Rhs) {
	const int32_t *R = Rhs.AsArray();
	return LWSVector4<int32_t>(Lhs / R[0], Lhs / R[1], Lhs / R[2], Lhs / R[3]);
}

LWSVector4<int32_t> LWSVector4<int32_t>::AAAB(const LWSVector4<int32_t>& B) const {
	return _mm_blend_epi16(m_Data, B.m_Data, 0xC0);
}

LWSVector4<int32_t> LWSVector4<int32_t>::AABA(const LWSVector4<int32_t>& B) const {
	return _mm_blend_epi16(m_Data, B.m_Data, 0x30);
}

LWSVector4<int32_t> LWSVector4<int32_t>::AABB(const LWSVector4<int32_t>& B) const {
	return _mm_blend_epi16(m_Data, B.m_Data, 0xF0);
}

LWSVector4<int32_t> LWSVector4<int32_t>::ABAA(const LWSVector4<int32_t>& B) const {
	return _mm_blend_epi16(m_Data, B.m_Data, 0xC);
}

LWSVector4<int32_t> LWSVector4<int32_t>::ABAB(const LWSVector4<int32_t>& B) const {
	return _mm_blend_epi16(m_Data, B.m_Data, 0xCC);
}

LWSVector4<int32_t> LWSVector4<int32_t>::ABBA(const LWSVector4<int32_t>& B) const {
	return _mm_blend_epi16(m_Data, B.m_Data, 0x3C);
}

LWSVector4<int32_t> LWSVector4<int32_t>::ABBB(const LWSVector4<int32_t>& B) const {
	return _mm_blend_epi16(m_Data, B.m_Data, 0xFC);
}

LWSVector4<int32_t> LWSVector4<int32_t>::BAAA(const LWSVector4<int32_t>& B) const {
	return _mm_blend_epi16(m_Data, B.m_Data, 0x3);
}

LWSVector4<int32_t> LWSVector4<int32_t>::BAAB(const LWSVector4<int32_t>& B) const {
	return _mm_blend_epi16(m_Data, B.m_Data, 0xC3);
}

LWSVector4<int32_t> LWSVector4<int32_t>::BABA(const LWSVector4<int32_t>& B) const {
	return _mm_blend_epi16(m_Data, B.m_Data, 0x33);
}

LWSVector4<int32_t> LWSVector4<int32_t>::BABB(const LWSVector4<int32_t>& B) const {
	return _mm_blend_epi16(m_Data, B.m_Data, 0xF3);
}

LWSVector4<int32_t> LWSVector4<int32_t>::BBAA(const LWSVector4<int32_t>& B) const {
	return _mm_blend_epi16(m_Data, B.m_Data, 0xF);
}

LWSVector4<int32_t> LWSVector4<int32_t>::BBAB(const LWSVector4<int32_t>& B) const {
	return _mm_blend_epi16(m_Data, B.m_Data, 0xCF);
}

LWSVector4<int32_t> LWSVector4<int32_t>::BBBA(const LWSVector4<int32_t>& B) const {
	return _mm_blend_epi16(m_Data, B.m_Data, 0x3F);
}

LWSVector4<int32_t> LWSVector4<int32_t>::xxxx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 0, 0, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xxxy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 0, 0, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xxxz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 0, 0, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xxxw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 0, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xxyx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 1, 0, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xxyy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 1, 0, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xxyz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 1, 0, 0));
}
LWSVector4<int32_t> LWSVector4<int32_t>::xxyw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 0, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xxzx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 2, 0, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xxzy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 2, 0, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xxzz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 2, 0, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xxzw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 0, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xxwx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 3, 0, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xxwy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 3, 0, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xxwz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 3, 0, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xxww(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 3, 0, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xyxx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 0, 1, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xyxy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 0, 1, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xyxz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 0, 1, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xyxw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 1, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xyyx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 1, 1, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xyyy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 1, 1, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xyyz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 1, 1, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xyyw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 1, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xyzx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 2, 1, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xyzy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 2, 1, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xyzz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 2, 1, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xywx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 3, 1, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xywy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 3, 1, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xywz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 3, 1, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xyww(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 3, 1, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xzxx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 0, 2, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xzxy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 0, 2, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xzxz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 0, 2, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xzxw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 2, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xzyx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 1, 2, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xzyy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 1, 2, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xzyz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 1, 2, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xzyw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 2, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xzzx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 2, 2, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xzzy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 2, 2, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xzzz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 2, 2, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xzzw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 2, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xzwx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 3, 2, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xzwy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 3, 2, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xzwz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 3, 2, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xzww(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 3, 2, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xwxx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 0, 3, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xwxy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 0, 3, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xwxz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 0, 3, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xwxw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 3, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xwyx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 1, 3, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xwyy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 1, 3, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xwyz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 1, 3, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xwyw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 3, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xwzx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 2, 3, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xwzy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 2, 3, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xwzz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 2, 3, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xwzw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 3, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xwwx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 3, 3, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xwwy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 3, 3, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xwwz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 3, 3, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xwww(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 3, 3, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yxxx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 0, 0, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yxxy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 0, 0, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yxxz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 0, 0, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yxxw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 0, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yxyx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 1, 0, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yxyy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 1, 0, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yxyz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 1, 0, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yxyw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 0, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yxzx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 2, 0, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yxzy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 2, 0, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yxzz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 2, 0, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yxzw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 0, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yxwx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 3, 0, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yxwy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 3, 0, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yxwz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 3, 0, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yxww(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 3, 0, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yyxx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 0, 1, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yyxy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 0, 1, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yyxz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 0, 1, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yyxw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 1, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yyyx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 1, 1, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yyyy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 1, 1, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yyyz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 1, 1, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yyyw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 1, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yyzx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 2, 1, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yyzy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 2, 1, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yyzz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 2, 1, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yyzw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 1, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yywx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 3, 1, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yywy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 3, 1, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yywz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 3, 1, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yyww(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 3, 1, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yzxx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 0, 2, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yzxy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 0, 2, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yzxz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 0, 2, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yzxw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 2, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yzyx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 1, 2, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yzyy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 1, 2, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yzyz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 1, 2, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yzyw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 2, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yzzx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 2, 2, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yzzy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 2, 2, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yzzz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 2, 2, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yzzw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 2, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yzwx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 3, 2, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yzwy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 3, 2, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yzwz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 3, 2, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yzww(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 3, 2, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::ywxx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 0, 3, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::ywxy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 0, 3, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::ywxz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 0, 3, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::ywxw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 3, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::ywyx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 1, 3, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::ywyy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 1, 3, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::ywyz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 1, 3, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::ywyw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 3, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::ywzx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 2, 3, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::ywzy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 2, 3, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::ywzz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 2, 3, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::ywzw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 3, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::ywwx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 3, 3, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::ywwy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 3, 3, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::ywwz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 3, 3, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::ywww(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 3, 3, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zxxx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 0, 0, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zxxy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 0, 0, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zxxz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 0, 0, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zxxw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 0, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zxyx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 1, 0, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zxyy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 1, 0, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zxyz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 1, 0, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zxyw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 0, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zxzx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 2, 0, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zxzy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 2, 0, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zxzz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 2, 0, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zxzw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 0, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zxwx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 3, 0, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zxwy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 3, 0, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zxwz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 3, 0, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zxww(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 3, 0, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zyxx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 0, 1, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zyxy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 0, 1, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zyxz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 0, 1, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zyxw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 1, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zyyx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 1, 1, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zyyy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 1, 1, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zyyz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 1, 1, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zyyw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 1, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zyzx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 2, 1, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zyzy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 2, 1, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zyzz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 2, 1, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zyzw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 1, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zywx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 3, 1, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zywy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 3, 1, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zywz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 3, 1, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zyww(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 3, 1, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zzxx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 0, 2, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zzxy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 0, 2, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zzxz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 0, 2, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zzxw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 2, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zzyx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 1, 2, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zzyy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 1, 2, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zzyz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 1, 2, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zzyw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 2, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zzzx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 2, 2, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zzzy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 2, 2, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zzzz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 2, 2, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zzzw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 2, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zzwx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 3, 2, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zzwy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 3, 2, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zzwz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 3, 2, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zzww(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 3, 2, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zwxx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 0, 3, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zwxy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 0, 3, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zwxz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 0, 3, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zwxw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 3, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zwyx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 1, 3, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zwyy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 1, 3, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zwyz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 1, 3, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zwyw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 3, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zwzx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 2, 3, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zwzy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 2, 3, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zwzz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 2, 3, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zwzw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 3, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zwwx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 3, 3, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zwwy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 3, 3, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zwwz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 3, 3, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zwww(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 3, 3, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wxxx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 0, 0, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wxxy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 0, 0, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wxxz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 0, 0, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wxxw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 0, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wxyx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 1, 0, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wxyy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 1, 0, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wxyz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 1, 0, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wxyw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 0, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wxzx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 2, 0, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wxzy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 2, 0, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wxzz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 2, 0, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wxzw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 0, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wxwx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 3, 0, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wxwy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 3, 0, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wxwz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 3, 0, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wxww(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 3, 0, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wyxx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 0, 1, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wyxy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 0, 1, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wyxz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 0, 1, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wyxw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 1, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wyyx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 1, 1, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wyyy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 1, 1, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wyyz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 1, 1, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wyyw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 1, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wyzx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 2, 1, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wyzy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 2, 1, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wyzz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 2, 1, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wyzw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 1, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wywx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 3, 1, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wywy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 3, 1, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wywz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 3, 1, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wyww(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 3, 1, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wzxx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 0, 2, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wzxy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 0, 2, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wzxz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 0, 2, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wzxw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 2, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wzyx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 1, 2, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wzyy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 1, 2, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wzyz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 1, 2, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wzyw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 2, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wzzx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 2, 2, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wzzy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 2, 2, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wzzz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 2, 2, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wzzw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 2, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wzwx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 3, 2, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wzwy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 3, 2, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wzwz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 3, 2, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wzww(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 3, 2, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wwxx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 0, 3, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wwxy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 0, 3, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wwxz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 0, 3, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wwxw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 3, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wwyx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 1, 3, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wwyy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 1, 3, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wwyz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 1, 3, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wwyw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 3, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wwzx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 2, 3, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wwzy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 2, 3, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wwzz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 2, 3, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wwzw(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 3, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wwwx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 3, 3, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wwwy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 3, 3, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wwwz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 3, 3, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::wwww(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 3, 3, 3));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xxx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 0, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xxy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 0, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xxz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 0, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xyx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 1, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xyy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 1, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xyz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 1, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xzx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 2, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xzy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 2, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xzz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 2, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yxx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 0, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yxy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 0, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yxz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 0, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yyx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 1, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yyy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 1, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yyz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 1, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yzx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 2, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yzy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 2, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yzz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 2, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zxx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 0, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zxy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 0, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zxz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 0, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zyx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 1, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zyy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 1, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zyz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 1, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zzx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 2, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zzy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 2, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::zzz(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 2, 2));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 0, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::xy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 1, 0));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yx(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 0, 1));
}

LWSVector4<int32_t> LWSVector4<int32_t>::yy(void) const {
	return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 1, 1));
}

int32_t LWSVector4<int32_t>::x(void) const {
	return ((int32_t*)&m_Data)[0];
}

int32_t LWSVector4<int32_t>::y(void) const {
	return ((int32_t*)&m_Data)[1];
}

int32_t LWSVector4<int32_t>::z(void) const {
	return ((int32_t*)&m_Data)[2];

}

int32_t LWSVector4<int32_t>::w(void) const {
	return ((int32_t*)&m_Data)[3];
}

LWSVector4<int32_t>::LWSVector4(__m128i Data) : m_Data(Data) {}

LWSVector4<int32_t>::LWSVector4(const LWVector4<int32_t>& vxyzw) : m_Data(_mm_set_epi32(vxyzw.w, vxyzw.z, vxyzw.y, vxyzw.x)) {}

LWSVector4<int32_t>::LWSVector4(const LWVector3<int32_t>& vxyz, int32_t vw) : m_Data(_mm_set_epi32(vw, vxyz.z, vxyz.y, vxyz.x)) {}

LWSVector4<int32_t>::LWSVector4(const LWVector2<int32_t>& vxy, const LWVector2<int32_t>& vzw) : m_Data(_mm_set_epi32(vzw.y, vzw.x, vxy.y, vxy.x)) {}

LWSVector4<int32_t>::LWSVector4(const LWVector2<int32_t>& vxy, int32_t vz, int32_t vw) : m_Data(_mm_set_epi32(vw, vz, vxy.y, vxy.x)) {}

LWSVector4<int32_t>::LWSVector4(int32_t vx, int32_t vy, int32_t vz, int32_t vw) : m_Data(_mm_set_epi32(vw, vz, vy, vx)) {}

LWSVector4<int32_t>::LWSVector4(int32_t f) : m_Data(_mm_set1_epi32(f)) {}

#endif
