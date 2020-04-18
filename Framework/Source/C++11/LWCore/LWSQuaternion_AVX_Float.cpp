#include "LWCore/LWSQuaternion.h"
#include "LWCore/LWSMatrix.h"
#ifndef LW_NOAVX

LWSQuaternion<float> LWSQuaternion<float>::FromEuler(float Pitch, float Yaw, float Roll) {
	float c1 = cosf(Yaw * 0.5f);
	float c2 = cosf(Pitch * 0.5f);
	float c3 = cosf(Roll * 0.5f);
	float s1 = sinf(Yaw * 0.5f);
	float s2 = sinf(Pitch * 0.5f);
	float s3 = sinf(Roll * 0.5f);
	float c1c2 = c1 * c2;
	float s1s2 = s1 * s2;
	return LWSQuaternion(c1c2 * c3 - s1s2 * s3, c1c2 * s3 + s1s2 * c3, s1 * c2 * c3 + c1 * s2 * s3, c1 * s2 * c3 - s1 * c2 * s3);
};

LWSQuaternion<float> LWSQuaternion<float>::FromEuler(const LWVector3<float>& Euler) {
	return FromEuler(Euler.x, Euler.y, Euler.z);
}

LWSQuaternion<float> LWSQuaternion<float>::FromAxis(float xAxis, float yAxis, float zAxis, float Theta) {
	float s = sinf(Theta * 0.5f);
	return LWSQuaternion(cosf(Theta * 0.5f), xAxis * s, yAxis * s, zAxis * s);
}

LWSQuaternion<float> LWSQuaternion<float>::FromAxis(const LWVector4<float>& AxisAngle) {
	return FromAxis(AxisAngle.x, AxisAngle.y, AxisAngle.z, AxisAngle.w);
}

LWSQuaternion<float> LWSQuaternion<float>::SLERP(const LWSQuaternion<float>& A, const LWSQuaternion<float>& B, float t) {
	LWSQuaternion<float> Res = B;
	float D = A.Dot(B);
	if (D < 0.0f) {
		D = -D;
		Res = -Res;
	}
	if (D < 1.0f - std::numeric_limits<float>::epsilon()) {
		float Theta = acosf(D);
		float sT = 1.0f / sinf(Theta);
		float sA = sinf(Theta * (1.0f - t));
		float sB = sinf(Theta * t);
		return ((A * sA) + Res * sB * sT).Normalize();
	}
	return A;
};

LWSQuaternion<float> LWSQuaternion<float>::NLERP(const LWSQuaternion<float>& A, const LWSQuaternion<float>& B, float t) {
	return (A + (B - A) * t).Normalize();
}

LWQuaternion<float> LWSQuaternion<float>::AsQuaternion(void) const {
	alignas(16) LWQuaternion<float> R;
	_mm_store_ps(&R.x, m_Data);
	return R;
}

LWSVector4<float> LWSQuaternion<float>::AsSVec4(void) const {
	return LWSVector4<float>(m_Data);
};

