#ifndef LWESVECTOR_H
#define LWESVECTOR_H
#include <LWCore/LWTypes.h>
#include <LWCore/LWVector.h>
#include <ostream>

/*!< \brief an accelerated simd vector4 class, non-implemented or disabled sse functions default to a generic class. */
template<class Type>
struct LWSVector4 {
	Type m_x, m_y, m_z, m_w;

	/*!< \brief converts to a regular non-simd LWVector4 object. */
	LWVector4<Type> AsVec4(void) const {
		return LWVector4<Type>(m_x, m_y, m_z, m_w);
	}

	/*!< \brief normalizes the vec4. */
	LWSVector4<Type> Normalize(void) const {
		Type L = m_x * m_x + m_y * m_y + m_z * m_z + m_w * m_w;
		if (L < std::numeric_limits<Type>::epsilon()) L = (Type)0;
		else L = (Type)1 / (Type)sqrt(L);
		return LWSVector4<Type>(m_x*L, m_y*L, m_z*L, m_w*L);
	}

	/*!< \brief normalizes as if it were a vec3 object, ignoring the w component. */
	LWSVector4<Type> Normalize3(void) const {
		Type L = m_x * m_x + m_y * m_y + m_z * m_z;
		if (L < std::numeric_limits<Type>::epsilon()) L = (Type)0;
		else L = (Type)1 / (Type)sqrt(L);
		return LWSVector4<Type>(m_x*L, m_y*L, m_z*L, m_w);
	}

	/*!< \brief normalizes as if it were a vec2 object, ignoring the z and w component. */
	LWSVector4<Type> Normalize2(void) const {
		Type L = m_x * m_x + m_y * m_y;
		if (L < std::numeric_limits<Type>::epsilon()) L = (Type)0;
		else L = (Type)1 / (Type)sqrt(L);
		return LWSVector4<Type>(m_x*L, m_y*L, m_z, m_w);
	}

	/*!< \brief gets the dot product between two vector4's. */
	Type Dot(const LWSVector4<Type> &O) const {
		return m_x * O.m_x + m_y * O.m_y + m_z * O.m_z + m_w * O.m_w;
	}

	/*!< \brief gets the dot product between two vector4's as if they were vec3 object's, ignoring the w component. */
	Type Dot3(const LWSVector4<Type> &O) const {
		return m_x * O.m_x + m_y * O.m_y + m_z * O.m_z;
	}

	/*!< \brief gets the dot product between two vector4's as if they were vec2 object's, ignoring the w component. */
	Type Dot2(const LWSVector4<Type> &O) const {
		return m_x * O.m_x + m_y * O.m_y;
	}
	/*!< \brief returns a summed version of the SVector4 into each vec4 component. */
	LWSVector4<Type> Sum(void) const {
		Type S = Sum4();
		return LWSVector4<Type>(S, S, S, S);
	}

	/*!< \brief returns the 4 component sum of the vec4. */
	Type Sum(void) const {
		return m_x + m_y + m_z + m_w;
	};

	/*!< \brief returns the 3 component sum of the xyz components of the vec4. */
	Type Sum3(void) const {
		return m_x + m_y + m_z;
	}

	/*!< \brief returns the 2 component sum of the xy components of the vec4. */
	Type Sum2(void) const {
		return m_x + m_y;
	}

	/*!< \brief get's the minimum value of all components. */
	Type Min(void) const {
		return std::min<Type>(std::min<Type>(std::min<Type>(m_x, m_y), m_z), m_w);
	}

	/*!< \brief get's the minimum value of the x, y, and z components. */
	Type Min3(void) const {
		return std::min<Type>(std::min<Type>(m_x, m_y), m_z);
	}

	/*!< \brief get's the minimum value of the x, y components. */
	Type Min2(void) const {
		return std::min<Type>(m_x, m_y);
	}

	/*!< \brief get's the maximum value of all components. */
	Type Max(void) const {
		return std::max<Type>(std::max<Type>(std::max<Type>(m_x, m_y), m_z), m_w);
	}

	/*!< \brief get's the maximum value of the x, y, and z components. */
	Type Max3(void) const {
		return std::max<Type>(std::max<Type>(m_x, m_y), m_z);
	}

	/*!< \brief get's the maximum value of the x, y component. */
	Type Max2(void) const {
		return std::max<Type>(m_x, m_y);
	}

	/*!< \brief get's the lower value of each component between this and A's components. */
	LWSVector4<Type> Min(const LWSVector4<Type> &A) const {
		return LWSVector4<Type>(std::min<Type>(m_x, A.m_x), std::min<Type>(m_y, A.m_y), std::min<Type>(m_z, A.m_z), std::min<Type>(m_w, A.m_w));
	}

	/*!< \brief get's the higher value of each component between this and A's components. */
	LWSVector4<Type> Max(const LWSVector4<Type> &A) const {
		return LWSVector4<Type>(std::max<Type>(m_x, A.m_x), std::max<Type>(m_y, A.m_y), std::max<Type>(m_z, A.m_z), std::max<Type>(m_w, A.m_w));
	}

	/*!< \brief calculates a cross product between two simd 4 vectors, treated as vec3's. */
	LWSVector4<Type> Cross3(const LWSVector4<Type> &O) const {
		return LWSVector4<Type>(m_y*O.m_z - m_z * O.m_y, m_z*O.m_x - m_x * O.m_z, m_x*O.m_y - m_y * O.m_x, m_w);
	}

	/*!< \brief calculates a perpindicular xy components of the simd, treated as vec2's. */
	LWSVector4<Type> Perpindicular2(void) const{
		return LWSVector4<Type>(-m_y, m_x);
	}

	/*!< \brief return's the length of the vector. */
	Type Length(void) const {
		Type L = m_x * m_x + m_y * m_y + m_z * m_z + m_w * m_w;
		if (L < std::numeric_limits<Type>::epsilon()) return (Type)0;
		return (Type)sqrt(L);
	}

	/*!< \brief return's the length of the x, y, and z of the vector. */
	Type Length3(void) const {
		Type L = m_x * m_x + m_y * m_y + m_z * m_z;
		if (L < std::numeric_limits<Type>::epsilon()) return (Type)0;
		return (Type)sqrt(L);
	}

	/*!< \brief return's the length of the x, y of the vector. */
	Type Length2(void) const {
		Type L = m_x * m_x + m_y * m_y;
		if (L < std::numeric_limits<Type>::epsilon()) return (Type)0;
		return (Type)sqrt(L);
	}

	/*!< \brief return's the squared length of the vector. */
	Type LengthSquared(void) const {
		return m_x * m_x + m_y * m_y + m_z * m_z + m_w * m_w;
	}

	/*!< \brief return's the squared length of the x, y, and z of the vector. */
	Type LengthSquared3(void) const {
		return m_x * m_x + m_y * m_y + m_z * m_z;
	}

	/*!< \brief return's the squared length of the x, and y of the vector. */
	Type LengthSquared2(void) const {
		return m_x * m_x + m_y * m_y;
	}

	/*!< \brief return's the distance between this and O vector. */
	Type Distance(const LWSVector4<Type> &O) const {
		return (O - *this).Length();
	}

