#ifndef LWMATH_H
#define LWMATH_H
#include "LWTypes.h"

#define LW_PI   3.14159265358979f //180
#define LW_PI_2 1.57079632679489f //90
#define LW_2PI  6.28318530717958f //360
#define LW_PI_4 0.78539816339744f //45
#define LW_PI_8 0.39269908169872f //22.5
#define LW_DEGTORAD  0.01745329251994f
#define LW_RADTODEG 57.29577951308232f

#define LWPACK_COLOR(r,g,b,a) (uint32_t)((((uint32_t)((r)*255.0f))<<24)|(((uint32_t)((g)*255.0f))<<16)|(((uint32_t)((b)*255.0f))<<8)|(((uint32_t)((a)*255.0f))))
#define LWUNPACK_COLOR(c, shift) (((float)(((c)>>(shift))&0xFF))/255.0f)
#define LWUNPACK_COLORVEC4f(c) LWVector4f(LWUNPACK_COLOR((c), 24), LWUNPACK_COLOR((c), 16), LWUNPACK_COLOR((c), 8), LWUNPACK_COLOR((c), 0))

#define LWEpsilon 0.001f
/*!< \brief encodes a floating point number with a specific range into N bits (i.e: LWENCODE_FLOAT(LW_PI, LW_2PI, 100) would result in 50. */
#define LWENCODE_FLOAT(Value, Range, N) (std::min<uint32_t>((uint32_t)(((Value)/(Range))*(float)(N)+LWEpsilon), (N)))
#define LWENCODE_FLOAT64(Value, Range, N) (std::min<uint64_t>((uint64_t)(((Value)/(Range))*(float)(N)+LWEpsilon), (N)))

/*!< \brief decodes an encoded floating point number with a specefic range back to a floating point value. (i.e: LWDECODE_FLOAT(50, LW_2PI, 100) would result in LW_PI value.) */
#define LWDECODE_FLOAT(Value, Range, N) (std::min<float>((((Value)/(float)(N))*(Range)), (Range)))

/*!< \brief circular rotate's value left by n Bits. */
template<class Type>
constexpr Type LWRotateLeft(Type Value, Type n) {
	return (Value << n) | (Value >> (sizeof(Type) * 8 - n));
}

/*!< \brief circular rotate's value right by n bits. */
template<class Type>
constexpr Type LWRotateRight(Type Value, Type n) {
	return (Value >> n) | (Value << (sizeof(Type) * 8 - n));
}

/*!< \brief convert's a HSL+Alpha color to RGB+Alpha value. */
LWVector4f LWHSLAtoRGBA(const LWVector4f &HSLA);

/*!< \brief convert's a RGB+Alpha color to HSL+Alpha value. */
LWVector4f LWRGBAtoHSLA(const LWVector4f &RGBA);

/*!< \brief interpolate's HSL color from Src to Dst, where p=0 is full Source, and p=1 is full Dst, alpha is linearly interpolated as is. */
LWVector4f LWHSLAInterpolate(const LWVector4f &Src, const LWVector4f &Dst, float p);

/*!< \brief returns v if it is already a 2n number, or the next 2n number. */
uint8_t LWNext2N(uint8_t v);

/*!< \brief returns v if it is already a 2n number, or the next 2n number. */
uint16_t LWNext2N(uint16_t v);

/*!< \brief returns v if it is already a 2n number, or the next 2n number. */
uint32_t LWNext2N(uint32_t v);

/*!< \brief returns v if it is already a 2n number, or the next 2n number. */
uint64_t LWNext2N(uint64_t v);

#endif

