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

	LWSVector4<float> Sign(void) const;

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
#endif