	/*!< \brief return's the distance between this and O of the x, y, and z components. */
	Type Distance3(const LWSVector4<Type> &O) const {
		return (O - *this).Length3();
	}

	/*!< \brief return's the distance between this and O of the x, and y components. */
	Type Distance2(const LWSVector4<Type> &O) const {
		return (O - *this).Length2();
	}

	/*!< \brief return's the squared distance between this and O of vector. */
	Type DistanceSquared(const LWSVector4<Type> &O) const {
		return (O - *this).LengthSquared();
	}

	/*!< \brief return's the squared distance between this and O of the x, y, and z components. */
	Type DistanceSquared3(const LWSVector4<Type> &O) const {
		return (O - *this).LengthSquared3();
	}

	/*!< \brief return's the squared distance betweent his and O of the x, and y components. */
	Type DistanceSquared2(const LWSVector4<Type> &O) const {
		return (O - *this).LengthSquared2();
	}

	LWSVector4<Type> &operator = (const LWSVector4<Type> &Rhs) {
		m_x = Rhs.m_x;
		m_y = Rhs.m_y;
		m_z = Rhs.m_z;
		m_w = Rhs.m_w;
		return *this;
	};

	LWSVector4<Type> &operator += (const LWSVector4<Type> &Rhs) {
		m_x += Rhs.m_x;
		m_y += Rhs.m_y;
		m_z += Rhs.m_z;
		m_w += Rhs.m_w;
		return *this;
	}

	LWSVector4<Type> &operator += (Type Rhs) {
		m_x += Rhs;
		m_y += Rhs;
		m_z += Rhs;
		m_w += Rhs;
		return *this;
	}

	LWSVector4<Type> &operator -= (const LWSVector4<Type> &Rhs) {
		m_x -= Rhs.m_x;
		m_y -= Rhs.m_y;
		m_z -= Rhs.m_z;
		m_w -= Rhs.m_w;
		return *this;
	}

	LWSVector4<Type> &operator -= (Type Rhs) {
		m_x -= Rhs;
		m_y -= Rhs;
		m_z -= Rhs;
		m_w -= Rhs;
		return *this;
	}

	LWSVector4<Type> &operator *= (const LWSVector4<Type> &Rhs) {
		m_x *= Rhs.m_x;
		m_y *= Rhs.m_y;
		m_z *= Rhs.m_z;
		m_w *= Rhs.m_w;
		return *this;
	}

	LWSVector4<Type> &operator *= (Type Rhs) {
		m_x *= Rhs;
		m_y *= Rhs;
		m_z *= Rhs;
		m_w *= Rhs;
		return *this;
	}

	LWSVector4<Type> &operator /= (const LWSVector4<Type> &Rhs) {
		m_x /= Rhs.m_x;
		m_y /= Rhs.m_y;
		m_z /= Rhs.m_z;
		m_w /= Rhs.m_w;
		return *this;
	}

	LWSVector4<Type> &operator /= (Type Rhs) {
		m_x /= Rhs;
		m_y /= Rhs;
		m_z /= Rhs;
		m_w /= Rhs;
		return *this;
	}

	friend LWSVector4<Type> operator + (const LWSVector4<Type> &Rhs) {
		return Rhs;
	}

	friend LWSVector4<Type> operator - (const LWSVector4<Type> &Rhs) {
		return LWSVector4<Type>(-Rhs.m_x, -Rhs.m_y, -Rhs.m_z, -Rhs.m_w);
	}

	bool operator == (const LWSVector4<Type> &Rhs) const {
		return m_x == Rhs.m_x && m_y == Rhs.m_y && m_z == Rhs.m_z && m_w == Rhs.m_w;
	}

	bool operator != (const LWSVector4<Type> &Rhs) const {
		return !(*this == Rhs);
	}

	friend std::ostream &operator<<(std::ostream &o, const LWSVector4<Type> &v) {
		o << v.m_x << " " << v.m_y << " " << v.m_z << " " << v.m_w;
		return o;
	}

	friend LWSVector4<Type> operator + (const LWSVector4<Type> &Lhs, const LWSVector4<Type> &Rhs) {
		return LWSVector4<Type>(Lhs.m_x + Rhs.m_x, Lhs.m_y + Rhs.m_y, Lhs.m_z + Rhs.m_z, Lhs.m_w + Rhs.m_w);
	}

	friend LWSVector4<Type> operator + (const LWSVector4<Type> &Lhs, Type Rhs) {
		return LWSVector4<Type>(Lhs.m_x + Rhs, Lhs.m_y + Rhs, Lhs.m_z + Rhs, Lhs.m_w + Rhs);
	}

	friend LWSVector4<Type> operator + (Type Lhs, const LWSVector4<Type> &Rhs) {
		return LWSVector4<Type>(Lhs + Rhs.m_x, Lhs + Rhs.m_y, Lhs + Rhs.m_z, Lhs + Rhs.m_w);
	}


	friend LWSVector4<Type> operator - (const LWSVector4<Type> &Lhs, const LWSVector4<Type> &Rhs) {
		return LWSVector4<Type>(Lhs.m_x - Rhs.m_x, Lhs.m_y - Rhs.m_y, Lhs.m_z - Rhs.m_z, Lhs.m_w - Rhs.m_w);
	}

	friend LWSVector4<Type> operator - (const LWSVector4<Type> &Lhs, Type Rhs) {
		return LWSVector4<Type>(Lhs.m_x - Rhs, Lhs.m_y - Rhs, Lhs.m_z - Rhs, Lhs.m_w - Rhs);
	}

	friend LWSVector4<Type> operator - (Type Lhs, const LWSVector4<Type> &Rhs) {
		return LWSVector4<Type>(Lhs - Rhs.m_x, Lhs - Rhs.m_y, Lhs - Rhs.m_z, Lhs - Rhs.m_w);
	}

	friend LWSVector4<Type> operator * (const LWSVector4<Type> &Lhs, const LWSVector4<Type> &Rhs) {
		return LWSVector4<Type>(Lhs.m_x * Rhs.m_x, Lhs.m_y * Rhs.m_y, Lhs.m_z * Rhs.m_z, Lhs.m_w * Rhs.m_w);
	}

	friend LWSVector4<Type> operator * (const LWSVector4<Type> &Lhs, Type Rhs) {
		return LWSVector4<Type>(Lhs.m_x * Rhs, Lhs.m_y * Rhs, Lhs.m_z * Rhs, Lhs.m_w * Rhs);
	}

	friend LWSVector4<Type> operator * (Type Lhs, const LWSVector4<Type> &Rhs) {
		return LWSVector4<Type>(Lhs * Rhs.m_x, Lhs * Rhs.m_y, Lhs * Rhs.m_z, Lhs * Rhs.m_w);
	}

	friend LWSVector4<Type> operator / (const LWSVector4<Type> &Lhs, const LWSVector4<Type> &Rhs) {
		return LWSVector4<Type>(Lhs.m_x / Rhs.m_x, Lhs.m_y / Rhs.m_y, Lhs.m_z / Rhs.m_z, Lhs.m_w / Rhs.m_w);
	}

	friend LWSVector4<Type> operator / (const LWSVector4<Type> &Lhs, Type Rhs) {
		return LWSVector4<Type>(Lhs.m_x / Rhs, Lhs.m_y / Rhs, Lhs.m_z / Rhs, Lhs.m_w / Rhs);
	}

