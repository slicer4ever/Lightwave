<<<<<<< HEAD
#ifndef LWSVECTOR4_AVX_FLOAT_H
#define LWSVECTOR4_AVX_FLOAT_H
#include "LWSVector.h"
#include <smmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

template<>
struct LWSVector4<float> {
	__m128 m_Data;

	LWVector4<float> AsVec4(void) const;

	float *AsArray(void);

	const float *AsArray(void) const;

	LWSVector4<float> &sX(float Value);
	
	LWSVector4<float> &sY(float Value);
	
	LWSVector4<float> &sZ(float Value);

	LWSVector4<float> &sW(float Value);

	LWSVector4<float> Normalize(void) const;

	LWSVector4<float> Normalize3(void) const;

	LWSVector4<float> Normalize2(void) const;

	float Dot(const LWSVector4<float>& O) const;

	float Dot3(const LWSVector4<float>& O) const;

	float Dot2(const LWSVector4<float>& O) const;

	LWSVector4<float> Sum(void) const;

	float Sum4(void) const;

	float Sum3(void) const;

	float Sum2(void) const;

	float Min(void) const;

	float Min3(void) const;

	float Min2(void) const;

	float Max(void) const;

	float Max3(void) const;

	float Max2(void) const;

	LWSVector4<float> Min(const LWSVector4<float>& A) const;

	LWSVector4<float> Max(const LWSVector4<float>& A) const;

	LWSVector4<float> Cross3(const LWSVector4<float>& O) const;

	void Orthogonal3(LWSVector4<float> &Right, LWSVector4<float> &Up) const;

	LWSVector4<float> Perpindicular2(void) const;

	float Length(void) const;

	float Length3(void) const;

	float Length2(void) const;

	float LengthSquared(void) const;

	float LengthSquared3(void) const;

	float LengthSquared2(void) const;

	float Distance(const LWSVector4<float>& O) const;

	float Distance3(const LWSVector4<float>& O) const;

	float Distance2(const LWSVector4<float>& O) const;

	float DistanceSquared(const LWSVector4<float>& O) const;

	float DistanceSquared3(const LWSVector4<float>& O) const;

	float DistanceSquared2(const LWSVector4<float>& O) const;

	/*! \brief returns the absolute value of each component. */
	LWSVector4<float> Abs(void) const;

	/*! \brief returns the absolute value of x,y, and z component. */
	LWSVector4<float> Abs3(void) const;

	/*! \brief returns the absolute value of x, and y component. */
	LWSVector4<float> Abs2(void) const;

	/*! \brief compares each component, if component is < rhs, then stores Value's component, otherwise keeps current component. */
	LWSVector4<float> Blend_Less(const LWSVector4<float> &Rhs, const LWSVector4<float> &Value) const;

	/*! \brief compares x, y, and z component, if component is < rhs than store's value's component, otherwise keeps current component. */
	LWSVector4<float> Blend_Less3(const LWSVector4<float> &Rhs, const LWSVector4<float> &Value) const;

	/*! \brief compares x, and y component, if component is < rhs than store's value's component, otherwise keeps current component. */
	LWSVector4<float> Blend_Less2(const LWSVector4<float> &Rhs, const LWSVector4<float> &Value) const;

	/*! \brief compares each component, if component is <= rhs, than stores Value's component, otherwise keeps current component. */
	LWSVector4<float> Blend_LessEqual(const LWSVector4<float> &Rhs, const LWSVector4<float> &Value) const;

	/*! \brief compares x, y, and z component, if component is <= rhs, than stores Value's component, otherwise keeps current component. */
	LWSVector4<float> Blend_LessEqual3(const LWSVector4<float> &Rhs, const LWSVector4<float> &Value) const;

	/*! \brief compares x, and y component, if component is <= rhs, than stores Value's component, otherwise keeps current component. */
	LWSVector4<float> Blend_LessEqual2(const LWSVector4<float> &Rhs, const LWSVector4<float> &Value) const;

	/*! \brief compares each component, if component is > rhs than stores Value's component, otherwise keeps current component. */
	LWSVector4<float> Blend_Greater(const LWSVector4<float> &Rhs, const LWSVector4<float> &Value) const;

	/*! \brief compares x, y, and z component, if component is > rhs than stores Value's component, otherwise keeps current component. */
	LWSVector4<float> Blend_Greater3(const LWSVector4<float> &Rhs, const LWSVector4<float> &Value) const;

	/*! \brief compares x, and y component, if component is > rhs than stores Value's component, otherwise keeps current component. */
	LWSVector4<float> Blend_Greater2(const LWSVector4<float> &Rhs, const LWSVector4<float> &Value) const;

	/*! \brief compares each component if component is >= rhs than stores value's component, otherwise keeps current component. */
	LWSVector4<float> Blend_GreaterEqual(const LWSVector4<float> &Rhs, const LWSVector4<float> &Value) const;

	/*! \brief compares x, y, and z component if component is >= rhs than stores value's component, otherwise keeps current component. */
	LWSVector4<float> Blend_GreaterEqual3(const LWSVector4<float> &Rhs, const LWSVector4<float> &Value) const;

	/*! \brief compares x, and y component if component is >= rhs than stores value's component, otherwise keeps current component. */
	LWSVector4<float> Blend_GreaterEqual2(const LWSVector4<float> &Rhs, const LWSVector4<float> &Value) const;

	/*! \brief compares each component, if component is == rhs(use's float epsilon for comparison) than stores value's component, otherwise keeps current component. */
	LWSVector4<float> Blend_Equal(const LWSVector4<float> &Rhs, const LWSVector4<float> &Value) const;

	/*! \brief compares x, y, and z component, if component is == rhs(use's float epsilon for comparison) than stores value's component, otherwise keeps current component. */
	LWSVector4<float> Blend_Equal3(const LWSVector4<float> &Rhs, const LWSVector4<float> &Value) const;

	/*! \brief compares x, and y component, if component is == rhs(use's float epsilon for comparison) than stores value's component, otherwise keeps current component. */
	LWSVector4<float> Blend_Equal2(const LWSVector4<float> &Rhs, const LWSVector4<float> &Value) const;

	/*! \brief compares each component, if component is != rhs(use's float epsilon for comparison) than stores value's component, otherwise keeps current component. */
	LWSVector4<float> Blend_NotEqual(const LWSVector4<float> &Rhs, const LWSVector4<float> &Value) const;

	/*! \brief compares x, y, and z component, if component is != rhs(use's float epsilon for comparison) than stores value's component, otherwise keeps current component. */
	LWSVector4<float> Blend_NotEqual3(const LWSVector4<float> &Rhs, const LWSVector4<float> &Value) const;

	/*! \brief compares x, and y component, if component is != rhs(use's float epsilon for comparison) than stores value's component, otherwise keeps current component. */
	LWSVector4<float> Blend_NotEqual2(const LWSVector4<float> &Rhs, const LWSVector4<float> &Value) const;

	bool Less3(const LWSVector4<float> &Rhs) const;

	bool Less2(const LWSVector4<float> &Rhs) const;

	bool LessEqual3(const LWSVector4<float> &Rhs) const;

	bool LessEqual2(const LWSVector4<float> &Rhs) const;

	bool Greater3(const LWSVector4<float> &Rhs) const;

	bool Greater2(const LWSVector4<float> &Rhs) const;

	bool GreaterEqual3(const LWSVector4<float> &Rhs) const;

