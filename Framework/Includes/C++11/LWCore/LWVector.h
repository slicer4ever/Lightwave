#ifndef LWVECTOR_H
#define LWVECTOR_H
#include <limits>
#include <math.h>
#include "LWCore/LWTypes.h"
#include <ostream>
#include <algorithm>

/*! \addtogroup LWCore LWCore
	@{
*/

/*! \brief A vector4 math utility which allows for numerous mathematical expressions to be expressed quickly and easily.
*/

template<class Type>
struct LWVector4{
	Type x; /*!< \brief x component of the Vector4 */
	Type y; /*!< \brief y component of the Vector4 */
	Type z; /*!< \brief z component of the Vector4 */
	Type w; /*!< \brief w component of the Vector4 */

	/*! \brief returns a simd version of this vector4. */
	LWSVector4<Type> AsSVec4(void) const {
		return LWSVector4<Type>(x, y, z, w);
	}

	/*! \brief returns a copy of the normalized vector4. */
	LWVector4<Type> Normalize(void) const{
		Type L = x*x + y*y + z*z + w*w;
		if (L <= std::numeric_limits<Type>::epsilon()) L = 0;
		else L = (Type)1 / (Type)sqrt(L);
		return LWVector4<Type>(x*L, y*L, z*L, w*L);
	}

	/*! \brief writes into result the normalized of this vector4. 
		\param Result the variable to store the normalized result into.
	*/
	void Normalize(LWVector4<Type> &Result) const {
		Result = Normalize();
		return;
	}

	/*!< \brief returns the min element of all vec4 components. */
	Type Min(void) const {
		return std::min<Type>(std::min<Type>(std::min<Type>(x, y), z), w);
	}

	/*!< \brief returns the min of each component between this vector4 and A vector4 */
	LWVector4<Type> Min(const LWVector4<Type> &A) const {
		return LWVector4<Type>(std::min<Type>(x, A.x), std::min<Type>(y, A.y), std::min<Type>(z, A.z), std::min<Type>(w, A.w));
	}

	/*!< \brief Required to be compatible with LWSVector4 in the case that LW_NOSIMD is specified during compilation. */
	LWVector4<Type> AsVec4(void) const {
		return *this;
	}

	/*!< \brief writes into result the min of each component between this vector4 and A vector4 */
	void Min(const LWVector4<Type> &A, const LWVector4<Type> &Result) const {
		Result = Min(A);
		return;
	}

	/*!< \brief returns the max element of all vec4 components. */
	Type Max(void) const {
		return std::max<Type>(std::max<Type>(std::max<Type>(x, y), z), w);
	}

	/*!< \brief returns the max of each component between this vector4 and A vector4 */
	LWVector4<Type> Max(const LWVector4<Type> &A) const {
		return LWVector4<Type>(std::max<Type>(x, A.x), std::max<Type>(y, A.y), std::max<Type>(z, A.z), std::max<Type>(w, A.w));
	}


	/*!< \brief writes into result the max of each component between this vector4 and A vector4 */
	void Max(const LWVector4<Type> &A, const LWVector4<Type> &Result) const {
		Result = Max(A);
		return;
	}

	/*! \brief Gets the length of the vector4.
		\return the length of the vector4.
	*/
	Type Length(void) const{
		Type L = x*x + y*y + z*z + w*w;
		if (L <= std::numeric_limits<Type>::epsilon()) return 0;
		return (Type)sqrt(L);
	}

	/*! \brief Gets the squared length of the vector4.
		\return the squared length of the vector4.
	*/
	Type LengthSquared(void) const{
		return x*x + y*y + z*z + w*w;
	}

	/*! \brief returns a value of the distance between two vector4's.
		\return the distance between two Vector4's.
	*/
	Type Distance(const LWVector4<Type> &O) const{
		return (*this - O).Length();
	}

	/*! \brief return the squared distance between two vector4's.
		\return the distance between two Vector4's.
	*/
	Type DistanceSquared(const LWVector4<Type> &O) const{
		return (*this - O).LengthSquared();
	}

	/*! \brief returns the dot product between tow vector4's.
	*/
	Type Dot(const LWVector4<Type> &O) const{
		return x*O.x + y*O.y + z*O.z + w*O.w;
	}

	/*! \cond */

	LWVector4<Type> &operator = (const LWVector4<Type> &Rhs){
		x = Rhs.x;
		y = Rhs.y;
		z = Rhs.z;
		w = Rhs.w;
		return *this;
	}

	LWVector4<Type> &operator += (const LWVector4<Type> &Rhs){
		x += Rhs.x;
		y += Rhs.y;
		z += Rhs.z;
		w += Rhs.w;
		return *this;
	}

	LWVector4<Type> &operator += (Type Rhs){
		x += Rhs;
		y += Rhs;
		z += Rhs;
		w += Rhs;
		return *this;
	}

	LWVector4<Type> &operator -= (const LWVector4<Type> &Rhs){
		x -= Rhs.x;
		y -= Rhs.y;
		z -= Rhs.z;
		w -= Rhs.w;
		return *this;
	}

	LWVector4<Type> &operator -= (Type Rhs){
		x -= Rhs;
		y -= Rhs;
		z -= Rhs;
		w -= Rhs;
		return *this;
	}

	LWVector4<Type> &operator *= (const LWVector4<Type> &Rhs){
		x *= Rhs.x;
		y *= Rhs.y;
		z *= Rhs.z;
		w *= Rhs.w;
		return *this;
	}

	LWVector4<Type> &operator *= (Type Rhs){
		x *= Rhs;
		y *= Rhs;
		z *= Rhs;
		w *= Rhs;
		return *this;
	}

	LWVector4<Type> &operator /= (const LWVector4<Type> &Rhs){
		x /= Rhs.x;
		y /= Rhs.y;
		z /= Rhs.z;
		w /= Rhs.w;
		return *this;
	}

	LWVector4<Type> &operator /= (Type Rhs){
		x /= Rhs;
		y /= Rhs;
		z /= Rhs;
		w /= Rhs;
		return *this;
	}

	friend LWVector4<Type> operator + (const LWVector4<Type> &Rhs){
		return LWVector4<Type>(Rhs.x, Rhs.y, Rhs.z, Rhs.w);
	}

	friend LWVector4<Type> operator - (const LWVector4<Type> &Rhs){
		return LWVector4<Type>(-Rhs.x, -Rhs.y, -Rhs.z, -Rhs.w);
	}

	bool operator == (const LWVector4<Type> &Rhs) const{
		const Type e = std::numeric_limits<Type>::epsilon();
		return (Type)abs(x - Rhs.x) <= e && (Type)abs(y - Rhs.y) <= e && (Type)abs(z - Rhs.z) <= e && (Type)abs(w - Rhs.w) <= e;
	}

	bool operator != (const LWVector4<Type> &Rhs) const{
		return !(*this == Rhs);
	}

	friend std::ostream &operator<<(std::ostream &o, const LWVector4<Type> &v) {
		o << v.x << " " << v.y << " " << v.z << " " << v.w;
		return o;
	}

	friend LWVector4<Type> operator + (const LWVector4<Type> &Lhs, const LWVector4<Type> &Rhs){
		return LWVector4<Type>(Lhs.x + Rhs.x, Lhs.y + Rhs.y, Lhs.z + Rhs.z, Lhs.w + Rhs.w);
	}

	friend LWVector4<Type> operator + (const LWVector4<Type> &Lhs, Type Rhs){
		return LWVector4<Type>(Lhs.x + Rhs, Lhs.y + Rhs, Lhs.z + Rhs, Lhs.w + Rhs);
	}

	friend LWVector4<Type> operator + (Type Lhs, const LWVector4<Type> &Rhs){
		return LWVector4<Type>(Lhs + Rhs.x, Lhs + Rhs.y, Lhs + Rhs.z, Lhs + Rhs.w);
	}

	friend LWVector4<Type> operator - (const LWVector4<Type> &Lhs, const LWVector4<Type> &Rhs){
		return LWVector4<Type>(Lhs.x - Rhs.x, Lhs.y - Rhs.y, Lhs.z - Rhs.z, Lhs.w - Rhs.w);
	}