	friend LWSVector4<Type> operator / (Type Lhs, const LWSVector4<Type> &Rhs) {
		return LWSVector4<Type>(Lhs / Rhs.m_x, Lhs / Rhs.m_y, Lhs / Rhs.m_z, Lhs / Rhs.m_w);
	}

	/*!< \brief returns xyz from A, and w from B. */
	LWSVector4<Type> AAAB(const LWSVector4<Type> &B) const {
		return { A.m_x, A.m_y, A.m_z, B.m_w };
	}

	/*!< \brief returns xyw from A(this), and z from B. */
	LWSVector4<Type> AABA(const LWSVector4<Type> &B) const {
		return { m_x, m_y, B.m_z, m_w };
	}

	/*!< \brief returns xy from A(this), and zw from B. */
	LWSVector4<Type> AABB(const LWSVector4<Type> &B) const {
		return { m_x, m_y, B.m_z, B.m_w };
	}

	/*!< \brief returns xzw from A(this), and y from B. */
	LWSVector4<Type> ABAA(const LWSVector4<Type> &B) const {
		return { m_x, B.m_y, m_z, m_w };
	}

	/*!< \brief returns xz from A(this), and yw from B. */
	LWSVector4<Type> ABAB(const LWSVector4<Type> &B) const {
		return { m_x, B.m_y, m_z, B.m_w };
	}

	/*!< \brief returns xw from A(this), and yz from B. */
	LWSVector4<Type> ABBA(const LWSVector4<Type> &B) const {
		return { m_x, B.m_y, B.m_z, m_w };
	}

	/*!< \brief returns x from A(this), and yzw from B. */
	LWSVector4<Type> ABBB(const LWSVector4<Type> &B) const {
		return { m_x, B.m_y, B.m_z, B.m_w };
	}

	/*!< \brief returns yzw from A(this), and x from B. */
	LWSVector4<Type> BAAA(const LWSVector4<Type> &B) const {
		return { B.m_w, m_y, m_z, m_w };
	}

	/*!< \brief returns yz from A(this), and xw from B. */
	LWSVector4<Type> BAAB(const LWSVector4<Type> &B) const {
		return { B.m_x, m_y, m_z, B.m_w };
	}

	/*!< \brief returns yw from A(this), and xz from B. */
	LWSVector4<Type> BABA(const LWSVector4<Type> &B) const {
		return { B.m_x, m_y, B.m_z, m_w };
	}

	/*!< \brief returns y from A(this), and xzw from B. */
	LWSVector4<Type> BABB(const LWSVector4<Type> &B) const {
		return { B.m_x, m_y, B.m_z, B.m_w };
	}

	/*!< \brief returns zw from A(this), and xy from B. */
	LWSVector4<Type> BBAA(const LWSVector4<Type> &B) const {
		return { B.m_x, B.m_y, m_z, A.m_w };
	}

	/*!< \brief returns z from A(this), and xyw from B. */
	LWSVector4<Type> BBAB(const LWSVector4<Type> &B) const {
		return { B.m_x, B.m_y, A.m_z, B.m_w };
	}

	/*!< \brief returns w from A(this), and xyz from B. */
	LWSVector4<Type> BBBA(const LWSVector4<Type> &B) const {
		return { B.m_x, B.m_y, B.m_z, A.m_w };
	}

	LWSVector4<Type> xxxx(void) const {
		return LWSVector4<Type>(m_x, m_x, m_x, m_x);
	}

	LWSVector4<Type> xxxy(void) const {
		return LWSVector4<Type>(m_x, m_x, m_x, m_y);
	}
	
	LWSVector4<Type> xxxz(void) const {
		return LWSVector4<Type>(m_x, m_x, m_x, m_z);
	}
	
	LWSVector4<Type> xxxw(void) const {
		return LWSVector4<Type>(m_x, m_x, m_x, m_w);
	}
	
	LWSVector4<Type> xxyx(void) const {
		return LWSVector4<Type>(m_x, m_x, m_y, m_x);
	}

	LWSVector4<Type> xxyy(void) const {
		return LWSVector4<Type>(m_x, m_x, m_y, m_y);
	}

	LWSVector4<Type> xxyz(void) const {
		return LWSVector4<Type>(m_x, m_x, m_y, m_z);
	}

	LWSVector4<Type> xxyw(void) const {
		return LWSVector4<Type>(m_x, m_x, m_y, m_w);
	}

	LWSVector4<Type> xxzx(void) const {
		return LWSVector4<Type>(m_x, m_x, m_z, m_x);
	}

	LWSVector4<Type> xxzy(void) const {
		return LWSVector4<Type>(m_x, m_x, m_z, m_y);
	}

	LWSVector4<Type> xxzz(void) const {
		return LWSVector4<Type>(m_x, m_x, m_z, m_z);
	}

	LWSVector4<Type> xxzw(void) const {
		return LWSVector4<Type>(m_x, m_x, m_z, m_w);
	}

	LWSVector4<Type> xxwx(void) const {
		return LWSVector4<Type>(m_x, m_x, m_w, m_x);
	}

	LWSVector4<Type> xxwy(void) const {
		return LWSVector4<Type>(m_x, m_x, m_w, m_y);
	}

	LWSVector4<Type> xxwz(void) const {
		return LWSVector4<Type>(m_x, m_x, m_w, m_z);
	}

	LWSVector4<Type> xxww(void) const {
		return LWSVector4<Type>(m_x, m_x, m_w, m_w);
	}

	LWSVector4<Type> xyxx(void) const {
		return LWSVector4<Type>(m_x, m_y, m_x, m_x);
	}

	LWSVector4<Type> xyxy(void) const {
		return LWSVector4<Type>(m_x, m_y, m_x, m_y);
	}

	LWSVector4<Type> xyxz(void) const {
		return LWSVector4<Type>(m_x, m_y, m_x, m_z);
	}

	LWSVector4<Type> xyxw(void) const {
		return LWSVector4<Type>(m_x, m_y, m_x, m_w);
	}

	LWSVector4<Type> xyyx(void) const {
		return LWSVector4<Type>(m_x, m_y, m_y, m_x);
	}

	LWSVector4<Type> xyyy(void) const {
		return LWSVector4<Type>(m_x, m_y, m_y, m_y);
	}

	LWSVector4<Type> xyyz(void) const {
		return LWSVector4<Type>(m_x, m_y, m_y, m_z);
	}

	LWSVector4<Type> xyyw(void) const {
		return LWSVector4<Type>(m_x, m_y, m_y, m_w);
	}

	LWSVector4<Type> xyzx(void) const {
		return LWSVector4<Type>(m_x, m_y, m_z, m_x);
	}

	LWSVector4<Type> xyzy(void) const {
		return LWSVector4<Type>(m_x, m_y, m_z, m_y);
	}

	LWSVector4<Type> xyzz(void) const {
		return LWSVector4<Type>(m_x, m_y, m_z, m_z);
	}

	LWSVector4<Type> xywx(void) const {
		return LWSVector4<Type>(m_x, m_y, m_w, m_x);
	}

	LWSVector4<Type> xywy(void) const {
		return LWSVector4<Type>(m_x, m_y, m_w, m_y);
	}