	bool GreaterEqual2(const LWSVector4<float> &Rhs) const;

	LWSVector4<float>& operator = (const LWSVector4<float>& Rhs);

	LWSVector4<float>& operator += (const LWSVector4<float>& Rhs);

	LWSVector4<float>& operator += (float Rhs);

	LWSVector4<float>& operator -= (const LWSVector4<float>& Rhs);

	LWSVector4<float>& operator -= (float Rhs);

	LWSVector4<float>& operator *= (const LWSVector4<float>& Rhs);

	LWSVector4<float>& operator *= (float Rhs);

	LWSVector4<float>& operator /= (const LWSVector4<float>& Rhs);

	LWSVector4<float>& operator /= (float Rhs);

	friend LWSVector4<float> operator + (const LWSVector4<float>& Rhs);

	friend LWSVector4<float> operator - (const LWSVector4<float>& Rhs);

	bool operator > (const LWSVector4<float> &Rhs) const;

	bool operator >= (const LWSVector4<float> &Rhs) const;

	bool operator < (const LWSVector4<float> &Rhs) const;

	bool operator <= (const LWSVector4<float> &Rhs) const;

	bool operator == (const LWSVector4<float> &Rhs) const;

	bool operator != (const LWSVector4<float> &Rhs) const;

	friend std::ostream& operator<<(std::ostream& o, const LWSVector4<float>& v);

	friend LWSVector4<float> operator + (const LWSVector4<float>& Lhs, const LWSVector4<float>& Rhs);

	friend LWSVector4<float> operator + (const LWSVector4<float>& Lhs, float Rhs);

	friend LWSVector4<float> operator + (float Lhs, const LWSVector4<float>& Rhs);

	friend LWSVector4<float> operator - (const LWSVector4<float>& Lhs, const LWSVector4<float>& Rhs);

	friend LWSVector4<float> operator - (const LWSVector4<float>& Lhs, float Rhs);

	friend LWSVector4<float> operator - (float Lhs, const LWSVector4<float>& Rhs);

	friend LWSVector4<float> operator * (const LWSVector4<float>& Lhs, const LWSVector4<float>& Rhs);

	friend LWSVector4<float> operator * (const LWSVector4<float>& Lhs, float Rhs);

	friend LWSVector4<float> operator * (float Lhs, const LWSVector4<float>& Rhs);

	friend LWSVector4<float> operator / (const LWSVector4<float>& Lhs, const LWSVector4<float>& Rhs);

	friend LWSVector4<float> operator / (const LWSVector4<float>& Lhs, float Rhs);

	friend LWSVector4<float> operator / (float Lhs, const LWSVector4<float>& Rhs);

	LWSVector4<float> AAAB(const LWSVector4<float>& B) const;

	LWSVector4<float> AABA(const LWSVector4<float>& B) const;

	LWSVector4<float> AABB(const LWSVector4<float>& B) const;

	LWSVector4<float> ABAA(const LWSVector4<float>& B) const;

	LWSVector4<float> ABAB(const LWSVector4<float>& B) const;

	LWSVector4<float> ABBA(const LWSVector4<float>& B) const;

	LWSVector4<float> ABBB(const LWSVector4<float>& B) const;

	LWSVector4<float> BAAA(const LWSVector4<float>& B) const;

	LWSVector4<float> BAAB(const LWSVector4<float>& B) const;

	LWSVector4<float> BABA(const LWSVector4<float>& B) const;

	LWSVector4<float> BABB(const LWSVector4<float>& B) const;

	LWSVector4<float> BBAA(const LWSVector4<float>& B) const;

	LWSVector4<float> BBAB(const LWSVector4<float>& B) const;

	LWSVector4<float> BBBA(const LWSVector4<float>& B) const;

	LWSVector4<float> xxxx(void) const;

	LWSVector4<float> xxxy(void) const;

	LWSVector4<float> xxxz(void) const;

	LWSVector4<float> xxxw(void) const;

	LWSVector4<float> xxyx(void) const;

	LWSVector4<float> xxyy(void) const;

	LWSVector4<float> xxyz(void) const;

	LWSVector4<float> xxyw(void) const;

	LWSVector4<float> xxzx(void) const;

	LWSVector4<float> xxzy(void) const;

	LWSVector4<float> xxzz(void) const;

	LWSVector4<float> xxzw(void) const;

	LWSVector4<float> xxwx(void) const;

	LWSVector4<float> xxwy(void) const;

	LWSVector4<float> xxwz(void) const;

	LWSVector4<float> xxww(void) const;

	LWSVector4<float> xyxx(void) const;

	LWSVector4<float> xyxy(void) const;

	LWSVector4<float> xyxz(void) const;

	LWSVector4<float> xyxw(void) const;

	LWSVector4<float> xyyx(void) const;

	LWSVector4<float> xyyy(void) const;

	LWSVector4<float> xyyz(void) const;

	LWSVector4<float> xyyw(void) const;

	LWSVector4<float> xyzx(void) const;

	LWSVector4<float> xyzy(void) const;

	LWSVector4<float> xyzz(void) const;

	LWSVector4<float> xywx(void) const;

	LWSVector4<float> xywy(void) const;

	LWSVector4<float> xywz(void) const;

	LWSVector4<float> xyww(void) const;

	LWSVector4<float> xzxx(void) const;

	LWSVector4<float> xzxy(void) const;

	LWSVector4<float> xzxz(void) const;

	LWSVector4<float> xzxw(void) const;

	LWSVector4<float> xzyx(void) const;

	LWSVector4<float> xzyy(void) const;

	LWSVector4<float> xzyz(void) const;

	LWSVector4<float> xzyw(void) const;

	LWSVector4<float> xzzx(void) const;

	LWSVector4<float> xzzy(void) const;

	LWSVector4<float> xzzz(void) const;

	LWSVector4<float> xzzw(void) const;

	LWSVector4<float> xzwx(void) const;

	LWSVector4<float> xzwy(void) const;

	LWSVector4<float> xzwz(void) const;

	LWSVector4<float> xzww(void) const;

	LWSVector4<float> xwxx(void) const;

	LWSVector4<float> xwxy(void) const;

	LWSVector4<float> xwxz(void) const;

	LWSVector4<float> xwxw(void) const;

	LWSVector4<float> xwyx(void) const;

	LWSVector4<float> xwyy(void) const;

	LWSVector4<float> xwyz(void) const;

	LWSVector4<float> xwyw(void) const;

	LWSVector4<float> xwzx(void) const;

	LWSVector4<float> xwzy(void) const;

	LWSVector4<float> xwzz(void) const;

	LWSVector4<float> xwzw(void) const;

	LWSVector4<float> xwwx(void) const;

	LWSVector4<float> xwwy(void) const;

	LWSVector4<float> xwwz(void) const;

	LWSVector4<float> xwww(void) const;

	LWSVector4<float> yxxx(void) const;

	LWSVector4<float> yxxy(void) const;

	LWSVector4<float> yxxz(void) const;

	LWSVector4<float> yxxw(void) const;

	LWSVector4<float> yxyx(void) const;

	LWSVector4<float> yxyy(void) const;

	LWSVector4<float> yxyz(void) const;

	LWSVector4<float> yxyw(void) const;

	LWSVector4<float> yxzx(void) const;

	LWSVector4<float> yxzy(void) const;

	LWSVector4<float> yxzz(void) const;

	LWSVector4<float> yxzw(void) const;

	LWSVector4<float> yxwx(void) const;