	friend LWVector4<Type> operator - (const LWVector4<Type> &Lhs, Type Rhs){
		return LWVector4<Type>(Lhs.x - Rhs, Lhs.y - Rhs, Lhs.z - Rhs, Lhs.w - Rhs);
	}

	friend LWVector4<Type> operator - (Type Lhs, const LWVector4<Type> &Rhs){
		return LWVector4<Type>(Lhs - Rhs.x, Lhs - Rhs.y, Lhs - Rhs.z, Lhs - Rhs.w);
	}

	friend LWVector4<Type> operator * (const LWVector4<Type> &Lhs, const LWVector4<Type> &Rhs){
		return LWVector4<Type>(Lhs.x * Rhs.x, Lhs.y * Rhs.y, Lhs.z * Rhs.z, Lhs.w * Rhs.w);
	}

	friend LWVector4<Type> operator * (const LWVector4<Type> &Lhs, Type Rhs){
		return LWVector4<Type>(Lhs.x * Rhs, Lhs.y * Rhs, Lhs.z * Rhs, Lhs.w * Rhs);
	}

	friend LWVector4<Type> operator * (Type Lhs, const LWVector4<Type> &Rhs){
		return LWVector4<Type>(Lhs * Rhs.x, Lhs * Rhs.y, Lhs * Rhs.z, Lhs * Rhs.w);
	}

	friend LWVector4<Type> operator / (const LWVector4<Type> &Lhs, const LWVector4<Type> &Rhs){
		return LWVector4<Type>(Lhs.x / Rhs.x, Lhs.y / Rhs.y, Lhs.z / Rhs.z, Lhs.w / Rhs.w);
	}

	friend LWVector4<Type> operator / (const LWVector4<Type> &Lhs, Type Rhs){
		return LWVector4<Type>(Lhs.x / Rhs, Lhs.y / Rhs, Lhs.z / Rhs, Lhs.w / Rhs);
	}

	friend LWVector4<Type> operator / (Type Lhs, const LWVector4<Type> &Rhs){
		return LWVector4<Type>(Lhs / Rhs.x, Lhs / Rhs.y, Lhs / Rhs.z, Lhs / Rhs.w);
	}

	LWVector4<Type> xxxx(void) const {
		return { x,x,x,x };
	}

	LWVector4<Type> xxxy(void) const {
		return { x,x,x,y };
	}

	LWVector4<Type> xxxz(void) const {
		return { x,x,x,z };
	}

	LWVector4<Type> xxxw(void) const {
		return { x,x,x,w };
	}

	LWVector4<Type> xxyx(void) const {
		return { x,x,y,x };
	}

	LWVector4<Type> xxyy(void) const {
		return { x,x,y,y };
	}

	LWVector4<Type> xxyz(void) const {
		return { x,x,y,z };
	}

	LWVector4<Type> xxyw(void) const {
		return { x,x,y,w };
	}

	LWVector4<Type> xxzx(void) const {
		return { x,x,z,x };
	}

	LWVector4<Type> xxzy(void) const {
		return { x,x,z,y };
	}

	LWVector4<Type> xxzz(void) const {
		return { x,x,z,z };
	}

	LWVector4<Type> xxzw(void) const {
		return { x,x,z,w };
	}

	LWVector4<Type> xxwx(void) const {
		return { x,x,w,x };
	}

	LWVector4<Type> xxwy(void) const {
		return { x,x,w,y };
	}

	LWVector4<Type> xxwz(void) const {
		return { x,x,w,z };
	}

	LWVector4<Type> xxww(void) const {
		return { x,x,w,w };
	}

	LWVector4<Type> xyxx(void) const {
		return { x,y,x,x };
	}

	LWVector4<Type> xyxy(void) const {
		return { x,y,x,y };
	}

	LWVector4<Type> xyxz(void) const {
		return { x,y,x,z };
	}

	LWVector4<Type> xyxw(void) const {
		return { x,y,x,w };
	}

	LWVector4<Type> xyyx(void) const {
		return { x,y,y,x };
	}

	LWVector4<Type> xyyy(void) const {
		return { x,y,y,y };
	}

	LWVector4<Type> xyyz(void) const {
		return { x,y,y,z };
	}

	LWVector4<Type> xyyw(void) const {
		return { x,y,y,w };
	}

	LWVector4<Type> xyzx(void) const {
		return { x,y,z,x };
	}

	LWVector4<Type> xyzy(void) const {
		return { x,y,z,y };
	}

	LWVector4<Type> xyzz(void) const {
		return { x,y,z,z };
	}

	LWVector4<Type> xywx(void) const {
		return { x,y,w,x };
	}

	LWVector4<Type> xywy(void) const {
		return { x,y,w,y };
	}

	LWVector4<Type> xywz(void) const {
		return { x,y,w,z };
	}

	LWVector4<Type> xyww(void) const {
		return { x,y,w,w };
	}

	LWVector4<Type> xzxx(void) const {
		return { x,z,x,x };
	}

	LWVector4<Type> xzxy(void) const {
		return { x,z,x,y };
	}

	LWVector4<Type> xzxz(void) const {
		return { x,z,x,z };
	}

	LWVector4<Type> xzxw(void) const {
		return { x,z,x,w };
	}

	LWVector4<Type> xzyx(void) const {
		return { x,z,y,x };
	}

	LWVector4<Type> xzyy(void) const {
		return { x,z,y,y };
	}

	LWVector4<Type> xzyz(void) const {
		return { x,z,y,z };
	}

	LWVector4<Type> xzyw(void) const {
		return { x,z,y,w };
	}

	LWVector4<Type> xzzx(void) const {
		return { x,z,z,x };
	}

	LWVector4<Type> xzzy(void) const {
		return { x,z,z,y };
	}

	LWVector4<Type> xzzz(void) const {
		return { x,z,z,z };
	}

	LWVector4<Type> xzzw(void) const {
		return { x,z,z,w };
	}

	LWVector4<Type> xzwx(void) const {
		return { x,z,w,x };
	}

	LWVector4<Type> xzwy(void) const {
		return { x,z,w,y };
	}

	LWVector4<Type> xzwz(void) const {
		return { x,z,w,z };
	}

	LWVector4<Type> xzww(void) const {
		return { x,z,w,w };
	}

	LWVector4<Type> xwxx(void) const {
		return { x,w,x,x };
	}

	LWVector4<Type> xwxy(void) const {
		return { x,w,x,y };
	}

	LWVector4<Type> xwxz(void) const {
		return { x,w,x,z };
	}

	LWVector4<Type> xwxw(void) const {
		return { x,w,x,w };
	}

	LWVector4<Type> xwyx(void) const {
		return { x,w,y,x };
	}

	LWVector4<Type> xwyy(void) const {
		return { x,w,y,y };
	}

	LWVector4<Type> xwyz(void) const {
		return { x,w,y,z };
	}

	LWVector4<Type> xwyw(void) const {
		return { x,w,y,w };
	}

	LWVector4<Type> xwzx(void) const {
		return { x,w,z,x };
	}

	LWVector4<Type> xwzy(void) const {
		return { x,w,z,y };
	}

	LWVector4<Type> xwzz(void) const {
		return { x,w,z,z };
	}

	LWVector4<Type> xwzw(void) const {
		return { x,w,z,w };
	}

	LWVector4<Type> xwwx(void) const {
		return { x,w,w,x };
	}

	LWVector4<Type> xwwy(void) const {
		return { x,w,w,y };
	}

	LWVector4<Type> xwwz(void) const {
		return { x,w,w,z };
	}

	LWVector4<Type> xwww(void) const {
		return { x,w,w,w };
	}

	LWVector4<Type> yxxx(void) const {
		return { y,x,x,x };
	}

	LWVector4<Type> yxxy(void) const {
		return { y,x,x,y };
	}

	LWVector4<Type> yxxz(void) const {
		return { y,x,x,z };
	}

	LWVector4<Type> yxxw(void) const {
		return { y,x,x,w };
	}

	LWVector4<Type> yxyx(void) const {
		return { y,x,y,x };
	}

	LWVector4<Type> yxyy(void) const {
		return { y,x,y,y };
	}

	LWVector4<Type> yxyz(void) const {
		return { y,x,y,z };
	}

