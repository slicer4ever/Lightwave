#ifndef LWCORETYPES_H
#define LWCORETYPES_H
#include <cstdint>

//Add libfmt support.
#ifndef _HAS_EXCEPTIONS
#define _HAS_EXCEPTIONS 0 //Because subsystem:Windows only supports 4096 characters, libfmt will throw an exception when such an issue is encountered, this ideally prevents it during release(and debug builds should be set to console mode, however if not then a debug build will assert).
#endif
#include "../../../../Dependency/libfmt/include/fmt/format.h"
#include "../../../../Dependency/libfmt/include/fmt/chrono.h"
#include "../../../../Dependency/libfmt/include/fmt/ostream.h"

/*! \defgroup LWCore LWCore
	\brief the core of the entire framework that is built upon these classes.
	@{
*/

#ifndef char8_t
typedef char char8_t;
#endif

class LWAllocator;

class LWByteBuffer;

class LWByteStream;

class LWFileStream;

template<class Type>
class LWUnicode;

template<class Type>
class LWUnicodeIterator;

template<class Type>
class LWUnicodeGraphemeIterator;

class LWTimer;

template<class Type>
struct LWVector4;

template<class Type>
struct LWVector3;

template<class Type>
struct LWVector2;

template<class Type>
struct LWMatrix4;

template<class Type>
struct LWMatrix3;

template<class Type>
struct LWMatrix2;

template<class Type>
struct LWQuaternion;

template<class Type>
struct LWSVector4;

template<class Type>
struct LWSMatrix4;

template<class Type>
struct LWSQuaternion;

class LWAllocator;

class LWAllocator_Default;

class LWAllocator_LocalCircular;

class LWAllocator_ConcurrentCircular;

class LWAllocator_LocalHeap;

template<class Type>
class LWFIFO;

template<class Type, uint32_t MaxElements>
class LWConcurrentFIFO;

typedef LWUnicode<char8_t> LWUTF8;
typedef LWUnicode<char16_t> LWUTF16;
typedef LWUnicode<char32_t> LWUTF32;

typedef LWUnicodeIterator<char8_t> LWUTF8Iterator;
typedef LWUnicodeIterator<char16_t> LWUTF16Iterator;
typedef LWUnicodeIterator<char32_t> LWUTF32Iterator;

typedef LWUnicodeGraphemeIterator<char8_t> LWUTF8GraphemeIterator;
typedef LWUnicodeGraphemeIterator<char16_t> LWUTF16GraphemeIterator;
typedef LWUnicodeGraphemeIterator<char32_t> LWUTF32GraphemeIterator;

typedef LWUTF8Iterator LWUTF8I; //Shorthand for utf8 iterator.
typedef LWUTF16Iterator LWUTF16I;
typedef LWUTF32Iterator LWUTF32I;

typedef LWUTF8GraphemeIterator LWUTF8GI; //Shorthand for utf8 grapheme iterator;
typedef LWUTF16GraphemeIterator LWUTF16GI;
typedef LWUTF32GraphemeIterator LWUTF32GI;

/*!< \brief defined double version of the quaternion class. */
typedef LWQuaternion<double> LWQuaterniond;
/*!< \brief defined float version of the quaternion class. */
typedef LWQuaternion<float> LWQuaternionf;

/*! \brief defined double version of the Vector4 class. */
typedef LWVector4<double> LWVector4d;
/*! \brief defined float version of the Vector4 class. */
typedef LWVector4<float> LWVector4f;
/*! \brief defined int32 version of the Vector4 class. */
typedef LWVector4<int32_t> LWVector4i;
/*! \brief defined uint32 version of the Vector4 class. */
typedef LWVector4<uint32_t> LWVector4ui;

/*! \brief defined double version of the Vector3 class. */
typedef LWVector3<double> LWVector3d;
/*! \brief defined float version of the Vector3 class. */
typedef LWVector3<float> LWVector3f;
/*! \brief defined int32 version of the Vector3 class. */
typedef LWVector3<int32_t> LWVector3i;
/*! \brief defined uint32 version of the Vector3 class. */
typedef LWVector3<uint32_t> LWVector3ui;

/*! \brief defined double version of the Vector2 class. */
typedef LWVector2<double> LWVector2d;
/*! \brief defined float version of the Vector2 class. */
typedef LWVector2<float> LWVector2f;
/*! \brief defined int32 version of the Vector2 class. */
typedef LWVector2<int32_t> LWVector2i;
/*!< \brief defined uint32 version of the vector2 class. */
typedef LWVector2<uint32_t> LWVector2ui;

/*! \brief defined double version of the Matrix4 class. */
typedef LWMatrix4<double> LWMatrix4d;

/*! \brief defined float version of the Matrix4 class. */
typedef LWMatrix4<float> LWMatrix4f;

/*! \brief defined int32 version of the Matrix4 class. */
typedef LWMatrix4<int32_t> LWMatrix4i;

/*! \brief defined double version of the Matrix3 class. */
typedef LWMatrix3<double> LWMatrix3d;

/*! \brief defined float version of the Matrix3 class. */
typedef LWMatrix3<float> LWMatrix3f;

/*! \brief defined int32 version of the Matrix3 class. */
typedef LWMatrix3<int32_t> LWMatrix3i;

/*! \brief defined double version of the Matrix2 class. */
typedef LWMatrix2<double> LWMatrix2d;

/*! \brief defined float version of the Matrix2 class. */
typedef LWMatrix2<float> LWMatrix2f;

/*! \brief defined int32 version of the Matrix2 class. */
typedef LWMatrix2<int32_t> LWMatrix2i;

/*!< \brief defined float variant of simd vector4 class. */
typedef LWSVector4<float> LWSVector4f;

/*!< \brief defined double variant of simd vector4 class. */
typedef LWSVector4<double> LWSVector4d;

/*!< \brief defined int32 version of the simd vector4 class. */
typedef LWSVector4<int32_t> LWSVector4i;

/*!< \brief defined float variant of simd matrix4 class. */
typedef LWSMatrix4<float> LWSMatrix4f;

/*!< \brief defined double variant of simd matrix4 class. */
typedef LWSMatrix4<double> LWSMatrix4d;

/*!< \brief defined int32 version of the simd matrix4 class. */
typedef LWSMatrix4<int32_t> LWSMatrix4i;

/*!< \brief defined float variant of simd quaternion class. */
typedef LWSQuaternion<float> LWSQuaternionf;

/*!< \brief defined double variant of the simd quaternion class. */
typedef LWSQuaternion<double> LWSQuaterniond;

/*!< \brief internal variable for Matrix4's ortho function, this will automatically be set depending on which video driver object is last created.  Side note: i really hate that i had to implement this, but I could not think of any reasonable solution since OpenGL and directX use different depth NDC ranges.) */
extern bool LWMatrix4_UseDXOrtho;

/* @} */
/*! \mainpage The Lightwave Framework Index.
	\section intro Introduction
	The lightwave framework is designed to be a relatively light weight, module based system that can be built upon.
*/


#endif