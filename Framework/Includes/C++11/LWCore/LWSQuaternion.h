<<<<<<< HEAD
#ifndef LWSQUATERNION_H
#define LWSQUATERNION_H
#include <LWCore/LWTypes.h>
#include <LWCore/LWSVector.h>
#include <LWCore/LWQuaternion.h>

/*!< \brief simd accelerated quaternion class, if simd is not enabled then defaults to a generic implementation. 
	 \note the member variables should not be accessed directly, as they are subject to change depending on which instruction set's are enabled.
*/
template<class Type>
struct LWSQuaternion {
	Type m_x; /*!< \brief imaginary x component of quaternion */
	Type m_y; /*!< \brief imaginary y component of quaternion */
	Type m_z; /*!< \brief imaginary z component of quaternion */
	Type m_w; /*!< \brief real component of quaternion */

	/*!< \brief constructs a quaternion for the provided yaw, pitch, and roll angles. */
	static LWSQuaternion FromEuler(Type Pitch, Type Yaw, Type Roll) {
		Type c1 = (Type)cos(Yaw * (Type)0.5);
		Type c2 = (Type)cos(Pitch * (Type)0.5);
		Type c3 = (Type)cos(Roll * (Type)0.5);
		Type s1 = (Type)sin(Yaw * (Type)0.5);
		Type s2 = (Type)sin(Pitch * (Type)0.5);
		Type s3 = (Type)sin(Roll * (Type)0.5);
		Type c1c2 = c1 * c2;
		Type s1s2 = s1 * s2;
		return LWSQuaternion(c1c2*c3 - s1s2 * s3, c1c2*s3 + s1s2 * c3, s1*c2*c3 + c1 * s2*s3, c1*s2*c3 - s1 * c2*s3);

	};

	/*!< \brief constructs a quaternion from the provided euler angles(x = Pitch, y = Yaw, z = Roll) */
	static LWSQuaternion FromEuler(const LWVector3<Type> &Euler) {
		return FromEuler(Euler.x, Euler.y, Euler.z);
	}

	/*!< \brief constructs a quaternion from the provided normalized axis angles. */
	static LWSQuaternion FromAxis(Type xAxis, Type yAxis, Type zAxis, Type Theta) {
		Type s = (Type)sin(Theta*(Type)0.5);
		return LWSQuaternion((Type)cos(Theta*(Type)0.5), xAxis*s, yAxis*s, zAxis*s);
	}

	/*!< \brief constructs a quaternion from the provided axis angles(x = xAxis, y = yAxis, z = zAxis, w = Theta) */
	static LWSQuaternion FromAxis(const LWVector4<Type> &AxisAngle) {
		return FromAxis(AxisAngle.x, AxisAngle.y, AxisAngle.z, AxisAngle.w);
	}

	/*!< \brief performs a spherical lerp between two quaternions. t is between 0 and 1. */
	static LWSQuaternion SLERP(const LWSQuaternion<Type> &A, const LWSQuaternion<Type> &B, Type t) {
		LWSQuaternion<Type> Res = B;
		Type D = A.Dot(B);
		if (D < 0) {
			D = -D;
			Res = -Res;
		}
		if (D < 1.0f - std::numeric_limits<Type>::epsilon()) {
			Type Theta = (Type)acos(D);

			Type sT = (Type)sin(Theta);
			sT = 1 / sT;
			Type sA = (Type)sin(Theta*(1 - t));
			Type sB = (Type)sin(Theta*t);
			return ((A*sA) + Res * sB * sT).Normalize();
		}
		return A;
	};

	/*!< \brief performs a linear interpolation between two quaternions, t is between 0 and 1. */
	static LWSQuaternion NLERP(const LWSQuaternion<Type> &A, const LWSQuaternion<Type> &B, Type t) {
		return (A + (B - A)*t).Normalize();
	}

	LWQuaternion<Type> AsQuaternion(void) const {
		return LWQuaternion<Type>(m_w, m_x, m_y, m_z);
	}

	LWSVector4<Type> AsSVec4(void) const {
		return LWSVector4<Type>(m_x, m_y, m_z, m_w);
	};