	LWVector4<Type> yxyw(void) const {
		return { y,x,y,w };
	}

	LWVector4<Type> yxzx(void) const {
		return { y,x,z,x };
	}

	LWVector4<Type> yxzy(void) const {
		return { y,x,z,y };
	}

	LWVector4<Type> yxzz(void) const {
		return { y,x,z,z };
	}

	LWVector4<Type> yxzw(void) const {
		return { y,x,z,w };
	}

	LWVector4<Type> yxwx(void) const {
		return { y,x,w,x };
	}

	LWVector4<Type> yxwy(void) const {
		return { y,x,w,y };
	}

	LWVector4<Type> yxwz(void) const {
		return { y,x,w,z };
	}

	LWVector4<Type> yxww(void) const {
		return { y,x,w,w };
	}

	LWVector4<Type> yyxx(void) const {
		return { y,y,x,x };
	}

	LWVector4<Type> yyxy(void) const {
		return { y,y,x,y };
	}

	LWVector4<Type> yyxz(void) const {
		return { y,y,x,z };
	}

	LWVector4<Type> yyxw(void) const {
		return { y,y,x,w };
	}

	LWVector4<Type> yyyx(void) const {
		return { y,y,y,x };
	}

	LWVector4<Type> yyyy(void) const {
		return { y,y,y,y };
	}

	LWVector4<Type> yyyz(void) const {
		return { y,y,y,z };
	}

	LWVector4<Type> yyyw(void) const {
		return { y,y,y,w };
	}

	LWVector4<Type> yyzx(void) const {
		return { y,y,z,x };
	}

	LWVector4<Type> yyzy(void) const {
		return { y,y,z,y };
	}

	LWVector4<Type> yyzz(void) const {
		return { y,y,z,z };
	}

	LWVector4<Type> yyzw(void) const {
		return { y,y,z,w };
	}

	LWVector4<Type> yywx(void) const {
		return { y,y,w,x };
	}

	LWVector4<Type> yywy(void) const {
		return { y,y,w,y };
	}

	LWVector4<Type> yywz(void) const {
		return { y,y,w,z };
	}

	LWVector4<Type> yyww(void) const {
		return { y,y,w,w };
	}

	LWVector4<Type> yzxx(void) const {
		return { y,z,x,x };
	}

	LWVector4<Type> yzxy(void) const {
		return { y,z,x,y };
	}

	LWVector4<Type> yzxz(void) const {
		return { y,z,x,z };
	}

	LWVector4<Type> yzxw(void) const {
		return { y,z,x,w };
	}

	LWVector4<Type> yzyx(void) const {
		return { y,z,y,x };
	}

	LWVector4<Type> yzyy(void) const {
		return { y,z,y,y };
	}

	LWVector4<Type> yzyz(void) const {
		return { y,z,y,z };
	}

	LWVector4<Type> yzyw(void) const {
		return { y,z,y,w };
	}

	LWVector4<Type> yzzx(void) const {
		return { y,z,z,x };
	}

	LWVector4<Type> yzzy(void) const {
		return { y,z,z,y };
	}

	LWVector4<Type> yzzz(void) const {
		return { y,z,z,z };
	}

	LWVector4<Type> yzzw(void) const {
		return { y,z,z,w };
	}

	LWVector4<Type> yzwx(void) const {
		return { y,z,w,x };
	}

	LWVector4<Type> yzwy(void) const {
		return { y,z,w,y };
	}

	LWVector4<Type> yzwz(void) const {
		return { y,z,w,z };
	}

	LWVector4<Type> yzww(void) const {
		return { y,z,w,w };
	}

	LWVector4<Type> ywxx(void) const {
		return { y,w,x,x };
	}

	LWVector4<Type> ywxy(void) const {
		return { y,w,x,y };
	}

	LWVector4<Type> ywxz(void) const {
		return { y,w,x,z };
	}

	LWVector4<Type> ywxw(void) const {
		return { y,w,x,w };
	}

	LWVector4<Type> ywyx(void) const {
		return { y,w,y,x };
	}

	LWVector4<Type> ywyy(void) const {
		return { y,w,y,y };
	}

	LWVector4<Type> ywyz(void) const {
		return { y,w,y,z };
	}

	LWVector4<Type> ywyw(void) const {
		return { y,w,y,w };
	}

	LWVector4<Type> ywzx(void) const {
		return { y,w,z,x };
	}

	LWVector4<Type> ywzy(void) const {
		return { y,w,z,y };
	}

	LWVector4<Type> ywzz(void) const {
		return { y,w,z,z };
	}

	LWVector4<Type> ywzw(void) const {
		return { y,w,z,w };
	}

	LWVector4<Type> ywwx(void) const {
		return { y,w,w,x };
	}

	LWVector4<Type> ywwy(void) const {
		return { y,w,w,y };
	}

	LWVector4<Type> ywwz(void) const {
		return { y,w,w,z };
	}

	LWVector4<Type> ywww(void) const {
		return { y,w,w,w };
	}



	LWVector4<Type> zxxx(void) const {
		return { z,x,x,x };
	}

	LWVector4<Type> zxxy(void) const {
		return { z,x,x,y };
	}

	LWVector4<Type> zxxz(void) const {
		return { z,x,x,z };
	}

	LWVector4<Type> zxxw(void) const {
		return { z,x,x,w };
	}

	LWVector4<Type> zxyx(void) const {
		return { z,x,y,x };
	}

	LWVector4<Type> zxyy(void) const {
		return { z,x,y,y };
	}

	LWVector4<Type> zxyz(void) const {
		return { z,x,y,z };
	}

	LWVector4<Type> zxyw(void) const {
		return { z,x,y,w };
	}

	LWVector4<Type> zxzx(void) const {
		return { z,x,z,x };
	}

	LWVector4<Type> zxzy(void) const {
		return { z,x,z,y };
	}

	LWVector4<Type> zxzz(void) const {
		return { z,x,z,z };
	}

	LWVector4<Type> zxzw(void) const {
		return { z,x,z,w };
	}

	LWVector4<Type> zxwx(void) const {
		return { z,x,w,x };
	}

	LWVector4<Type> zxwy(void) const {
		return { z,x,w,y };
	}

	LWVector4<Type> zxwz(void) const {
		return { z,x,w,z };
	}

	LWVector4<Type> zxww(void) const {
		return { z,x,w,w };
	}

	LWVector4<Type> zyxx(void) const {
		return { z,y,x,x };
	}

	LWVector4<Type> zyxy(void) const {
		return { z,y,x,y };
	}

	LWVector4<Type> zyxz(void) const {
		return { z,y,x,z };
	}

	LWVector4<Type> zyxw(void) const {
		return { z,y,x,w };
	}

	LWVector4<Type> zyyx(void) const {
		return { z,y,y,x };
	}

	LWVector4<Type> zyyy(void) const {
		return { z,y,y,y };
	}

	LWVector4<Type> zyyz(void) const {
		return { z,y,y,z };
	}

	LWVector4<Type> zyyw(void) const {
		return { z,y,y,w };
	}

	LWVector4<Type> zyzx(void) const {
		return { z,y,z,x };
	}

	LWVector4<Type> zyzy(void) const {
		return { z,y,z,y };
	}

	LWVector4<Type> zyzz(void) const {
		return { z,y,z,z };
	}

	LWVector4<Type> zyzw(void) const {
		return { z,y,z,w };
	}

	LWVector4<Type> zywx(void) const {
		return { z,y,w,x };
	}

	LWVector4<Type> zywy(void) const {
		return { z,y,w,y };
	}

	LWVector4<Type> zywz(void) const {
		return { z,y,w,z };
	}

	LWVector4<Type> zyww(void) const {
		return { z,y,w,w };
	}

	LWVector4<Type> zzxx(void) const {
		return { z,z,x,x };
	}

	LWVector4<Type> zzxy(void) const {
		return { z,z,x,y };
	}

	LWVector4<Type> zzxz(void) const {
		return { z,z,x,z };
	}

	LWVector4<Type> zzxw(void) const {
		return { z,z,x,w };
	}

	LWVector4<Type> zzyx(void) const {
		return { z,z,y,x };
	}