	LWSVector4<float> yxwy(void) const;

	LWSVector4<float> yxwz(void) const;

	LWSVector4<float> yxww(void) const;

	LWSVector4<float> yyxx(void) const;

	LWSVector4<float> yyxy(void) const;

	LWSVector4<float> yyxz(void) const;

	LWSVector4<float> yyxw(void) const;

	LWSVector4<float> yyyx(void) const;

	LWSVector4<float> yyyy(void) const;

	LWSVector4<float> yyyz(void) const;

	LWSVector4<float> yyyw(void) const;

	LWSVector4<float> yyzx(void) const;

	LWSVector4<float> yyzy(void) const;

	LWSVector4<float> yyzz(void) const;

	LWSVector4<float> yyzw(void) const;

	LWSVector4<float> yywx(void) const;

	LWSVector4<float> yywy(void) const;

	LWSVector4<float> yywz(void) const;

	LWSVector4<float> yyww(void) const;

	LWSVector4<float> yzxx(void) const;

	LWSVector4<float> yzxy(void) const;

	LWSVector4<float> yzxz(void) const;

	LWSVector4<float> yzxw(void) const;

	LWSVector4<float> yzyx(void) const;

	LWSVector4<float> yzyy(void) const;

	LWSVector4<float> yzyz(void) const;

	LWSVector4<float> yzyw(void) const;

	LWSVector4<float> yzzx(void) const;

	LWSVector4<float> yzzy(void) const;

	LWSVector4<float> yzzz(void) const;

	LWSVector4<float> yzzw(void) const;

	LWSVector4<float> yzwx(void) const;

	LWSVector4<float> yzwy(void) const;

	LWSVector4<float> yzwz(void) const;

	LWSVector4<float> yzww(void) const;

	LWSVector4<float> ywxx(void) const;

	LWSVector4<float> ywxy(void) const;

	LWSVector4<float> ywxz(void) const;

	LWSVector4<float> ywxw(void) const;

	LWSVector4<float> ywyx(void) const;

	LWSVector4<float> ywyy(void) const;

	LWSVector4<float> ywyz(void) const;

	LWSVector4<float> ywyw(void) const;

	LWSVector4<float> ywzx(void) const;

	LWSVector4<float> ywzy(void) const;

	LWSVector4<float> ywzz(void) const;

	LWSVector4<float> ywzw(void) const;

	LWSVector4<float> ywwx(void) const;

	LWSVector4<float> ywwy(void) const;

	LWSVector4<float> ywwz(void) const;

	LWSVector4<float> ywww(void) const;

	LWSVector4<float> zxxx(void) const;

	LWSVector4<float> zxxy(void) const;

	LWSVector4<float> zxxz(void) const;

	LWSVector4<float> zxxw(void) const;

	LWSVector4<float> zxyx(void) const;

	LWSVector4<float> zxyy(void) const;

	LWSVector4<float> zxyz(void) const;

	LWSVector4<float> zxyw(void) const;

	LWSVector4<float> zxzx(void) const;

	LWSVector4<float> zxzy(void) const;

	LWSVector4<float> zxzz(void) const;

	LWSVector4<float> zxzw(void) const;

	LWSVector4<float> zxwx(void) const;

	LWSVector4<float> zxwy(void) const;

	LWSVector4<float> zxwz(void) const;

	LWSVector4<float> zxww(void) const;

	LWSVector4<float> zyxx(void) const;

	LWSVector4<float> zyxy(void) const;

	LWSVector4<float> zyxz(void) const;

	LWSVector4<float> zyxw(void) const;

	LWSVector4<float> zyyx(void) const;

	LWSVector4<float> zyyy(void) const;

	LWSVector4<float> zyyz(void) const;

	LWSVector4<float> zyyw(void) const;

	LWSVector4<float> zyzx(void) const;

	LWSVector4<float> zyzy(void) const;

	LWSVector4<float> zyzz(void) const;

	LWSVector4<float> zyzw(void) const;

	LWSVector4<float> zywx(void) const;

	LWSVector4<float> zywy(void) const;

	LWSVector4<float> zywz(void) const;

	LWSVector4<float> zyww(void) const;

	LWSVector4<float> zzxx(void) const;

	LWSVector4<float> zzxy(void) const;

	LWSVector4<float> zzxz(void) const ;

	LWSVector4<float> zzxw(void) const ;

	LWSVector4<float> zzyx(void) const ;

	LWSVector4<float> zzyy(void) const ;

	LWSVector4<float> zzyz(void) const ;

	LWSVector4<float> zzyw(void) const ;

	LWSVector4<float> zzzx(void) const ;

	LWSVector4<float> zzzy(void) const ;

	LWSVector4<float> zzzz(void) const ;

	LWSVector4<float> zzzw(void) const ;

	LWSVector4<float> zzwx(void) const ;

	LWSVector4<float> zzwy(void) const ;

	LWSVector4<float> zzwz(void) const ;

	LWSVector4<float> zzww(void) const ;

	LWSVector4<float> zwxx(void) const ;

	LWSVector4<float> zwxy(void) const ;

	LWSVector4<float> zwxz(void) const ;

	LWSVector4<float> zwxw(void) const ;

	LWSVector4<float> zwyx(void) const ;

	LWSVector4<float> zwyy(void) const ;

	LWSVector4<float> zwyz(void) const ;

	LWSVector4<float> zwyw(void) const ;

	LWSVector4<float> zwzx(void) const;

	LWSVector4<float> zwzy(void) const;

	LWSVector4<float> zwzz(void) const;

	LWSVector4<float> zwzw(void) const;

	LWSVector4<float> zwwx(void) const;

	LWSVector4<float> zwwy(void) const;

	LWSVector4<float> zwwz(void) const;

	LWSVector4<float> zwww(void) const;

	LWSVector4<float> wxxx(void) const;

	LWSVector4<float> wxxy(void) const;

	LWSVector4<float> wxxz(void) const;

	LWSVector4<float> wxxw(void) const;

	LWSVector4<float> wxyx(void) const;

	LWSVector4<float> wxyy(void) const;

	LWSVector4<float> wxyz(void) const;

	LWSVector4<float> wxyw(void) const;

	LWSVector4<float> wxzx(void) const;

	LWSVector4<float> wxzy(void) const;

	LWSVector4<float> wxzz(void) const;

	LWSVector4<float> wxzw(void) const;

	LWSVector4<float> wxwx(void) const;

	LWSVector4<float> wxwy(void) const;

	LWSVector4<float> wxwz(void) const;

	LWSVector4<float> wxww(void) const;

	LWSVector4<float> wyxx(void) const;

	LWSVector4<float> wyxy(void) const;

	LWSVector4<float> wyxz(void) const;

	LWSVector4<float> wyxw(void) const;

	LWSVector4<float> wyyx(void) const;

	LWSVector4<float> wyyy(void) const;

	LWSVector4<float> wyyz(void) const;

	LWSVector4<float> wyyw(void) const;

	LWSVector4<float> wyzx(void) const;

	LWSVector4<float> wyzy(void) const;

	LWSVector4<float> wyzz(void) const;

	LWSVector4<float> wyzw(void) const;

	LWSVector4<float> wywx(void) const;

	LWSVector4<float> wywy(void) const;

	LWSVector4<float> wywz(void) const;

	LWSVector4<float> wyww(void) const;

	LWSVector4<float> wzxx(void) const;

	LWSVector4<float> wzxy(void) const;