	/*1< \brief converts the quaternion into euler angles in (Pitch, Yaw, Roll order). */
	LWSVector4<Type> ToEuler(void) const {
		Type sqw = m_w * m_w;
		Type sqx = m_x * m_x;
		Type sqy = m_y * m_y;
		Type sqz = m_z * m_z;
		Type LenSq = sqx + sqy + sqz + sqw;
		Type Test = m_x * m_y + m_z * m_w;
		if (Test > 0.499*LenSq) return LWSVector4<Type>(LW_PI_2, (Type)2 * atan2(m_x, m_w), 0);
		if (Test < -0.499*LenSq) return LWSVector4<Type>(-LW_PI_2, (Type)-2 * atan2(m_x, m_w), 0);

		Type Yaw = (Type)atan2((Type)2 * m_y*m_w - (Type)2 * m_x*m_z, sqx - sqy - sqz + sqw);
		Type Pitch = (Type)asin((Type)2 * Test / LenSq);
		Type Roll = (Type)atan2((Type)2 * m_x*m_w - (Type)2 * m_y*m_z, -sqx + sqy - sqz + sqw);
		return LWSVector4<Type>(Pitch, Yaw, Roll, (Type)0);

	}

	/*!< \brief normalizes the quaternion to unit length, returns the result without affecting this object. */
	LWSQuaternion Normalize(void) const {
		Type L = m_x * m_x + m_y * m_y + m_z * m_z + m_w * m_w;
		if (L < std::numeric_limits<Type>::epsilon()) L = (Type)0;
		else L = (Type)(1 / sqrt(L));
		return *this*L;
	}

	/*!< \brief returns the Length squared of this quaternion. */
	Type LengthSq(void) const {
		return m_x * m_x + m_y * m_y + m_z * m_z + m_w * m_w;
	}

	/*!< \brief returns the length of this quaternion. */
	Type Length(void) const {
		return (Type)sqrt(m_x * m_x + m_y * m_y + m_z * m_z + m_w * m_w);
	}

	/*!< \brief returns the dot product between the two quaternions. */
	Type Dot(const LWSQuaternion &O) const {
		return m_x * O.m_x + m_y * O.m_y + m_z * O.m_z + m_w * O.m_w;
	}

	/*!< \brief returns the conjugate of the quaternion. */
	LWSQuaternion Conjugate(void) const {
		return LWSQuaternion(m_w, -m_x, -m_y, -m_z);
	}

	/*!< \brief returns the inverse of the quaternion. */
	LWSQuaternion Inverse(void) const {
		Type iLenSq = 1.0 / LengthSq();
		return LWSQuaternion(m_w, -m_x, -m_y, -m_z) * iLenSq;
	}

	LWSVector4<Type> RotatePoint(const LWSVector4<Type> Pnt) const {
		LWSVector4<Type> u = LWSVector4<Type>(m_x, m_y, m_z, 0.0f);
		float dA = u.Dot3(Pnt);
		float dB = u.LengthSquared();
		LWSVector4<Type> r = LWSVector4<Type>((Type)2 * dA*u + (m_w*m_w - dB)*Pnt + 2 * m_w*u.Cross3(Pnt));
		r = r * LWSVector4<Type>(1.0, 1.0, 1.0, 0.0) + Pnt * LWSVector4<Type>(0.0, 0.0, 0.0, 1.0);
		return r;
	}

	bool operator == (const LWSQuaternion<Type> &Rhs) const {
		const Type e = std::numeric_limits<Type>::epsilon();
		return (Type)abs(m_x-Rhs.m_x)<=e && (Type)abs(m_y-Rhs.m_y)<=e && (Type)abs(m_z-Rhs.m_z)<=e && (Type)abs(m_w-Rhs.m_w)<=e;
	}

	bool operator != (const LWSQuaternion<Type> &Rhs) const {
		return !(*this == Rhs);
	}

	LWSQuaternion operator+(const LWSQuaternion &rhs) const {
		return LWSQuaternion(m_w + rhs.m_w, m_x + rhs.m_x, m_y + rhs.m_y, m_z + rhs.m_z);
	}