	LWVector4<Type> zzyy(void) const {
		return { z,z,y,y };
	}

	LWVector4<Type> zzyz(void) const {
		return { z,z,y,z };
	}

	LWVector4<Type> zzyw(void) const {
		return { z,z,y,w };
	}

	LWVector4<Type> zzzx(void) const {
		return { z,z,z,x };
	}

	LWVector4<Type> zzzy(void) const {
		return { z,z,z,y };
	}

	LWVector4<Type> zzzz(void) const {
		return { z,z,z,z };
	}

	LWVector4<Type> zzzw(void) const {
		return { z,z,z,w };
	}

	LWVector4<Type> zzwx(void) const {
		return { z,z,w,x };
	}

	LWVector4<Type> zzwy(void) const {
		return { z,z,w,y };
	}

	LWVector4<Type> zzwz(void) const {
		return { z,z,w,z };
	}

	LWVector4<Type> zzww(void) const {
		return { z,z,w,w };
	}

	LWVector4<Type> zwxx(void) const {
		return { z,w,x,x };
	}

	LWVector4<Type> zwxy(void) const {
		return { z,w,x,y };
	}

	LWVector4<Type> zwxz(void) const {
		return { z,w,x,z };
	}

	LWVector4<Type> zwxw(void) const {
		return { z,w,x,w };
	}

	LWVector4<Type> zwyx(void) const {
		return { z,w,y,x };
	}

	LWVector4<Type> zwyy(void) const {
		return { z,w,y,y };
	}

	LWVector4<Type> zwyz(void) const {
		return { z,w,y,z };
	}

	LWVector4<Type> zwyw(void) const {
		return { z,w,y,w };
	}

	LWVector4<Type> zwzx(void) const {
		return { z,w,z,x };
	}

	LWVector4<Type> zwzy(void) const {
		return { z,w,z,y };
	}

	LWVector4<Type> zwzz(void) const {
		return { z,w,z,z };
	}

	LWVector4<Type> zwzw(void) const {
		return { z,w,z,w };
	}

	LWVector4<Type> zwwx(void) const {
		return { z,w,w,x };
	}

	LWVector4<Type> zwwy(void) const {
		return { z,w,w,y };
	}

	LWVector4<Type> zwwz(void) const {
		return { z,w,w,z };
	}

	LWVector4<Type> zwww(void) const {
		return { z,w,w,w };
	}

	LWVector4<Type> wxxx(void) const {
		return { w,x,x,x };
	}

	LWVector4<Type> wxxy(void) const {
		return { w,x,x,y };
	}

	LWVector4<Type> wxxz(void) const {
		return { w,x,x,z };
	}

	LWVector4<Type> wxxw(void) const {
		return { w,x,x,w };
	}

	LWVector4<Type> wxyx(void) const {
		return { w,x,y,x };
	}

	LWVector4<Type> wxyy(void) const {
		return { w,x,y,y };
	}

	LWVector4<Type> wxyz(void) const {
		return { w,x,y,z };
	}

	LWVector4<Type> wxyw(void) const {
		return { w,x,y,w };
	}

	LWVector4<Type> wxzx(void) const {
		return { w,x,z,x };
	}

	LWVector4<Type> wxzy(void) const {
		return { w,x,z,y };
	}

	LWVector4<Type> wxzz(void) const {
		return { w,x,z,z };
	}

	LWVector4<Type> wxzw(void) const {
		return { w,x,z,w };
	}

	LWVector4<Type> wxwx(void) const {
		return { w,x,w,x };
	}

	LWVector4<Type> wxwy(void) const {
		return { w,x,w,y };
	}

	LWVector4<Type> wxwz(void) const {
		return { w,x,w,z };
	}

	LWVector4<Type> wxww(void) const {
		return { w,x,w,w };
	}

	LWVector4<Type> wyxx(void) const {
		return { w,y,x,x };
	}

	LWVector4<Type> wyxy(void) const {
		return { w,y,x,y };
	}

	LWVector4<Type> wyxz(void) const {
		return { w,y,x,z };
	}

	LWVector4<Type> wyxw(void) const {
		return { w,y,x,w };
	}

	LWVector4<Type> wyyx(void) const {
		return { w,y,y,x };
	}

	LWVector4<Type> wyyy(void) const {
		return { w,y,y,y };
	}

	LWVector4<Type> wyyz(void) const {
		return { w,y,y,z };
	}

	LWVector4<Type> wyyw(void) const {
		return { w,y,y,w };
	}

	LWVector4<Type> wyzx(void) const {
		return { w,y,z,x };
	}

	LWVector4<Type> wyzy(void) const {
		return { w,y,z,y };
	}

	LWVector4<Type> wyzz(void) const {
		return { w,y,z,z };
	}

	LWVector4<Type> wyzw(void) const {
		return { w,y,z,w };
	}

	LWVector4<Type> wywx(void) const {
		return { w,y,w,x };
	}

	LWVector4<Type> wywy(void) const {
		return { w,y,w,y };
	}

	LWVector4<Type> wywz(void) const {
		return { w,y,w,z };
	}

	LWVector4<Type> wyww(void) const {
		return { w,y,w,w };
	}

	LWVector4<Type> wzxx(void) const {
		return { w,z,x,x };
	}

	LWVector4<Type> wzxy(void) const {
		return { w,z,x,y };
	}

	LWVector4<Type> wzxz(void) const {
		return { w,z,x,z };
	}

	LWVector4<Type> wzxw(void) const {
		return { w,z,x,w };
	}

	LWVector4<Type> wzyx(void) const {
		return { w,z,y,x };
	}

	LWVector4<Type> wzyy(void) const {
		return { w,z,y,y };
	}

	LWVector4<Type> wzyz(void) const {
		return { w,z,y,z };
	}

	LWVector4<Type> wzyw(void) const {
		return { w,z,y,w };
	}

	LWVector4<Type> wzzx(void) const {
		return { w,z,z,x };
	}

	LWVector4<Type> wzzy(void) const {
		return { w,z,z,y };
	}

	LWVector4<Type> wzzz(void) const {
		return { w,z,z,z };
	}

	LWVector4<Type> wzzw(void) const {
		return { w,z,z,w };
	}

	LWVector4<Type> wzwx(void) const {
		return { w,z,w,x };
	}

	LWVector4<Type> wzwy(void) const {
		return { w,z,w,y };
	}

	LWVector4<Type> wzwz(void) const {
		return { w,z,w,z };
	}

	LWVector4<Type> wzww(void) const {
		return { w,z,w,w };
	}

	LWVector4<Type> wwxx(void) const {
		return { w,w,x,x };
	}

	LWVector4<Type> wwxy(void) const {
		return { w,w,x,y };
	}

	LWVector4<Type> wwxz(void) const {
		return { w,w,x,z };
	}

	LWVector4<Type> wwxw(void) const {
		return { w,w,x,w };
	}

	LWVector4<Type> wwyx(void) const {
		return { w,w,y,x };
	}

	LWVector4<Type> wwyy(void) const {
		return { w,w,y,y };
	}

	LWVector4<Type> wwyz(void) const {
		return { w,w,y,z };
	}

	LWVector4<Type> wwyw(void) const {
		return { w,w,y,w };
	}

	LWVector4<Type> wwzx(void) const {
		return { w,w,z,x };
	}

	LWVector4<Type> wwzy(void) const {
		return { w,w,z,y };
	}

	LWVector4<Type> wwzz(void) const {
		return { w,w,z,z };
	}

	LWVector4<Type> wwzw(void) const {
		return { w,w,z,w };
	}

	LWVector4<Type> wwwx(void) const {
		return { w,w,w,x };
	}

	LWVector4<Type> wwwy(void) const {
		return { w,w,w,y };
	}

	LWVector4<Type> wwwz(void) const {
		return { w,w,w,z };
	}

	LWVector4<Type> wwww(void) const {
		return { w,w,w,w };
	}

	LWVector3<Type> xxx(void) const {
		return { x,x,x };
	}

	LWVector3<Type> xxy(void) const {
		return { x,x,y };
	}

	LWVector3<Type> xxz(void) const {
		return { x,x,z };
	}

	LWVector3<Type> xxw(void) const {
		return { x,x,w };
	}

