<<<<<<< HEAD
#ifndef LWSVECTOR_AVX_INT_H
#define LWSVECTOR_AVX_INT_H

#include "LWSVector.h"
#include <smmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

template<>
struct LWSVector4<int32_t> {
	__m128i m_Data;

	LWVector4<int32_t> AsVec4(void) const;

	int32_t *AsArray(void);

	const int32_t *AsArray(void) const;

	LWSVector4<int32_t> &sX(int32_t Value);

	LWSVector4<int32_t> &sY(int32_t Value);

	LWSVector4<int32_t> &sZ(int32_t Value);

	LWSVector4<int32_t> &sW(int32_t Value);

	LWSVector4<int32_t> Normalize(void) const;

	LWSVector4<int32_t> Normalize3(void) const;

	LWSVector4<int32_t> Normalize2(void) const;

	int32_t Min(void) const;

	int32_t Min3(void) const;

	int32_t Min2(void) const;

	int32_t Dot(const LWSVector4<int32_t>& O) const;

	int32_t Dot3(const LWSVector4<int32_t>& O) const;

	int32_t Dot2(const LWSVector4<int32_t>& O) const;

	LWSVector4<int32_t> Sum(void) const;

	int32_t Sum4(void) const;

	int32_t Sum3(void) const;

	int32_t Sum2(void) const;

	int32_t Max(void) const;

	int32_t Max3(void) const;

	int32_t Max2(void) const;

	LWSVector4<int32_t> Min(const LWSVector4<int32_t> &A) const;

	LWSVector4<int32_t> Max(const LWSVector4<int32_t> &A) const;

	LWSVector4<int32_t> Cross3(const LWSVector4<int32_t>& O) const;

	void Orthogonal3(LWSVector4<int32_t> &Right, LWSVector4<int32_t> &Up) const;

	LWSVector4<int32_t> Perpindicular2(void) const;

	int32_t Length(void) const;

	int32_t Length3(void) const;

	int32_t Length2(void) const;

	int32_t LengthSquared(void) const;

	int32_t LengthSquared3(void) const;

	int32_t LengthSquared2(void) const;

	int32_t Distance(const LWSVector4<int32_t>& O) const;

	int32_t Distance3(const LWSVector4<int32_t>& O) const;

	int32_t Distance2(const LWSVector4<int32_t>& O) const;

	int32_t DistanceSquared(const LWSVector4<int32_t>& O) const;

	int32_t DistanceSquared3(const LWSVector4<int32_t>& O) const;

	int32_t DistanceSquared2(const LWSVector4<int32_t>& O) const;

	/*! \brief returns the absolute value of each component. */
	LWSVector4<int32_t> Abs(void) const;

	/*! \brief returns the absolute value of x,y, and z component. */
	LWSVector4<int32_t> Abs3(void) const;

	/*! \brief returns the absolute value of x, and y component. */
	LWSVector4<int32_t> Abs2(void) const;

	/*! \brief compares each component, if component is < rhs, then stores Value's component, otherwise keeps current component. */
	LWSVector4<int32_t> Blend_Less(const LWSVector4<int32_t> &Rhs, const LWSVector4<int32_t> &Value) const;

	/*! \brief compares x, y, and z component, if component is < rhs than store's value's component, otherwise keeps current component. */
	LWSVector4<int32_t> Blend_Less3(const LWSVector4<int32_t> &Rhs, const LWSVector4<int32_t> &Value) const;

	/*! \brief compares x, and y component, if component is < rhs than store's value's component, otherwise keeps current component. */
	LWSVector4<int32_t> Blend_Less2(const LWSVector4<int32_t> &Rhs, const LWSVector4<int32_t> &Value) const;

	/*! \brief compares each component, if component is <= rhs, than stores Value's component, otherwise keeps current component. */
	LWSVector4<int32_t> Blend_LessEqual(const LWSVector4<int32_t> &Rhs, const LWSVector4<int32_t> &Value) const;

	/*! \brief compares x, y, and z component, if component is <= rhs, than stores Value's component, otherwise keeps current component. */
	LWSVector4<int32_t> Blend_LessEqual3(const LWSVector4<int32_t> &Rhs, const LWSVector4<int32_t> &Value) const;

	/*! \brief compares x, and y component, if component is <= rhs, than stores Value's component, otherwise keeps current component. */
	LWSVector4<int32_t> Blend_LessEqual2(const LWSVector4<int32_t> &Rhs, const LWSVector4<int32_t> &Value) const;

	/*! \brief compares each component, if component is > rhs than stores Value's component, otherwise keeps current component. */
	LWSVector4<int32_t> Blend_Greater(const LWSVector4<int32_t> &Rhs, const LWSVector4<int32_t> &Value) const;

	/*! \brief compares x, y, and z component, if component is > rhs than stores Value's component, otherwise keeps current component. */
	LWSVector4<int32_t> Blend_Greater3(const LWSVector4<int32_t> &Rhs, const LWSVector4<int32_t> &Value) const;

	/*! \brief compares x, and y component, if component is > rhs than stores Value's component, otherwise keeps current component. */
	LWSVector4<int32_t> Blend_Greater2(const LWSVector4<int32_t> &Rhs, const LWSVector4<int32_t> &Value) const;

	/*! \brief compares each component if component is >= rhs than stores value's component, otherwise keeps current component. */
	LWSVector4<int32_t> Blend_GreaterEqual(const LWSVector4<int32_t> &Rhs, const LWSVector4<int32_t> &Value) const;

	/*! \brief compares x, y, and z component if component is >= rhs than stores value's component, otherwise keeps current component. */
	LWSVector4<int32_t> Blend_GreaterEqual3(const LWSVector4<int32_t> &Rhs, const LWSVector4<int32_t> &Value) const;

	/*! \brief compares x, and y component if component is >= rhs than stores value's component, otherwise keeps current component. */
	LWSVector4<int32_t> Blend_GreaterEqual2(const LWSVector4<int32_t> &Rhs, const LWSVector4<int32_t> &Value) const;

	/*! \brief compares each component, if component is == rhs than stores value's component, otherwise keeps current component. */
	LWSVector4<int32_t> Blend_Equal(const LWSVector4<int32_t> &Rhs, const LWSVector4<int32_t> &Value) const;

	/*! \brief compares x, y, and z component, if component is == rhs than stores value's component, otherwise keeps current component. */
	LWSVector4<int32_t> Blend_Equal3(const LWSVector4<int32_t> &Rhs, const LWSVector4<int32_t> &Value) const;

	/*! \brief compares x, and y component, if component is == rhs than stores value's component, otherwise keeps current component. */
	LWSVector4<int32_t> Blend_Equal2(const LWSVector4<int32_t> &Rhs, const LWSVector4<int32_t> &Value) const;

	/*! \brief compares each component, if component is != rhs than stores value's component, otherwise keeps current component. */
	LWSVector4<int32_t> Blend_NotEqual(const LWSVector4<int32_t> &Rhs, const LWSVector4<int32_t> &Value) const;

	/*! \brief compares x, y, and z component, if component is != rhs than stores value's component, otherwise keeps current component. */
	LWSVector4<int32_t> Blend_NotEqual3(const LWSVector4<int32_t> &Rhs, const LWSVector4<int32_t> &Value) const;

	/*! \brief compares x, and y component, if component is != rhs than stores value's component, otherwise keeps current component. */
	LWSVector4<int32_t> Blend_NotEqual2(const LWSVector4<int32_t> &Rhs, const LWSVector4<int32_t> &Value) const;

	bool Less3(const LWSVector4<int32_t> &Rhs) const;

	bool Less2(const LWSVector4<int32_t> &Rhs) const;

	bool LessEqual3(const LWSVector4<int32_t> &Rhs) const;

	bool LessEqual2(const LWSVector4<int32_t> &Rhs) const;

	bool Greater3(const LWSVector4<int32_t> &Rhs) const;

	bool Greater2(const LWSVector4<int32_t> &Rhs) const;

