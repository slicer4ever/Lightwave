#ifndef LWQUATERNION_H
#define LWQUATERNION_H
#include <LWCore/LWMath.h>
#include <ostream>

/*!< \brief quaternion class that can be used for rotational representation of data.  (Type should only be either a floating point type.) */
template<class Type>
struct LWQuaternion {
	Type x; /*!< \brief imaginary x component of quaternion */
	Type y; /*!< \brief imaginary y component of quaternion */
	Type z; /*!< \brief imaginary z component of quaternion */
	Type w; /*!< \brief real component of quaternion */

	/*!< \brief constructs a quaternion for the provided yaw, pitch, and roll angles. */
	static LWQuaternion FromEuler(Type Yaw, Type Pitch, Type Roll) {

		Type ci = (Type)cos(Pitch*(Type)0.5);
		Type cj = (Type)cos(Yaw*(Type)0.5);
		Type ch = (Type)cos(Roll*(Type)0.5);
		Type si = (Type)sin(Pitch*(Type)0.5);
		Type sj = (Type)sin(Yaw*(Type)0.5);
		Type sh = (Type)sin(Roll*(Type)0.5);
		Type cc = ci * ch;
		Type cs = ci * sh;
		Type sc = si * ch;
		Type ss = si * sh;
		return LWQuaternion(cj*cc + sj * ss, cj*sc - sj * cs, cj*ss + sj * cc, cj*cs - sj * sc);
	};

	/*!< \brief constructs a quaternion from the provided euler angles(x = Yaw, y = Pitch, z = Roll) */
	static LWQuaternion FromEuler(const LWVector3<Type> &Euler) {
		return FromEuler(Euler.x, Euler.y, Euler.z);
	}

	/*!< \brief constructs a quaternion from the provided normalized axis angles. */
	static LWQuaternion FromAxis(Type xAxis, Type yAxis, Type zAxis, Type Theta) {
		Type s = (Type)sin(Theta*(Type)0.5);
		return LWQuaternion((Type)cos(Theta*(Type)0.5), xAxis*s, yAxis*s, zAxis*s);
	}

	/*!< \brief constructs a quaternion from the provided axis angles(x = xAxis, y = yAxis, z = zAxis, w = Theta) */
	static LWQuaternion FromAxis(const LWVector4<Type> &AxisAngle) {
		return FromAxis(AxisAngle.x, AxisAngle.y, AxisAngle.z, AxisAngle.w);
	}