/*1< \brief converts the quaternion into euler angles in (Pitch, Yaw, Roll order). */
LWSVector4<float> LWSQuaternion<float>::ToEuler(void) const {
	const float hE = 0.4999f;
	alignas(16) float vals[4];
	__m128 Sq = _mm_mul_ps(m_Data, m_Data);
	__m128 LenSq = _mm_hadd_ps(Sq, Sq);
	LenSq = _mm_hadd_ps(LenSq, LenSq);
	__m128 t = _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 3, 0, 1));
	t = _mm_permute_ps(_mm_mul_ps(t, m_Data), _MM_SHUFFLE(3, 1, 2, 0));
	t = _mm_hadd_ps(t, t);

	__m128 cP = _mm_mul_ps(LenSq, _mm_set_ps1(hE));
	__m128 cN = _mm_mul_ps(LenSq, _mm_set_ps1(-hE));
	__m128i GE = _mm_castps_si128(_mm_cmpge_ps(t, cP));
	__m128i LE = _mm_castps_si128(_mm_cmple_ps(t, cN));

	_mm_store_ps(vals, m_Data);

	if (_mm_test_all_ones(GE)) return LWSVector4<float>(LW_PI_2, 2.0f * atan2f(vals[0], vals[3]), 0.0f);
	if (_mm_test_all_ones(LE)) return LWSVector4<float>(-LW_PI_2, -2.0f * atan2f(vals[0], vals[3]), 0.0f);
	__m128 YSq = _mm_xor_ps(Sq, _mm_set_ps(0.0f, -0.0f, -0.0f, 0.0f));
	__m128 RSq = _mm_xor_ps(Sq, _mm_set_ps(0.0f, -0.0f, 0.0f, -0.0f));
	YSq = _mm_hadd_ps(YSq, YSq);
	YSq = _mm_hadd_ps(YSq, YSq);
	RSq = _mm_hadd_ps(RSq, RSq);
	RSq = _mm_hadd_ps(RSq, RSq);

	float Yaw = atan2f(2.0f * vals[1] * vals[3] - 2.0f * vals[0] * vals[2], _mm_cvtss_f32(YSq));
	float Pitch = asinf(2.0f * _mm_cvtss_f32(_mm_div_ps(t, LenSq)));
	float Roll = atan2f(2.0f * vals[0] * vals[3] - 2.0f * vals[1] * vals[2], _mm_cvtss_f32(RSq));
	return LWSVector4<float>(Pitch, Yaw, Roll, 0.0f);
}

LWSQuaternion<float> LWSQuaternion<float>::Normalize(void) const {
	const float e = std::numeric_limits<float>::epsilon();
	__m128 eps = _mm_set_ps1(e);
	__m128 t = _mm_dp_ps(m_Data, m_Data, 0xFF);
	if (_mm_test_all_ones(_mm_castps_si128(_mm_cmple_ps(t, eps)))) return _mm_set_ps1(0.0f);
	return _mm_div_ps(m_Data, _mm_sqrt_ps(t));
}

float LWSQuaternion<float>::LengthSq(void) const {
	return _mm_cvtss_f32(_mm_dp_ps(m_Data, m_Data, 0xFF));
}

float LWSQuaternion<float>::Length(void) const {
	return _mm_cvtss_f32(_mm_sqrt_ps(_mm_dp_ps(m_Data, m_Data, 0xFF)));
}

float LWSQuaternion<float>::Dot(const LWSQuaternion& O) const {
	return _mm_cvtss_f32(_mm_dp_ps(m_Data, O.m_Data, 0xFF));
}

LWSQuaternion<float> LWSQuaternion<float>::Conjugate(void) const {
	return _mm_xor_ps(m_Data, _mm_set_ps(-0.0f, -0.0f, -0.0f, 0.0f));
}

LWSQuaternion<float> LWSQuaternion<float>::Inverse(void) const {
	__m128 iLenSq = _mm_div_ps(_mm_set_ps1(1.0f), _mm_dp_ps(m_Data, m_Data, 0xFF));
	return _mm_mul_ps(_mm_xor_ps(m_Data, _mm_set_ps(-0.0f, -0.0f, -0.0f, 0.0f)), iLenSq);
}