	bool GreaterEqual3(const LWSVector4<int32_t> &Rhs) const;

	bool GreaterEqual2(const LWSVector4<int32_t> &Rhs) const;

	LWSVector4<int32_t>& operator = (const LWSVector4<int32_t>& Rhs);

	LWSVector4<int32_t>& operator += (const LWSVector4<int32_t>& Rhs);

	LWSVector4<int32_t>& operator += (int32_t Rhs);

	LWSVector4<int32_t>& operator -= (const LWSVector4<int32_t>& Rhs);

	LWSVector4<int32_t>& operator -= (int32_t Rhs);

	LWSVector4<int32_t>& operator *= (const LWSVector4<int32_t>& Rhs);

	LWSVector4<int32_t>& operator *= (int32_t Rhs);

	LWSVector4<int32_t>& operator /= (const LWSVector4<int32_t>& Rhs);

	LWSVector4<int32_t>& operator /= (int32_t Rhs);

	friend LWSVector4<int32_t> operator + (const LWSVector4<int32_t> &Rhs);

	friend LWSVector4<int32_t> operator - (const LWSVector4<int32_t> &Rhs);

	bool operator > (const LWSVector4<int32_t> &Rhs) const;

	bool operator >= (const LWSVector4<int32_t> &Rhs) const;

	bool operator < (const LWSVector4<int32_t> &Rhs) const;

	bool operator <= (const LWSVector4<int32_t> &Rhs) const;

	bool operator == (const LWSVector4<int32_t> &Rhs) const;

	bool operator != (const LWSVector4<int32_t> &Rhs) const;

	friend std::ostream& operator<<(std::ostream& o, const LWSVector4<int32_t>& v);

	friend LWSVector4<int32_t> operator + (const LWSVector4<int32_t> &Lhs, const LWSVector4<int32_t> &Rhs);

	friend LWSVector4<int32_t> operator + (const LWSVector4<int32_t>& Lhs, int32_t Rhs);

	friend LWSVector4<int32_t> operator + (int32_t Lhs, const LWSVector4<int32_t>& Rhs);

	friend LWSVector4<int32_t> operator - (const LWSVector4<int32_t> &Lhs, const LWSVector4<int32_t> &Rhs);

	friend LWSVector4<int32_t> operator - (const LWSVector4<int32_t>& Lhs, int32_t Rhs);

	friend LWSVector4<int32_t> operator - (int32_t Lhs, const LWSVector4<int32_t>& Rhs);

	friend LWSVector4<int32_t> operator * (const LWSVector4<int32_t> &Lhs, const LWSVector4<int32_t> &Rhs);

	friend LWSVector4<int32_t> operator * (const LWSVector4<int32_t>& Lhs, int32_t Rhs);

	friend LWSVector4<int32_t> operator * (int32_t Lhs, const LWSVector4<int32_t>& Rhs);

	friend LWSVector4<int32_t> operator / (const LWSVector4<int32_t>& Lhs, const LWSVector4<int32_t>& Rhs);

	friend LWSVector4<int32_t> operator / (const LWSVector4<int32_t>& Lhs, int32_t Rhs);

	friend LWSVector4<int32_t> operator / (int32_t Lhs, const LWSVector4<int32_t>& Rhs);

	LWSVector4<int32_t> AAAB(const LWSVector4<int32_t> &B) const;

	LWSVector4<int32_t> AABA(const LWSVector4<int32_t> &B) const;

	LWSVector4<int32_t> AABB(const LWSVector4<int32_t> &B) const;

	LWSVector4<int32_t> ABAA(const LWSVector4<int32_t> &B) const;

	LWSVector4<int32_t> ABAB(const LWSVector4<int32_t> &B) const;

	LWSVector4<int32_t> ABBA(const LWSVector4<int32_t> &B) const;

	LWSVector4<int32_t> ABBB(const LWSVector4<int32_t> &B) const;

	LWSVector4<int32_t> BAAA(const LWSVector4<int32_t> &B) const;

	LWSVector4<int32_t> BAAB(const LWSVector4<int32_t> &B) const;

	LWSVector4<int32_t> BABA(const LWSVector4<int32_t> &B) const;

	LWSVector4<int32_t> BABB(const LWSVector4<int32_t> &B) const;

	LWSVector4<int32_t> BBAA(const LWSVector4<int32_t> &B) const;

	LWSVector4<int32_t> BBAB(const LWSVector4<int32_t> &B) const;

	LWSVector4<int32_t> BBBA(const LWSVector4<int32_t> &B) const;

	LWSVector4<int32_t> xxxx(void) const;

	LWSVector4<int32_t> xxxy(void) const;

	LWSVector4<int32_t> xxxz(void) const;

	LWSVector4<int32_t> xxxw(void) const;

	LWSVector4<int32_t> xxyx(void) const;

	LWSVector4<int32_t> xxyy(void) const;

	LWSVector4<int32_t> xxyz(void) const;
	LWSVector4<int32_t> xxyw(void) const;

	LWSVector4<int32_t> xxzx(void) const;

	LWSVector4<int32_t> xxzy(void) const;

	LWSVector4<int32_t> xxzz(void) const;

	LWSVector4<int32_t> xxzw(void) const;

	LWSVector4<int32_t> xxwx(void) const;

	LWSVector4<int32_t> xxwy(void) const;

	LWSVector4<int32_t> xxwz(void) const;

	LWSVector4<int32_t> xxww(void) const;

	LWSVector4<int32_t> xyxx(void) const;

	LWSVector4<int32_t> xyxy(void) const;

	LWSVector4<int32_t> xyxz(void) const;

	LWSVector4<int32_t> xyxw(void) const;

	LWSVector4<int32_t> xyyx(void) const;

	LWSVector4<int32_t> xyyy(void) const;

	LWSVector4<int32_t> xyyz(void) const;

	LWSVector4<int32_t> xyyw(void) const;

	LWSVector4<int32_t> xyzx(void) const;

	LWSVector4<int32_t> xyzy(void) const;

	LWSVector4<int32_t> xyzz(void) const;

	LWSVector4<int32_t> xywx(void) const;

	LWSVector4<int32_t> xywy(void) const;

	LWSVector4<int32_t> xywz(void) const;

	LWSVector4<int32_t> xyww(void) const;

	LWSVector4<int32_t> xzxx(void) const;

	LWSVector4<int32_t> xzxy(void) const;

	LWSVector4<int32_t> xzxz(void) const;

	LWSVector4<int32_t> xzxw(void) const;

	LWSVector4<int32_t> xzyx(void) const;

	LWSVector4<int32_t> xzyy(void) const;

	LWSVector4<int32_t> xzyz(void) const;

	LWSVector4<int32_t> xzyw(void) const;

	LWSVector4<int32_t> xzzx(void) const;

	LWSVector4<int32_t> xzzy(void) const;

	LWSVector4<int32_t> xzzz(void) const;

	LWSVector4<int32_t> xzzw(void) const;

	LWSVector4<int32_t> xzwx(void) const;

	LWSVector4<int32_t> xzwy(void) const;

	LWSVector4<int32_t> xzwz(void) const;

	LWSVector4<int32_t> xzww(void) const;

	LWSVector4<int32_t> xwxx(void) const;

	LWSVector4<int32_t> xwxy(void) const;

	LWSVector4<int32_t> xwxz(void) const;

	LWSVector4<int32_t> xwxw(void) const;

	LWSVector4<int32_t> xwyx(void) const;

	LWSVector4<int32_t> xwyy(void) const;

	LWSVector4<int32_t> xwyz(void) const;

	LWSVector4<int32_t> xwyw(void) const;

	LWSVector4<int32_t> xwzx(void) const;

	LWSVector4<int32_t> xwzy(void) const;

	LWSVector4<int32_t> xwzz(void) const;

	LWSVector4<int32_t> xwzw(void) const;

	LWSVector4<int32_t> xwwx(void) const;

	LWSVector4<int32_t> xwwy(void) const;

	LWSVector4<int32_t> xwwz(void) const;