	LWVector3<Type> xyx(void) const {
		return { x,y,x };
	}

	LWVector3<Type> xyy(void) const {
		return { x,y,y };
	}

	LWVector3<Type> xyz(void) const {
		return { x,y,z };
	}

	LWVector3<Type> xyw(void) const {
		return { x,y,w };
	}

	LWVector3<Type> xzx(void) const {
		return { x,z,x };
	}

	LWVector3<Type> xzy(void) const {
		return { x,z,y };
	}

	LWVector3<Type> xzz(void) const {
		return { x,z,z };
	}

	LWVector3<Type> xzw(void) const {
		return { x,z,w };
	}

	LWVector3<Type> xwx(void) const {
		return { x,w,x };
	}

	LWVector3<Type> xwy(void) const {
		return { x,w,y };
	}

	LWVector3<Type> xwz(void) const {
		return { x,w,z };
	}

	LWVector3<Type> xww(void) const {
		return { x,w,w };
	}

	LWVector3<Type> yxx(void) const {
		return { y,x,x };
	}

	LWVector3<Type> yxy(void) const {
		return { y,x,y };
	}

	LWVector3<Type> yxz(void) const {
		return { y,x,z };
	}

	LWVector3<Type> yxw(void) const {
		return { y,x,w };
	}

	LWVector3<Type> yyx(void) const {
		return { y,y,x };
	}

	LWVector3<Type> yyy(void) const {
		return { y,y,y };
	}

	LWVector3<Type> yyz(void) const {
		return { y,y,z };
	}

	LWVector3<Type> yyw(void) const {
		return { y,y,w };
	}

	LWVector3<Type> yzx(void) const {
		return { y,z,x };
	}

	LWVector3<Type> yzy(void) const {
		return { y,z,y };
	}

	LWVector3<Type> yzz(void) const {
		return { y,z,z };
	}

	LWVector3<Type> yzw(void) const {
		return { y,z,w };
	}

	LWVector3<Type> ywx(void) const {
		return { y,w,x };
	}

	LWVector3<Type> ywy(void) const {
		return { y,w,y };
	}

	LWVector3<Type> ywz(void) const {
		return { y,w,z };
	}

	LWVector3<Type> yww(void) const {
		return { y,w,w };
	}

	LWVector3<Type> zxx(void) const {
		return { z,x,x };
	}

	LWVector3<Type> zxy(void) const {
		return { z,x,y };
	}

	LWVector3<Type> zxz(void) const {
		return { z,x,z };
	}

	LWVector3<Type> zxw(void) const {
		return { z,x,w };
	}

	LWVector3<Type> zyx(void) const {
		return { z,y,x };
	}

	LWVector3<Type> zyy(void) const {
		return { z,y,y };
	}

	LWVector3<Type> zyz(void) const {
		return { z,y,z };
	}

	LWVector3<Type> zyw(void) const {
		return { z,y,w };
	}

	LWVector3<Type> zzx(void) const {
		return { z,z,x };
	}

	LWVector3<Type> zzy(void) const {
		return { z,z,y };
	}

	LWVector3<Type> zzz(void) const {
		return { z,z,z };
	}

	LWVector3<Type> zzw(void) const {
		return { z,z,w };
	}

	LWVector3<Type> zwx(void) const {
		return { z,w,x };
	}

	LWVector3<Type> zwy(void) const {
		return { z,w,y };
	}

	LWVector3<Type> zwz(void) const {
		return { z,w,z };
	}

	LWVector3<Type> zww(void) const {
		return { z,w,w };
	}

	LWVector3<Type> wxx(void) const {
		return { w,x,x };
	}

	LWVector3<Type> wxy(void) const {
		return { w,x,y };
	}

	LWVector3<Type> wxz(void) const {
		return { w,x,z };
	}

	LWVector3<Type> wxw(void) const {
		return { w,x,w };
	}

	LWVector3<Type> wyx(void) const {
		return { w,y,x };
	}

	LWVector3<Type> wyy(void) const {
		return { w,y,y };
	}

	LWVector3<Type> wyz(void) const {
		return { w,y,z };
	}

	LWVector3<Type> wyw(void) const {
		return { w,y,w };
	}

	LWVector3<Type> wzx(void) const {
		return { w,z,x };
	}

	LWVector3<Type> wzy(void) const {
		return { w,z,y };
	}

	LWVector3<Type> wzz(void) const {
		return { w,z,z };
	}

	LWVector3<Type> wzw(void) const {
		return { w,z,w };
	}

	LWVector3<Type> wwx(void) const {
		return { w,w,x };
	}

	LWVector3<Type> wwy(void) const {
		return { w,w,y };
	}

	LWVector3<Type> wwz(void) const {
		return { w,w,z };
	}

	LWVector3<Type> www(void) const {
		return { w,w,w };
	}

	LWVector2<Type> xx(void) const {
		return { x,x };
	}

	LWVector2<Type> xy(void) const {
		return { x,y };
	}

	LWVector2<Type> xz(void) const {
		return { x,z };
	}

	LWVector2<Type> xw(void) const {
		return { x,w };
	}

	LWVector2<Type> yx(void) const {
		return { y,x };
	}

	LWVector2<Type> yy(void) const {
		return { y,y };
	}

	LWVector2<Type> yz(void) const {
		return { y,z };
	}

	LWVector2<Type> yw(void) const {
		return { y,w };
	}

	LWVector2<Type> zx(void) const {
		return { z,x };
	}

	LWVector2<Type> zy(void) const {
		return { z,y };
	}

	LWVector2<Type> zz(void) const {
		return { z,z };
	}

	LWVector2<Type> zw(void) const {
		return { z,w };
	}

	LWVector2<Type> wx(void) const {
		return { w,x };
	}

	LWVector2<Type> wy(void) const {
		return { w,y };
	}

	LWVector2<Type> wz(void) const {
		return { w,z };
	}

	LWVector2<Type> ww(void) const {
		return { w,w };
	}
	/*! \endcond */

	/*! \brief casts from current type into a new type so long as the underlying types are castable.
	*/
	template<class NewType>
	LWVector4<NewType> CastTo(void) const{
		return LWVector4<NewType>((NewType)x, (NewType)y, (NewType)z, (NewType)w);
	}

	/*! \brief Construct for LWVector4 type where all parameters can be specified. */
	LWVector4(Type x, Type y, Type z, Type w) : x(x), y(y), z(z), w(w){}

	/*! \brief Construct for LWVector4 Type where parameters are passed in as an array of elements.*/

	LWVector4(const Type *xyzw) : x(xyzw[0]), y(xyzw[1]), z(xyzw[2]), w(xyzw[3]){}

	/*! \brief Constructs for LWVector4 Type with parameters are a vec3 type, and a type. */
	LWVector4(const LWVector3<Type> &Xyz, Type w) : x(Xyz.x), y(Xyz.y), z(Xyz.z), w(w){}

	/*! \brief Constructs for LWVector4 Type with parameters are a type and vec3 type. */
	LWVector4(Type x, const LWVector3<Type> &Yzw) : x(x), y(Yzw.x), z(Yzw.y), w(Yzw.z){}

	/*! \brief Construct for LWVector4 Type with paramertas are a Vec2 Type. */
	LWVector4(const LWVector2<Type> &Xy, const LWVector2<Type> &Zw) : x(Xy.x), y(Xy.y), z(Zw.x), w(Zw.y){}

	/*! \brief Construct for LWVector4 Type with parameters are a Vec2 Type, and a type. */
	LWVector4(const LWVector2<Type> &Xy, Type z, Type w) : x(Xy.x), y(Xy.y), z(z), w(w){}

	/*! \brief Construct for LWVector4 Type with parameters are a type, Vec2 Type, and a Type. */
	LWVector4(Type x, const LWVector2<Type> &Yz, Type w) : x(x), y(Yz.x), z(Yz.y), w(w){}

	/*! \brief Construct for LWVector4 Type with parameters are a type, and vec2 type. */
	LWVector4(Type x, Type y, const LWVector2<Type> &Zw) : x(x), y(y), z(Zw.x), w(Zw.y){}