	/*!< \brief performs a spherical lerp between two quaternions. t is between 0 and 1. */
	static LWQuaternion SLERP(const LWQuaternion<Type> &A, const LWQuaternion<Type> &B, Type t) {

		LWQuaternion<Type> Res = B;
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
	static LWQuaternion NLERP(const LWQuaternion<Type> &A, const LWQuaternion<Type> &B, float t) {
		return (A + (B - A)*t).Normalize();
	}

	/*!< \brief decomposes a quaternion rotation around a specified axis and gives the rotational twist(rotation around that axis), and the rotational swing(perpinduclar to the axis. */
	void Decompose(const LWVector3<Type> &Axis, LWQuaternion<Type> &Twist, LWQuaternion<Type> &Swing) {
		LWVector3<Type> r = LWVector3<Type>(x, y, z);
		LWVector3<Type> p = Axis.Project(r);
		float l = w * w + p.x*p.x + p.y*p.y + p.z*p.z;
		if (l <= std::numeric_limits<float>::epsilon()) Twist = LWQuaternion();
		else Twist = LWQuaternion<Type>(w, p.x, p.y, p.z).Normalize();
		Swing = *this*Twist.Conjugate();
		return;
	}

	/*1< \brief converts the quaternion into euler angles in (yaw, pitch, roll order). */
	LWVector3<Type> ToEuler(void) const {
		Type sqw = w * w;
		Type sqx = x * x;
		Type sqy = y * y;
		Type sqz = z * z;
		Type LenSq = sqx + sqy + sqz + sqw;
		Type Test = x * y + z * w;
		if (Test > 0.499*LenSq) return LWVector3<Type>((Type)2 * atan2(x, w), 0, LW_PI_2);
		if (Test < -0.499*LenSq) return LWVector3<Type>((Type)-2 * atan2(x, w), 0, -LW_PI_2);
		Type Yaw = (Type)atan2((Type)2 * y*w - (Type)2 * x*z, sqx - sqy - sqz + sqw);
		Type Pitch = (Type)asin((Type)2 * Test / LenSq);
		Type Roll = (Type)atan2((Type)2 * x*w - (Type)2 * y*z, -sqx + sqy - sqz + sqw);

		return LWVector3<Type>(Yaw, Roll, Pitch);
	}

	/*!< \brief normalizes the quaternion to unit length, returns the result without affecting this object. */
	LWQuaternion Normalize(void) const {
		Type L = x*x + y*y + z*z + w*w;
		if (L < std::numeric_limits<Type>::epsilon()) L = (Type)0;
		else L = (Type)(1/sqrt(L));
		return *this*L;
	}

	/*!< \brief Normalizes the quaternion to unit length, storing the result in Result. */
	void Normalize(LWQuaternion &Result) const {
		Result = Normalize();
	}

	/*!< \brief returns the Length squared of this quaternion. */
	Type LengthSq(void) const {
		return x*x + y*y + z*z + w*w;
	}

	/*!< \brief returns the length of this quaternion. */
	Type Length(void) const {
		return (Type)sqrt(x*x + y*y + z*z + w*w);
	}

	/*!< \brief returns the dot product between the two quaternions. */
	Type Dot(const LWQuaternion &O) const{
		return x*O.x + y*O.y + z*O.z + w*O.w;
	}

	/*!< \brief returns the conjugate of the quaternion. */
	LWQuaternion Conjugate(void) const{
		return LWQuaternion(w, -x, -y, -z);
	}

	/*!< \brief writes into result the conjugate of this quaternion. */
	void Conjugate(LWQuaternion &Result) {
		Result = Conjugate();
		return;
	}

	/*!< \brief returns the inverse of the quaternion. */
	LWQuaternion Inverse(void) const {
		float iLenSq = 1.0f / LengthSq();
		return LWQuaternion(w*iLenSq, -x*iLenSq, -y*iLenSq, -z*iLenSq);
	}

	/*!< \brief writes into result the inverse of the this quaternion. */
	void Inverse(LWQuaternion &Result) {
		Result = Inverse();
		return;
	}

	template<class VecType>
	LWVector2<VecType> RotatePoint(const LWVector2<VecType> Pnt) const {
		LWVector3<VecType> u = LWVector3<VecType>(x, y, z);
		LWVector3<VecType> v = LWVector3<VecType>(Pnt.x,Pnt.y, 0);
		float dA = u.Dot(Pnt);
		float dB = u.Dot(u);
		LWVector3<VecType> r = (VecType)2 * dA*u + (w*w - dB)*Pnt + (VecType)2 * w*u.Cross(Pnt);
		return LWVector2<VecType>(r.x, r.y);
	}

	template<class VecType>
	LWVector3<VecType> RotatePoint(const LWVector3<VecType> Pnt) const {
		LWVector3<VecType> u = LWVector3<VecType>(x, y, z);
		LWVector3<VecType> v = Pnt;
		float dA = u.Dot(Pnt);
		float dB = u.Dot(u);
		return (VecType)2 * dA*u + (w*w - dB)*Pnt + (VecType)2 * w*u.Cross(Pnt);
	}

	template<class VecType>
	LWVector4<VecType> RotatePoint(const LWVector4<VecType> Pnt) const {
		LWVector3<VecType> u = LWVector3<VecType>(x, y, z);
		LWVector3<VecType> v = LWVector3<VecType>(Pnt.x, Pnt.y, Pnt.z);
		float dA = u.Dot(Pnt);
		float dB = u.Dot(u);
		return LWVector4<VecType>((VecType)2 * dA*u + (w*w - dB)*Pnt + (VecType)2 * w*u.Cross(Pnt), Pnt.w);
	}

	bool operator == (const LWQuaternion<Type> &Rhs) const {
		return x == Rhs.x && y == Rhs.y && z==Rhs.z && w==Rhs.w;
	}

	bool operator != (const LWQuaternion<Type> &Rhs) const {
		return !(*this == Rhs);
	}

	LWQuaternion operator+(const LWQuaternion &rhs) const {
		return LWQuaternion(w + rhs.w, x + rhs.x, y + rhs.y, z + rhs.z);
	}

	LWQuaternion operator-(const LWQuaternion &rhs) const {
		return LWQuaternion(w - rhs.w, x - rhs.x, y - rhs.y, z - rhs.z);
	}

	LWQuaternion operator*(const LWQuaternion &rhs) const {
		return LWQuaternion(
			(w*rhs.w) - (x*rhs.x) - (y*rhs.y) - (z*rhs.z),
			(w*rhs.x) + (x*rhs.w) + (y*rhs.z) - (z*rhs.y),
			(w*rhs.y) + (y*rhs.w) + (z*rhs.x) - (x*rhs.z),
			(w*rhs.z) + (z*rhs.w) + (x*rhs.y) - (y*rhs.x));
	}

	LWQuaternion operator*(Type rhs) const {
		return LWQuaternion(w*rhs, x*rhs, y*rhs, z*rhs);
	}


	friend std::ostream &operator<<(std::ostream &o, const LWQuaternion<Type> &q) {
		o << q.w << " " << q.x << " " << q.y << " " << q.z;
		return o;
	}

	LWQuaternion &operator*=(const LWQuaternion &rhs) {
		*this = (*this*rhs);
		return *this;
	}

	LWQuaternion &operator*=(Type rhs) const {
		w *= rhs;
		x *= rhs;
		y *= rhs;
		z *= rhs;
		return *this;
	}

	LWQuaternion &operator+=(const LWQuaternion &rhs) {
		w += rhs.w;
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
		return *this;
	}

	LWQuaternion &operator-=(const LWQuaternion &rhs) {
		w -= rhs.w;
		x -= rhs.x;
		y -= rhs.y;
		z -= rhs.z;
		return *this;
	}

	LWQuaternion &operator=(const LWQuaternion &rhs) {
		w = rhs.w;
		x = rhs.x;
		y = rhs.y;
		z = rhs.z;
		return *this;
	}

	LWQuaternion operator-() const {
		return LWQuaternion(-w, -x, -y, -z);
	}

	LWQuaternion(Type w, Type x, Type y, Type z) : w(w), x(x), y(y), z(z){}

	LWQuaternion(const LWMatrix3<Type> &Mat) {
		Type tr = Mat.m_Rows[0].x + Mat.m_Rows[1].y + Mat.m_Rows[2].z;
		if (tr > (Type)0) {
			Type s = (Type)sqrt(tr + (Type)1.0)*(Type)2.0;
			Type iS = (Type)1 / s;
			*this = LWQuaternion((Type)0.25*s, (Mat.m_Rows[2].y - Mat.m_Rows[1].z)*iS, (Mat.m_Rows[0].z - Mat.m_Rows[2].x)*iS, (Mat.m_Rows[1].x - Mat.m_Rows[0].y)*iS).Normalize();
			return;
		} else if (Mat.m_Rows[0].x > Mat.m_Rows[1].y && Mat.m_Rows[0].x > Mat.m_Rows[2].z) {
			Type s = (Type)sqrt(1.0 + Mat.m_Rows[0].x - Mat.m_Rows[1].y - Mat.m_Rows[2].z) * 2;
			Type iS = (Type)1 / s;
			*this = LWQuaternion((Mat.m_Rows[2].y - Mat.m_Rows[1].z)*iS, (Type)0.25*s, (Mat.m_Rows[0].y + Mat.m_Rows[1].x)*iS, (Mat.m_Rows[0].z + Mat.m_Rows[2].x)*iS).Normalize();
			return;
		} else if (Mat.m_Rows[1].y > Mat.m_Rows[2].z) {
			Type s = (Type)sqrt(1.0 + Mat.m_Rows[1].y - Mat.m_Rows[0].x - Mat.m_Rows[2].z) * 2;
			Type iS = (Type)1 / s;
			*this = LWQuaternion((Mat.m_Rows[0].z - Mat.m_Rows[2].x)*iS, (Mat.m_Rows[0].y + Mat.m_Rows[1].x)*iS, (Type)0.25*s , (Mat.m_Rows[1].z + Mat.m_Rows[2].y)*iS).Normalize();
			return;
		}
		Type s = (Type)sqrt(1.0 + Mat.m_Rows[2].z - Mat.m_Rows[0].x - Mat.m_Rows[1].y) * 2;
		Type iS = (Type)1 / s;
		*this = LWQuaternion((Mat.m_Rows[1].x - Mat.m_Rows[0].y)*iS, (Mat.m_Rows[0].z + Mat.m_Rows[2].x)*iS, (Mat.m_Rows[1].z + Mat.m_Rows[2].y)*iS, (Type)0.25*s).Normalize();
	}

	LWQuaternion(const LWMatrix4<Type> &Mat) {
		Type tr = Mat.m_Rows[0].x + Mat.m_Rows[1].y + Mat.m_Rows[2].z;
		if (tr > (Type)0) {
			Type s = (Type)sqrt(tr + (Type)1.0)*(Type)2.0;
			Type iS = (Type)1 / s;
			*this = LWQuaternion((Type)0.25*s, (Mat.m_Rows[2].y - Mat.m_Rows[1].z)*iS, (Mat.m_Rows[0].z - Mat.m_Rows[2].x)*iS, (Mat.m_Rows[1].x - Mat.m_Rows[0].y)*iS).Normalize();
			return;
		} else if (Mat.m_Rows[0].x > Mat.m_Rows[1].y && Mat.m_Rows[0].x > Mat.m_Rows[2].z) {
			Type s = (Type)sqrt(1.0 + Mat.m_Rows[0].x - Mat.m_Rows[1].y - Mat.m_Rows[2].z) * 2;
			Type iS = (Type)1 / s;
			*this = LWQuaternion((Mat.m_Rows[2].y - Mat.m_Rows[1].z)*iS, (Type)0.25*s, (Mat.m_Rows[0].y + Mat.m_Rows[1].x)*iS, (Mat.m_Rows[0].z + Mat.m_Rows[2].x)*iS).Normalize();
			return;
		} else if (Mat.m_Rows[1].y > Mat.m_Rows[2].z) {
			Type s = (Type)sqrt(1.0 + Mat.m_Rows[1].y - Mat.m_Rows[0].x - Mat.m_Rows[2].z) * 2;
			Type iS = (Type)1 / s;
			*this = LWQuaternion((Mat.m_Rows[0].z - Mat.m_Rows[2].x)*iS, (Mat.m_Rows[0].y + Mat.m_Rows[1].x)*iS, (Type)0.25*s, (Mat.m_Rows[1].z + Mat.m_Rows[2].y)*iS).Normalize();
			return;
		}
		Type s = (Type)sqrt(1.0 + Mat.m_Rows[2].z - Mat.m_Rows[0].x - Mat.m_Rows[1].y) * 2;
		Type iS = (Type)1 / s;
		*this = LWQuaternion((Mat.m_Rows[1].x - Mat.m_Rows[0].y)*iS, (Mat.m_Rows[0].z + Mat.m_Rows[2].x)*iS, (Mat.m_Rows[1].z + Mat.m_Rows[2].y)*iS, (Type)0.25*s).Normalize();
	}

	/*!< \brief constructs a unit identity quaternion. */
	LWQuaternion() : w(1.0f), x(0.0f), y(0.0f), z(0.0f) {}

};


#endif