	LWSVector4<int32_t> xwww(void) const;

	LWSVector4<int32_t> yxxx(void) const;

	LWSVector4<int32_t> yxxy(void) const;

	LWSVector4<int32_t> yxxz(void) const;

	LWSVector4<int32_t> yxxw(void) const;

	LWSVector4<int32_t> yxyx(void) const;

	LWSVector4<int32_t> yxyy(void) const;

	LWSVector4<int32_t> yxyz(void) const;

	LWSVector4<int32_t> yxyw(void) const;

	LWSVector4<int32_t> yxzx(void) const;

	LWSVector4<int32_t> yxzy(void) const;

	LWSVector4<int32_t> yxzz(void) const;

	LWSVector4<int32_t> yxzw(void) const;

	LWSVector4<int32_t> yxwx(void) const;

	LWSVector4<int32_t> yxwy(void) const;

	LWSVector4<int32_t> yxwz(void) const;

	LWSVector4<int32_t> yxww(void) const;

	LWSVector4<int32_t> yyxx(void) const;

	LWSVector4<int32_t> yyxy(void) const;

	LWSVector4<int32_t> yyxz(void) const;

	LWSVector4<int32_t> yyxw(void) const;

	LWSVector4<int32_t> yyyx(void) const;

	LWSVector4<int32_t> yyyy(void) const;

	LWSVector4<int32_t> yyyz(void) const;

	LWSVector4<int32_t> yyyw(void) const;

	LWSVector4<int32_t> yyzx(void) const;

	LWSVector4<int32_t> yyzy(void) const;

	LWSVector4<int32_t> yyzz(void) const;

	LWSVector4<int32_t> yyzw(void) const;

	LWSVector4<int32_t> yywx(void) const;

	LWSVector4<int32_t> yywy(void) const;

	LWSVector4<int32_t> yywz(void) const;

	LWSVector4<int32_t> yyww(void) const;

	LWSVector4<int32_t> yzxx(void) const;

	LWSVector4<int32_t> yzxy(void) const;

	LWSVector4<int32_t> yzxz(void) const;

	LWSVector4<int32_t> yzxw(void) const;

	LWSVector4<int32_t> yzyx(void) const;

	LWSVector4<int32_t> yzyy(void) const;

	LWSVector4<int32_t> yzyz(void) const;

	LWSVector4<int32_t> yzyw(void) const;

	LWSVector4<int32_t> yzzx(void) const;

	LWSVector4<int32_t> yzzy(void) const;

	LWSVector4<int32_t> yzzz(void) const;

	LWSVector4<int32_t> yzzw(void) const;

	LWSVector4<int32_t> yzwx(void) const;

	LWSVector4<int32_t> yzwy(void) const;

	LWSVector4<int32_t> yzwz(void) const;

	LWSVector4<int32_t> yzww(void) const;

	LWSVector4<int32_t> ywxx(void) const;

	LWSVector4<int32_t> ywxy(void) const;

	LWSVector4<int32_t> ywxz(void) const;

	LWSVector4<int32_t> ywxw(void) const;

	LWSVector4<int32_t> ywyx(void) const;

	LWSVector4<int32_t> ywyy(void) const;

	LWSVector4<int32_t> ywyz(void) const;

	LWSVector4<int32_t> ywyw(void) const;

	LWSVector4<int32_t> ywzx(void) const;

	LWSVector4<int32_t> ywzy(void) const;

	LWSVector4<int32_t> ywzz(void) const;

	LWSVector4<int32_t> ywzw(void) const;

	LWSVector4<int32_t> ywwx(void) const;

	LWSVector4<int32_t> ywwy(void) const;

	LWSVector4<int32_t> ywwz(void) const;

	LWSVector4<int32_t> ywww(void) const;

	LWSVector4<int32_t> zxxx(void) const;

	LWSVector4<int32_t> zxxy(void) const;

	LWSVector4<int32_t> zxxz(void) const;

	LWSVector4<int32_t> zxxw(void) const;

	LWSVector4<int32_t> zxyx(void) const;

	LWSVector4<int32_t> zxyy(void) const;

	LWSVector4<int32_t> zxyz(void) const;

	LWSVector4<int32_t> zxyw(void) const;

	LWSVector4<int32_t> zxzx(void) const;

	LWSVector4<int32_t> zxzy(void) const;

	LWSVector4<int32_t> zxzz(void) const;

	LWSVector4<int32_t> zxzw(void) const;

	LWSVector4<int32_t> zxwx(void) const;

	LWSVector4<int32_t> zxwy(void) const;

	LWSVector4<int32_t> zxwz(void) const;

	LWSVector4<int32_t> zxww(void) const;

	LWSVector4<int32_t> zyxx(void) const;

	LWSVector4<int32_t> zyxy(void) const;

	LWSVector4<int32_t> zyxz(void) const;

	LWSVector4<int32_t> zyxw(void) const;

	LWSVector4<int32_t> zyyx(void) const;

	LWSVector4<int32_t> zyyy(void) const;

	LWSVector4<int32_t> zyyz(void) const;

	LWSVector4<int32_t> zyyw(void) const;

	LWSVector4<int32_t> zyzx(void) const;

	LWSVector4<int32_t> zyzy(void) const;

	LWSVector4<int32_t> zyzz(void) const;

	LWSVector4<int32_t> zyzw(void) const;

	LWSVector4<int32_t> zywx(void) const;

	LWSVector4<int32_t> zywy(void) const;

	LWSVector4<int32_t> zywz(void) const;

	LWSVector4<int32_t> zyww(void) const;

	LWSVector4<int32_t> zzxx(void) const;

	LWSVector4<int32_t> zzxy(void) const;

	LWSVector4<int32_t> zzxz(void) const;

	LWSVector4<int32_t> zzxw(void) const;

	LWSVector4<int32_t> zzyx(void) const;

	LWSVector4<int32_t> zzyy(void) const;

	LWSVector4<int32_t> zzyz(void) const;

	LWSVector4<int32_t> zzyw(void) const;

	LWSVector4<int32_t> zzzx(void) const;

	LWSVector4<int32_t> zzzy(void) const;

	LWSVector4<int32_t> zzzz(void) const;

	LWSVector4<int32_t> zzzw(void) const;

	LWSVector4<int32_t> zzwx(void) const;

	LWSVector4<int32_t> zzwy(void) const;

	LWSVector4<int32_t> zzwz(void) const;

	LWSVector4<int32_t> zzww(void) const;

	LWSVector4<int32_t> zwxx(void) const;

	LWSVector4<int32_t> zwxy(void) const;

	LWSVector4<int32_t> zwxz(void) const;

	LWSVector4<int32_t> zwxw(void) const;

	LWSVector4<int32_t> zwyx(void) const;

	LWSVector4<int32_t> zwyy(void) const;

	LWSVector4<int32_t> zwyz(void) const;

	LWSVector4<int32_t> zwyw(void) const;

	LWSVector4<int32_t> zwzx(void) const;

	LWSVector4<int32_t> zwzy(void) const;

	LWSVector4<int32_t> zwzz(void) const;

	LWSVector4<int32_t> zwzw(void) const;

	LWSVector4<int32_t> zwwx(void) const;

	LWSVector4<int32_t> zwwy(void) const;

	LWSVector4<int32_t> zwwz(void) const;

	LWSVector4<int32_t> zwww(void) const;

	LWSVector4<int32_t> wxxx(void) const;

	LWSVector4<int32_t> wxxy(void) const;

	LWSVector4<int32_t> wxxz(void) const;

	LWSVector4<int32_t> wxxw(void) const;

	LWSVector4<int32_t> wxyx(void) const;

	LWSVector4<int32_t> wxyy(void) const;

	LWSVector4<int32_t> wxyz(void) const;

	LWSVector4<int32_t> wxyw(void) const;

	LWSVector4<int32_t> wxzx(void) const;

	LWSVector4<int32_t> wxzy(void) const;

	LWSVector4<int32_t> wxzz(void) const;

	LWSVector4<int32_t> wxzw(void) const;