	LWSVector4<float> wzxz(void) const;

	LWSVector4<float> wzxw(void) const;

	LWSVector4<float> wzyx(void) const;

	LWSVector4<float> wzyy(void) const;

	LWSVector4<float> wzyz(void) const;

	LWSVector4<float> wzyw(void) const;

	LWSVector4<float> wzzx(void) const;

	LWSVector4<float> wzzy(void) const;

	LWSVector4<float> wzzz(void) const;

	LWSVector4<float> wzzw(void) const;

	LWSVector4<float> wzwx(void) const;

	LWSVector4<float> wzwy(void) const;

	LWSVector4<float> wzwz(void) const;

	LWSVector4<float> wzww(void) const;

	LWSVector4<float> wwxx(void) const;

	LWSVector4<float> wwxy(void) const;

	LWSVector4<float> wwxz(void) const;

	LWSVector4<float> wwxw(void) const;

	LWSVector4<float> wwyx(void) const;

	LWSVector4<float> wwyy(void) const;

	LWSVector4<float> wwyz(void) const;

	LWSVector4<float> wwyw(void) const;

	LWSVector4<float> wwzx(void) const;

	LWSVector4<float> wwzy(void) const;

	LWSVector4<float> wwzz(void) const;

	LWSVector4<float> wwzw(void) const;

	LWSVector4<float> wwwx(void) const;

	LWSVector4<float> wwwy(void) const;

	LWSVector4<float> wwwz(void) const;

	LWSVector4<float> wwww(void) const;

	LWSVector4<float> xxx(void) const;

	LWSVector4<float> xxy(void) const;

	LWSVector4<float> xxz(void) const;

	LWSVector4<float> xyx(void) const;

	LWSVector4<float> xyy(void) const;

	LWSVector4<float> xyz(void) const;

	LWSVector4<float> xzx(void) const;

	LWSVector4<float> xzy(void) const;

	LWSVector4<float> xzz(void) const;

	LWSVector4<float> yxx(void) const;

	LWSVector4<float> yxy(void) const;

	LWSVector4<float> yxz(void) const;

	LWSVector4<float> yyx(void) const;

	LWSVector4<float> yyy(void) const;

	LWSVector4<float> yyz(void) const;

	LWSVector4<float> yzx(void) const;

	LWSVector4<float> yzy(void) const;

	LWSVector4<float> yzz(void) const;

	LWSVector4<float> zxx(void) const;

	LWSVector4<float> zxy(void) const;

	LWSVector4<float> zxz(void) const;

	LWSVector4<float> zyx(void) const;

	LWSVector4<float> zyy(void) const;

	LWSVector4<float> zyz(void) const;

	LWSVector4<float> zzx(void) const;

	LWSVector4<float> zzy(void) const;

	LWSVector4<float> zzz(void) const;

	LWSVector4<float> xx(void) const;

	LWSVector4<float> xy(void) const;

	LWSVector4<float> yx(void) const;

	LWSVector4<float> yy(void) const;

	float x(void) const;

	float y(void) const;

	float z(void) const;

	float w(void) const;

	LWSVector4(__m128 Data);

	LWSVector4(const LWVector4<float>& vxyzw);

	LWSVector4(const LWVector3<float>& vxyz, float vw);

	LWSVector4(const LWVector2<float>& vxy, const LWVector2<float>& vzw);

	LWSVector4(const LWVector2<float>& vxy, float vz, float vw);

	LWSVector4(float vx, float vy, float vz, float vw);

	LWSVector4(float f = 0.0f);
};

=======
#ifndef LWSVECTOR4_AVX_FLOAT_H
#define LWSVECTOR4_AVX_FLOAT_H
#include "LWSVector.h"
#include <smmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

template<>
struct LWSVector4<float> {
	__m128 m_Data;

	LWVector4<float> AsVec4(void) const {
		alignas(16) LWVector4<float> R;
		_mm_store_ps(&R.x, m_Data);
		return R;
	}

	LWSVector4<float> Normalize(void) const {
		const float e = std::numeric_limits<float>::epsilon();
		__m128 eps = _mm_set_ps1(e);
		__m128 t = _mm_dp_ps(m_Data, m_Data, 0xFF);
		if (_mm_test_all_ones(_mm_castps_si128(_mm_cmple_ps(t, eps)))) return _mm_set_ps1(0.0f);
		return _mm_div_ps(m_Data, _mm_sqrt_ps(t));
	}

	LWSVector4<float> Normalize3(void) const {
		const float e = std::numeric_limits<float>::epsilon();
		__m128 eps = _mm_set_ps1(e);
		__m128 t = _mm_dp_ps(m_Data, m_Data, 0x7F);
		if (_mm_test_all_ones(_mm_castps_si128(_mm_cmple_ps(t, eps)))) return _mm_set_ps1(0.0f);
		return _mm_div_ps(m_Data, _mm_sqrt_ps(t));
	}

	LWSVector4<float> Normalize2(void) const {
		const float e = std::numeric_limits<float>::epsilon();
		__m128 eps = _mm_set_ps1(e);
		__m128 t = _mm_dp_ps(m_Data, m_Data, 0x3f);
		if (_mm_test_all_ones(_mm_castps_si128(_mm_cmple_ps(t, eps)))) return _mm_set_ps1(0.0f);
		return _mm_div_ps(m_Data, _mm_sqrt_ps(t));
	}

	float Dot(const LWSVector4<float> &O) const {
		return _mm_cvtss_f32(_mm_dp_ps(m_Data, O.m_Data, 0xFF));
	}

	float Dot3(const LWSVector4<float> &O) const {
		__m128 t = _mm_dp_ps(m_Data, O.m_Data, 0x7F);
		return _mm_cvtss_f32(_mm_dp_ps(m_Data, O.m_Data, 0x7F));
	}

	float Dot2(const LWSVector4<float> &O) const {
		return _mm_cvtss_f32(_mm_dp_ps(m_Data, O.m_Data, 0x3F));
	}