	LWSQuaternion operator-(const LWSQuaternion &rhs) const {
		return LWSQuaternion(m_w - rhs.m_w, m_x - rhs.m_x, m_y - rhs.m_y, m_z - rhs.m_z);
	}

	LWSQuaternion operator*(const LWSQuaternion &rhs) const {
		return LWSQuaternion(
			(m_w*rhs.m_w) - (m_x*rhs.m_x) - (m_y*rhs.m_y) - (m_z*rhs.m_z),
			(m_w*rhs.m_x) + (m_x*rhs.m_w) + (m_y*rhs.m_z) - (m_z*rhs.m_y),
			(m_w*rhs.m_y) + (m_y*rhs.m_w) + (m_z*rhs.m_x) - (m_x*rhs.m_z),
			(m_w*rhs.m_z) + (m_z*rhs.m_w) + (m_x*rhs.m_y) - (m_y*rhs.m_x));
	}

	LWSQuaternion operator*(Type rhs) const {
		return LWSQuaternion(m_w*rhs, m_x*rhs, m_y*rhs, m_z*rhs);
	}

	friend LWSQuaternion operator * (Type Lhs, const LWSQuaternion &Rhs) {
		return LWSQuaternion(Lhs * Rhs.m_w, Lhs * Rhs.m_x, Lhs * Rhs.m_y, Lhs * Rhs.m_z);
	}

	friend LWSQuaternion operator + (Type Lhs, const LWSQuaternion &Rhs) {
		return LWSQuaternion(Lhs + Rhs.m_w, Lhs + Rhs.m_x, Lhs + Rhs.m_y, Lhs + Rhs.m_z);
	}

	friend LWSQuaternion operator - (Type Lhs, const LWSQuaternion &Rhs) {
		return LWSQuaternion(Lhs - Rhs.m_w, Lhs - Rhs.m_x, Lhs - Rhs.m_y, Lhs - Rhs.m_z);
	}

	friend LWSQuaternion operator / (Type Lhs, const LWSQuaternion &Rhs) {
		return LWSQuaternion(Lhs / Rhs.m_w, Lhs / Rhs.m_x, Lhs / Rhs.m_y, Lhs / Rhs.m_z);
	}

	friend std::ostream &operator<<(std::ostream &o, const LWSQuaternion<Type> &q) {
		o << q.m_w << " " << q.m_x << " " << q.m_y << " " << q.m_z;
		return o;
	}

	LWSQuaternion &operator*=(const LWSQuaternion &rhs) {
		*this = (*this*rhs);
		return *this;
	}

	LWSQuaternion &operator*=(Type rhs) const {
		m_w *= rhs;
		m_x *= rhs;
		m_y *= rhs;
		m_z *= rhs;
		return *this;
	}

	LWSQuaternion &operator+=(const LWSQuaternion &rhs) {
		m_w += rhs.w;
		m_x += rhs.x;
		m_y += rhs.y;
		m_z += rhs.z;
		return *this;
	}

	LWSQuaternion &operator-=(const LWSQuaternion &rhs) {
		m_w -= rhs.m_w;
		m_x -= rhs.m_x;
		m_y -= rhs.m_y;
		m_z -= rhs.m_z;
		return *this;
	}

	LWSQuaternion &operator=(const LWSQuaternion &rhs) {
		m_w = rhs.m_w;
		m_x = rhs.m_x;
		m_y = rhs.m_y;
		m_z = rhs.m_z;
		return *this;
	}

