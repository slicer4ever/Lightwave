#ifndef LWSVECTOR_AVX2_DOUBLE_H
#define LWSVECTOR_AVX2_DOUBLE_H
#include "LWSVector.h"
#include <smmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

template<>
struct LWSVector4<double> {
	__m256d m_Data;

	LWVector4<double> AsVec4(void) const;

	double *AsArray(void);

	const double *AsArray(void) const;

	LWSVector4<double> &sX(double Value);

	LWSVector4<double> &sY(double Value);

	LWSVector4<double> &sZ(double Value);

	LWSVector4<double> &sW(double Value);

	LWSVector4<double> Normalize(void) const;

	LWSVector4<double> Normalize3(void) const;

	LWSVector4<double> Normalize2(void) const;

	double Dot(const LWSVector4<double>& O) const;

	double Dot3(const LWSVector4<double>& O) const;

	double Dot2(const LWSVector4<double>& O) const;

	LWSVector4<double> Sum(void) const;

	double Sum4(void) const;

	double Sum3(void) const;

	double Sum2(void) const;

	double Min(void) const;

	double Min3(void) const;

	double Min2(void) const;

	double Max(void) const;

	double Max3(void) const;

	double Max2(void) const;

	LWSVector4<double> Min(const LWSVector4<double> &A) const;

	LWSVector4<double> Max(const LWSVector4<double> &A) const;

	LWSVector4<double> Cross3(const LWSVector4<double>& O) const;

	void Orthogonal3(LWSVector4<double> &Right, LWSVector4<double> &Up) const;

	LWSVector4<double> Perpindicular2(void) const;

	double Length(void) const;

	double Length3(void) const;

	double Length2(void) const;

	double LengthSquared(void) const;

	double LengthSquared3(void) const;

	double LengthSquared2(void) const;

	double Distance(const LWSVector4<double>& O) const;

	double Distance3(const LWSVector4<double>& O) const;

	double Distance2(const LWSVector4<double>& O) const;

	double DistanceSquared(const LWSVector4<double>& O) const;

	double DistanceSquared3(const LWSVector4<double>& O) const;

	double DistanceSquared2(const LWSVector4<double>& O) const;

	/*! \brief returns the absolute value of each component. */
	LWSVector4<double> Abs(void) const;

	/*! \brief returns the absolute value of x,y, and z component. */
	LWSVector4<double> Abs3(void) const;

	/*! \brief returns the absolute value of x, and y component. */
	LWSVector4<double> Abs2(void) const;

	/*! \brief compares each component, if component is < rhs, then stores Value's component, otherwise keeps current component. */
	LWSVector4<double> Blend_Less(const LWSVector4<double> &Rhs, const LWSVector4<double> &Value) const;

	/*! \brief compares x, y, and z component, if component is < rhs than store's value's component, otherwise keeps current component. */
	LWSVector4<double> Blend_Less3(const LWSVector4<double> &Rhs, const LWSVector4<double> &Value) const;

	/*! \brief compares x, and y component, if component is < rhs than store's value's component, otherwise keeps current component. */
	LWSVector4<double> Blend_Less2(const LWSVector4<double> &Rhs, const LWSVector4<double> &Value) const;

	/*! \brief compares each component, if component is <= rhs, than stores Value's component, otherwise keeps current component. */
	LWSVector4<double> Blend_LessEqual(const LWSVector4<double> &Rhs, const LWSVector4<double> &Value) const;

	/*! \brief compares x, y, and z component, if component is <= rhs, than stores Value's component, otherwise keeps current component. */
	LWSVector4<double> Blend_LessEqual3(const LWSVector4<double> &Rhs, const LWSVector4<double> &Value) const;

	/*! \brief compares x, and y component, if component is <= rhs, than stores Value's component, otherwise keeps current component. */
	LWSVector4<double> Blend_LessEqual2(const LWSVector4<double> &Rhs, const LWSVector4<double> &Value) const;

	/*! \brief compares each component, if component is > rhs than stores Value's component, otherwise keeps current component. */
	LWSVector4<double> Blend_Greater(const LWSVector4<double> &Rhs, const LWSVector4<double> &Value) const;

	/*! \brief compares x, y, and z component, if component is > rhs than stores Value's component, otherwise keeps current component. */
	LWSVector4<double> Blend_Greater3(const LWSVector4<double> &Rhs, const LWSVector4<double> &Value) const;