	float Min(void) const {
		__m128 A = _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 1, 0, 3));
		__m128 B = _mm_min_ps(m_Data, A);
		A = _mm_permute_ps(B, _MM_SHUFFLE(1, 0, 3, 2));
		return _mm_cvtss_f32(_mm_min_ps(B, A));
	}

	float Min3(void) const {
		__m128 A = _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 0, 2));
		__m128 B = _mm_min_ps(m_Data, A);
		A = _mm_permute_ps(B, _MM_SHUFFLE(3, 0, 2, 1));
		return _mm_cvtss_f32(_mm_min_ps(B, A));
	}

	float Min2(void) const {
		__m128 A = _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 0, 1));
		return _mm_cvtss_f32(_mm_min_ps(m_Data, A));
	}

	float Max(void) const {
		__m128 A = _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 1, 0, 3));
		__m128 B = _mm_max_ps(m_Data, A);
		A = _mm_permute_ps(B, _MM_SHUFFLE(1, 0, 3, 2));
		return _mm_cvtss_f32(_mm_max_ps(B, A));
	}

	float Max3(void) const {
		__m128 A = _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 0, 2));
		__m128 B = _mm_max_ps(m_Data, A);
		A = _mm_permute_ps(B, _MM_SHUFFLE(3, 0, 2, 1));
		return _mm_cvtss_f32(_mm_max_ps(B, A));
	}

	float Max2(void) const {
		__m128 A = _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 0, 1));
		return _mm_cvtss_f32(_mm_max_ps(m_Data, A));
	}

	LWSVector4<float> Min(const LWSVector4<float> &A) const {
		return _mm_min_ps(m_Data, A.m_Data);
	}

	LWSVector4<float> Max(const LWSVector4<float> &A) const {
		return _mm_max_ps(m_Data, A.m_Data);
	}

	LWSVector4<float> Cross3(const LWSVector4<float> &O) const {
		__m128 A = yzxw().m_Data;
		__m128 B = O.zxyw().m_Data;
		__m128 C = zxyw().m_Data;
		__m128 D = O.yzxw().m_Data;
		return _mm_sub_ps(_mm_mul_ps(A, B), _mm_mul_ps(C, D));
	}

	LWSVector4<float> Perpindicular2(void) const {
		return _mm_mul_ps(yx().m_Data, _mm_set_ps(-1.0, 1.0, 1.0, 1.0));
	}

	float Length(void) const {
		__m128 t = _mm_dp_ps(m_Data, m_Data, 0xFF);
		return _mm_cvtss_f32(_mm_sqrt_ps(t));
	}

	float Length3(void) const {
		__m128 t = _mm_dp_ps(m_Data, m_Data, 0x7F);
		return _mm_cvtss_f32(_mm_sqrt_ps(t));
	}

	float Length2(void) const {
		__m128 t = _mm_dp_ps(m_Data, m_Data, 0x3F);
		return _mm_cvtss_f32(_mm_sqrt_ps(t));
	}

	float LengthSquared(void) const {
		return _mm_cvtss_f32(_mm_dp_ps(m_Data, m_Data, 0xFF));
	}

	float LengthSquared3(void) const {
		return _mm_cvtss_f32(_mm_dp_ps(m_Data, m_Data, 0x7F));
	}

	float LengthSquared2(void) const {
		return _mm_cvtss_f32(_mm_dp_ps(m_Data, m_Data, 0x3F));
	}

	float Distance(const LWSVector4<float> &O) const {
		__m128 t = _mm_sub_ps(m_Data, O.m_Data);
		t = _mm_dp_ps(t, t, 0xFF);
		return _mm_cvtss_f32(_mm_sqrt_ps(t));
	}

	float Distance3(const LWSVector4<float> &O) const {
		__m128 t = _mm_sub_ps(m_Data, O.m_Data);
		t = _mm_dp_ps(t, t, 0x7F);
		return _mm_cvtss_f32(_mm_sqrt_ps(t));
	}

	float Distance2(const LWSVector4<float> &O) const {
		__m128 t = _mm_sub_ps(m_Data, O.m_Data);
		t = _mm_dp_ps(t, t, 0x3f);
		return _mm_cvtss_f32(_mm_sqrt_ps(t));
	}

	float DistanceSquared(const LWSVector4<float> &O) const {
		__m128 t = _mm_sub_ps(m_Data, O.m_Data);
		return _mm_cvtss_f32(_mm_dp_ps(t, t, 0xFF));
	}

	float DistanceSquared3(const LWSVector4<float> &O) const {
		__m128 t = _mm_sub_ps(m_Data, O.m_Data);
		return _mm_cvtss_f32(_mm_dp_ps(t, t, 0x7F));
	}

	float DistanceSquared2(const LWSVector4<float> &O) const {
		__m128 t = _mm_sub_ps(m_Data, O.m_Data);
		return _mm_cvtss_f32(_mm_dp_ps(t, t, 0x3F));
	}

	LWSVector4<float> &operator = (const LWSVector4<float> &Rhs) {
		m_Data = Rhs.m_Data;
		return *this;
	}

	LWSVector4<float> &operator += (const LWSVector4<float> &Rhs) {
		m_Data = _mm_add_ps(m_Data, Rhs.m_Data);
		return *this;
	}

	LWSVector4<float> &operator += (float Rhs) {
		__m128 t = _mm_set_ps1(Rhs);
		m_Data = _mm_add_ps(m_Data, t);
		return *this;
	}

	LWSVector4<float> &operator -= (const LWSVector4<float> &Rhs) {
		m_Data = _mm_sub_ps(m_Data, Rhs.m_Data);
		return *this;
	}

	LWSVector4<float> &operator -= (float Rhs) {
		__m128 t = _mm_set_ps1(Rhs);
		m_Data = _mm_sub_ps(m_Data, t);
		return *this;
	}

	LWSVector4<float> &operator *= (const LWSVector4<float> &Rhs) {
		m_Data = _mm_mul_ps(m_Data, Rhs.m_Data);
		return *this;
	}

	LWSVector4<float> &operator *= (float Rhs) {
		__m128 t = _mm_set_ps1(Rhs);
		m_Data = _mm_mul_ps(m_Data, t);
		return *this;
	}

	LWSVector4<float> &operator /= (const LWSVector4<float> &Rhs) {
		m_Data = _mm_div_ps(m_Data, Rhs.m_Data);
		return *this;
	}

	LWSVector4<float> &operator /= (float Rhs) {
		__m128 t = _mm_set_ps1(Rhs);
		m_Data = _mm_div_ps(m_Data, t);
		return *this;
	}

	friend LWSVector4<float> operator + (const LWSVector4<float> &Rhs) {
		return Rhs.m_Data;
	}

	friend LWSVector4<float> operator - (const LWSVector4<float> &Rhs) {
		__m128 t = _mm_set_ps1(-1.0f);
		return _mm_mul_ps(Rhs.m_Data, t);
	}

	bool operator == (const LWSVector4<float> &Rhs) const {
		__m128i t = _mm_castps_si128(_mm_cmpeq_ps(m_Data, Rhs.m_Data));
		return _mm_test_all_ones(t);
	}

	bool operator != (const LWSVector4<float> &Rhs) const {
		return !(*this == Rhs);
	}

	friend std::ostream &operator<<(std::ostream &o, const LWSVector4<float> &v) {
		alignas(16) float Values[4];
		_mm_store_ps(Values, v.m_Data);
		o << Values[0] << " " << Values[1] << " " << Values[2] << " " << Values[3];
		return o;
	}

	friend LWSVector4<float> operator + (const LWSVector4<float> &Lhs, const LWSVector4<float> &Rhs) {
		return _mm_add_ps(Lhs.m_Data, Rhs.m_Data);
	}

	friend LWSVector4<float> operator + (const LWSVector4<float> &Lhs, float Rhs) {
		__m128 t = _mm_set_ps1(Rhs);
		return _mm_add_ps(Lhs.m_Data, t);
	}

	friend LWSVector4<float> operator + (float Lhs, const LWSVector4<float> &Rhs) {
		__m128 t = _mm_set_ps1(Lhs);
		return _mm_add_ps(t, Rhs.m_Data);
	}

	friend LWSVector4<float> operator - (const LWSVector4<float> &Lhs, const LWSVector4<float> &Rhs) {
		return _mm_sub_ps(Lhs.m_Data, Rhs.m_Data);
	}

	friend LWSVector4<float> operator - (const LWSVector4<float> &Lhs, float Rhs) {
		__m128 t = _mm_set_ps1(Rhs);
		return _mm_sub_ps(Lhs.m_Data, t);
	}

	friend LWSVector4<float> operator - (float Lhs, const LWSVector4<float> &Rhs) {
		__m128 t = _mm_set_ps1(Lhs);
		return _mm_sub_ps(t, Rhs.m_Data);
	}

	friend LWSVector4<float> operator * (const LWSVector4<float> &Lhs, const LWSVector4<float> &Rhs) {
		return _mm_mul_ps(Lhs.m_Data, Rhs.m_Data);
	}

	friend LWSVector4<float> operator * (const LWSVector4<float> &Lhs, float Rhs) {
		__m128 t = _mm_set_ps1(Rhs);
		return _mm_mul_ps(Lhs.m_Data, t);
	}

	friend LWSVector4<float> operator * (float Lhs, const LWSVector4<float> &Rhs) {
		__m128 t = _mm_set_ps1(Lhs);
		return _mm_mul_ps(t, Rhs.m_Data);
	}

	friend LWSVector4<float> operator / (const LWSVector4<float> &Lhs, const LWSVector4<float> &Rhs) {
		return _mm_div_ps(Lhs.m_Data, Rhs.m_Data);
	}

	friend LWSVector4<float> operator / (const LWSVector4<float> &Lhs, float Rhs) {
		__m128 t = _mm_set_ps1(Rhs);
		return _mm_div_ps(Lhs.m_Data, t);
	}

	friend LWSVector4<float> operator / (float Lhs, const LWSVector4<float> &Rhs) {
		__m128 t = _mm_set_ps1(Lhs);
		return _mm_div_ps(t, Rhs.m_Data);
	}

	LWSVector4<float> xxxx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 0, 0, 0));
	}

	LWSVector4<float> xxxy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 0, 0, 0));
	}

	LWSVector4<float> xxxz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 0, 0, 0));
	}

	LWSVector4<float> xxxw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 0, 0));
	}

	LWSVector4<float> xxyx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 1, 0, 0));
	}

	LWSVector4<float> xxyy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 1, 0, 0));
	}

	LWSVector4<float> xxyz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 1, 0, 0));
	}
	LWSVector4<float> xxyw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 0, 0));
	}

	LWSVector4<float> xxzx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 2, 0, 0));
	}

	LWSVector4<float> xxzy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 2, 0, 0));
	}

	LWSVector4<float> xxzz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 2, 0, 0));
	}

	LWSVector4<float> xxzw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 0, 0));
	}

	LWSVector4<float> xxwx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 3, 0, 0));
	}

	LWSVector4<float> xxwy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 3, 0, 0));
	}

	LWSVector4<float> xxwz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 3, 0, 0));
	}

	LWSVector4<float> xxww(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 3, 0, 0));
	}

	LWSVector4<float> xyxx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 0, 1, 0));
	}

	LWSVector4<float> xyxy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 0, 1, 0));
	}

	LWSVector4<float> xyxz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 0, 1, 0));
	}

	LWSVector4<float> xyxw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 1, 0));
	}

	LWSVector4<float> xyyx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 1, 1, 0));
	}

	LWSVector4<float> xyyy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 1, 1, 0));
	}

	LWSVector4<float> xyyz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 1, 1, 0));
	}

	LWSVector4<float> xyyw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 1, 0));
	}

	LWSVector4<float> xyzx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 2, 1, 0));
	}

	LWSVector4<float> xyzy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 2, 1, 0));
	}

	LWSVector4<float> xyzz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 2, 1, 0));
	}

	LWSVector4<float> xywx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 3, 1, 0));
	}

	LWSVector4<float> xywy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 3, 1, 0));
	}

	LWSVector4<float> xywz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 3, 1, 0));
	}

	LWSVector4<float> xyww(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 3, 1, 0));
	}

	LWSVector4<float> xzxx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 0, 2, 0));
	}

	LWSVector4<float> xzxy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 0, 2, 0));
	}

	LWSVector4<float> xzxz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 0, 2, 0));
	}

	LWSVector4<float> xzxw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 2, 0));
	}

	LWSVector4<float> xzyx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 1, 2, 0));
	}

	LWSVector4<float> xzyy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 1, 2, 0));
	}

	LWSVector4<float> xzyz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 1, 2, 0));
	}

	LWSVector4<float> xzyw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 2, 0));
	}

	LWSVector4<float> xzzx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 2, 2, 0));
	}

	LWSVector4<float> xzzy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 2, 2, 0));
	}

	LWSVector4<float> xzzz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 2, 2, 0));
	}

	LWSVector4<float> xzzw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 2, 0));
	}

	LWSVector4<float> xzwx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 3, 2, 0));
	}

	LWSVector4<float> xzwy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 3, 2, 0));
	}

	LWSVector4<float> xzwz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 3, 2, 0));
	}

	LWSVector4<float> xzww(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 3, 2, 0));
	}

	LWSVector4<float> xwxx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 0, 3, 0));
	}

	LWSVector4<float> xwxy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 0, 3, 0));
	}

	LWSVector4<float> xwxz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 0, 3, 0));
	}

	LWSVector4<float> xwxw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 3, 0));
	}

	LWSVector4<float> xwyx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 1, 3, 0));
	}

	LWSVector4<float> xwyy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 1, 3, 0));
	}

	LWSVector4<float> xwyz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 1, 3, 0));
	}

	LWSVector4<float> xwyw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 3, 0));
	}

	LWSVector4<float> xwzx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 2, 3, 0));
	}

	LWSVector4<float> xwzy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 2, 3, 0));
	}

	LWSVector4<float> xwzz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 2, 3, 0));
	}

	LWSVector4<float> xwzw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 3, 0));
	}

	LWSVector4<float> xwwx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 3, 3, 0));
	}

	LWSVector4<float> xwwy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 3, 3, 0));
	}

	LWSVector4<float> xwwz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 3, 3, 0));
	}

	LWSVector4<float> xwww(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 3, 3, 0));
	}

	LWSVector4<float> yxxx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 0, 0, 1));
	}

	LWSVector4<float> yxxy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 0, 0, 1));
	}

	LWSVector4<float> yxxz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 0, 0, 1));
	}

	LWSVector4<float> yxxw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 0, 1));
	}

	LWSVector4<float> yxyx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 1, 0, 1));
	}

	LWSVector4<float> yxyy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 1, 0, 1));
	}

	LWSVector4<float> yxyz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 1, 0, 1));
	}

	LWSVector4<float> yxyw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 0, 1));
	}

	LWSVector4<float> yxzx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 2, 0, 1));
	}

	LWSVector4<float> yxzy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 2, 0, 1));
	}

	LWSVector4<float> yxzz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 2, 0, 1));
	}

	LWSVector4<float> yxzw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 0, 1));
	}

	LWSVector4<float> yxwx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 3, 0, 1));
	}

	LWSVector4<float> yxwy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 3, 0, 1));
	}

	LWSVector4<float> yxwz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 3, 0, 1));
	}

	LWSVector4<float> yxww(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 3, 0, 1));
	}

	LWSVector4<float> yyxx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 0, 1, 1));
	}

	LWSVector4<float> yyxy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 0, 1, 1));
	}

	LWSVector4<float> yyxz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 0, 1, 1));
	}

	LWSVector4<float> yyxw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 1, 1));
	}

	LWSVector4<float> yyyx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 1, 1, 1));
	}

	LWSVector4<float> yyyy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 1, 1, 1));
	}

	LWSVector4<float> yyyz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 1, 1, 1));
	}

	LWSVector4<float> yyyw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 1, 1));
	}

	LWSVector4<float> yyzx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 2, 1, 1));
	}

	LWSVector4<float> yyzy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 2, 1, 1));
	}

	LWSVector4<float> yyzz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 2, 1, 1));
	}

	LWSVector4<float> yyzw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 1, 1));
	}

	LWSVector4<float> yywx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 3, 1, 1));
	}

	LWSVector4<float> yywy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 3, 1, 1));
	}

	LWSVector4<float> yywz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 3, 1, 1));
	}

	LWSVector4<float> yyww(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 3, 1, 1));
	}

	LWSVector4<float> yzxx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 0, 2, 1));
	}

	LWSVector4<float> yzxy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 0, 2, 1));
	}

	LWSVector4<float> yzxz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 0, 2, 1));
	}

	LWSVector4<float> yzxw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 2, 1));
	}

	LWSVector4<float> yzyx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 1, 2, 1));
	}

	LWSVector4<float> yzyy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 1, 2, 1));
	}

	LWSVector4<float> yzyz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 1, 2, 1));
	}

	LWSVector4<float> yzyw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 2, 1));
	}

	LWSVector4<float> yzzx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 2, 2, 1));
	}

	LWSVector4<float> yzzy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 2, 2, 1));
	}

	LWSVector4<float> yzzz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 2, 2, 1));
	}

	LWSVector4<float> yzzw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 2, 1));
	}

	LWSVector4<float> yzwx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 3, 2, 1));
	}

	LWSVector4<float> yzwy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 3, 2, 1));
	}

	LWSVector4<float> yzwz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 3, 2, 1));
	}

	LWSVector4<float> yzww(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 3, 2, 1));
	}

	LWSVector4<float> ywxx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 0, 3, 1));
	}

	LWSVector4<float> ywxy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 0, 3, 1));
	}

	LWSVector4<float> ywxz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 0, 3, 1));
	}

	LWSVector4<float> ywxw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 3, 1));
	}

	LWSVector4<float> ywyx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 1, 3, 1));
	}

	LWSVector4<float> ywyy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 1, 3, 1));
	}

	LWSVector4<float> ywyz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 1, 3, 1));
	}

	LWSVector4<float> ywyw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 3, 1));
	}

	LWSVector4<float> ywzx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 2, 3, 1));
	}

	LWSVector4<float> ywzy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 2, 3, 1));
	}

	LWSVector4<float> ywzz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 2, 3, 1));
	}

	LWSVector4<float> ywzw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 3, 1));
	}

	LWSVector4<float> ywwx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 3, 3, 1));
	}

	LWSVector4<float> ywwy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 3, 3, 1));
	}

	LWSVector4<float> ywwz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 3, 3, 1));
	}

	LWSVector4<float> ywww(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 3, 3, 1));
	}

	LWSVector4<float> zxxx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 0, 0, 2));
	}

	LWSVector4<float> zxxy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 0, 0, 2));
	}

	LWSVector4<float> zxxz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 0, 0, 2));
	}

	LWSVector4<float> zxxw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 0, 2));
	}

	LWSVector4<float> zxyx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 1, 0, 2));
	}

	LWSVector4<float> zxyy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 1, 0, 2));
	}

	LWSVector4<float> zxyz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 1, 0, 2));
	}

	LWSVector4<float> zxyw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 0, 2));
	}

	LWSVector4<float> zxzx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 2, 0, 2));
	}

	LWSVector4<float> zxzy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 2, 0, 2));
	}

	LWSVector4<float> zxzz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 2, 0, 2));
	}

	LWSVector4<float> zxzw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 0, 2));
	}

	LWSVector4<float> zxwx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 3, 0, 2));
	}

	LWSVector4<float> zxwy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 3, 0, 2));
	}

	LWSVector4<float> zxwz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 3, 0, 2));
	}

	LWSVector4<float> zxww(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 3, 0, 2));
	}

	LWSVector4<float> zyxx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 0, 1, 2));
	}

	LWSVector4<float> zyxy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 0, 1, 2));
	}

	LWSVector4<float> zyxz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 0, 1, 2));
	}

	LWSVector4<float> zyxw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 1, 2));
	}

	LWSVector4<float> zyyx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 1, 1, 2));
	}

	LWSVector4<float> zyyy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 1, 1, 2));
	}

	LWSVector4<float> zyyz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 1, 1, 2));
	}

	LWSVector4<float> zyyw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 1, 2));
	}

	LWSVector4<float> zyzx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 2, 1, 2));
	}

	LWSVector4<float> zyzy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 2, 1, 2));
	}

	LWSVector4<float> zyzz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 2, 1, 2));
	}

	LWSVector4<float> zyzw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 1, 2));
	}

	LWSVector4<float> zywx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 3, 1, 2));
	}

	LWSVector4<float> zywy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 3, 1, 2));
	}

	LWSVector4<float> zywz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 3, 1, 2));
	}

	LWSVector4<float> zyww(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 3, 1, 2));
	}

	LWSVector4<float> zzxx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 0, 2, 2));
	}

	LWSVector4<float> zzxy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 0, 2, 2));
	}

	LWSVector4<float> zzxz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 0, 2, 2));
	}

	LWSVector4<float> zzxw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 2, 2));
	}

	LWSVector4<float> zzyx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 1, 2, 2));
	}

	LWSVector4<float> zzyy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 1, 2, 2));
	}

	LWSVector4<float> zzyz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 1, 2, 2));
	}

	LWSVector4<float> zzyw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 2, 2));
	}

	LWSVector4<float> zzzx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 2, 2, 2));
	}

	LWSVector4<float> zzzy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 2, 2, 2));
	}

	LWSVector4<float> zzzz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 2, 2, 2));
	}

	LWSVector4<float> zzzw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 2, 2));
	}

	LWSVector4<float> zzwx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 3, 2, 2));
	}

	LWSVector4<float> zzwy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 3, 2, 2));
	}

	LWSVector4<float> zzwz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 3, 2, 2));
	}

	LWSVector4<float> zzww(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 3, 2, 2));
	}

	LWSVector4<float> zwxx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 0, 3, 2));
	}

	LWSVector4<float> zwxy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 0, 3, 2));
	}

	LWSVector4<float> zwxz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 0, 3, 2));
	}

	LWSVector4<float> zwxw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 3, 2));
	}

	LWSVector4<float> zwyx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 1, 3, 2));
	}

	LWSVector4<float> zwyy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 1, 3, 2));
	}

	LWSVector4<float> zwyz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 1, 3, 2));
	}

	LWSVector4<float> zwyw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 3, 2));
	}

	LWSVector4<float> zwzx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 2, 3, 2));
	}

	LWSVector4<float> zwzy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 2, 3, 2));
	}

	LWSVector4<float> zwzz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 2, 3, 2));
	}

	LWSVector4<float> zwzw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 3, 2));
	}

	LWSVector4<float> zwwx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 3, 3, 2));
	}

	LWSVector4<float> zwwy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 3, 3, 2));
	}

	LWSVector4<float> zwwz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 3, 3, 2));
	}

	LWSVector4<float> zwww(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 3, 3, 2));
	}

	LWSVector4<float> wxxx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 0, 0, 3));
	}

	LWSVector4<float> wxxy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 0, 0, 3));
	}

	LWSVector4<float> wxxz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 0, 0, 3));
	}

	LWSVector4<float> wxxw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 0, 3));
	}

	LWSVector4<float> wxyx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 1, 0, 3));
	}

	LWSVector4<float> wxyy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 1, 0, 3));
	}

	LWSVector4<float> wxyz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 1, 0, 3));
	}

	LWSVector4<float> wxyw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 0, 3));
	}

	LWSVector4<float> wxzx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 2, 0, 3));
	}

	LWSVector4<float> wxzy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 2, 0, 3));
	}

	LWSVector4<float> wxzz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 2, 0, 3));
	}

	LWSVector4<float> wxzw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 0, 3));
	}

	LWSVector4<float> wxwx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 3, 0, 3));
	}

	LWSVector4<float> wxwy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 3, 0, 3));
	}

	LWSVector4<float> wxwz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 3, 0, 3));
	}

	LWSVector4<float> wxww(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 3, 0, 3));
	}

	LWSVector4<float> wyxx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 0, 1, 3));
	}

	LWSVector4<float> wyxy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 0, 1, 3));
	}

	LWSVector4<float> wyxz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 0, 1, 3));
	}

	LWSVector4<float> wyxw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 1, 3));
	}

	LWSVector4<float> wyyx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 1, 1, 3));
	}

	LWSVector4<float> wyyy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 1, 1, 3));
	}

	LWSVector4<float> wyyz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 1, 1, 3));
	}

	LWSVector4<float> wyyw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 1, 3));
	}

	LWSVector4<float> wyzx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 2, 1, 3));
	}

	LWSVector4<float> wyzy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 2, 1, 3));
	}

	LWSVector4<float> wyzz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 2, 1, 3));
	}

	LWSVector4<float> wyzw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 1, 3));
	}

	LWSVector4<float> wywx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 3, 1, 3));
	}

	LWSVector4<float> wywy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 3, 1, 3));
	}

	LWSVector4<float> wywz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 3, 1, 3));
	}

	LWSVector4<float> wyww(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 3, 1, 3));
	}

	LWSVector4<float> wzxx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 0, 2, 3));
	}

	LWSVector4<float> wzxy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 0, 2, 3));
	}

	LWSVector4<float> wzxz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 0, 2, 3));
	}

	LWSVector4<float> wzxw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 2, 3));
	}

	LWSVector4<float> wzyx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 1, 2, 3));
	}

	LWSVector4<float> wzyy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 1, 2, 3));
	}

	LWSVector4<float> wzyz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 1, 2, 3));
	}

	LWSVector4<float> wzyw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 2, 3));
	}

	LWSVector4<float> wzzx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 2, 2, 3));
	}

	LWSVector4<float> wzzy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 2, 2, 3));
	}

	LWSVector4<float> wzzz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 2, 2, 3));
	}

	LWSVector4<float> wzzw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 2, 3));
	}

	LWSVector4<float> wzwx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 3, 2, 3));
	}

	LWSVector4<float> wzwy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 3, 2, 3));
	}

	LWSVector4<float> wzwz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 3, 2, 3));
	}

	LWSVector4<float> wzww(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 3, 2, 3));
	}

	LWSVector4<float> wwxx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 0, 3, 3));
	}

	LWSVector4<float> wwxy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 0, 3, 3));
	}

	LWSVector4<float> wwxz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 0, 3, 3));
	}

	LWSVector4<float> wwxw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 3, 3));
	}

	LWSVector4<float> wwyx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 1, 3, 3));
	}

	LWSVector4<float> wwyy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 1, 3, 3));
	}

	LWSVector4<float> wwyz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 1, 3, 3));
	}

	LWSVector4<float> wwyw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 3, 3));
	}

	LWSVector4<float> wwzx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 2, 3, 3));
	}

	LWSVector4<float> wwzy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 2, 3, 3));
	}

	LWSVector4<float> wwzz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 2, 3, 3));
	}

	LWSVector4<float> wwzw(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 3, 3));
	}

	LWSVector4<float> wwwx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(0, 3, 3, 3));
	}

	LWSVector4<float> wwwy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(1, 3, 3, 3));
	}

	LWSVector4<float> wwwz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(2, 3, 3, 3));
	}

	LWSVector4<float> wwww(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 3, 3, 3));
	}

	LWSVector4<float> xxx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 0, 0));
	}

	LWSVector4<float> xxy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 0, 0));
	}

	LWSVector4<float> xxz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 0, 0));
	}

	LWSVector4<float> xyx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 1, 0));
	}

	LWSVector4<float> xyy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 1, 0));
	}

	LWSVector4<float> xyz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 1, 0));
	}

	LWSVector4<float> xzx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 2, 0));
	}

	LWSVector4<float> xzy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 2, 0));
	}

	LWSVector4<float> xzz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 2, 0));
	}

	LWSVector4<float> yxx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 0, 1));
	}

	LWSVector4<float> yxy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 0, 1));
	}

	LWSVector4<float> yxz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 0, 1));
	}

	LWSVector4<float> yyx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 1, 1));
	}

	LWSVector4<float> yyy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 1, 1));
	}

	LWSVector4<float> yyz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 1, 1));
	}

	LWSVector4<float> yzx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 2, 1));
	}

	LWSVector4<float> yzy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 2, 1));
	}

	LWSVector4<float> yzz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 2, 1));
	}

	LWSVector4<float> zxx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 0, 2));
	}

	LWSVector4<float> zxy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 0, 2));
	}

	LWSVector4<float> zxz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 0, 2));
	}

	LWSVector4<float> zyx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 1, 2));
	}

	LWSVector4<float> zyy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 1, 2));
	}

	LWSVector4<float> zyz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 1, 2));
	}

	LWSVector4<float> zzx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 0, 2, 2));
	}

	LWSVector4<float> zzy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 1, 2, 2));
	}

	LWSVector4<float> zzz(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 2, 2));
	}

	LWSVector4<float> xx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 0, 0));
	}

	LWSVector4<float> xy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 1, 0));
	}

	LWSVector4<float> yx(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 0, 1));
	}

	LWSVector4<float> yy(void) const {
		return _mm_permute_ps(m_Data, _MM_SHUFFLE(3, 2, 1, 1));
	}

	float x(void) const {
		alignas(16) LWVector4<float> R;
		_mm_store_ps(&R.x, m_Data);
		return R.x;
	}

	float y(void) const {
		alignas(16) LWVector4<float> R;
		_mm_store_ps(&R.x, m_Data);
		return R.y;
	}

	float z(void) const {
		alignas(16) LWVector4<float> R;
		_mm_store_ps(&R.x, m_Data);
		return R.z;
	}

	float w(void) const {
		alignas(16) LWVector4<float> R;
		_mm_store_ps(&R.x, m_Data);
		return R.w;
	}

	LWSVector4(__m128 Data) : m_Data(Data) {}

	LWSVector4(const LWVector4<float> &vxyzw) : m_Data(_mm_set_ps(vxyzw.w, vxyzw.z, vxyzw.y, vxyzw.x)) {}

	LWSVector4(const LWVector3<float> &vxyz, float vw) : m_Data(_mm_set_ps(vw, vxyz.z, vxyz.y, vxyz.x)) {}

	LWSVector4(const LWVector2<float> &vxy, const LWVector2<float> &vzw) : m_Data(_mm_set_ps(vzw.y, vzw.x, vxy.y, vxy.x)) {}

	LWSVector4(const LWVector2<float> &vxy, float vz, float vw) : m_Data(_mm_set_ps(vw, vz, vxy.y, vxy.x)) {}

	LWSVector4(float vx, float vy, float vz, float vw) : m_Data(_mm_set_ps(vw, vz, vy, vx)) {}

	LWSVector4(float f = 0.0f) : m_Data(_mm_set_ps1(f)) {}
};

>>>>>>> Added initial inroads for vulkan support(this is far from complete and not usable yet).
#endif