	LWSQuaternion operator-() const {
		return LWSQuaternion(-m_w, -m_x, -m_y, -m_z);
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

	LWSQuaternion(const LWQuaternion<Type> &Q) : m_w(Q.w), m_x(Q.x), m_y(Q.y), m_z(Q.z) {}

	LWSQuaternion(Type vw, Type vx, Type vy, Type vz) : m_w(vw), m_x(vx), m_y(vy), m_z(vz) {}

	LWSQuaternion(const LWSMatrix4<Type> &Mat) {
		LWVector4<Type> R0 = Mat.m_Rows[0].AsVec4();
		LWVector4<Type> R1 = Mat.m_Rows[1].AsVec4();
		LWVector4<Type> R2 = Mat.m_Rows[2].AsVec4();
		LWVector4<Type> R3 = Mat.m_Rows[3].AsVec4();

		Type tr = R0.x + R1.y + R2.z;
		if (tr > (Type)0) {
			Type s = (Type)sqrt(tr + (Type)1.0)*(Type)2.0;
			Type iS = (Type)1 / s;
			*this = LWSQuaternion((Type)0.25*s, (R2.y - R1.z)*iS, (R0.z - R2.x)*iS, (R1.x - R0.y)*iS).Normalize();
			return;
		}else if(R0.x>R1.y && R0.x>R2.z){

			Type s = (Type)sqrt(1.0 + R0.x - R1.y - R2.z) * 2;
			Type iS = (Type)1 / s;
			*this = LWSQuaternion((R2.y - R1.z)*iS, (Type)0.25*s, (R0.y + R1.x)*iS, (R0.z + R2.x)*iS).Normalize();
			return;
		} else if (R1.y > R2.z) {
			Type s = (Type)sqrt(1.0 + R1.y - R0.x - R2.z) * 2;
			Type iS = (Type)1 / s;
			*this = LWSQuaternion((R0.z - R2.x)*iS, (R0.y + R1.x)*iS, (Type)0.25*s, (R1.z + R2.y)*iS).Normalize();
			return;
		}
		Type s = (Type)sqrt(1.0 + R2.z - R0.x - R1.y) * 2;
		Type iS = (Type)1 / s;
		*this = LWSQuaternion((R1.x - R0.y)*iS, (R0.z + R2.x)*iS, (R1.z + R2.y)*iS, (Type)0.25*s).Normalize();
	}

	/*!< \brief constructs a unit identity quaternion. */
	LWSQuaternion() : m_w((Type)1), m_x((Type)0), m_y((Type)0), m_z((Type)0) {}
};

#ifndef LW_NOAVX
#include "LWCore/LWSQuaternion_AVX_Float.h"
#endif

#ifndef LW_NOAVX2
#include "LWCore/LWSQuaternion_AVX2_Double.h"
#endif

=======
#ifndef LWSQUATERNION_H
#define LWSQUATERNION_H
#include <LWCore/LWTypes.h>
#include <LWCore/LWSVector.h>
#include <LWCore/LWQuaternion.h>

/*!< \brief simd accelerated quaternion class, if simd is not enabled then defaults to a generic implementation. 
	 \note the member variables should not be accessed directly, as they are subject to change depending on which instruction set's are enabled.
*/
template<class Type>
struct LWSQuaternion {
	Type m_x; /*!< \brief imaginary x component of quaternion */
	Type m_y; /*!< \brief imaginary y component of quaternion */
	Type m_z; /*!< \brief imaginary z component of quaternion */
	Type m_w; /*!< \brief real component of quaternion */

	/*!< \brief constructs a quaternion for the provided yaw, pitch, and roll angles. */
	static LWSQuaternion FromEuler(Type Pitch, Type Yaw, Type Roll) {
		Type c1 = (Type)cos(Yaw * (Type)0.5);
		Type c2 = (Type)cos(Pitch * (Type)0.5);
		Type c3 = (Type)cos(Roll * (Type)0.5);
		Type s1 = (Type)sin(Yaw * (Type)0.5);
		Type s2 = (Type)sin(Pitch * (Type)0.5);
		Type s3 = (Type)sin(Roll * (Type)0.5);
		Type c1c2 = c1 * c2;
		Type s1s2 = s1 * s2;
		return LWSQuaternion(c1c2*c3 - s1s2 * s3, c1c2*s3 + s1s2 * c3, s1*c2*c3 + c1 * s2*s3, c1*s2*c3 - s1 * c2*s3);

	};

	/*!< \brief constructs a quaternion from the provided euler angles(x = Pitch, y = Yaw, z = Roll) */
	static LWSQuaternion FromEuler(const LWVector3<Type> &Euler) {
		return FromEuler(Euler.x, Euler.y, Euler.z);
	}