	LWSVector4<Type> xywz(void) const {
		return LWSVector4<Type>(m_x, m_y, m_w, m_z);
	}

	LWSVector4<Type> xyww(void) const {
		return LWSVector4<Type>(m_x, m_y, m_w, m_w);
	}

	LWSVector4<Type> xzxx(void) const {
		return LWSVector4<Type>(m_x, m_z, m_x, m_x);
	}

	LWSVector4<Type> xzxy(void) const {
		return LWSVector4<Type>(m_x, m_z, m_x, m_y);
	}

	LWSVector4<Type> xzxz(void) const {
		return LWSVector4<Type>(m_x, m_z, m_x, m_z);
	}

	LWSVector4<Type> xzxw(void) const {
		return LWSVector4<Type>(m_x, m_z, m_x, m_w);
	}

	LWSVector4<Type> xzyx(void) const {
		return LWSVector4<Type>(m_x, m_z, m_y, m_x);
	}

	LWSVector4<Type> xzyy(void) const {
		return LWSVector4<Type>(m_x, m_z, m_y, m_y);
	}

	LWSVector4<Type> xzyz(void) const {
		return LWSVector4<Type>(m_x, m_z, m_y, m_z);
	}

	LWSVector4<Type> xzyw(void) const {
		return LWSVector4<Type>(m_x, m_z, m_y, m_w);
	}

	LWSVector4<Type> xzzx(void) const {
		return LWSVector4<Type>(m_x, m_z, m_z, m_x);
	}

	LWSVector4<Type> xzzy(void) const {
		return LWSVector4<Type>(m_x, m_z, m_z, m_y);
	}

	LWSVector4<Type> xzzz(void) const {
		return LWSVector4<Type>(m_x, m_z, m_z, m_z);
	}

	LWSVector4<Type> xzzw(void) const {
		return LWSVector4<Type>(m_x, m_z, m_z, m_w);
	}

	LWSVector4<Type> xzwx(void) const {
		return LWSVector4<Type>(m_x, m_z, m_w, m_x);
	}

	LWSVector4<Type> xzwy(void) const {
		return LWSVector4<Type>(m_x, m_z, m_w, m_y);
	}

	LWSVector4<Type> xzwz(void) const {
		return LWSVector4<Type>(m_x, m_z, m_w, m_z);
	}

	LWSVector4<Type> xzww(void) const {
		return LWSVector4<Type>(m_x, m_z, m_w, m_w);
	}

	LWSVector4<Type> xwxx(void) const {
		return LWSVector4<Type>(m_x, m_w, m_x, m_x);
	}

	LWSVector4<Type> xwxy(void) const {
		return LWSVector4<Type>(m_x, m_w, m_x, m_y);
	}

	LWSVector4<Type> xwxz(void) const {
		return LWSVector4<Type>(m_x, m_w, m_x, m_z);
	}

	LWSVector4<Type> xwxw(void) const {
		return LWSVector4<Type>(m_x, m_w, m_x, m_w);
	}

	LWSVector4<Type> xwyx(void) const {
		return LWSVector4<Type>(m_x, m_w, m_y, m_x);
	}

	LWSVector4<Type> xwyy(void) const {
		return LWSVector4<Type>(m_x, m_w, m_y, m_y);
	}

	LWSVector4<Type> xwyz(void) const {
		return LWSVector4<Type>(m_x, m_w, m_y, m_z);
	}

	LWSVector4<Type> xwyw(void) const {
		return LWSVector4<Type>(m_x, m_w, m_y, m_w);
	}

	LWSVector4<Type> xwzx(void) const {
		return LWSVector4<Type>(m_x, m_w, m_z, m_x);
	}

	LWSVector4<Type> xwzy(void) const {
		return LWSVector4<Type>(m_x, m_w, m_z, m_y);
	}

	LWSVector4<Type> xwzz(void) const {
		return LWSVector4<Type>(m_x, m_w, m_z, m_z);
	}

	LWSVector4<Type> xwzw(void) const {
		return LWSVector4<Type>(m_x, m_w, m_z, m_w);
	}

	LWSVector4<Type> xwwx(void) const {
		return LWSVector4<Type>(m_x, m_w, m_w, m_x);
	}

	LWSVector4<Type> xwwy(void) const {
		return LWSVector4<Type>(m_x, m_w, m_w, m_y);
	}

	LWSVector4<Type> xwwz(void) const {
		return LWSVector4<Type>(m_x, m_w, m_w, m_z);
	}

	LWSVector4<Type> xwww(void) const {
		return LWSVector4<Type>(m_x, m_w, m_w, m_w);
	}

	LWSVector4<Type> yxxx(void) const {
		return LWSVector4<Type>(m_y, m_x, m_x, m_x);
	}

	LWSVector4<Type> yxxy(void) const {
		return LWSVector4<Type>(m_y, m_x, m_x, m_y);
	}

	LWSVector4<Type> yxxz(void) const {
		return LWSVector4<Type>(m_y, m_x, m_x, m_z);
	}

	LWSVector4<Type> yxxw(void) const {
		return LWSVector4<Type>(m_y, m_x, m_x, m_w);
	}

	LWSVector4<Type> yxyx(void) const {
		return LWSVector4<Type>(m_y, m_x, m_y, m_x);
	}

	LWSVector4<Type> yxyy(void) const {
		return LWSVector4<Type>(m_y, m_x, m_y, m_y);
	}

	LWSVector4<Type> yxyz(void) const {
		return LWSVector4<Type>(m_y, m_x, m_y, m_z);
	}

	LWSVector4<Type> yxyw(void) const {
		return LWSVector4<Type>(m_y, m_x, m_y, m_w);
	}

	LWSVector4<Type> yxzx(void) const {
		return LWSVector4<Type>(m_y, m_x, m_z, m_x);
	}

	LWSVector4<Type> yxzy(void) const {
		return LWSVector4<Type>(m_y, m_x, m_z, m_y);
	}

	LWSVector4<Type> yxzz(void) const {
		return LWSVector4<Type>(m_y, m_x, m_z, m_z);
	}

	LWSVector4<Type> yxzw(void) const {
		return LWSVector4<Type>(m_y, m_x, m_z, m_w);
	}

	LWSVector4<Type> yxwx(void) const {
		return LWSVector4<Type>(m_y, m_x, m_w, m_x);
	}

	LWSVector4<Type> yxwy(void) const {
		return LWSVector4<Type>(m_y, m_x, m_w, m_y);
	}

	LWSVector4<Type> yxwz(void) const {
		return LWSVector4<Type>(m_y, m_x, m_w, m_z);
	}

	LWSVector4<Type> yxww(void) const {
		return LWSVector4<Type>(m_y, m_x, m_w, m_w);
	}

	LWSVector4<Type> yyxx(void) const {
		return LWSVector4<Type>(m_y, m_y, m_x, m_x);
	}

	LWSVector4<Type> yyxy(void) const {
		return LWSVector4<Type>(m_y, m_y, m_x, m_y);
	}

	LWSVector4<Type> yyxz(void) const {
		return LWSVector4<Type>(m_y, m_y, m_x, m_z);
	}

	LWSVector4<Type> yyxw(void) const {
		return LWSVector4<Type>(m_y, m_y, m_x, m_w);
	}