	LWSVector4<int32_t> wxwx(void) const;

	LWSVector4<int32_t> wxwy(void) const;

	LWSVector4<int32_t> wxwz(void) const;

	LWSVector4<int32_t> wxww(void) const;

	LWSVector4<int32_t> wyxx(void) const;

	LWSVector4<int32_t> wyxy(void) const;

	LWSVector4<int32_t> wyxz(void) const;

	LWSVector4<int32_t> wyxw(void) const;

	LWSVector4<int32_t> wyyx(void) const;

	LWSVector4<int32_t> wyyy(void) const;

	LWSVector4<int32_t> wyyz(void) const;

	LWSVector4<int32_t> wyyw(void) const;

	LWSVector4<int32_t> wyzx(void) const;

	LWSVector4<int32_t> wyzy(void) const;

	LWSVector4<int32_t> wyzz(void) const;

	LWSVector4<int32_t> wyzw(void) const;

	LWSVector4<int32_t> wywx(void) const;

	LWSVector4<int32_t> wywy(void) const;

	LWSVector4<int32_t> wywz(void) const;

	LWSVector4<int32_t> wyww(void) const;

	LWSVector4<int32_t> wzxx(void) const;

	LWSVector4<int32_t> wzxy(void) const;

	LWSVector4<int32_t> wzxz(void) const;

	LWSVector4<int32_t> wzxw(void) const;

	LWSVector4<int32_t> wzyx(void) const;

	LWSVector4<int32_t> wzyy(void) const;

	LWSVector4<int32_t> wzyz(void) const;

	LWSVector4<int32_t> wzyw(void) const;

	LWSVector4<int32_t> wzzx(void) const;

	LWSVector4<int32_t> wzzy(void) const;

	LWSVector4<int32_t> wzzz(void) const;

	LWSVector4<int32_t> wzzw(void) const;

	LWSVector4<int32_t> wzwx(void) const;

	LWSVector4<int32_t> wzwy(void) const;

	LWSVector4<int32_t> wzwz(void) const;

	LWSVector4<int32_t> wzww(void) const;

	LWSVector4<int32_t> wwxx(void) const;

	LWSVector4<int32_t> wwxy(void) const;

	LWSVector4<int32_t> wwxz(void) const;

	LWSVector4<int32_t> wwxw(void) const;

	LWSVector4<int32_t> wwyx(void) const;

	LWSVector4<int32_t> wwyy(void) const;

	LWSVector4<int32_t> wwyz(void) const;

	LWSVector4<int32_t> wwyw(void) const;

	LWSVector4<int32_t> wwzx(void) const;

	LWSVector4<int32_t> wwzy(void) const;

	LWSVector4<int32_t> wwzz(void) const;

	LWSVector4<int32_t> wwzw(void) const;

	LWSVector4<int32_t> wwwx(void) const;

	LWSVector4<int32_t> wwwy(void) const;

	LWSVector4<int32_t> wwwz(void) const;

	LWSVector4<int32_t> wwww(void) const;

	LWSVector4<int32_t> xxx(void) const;

	LWSVector4<int32_t> xxy(void) const;

	LWSVector4<int32_t> xxz(void) const;

	LWSVector4<int32_t> xyx(void) const;

	LWSVector4<int32_t> xyy(void) const;

	LWSVector4<int32_t> xyz(void) const;

	LWSVector4<int32_t> xzx(void) const;

	LWSVector4<int32_t> xzy(void) const;

	LWSVector4<int32_t> xzz(void) const;

	LWSVector4<int32_t> yxx(void) const;

	LWSVector4<int32_t> yxy(void) const;

	LWSVector4<int32_t> yxz(void) const;

	LWSVector4<int32_t> yyx(void) const;

	LWSVector4<int32_t> yyy(void) const;

	LWSVector4<int32_t> yyz(void) const;

	LWSVector4<int32_t> yzx(void) const;

	LWSVector4<int32_t> yzy(void) const;

	LWSVector4<int32_t> yzz(void) const;

	LWSVector4<int32_t> zxx(void) const;

	LWSVector4<int32_t> zxy(void) const;

	LWSVector4<int32_t> zxz(void) const;

	LWSVector4<int32_t> zyx(void) const;

	LWSVector4<int32_t> zyy(void) const;

	LWSVector4<int32_t> zyz(void) const;

	LWSVector4<int32_t> zzx(void) const;

	LWSVector4<int32_t> zzy(void) const;

	LWSVector4<int32_t> zzz(void) const;

	LWSVector4<int32_t> xx(void) const;

	LWSVector4<int32_t> xy(void) const;

	LWSVector4<int32_t> yx(void) const;

	LWSVector4<int32_t> yy(void) const;

	int32_t x(void) const;

	int32_t y(void) const;

	int32_t z(void) const;

	int32_t w(void) const;

	LWSVector4(__m128i Data);

	LWSVector4(const LWVector4<int32_t>& vxyzw);

	LWSVector4(const LWVector3<int32_t>& vxyz, int32_t vw);

	LWSVector4(const LWVector2<int32_t>& vxy, const LWVector2<int32_t>& vzw);

	LWSVector4(const LWVector2<int32_t>& vxy, int32_t vz, int32_t vw);

	LWSVector4(int32_t vx, int32_t vy, int32_t vz, int32_t vw);

	LWSVector4(int32_t f = 0);
};

=======
#ifndef LWSVECTOR_AVX_INT_H
#define LWSVECTOR_AVX_INT_H

#include "LWSVector.h"
#include <smmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

template<>
struct LWSVector4<int32_t> {
	__m128i m_Data;

	LWVector4<int32_t> AsVec4(void) const {
		alignas(16) LWVector4<int32_t> R;
		_mm_store_si128((__m128i*)&R.x, m_Data);
		return R;
	}