	/*!< \brief constructs a quaternion from the provided normalized axis angles. */
	static LWSQuaternion FromAxis(Type xAxis, Type yAxis, Type zAxis, Type Theta) {
		Type s = (Type)sin(Theta*(Type)0.5);
		return LWSQuaternion((Type)cos(Theta*(Type)0.5), xAxis*s, yAxis*s, zAxis*s);
	}

	/*!< \brief constructs a quaternion from the provided axis angles(x = xAxis, y = yAxis, z = zAxis, w = Theta) */
	static LWSQuaternion FromAxis(const LWVector4<Type> &AxisAngle) {
		return FromAxis(AxisAngle.x, AxisAngle.y, AxisAngle.z, AxisAngle.w);
	}

	/*!< \brief performs a spherical lerp between two quaternions. t is between 0 and 1. */
	static LWSQuaternion SLERP(const LWSQuaternion<Type> &A, const LWSQuaternion<Type> &B, Type t) {
		LWSQuaternion<Type> Res = B;
		Type D = A.Dot(B);
		if (D < 0) {
			D = -D;
			Res = -Res;
		}
		if (D < 1.0f - std::numeric_limits<Type>::epsilon()) {
			Type Theta = (Type)acos(D);

			Type sT = (Type)sin(Theta);
			sT = 1 / sT;
			Type sA = (Type)sin(Theta*(1 - t));
			Type sB = (Type)sin(Theta*t);
			return ((A*sA) + Res * sB * sT).Normalize();
		}
		return A;
	};

	/*!< \brief performs a linear interpolation between two quaternions, t is between 0 and 1. */
	static LWSQuaternion NLERP(const LWSQuaternion<Type> &A, const LWSQuaternion<Type> &B, Type t) {
		return (A + (B - A)*t).Normalize();
	}

	LWQuaternion<Type> AsQuaternion(void) const {
		return LWQuaternion<Type>(m_w, m_x, m_y, m_z);
	}

	LWSVector4<Type> AsSVec4(void) const {
		return LWSVector4<Type>(m_x, m_y, m_z, m_w);
	};

	/*1< \brief converts the quaternion into euler angles in (Pitch, Yaw, Roll order). */
	LWSVector4<Type> ToEuler(void) const {
		Type sqw = m_w * m_w;
		Type sqx = m_x * m_x;
		Type sqy = m_y * m_y;
		Type sqz = m_z * m_z;
		Type LenSq = sqx + sqy + sqz + sqw;
		Type Test = m_x * m_y + m_z * m_w;
		if (Test > 0.499*LenSq) return LWSVector4<Type>(LW_PI_2, (Type)2 * atan2(m_x, m_w), 0);
		if (Test < -0.499*LenSq) return LWSVector4<Type>(-LW_PI_2, (Type)-2 * atan2(m_x, m_w), 0);

		Type Yaw = (Type)atan2((Type)2 * m_y*m_w - (Type)2 * m_x*m_z, sqx - sqy - sqz + sqw);
		Type Pitch = (Type)asin((Type)2 * Test / LenSq);
		Type Roll = (Type)atan2((Type)2 * m_x*m_w - (Type)2 * m_y*m_z, -sqx + sqy - sqz + sqw);
		return LWSVector4<Type>(Pitch, Yaw, Roll, (Type)0);

	}

	/*!< \brief normalizes the quaternion to unit length, returns the result without affecting this object. */
	LWSQuaternion Normalize(void) const {
		Type L = m_x * m_x + m_y * m_y + m_z * m_z + m_w * m_w;
		if (L < std::numeric_limits<Type>::epsilon()) L = (Type)0;
		else L = (Type)(1 / sqrt(L));
		return *this*L;
	}

	/*!< \brief returns the Length squared of this quaternion. */
	Type LengthSq(void) const {
		return m_x * m_x + m_y * m_y + m_z * m_z + m_w * m_w;
	}

	/*!< \brief returns the length of this quaternion. */
	Type Length(void) const {
		return (Type)sqrt(m_x * m_x + m_y * m_y + m_z * m_z + m_w * m_w);
	}

	/*!< \brief returns the dot product between the two quaternions. */
	Type Dot(const LWSQuaternion &O) const {
		return m_x * O.m_x + m_y * O.m_y + m_z * O.m_z + m_w * O.m_w;
	}