	LWSVector4<Type> yyyx(void) const {
		return LWSVector4<Type>(m_y, m_y, m_y, m_x);
	}

	LWSVector4<Type> yyyy(void) const {
		return LWSVector4<Type>(m_y, m_y, m_y, m_y);
	}

	LWSVector4<Type> yyyz(void) const {
		return LWSVector4<Type>(m_y, m_y, m_y, m_z);
	}

	LWSVector4<Type> yyyw(void) const {
		return LWSVector4<Type>(m_y, m_y, m_y, m_w);
	}

	LWSVector4<Type> yyzx(void) const {
		return LWSVector4<Type>(m_y, m_y, m_z, m_x);
	}

	LWSVector4<Type> yyzy(void) const {
		return LWSVector4<Type>(m_y, m_y, m_z, m_y);
	}

	LWSVector4<Type> yyzz(void) const {
		return LWSVector4<Type>(m_y, m_y, m_z, m_z);
	}

	LWSVector4<Type> yyzw(void) const {
		return LWSVector4<Type>(m_y, m_y, m_z, m_w);
	}

	LWSVector4<Type> yywx(void) const {
		return LWSVector4<Type>(m_y, m_y, m_w, m_x);
	}

	LWSVector4<Type> yywy(void) const {
		return LWSVector4<Type>(m_y, m_y, m_w, m_y);
	}

	LWSVector4<Type> yywz(void) const {
		return LWSVector4<Type>(m_y, m_y, m_w, m_z);
	}

	LWSVector4<Type> yyww(void) const {
		return LWSVector4<Type>(m_y, m_y, m_w, m_w);
	}

	LWSVector4<Type> yzxx(void) const {
		return LWSVector4<Type>(m_y, m_z, m_x, m_x);
	}

	LWSVector4<Type> yzxy(void) const {
		return LWSVector4<Type>(m_y, m_z, m_x, m_y);
	}

	LWSVector4<Type> yzxz(void) const {
		return LWSVector4<Type>(m_y, m_z, m_x, m_z);
	}

	LWSVector4<Type> yzxw(void) const {
		return LWSVector4<Type>(m_y, m_z, m_x, m_w);
	}

	LWSVector4<Type> yzyx(void) const {
		return LWSVector4<Type>(m_y, m_z, m_y, m_x);
	}

	LWSVector4<Type> yzyy(void) const {
		return LWSVector4<Type>(m_y, m_z, m_y, m_y);
	}

	LWSVector4<Type> yzyz(void) const {
		return LWSVector4<Type>(m_y, m_z, m_y, m_z);
	}

	LWSVector4<Type> yzyw(void) const {
		return LWSVector4<Type>(m_y, m_z, m_y, m_w);
	}

	LWSVector4<Type> yzzx(void) const {
		return LWSVector4<Type>(m_y, m_z, m_z, m_x);
	}

	LWSVector4<Type> yzzy(void) const {
		return LWSVector4<Type>(m_y, m_z, m_z, m_y);
	}

	LWSVector4<Type> yzzz(void) const {
		return LWSVector4<Type>(m_y, m_z, m_z, m_z);
	}

	LWSVector4<Type> yzzw(void) const {
		return LWSVector4<Type>(m_y, m_z, m_z, m_w);
	}

	LWSVector4<Type> yzwx(void) const {
		return LWSVector4<Type>(m_y, m_z, m_w, m_x);
	}

	LWSVector4<Type> yzwy(void) const {
		return LWSVector4<Type>(m_y, m_z, m_w, m_y);
	}

	LWSVector4<Type> yzwz(void) const {
		return LWSVector4<Type>(m_y, m_z, m_w, m_z);
	}

	LWSVector4<Type> yzww(void) const {
		return LWSVector4<Type>(m_y, m_z, m_w, m_w);
	}

	LWSVector4<Type> ywxx(void) const {
		return LWSVector4<Type>(m_y, m_w, m_x, m_x);
	}

	LWSVector4<Type> ywxy(void) const {
		return LWSVector4<Type>(m_y, m_w, m_x, m_y);
	}

	LWSVector4<Type> ywxz(void) const {
		return LWSVector4<Type>(m_y, m_w, m_x, m_z);
	}

	LWSVector4<Type> ywxw(void) const {
		return LWSVector4<Type>(m_y, m_w, m_x, m_w);
	}

	LWSVector4<Type> ywyx(void) const {
		return LWSVector4<Type>(m_y, m_w, m_y, m_x);
	}

	LWSVector4<Type> ywyy(void) const {
		return LWSVector4<Type>(m_y, m_w, m_y, m_y);
	}

	LWSVector4<Type> ywyz(void) const {
		return LWSVector4<Type>(m_y, m_w, m_y, m_z);
	}

	LWSVector4<Type> ywyw(void) const {
		return LWSVector4<Type>(m_y, m_w, m_y, m_w);
	}

	LWSVector4<Type> ywzx(void) const {
		return LWSVector4<Type>(m_y, m_w, m_z, m_x);
	}

	LWSVector4<Type> ywzy(void) const {
		return LWSVector4<Type>(m_y, m_w, m_z, m_y);
	}

	LWSVector4<Type> ywzz(void) const {
		return LWSVector4<Type>(m_y, m_w, m_z, m_z);
	}

	LWSVector4<Type> ywzw(void) const {
		return LWSVector4<Type>(m_y, m_w, m_z, m_w);
	}

	LWSVector4<Type> ywwx(void) const {
		return LWSVector4<Type>(m_y, m_w, m_w, m_x);
	}

	LWSVector4<Type> ywwy(void) const {
		return LWSVector4<Type>(m_y, m_w, m_w, m_y);
	}

	LWSVector4<Type> ywwz(void) const {
		return LWSVector4<Type>(m_y, m_w, m_w, m_z);
	}

	LWSVector4<Type> ywww(void) const {
		return LWSVector4<Type>(m_y, m_w, m_w, m_w);
	}

	LWSVector4<Type> zxxx(void) const {
		return LWSVector4<Type>(m_z, m_x, m_x, m_x);
	}

	LWSVector4<Type> zxxy(void) const {
		return LWSVector4<Type>(m_z, m_x, m_x, m_y);
	}

	LWSVector4<Type> zxxz(void) const {
		return LWSVector4<Type>(m_z, m_x, m_x, m_z);
	}

	LWSVector4<Type> zxxw(void) const {
		return LWSVector4<Type>(m_z, m_x, m_x, m_w);
	}

	LWSVector4<Type> zxyx(void) const {
		return LWSVector4<Type>(m_z, m_x, m_y, m_x);
	}

	LWSVector4<Type> zxyy(void) const {
		return LWSVector4<Type>(m_z, m_x, m_y, m_y);
	}

	LWSVector4<Type> zxyz(void) const {
		return LWSVector4<Type>(m_z, m_x, m_y, m_z);
	}

	LWSVector4<Type> zxyw(void) const {
		return LWSVector4<Type>(m_z, m_x, m_y, m_w);
	}

	LWSVector4<Type> zxzx(void) const {
		return LWSVector4<Type>(m_z, m_x, m_z, m_x);
	}

	LWSVector4<Type> zxzy(void) const {
		return LWSVector4<Type>(m_z, m_x, m_z, m_y);
	}

