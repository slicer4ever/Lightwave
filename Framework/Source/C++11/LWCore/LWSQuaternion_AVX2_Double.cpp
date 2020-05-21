#include "LWCore/LWSQuaternion.h"
#include "LWCore/LWSMatrix.h"
#ifndef LW_NOAVX2

LWSQuaternion<double> LWSQuaternion<double>::FromEuler(double Pitch, double Yaw, double Roll) {
	double c1 = cos(Yaw * 0.5);
	double c2 = cos(Pitch * 0.5);
	double c3 = cos(Roll * 0.5);
	double s1 = sin(Yaw * 0.5);
	double s2 = sin(Pitch * 0.5);
	double s3 = sin(Roll * 0.5);
	double c1c2 = c1 * c2;
	double s1s2 = s1 * s2;
	return LWSQuaternion(c1c2 * c3 - s1s2 * s3, c1c2 * s3 + s1s2 * c3, s1 * c2 * c3 + c1 * s2 * s3, c1 * s2 * c3 - s1 * c2 * s3);
};

LWSQuaternion<double> LWSQuaternion<double>::FromEuler(const LWVector3<double>& Euler) {
	return FromEuler(Euler.x, Euler.y, Euler.z);
}

LWSQuaternion<double> LWSQuaternion<double>::FromAxis(double xAxis, double yAxis, double zAxis, double Theta) {
	double s = sin(Theta * 0.5f);
	return LWSQuaternion<double>(cos(Theta * 0.5f), xAxis * s, yAxis * s, zAxis * s);
}

LWSQuaternion<double> LWSQuaternion<double>::FromAxis(const LWVector4<double>& AxisAngle) {
	return FromAxis(AxisAngle.x, AxisAngle.y, AxisAngle.z, AxisAngle.w);
}

LWSQuaternion<double> LWSQuaternion<double>::SLERP(const LWSQuaternion<double>& A, const LWSQuaternion<double>& B, double t) {
	LWSQuaternion<double> Res = B;
	double D = A.Dot(B);
	if (D < 0.0f) {
		D = -D;
		Res = -Res;
	}
	if (D < 1.0f - std::numeric_limits<double>::epsilon()) {
		double Theta = acos(D);
		double sT = 1.0f / sin(Theta);
		double sA = sin(Theta * (1.0 - t));
		double sB = sin(Theta * t);
		return ((A * sA) + Res * sB * sT).Normalize();
	}
	return A;
};

LWSQuaternion<double> LWSQuaternion<double>::NLERP(const LWSQuaternion<double>& A, const LWSQuaternion<double>& B, double t) {
	return (A + (B - A) * t).Normalize();
}

LWQuaternion<double> LWSQuaternion<double>::AsQuaternion(void) const {
	alignas(32) LWQuaternion<double> R;
	_mm256_store_pd(&R.x, m_Data);
	return R;
}

LWSVector4<double> LWSQuaternion<double>::AsSVec4(void) const {
	return LWSVector4<double>(m_Data);
};

/*1< \brief converts the quaternion into euler angles in (Pitch, Yaw, Roll order). */
LWSVector4<double> LWSQuaternion<double>::ToEuler(void) const {
	const double hE = 0.49999;
	alignas(32) double vals[4];
	__m256d Sq = _mm256_mul_pd(m_Data, m_Data);

	__m256d LenSq = _mm256_hadd_pd(Sq, Sq);
	LenSq = _mm256_permute4x64_pd(LenSq, _MM_SHUFFLE(0, 2, 1, 3));
	LenSq = _mm256_hadd_pd(LenSq, LenSq);

	__m256d t = _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 3, 0, 1));
	t = _mm256_permute4x64_pd(_mm256_mul_pd(t, m_Data), _MM_SHUFFLE(3, 1, 2, 0));
	t = _mm256_hadd_pd(t, t);
	t = _mm256_permute4x64_pd(t, _MM_SHUFFLE(0, 2, 1, 3));

	__m256d cP = _mm256_mul_pd(LenSq, _mm256_set1_pd(hE));
	__m256d cN = _mm256_mul_pd(LenSq, _mm256_set1_pd(-hE));
	__m256i GE = _mm256_castpd_si256(_mm256_cmp_pd(t, cP, _CMP_GE_OS));
	__m256i LE = _mm256_castpd_si256(_mm256_cmp_pd(t, cN, _CMP_LE_OS));

	_mm256_store_pd(vals, m_Data);

	if (_mm256_test_all_ones(GE)) return LWSVector4<double>(LW_PI_2, 2.0 * atan2(vals[0], vals[3]), 0.0, 0.0);
	if (_mm256_test_all_ones(LE)) return LWSVector4<double>(-LW_PI_2, -2.0 * atan2(vals[0], vals[3]), 0.0, 0.0);
	__m256d YSq = _mm256_xor_pd(Sq, _mm256_set_pd(0.0, -0.0,-0.0,  0.0));
	__m256d RSq = _mm256_xor_pd(Sq, _mm256_set_pd(0.0, -0.0, 0.0, -0.0));
	YSq = _mm256_hadd_pd(YSq, YSq);
	YSq = _mm256_permute4x64_pd(YSq, _MM_SHUFFLE(0, 2, 1, 3));
	YSq = _mm256_hadd_pd(YSq, YSq);
	RSq = _mm256_hadd_pd(RSq, RSq);
	RSq = _mm256_permute4x64_pd(RSq, _MM_SHUFFLE(0, 2, 1, 3));
	RSq = _mm256_hadd_pd(RSq, RSq);

	double Yaw = atan2(2.0 * vals[1] * vals[3] - 2.0 * vals[0] * vals[2], _mm256_cvtsd_f64(YSq));
	double Pitch = asin(2.0 * _mm256_cvtsd_f64(_mm256_div_pd(t, LenSq)));
	double Roll = atan2(2.0 * vals[0] * vals[3] - 2.0 * vals[1] * vals[2], _mm256_cvtsd_f64(RSq));
	return LWSVector4<double>(Pitch, Yaw, Roll, 0.0);
}