	/*! \brief compares x, and y component, if component is > rhs than stores Value's component, otherwise keeps current component. */
	LWSVector4<double> Blend_Greater2(const LWSVector4<double> &Rhs, const LWSVector4<double> &Value) const;

	/*! \brief compares each component if component is >= rhs than stores value's component, otherwise keeps current component. */
	LWSVector4<double> Blend_GreaterEqual(const LWSVector4<double> &Rhs, const LWSVector4<double> &Value) const;

	/*! \brief compares x, y, and z component if component is >= rhs than stores value's component, otherwise keeps current component. */
	LWSVector4<double> Blend_GreaterEqual3(const LWSVector4<double> &Rhs, const LWSVector4<double> &Value) const;

	/*! \brief compares x, and y component if component is >= rhs than stores value's component, otherwise keeps current component. */
	LWSVector4<double> Blend_GreaterEqual2(const LWSVector4<double> &Rhs, const LWSVector4<double> &Value) const;

	/*! \brief compares each component, if component is == rhs(use's float epsilon for comparison) than stores value's component, otherwise keeps current component. */
	LWSVector4<double> Blend_Equal(const LWSVector4<double> &Rhs, const LWSVector4<double> &Value) const;

	/*! \brief compares x, y, and z component, if component is == rhs(use's float epsilon for comparison) than stores value's component, otherwise keeps current component. */
	LWSVector4<double> Blend_Equal3(const LWSVector4<double> &Rhs, const LWSVector4<double> &Value) const;

	/*! \brief compares x, and y component, if component is == rhs(use's float epsilon for comparison) than stores value's component, otherwise keeps current component. */
	LWSVector4<double> Blend_Equal2(const LWSVector4<double> &Rhs, const LWSVector4<double> &Value) const;

	/*! \brief compares each component, if component is != rhs(use's double float for comparison) than stores value's component, otherwise keeps current component. */
	LWSVector4<double> Blend_NotEqual(const LWSVector4<double> &Rhs, const LWSVector4<double> &Value) const;

	/*! \brief compares x, y, and z component, if component is != rhs(use's float epsilon for comparison) than stores value's component, otherwise keeps current component. */
	LWSVector4<double> Blend_NotEqual3(const LWSVector4<double> &Rhs, const LWSVector4<double> &Value) const;

	/*! \brief compares x, and y component, if component is != rhs(use's float epsilon for comparison) than stores value's component, otherwise keeps current component. */
	LWSVector4<double> Blend_NotEqual2(const LWSVector4<double> &Rhs, const LWSVector4<double> &Value) const;

	bool Less3(const LWSVector4<double> &Rhs) const;

	bool Less2(const LWSVector4<double> &Rhs) const;

	bool LessEqual3(const LWSVector4<double> &Rhs) const;

	bool LessEqual2(const LWSVector4<double> &Rhs) const;

	bool Greater3(const LWSVector4<double> &Rhs) const;

	bool Greater2(const LWSVector4<double> &Rhs) const;

	bool GreaterEqual3(const LWSVector4<double> &Rhs) const;

	bool GreaterEqual2(const LWSVector4<double> &Rhs) const;

	LWSVector4<double>& operator = (const LWSVector4<double>& Rhs);

	LWSVector4<double>& operator += (const LWSVector4<double>& Rhs);

	LWSVector4<double>& operator += (double Rhs);

	LWSVector4<double>& operator -= (const LWSVector4<double>& Rhs);

	LWSVector4<double>& operator -= (double Rhs);

	LWSVector4<double>& operator *= (const LWSVector4<double>& Rhs);

	LWSVector4<double>& operator *= (double Rhs);

	LWSVector4<double>& operator /= (const LWSVector4<double>& Rhs);

	LWSVector4<double>& operator /= (double Rhs);

	friend LWSVector4<double> operator + (const LWSVector4<double> &Rhs);

	friend LWSVector4<double> operator - (const LWSVector4<double>& Rhs);

	bool operator > (const LWSVector4<double> &Rhs) const;

	bool operator >= (const LWSVector4<double> &Rhs) const;

	bool operator < (const LWSVector4<double> &Rhs) const;

	bool operator <= (const LWSVector4<double> &Rhs) const;

	bool operator == (const LWSVector4<double>& Rhs) const;

