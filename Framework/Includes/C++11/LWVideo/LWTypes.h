#ifndef LWVIDEOTYPES_H
#define LWVIDEOTYPES_H

/*! \defgroup LWVideo LWVideo
	\brief the main graphics component of the LWFramework.
	@{
*/


/*!< \brief base format that all LWVideo resources(LWVideoBuffer, LWTexture, LWShader, LWPipeline, LWFrameBuffer), inherit to identify them when passed to a pipeline to prevent incorrect type mapping. */
class LWVideoResource {
public:
	/*!< \brief dynamic cast the resource to the specified type, returns null if this is not that type. */
	template<class Type>
	Type *As(void) {
		return dynamic_cast<Type*>(this);
	}

	/*!< \brief virtual destructor to make class polymorphic. */
	virtual ~LWVideoResource() {}
};


class LWImage;

class LWFrameBuffer;

template<class Type>
class LWFrameBufferCon;

class LWVideoState;

template<class Type>
class LWVideoStateCon;

class LWTexture;

template<class Type>
class LWTextureCon;

class LWVideoBuffer;

template<class Type>
class LWVideoBufferCon;

class LWPipeline;

template<class Type>
class LWPipelineCon;

class LWFrameBuffer;

class LWVideoState;

class LWBaseMesh;

template<class VertexType>
class LWMesh;

class LWShader;

template<class Type>
class LWShaderCon;

struct LWGlyph;

class LWFont;

class LWVideoDriver;
/*@} */
#endif