LWSQuaternion<double> LWSQuaternion<double>::Normalize(void) const {
	const double e = std::numeric_limits<double>::epsilon();
	__m256d eps = _mm256_set1_pd(e);

	__m256d t = _mm256_mul_pd(m_Data, m_Data);
	t = _mm256_hadd_pd(t, t);
	t = _mm256_permute4x64_pd(t, _MM_SHUFFLE(0, 2, 1, 3));
	t = _mm256_hadd_pd(t, t);

	if (_mm256_test_all_ones(_mm256_castpd_si256(_mm256_cmp_pd(t, eps, _CMP_LE_OS)))) return _mm256_set1_pd(0.0);
	return _mm256_div_pd(m_Data, _mm256_sqrt_pd(t));
}

double LWSQuaternion<double>::LengthSq(void) const {
	__m256d t = _mm256_mul_pd(m_Data, m_Data);
	t = _mm256_hadd_pd(t, t);
	t = _mm256_permute4x64_pd(t, _MM_SHUFFLE(0, 2, 1, 3));
	t = _mm256_hadd_pd(t, t);
	return _mm256_cvtsd_f64(t);
}

double LWSQuaternion<double>::Length(void) const {
	__m256d t = _mm256_mul_pd(m_Data, m_Data);
	t = _mm256_hadd_pd(t, t);
	t = _mm256_permute4x64_pd(t, _MM_SHUFFLE(0, 2, 1, 3));
	t = _mm256_hadd_pd(t, t);
	return _mm256_cvtsd_f64(_mm256_sqrt_pd(t));
}

double LWSQuaternion<double>::Dot(const LWSQuaternion& O) const {
	__m256d t = _mm256_mul_pd(m_Data, O.m_Data);
	t = _mm256_hadd_pd(t, t);
	t = _mm256_permute4x64_pd(t, _MM_SHUFFLE(0, 2, 1, 3));
	t = _mm256_hadd_pd(t, t);
	return _mm256_cvtsd_f64(t);
}

LWSQuaternion<double> LWSQuaternion<double>::Conjugate(void) const {
	return _mm256_xor_pd(m_Data, _mm256_set_pd(0.0, -0.0, -0.0, -0.0));
}

LWSQuaternion<double> LWSQuaternion<double>::Inverse(void) const {
	__m256d t = _mm256_mul_pd(m_Data, m_Data);
	t = _mm256_hadd_pd(t, t);
	t = _mm256_permute4x64_pd(t, _MM_SHUFFLE(0, 2, 1, 3));
	t = _mm256_hadd_pd(t, t);

	__m256d iLenSq = _mm256_div_pd(_mm256_set1_pd(1.0), t);
	return _mm256_mul_pd(_mm256_xor_pd(m_Data, _mm256_set_pd(0.0, -0.0, -0.0, -0.0)), iLenSq);
}