	LWSVector4<Type> zxzz(void) const {
		return LWSVector4<Type>(m_z, m_x, m_z, m_z);
	}

	LWSVector4<Type> zxzw(void) const {
		return LWSVector4<Type>(m_z, m_x, m_z, m_w);
	}

	LWSVector4<Type> zxwx(void) const {
		return LWSVector4<Type>(m_z, m_x, m_w, m_x);
	}

	LWSVector4<Type> zxwy(void) const {
		return LWSVector4<Type>(m_z, m_x, m_w, m_y);
	}

	LWSVector4<Type> zxwz(void) const {
		return LWSVector4<Type>(m_z, m_x, m_w, m_z);
	}

	LWSVector4<Type> zxww(void) const {
		return LWSVector4<Type>(m_z, m_x, m_w, m_w);
	}

	LWSVector4<Type> zyxx(void) const {
		return LWSVector4<Type>(m_z, m_y, m_x, m_x);
	}

	LWSVector4<Type> zyxy(void) const {
		return LWSVector4<Type>(m_z, m_y, m_x, m_y);
	}

	LWSVector4<Type> zyxz(void) const {
		return LWSVector4<Type>(m_z, m_y, m_x, m_z);
	}

	LWSVector4<Type> zyxw(void) const {
		return LWSVector4<Type>(m_z, m_y, m_x, m_w);
	}

	LWSVector4<Type> zyyx(void) const {
		return LWSVector4<Type>(m_z, m_y, m_y, m_x);
	}

	LWSVector4<Type> zyyy(void) const {
		return LWSVector4<Type>(m_z, m_y, m_y, m_y);
	}

	LWSVector4<Type> zyyz(void) const {
		return LWSVector4<Type>(m_z, m_y, m_y, m_z);
	}

	LWSVector4<Type> zyyw(void) const {
		return LWSVector4<Type>(m_z, m_y, m_y, m_w);
	}

	LWSVector4<Type> zyzx(void) const {
		return LWSVector4<Type>(m_z, m_y, m_z, m_x);
	}

	LWSVector4<Type> zyzy(void) const {
		return LWSVector4<Type>(m_z, m_y, m_z, m_y);
	}

	LWSVector4<Type> zyzz(void) const {
		return LWSVector4<Type>(m_z, m_y, m_z, m_z);
	}

	LWSVector4<Type> zyzw(void) const {
		return LWSVector4<Type>(m_z, m_y, m_z, m_w);
	}

	LWSVector4<Type> zywx(void) const {
		return LWSVector4<Type>(m_z, m_y, m_w, m_x);
	}

	LWSVector4<Type> zywy(void) const {
		return LWSVector4<Type>(m_z, m_y, m_w, m_y);
	}

	LWSVector4<Type> zywz(void) const {
		return LWSVector4<Type>(m_z, m_y, m_w, m_z);
	}

	LWSVector4<Type> zyww(void) const {
		return LWSVector4<Type>(m_z, m_y, m_w, m_w);
	}

	LWSVector4<Type> zzxx(void) const {
		return LWSVector4<Type>(m_z, m_z, m_x, m_x);
	}

	LWSVector4<Type> zzxy(void) const {
		return LWSVector4<Type>(m_z, m_z, m_x, m_y);
	}

	LWSVector4<Type> zzxz(void) const {
		return LWSVector4<Type>(m_z, m_z, m_x, m_z);
	}

	LWSVector4<Type> zzxw(void) const {
		return LWSVector4<Type>(m_z, m_z, m_x, m_w);
	}

	LWSVector4<Type> zzyx(void) const {
		return LWSVector4<Type>(m_z, m_z, m_y, m_x);
	}

	LWSVector4<Type> zzyy(void) const {
		return LWSVector4<Type>(m_z, m_z, m_y, m_y);
	}

	LWSVector4<Type> zzyz(void) const {
		return LWSVector4<Type>(m_z, m_z, m_y, m_z);
	}

	LWSVector4<Type> zzyw(void) const {
		return LWSVector4<Type>(m_z, m_z, m_y, m_w);
	}

	LWSVector4<Type> zzzx(void) const {
		return LWSVector4<Type>(m_z, m_z, m_z, m_x);
	}

	LWSVector4<Type> zzzy(void) const {
		return LWSVector4<Type>(m_z, m_z, m_z, m_y);
	}

	LWSVector4<Type> zzzz(void) const {
		return LWSVector4<Type>(m_z, m_z, m_z, m_z);
	}

	LWSVector4<Type> zzzw(void) const {
		return LWSVector4<Type>(m_z, m_z, m_z, m_w);
	}

	LWSVector4<Type> zzwx(void) const {
		return LWSVector4<Type>(m_z, m_z, m_w, m_x);
	}

	LWSVector4<Type> zzwy(void) const {
		return LWSVector4<Type>(m_z, m_z, m_w, m_y);
	}

	LWSVector4<Type> zzwz(void) const {
		return LWSVector4<Type>(m_z, m_z, m_w, m_z);
	}

	LWSVector4<Type> zzww(void) const {
		return LWSVector4<Type>(m_z, m_z, m_w, m_w);
	}

	LWSVector4<Type> zwxx(void) const {
		return LWSVector4<Type>(m_z, m_w, m_x, m_x);
	}

	LWSVector4<Type> zwxy(void) const {
		return LWSVector4<Type>(m_z, m_w, m_x, m_y);
	}

	LWSVector4<Type> zwxz(void) const {
		return LWSVector4<Type>(m_z, m_w, m_x, m_z);
	}

	LWSVector4<Type> zwxw(void) const {
		return LWSVector4<Type>(m_z, m_w, m_x, m_w);
	}

	LWSVector4<Type> zwyx(void) const {
		return LWSVector4<Type>(m_z, m_w, m_y, m_x);
	}

	LWSVector4<Type> zwyy(void) const {
		return LWSVector4<Type>(m_z, m_w, m_y, m_y);
	}

	LWSVector4<Type> zwyz(void) const {
		return LWSVector4<Type>(m_z, m_w, m_y, m_z);
	}

	LWSVector4<Type> zwyw(void) const {
		return LWSVector4<Type>(m_z, m_w, m_y, m_w);
	}

	LWSVector4<Type> zwzx(void) const {
		return LWSVector4<Type>(m_z, m_w, m_z, m_x);
	}

	LWSVector4<Type> zwzy(void) const {
		return LWSVector4<Type>(m_z, m_w, m_z, m_y);
	}

	LWSVector4<Type> zwzz(void) const {
		return LWSVector4<Type>(m_z, m_w, m_z, m_z);
	}

	LWSVector4<Type> zwzw(void) const {
		return LWSVector4<Type>(m_z, m_w, m_z, m_w);
	}

	LWSVector4<Type> zwwx(void) const {
		return LWSVector4<Type>(m_z, m_w, m_w, m_x);
	}

	LWSVector4<Type> zwwy(void) const {
		return LWSVector4<Type>(m_z, m_w, m_w, m_y);
	}

	LWSVector4<Type> zwwz(void) const {
		return LWSVector4<Type>(m_z, m_w, m_w, m_z);
	}

	LWSVector4<Type> zwww(void) const {
		return LWSVector4<Type>(m_z, m_w, m_w, m_w);
	}

