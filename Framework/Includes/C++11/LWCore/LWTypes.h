#ifndef LWCORETYPES_H
#define LWCORETYPES_H
#include <cstdint>
/*! \defgroup LWCore LWCore
	\brief the core of the entire framework that is built upon these classes.
	@{
*/

#define LWMOD_EQL 0 /*!< \brief mod value to pass to ModFunc that sets A to B. */
#define LWMOD_OR  1 /*!< \brief mod value to pass to ModFunc that or's A with B. */
#define LWMOD_XOR 2 /*!< \brief mod value to pass to ModFunc that xor's A with B. */
#define LWMOD_AND 3 /*!< \brief mod value to pass to ModFunc that and's A with B. */
#define LWMOD_NOT 4 /*!< \brief mod value to pass to ModFunc that A does not have any B bits. */

/*! \brief mods A with B depending on the function specefied. */
#define LWMOD(A, B, ModFunc) switch(ModFunc){ case LWMOD_EQL: A = B; break; case LWMOD_OR: A|=B; break; case LWMOD_XOR: A^=B; break; case LWMOD_AND: A&=B; break; case LWMOD_NOT: A&=~B; break }; 

class LWByteBuffer;

class LWByteStream;

class LWFileStream;

class LWText;

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

class LWAllocator;

class LWAllocator_Default;

class LWAllocator_LocalCircular;

class LWAllocator_ConcurrentCircular;

class LWAllocator_LocalHeap;

template<class Type>
class LWFIFO;

template<class Type, uint32_t MaxElements>
class LWConcurrentFIFO;

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

/*!< \brief internal variable for Matrix4's ortho function, this will automatically be set depending on which video driver object is last created.  Side note: i really hate that i had to implement this, but I could not think of any reasonable solution since OpenGL and directX use different depth NDC ranges.) */
extern bool LWMatrix4_UseDXOrtho;

/* @} */
/*! \mainpage The Lightwave Framework Index.
	\section intro Introduction
	The lightwave framework is designed to be a relatively light weight, module based system that can be built upon.
*/


#endif