LWSVector4<double> LWSQuaternion<double>::RotatePoint(const LWSVector4<double> Pnt) const {
	__m256d u = _mm256_mul_pd(m_Data, _mm256_set_pd(0.0, 1.0, 1.0, 1.0));

	//Dot u.Dot3(Pnt);
	__m256d dA = _mm256_mul_pd(u, Pnt.m_Data);
	dA = _mm256_hadd_pd(dA, dA);
	dA = _mm256_permute4x64_pd(dA, _MM_SHUFFLE(0, 2, 1, 3));
	dA = _mm256_hadd_pd(dA, dA);
	//u.LengthSq
	__m256d dB = _mm256_mul_pd(u, u);
	dB = _mm256_hadd_pd(dB, dB);
	dB = _mm256_permute4x64_pd(dB, _MM_SHUFFLE(0, 2, 1, 3));
	dB = _mm256_hadd_pd(dB, dB);

	__m256d Two = _mm256_set1_pd(2.0);
	__m256d mw = _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 3, 3, 3));
	__m256d wSq = _mm256_mul_pd(mw, mw);
	//Do cross product:
	__m256d A = _mm256_permute4x64_pd(u, _MM_SHUFFLE(3, 0, 2, 1));
	__m256d B = _mm256_permute4x64_pd(Pnt.m_Data, _MM_SHUFFLE(3, 1, 0, 2));
	__m256d C = _mm256_permute4x64_pd(u, _MM_SHUFFLE(3, 1, 0, 2));
	__m256d D = _mm256_permute4x64_pd(Pnt.m_Data, _MM_SHUFFLE(3, 0, 2, 1));
	__m256d Crss = _mm256_sub_pd(_mm256_mul_pd(A, B), _mm256_mul_pd(C, D));

	__m256d PtA = _mm256_mul_pd(_mm256_mul_pd(Two, dA), u);
	__m256d PtB = _mm256_mul_pd(_mm256_sub_pd(wSq, dB), Pnt.m_Data);
	__m256d PtC = _mm256_mul_pd(_mm256_mul_pd(Two, mw), Crss);

	__m256d r = _mm256_add_pd(_mm256_add_pd(PtA, PtB), PtC);
	r = _mm256_blend_pd(r, Pnt.m_Data, 0x8);
	return r;
}

bool LWSQuaternion<double>::operator == (const LWSQuaternion<double>& Rhs) const {
	//use float epison for closeness.
	__m256d e = _mm256_set1_pd((double)std::numeric_limits<float>::epsilon());
	__m256d t = _mm256_sub_pd(m_Data, Rhs.m_Data);
	t = _mm256_andnot_pd(_mm256_set1_pd(-0.0), t); //Get absolute value of difference.
	t = _mm256_cmp_pd(t, e, _CMP_LE_OS);
	return _mm256_test_all_ones(_mm256_castpd_si256(t));
}

bool LWSQuaternion<double>::operator != (const LWSQuaternion<double>& Rhs) const {
	return !(*this == Rhs);
}

LWSQuaternion<double> LWSQuaternion<double>::operator+(const LWSQuaternion<double>& Rhs) const {
	return _mm256_add_pd(m_Data, Rhs.m_Data);
}

LWSQuaternion<double> LWSQuaternion<double>:: operator-(const LWSQuaternion<double>& Rhs) const {
	return _mm256_sub_pd(m_Data, Rhs.m_Data);
}

LWSQuaternion<double> LWSQuaternion<double>:: operator*(const LWSQuaternion<double>& Rhs) const {
	__m256d mwwww = _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(3, 3, 3, 3));
	__m256d mxyzx = _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(0, 2, 1, 0));
	__m256d myzxy = _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(1, 0, 2, 1));
	__m256d mzxyz = _mm256_permute4x64_pd(m_Data, _MM_SHUFFLE(2, 1, 0, 2));
	__m256d rwwwx = _mm256_permute4x64_pd(Rhs.m_Data, _MM_SHUFFLE(0, 3, 3, 3));
	__m256d rzxyy = _mm256_permute4x64_pd(Rhs.m_Data, _MM_SHUFFLE(1, 1, 0, 2));

	mxyzx = _mm256_xor_pd(mxyzx, _mm256_set_pd(-0.0, 0.0, 0.0, 0.0));
	myzxy = _mm256_xor_pd(myzxy, _mm256_set_pd(-0.0, 0.0, 0.0, 0.0));
	mzxyz = _mm256_xor_pd(mzxyz, _mm256_set1_pd(-0.0));

	__m256d ryzxz = _mm256_permute4x64_pd(Rhs.m_Data, _MM_SHUFFLE(2, 0, 2, 1));
	__m256d r = _mm256_mul_pd(mwwww, Rhs.m_Data);
	r = _mm256_add_pd(r, _mm256_mul_pd(mxyzx, rwwwx));
	r = _mm256_add_pd(r, _mm256_mul_pd(myzxy, rzxyy));
	r = _mm256_add_pd(r, _mm256_mul_pd(mzxyz, ryzxz));
	return r;
}

LWSQuaternion<double> LWSQuaternion<double>::operator*(double Rhs) const {
	return _mm256_mul_pd(m_Data, _mm256_set1_pd(Rhs));
}


std::ostream& operator<<(std::ostream& o, const LWSQuaternion<double>& q) {
	alignas(32) LWQuaternion<double> R;
	_mm256_store_pd(&R.x, q.m_Data);
	o << R.w << " " << R.x << " " << R.y << " " << R.z;
	return o;
}