	LWSVector4<Type> wxxx(void) const {
		return LWSVector4<Type>(m_w, m_x, m_x, m_x);
	}

	LWSVector4<Type> wxxy(void) const {
		return LWSVector4<Type>(m_w, m_x, m_x, m_y);
	}

	LWSVector4<Type> wxxz(void) const {
		return LWSVector4<Type>(m_w, m_x, m_x, m_z);
	}

	LWSVector4<Type> wxxw(void) const {
		return LWSVector4<Type>(m_w, m_x, m_x, m_w);
	}

	LWSVector4<Type> wxyx(void) const {
		return LWSVector4<Type>(m_w, m_x, m_y, m_x);
	}

	LWSVector4<Type> wxyy(void) const {
		return LWSVector4<Type>(m_w, m_x, m_y, m_y);
	}

	LWSVector4<Type> wxyz(void) const {
		return LWSVector4<Type>(m_w, m_x, m_y, m_z);
	}

	LWSVector4<Type> wxyw(void) const {
		return LWSVector4<Type>(m_w, m_x, m_y, m_w);
	}

	LWSVector4<Type> wxzx(void) const {
		return LWSVector4<Type>(m_w, m_x, m_z, m_x);
	}

	LWSVector4<Type> wxzy(void) const {
		return LWSVector4<Type>(m_w, m_x, m_z, m_y);
	}

	LWSVector4<Type> wxzz(void) const {
		return LWSVector4<Type>(m_w, m_x, m_z, m_z);
	}

	LWSVector4<Type> wxzw(void) const {
		return LWSVector4<Type>(m_w, m_x, m_z, m_w);
	}

	LWSVector4<Type> wxwx(void) const {
		return LWSVector4<Type>(m_w, m_x, m_w, m_x);
	}

	LWSVector4<Type> wxwy(void) const {
		return LWSVector4<Type>(m_w, m_x, m_w, m_y);
	}

	LWSVector4<Type> wxwz(void) const {
		return LWSVector4<Type>(m_w, m_x, m_w, m_z);
	}

	LWSVector4<Type> wxww(void) const {
		return LWSVector4<Type>(m_w, m_x, m_w, m_w);
	}

	LWSVector4<Type> wyxx(void) const {
		return LWSVector4<Type>(m_w, m_y, m_x, m_x);
	}

	LWSVector4<Type> wyxy(void) const {
		return LWSVector4<Type>(m_w, m_y, m_x, m_y);
	}

	LWSVector4<Type> wyxz(void) const {
		return LWSVector4<Type>(m_w, m_y, m_x, m_z);
	}

	LWSVector4<Type> wyxw(void) const {
		return LWSVector4<Type>(m_w, m_y, m_x, m_w);
	}

	LWSVector4<Type> wyyx(void) const {
		return LWSVector4<Type>(m_w, m_y, m_y, m_x);
	}

	LWSVector4<Type> wyyy(void) const {
		return LWSVector4<Type>(m_w, m_y, m_y, m_y);
	}

	LWSVector4<Type> wyyz(void) const {
		return LWSVector4<Type>(m_w, m_y, m_y, m_z);
	}

	LWSVector4<Type> wyyw(void) const {
		return LWSVector4<Type>(m_w, m_y, m_y, m_w);
	}

	LWSVector4<Type> wyzx(void) const {
		return LWSVector4<Type>(m_w, m_y, m_z, m_x);
	}

	LWSVector4<Type> wyzy(void) const {
		return LWSVector4<Type>(m_w, m_y, m_z, m_y);
	}

	LWSVector4<Type> wyzz(void) const {
		return LWSVector4<Type>(m_w, m_y, m_z, m_z);
	}

	LWSVector4<Type> wyzw(void) const {
		return LWSVector4<Type>(m_w, m_y, m_z, m_w);
	}

	LWSVector4<Type> wywx(void) const {
		return LWSVector4<Type>(m_w, m_y, m_w, m_x);
	}

	LWSVector4<Type> wywy(void) const {
		return LWSVector4<Type>(m_w, m_y, m_w, m_y);
	}

	LWSVector4<Type> wywz(void) const {
		return LWSVector4<Type>(m_w, m_y, m_w, m_z);
	}

	LWSVector4<Type> wyww(void) const {
		return LWSVector4<Type>(m_w, m_y, m_w, m_w);
	}

	LWSVector4<Type> wzxx(void) const {
		return LWSVector4<Type>(m_w, m_z, m_x, m_x);
	}

	LWSVector4<Type> wzxy(void) const {
		return LWSVector4<Type>(m_w, m_z, m_x, m_y);
	}

	LWSVector4<Type> wzxz(void) const {
		return LWSVector4<Type>(m_w, m_z, m_x, m_z);
	}

	LWSVector4<Type> wzxw(void) const {
		return LWSVector4<Type>(m_w, m_z, m_x, m_w);
	}

	LWSVector4<Type> wzyx(void) const {
		return LWSVector4<Type>(m_w, m_z, m_y, m_x);
	}

	LWSVector4<Type> wzyy(void) const {
		return LWSVector4<Type>(m_w, m_z, m_y, m_y);
	}

	LWSVector4<Type> wzyz(void) const {
		return LWSVector4<Type>(m_w, m_z, m_y, m_z);
	}

	LWSVector4<Type> wzyw(void) const {
		return LWSVector4<Type>(m_w, m_z, m_y, m_w);
	}

	LWSVector4<Type> wzzx(void) const {
		return LWSVector4<Type>(m_w, m_z, m_z, m_x);
	}

	LWSVector4<Type> wzzy(void) const {
		return LWSVector4<Type>(m_w, m_z, m_z, m_y);
	}

	LWSVector4<Type> wzzz(void) const {
		return LWSVector4<Type>(m_w, m_z, m_z, m_z);
	}

	LWSVector4<Type> wzzw(void) const {
		return LWSVector4<Type>(m_w, m_z, m_z, m_w);
	}

	LWSVector4<Type> wzwx(void) const {
		return LWSVector4<Type>(m_w, m_z, m_w, m_x);
	}

	LWSVector4<Type> wzwy(void) const {
		return LWSVector4<Type>(m_w, m_z, m_w, m_y);
	}

	LWSVector4<Type> wzwz(void) const {
		return LWSVector4<Type>(m_w, m_z, m_w, m_z);
	}

	LWSVector4<Type> wzww(void) const {
		return LWSVector4<Type>(m_w, m_z, m_w, m_w);
	}

	LWSVector4<Type> wwxx(void) const {
		return LWSVector4<Type>(m_w, m_w, m_x, m_x);
	}

	LWSVector4<Type> wwxy(void) const {
		return LWSVector4<Type>(m_w, m_w, m_x, m_y);
	}

	LWSVector4<Type> wwxz(void) const {
		return LWSVector4<Type>(m_w, m_w, m_x, m_z);
	}

	LWSVector4<Type> wwxw(void) const {
		return LWSVector4<Type>(m_w, m_w, m_x, m_w);
	}

	LWSVector4<Type> wwyx(void) const {
		return LWSVector4<Type>(m_w, m_w, m_y, m_x);
	}