	/*! \brief Construct for LWvector4 Type with parameters are a Vec2 and an array of Type. */
	LWVector4(const LWVector2<Type> &Xy, const Type *Zw) : x(Xy.x), y(Xy.y), z(Zw[0]), w(Zw[1]){}

	/*! \brief Construct for LWvector4 Type with parameters are an array of Type and a Vec2. */
	LWVector4(const Type *Xy, const LWVector2<Type> &Zw) : x(Xy[0]), y(Xy[1]), z(Zw.x), w(Zw.y){}

	/*! \brief Construct for LWVector4 Type with defaults all parameters to f. */
	LWVector4(Type f=0) : x(f), y(f), z(f), w(f){}
};

/*! \brief A vector3 math utility which allows for numerous mathematical expressions to be expressed quickly and easily.
*/

template<class Type>
struct LWVector3{
	Type x; /*!< \brief x component of the Vector3 */
	Type y; /*!< \brief y component of the Vector3 */
	Type z; /*!< \brief z component of the Vector3 */

	/*! \brief returns a copy of the normalized vector3. */
	LWVector3<Type> Normalize(void) const{
		Type L = x*x + y*y + z*z;
		if (L <= std::numeric_limits<Type>::epsilon()) L = 0;
		else L = (Type)1 / sqrt(L);
		return LWVector3<Type>(x*L, y*L, z*L);
	}

	/*! \brief writes into result the normalized of this vector3.
		\param Result the variable to store the normalized result into.
	*/
	void Normalize(LWVector3<Type> &Result) const{
		Result = Normalize();
		return;
	}

	/*!< \brief returns the min of all components of the vec3. */
	Type Min(void) const {
		return std::min<Type>(std::min<Type>(x, y), z);
	}

	/*!< \brief returns the min of each component between this vector3 and A vector3 */
	LWVector3<Type> Min(const LWVector3<Type> &A) const {
		return LWVector3<Type>(std::min<Type>(x, A.x), std::min<Type>(y, A.y), std::min<Type>(z, A.z));
	}


	/*!< \brief writes into result the min of each component between this vector3 and A vector3 */
	void Min(const LWVector3<Type> &A, const LWVector3<Type> &Result) const {
		Result = Min(A);
		return;
	}

	/*!< \brief returns the max of all components of the vec3. */
	Type Max(void) const {
		return std::max<Type>(std::max<Type>(x, y), z);
	}

	/*!< \brief returns the max of each component between this vector3 and A vector3 */
	LWVector3<Type> Max(const LWVector3<Type> &A) const {
		return LWVector3<Type>(std::max<Type>(x, A.x), std::max<Type>(y, A.y), std::max<Type>(z, A.z));
	}


	/*!< \brief writes into result the max of each component between this vector3 and A vector3 */
	void Max(const LWVector3<Type> &A, const LWVector3<Type> &Result) const {
		Result = Max(A);
		return;
	}

	/*!< \brief projects Pnt onto this vector and returns the projected result. */
	LWVector3<Type> Project(const LWVector3<Type> &Pnt) const {
		LWVector3f N = Normalize();
		return N*N.Dot(Pnt);
	}

	/*<! \brief projects Pnt onto this vector and stores the resultant into Result. */
	void Project(const LWVector3<Type> &Pnt, LWVector3<Type> &Result) const {
		Result = Project(Pnt);
		return;
	}

	/*!< \brief attempts to generate orthogonal angles to the supplied axis. */
	void Othogonal(LWVector3<Type> &Right, LWVector3<Type> &Up) const {
		const LWVector3<Type> XAxis = LWVector3<Type>(1, 0, 0);
		const LWVector3<Type> YAxis = LWVector3<Type>(0, 1, 0);
		LWVector3 A = XAxis;
		float d = fabs(Dot(A));
		if (d > 0.8) A = YAxis;
		Right = Cross(A).Normalize();
		Up = Cross(Right);
		return;
	}

	/*! \brief returns the cross product of two vector3's. 
	*/
	LWVector3<Type> Cross(const LWVector3<Type> &O) const{
		return LWVector3<Type>(y*O.z - z*O.y, z*O.x - x*O.z, x*O.y - y*O.x);
	}

	/*! \brief returns the cross product of two Vctor3's.
		\param O the second vector3 to cross product with.
		\param Result the result of the cross product is placed inside this vec3.
	*/
	void Cross(const LWVector3<Type> &O, LWVector3<Type> &Result) const{
		Result = Cross(O);
		return;
	}

	/*! \brief Gets the length of the vector3.
		\return the length of the vector3.
	*/
	Type Length(void) const{
		Type L = x*x + y*y + z*z;
		if (L <= std::numeric_limits<Type>::epsilon()) return 0;
		return (Type)sqrt(L);
	}

	/*! \brief Gets the squared length of the vector4.
		\return the squared length of the vector3.
	*/
	Type LengthSquared(void) const{
		return x*x + y*y + z*z;
	}

	/*! \brief returns a value of the distance between two vector3's.
		\return the distance between two Vector3's.
	*/
	Type Distance(const LWVector3<Type> &O) const{
		return (*this - O).Length();
	}

	/*! \brief return the squared distance between two vector3's.
		\return the distance between two Vector3's.
	*/
	Type DistanceSquared(const LWVector3<Type> &O) const{
		return (*this - O).LengthSquared();
	}

	/*! \brief returns the dot product between tow vector3's.
	*/
	Type Dot(const LWVector3<Type> &O) const{
		return x*O.x + y*O.y + z*O.z;
	}

	/*! \cond */

	LWVector3<Type> &operator = (const LWVector3<Type> &Rhs){
		x = Rhs.x;
		y = Rhs.y;
		z = Rhs.z;
		return *this;
	}

	LWVector3<Type> &operator += (const LWVector3<Type> &Rhs){
		x += Rhs.x;
		y += Rhs.y;
		z += Rhs.z;
		return *this;
	}

	LWVector3<Type> &operator += (Type Rhs){
		x += Rhs;
		y += Rhs;
		z += Rhs;
		return *this;
	}

	LWVector3<Type> &operator -= (const LWVector3<Type> &Rhs){
		x -= Rhs.x;
		y -= Rhs.y;
		z -= Rhs.z;
		return *this;
	}

	LWVector3<Type> &operator -= (Type Rhs){
		x -= Rhs;
		y -= Rhs;
		z -= Rhs;
		return *this;
	}

	LWVector3<Type> &operator *= (const LWVector3<Type> &Rhs){
		x *= Rhs.x;
		y *= Rhs.y;
		z *= Rhs.z;
		return *this;
	}

	LWVector3<Type> &operator *= (Type Rhs){
		x *= Rhs;
		y *= Rhs;
		z *= Rhs;
		return *this;
	}

	LWVector3<Type> &operator /= (const LWVector3<Type> &Rhs){
		x /= Rhs.x;
		y /= Rhs.y;
		z /= Rhs.z;
		return *this;
	}

	LWVector3<Type> &operator /= (Type Rhs){
		x /= Rhs;
		y /= Rhs;
		z /= Rhs;
		return *this;
	}

	friend LWVector3<Type> operator + (const LWVector3<Type> &Rhs){
		return LWVector3<Type>(Rhs.x, Rhs.y, Rhs.z);
	}

	friend LWVector3<Type> operator - (const LWVector3<Type> &Rhs){
		return LWVector3<Type>(-Rhs.x, -Rhs.y, -Rhs.z);
	}

	bool operator == (const LWVector3<Type> &Rhs) const{
		const Type e = std::numeric_limits<Type>::epsilon();
		return (Type)abs(x - Rhs.x) <= e && (Type)abs(y - Rhs.y) <= e && (Type)abs(z - Rhs.z) <= e;
	}

	bool operator != (const LWVector3<Type> &Rhs) const{
		return !(*this == Rhs);
	}

	friend std::ostream &operator<<(std::ostream &o, const LWVector3<Type> &v) {
		o << v.x << " " << v.y << " " << v.z;
		return o;
	}

	friend LWVector3<Type> operator + (const LWVector3<Type> &Lhs, const LWVector3<Type> &Rhs){
		return LWVector3<Type>(Lhs.x + Rhs.x, Lhs.y + Rhs.y, Lhs.z + Rhs.z);
	}