LWSQuaternion<double> &LWSQuaternion<double>::operator*=(const LWSQuaternion<double>& Rhs) {
	*this = (*this * Rhs);
	return *this;
}

LWSQuaternion<double>& LWSQuaternion<double>::operator*=(double Rhs) {
	m_Data = _mm256_mul_pd(m_Data, _mm256_set1_pd(Rhs));
	return *this;
}

LWSQuaternion<double>& LWSQuaternion<double>::operator+=(const LWSQuaternion<double>& Rhs) {
	m_Data = _mm256_add_pd(m_Data, Rhs.m_Data);
	return *this;
}

LWSQuaternion<double>& LWSQuaternion<double>::operator-=(const LWSQuaternion<double>& Rhs) {
	m_Data = _mm256_sub_pd(m_Data, Rhs.m_Data);
	return *this;
}

LWSQuaternion<double>& LWSQuaternion<double>::operator=(const LWSQuaternion<double>& Rhs) {
	m_Data = Rhs.m_Data;
	return *this;
}

LWSQuaternion<double> LWSQuaternion<double>::operator-() const {
	return _mm256_xor_pd(m_Data, _mm256_set1_pd(-0.0));
}

LWSQuaternion<double> operator * (double Lhs, const LWSQuaternion<double> &Rhs) {
	return _mm256_mul_pd(_mm256_set1_pd(Lhs), Rhs.m_Data);
}

LWSQuaternion<double> operator + (double Lhs, const LWSQuaternion<double> &Rhs) {
	return _mm256_add_pd(_mm256_set1_pd(Lhs), Rhs.m_Data);
}

LWSQuaternion<double> operator - (double Lhs, const LWSQuaternion<double> &Rhs) {
	return _mm256_sub_pd(_mm256_set1_pd(Lhs), Rhs.m_Data);
}

LWSQuaternion<double> operator / (double Lhs, const LWSQuaternion<double> &Rhs) {
	return _mm256_div_pd(_mm256_set1_pd(Lhs), Rhs.m_Data);
}

double LWSQuaternion<double>::x(void) const {
	return ((double*)&m_Data)[0];
}

double LWSQuaternion<double>::y(void) const {
	return ((double*)&m_Data)[1];
}

double LWSQuaternion<double>::z(void) const {
	return ((double*)&m_Data)[2];
}

double LWSQuaternion<double>::w(void) const {
	return ((double*)&m_Data)[3];
}

LWSQuaternion<double>::LWSQuaternion(__m256d Data) : m_Data(Data) {}

LWSQuaternion<double>::LWSQuaternion(const LWQuaternion<double> &Q) : m_Data(_mm256_set_pd(Q.w, Q.z, Q.y, Q.x)) {}

LWSQuaternion<double>::LWSQuaternion(double vw, double vx, double vy, double vz) : m_Data(_mm256_set_pd(vw, vz, vy, vx)) {}

LWSQuaternion<double>::LWSQuaternion(const LWSMatrix4<double> &Mat) {
	LWVector4<double> R0 = Mat.Row(0).AsVec4();
	LWVector4<double> R1 = Mat.Row(1).AsVec4();
	LWVector4<double> R2 = Mat.Row(2).AsVec4();
	LWVector4<double> R3 = Mat.Row(3).AsVec4();

	double tr = R0.x + R1.y + R2.z;
	if (tr > 0.0) {
		double s = sqrt(tr + 1.0) * 2.0;
		double iS = 1.0 / s;
		*this = LWSQuaternion(0.25 * s, (R2.y - R1.z) * iS, (R0.z - R2.x) * iS, (R1.x - R0.y) * iS).Normalize();
		return;
	} else if (R0.x > R1.y && R0.x > R2.z) {
		double s = sqrt(1.0 + R0.x - R1.y - R2.z) * 2.0;
		double iS = 1.0 / s;
		*this = LWSQuaternion((R2.y - R1.z) * iS, 0.25 * s, (R0.y + R1.x) * iS, (R0.z + R2.x) * iS).Normalize();
		return;
	} else if (R1.y > R2.z) {
		double s = sqrt(1.0 + R1.y - R0.x - R2.z) * 2.0;
		double iS = 1.0 / s;
		*this = LWSQuaternion((R0.z - R2.x) * iS, (R0.y + R1.x) * iS, 0.25 * s, (R1.z + R2.y) * iS).Normalize();
		return;
	}
	double s = sqrt(1.0 + R2.z - R0.x - R1.y) * 2.0;
	double iS = 1.0 / s;
	*this = LWSQuaternion((R1.x - R0.y) * iS, (R0.z + R2.x) * iS, (R1.z + R2.y) * iS, 0.25 * s).Normalize();
}

LWSQuaternion<double>::LWSQuaternion() : m_Data(_mm256_set_pd(1.0, 0.0, 0.0, 0.0)) {}

#endif