LWSVector4<float> LWSQuaternion<float>::RotatePoint(const LWSVector4<float> Pnt) const {
	__m128 u = _mm_mul_ps(m_Data, _mm_set_ps(0.0f, 1.0f, 1.0f, 1.0f));
	__m128 dA = _mm_dp_ps(u, Pnt.m_Data, 0x7F);
	__m128 dB = _mm_mul_ps(u, u);

	__m128 Two = _mm_set_ps1(2.0f);
	__m128 mw = _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 3, 3, 3));
	__m128 wSq = _mm_mul_ps(mw, mw);
	//Do cross product:
	__m128 A = _mm_permute_ps(u, _MM_SHUFFLE(3, 0, 2, 1));
	__m128 B = _mm_permute_ps(Pnt.m_Data, _MM_SHUFFLE(3, 1, 0, 2));
	__m128 C = _mm_permute_ps(u, _MM_SHUFFLE(3, 1, 0, 2));
	__m128 D = _mm_permute_ps(Pnt.m_Data, _MM_SHUFFLE(3, 0, 2, 1));
	__m128 Crss = _mm_sub_ps(_mm_mul_ps(A, B), _mm_mul_ps(C, D));

	__m128 PtA = _mm_mul_ps(_mm_mul_ps(Two, dA), u);
	__m128 PtB = _mm_mul_ps(_mm_sub_ps(wSq, dB), Pnt.m_Data);
	__m128 PtC = _mm_mul_ps(_mm_mul_ps(Two, mw), Crss);

	__m128 r = _mm_add_ps(_mm_add_ps(PtA, PtB), PtC);
	r = _mm_blend_ps(_mm_mul_ps(r, _mm_set_ps(1.0f, 1.0f, 1.0f, 0.0f)), Pnt.m_Data, 0x8);
	return r;
}

bool LWSQuaternion<float>::operator == (const LWSQuaternion<float>& Rhs) const {
	__m128 e = _mm_set_ps1(std::numeric_limits<float>::epsilon());
	__m128 t = _mm_sub_ps(m_Data, Rhs.m_Data);
	t = _mm_andnot_ps(_mm_set_ps1(-0.0), t); //Get absolute value of difference.
	t = _mm_cmple_ps(t, e);
	return _mm_test_all_ones(_mm_castps_si128(t));
}

bool LWSQuaternion<float>::operator != (const LWSQuaternion<float>& Rhs) const {
	return !(*this == Rhs);
}

LWSQuaternion<float> LWSQuaternion<float>::operator+(const LWSQuaternion<float>& Rhs) const {
	return _mm_add_ps(m_Data, Rhs.m_Data);
}

LWSQuaternion<float> LWSQuaternion<float>:: operator-(const LWSQuaternion<float>& Rhs) const {
	return _mm_sub_ps(m_Data, Rhs.m_Data);
}

LWSQuaternion<float> LWSQuaternion<float>:: operator*(const LWSQuaternion<float>& Rhs) const {
	__m128 mwwww = _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 3, 3, 3));
	__m128 mxyzx = _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 2, 1, 0));
	__m128 myzxy = _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 0, 2, 1));
	__m128 mzxyz = _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 1, 0, 2));
	__m128 rwwwx = _mm_permute_ps(Rhs.m_Data, _MM_SHUFFLE(0, 3, 3, 3));
	__m128 rzxyy = _mm_permute_ps(Rhs.m_Data, _MM_SHUFFLE(1, 1, 0, 2));

	mxyzx = _mm_xor_ps(mxyzx, _mm_set_ps(0.0f, 0.0f, 0.0f, -0.0f));
	myzxy = _mm_xor_ps(myzxy, _mm_set_ps(0.0f, 0.0f, 0.0f, -0.0f));
	mzxyz = _mm_xor_ps(mzxyz, _mm_set_ps1(-0.0f));

	__m128 ryzxz = _mm_permute_ps(Rhs.m_Data, _MM_SHUFFLE(2, 0, 2, 1));
	__m128 r = _mm_mul_ps(mwwww, Rhs.m_Data);
	r = _mm_add_ps(r, _mm_mul_ps(mxyzx, rwwwx));
	r = _mm_add_ps(r, _mm_mul_ps(myzxy, rzxyy));
	r = _mm_add_ps(r, _mm_mul_ps(mzxyz, ryzxz));
	return r;
}

LWSQuaternion<float> LWSQuaternion<float>::operator*(float Rhs) const {
	return _mm_mul_ps(m_Data, _mm_set_ps1(Rhs));
}


std::ostream& operator<<(std::ostream& o, const LWSQuaternion<float>& q) {
	alignas(16) LWQuaternion<float> R;
	_mm_store_ps(&R.x, q.m_Data);
	o << R.x << " " << R.y << " " << R.z << " " << R.w;
	return o;
}

LWSQuaternion<float> &LWSQuaternion<float>::operator*=(const LWSQuaternion<float>& Rhs) {
	*this = (*this * Rhs);
	return *this;
}