	/*!< \brief returns the conjugate of the quaternion. */
	LWSQuaternion Conjugate(void) const {
		return LWSQuaternion(m_w, -m_x, -m_y, -m_z);
	}

	/*!< \brief returns the inverse of the quaternion. */
	LWSQuaternion Inverse(void) const {
		Type iLenSq = 1.0 / LengthSq();
		return LWSQuaternion(m_w, -m_x, -m_y, -m_z) * iLenSq;
	}

	LWSVector4<Type> RotatePoint(const LWSVector4<Type> Pnt) const {
		LWSVector4<Type> u = LWSVector4<Type>(m_x, m_y, m_z, 0.0f);
		float dA = u.Dot3(Pnt);
		float dB = u.LengthSquared();
		LWSVector4<Type> r = LWSVector4<Type>((Type)2 * dA*u + (m_w*m_w - dB)*Pnt + 2 * m_w*u.Cross3(Pnt));
		r = r * LWSVector4<Type>(1.0, 1.0, 1.0, 0.0) + Pnt * LWSVector4<Type>(0.0, 0.0, 0.0, 1.0);
		return r;
	}

	bool operator == (const LWSQuaternion<Type> &Rhs) const {
		const Type e = std::numeric_limits<Type>::epsilon();
		return (Type)abs(m_x-Rhs.m_x)<=e && (Type)abs(m_y-Rhs.m_y)<=e && (Type)abs(m_z-Rhs.m_z)<=e && (Type)abs(m_w-Rhs.m_w)<=e;
	}

	bool operator != (const LWSQuaternion<Type> &Rhs) const {
		return !(*this == Rhs);
	}

	LWSQuaternion operator+(const LWSQuaternion &rhs) const {
		return LWSQuaternion(m_w + rhs.m_w, m_x + rhs.m_x, m_y + rhs.m_y, m_z + rhs.m_z);
	}

	LWSQuaternion operator-(const LWSQuaternion &rhs) const {
		return LWSQuaternion(m_w - rhs.m_w, m_x - rhs.m_x, m_y - rhs.m_y, m_z - rhs.m_z);
	}

	LWSQuaternion operator*(const LWSQuaternion &rhs) const {
		return LWSQuaternion(
			(m_w*rhs.m_w) - (m_x*rhs.m_x) - (m_y*rhs.m_y) - (m_z*rhs.m_z),
			(m_w*rhs.m_x) + (m_x*rhs.m_w) + (m_y*rhs.m_z) - (m_z*rhs.m_y),
			(m_w*rhs.m_y) + (m_y*rhs.m_w) + (m_z*rhs.m_x) - (m_x*rhs.m_z),
			(m_w*rhs.m_z) + (m_z*rhs.m_w) + (m_x*rhs.m_y) - (m_y*rhs.m_x));
	}

	LWSQuaternion operator*(Type rhs) const {
		return LWSQuaternion(m_w*rhs, m_x*rhs, m_y*rhs, m_z*rhs);
	}

	friend LWSQuaternion operator * (Type Lhs, const LWSQuaternion &Rhs) {
		return LWSQuaternion(Lhs * Rhs.m_w, Lhs * Rhs.m_x, Lhs * Rhs.m_y, Lhs * Rhs.m_z);
	}

	friend LWSQuaternion operator + (Type Lhs, const LWSQuaternion &Rhs) {
		return LWSQuaternion(Lhs + Rhs.m_w, Lhs + Rhs.m_x, Lhs + Rhs.m_y, Lhs + Rhs.m_z);
	}

	friend LWSQuaternion operator - (Type Lhs, const LWSQuaternion &Rhs) {
		return LWSQuaternion(Lhs - Rhs.m_w, Lhs - Rhs.m_x, Lhs - Rhs.m_y, Lhs - Rhs.m_z);
	}

	friend LWSQuaternion operator / (Type Lhs, const LWSQuaternion &Rhs) {
		return LWSQuaternion(Lhs / Rhs.m_w, Lhs / Rhs.m_x, Lhs / Rhs.m_y, Lhs / Rhs.m_z);
	}