	friend LWVector3<Type> operator + (const LWVector3<Type> &Lhs, Type Rhs){
		return LWVector3<Type>(Lhs.x + Rhs, Lhs.y + Rhs, Lhs.z + Rhs);
	}

	friend LWVector3<Type> operator + (Type Lhs, const LWVector3<Type> &Rhs){
		return LWVector3<Type>(Lhs + Rhs.x, Lhs + Rhs.y, Lhs + Rhs.z);
	}

	friend LWVector3<Type> operator - (const LWVector3<Type> &Lhs, const LWVector3<Type> &Rhs){
		return LWVector3<Type>(Lhs.x - Rhs.x, Lhs.y - Rhs.y, Lhs.z - Rhs.z);
	}

	friend LWVector3<Type> operator - (const LWVector3<Type> &Lhs, Type Rhs){
		return LWVector3<Type>(Lhs.x - Rhs, Lhs.y - Rhs, Lhs.z - Rhs);
	}

	friend LWVector3<Type> operator - (Type Lhs, const LWVector3<Type> &Rhs){
		return LWVector3<Type>(Lhs - Rhs.x, Lhs - Rhs.y, Lhs - Rhs.z);
	}

	friend LWVector3<Type> operator * (const LWVector3<Type> &Lhs, const LWVector3<Type> &Rhs){
		return LWVector3<Type>(Lhs.x * Rhs.x, Lhs.y * Rhs.y, Lhs.z * Rhs.z);
	}

	friend LWVector3<Type> operator * (const LWVector3<Type> &Lhs, Type Rhs){
		return LWVector3<Type>(Lhs.x * Rhs, Lhs.y * Rhs, Lhs.z * Rhs);
	}

	friend LWVector3<Type> operator * (Type Lhs, const LWVector3<Type> &Rhs){
		return LWVector3<Type>(Lhs * Rhs.x, Lhs * Rhs.y, Lhs * Rhs.z);
	}

	friend LWVector3<Type> operator / (const LWVector3<Type> &Lhs, const LWVector3<Type> &Rhs){
		return LWVector3<Type>(Lhs.x / Rhs.x, Lhs.y / Rhs.y, Lhs.z / Rhs.z);
	}

	friend LWVector3<Type> operator / (const LWVector3<Type> &Lhs, Type Rhs){
		return LWVector3<Type>(Lhs.x / Rhs, Lhs.y / Rhs, Lhs.z / Rhs);
	}

	friend LWVector3<Type> operator / (Type Lhs, const LWVector3<Type> &Rhs){
		return LWVector3<Type>(Lhs / Rhs.x, Lhs / Rhs.y, Lhs / Rhs.z);
	}

	LWVector3<Type> xxx(void) const {
		return { x,x,x };
	}

	LWVector3<Type> xxy(void) const {
		return { x,x,y };
	}

	LWVector3<Type> xxz(void) const {
		return { x,x,z };
	}

	LWVector3<Type> xyx(void) const {
		return { x,y,x };
	}

	LWVector3<Type> xyy(void) const {
		return { x,y,y };
	}

	LWVector3<Type> xzx(void) const {
		return { x,z,x };
	}

	LWVector3<Type> xzy(void) const {
		return { x,z,y };
	}

	LWVector3<Type> xzz(void) const {
		return { x,z,z };
	}

	LWVector3<Type> yxx(void) const {
		return { y,x,x };
	}

	LWVector3<Type> yxy(void) const {
		return { y,x,y };
	}

	LWVector3<Type> yxz(void) const {
		return { y,x,z };
	}

	LWVector3<Type> yyx(void) const {
		return { y,y,x };
	}

	LWVector3<Type> yyy(void) const {
		return { y,y,y };
	}

	LWVector3<Type> yyz(void) const {
		return { y,y,z };
	}

	LWVector3<Type> yzx(void) const {
		return { y,z,x };
	}

	LWVector3<Type> yzy(void) const {
		return { y,z,y };
	}

	LWVector3<Type> yzz(void) const {
		return { y,z,z };
	}

	LWVector3<Type> zxx(void) const {
		return { z,x,x };
	}

	LWVector3<Type> zxy(void) const {
		return { z,x,y };
	}

	LWVector3<Type> zxz(void) const {
		return { z,x,z };
	}

	LWVector3<Type> zyx(void) const {
		return { z,y,x };
	}

	LWVector3<Type> zyy(void) const {
		return { z,y,y };
	}

	LWVector3<Type> zyz(void) const {
		return { z,y,z };
	}

	LWVector3<Type> zzx(void) const {
		return { z,z,x };
	}

	LWVector3<Type> zzy(void) const {
		return { z,z,y };
	}

	LWVector3<Type> zzz(void) const {
		return { z,z,z };
	}

	LWVector2<Type> xx(void) const {
		return { x,x };
	}

	LWVector2<Type> xy(void) const {
		return { x,y };
	}

	LWVector2<Type> xz(void) const {
		return { x,z };
	}

	LWVector2<Type> yx(void) const {
		return { y,x };
	}

	LWVector2<Type> yy(void) const {
		return { y,y };
	}

	LWVector2<Type> yz(void) const {
		return { y,z };
	}

	LWVector2<Type> zx(void) const {
		return { z,x };
	}

	LWVector2<Type> zy(void) const {
		return { z,y };
	}

	LWVector2<Type> zz(void) const {
		return { z,z };
	}

	/*! \endcond */

	/*! \brief casts from current type into a new type so long as the underlying types are castable.
	*/
	template<class NewType>
	LWVector3<NewType> CastTo(void) const{
		return LWVector3<NewType>((NewType)x, (NewType)y, (NewType)z);
	}

	/*! \brief Construct for LWVector3 type where all parameters can be specified. */
	LWVector3(Type x, Type y, Type z) : x(x), y(y), z(z){}

	/*! \brief Construct for LWVector3 Type where parameters are passed in as an array of elements.*/
	LWVector3(Type *xyz) : x(xyz[0]), y(xyz[1]), z(xyz[2]){}

	/*! \brief Construct for LWVector3 Type where parameters are a vec2 and a type. */
	LWVector3(const LWVector2<Type> &Xy, Type z) : x(Xy.x), y(Xy.y), z(z){}

	/*! \brief Construct for LWVector3 Type where parameters are a type and a vec2. */
	LWVector3(Type x, const LWVector2<Type> &Yz) : x(x), y(Yz.x), z(Yz.y){}
	
	/*! \brief Construct for LWVector3 Type with defaults all parameters to f. */
	LWVector3(Type f = 0) : x(f), y(f), z(f){}
};


/*! \brief A vector2 math utility which allows for numerous mathematical expressions to be expressed quickly and easily.
*/

template<class Type>
struct LWVector2{
	Type x; /*!< \brief x component of the Vector2 */
	Type y; /*!< \brief y component of the Vector2 */

	/*! \brief Construct a LWVector2 from the input radian theta. */

	static LWVector2<Type> MakeTheta(Type Theta){
		return LWVector2<Type>(cos(Theta), sin(Theta));
	}

	/*! \brief returns a copy of the normalized vector3. */
	LWVector2<Type> Normalize(void) const{
		Type L = x*x + y*y;
		if (L <= std::numeric_limits<Type>::epsilon()) L = 0;
		else L = (Type)1 / sqrt(L);
		return LWVector2<Type>(x*L, y*L);
	}

	/*! \brief writes into result the normalized of this vector2.
	\param Result the variable to store the normalized result into.
	*/
	void Normalize(LWVector2<Type> &Result) const{
		Result = Normalize();
		return;
	}
	/*!< \brief returns the min component of the Vec2. */
	Type Min(void) const {
		return std::min<Type>(x, y);
	}

	/*!< \brief returns the min of each component between this vector2 and A vector2. */
	LWVector2<Type> Min(const LWVector2<Type> &A) const {
		return LWVector2<Type>(std::min<Type>(x, A.x), std::min<Type>(y, A.y));
	}


	/*!< \brief writes into result the min of each component between this vector2 and A vector2. */
	void Min(const LWVector2<Type> &A, const LWVector2<Type> &Result) const {
		Result = Min(A);
		return;
	}