	bool operator != (const LWSVector4<double> &Rhs) const;

	friend std::ostream& operator<<(std::ostream& o, const LWSVector4<double>& v);

	friend LWSVector4<double> operator + (const LWSVector4<double> &Lhs, const LWSVector4<double> &Rhs);

	friend LWSVector4<double> operator + (const LWSVector4<double>& Lhs, double Rhs);

	friend LWSVector4<double> operator + (double Lhs, const LWSVector4<double>& Rhs);

	friend LWSVector4<double> operator - (const LWSVector4<double> &Lhs, const LWSVector4<double> &Rhs);

	friend LWSVector4<double> operator - (const LWSVector4<double>& Lhs, double Rhs);

	friend LWSVector4<double> operator - (double Lhs, const LWSVector4<double>& Rhs);

	friend LWSVector4<double> operator * (const LWSVector4<double> &Lhs, const LWSVector4<double> &Rhs);

	friend LWSVector4<double> operator * (const LWSVector4<double>& Lhs, double Rhs);

	friend LWSVector4<double> operator * (double Lhs, const LWSVector4<double>& Rhs);

	friend LWSVector4<double> operator / (const LWSVector4<double> &Lhs, const LWSVector4<double> &Rhs);

	friend LWSVector4<double> operator / (const LWSVector4<double>& Lhs, double Rhs);

	friend LWSVector4<double> operator / (double Lhs, const LWSVector4<double>& Rhs);

	LWSVector4<double> AAAB(const LWSVector4<double> &B) const;

	LWSVector4<double> AABA(const LWSVector4<double> &B) const;

	LWSVector4<double> AABB(const LWSVector4<double> &B) const;

	LWSVector4<double> ABAA(const LWSVector4<double> &B) const;

	LWSVector4<double> ABAB(const LWSVector4<double> &B) const;

	LWSVector4<double> ABBA(const LWSVector4<double> &B) const;

	LWSVector4<double> ABBB(const LWSVector4<double> &B) const;

	LWSVector4<double> BAAA(const LWSVector4<double> &B) const;

	LWSVector4<double> BAAB(const LWSVector4<double> &B) const;

	LWSVector4<double> BABA(const LWSVector4<double> &B) const;

	LWSVector4<double> BABB(const LWSVector4<double> &B) const;

	LWSVector4<double> BBAA(const LWSVector4<double> &B) const;

	LWSVector4<double> BBAB(const LWSVector4<double> &B) const;

	LWSVector4<double> BBBA(const LWSVector4<double> &B) const;

	LWSVector4<double> xxxx(void) const;

	LWSVector4<double> xxxy(void) const;

	LWSVector4<double> xxxz(void) const;

	LWSVector4<double> xxxw(void) const;

	LWSVector4<double> xxyx(void) const;

	LWSVector4<double> xxyy(void) const;

	LWSVector4<double> xxyz(void) const;
	LWSVector4<double> xxyw(void) const;

	LWSVector4<double> xxzx(void) const;

	LWSVector4<double> xxzy(void) const;

	LWSVector4<double> xxzz(void) const;

	LWSVector4<double> xxzw(void) const;

	LWSVector4<double> xxwx(void) const;

	LWSVector4<double> xxwy(void) const;

	LWSVector4<double> xxwz(void) const;

	LWSVector4<double> xxww(void) const;

	LWSVector4<double> xyxx(void) const;

	LWSVector4<double> xyxy(void) const;

	LWSVector4<double> xyxz(void) const;

	LWSVector4<double> xyxw(void) const;

	LWSVector4<double> xyyx(void) const;

	LWSVector4<double> xyyy(void) const;

	LWSVector4<double> xyyz(void) const;

	LWSVector4<double> xyyw(void) const;

	LWSVector4<double> xyzx(void) const;

	LWSVector4<double> xyzy(void) const;

	LWSVector4<double> xyzz(void) const;

	LWSVector4<double> xywx(void) const;

	LWSVector4<double> xywy(void) const;

	LWSVector4<double> xywz(void) const;

	LWSVector4<double> xyww(void) const;

	LWSVector4<double> xzxx(void) const;

	LWSVector4<double> xzxy(void) const;

	LWSVector4<double> xzxz(void) const;

	LWSVector4<double> xzxw(void) const;

	LWSVector4<double> xzyx(void) const;

