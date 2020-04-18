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

#endif