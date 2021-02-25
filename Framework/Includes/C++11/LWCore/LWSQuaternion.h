#ifndef LWSQUATERNION_H
#define LWSQUATERNION_H
#include <LWCore/LWTypes.h>
#include <LWCore/LWSVector.h>
#include <LWCore/LWQuaternion.h>

/*!< \brief simd accelerated quaternion class, if simd is not enabled then defaults to a generic implementation. 
	 \note the member variables should not be accessed directly, as they are subject to change depending on which instruction set's are enabled.
*/
template<class Type>
struct alignas(Type[4]) LWSQuaternion {
	Type x; /*!< \brief imaginary x component of quaternion */
	Type y; /*!< \brief imaginary y component of quaternion */
	Type z; /*!< \brief imaginary z component of quaternion */
	Type w; /*!< \brief real component of quaternion */

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
		return LWQuaternion<Type>(w, x, y, z);
	}

	LWSVector4<Type> AsSVec4(void) const {
		return LWSVector4<Type>(x, y, z, w);
	};

	/*1< \brief converts the quaternion into euler angles in (Pitch, Yaw, Roll order). */
	LWSVector4<Type> ToEuler(void) const {
		Type sqw = w * w;
		Type sqx = x * x;
		Type sqy = y * y;
		Type sqz = z * z;
		Type LenSq = sqx + sqy + sqz + sqw;
		Type Test = x * y + z * w;
		if (Test > 0.499*LenSq) return LWSVector4<Type>(LW_PI_2, (Type)2 * atan2(x, w), 0);
		if (Test < -0.499*LenSq) return LWSVector4<Type>(-LW_PI_2, (Type)-2 * atan2(x, w), 0);

		Type Yaw = (Type)atan2((Type)2 * y*w - (Type)2 * x*z, sqx - sqy - sqz + sqw);
		Type Pitch = (Type)asin((Type)2 * Test / LenSq);
		Type Roll = (Type)atan2((Type)2 * x*w - (Type)2 * y*z, -sqx + sqy - sqz + sqw);
		return LWSVector4<Type>(Pitch, Yaw, Roll, (Type)0);

	}

	/*!< \brief normalizes the quaternion to unit length, returns the result without affecting this object. */
	LWSQuaternion Normalize(void) const {
		Type L = x * x + y * y + z * z + w * w;
		if (L < std::numeric_limits<Type>::epsilon()) L = (Type)0;
		else L = (Type)(1 / sqrt(L));
		return *this*L;
	}

	/*!< \brief returns the Length squared of this quaternion. */
	Type LengthSq(void) const {
		return x * x + y * y + z * z + w * w;
	}

	/*!< \brief returns the length of this quaternion. */
	Type Length(void) const {
		return (Type)sqrt(x * x + y * y + z * z + w * w);
	}

	/*!< \brief returns the dot product between the two quaternions. */
	Type Dot(const LWSQuaternion &O) const {
		return x * O.x + y * O.y + z * O.z + w * O.w;
	}

	/*!< \brief returns the conjugate of the quaternion. */
	LWSQuaternion Conjugate(void) const {
		return LWSQuaternion(w, -x, -y, -z);
	}

	/*!< \brief returns the inverse of the quaternion. */
	LWSQuaternion Inverse(void) const {
		Type iLenSq = 1.0 / LengthSq();
		return LWSQuaternion(w, -x, -y, -z) * iLenSq;
	}

	LWSVector4<Type> RotatePoint(const LWSVector4<Type> Pnt) const {
		LWSVector4<Type> u = LWSVector4<Type>(x, y, z, 0.0f);
		float dA = u.Dot3(Pnt);
		float dB = u.LengthSquared();
		LWSVector4<Type> r = LWSVector4<Type>((Type)2 * dA*u + (w*w - dB)*Pnt + 2 * w*u.Cross3(Pnt));
		r = r * LWSVector4<Type>(1.0, 1.0, 1.0, 0.0) + Pnt * LWSVector4<Type>(0.0, 0.0, 0.0, 1.0);
		return r;
	}

	Type operator[](uint32_t i) const {
		return (&x)[i];
	}

	Type &operator[](uint32_t i) {
		return (&x)[i];
	}

	bool operator == (const LWSQuaternion<Type> &Rhs) const {
		const Type e = std::numeric_limits<Type>::epsilon();
		return (Type)abs(x-Rhs.x)<=e && (Type)abs(y-Rhs.y)<=e && (Type)abs(z-Rhs.z)<=e && (Type)abs(w-Rhs.w)<=e;
	}

	bool operator != (const LWSQuaternion<Type> &Rhs) const {
		return !(*this == Rhs);
	}

	LWSQuaternion operator+(const LWSQuaternion &rhs) const {
		return LWSQuaternion(w + rhs.w, x + rhs.x, y + rhs.y, z + rhs.z);
	}

	LWSQuaternion operator-(const LWSQuaternion &rhs) const {
		return LWSQuaternion(w - rhs.w, x - rhs.x, y - rhs.y, z - rhs.z);
	}

	LWSQuaternion operator*(const LWSQuaternion &rhs) const {
		return LWSQuaternion(
			(w*rhs.w) - (x*rhs.x) - (y*rhs.y) - (z*rhs.z),
			(w*rhs.x) + (x*rhs.w) + (y*rhs.z) - (z*rhs.y),
			(w*rhs.y) + (y*rhs.w) + (z*rhs.x) - (x*rhs.z),
			(w*rhs.z) + (z*rhs.w) + (x*rhs.y) - (y*rhs.x));
	}

	LWSQuaternion operator*(Type rhs) const {
		return LWSQuaternion(w*rhs, x*rhs, y*rhs, z*rhs);
	}

	friend LWSQuaternion operator * (Type Lhs, const LWSQuaternion &Rhs) {
		return LWSQuaternion(Lhs * Rhs.w, Lhs * Rhs.x, Lhs * Rhs.y, Lhs * Rhs.z);
	}

	friend LWSQuaternion operator + (Type Lhs, const LWSQuaternion &Rhs) {
		return LWSQuaternion(Lhs + Rhs.w, Lhs + Rhs.x, Lhs + Rhs.y, Lhs + Rhs.z);
	}

	friend LWSQuaternion operator - (Type Lhs, const LWSQuaternion &Rhs) {
		return LWSQuaternion(Lhs - Rhs.w, Lhs - Rhs.x, Lhs - Rhs.y, Lhs - Rhs.z);
	}

	friend LWSQuaternion operator / (Type Lhs, const LWSQuaternion &Rhs) {
		return LWSQuaternion(Lhs / Rhs.w, Lhs / Rhs.x, Lhs / Rhs.y, Lhs / Rhs.z);
	}

	friend std::ostream &operator<<(std::ostream &o, const LWSQuaternion<Type> &q) {
		o << q.w << " " << q.x << " " << q.y << " " << q.z;
		return o;
	}

	LWSQuaternion &operator*=(const LWSQuaternion &rhs) {
		*this = (*this*rhs);
		return *this;
	}

	LWSQuaternion &operator*=(Type rhs) const {
		w *= rhs;
		x *= rhs;
		y *= rhs;
		z *= rhs;
		return *this;
	}

	LWSQuaternion &operator+=(const LWSQuaternion &rhs) {
		w += rhs.w;
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
		return *this;
	}

	LWSQuaternion &operator-=(const LWSQuaternion &rhs) {
		w -= rhs.w;
		x -= rhs.x;
		y -= rhs.y;
		z -= rhs.z;
		return *this;
	}

	LWSQuaternion &operator=(const LWSQuaternion &rhs) {
		w = rhs.w;
		x = rhs.x;
		y = rhs.y;
		z = rhs.z;
		return *this;
	}

	LWSQuaternion operator-() const {
		return LWSQuaternion(-w, -x, -y, -z);
	}

	LWSQuaternion(const LWQuaternion<Type> &Q) : w(Q.w), x(Q.x), y(Q.y), z(Q.z) {}

	LWSQuaternion(Type vw, Type vx, Type vy, Type vz) : w(vw), x(vx), y(vy), z(vz) {}

	LWSQuaternion(const LWSMatrix4<Type> &Mat) {
		LWVector4<Type> R0 = Mat.Rows[0].AsVec4();
		LWVector4<Type> R1 = Mat.Rows[1].AsVec4();
		LWVector4<Type> R2 = Mat.Rows[2].AsVec4();
		LWVector4<Type> R3 = Mat.Rows[3].AsVec4();

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
	LWSQuaternion() : w((Type)1), x((Type)0), y((Type)0), z((Type)0) {}
};

#ifdef __AVX__
#include "LWCore/LWSQuaternion_AVX_Float.h"
#endif

#ifdef __AVX2__
#include "LWCore/LWSQuaternion_AVX2_Double.h"
#endif
#endif