	friend std::ostream &operator<<(std::ostream &o, const LWSQuaternion<Type> &q) {
		o << q.m_w << " " << q.m_x << " " << q.m_y << " " << q.m_z;
		return o;
	}

	LWSQuaternion &operator*=(const LWSQuaternion &rhs) {
		*this = (*this*rhs);
		return *this;
	}

	LWSQuaternion &operator*=(Type rhs) const {
		m_w *= rhs;
		m_x *= rhs;
		m_y *= rhs;
		m_z *= rhs;
		return *this;
	}

	LWSQuaternion &operator+=(const LWSQuaternion &rhs) {
		m_w += rhs.w;
		m_x += rhs.x;
		m_y += rhs.y;
		m_z += rhs.z;
		return *this;
	}

	LWSQuaternion &operator-=(const LWSQuaternion &rhs) {
		m_w -= rhs.m_w;
		m_x -= rhs.m_x;
		m_y -= rhs.m_y;
		m_z -= rhs.m_z;
		return *this;
	}

	LWSQuaternion &operator=(const LWSQuaternion &rhs) {
		m_w = rhs.m_w;
		m_x = rhs.m_x;
		m_y = rhs.m_y;
		m_z = rhs.m_z;
		return *this;
	}

	LWSQuaternion operator-() const {
		return LWSQuaternion(-m_w, -m_x, -m_y, -m_z);
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

	LWSQuaternion(const LWQuaternion<Type> &Q) : m_w(Q.w), m_x(Q.x), m_y(Q.y), m_z(Q.z) {}

	LWSQuaternion(Type vw, Type vx, Type vy, Type vz) : m_w(vw), m_x(vx), m_y(vy), m_z(vz) {}

	LWSQuaternion(const LWSMatrix4<Type> &Mat) {
		LWVector4<Type> R0 = Mat.m_Rows[0].AsVec4();
		LWVector4<Type> R1 = Mat.m_Rows[1].AsVec4();
		LWVector4<Type> R2 = Mat.m_Rows[2].AsVec4();
		LWVector4<Type> R3 = Mat.m_Rows[3].AsVec4();

		Type tr = R0.x + R1.y + R2.z;
		if (tr > (Type)0) {
			Type s = (Type)sqrt(tr + (Type)1.0)*(Type)2.0;
			Type iS = (Type)1 / s;
			*this = LWSQuaternion((Type)0.25*s, (R2.y - R1.z)*iS, (R0.z - R2.x)*iS, (R1.x - R0.y)*iS).Normalize();
			return;
		}else if(R0.x>R1.y && R0.x>R2.z){

			Type s = (Type)sqrt(1.0 + R0.x - R1.y - R2.z) * 2;
			Type iS = (Type)1 / s;
			*this = LWSQuaternion((R2.y - R1.z)*iS, (Type)0.25*s, (R0.y + R1.x)*iS, (R0.z + R2.x)*iS).Normalize();
			return;
		} else if (R1.y > R2.z) {
			Type s = (Type)sqrt(1.0 + R1.y - R0.x - R2.z) * 2;
			Type iS = (Type)1 / s;
			*this = LWSQuaternion((R0.z - R2.x)*iS, (R0.y + R1.x)*iS, (Type)0.25*s, (R1.z + R2.y)*iS).Normalize();
			return;
		}
		Type s = (Type)sqrt(1.0 + R2.z - R0.x - R1.y) * 2;
		Type iS = (Type)1 / s;
		*this = LWSQuaternion((R1.x - R0.y)*iS, (R0.z + R2.x)*iS, (R1.z + R2.y)*iS, (Type)0.25*s).Normalize();
	}

	/*!< \brief constructs a unit identity quaternion. */
	LWSQuaternion() : m_w((Type)1), m_x((Type)0), m_y((Type)0), m_z((Type)0) {}
};

#ifndef LW_NOAVX
#include "LWCore/LWSQuaternion_AVX_Float.h"
#endif

#ifndef LW_NOAVX2
#include "LWCore/LWSQuaternion_AVX2_Double.h"
#endif

>>>>>>> 22c61301ca6da3efa2e8f518c9b87dbddd76d08b
#endif