	/*!< \brief returns the max component of the Vec2. */
	Type Max(void) const {
		return std::max<Type>(x, y);
	}

	/*!< \brief returns the max of each component between this vector2 and A vector2. */
	LWVector2<Type> Max(const LWVector2<Type> &A) const {
		return LWVector2<Type>(std::max<Type>(x, A.x), std::max<Type>(y, A.y));
	}


	/*!< \brief writes into result the max of each component between this vector2 and A vector2. */
	void Max(const LWVector2<Type> &A, const LWVector2<Type> &Result) const {
		Result = Max(A);
		return;
	}

	/*! \brief returns a perpendicular vector of the Vector2.
	*/
	LWVector2<Type> Perpindicular(void) const{
		return LWVector2<Type>(-y, x);
	}

	/*! \brief returns the theta angle between the x and y component with atan2.
	*/
	Type Theta(void) const{
		return atan2(y, x);
	}

	/*! \brief writes the perpendicular vector of vector2 into result.
	*/
	void Perpindicular(LWVector2<Type> &Result) const{
		Result = Perpindicular();
		return;
	}

	/*! \brief Gets the length of the vector2.
	\return the length of the vector2.
	*/
	Type Length(void) const{
		Type L = x*x + y*y;
		if (L <= std::numeric_limits<Type>::epsilon()) return 0;
		return (Type)sqrt(L);
	}

	/*! \brief Gets the squared length of the vector2.
	\return the squared length of the vector2.
	*/
	Type LengthSquared(void) const{
		return x*x + y*y;
	}

	/*! \brief returns a value of the distance between two vector2's.
	\return the distance between two Vector2's.
	*/
	Type Distance(const LWVector2<Type> &O) const{
		return (*this - O).Length();
	}

	/*! \brief return the squared distance between two vector2's.
	\return the distance between two Vector2's.
	*/
	Type DistanceSquared(const LWVector2<Type> &O) const{
		return (*this - O).LengthSquared();
	}

	/*! \brief returns the dot product between tow vector2's.
	*/
	Type Dot(const LWVector2<Type> &O) const{
		return x*O.x + y*O.y;
	}

	/*!< \brief rotates a point around the rotation vector. */
	LWVector2<Type> Rotate(const LWVector2<Type> &Pnt) const{
		return LWVector2<Type>(Pnt.x*x + Pnt.y*y, Pnt.x*y - Pnt.y*x);
	}

	/*!< \brief write the rotated point into result. */
	void Rotate(LWVector2<Type> &Result, const LWVector2<Type> &Pnt) {
		Result = Rotate(Pnt);
		return;
	}

	/*! \cond */

	LWVector2<Type> &operator = (const LWVector2<Type> &Rhs){
		x = Rhs.x;
		y = Rhs.y;
		return *this;
	}

	LWVector2<Type> &operator += (const LWVector2<Type> &Rhs){
		x += Rhs.x;
		y += Rhs.y;
		return *this;
	}

	LWVector2<Type> &operator += (Type Rhs){
		x += Rhs;
		y += Rhs;
		return *this;
	}

	LWVector2<Type> &operator -= (const LWVector2<Type> &Rhs){
		x -= Rhs.x;
		y -= Rhs.y;
		return *this;
	}

	LWVector2<Type> &operator -= (Type Rhs){
		x -= Rhs;
		y -= Rhs;
		return *this;
	}

	LWVector2<Type> &operator *= (const LWVector2<Type> &Rhs){
		x *= Rhs.x;
		y *= Rhs.y;
		return *this;
	}

	LWVector2<Type> &operator *= (Type Rhs){
		x *= Rhs;
		y *= Rhs;
		return *this;
	}

	LWVector2<Type> &operator /= (const LWVector2<Type> &Rhs){
		x /= Rhs.x;
		y /= Rhs.y;
		return *this;
	}

	LWVector2<Type> &operator /= (Type Rhs){
		x /= Rhs;
		y /= Rhs;
		return *this;
	}

	friend LWVector2<Type> operator + (const LWVector2<Type> &Rhs){
		return LWVector2<Type>(Rhs.x, Rhs.y);
	}

	friend LWVector2<Type> operator - (const LWVector2<Type> &Rhs){
		return LWVector2<Type>(-Rhs.x, -Rhs.y);
	}

	bool operator == (const LWVector2<Type> &Rhs) const{
		const Type e = std::numeric_limits<Type>::epsilon();
		return (Type)abs(x - Rhs.x) <= e && (Type)abs(y - Rhs.y) <= e;
	}

	bool operator != (const LWVector2<Type> &Rhs) const{
		return !(*this == Rhs);
	}

	friend std::ostream &operator<<(std::ostream &o, const LWVector2<Type> &v) {
		o << v.x << " " << v.y;
		return o;
	}

	friend LWVector2<Type> operator + (const LWVector2<Type> &Lhs, const LWVector2<Type> &Rhs){
		return LWVector2<Type>(Lhs.x + Rhs.x, Lhs.y + Rhs.y);
	}

	friend LWVector2<Type> operator + (const LWVector2<Type> &Lhs, Type Rhs){
		return LWVector2<Type>(Lhs.x + Rhs, Lhs.y + Rhs);
	}

	friend LWVector2<Type> operator + (Type Lhs, const LWVector2<Type> &Rhs){
		return LWVector2<Type>(Lhs + Rhs.x, Lhs + Rhs.y);
	}

	friend LWVector2<Type> operator - (const LWVector2<Type> &Lhs, const LWVector2<Type> &Rhs){
		return LWVector2<Type>(Lhs.x - Rhs.x, Lhs.y - Rhs.y);
	}

	friend LWVector2<Type> operator - (const LWVector2<Type> &Lhs, Type Rhs){
		return LWVector2<Type>(Lhs.x - Rhs, Lhs.y - Rhs);
	}

	friend LWVector2<Type> operator - (Type Lhs, const LWVector2<Type> &Rhs){
		return LWVector2<Type>(Lhs - Rhs.x, Lhs - Rhs.y);
	}

	friend LWVector2<Type> operator * (const LWVector2<Type> &Lhs, const LWVector2<Type> &Rhs){
		return LWVector2<Type>(Lhs.x * Rhs.x, Lhs.y * Rhs.y);
	}

	friend LWVector2<Type> operator * (const LWVector2<Type> &Lhs, Type Rhs){
		return LWVector2<Type>(Lhs.x * Rhs, Lhs.y * Rhs);
	}

	friend LWVector2<Type> operator * (Type Lhs, const LWVector2<Type> &Rhs){
		return LWVector2<Type>(Lhs * Rhs.x, Lhs * Rhs.y);
	}

	friend LWVector2<Type> operator / (const LWVector2<Type> &Lhs, const LWVector2<Type> &Rhs){
		return LWVector2<Type>(Lhs.x / Rhs.x, Lhs.y / Rhs.y);
	}

	friend LWVector2<Type> operator / (const LWVector2<Type> &Lhs, Type Rhs){
		return LWVector2<Type>(Lhs.x / Rhs, Lhs.y / Rhs);
	}

	friend LWVector2<Type> operator / (Type Lhs, const LWVector2<Type> &Rhs){
		return LWVector2<Type>(Lhs / Rhs.x, Lhs / Rhs.y);
	}

	LWVector2<Type> xx(void) const {
		return { x,x };
	}

	LWVector2<Type> yx(void) const {
		return { y,x };
	}

	LWVector2<Type> yy(void) const {
		return { y,y };
	}

	/*! \endcond */

	/*! \brief casts from current type into a new type so long as the underlying types are castable.
	*/
	template<class NewType>
	LWVector2<NewType> CastTo(void) const{
		return LWVector2<NewType>((NewType)x, (NewType)y);
	}

	/*! \brief Construct for LWVector2 type where all parameters can be specified. */

	LWVector2(Type x, Type y) : x(x), y(y){}

	/*! \brief Construct for LWVector2 Type where parameters are passed in as an array of elements.*/

	LWVector2(Type *xy) : x(xy[0]), y(xy[1]){}

	/*! \brief Construct for LWVector2 Type with defaults all parameters to f. */

	LWVector2(Type f = 0) : x(f), y(f){}
};
/*! @} */
#endif