	LWSVector4<double> xzyy(void) const;

	LWSVector4<double> xzyz(void) const;

	LWSVector4<double> xzyw(void) const;

	LWSVector4<double> xzzx(void) const;

	LWSVector4<double> xzzy(void) const;

	LWSVector4<double> xzzz(void) const;

	LWSVector4<double> xzzw(void) const;

	LWSVector4<double> xzwx(void) const;

	LWSVector4<double> xzwy(void) const;

	LWSVector4<double> xzwz(void) const;

	LWSVector4<double> xzww(void) const;

	LWSVector4<double> xwxx(void) const;

	LWSVector4<double> xwxy(void) const;

	LWSVector4<double> xwxz(void) const;

	LWSVector4<double> xwxw(void) const;

	LWSVector4<double> xwyx(void) const;

	LWSVector4<double> xwyy(void) const;

	LWSVector4<double> xwyz(void) const;

	LWSVector4<double> xwyw(void) const;

	LWSVector4<double> xwzx(void) const;

	LWSVector4<double> xwzy(void) const;

	LWSVector4<double> xwzz(void) const;

	LWSVector4<double> xwzw(void) const;

	LWSVector4<double> xwwx(void) const;

	LWSVector4<double> xwwy(void) const;

	LWSVector4<double> xwwz(void) const;

	LWSVector4<double> xwww(void) const;

	LWSVector4<double> yxxx(void) const;

	LWSVector4<double> yxxy(void) const;

	LWSVector4<double> yxxz(void) const;

	LWSVector4<double> yxxw(void) const;

	LWSVector4<double> yxyx(void) const;

	LWSVector4<double> yxyy(void) const;

	LWSVector4<double> yxyz(void) const;

	LWSVector4<double> yxyw(void) const;

	LWSVector4<double> yxzx(void) const;

	LWSVector4<double> yxzy(void) const;

	LWSVector4<double> yxzz(void) const;

	LWSVector4<double> yxzw(void) const;

	LWSVector4<double> yxwx(void) const;

	LWSVector4<double> yxwy(void) const;

	LWSVector4<double> yxwz(void) const;

	LWSVector4<double> yxww(void) const;

	LWSVector4<double> yyxx(void) const;

	LWSVector4<double> yyxy(void) const;

	LWSVector4<double> yyxz(void) const;

	LWSVector4<double> yyxw(void) const;

	LWSVector4<double> yyyx(void) const;

	LWSVector4<double> yyyy(void) const;

	LWSVector4<double> yyyz(void) const;

	LWSVector4<double> yyyw(void) const;

	LWSVector4<double> yyzx(void) const;

	LWSVector4<double> yyzy(void) const;

	LWSVector4<double> yyzz(void) const;

	LWSVector4<double> yyzw(void) const;

	LWSVector4<double> yywx(void) const;

	LWSVector4<double> yywy(void) const;

	LWSVector4<double> yywz(void) const;

	LWSVector4<double> yyww(void) const;

	LWSVector4<double> yzxx(void) const;

	LWSVector4<double> yzxy(void) const;

	LWSVector4<double> yzxz(void) const;

	LWSVector4<double> yzxw(void) const;

	LWSVector4<double> yzyx(void) const;

	LWSVector4<double> yzyy(void) const;

	LWSVector4<double> yzyz(void) const;

	LWSVector4<double> yzyw(void) const;

	LWSVector4<double> yzzx(void) const;

	LWSVector4<double> yzzy(void) const;

	LWSVector4<double> yzzz(void) const;

	LWSVector4<double> yzzw(void) const;

	LWSVector4<double> yzwx(void) const;

	LWSVector4<double> yzwy(void) const;

	LWSVector4<double> yzwz(void) const;

	LWSVector4<double> yzww(void) const;

	LWSVector4<double> ywxx(void) const;

	LWSVector4<double> ywxy(void) const;

	LWSVector4<double> ywxz(void) const;

	LWSVector4<double> ywxw(void) const;

	LWSVector4<double> ywyx(void) const;

	LWSVector4<double> ywyy(void) const;

	LWSVector4<double> ywyz(void) const;

	LWSVector4<double> ywyw(void) const;

	LWSVector4<double> ywzx(void) const;

	LWSVector4<double> ywzy(void) const;

	LWSVector4<double> ywzz(void) const;

	LWSVector4<double> ywzw(void) const;