	LWSVector4<Type> wwyy(void) const {
		return LWSVector4<Type>(m_w, m_w, m_y, m_y);
	}

	LWSVector4<Type> wwyz(void) const {
		return LWSVector4<Type>(m_w, m_w, m_y, m_z);
	}

	LWSVector4<Type> wwyw(void) const {
		return LWSVector4<Type>(m_w, m_w, m_y, m_w);
	}

	LWSVector4<Type> wwzx(void) const {
		return LWSVector4<Type>(m_w, m_w, m_z, m_x);
	}

	LWSVector4<Type> wwzy(void) const {
		return LWSVector4<Type>(m_w, m_w, m_z, m_y);
	}

	LWSVector4<Type> wwzz(void) const {
		return LWSVector4<Type>(m_w, m_w, m_z, m_z);
	}

	LWSVector4<Type> wwzw(void) const {
		return LWSVector4<Type>(m_w, m_w, m_z, m_w);
	}

	LWSVector4<Type> wwwx(void) const {
		return LWSVector4<Type>(m_w, m_w, m_w, m_x);
	}

	LWSVector4<Type> wwwy(void) const {
		return LWSVector4<Type>(m_w, m_w, m_w, m_y);
	}

	LWSVector4<Type> wwwz(void) const {
		return LWSVector4<Type>(m_w, m_w, m_w, m_z);
	}

	LWSVector4<Type> wwww(void) const {
		return LWSVector4<Type>(m_w, m_w, m_w, m_z);
	}

	LWSVector4<Type> xxx(void) const {
		return LWSVector4<Type>(m_x, m_x, m_x, m_w);
	}

	LWSVector4<Type> xxy(void) const {
		return LWSVector4<Type>(m_x, m_x, m_y, m_w);
	}

	LWSVector4<Type> xxz(void) const {
		return LWSVector4<Type>(m_x, m_x, m_z, m_w);
	}

	LWSVector4<Type> xyx(void) const {
		return LWSVector4<Type>(m_x, m_y, m_x, m_w);
	}

	LWSVector4<Type> xyy(void) const {
		return LWSVector4<Type>(m_x, m_y, m_y, m_w);
	}

	LWSVector4<Type> xyz(void) const {
		return LWSVector4<Type>(m_x, m_y, m_z, m_w);
	}

	LWSVector4<Type> xzx(void) const {
		return LWSVector4<Type>(m_x, m_z, m_x, m_w);
	}

	LWSVector4<Type> xzy(void) const {
		return LWSVector4<Type>(m_x, m_z, m_y, m_w);
	}

	LWSVector4<Type> xzz(void) const {
		return LWSVector4<Type>(m_x, m_z, m_z, m_w);
	}

	LWSVector4<Type> yxx(void) const {
		return LWSVector4<Type>(m_y, m_x, m_x, m_w);
	}

	LWSVector4<Type> yxy(void) const {
		return LWSVector4<Type>(m_y, m_x, m_y, m_w);
	}

	LWSVector4<Type> yxz(void) const {
		return LWSVector4<Type>(m_y, m_x, m_z, m_w);
	}

	LWSVector4<Type> yyx(void) const {
		return LWSVector4<Type>(m_y, m_y, m_x, m_w);
	}

	LWSVector4<Type> yyy(void) const {
		return LWSVector4<Type>(m_y, m_y, m_y, m_w);
	}

	LWSVector4<Type> yyz(void) const {
		return LWSVector4<Type>(m_y, m_y, m_z, m_w);
	}

	LWSVector4<Type> yzx(void) const {
		return LWSVector4<Type>(m_y, m_z, m_x, m_w);
	}

	LWSVector4<Type> yzy(void) const {
		return LWSVector4<Type>(m_y, m_z, m_y, m_w);
	}

	LWSVector4<Type> yzz(void) const {
		return LWSVector4<Type>(m_y, m_z, m_z, m_w);
	}

	LWSVector4<Type> zxx(void) const {
		return LWSVector4<Type>(m_z, m_x, m_x, m_w);
	}

	LWSVector4<Type> zxy(void) const {
		return LWSVector4<Type>(m_z, m_x, m_y, m_w);
	}

	LWSVector4<Type> zxz(void) const {
		return LWSVector4<Type>(m_z, m_x, m_z, m_w);
	}

	LWSVector4<Type> zyx(void) const {
		return LWSVector4<Type>(m_z, m_y, m_x, m_w);
	}

	LWSVector4<Type> zyy(void) const {
		return LWSVector4<Type>(m_z, m_y, m_y, m_w);
	}

	LWSVector4<Type> zyz(void) const {
		return LWSVector4<Type>(m_z, m_y, m_z, m_w);
	}

	LWSVector4<Type> zzx(void) const {
		return LWSVector4<Type>(m_z, m_z, m_x, m_w);
	}

	LWSVector4<Type> zzy(void) const {
		return LWSVector4<Type>(m_z, m_z, m_y, m_w);
	}

	LWSVector4<Type> zzz(void) const {
		return LWSVector4<Type>(m_z, m_z, m_z, m_w);
	}

	LWSVector4<Type> xx(void) const {
		return LWSVector4<Type>(m_x, m_x, m_z, m_w);
	}

	LWSVector4<Type> xy(void) const {
		return LWSVector4<Type>(m_x, m_y, m_z, m_w);
	}

	LWSVector4<Type> yx(void) const {
		return LWSVector4<Type>(m_y, m_x, m_z, m_w);
	}

	LWSVector4<Type> yy(void) const {
		return LWSVector4<Type>(m_y, m_y, m_z, m_w);
	}

	Type x(void) const {
		return m_x;
	}

	Type y(void) const {
		return m_y;
	}

	Type z(void) const {
		return m_z;
	}

	Type w(void) const {
		return m_w;
	}

	LWSVector4(const LWVector4<Type> &vxyzw) : m_x(vxyzw.x), m_y(vxyzw.y), m_z(vxyzw.z), m_w(vxyzw.w) {}

	LWSVector4(const LWVector3<Type> &vxyz, Type vw = 0) : m_x(vxyz.x), m_y(vxyz.y), m_z(vxyz.z), m_w(vw) {}

	LWSVector4(const LWVector2<Type> &vxy, const LWVector2<Type> &zw) : m_x(vxy.x), m_y(vxy.y), m_z(zw.x), m_w(zw.y) {}

	LWSVector4(const LWVector2<Type> &vxy, Type vz = 0, Type vw = 0) : m_x(vxy.x), m_y(vxy.y), m_z(vz), m_w(vw) {}

	LWSVector4(Type vx, Type vy, Type vz, Type vw) : m_x(vx), m_y(vy), m_z(vz), m_w(vw) {}

	LWSVector4(Type f = 0) : m_x(f), m_y(f), m_z(f), m_w(f) {}
};

/*!< \brief float implementation of LWSVector4 */
#ifndef LW_NOAVX
#include "LWCore/LWSVector_AVX_Float.h"
#endif

/*!< \brief int32_t implementation of LWSVector4. */
#ifndef LW_NOAVX
#include "LWCore/LWSVector_AVX_Int.h"
#endif

/*!< \brief double implementation of LWSVector4 */
#ifndef LW_NOAVX2
#include "LWCORE/LWSVector_AVX2_Double.h"
#endif


#endif