LWSQuaternion<float>& LWSQuaternion<float>::operator*=(float Rhs) {
	m_Data = _mm_mul_ps(m_Data, _mm_set_ps1(Rhs));
	return *this;
}

LWSQuaternion<float>& LWSQuaternion<float>::operator+=(const LWSQuaternion<float>& Rhs) {
	m_Data = _mm_add_ps(m_Data, Rhs.m_Data);
	return *this;
}

LWSQuaternion<float>& LWSQuaternion<float>::operator-=(const LWSQuaternion<float>& Rhs) {
	m_Data = _mm_sub_ps(m_Data, Rhs.m_Data);
	return *this;
}

LWSQuaternion<float>& LWSQuaternion<float>::operator=(const LWSQuaternion<float>& Rhs) {
	m_Data = Rhs.m_Data;
	return *this;
}

LWSQuaternion<float> LWSQuaternion<float>::operator-() const {
	return _mm_xor_ps(m_Data, _mm_set_ps1(-0.0f));
}

float LWSQuaternion<float>::x(void) const {
	return _mm_cvtss_f32(_mm_permute_ps(m_Data, _MM_SHUFFLE(0, 0, 0, 0)));
}

float LWSQuaternion<float>::y(void) const {
	return _mm_cvtss_f32(_mm_permute_ps(m_Data, _MM_SHUFFLE(1, 1, 1, 1)));
}

float LWSQuaternion<float>::z(void) const {
	return _mm_cvtss_f32(_mm_permute_ps(m_Data, _MM_SHUFFLE(2, 2, 2, 2)));
}

float LWSQuaternion<float>::w(void) const {
	return _mm_cvtss_f32(_mm_permute_ps(m_Data, _MM_SHUFFLE(3, 3, 3, 3)));
}

LWSQuaternion<float>::LWSQuaternion(__m128 Data) : m_Data(Data) {}

LWSQuaternion<float>::LWSQuaternion(float vw, float vx, float vy, float vz) : m_Data(_mm_set_ps(vw, vz, vy, vx)) {}

LWSQuaternion<float>::LWSQuaternion(const LWSMatrix4<float>& Mat) {
	LWVector4<float> R0 = Mat.Row(0).AsVec4();
	LWVector4<float> R1 = Mat.Row(1).AsVec4();
	LWVector4<float> R2 = Mat.Row(2).AsVec4();
	LWVector4<float> R3 = Mat.Row(3).AsVec4();

	float tr = R0.x + R1.y + R2.z;
	if (tr > 0.0f) {
		float s = sqrtf(tr + 1.0f) * 2.0f;
		float iS = 1.0f / s;
		*this = LWSQuaternion(0.25f * s, (R2.y - R1.z) * iS, (R0.z - R2.x) * iS, (R1.x - R0.y) * iS).Normalize();
		return;
	} else if (R0.x > R1.y && R0.x > R2.z) {
		float s = sqrtf(1.0f + R0.x - R1.y - R2.z) * 2.0f;
		float iS = 1.0f / s;
		*this = LWSQuaternion((R2.y - R1.z) * iS, 0.25f * s, (R0.y + R1.x) * iS, (R0.z + R2.x) * iS).Normalize();
		return;
	} else if (R1.y > R2.z) {
		float s = sqrtf(1.0f + R1.y - R0.x - R2.z) * 2.0f;
		float iS = 1.0f / s;
		*this = LWSQuaternion((R0.z - R2.x) * iS, (R0.y + R1.x) * iS, 0.25f * s, (R1.z + R2.y) * iS).Normalize();
		return;
	}
	float s = sqrtf(1.0f + R2.z - R0.x - R1.y) * 2.0f;
	float iS = 1.0f / s;
	*this = LWSQuaternion((R1.x - R0.y) * iS, (R0.z + R2.x) * iS, (R1.z + R2.y) * iS, 0.25f * s).Normalize();
}

	LWSQuaternion<float>::LWSQuaternion() : m_Data(_mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f)) {}

#endif