	LWSVector4<double> ywwx(void) const;

	LWSVector4<double> ywwy(void) const;

	LWSVector4<double> ywwz(void) const;

	LWSVector4<double> ywww(void) const;

	LWSVector4<double> zxxx(void) const;

	LWSVector4<double> zxxy(void) const;

	LWSVector4<double> zxxz(void) const;

	LWSVector4<double> zxxw(void) const;

	LWSVector4<double> zxyx(void) const;

	LWSVector4<double> zxyy(void) const;

	LWSVector4<double> zxyz(void) const;

	LWSVector4<double> zxyw(void) const;

	LWSVector4<double> zxzx(void) const;

	LWSVector4<double> zxzy(void) const;

	LWSVector4<double> zxzz(void) const;

	LWSVector4<double> zxzw(void) const;

	LWSVector4<double> zxwx(void) const;

	LWSVector4<double> zxwy(void) const;

	LWSVector4<double> zxwz(void) const;

	LWSVector4<double> zxww(void) const;

	LWSVector4<double> zyxx(void) const;

	LWSVector4<double> zyxy(void) const;

	LWSVector4<double> zyxz(void) const;

	LWSVector4<double> zyxw(void) const;

	LWSVector4<double> zyyx(void) const;

	LWSVector4<double> zyyy(void) const;

	LWSVector4<double> zyyz(void) const;

	LWSVector4<double> zyyw(void) const;

	LWSVector4<double> zyzx(void) const;

	LWSVector4<double> zyzy(void) const;

	LWSVector4<double> zyzz(void) const;

	LWSVector4<double> zyzw(void) const;

	LWSVector4<double> zywx(void) const;

	LWSVector4<double> zywy(void) const;

	LWSVector4<double> zywz(void) const;

	LWSVector4<double> zyww(void) const;

	LWSVector4<double> zzxx(void) const;

	LWSVector4<double> zzxy(void) const;

	LWSVector4<double> zzxz(void) const;

	LWSVector4<double> zzxw(void) const;

	LWSVector4<double> zzyx(void) const;

	LWSVector4<double> zzyy(void) const;

	LWSVector4<double> zzyz(void) const;

	LWSVector4<double> zzyw(void) const;

	LWSVector4<double> zzzx(void) const;

	LWSVector4<double> zzzy(void) const;

	LWSVector4<double> zzzz(void) const;

	LWSVector4<double> zzzw(void) const;

	LWSVector4<double> zzwx(void) const;

	LWSVector4<double> zzwy(void) const;

	LWSVector4<double> zzwz(void) const;

	LWSVector4<double> zzww(void) const;

	LWSVector4<double> zwxx(void) const;

	LWSVector4<double> zwxy(void) const;

	LWSVector4<double> zwxz(void) const;

	LWSVector4<double> zwxw(void) const;

	LWSVector4<double> zwyx(void) const;

	LWSVector4<double> zwyy(void) const;

	LWSVector4<double> zwyz(void) const;

	LWSVector4<double> zwyw(void) const;

	LWSVector4<double> zwzx(void) const;

	LWSVector4<double> zwzy(void) const;

	LWSVector4<double> zwzz(void) const;

	LWSVector4<double> zwzw(void) const;

	LWSVector4<double> zwwx(void) const;

	LWSVector4<double> zwwy(void) const;

	LWSVector4<double> zwwz(void) const;

	LWSVector4<double> zwww(void) const;

	LWSVector4<double> wxxx(void) const;

	LWSVector4<double> wxxy(void) const;

	LWSVector4<double> wxxz(void) const;

	LWSVector4<double> wxxw(void) const;

	LWSVector4<double> wxyx(void) const;

	LWSVector4<double> wxyy(void) const;

	LWSVector4<double> wxyz(void) const;

	LWSVector4<double> wxyw(void) const;

	LWSVector4<double> wxzx(void) const;

	LWSVector4<double> wxzy(void) const;

	LWSVector4<double> wxzz(void) const;

	LWSVector4<double> wxzw(void) const;

	LWSVector4<double> wxwx(void) const;

	LWSVector4<double> wxwy(void) const;

	LWSVector4<double> wxwz(void) const;

	LWSVector4<double> wxww(void) const;

	LWSVector4<double> wyxx(void) const;

	LWSVector4<double> wyxy(void) const;