	int32_t Min(void) const {
		__m128i A = _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 1, 0, 3));
		__m128i B = _mm_min_epi32(m_Data, A);
		A = _mm_shuffle_epi32(B, _MM_SHUFFLE(1, 0, 3, 2));
		return _mm_cvtsi128_si32(_mm_min_epi32(B, A));
	}

	int32_t Min3(void) const {
		__m128i A = _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 0, 2));
		__m128i B = _mm_min_epi32(m_Data, A);
		A = _mm_shuffle_epi32(B, _MM_SHUFFLE(3, 0, 2, 1));
		return _mm_cvtsi128_si32(_mm_min_epi32(B, A));
	}

	int32_t Min2(void) const {
		__m128i A = _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 0, 1));
		return _mm_cvtsi128_si32(_mm_min_epi32(m_Data, A));
	}

	int32_t Max(void) const {
		__m128i A = _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 1, 0, 3));
		__m128i B = _mm_max_epi32(m_Data, A);
		A = _mm_shuffle_epi32(B, _MM_SHUFFLE(1, 0, 3, 2));
		return _mm_cvtsi128_si32(_mm_max_epi32(B, A));
	}

	int32_t Max3(void) const {
		__m128i A = _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 0, 2));
		__m128i B = _mm_max_epi32(m_Data, A);
		A = _mm_shuffle_epi32(B, _MM_SHUFFLE(3, 0, 2, 1));
		return _mm_cvtsi128_si32(_mm_max_epi32(B, A));
	}

	int32_t Max2(void) const {
		__m128i A = _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 0, 1));
		return _mm_cvtsi128_si32(_mm_max_epi32(m_Data, A));
	}

	LWSVector4<int32_t> Min(const LWSVector4<int32_t> &A) const {
		return _mm_min_epi32(m_Data, A.m_Data);
	}

	LWSVector4<int32_t> Max(const LWSVector4<int32_t> &A) const {
		return _mm_max_epi32(m_Data, A.m_Data);
	}

	LWSVector4<int32_t> &operator = (const LWSVector4<int32_t> &Rhs) {
		m_Data = Rhs.m_Data;
		return *this;
	}

	LWSVector4<int32_t> &operator += (const LWSVector4<int32_t> &Rhs) {
		m_Data = _mm_add_epi32(m_Data, Rhs.m_Data);
		return *this;
	}

	LWSVector4<int32_t> &operator += (int32_t Rhs) {
		__m128i t = _mm_set1_epi32(Rhs);
		m_Data = _mm_add_epi32(m_Data, t);
		return *this;
	}

	LWSVector4<int32_t> &operator -= (const LWSVector4<int32_t> &Rhs) {
		m_Data = _mm_sub_epi32(m_Data, Rhs.m_Data);
		return *this;
	}

	LWSVector4<int32_t> &operator -= (int32_t Rhs) {
		__m128i t = _mm_set1_epi32(Rhs);
		m_Data = _mm_sub_epi32(m_Data, t);
		return *this;
	}

	LWSVector4<int32_t> &operator *= (const LWSVector4<int32_t> &Rhs) {
		m_Data = _mm_mullo_epi32(m_Data, Rhs.m_Data);
		return *this;
	}

	LWSVector4<int32_t> &operator *= (int32_t Rhs) {
		__m128i t = _mm_set1_epi32(Rhs);
		m_Data = _mm_mullo_epi32(m_Data, t);
		return *this;
	}

	LWSVector4<int32_t> &operator /= (const LWSVector4<int32_t> &Rhs) {
		//No integer division, cast data to float, divide, cast back to m128i
		alignas (16) LWVector4i RiA;
		alignas (16) LWVector4i RiB;
		alignas (16) LWVector4f Rf;
		_mm_store_si128((__m128i*)&RiA.x, m_Data);
		_mm_store_si128((__m128i*)&RiB.x, Rhs.m_Data);
		__m128 tA = _mm_set_ps((float)RiA.x, (float)RiA.y, (float)RiA.z, (float)RiA.w);
		__m128 tB = _mm_set_ps((float)RiB.x, (float)RiB.y, (float)RiB.z, (float)RiB.w);
		__m128 t = _mm_div_ps(tA, tB);
		_mm_store_ps(&Rf.x, t);
		m_Data = _mm_set_epi32((int32_t)Rf.x, (int32_t)Rf.y, (int32_t)Rf.z, (int32_t)Rf.w);
		return *this;
	}

	LWSVector4<int32_t> &operator /= (int32_t Rhs) {
		//No integer division, cast data to float, divide, cast back to m128i
		alignas (16) LWVector4i RiA;
		alignas (16) LWVector4f Rf;
		_mm_store_si128((__m128i*)&RiA.x, m_Data);
		__m128 tA = _mm_set_ps((float)RiA.x, (float)RiA.y, (float)RiA.z, (float)RiA.w);
		__m128 tB = _mm_set_ps1((float)Rhs);
		__m128 t = _mm_div_ps(tA, tB);
		_mm_store_ps(&Rf.x, t);
		m_Data = _mm_set_epi32((int32_t)Rf.x, (int32_t)Rf.y, (int32_t)Rf.z, (int32_t)Rf.w);
		return *this;
	}

	friend LWSVector4<int32_t> operator + (const LWSVector4<int32_t> &Rhs) {
		return Rhs.m_Data;
	}

	friend LWSVector4<int32_t> operator - (const LWSVector4<int32_t> &Rhs) {
		return _mm_sign_epi32(Rhs.m_Data, _mm_set1_epi32(-1));
	}

	bool operator == (const LWSVector4<int32_t> &Rhs) {
		return _mm_test_all_ones(_mm_cmpeq_epi32(m_Data, Rhs.m_Data));
	}

	bool operator != (const LWSVector4<int32_t> &Rhs) {
		return !(*this == Rhs);
	}

	friend std::ostream &operator<<(std::ostream &o, const LWSVector4<int32_t> &v) {
		alignas(16) int32_t Values[4];
		_mm_store_si128((__m128i*)Values, v.m_Data);
		o << Values[0] << " " << Values[1] << " " << Values[2] << " " << Values[3];
		return o;
	}

	friend LWSVector4<int32_t> operator + (const LWSVector4<int32_t> &Lhs, const LWSVector4<int32_t> &Rhs) {
		return _mm_add_epi32(Lhs.m_Data, Rhs.m_Data);
	}

	friend LWSVector4<int32_t> operator + (const LWSVector4<int32_t> &Lhs, int32_t Rhs) {
		__m128i t = _mm_set1_epi32(Rhs);
		return _mm_add_epi32(Lhs.m_Data, t);
	}

	friend LWSVector4<int32_t> operator + (int32_t Lhs, const LWSVector4<int32_t> &Rhs) {
		__m128i t = _mm_set1_epi32(Lhs);
		return _mm_add_epi32(t, Rhs.m_Data);
	}

	friend LWSVector4<int32_t> operator - (const LWSVector4<int32_t> &Lhs, const LWSVector4<int32_t> &Rhs) {
		return _mm_sub_epi32(Lhs.m_Data, Rhs.m_Data);
	}

	friend LWSVector4<int32_t> operator - (const LWSVector4<int32_t> &Lhs, int32_t Rhs) {
		__m128i t = _mm_set1_epi32(Rhs);
		return _mm_sub_epi32(Lhs.m_Data, t);
	}

	friend LWSVector4<int32_t> operator - (int32_t Lhs, const LWSVector4<int32_t> &Rhs) {
		__m128i t = _mm_set1_epi32(Lhs);
		return _mm_sub_epi32(t, Rhs.m_Data);
	}

	friend LWSVector4<int32_t> operator * (const LWSVector4<int32_t> &Lhs, const LWSVector4<int32_t> &Rhs) {
		return _mm_mullo_epi32(Lhs.m_Data, Rhs.m_Data);
	}

	friend LWSVector4<int32_t> operator * (const LWSVector4<int32_t> &Lhs, int32_t Rhs) {
		__m128i t = _mm_set1_epi32(Rhs);
		return _mm_mullo_epi32(Lhs.m_Data, t);
	}

	friend LWSVector4<int32_t> operator * (int32_t Lhs, const LWSVector4<int32_t> &Rhs) {
		__m128i t = _mm_set1_epi32(Lhs);
		return _mm_mullo_epi32(t, Rhs.m_Data);
	}

	friend LWSVector4<int32_t> operator / (const LWSVector4<int32_t> &Lhs, const LWSVector4<int32_t> &Rhs) {
		alignas (16) LWVector4i RiA;
		alignas (16) LWVector4i RiB;
		alignas (16) LWVector4f Rf;
		_mm_store_si128((__m128i*)&RiA.x, Lhs.m_Data);
		_mm_store_si128((__m128i*)&RiB.x, Rhs.m_Data);
		__m128 tA = _mm_set_ps((float)RiA.x, (float)RiA.y, (float)RiA.z, (float)RiA.w);
		__m128 tB = _mm_set_ps((float)RiB.x, (float)RiB.y, (float)RiB.z, (float)RiB.w);
		__m128 t = _mm_div_ps(tA, tB);
		_mm_store_ps(&Rf.x, t);
		return _mm_set_epi32((int32_t)Rf.x, (int32_t)Rf.y, (int32_t)Rf.z, (int32_t)Rf.w);
	}

	friend LWSVector4<int32_t> operator / (const LWSVector4<int32_t> &Lhs, int32_t Rhs) {
		alignas (16) LWVector4i RiA;
		alignas (16) LWVector4f Rf;
		_mm_store_si128((__m128i*)&RiA.x, Lhs.m_Data);
		__m128 tA = _mm_set_ps((float)RiA.x, (float)RiA.y, (float)RiA.z, (float)RiA.w);
		__m128 tB = _mm_set_ps1((float)Rhs);
		__m128 t = _mm_div_ps(tA, tB);
		_mm_store_ps(&Rf.x, t);
		return _mm_set_epi32((int32_t)Rf.x, (int32_t)Rf.y, (int32_t)Rf.z, (int32_t)Rf.w);
	}

	friend LWSVector4<int32_t> operator / (int32_t Lhs, const LWSVector4<int32_t> &Rhs) {
		alignas (16) LWVector4f Rf;
		alignas (16) LWVector4i RiB;
		__m128 tA = _mm_set_ps1((float)Lhs);
		__m128 tB = _mm_set_ps((float)RiB.x, (float)RiB.y, (float)RiB.z, (float)RiB.w);
		__m128 t = _mm_div_ps(tA, tB);
		_mm_store_ps(&Rf.x, t);
		return _mm_set_epi32((int32_t)Rf.x, (int32_t)Rf.y, (int32_t)Rf.z, (int32_t)Rf.w);
	}

	LWSVector4<int32_t> xxxx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 0, 0, 0));
	}

	LWSVector4<int32_t> xxxy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 0, 0, 0));
	}

	LWSVector4<int32_t> xxxz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 0, 0, 0));
	}

	LWSVector4<int32_t> xxxw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 0, 0));
	}

	LWSVector4<int32_t> xxyx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 1, 0, 0));
	}

	LWSVector4<int32_t> xxyy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 1, 0, 0));
	}

	LWSVector4<int32_t> xxyz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 1, 0, 0));
	}
	LWSVector4<int32_t> xxyw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 0, 0));
	}

	LWSVector4<int32_t> xxzx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 2, 0, 0));
	}

	LWSVector4<int32_t> xxzy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 2, 0, 0));
	}

	LWSVector4<int32_t> xxzz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 2, 0, 0));
	}

	LWSVector4<int32_t> xxzw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 0, 0));
	}

	LWSVector4<int32_t> xxwx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 3, 0, 0));
	}

	LWSVector4<int32_t> xxwy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 3, 0, 0));
	}

	LWSVector4<int32_t> xxwz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 3, 0, 0));
	}

	LWSVector4<int32_t> xxww(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 3, 0, 0));
	}

	LWSVector4<int32_t> xyxx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 0, 1, 0));
	}

	LWSVector4<int32_t> xyxy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 0, 1, 0));
	}

	LWSVector4<int32_t> xyxz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 0, 1, 0));
	}

	LWSVector4<int32_t> xyxw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 1, 0));
	}

	LWSVector4<int32_t> xyyx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 1, 1, 0));
	}

	LWSVector4<int32_t> xyyy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 1, 1, 0));
	}

	LWSVector4<int32_t> xyyz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 1, 1, 0));
	}

	LWSVector4<int32_t> xyyw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 1, 0));
	}

	LWSVector4<int32_t> xyzx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 2, 1, 0));
	}

	LWSVector4<int32_t> xyzy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 2, 1, 0));
	}

	LWSVector4<int32_t> xyzz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 2, 1, 0));
	}

	LWSVector4<int32_t> xywx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 3, 1, 0));
	}

	LWSVector4<int32_t> xywy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 3, 1, 0));
	}

	LWSVector4<int32_t> xywz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 3, 1, 0));
	}

	LWSVector4<int32_t> xyww(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 3, 1, 0));
	}

	LWSVector4<int32_t> xzxx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 0, 2, 0));
	}

	LWSVector4<int32_t> xzxy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 0, 2, 0));
	}

	LWSVector4<int32_t> xzxz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 0, 2, 0));
	}

	LWSVector4<int32_t> xzxw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 2, 0));
	}

	LWSVector4<int32_t> xzyx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 1, 2, 0));
	}

	LWSVector4<int32_t> xzyy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 1, 2, 0));
	}

	LWSVector4<int32_t> xzyz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 1, 2, 0));
	}

	LWSVector4<int32_t> xzyw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 2, 0));
	}

	LWSVector4<int32_t> xzzx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 2, 2, 0));
	}

	LWSVector4<int32_t> xzzy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 2, 2, 0));
	}

	LWSVector4<int32_t> xzzz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 2, 2, 0));
	}

	LWSVector4<int32_t> xzzw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 2, 0));
	}

	LWSVector4<int32_t> xzwx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 3, 2, 0));
	}

	LWSVector4<int32_t> xzwy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 3, 2, 0));
	}

	LWSVector4<int32_t> xzwz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 3, 2, 0));
	}

	LWSVector4<int32_t> xzww(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 3, 2, 0));
	}

	LWSVector4<int32_t> xwxx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 0, 3, 0));
	}

	LWSVector4<int32_t> xwxy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 0, 3, 0));
	}

	LWSVector4<int32_t> xwxz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 0, 3, 0));
	}

	LWSVector4<int32_t> xwxw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 3, 0));
	}

	LWSVector4<int32_t> xwyx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 1, 3, 0));
	}

	LWSVector4<int32_t> xwyy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 1, 3, 0));
	}

	LWSVector4<int32_t> xwyz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 1, 3, 0));
	}

	LWSVector4<int32_t> xwyw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 3, 0));
	}

	LWSVector4<int32_t> xwzx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 2, 3, 0));
	}

	LWSVector4<int32_t> xwzy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 2, 3, 0));
	}

	LWSVector4<int32_t> xwzz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 2, 3, 0));
	}

	LWSVector4<int32_t> xwzw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 3, 0));
	}

	LWSVector4<int32_t> xwwx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 3, 3, 0));
	}

	LWSVector4<int32_t> xwwy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 3, 3, 0));
	}

	LWSVector4<int32_t> xwwz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 3, 3, 0));
	}

	LWSVector4<int32_t> xwww(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 3, 3, 0));
	}

	LWSVector4<int32_t> yxxx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 0, 0, 1));
	}

	LWSVector4<int32_t> yxxy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 0, 0, 1));
	}

	LWSVector4<int32_t> yxxz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 0, 0, 1));
	}

	LWSVector4<int32_t> yxxw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 0, 1));
	}

	LWSVector4<int32_t> yxyx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 1, 0, 1));
	}

	LWSVector4<int32_t> yxyy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 1, 0, 1));
	}

	LWSVector4<int32_t> yxyz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 1, 0, 1));
	}

	LWSVector4<int32_t> yxyw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 0, 1));
	}

	LWSVector4<int32_t> yxzx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 2, 0, 1));
	}

	LWSVector4<int32_t> yxzy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 2, 0, 1));
	}

	LWSVector4<int32_t> yxzz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 2, 0, 1));
	}

	LWSVector4<int32_t> yxzw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 0, 1));
	}

	LWSVector4<int32_t> yxwx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 3, 0, 1));
	}

	LWSVector4<int32_t> yxwy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 3, 0, 1));
	}

	LWSVector4<int32_t> yxwz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 3, 0, 1));
	}

	LWSVector4<int32_t> yxww(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 3, 0, 1));
	}

	LWSVector4<int32_t> yyxx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 0, 1, 1));
	}

	LWSVector4<int32_t> yyxy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 0, 1, 1));
	}

	LWSVector4<int32_t> yyxz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 0, 1, 1));
	}

	LWSVector4<int32_t> yyxw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 1, 1));
	}

	LWSVector4<int32_t> yyyx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 1, 1, 1));
	}

	LWSVector4<int32_t> yyyy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 1, 1, 1));
	}

	LWSVector4<int32_t> yyyz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 1, 1, 1));
	}

	LWSVector4<int32_t> yyyw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 1, 1));
	}

	LWSVector4<int32_t> yyzx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 2, 1, 1));
	}

	LWSVector4<int32_t> yyzy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 2, 1, 1));
	}

	LWSVector4<int32_t> yyzz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 2, 1, 1));
	}

	LWSVector4<int32_t> yyzw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 1, 1));
	}

	LWSVector4<int32_t> yywx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 3, 1, 1));
	}

	LWSVector4<int32_t> yywy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 3, 1, 1));
	}

	LWSVector4<int32_t> yywz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 3, 1, 1));
	}

	LWSVector4<int32_t> yyww(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 3, 1, 1));
	}

	LWSVector4<int32_t> yzxx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 0, 2, 1));
	}

	LWSVector4<int32_t> yzxy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 0, 2, 1));
	}

	LWSVector4<int32_t> yzxz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 0, 2, 1));
	}

	LWSVector4<int32_t> yzxw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 2, 1));
	}

	LWSVector4<int32_t> yzyx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 1, 2, 1));
	}

	LWSVector4<int32_t> yzyy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 1, 2, 1));
	}

	LWSVector4<int32_t> yzyz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 1, 2, 1));
	}

	LWSVector4<int32_t> yzyw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 2, 1));
	}

	LWSVector4<int32_t> yzzx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 2, 2, 1));
	}

	LWSVector4<int32_t> yzzy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 2, 2, 1));
	}

	LWSVector4<int32_t> yzzz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 2, 2, 1));
	}

	LWSVector4<int32_t> yzzw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 2, 1));
	}

	LWSVector4<int32_t> yzwx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 3, 2, 1));
	}

	LWSVector4<int32_t> yzwy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 3, 2, 1));
	}

	LWSVector4<int32_t> yzwz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 3, 2, 1));
	}

	LWSVector4<int32_t> yzww(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 3, 2, 1));
	}

	LWSVector4<int32_t> ywxx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 0, 3, 1));
	}

	LWSVector4<int32_t> ywxy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 0, 3, 1));
	}

	LWSVector4<int32_t> ywxz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 0, 3, 1));
	}

	LWSVector4<int32_t> ywxw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 3, 1));
	}

	LWSVector4<int32_t> ywyx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 1, 3, 1));
	}

	LWSVector4<int32_t> ywyy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 1, 3, 1));
	}

	LWSVector4<int32_t> ywyz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 1, 3, 1));
	}

	LWSVector4<int32_t> ywyw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 3, 1));
	}

	LWSVector4<int32_t> ywzx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 2, 3, 1));
	}

	LWSVector4<int32_t> ywzy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 2, 3, 1));
	}

	LWSVector4<int32_t> ywzz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 2, 3, 1));
	}

	LWSVector4<int32_t> ywzw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 3, 1));
	}

	LWSVector4<int32_t> ywwx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 3, 3, 1));
	}

	LWSVector4<int32_t> ywwy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 3, 3, 1));
	}

	LWSVector4<int32_t> ywwz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 3, 3, 1));
	}

	LWSVector4<int32_t> ywww(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 3, 3, 1));
	}

	LWSVector4<int32_t> zxxx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 0, 0, 2));
	}

	LWSVector4<int32_t> zxxy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 0, 0, 2));
	}

	LWSVector4<int32_t> zxxz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 0, 0, 2));
	}

	LWSVector4<int32_t> zxxw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 0, 2));
	}

	LWSVector4<int32_t> zxyx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 1, 0, 2));
	}

	LWSVector4<int32_t> zxyy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 1, 0, 2));
	}

	LWSVector4<int32_t> zxyz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 1, 0, 2));
	}

	LWSVector4<int32_t> zxyw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 0, 2));
	}

	LWSVector4<int32_t> zxzx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 2, 0, 2));
	}

	LWSVector4<int32_t> zxzy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 2, 0, 2));
	}

	LWSVector4<int32_t> zxzz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 2, 0, 2));
	}

	LWSVector4<int32_t> zxzw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 0, 2));
	}

	LWSVector4<int32_t> zxwx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 3, 0, 2));
	}

	LWSVector4<int32_t> zxwy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 3, 0, 2));
	}

	LWSVector4<int32_t> zxwz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 3, 0, 2));
	}

	LWSVector4<int32_t> zxww(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 3, 0, 2));
	}

	LWSVector4<int32_t> zyxx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 0, 1, 2));
	}

	LWSVector4<int32_t> zyxy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 0, 1, 2));
	}

	LWSVector4<int32_t> zyxz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 0, 1, 2));
	}

	LWSVector4<int32_t> zyxw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 1, 2));
	}

	LWSVector4<int32_t> zyyx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 1, 1, 2));
	}

	LWSVector4<int32_t> zyyy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 1, 1, 2));
	}

	LWSVector4<int32_t> zyyz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 1, 1, 2));
	}

	LWSVector4<int32_t> zyyw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 1, 2));
	}

	LWSVector4<int32_t> zyzx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 2, 1, 2));
	}

	LWSVector4<int32_t> zyzy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 2, 1, 2));
	}

	LWSVector4<int32_t> zyzz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 2, 1, 2));
	}

	LWSVector4<int32_t> zyzw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 1, 2));
	}

	LWSVector4<int32_t> zywx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 3, 1, 2));
	}

	LWSVector4<int32_t> zywy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 3, 1, 2));
	}

	LWSVector4<int32_t> zywz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 3, 1, 2));
	}

	LWSVector4<int32_t> zyww(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 3, 1, 2));
	}

	LWSVector4<int32_t> zzxx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 0, 2, 2));
	}

	LWSVector4<int32_t> zzxy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 0, 2, 2));
	}

	LWSVector4<int32_t> zzxz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 0, 2, 2));
	}

	LWSVector4<int32_t> zzxw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 2, 2));
	}

	LWSVector4<int32_t> zzyx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 1, 2, 2));
	}

	LWSVector4<int32_t> zzyy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 1, 2, 2));
	}

	LWSVector4<int32_t> zzyz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 1, 2, 2));
	}

	LWSVector4<int32_t> zzyw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 2, 2));
	}

	LWSVector4<int32_t> zzzx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 2, 2, 2));
	}

	LWSVector4<int32_t> zzzy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 2, 2, 2));
	}

	LWSVector4<int32_t> zzzz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 2, 2, 2));
	}

	LWSVector4<int32_t> zzzw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 2, 2));
	}

	LWSVector4<int32_t> zzwx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 3, 2, 2));
	}

	LWSVector4<int32_t> zzwy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 3, 2, 2));
	}

	LWSVector4<int32_t> zzwz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 3, 2, 2));
	}

	LWSVector4<int32_t> zzww(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 3, 2, 2));
	}

	LWSVector4<int32_t> zwxx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 0, 3, 2));
	}

	LWSVector4<int32_t> zwxy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 0, 3, 2));
	}

	LWSVector4<int32_t> zwxz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 0, 3, 2));
	}

	LWSVector4<int32_t> zwxw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 3, 2));
	}

	LWSVector4<int32_t> zwyx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 1, 3, 2));
	}

	LWSVector4<int32_t> zwyy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 1, 3, 2));
	}

	LWSVector4<int32_t> zwyz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 1, 3, 2));
	}

	LWSVector4<int32_t> zwyw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 3, 2));
	}

	LWSVector4<int32_t> zwzx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 2, 3, 2));
	}

	LWSVector4<int32_t> zwzy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 2, 3, 2));
	}

	LWSVector4<int32_t> zwzz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 2, 3, 2));
	}

	LWSVector4<int32_t> zwzw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 3, 2));
	}

	LWSVector4<int32_t> zwwx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 3, 3, 2));
	}

	LWSVector4<int32_t> zwwy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 3, 3, 2));
	}

	LWSVector4<int32_t> zwwz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 3, 3, 2));
	}

	LWSVector4<int32_t> zwww(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 3, 3, 2));
	}

	LWSVector4<int32_t> wxxx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 0, 0, 3));
	}

	LWSVector4<int32_t> wxxy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 0, 0, 3));
	}

	LWSVector4<int32_t> wxxz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 0, 0, 3));
	}

	LWSVector4<int32_t> wxxw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 0, 3));
	}

	LWSVector4<int32_t> wxyx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 1, 0, 3));
	}

	LWSVector4<int32_t> wxyy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 1, 0, 3));
	}

	LWSVector4<int32_t> wxyz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 1, 0, 3));
	}

	LWSVector4<int32_t> wxyw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 0, 3));
	}

	LWSVector4<int32_t> wxzx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 2, 0, 3));
	}

	LWSVector4<int32_t> wxzy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 2, 0, 3));
	}

	LWSVector4<int32_t> wxzz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 2, 0, 3));
	}

	LWSVector4<int32_t> wxzw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 0, 3));
	}

	LWSVector4<int32_t> wxwx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 3, 0, 3));
	}

	LWSVector4<int32_t> wxwy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 3, 0, 3));
	}

	LWSVector4<int32_t> wxwz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 3, 0, 3));
	}

	LWSVector4<int32_t> wxww(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 3, 0, 3));
	}

	LWSVector4<int32_t> wyxx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 0, 1, 3));
	}

	LWSVector4<int32_t> wyxy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 0, 1, 3));
	}

	LWSVector4<int32_t> wyxz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 0, 1, 3));
	}

	LWSVector4<int32_t> wyxw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 1, 3));
	}

	LWSVector4<int32_t> wyyx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 1, 1, 3));
	}

	LWSVector4<int32_t> wyyy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 1, 1, 3));
	}

	LWSVector4<int32_t> wyyz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 1, 1, 3));
	}

	LWSVector4<int32_t> wyyw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 1, 3));
	}

	LWSVector4<int32_t> wyzx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 2, 1, 3));
	}

	LWSVector4<int32_t> wyzy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 2, 1, 3));
	}

	LWSVector4<int32_t> wyzz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 2, 1, 3));
	}

	LWSVector4<int32_t> wyzw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 1, 3));
	}

	LWSVector4<int32_t> wywx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 3, 1, 3));
	}

	LWSVector4<int32_t> wywy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 3, 1, 3));
	}

	LWSVector4<int32_t> wywz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 3, 1, 3));
	}

	LWSVector4<int32_t> wyww(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 3, 1, 3));
	}

	LWSVector4<int32_t> wzxx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 0, 2, 3));
	}

	LWSVector4<int32_t> wzxy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 0, 2, 3));
	}

	LWSVector4<int32_t> wzxz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 0, 2, 3));
	}

	LWSVector4<int32_t> wzxw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 2, 3));
	}

	LWSVector4<int32_t> wzyx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 1, 2, 3));
	}

	LWSVector4<int32_t> wzyy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 1, 2, 3));
	}

	LWSVector4<int32_t> wzyz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 1, 2, 3));
	}

	LWSVector4<int32_t> wzyw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 2, 3));
	}

	LWSVector4<int32_t> wzzx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 2, 2, 3));
	}

	LWSVector4<int32_t> wzzy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 2, 2, 3));
	}

	LWSVector4<int32_t> wzzz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 2, 2, 3));
	}

	LWSVector4<int32_t> wzzw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 2, 3));
	}

	LWSVector4<int32_t> wzwx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 3, 2, 3));
	}

	LWSVector4<int32_t> wzwy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 3, 2, 3));
	}

	LWSVector4<int32_t> wzwz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 3, 2, 3));
	}

	LWSVector4<int32_t> wzww(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 3, 2, 3));
	}

	LWSVector4<int32_t> wwxx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 0, 3, 3));
	}

	LWSVector4<int32_t> wwxy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 0, 3, 3));
	}

	LWSVector4<int32_t> wwxz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 0, 3, 3));
	}

	LWSVector4<int32_t> wwxw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 3, 3));
	}

	LWSVector4<int32_t> wwyx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 1, 3, 3));
	}

	LWSVector4<int32_t> wwyy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 1, 3, 3));
	}

	LWSVector4<int32_t> wwyz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 1, 3, 3));
	}

	LWSVector4<int32_t> wwyw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 3, 3));
	}

	LWSVector4<int32_t> wwzx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 2, 3, 3));
	}

	LWSVector4<int32_t> wwzy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 2, 3, 3));
	}

	LWSVector4<int32_t> wwzz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 2, 3, 3));
	}

	LWSVector4<int32_t> wwzw(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 3, 3));
	}

	LWSVector4<int32_t> wwwx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(0, 3, 3, 3));
	}

	LWSVector4<int32_t> wwwy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(1, 3, 3, 3));
	}

	LWSVector4<int32_t> wwwz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(2, 3, 3, 3));
	}

	LWSVector4<int32_t> wwww(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 3, 3, 3));
	}

	LWSVector4<int32_t> xxx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 0, 0));
	}

	LWSVector4<int32_t> xxy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 0, 0));
	}

	LWSVector4<int32_t> xxz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 0, 0));
	}

	LWSVector4<int32_t> xyx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 1, 0));
	}

	LWSVector4<int32_t> xyy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 1, 0));
	}

	LWSVector4<int32_t> xyz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 1, 0));
	}

	LWSVector4<int32_t> xzx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 2, 0));
	}

	LWSVector4<int32_t> xzy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 2, 0));
	}

	LWSVector4<int32_t> xzz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 2, 0));
	}

	LWSVector4<int32_t> yxx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 0, 1));
	}

	LWSVector4<int32_t> yxy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 0, 1));
	}

	LWSVector4<int32_t> yxz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 0, 1));
	}

	LWSVector4<int32_t> yyx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 1, 1));
	}

	LWSVector4<int32_t> yyy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 1, 1));
	}

	LWSVector4<int32_t> yyz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 1, 1));
	}

	LWSVector4<int32_t> yzx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 2, 1));
	}

	LWSVector4<int32_t> yzy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 2, 1));
	}

	LWSVector4<int32_t> yzz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 2, 1));
	}

	LWSVector4<int32_t> zxx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 0, 2));
	}

	LWSVector4<int32_t> zxy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 0, 2));
	}

	LWSVector4<int32_t> zxz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 0, 2));
	}

	LWSVector4<int32_t> zyx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 1, 2));
	}

	LWSVector4<int32_t> zyy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 1, 2));
	}

	LWSVector4<int32_t> zyz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 1, 2));
	}

	LWSVector4<int32_t> zzx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 0, 2, 2));
	}

	LWSVector4<int32_t> zzy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 1, 2, 2));
	}

	LWSVector4<int32_t> zzz(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 2, 2));
	}

	LWSVector4<int32_t> xx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 0, 0));
	}

	LWSVector4<int32_t> xy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 1, 0));
	}

	LWSVector4<int32_t> yx(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 0, 1));
	}

	LWSVector4<int32_t> yy(void) const {
		return _mm_shuffle_epi32(m_Data, _MM_SHUFFLE(3, 2, 1, 1));
	}

	int32_t x(void) const {
		alignas(16) LWVector4<int32_t> R;
		_mm_store_si128((__m128i*)&R.x, m_Data);
		return R.x;
	}

	int32_t y(void) const {
		alignas(16) LWVector4<int32_t> R;
		_mm_store_si128((__m128i*)&R.x, m_Data);
		return R.y;
	}

	int32_t z(void) const {
		alignas(16) LWVector4<int32_t> R;
		_mm_store_si128((__m128i*)&R.x, m_Data);
		return R.z;
	}

	int32_t w(void) const {
		alignas(16) LWVector4<int32_t> R;
		_mm_store_si128((__m128i*)&R.x, m_Data);
		return R.w;
	}

	LWSVector4(__m128i Data) : m_Data(Data) {}

	LWSVector4(const LWVector4<int32_t> &vxyzw) : m_Data(_mm_set_epi32(vxyzw.w, vxyzw.z, vxyzw.y, vxyzw.x)) {}

	LWSVector4(const LWVector3<int32_t> &vxyz, int32_t vw) : m_Data(_mm_set_epi32(vw, vxyz.z, vxyz.y, vxyz.x)) {}

	LWSVector4(const LWVector2<int32_t> &vxy, const LWVector2<int32_t> &vzw) : m_Data(_mm_set_epi32(vzw.y, vzw.x, vxy.y, vxy.x)) {}

	LWSVector4(const LWVector2<int32_t> &vxy, int32_t vz, int32_t vw) : m_Data(_mm_set_epi32(vw, vz, vxy.y, vxy.x)) {}

	LWSVector4(int32_t vx, int32_t vy, int32_t vz, int32_t vw) : m_Data(_mm_set_epi32(vw, vz, vy, vx)) {}

	LWSVector4(int32_t f) : m_Data(_mm_set1_epi32(f)) {}
};

>>>>>>> Added initial inroads for vulkan support(this is far from complete and not usable yet).
#endif