	LWSVector4<double> wyxz(void) const;

	LWSVector4<double> wyxw(void) const;

	LWSVector4<double> wyyx(void) const;

	LWSVector4<double> wyyy(void) const;

	LWSVector4<double> wyyz(void) const;

	LWSVector4<double> wyyw(void) const;

	LWSVector4<double> wyzx(void) const;

	LWSVector4<double> wyzy(void) const;

	LWSVector4<double> wyzz(void) const;

	LWSVector4<double> wyzw(void) const;

	LWSVector4<double> wywx(void) const;

	LWSVector4<double> wywy(void) const;

	LWSVector4<double> wywz(void) const;

	LWSVector4<double> wyww(void) const;

	LWSVector4<double> wzxx(void) const;

	LWSVector4<double> wzxy(void) const;

	LWSVector4<double> wzxz(void) const;

	LWSVector4<double> wzxw(void) const;

	LWSVector4<double> wzyx(void) const;

	LWSVector4<double> wzyy(void) const;

	LWSVector4<double> wzyz(void) const;

	LWSVector4<double> wzyw(void) const;

	LWSVector4<double> wzzx(void) const;

	LWSVector4<double> wzzy(void) const;

	LWSVector4<double> wzzz(void) const;

	LWSVector4<double> wzzw(void) const;

	LWSVector4<double> wzwx(void) const;

	LWSVector4<double> wzwy(void) const;

	LWSVector4<double> wzwz(void) const;

	LWSVector4<double> wzww(void) const;

	LWSVector4<double> wwxx(void) const;

	LWSVector4<double> wwxy(void) const;

	LWSVector4<double> wwxz(void) const;

	LWSVector4<double> wwxw(void) const;

	LWSVector4<double> wwyx(void) const;

	LWSVector4<double> wwyy(void) const;

	LWSVector4<double> wwyz(void) const;

	LWSVector4<double> wwyw(void) const;

	LWSVector4<double> wwzx(void) const;

	LWSVector4<double> wwzy(void) const;

	LWSVector4<double> wwzz(void) const;

	LWSVector4<double> wwzw(void) const;

	LWSVector4<double> wwwx(void) const;

	LWSVector4<double> wwwy(void) const;

	LWSVector4<double> wwwz(void) const;

	LWSVector4<double> wwww(void) const;

	LWSVector4<double> xxx(void) const;

	LWSVector4<double> xxy(void) const;

	LWSVector4<double> xxz(void) const;

	LWSVector4<double> xyx(void) const;

	LWSVector4<double> xyy(void) const;

	LWSVector4<double> xyz(void) const;

	LWSVector4<double> xzx(void) const;

	LWSVector4<double> xzy(void) const;

	LWSVector4<double> xzz(void) const;

	LWSVector4<double> yxx(void) const;

	LWSVector4<double> yxy(void) const;

	LWSVector4<double> yxz(void) const;

	LWSVector4<double> yyx(void) const;

	LWSVector4<double> yyy(void) const;

	LWSVector4<double> yyz(void) const;

	LWSVector4<double> yzx(void) const;

	LWSVector4<double> yzy(void) const;

	LWSVector4<double> yzz(void) const;

	LWSVector4<double> zxx(void) const;

	LWSVector4<double> zxy(void) const;

	LWSVector4<double> zxz(void) const;

	LWSVector4<double> zyx(void) const;

	LWSVector4<double> zyy(void) const;

	LWSVector4<double> zyz(void) const;

	LWSVector4<double> zzx(void) const;

	LWSVector4<double> zzy(void) const;

	LWSVector4<double> zzz(void) const;

	LWSVector4<double> xx(void) const;

	LWSVector4<double> xy(void) const;

	LWSVector4<double> yx(void) const;

	LWSVector4<double> yy(void) const;

	double x(void) const;

	double y(void) const;

	double z(void) const;

	double w(void) const;

	LWSVector4(const __m256d Data);

	LWSVector4(const LWVector4<double>& vxyzw);

	LWSVector4(const LWVector3<double>& vxyz, double vw);

	LWSVector4(const LWVector2<double>& vxy, const LWVector2<double>& vzw);

	LWSVector4(const LWVector2<double>& vxy, double vz, double vw);

	LWSVector4(double vx, double vy, double vz, double vw);

	LWSVector4(double f = 0.0);
};


#endif