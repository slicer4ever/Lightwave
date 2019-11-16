#ifndef LWSHADER_H
#define LWSHADER_H
#include "LWCore/LWTypes.h"
#include "LWVideo/LWTypes.h"
#include <cstdarg>


/*!< \brief LWShader input descriptor. Lightwave is built around interleaving vertex data, and attempts to automatically generate the vertex input format, but if the shader compiler optimizes out an input then it will be unreliable, and should be manually specified to the pipeline(this is mostly a problem with older openGL implementations). */
struct LWShaderInput {
	enum {
		Float = 0, /*!< \brief type is a single float. */
		Int, /*!< \brief type is an int. */
		UInt, /*!< \brief type is an unsigned int. */
		Double, /*!< \brief type is an double. */
		Vec2, /*!< \brief type is 2 float's. */
		Vec3, /*!< \brief type is 3 float's. */
		Vec4, /*!< \brief type is 4 float's. */
		uVec2, /*< \brief type is 2 unsigned int's. */
		uVec3, /*!< \brief type is 3 unsigned int's. */
		uVec4, /*!< \brief type is 4 unsigned int's. */
		iVec2, /*!< \brief type is 2 int's. */
		iVec3, /*!< \brief type is 3 int's. */
		iVec4, /*!< \brief type is 4 int's. */
		dVec2, /*!< \brief type is 2 double's. */
		dVec3, /*!< \brief type is 3 double's. */
		dVec4, /*!< \brief type is 4 double's. */
		Count /*!< \brief total count list for inputs. */
	};

	/*!< \brief input for vertex shaders, creates namehash from name. offset will be automatically generated when created. */
	LWShaderInput(const char *Name, uint32_t Type, uint32_t Length);

	/*!< \brief input for vertex shaders.  offset will be automatically generated when created. */
	LWShaderInput(uint32_t NameHash, uint32_t Type, uint32_t Length);

	/*!< \brief default constructor. */
	LWShaderInput() = default;

	void *m_VideoContext = nullptr; /*!< \brief the underlying video context for the input.  This is mostly useful for openGL implementations.  */
	uint32_t m_NameHash = 0; /*!< \brief hashed name when looking up input. */
	uint32_t m_Type = 0; /*!< \brief underlying input type. */
	uint32_t m_Offset = 0; /*!< \brief offset in interleaved array for data. */
	uint32_t m_Length = 1; /*!< \brief the length of the interleaved of data. */

};

/*!< \brief shader resource/block's. */
struct LWShaderResource {
	enum {
		TypeBits = 0xFF, /*!< \brief flag bits which represent the type of the resource/block. */
		TypeBitOffset = 0x0, /*!< \brief bit offset to get just the type. */
		LengthBits = 0xFFFF00, /*!< \brief Bits representing the size of the resource(not applicable with all resource types).*/
		LengthBitOffset = 0x8, /*!< \brief bit offset to get just the length. */

		VertexStage = 0x10000000, /*!< \brief flag indicating the resource is apart of the vertex stage. */
		GeometryStage = 0x20000000, /*!< \brief flag indicating the resource is apart of the geometry stage. */
		PixelStage = 0x40000000, /*!< \brief flag indicating the resource is apart of the pixel stage. */
		ComputeStage = 0x80000000 /*!< \brief flag indicating the resource is apart of the compute stage. */
	};

	uint32_t m_NameHash = 0; /*!< \brief hashed version of the resource/block's name. */
	void *m_Resource = nullptr; /*!< \brief actual resource object(LWTexture/LWVideoBuffer. */
	void *m_VideoContext = nullptr; /*!< \brief underlying representation of the resource for the video driver.*/
	uint32_t m_Offset = 0; /*!< \brief offset of the resource if applicable. */
	uint32_t m_Flag = 0; /*!< \brief flag which contains type, length, and staging information for the resource. */

	/*!< \brief returns the type id for the resource(possible type id's found in LWPipeline). */
	uint32_t GetTypeID(void);

	/*!< \brief returns the size of the resource if applicable type. */
	uint32_t GetLength(void);

	/*!< \brief creates resource object. */
	LWShaderResource(const char *Name, uint32_t Flag, uint32_t Type, uint32_t Length);

	/*!< \brief creates resource object. */
	LWShaderResource(uint32_t NameHash, uint32_t Flag, uint32_t Type, uint32_t Length);
	
	/*!< \brief default constructor. */
	LWShaderResource() = default;
};

/*!< \brief LWShader is the compiled shader object which is used to create pipelines. */
class LWShader{
public:
	enum{
		Vertex = 0, /*!< \brief shader is a vertex type. */
		Pixel  = 1, /*!< \brief shader is a vertex type. */
		Geometry = 2, /*!< \brief shader is a vertex type. */
		Compute =  3, /*!< \brief shader is a vertex type. */
		MaxInputs = 32, /*!< \brief max input map size for shader. */
		MaxResources = 64, /*!< \brief max resource's mappable by a single stage. */
		MaxBlocks = 64 /*!< \brief max block's mappable by a single stage. */
	};

	/*!< \brief generates offset for inputs based on the inputmap provided. returns total size of mapped input. */
	static uint32_t GenerateInputOffsets(uint32_t Count, LWShaderInput *InputMap);

	/*!< \brief create's an input map list, each entry is expected to be a pair of const char * name, followed by the LWShaderInput type, followed by a integer length parameter. */
	LWShader &SetInputMap(uint32_t Count, ...);

	/*!< \brief the input map for vertex shader's, this map is user defined, as Lightwave is built around interleaved vertex formats being the norm, this means when certain graphics api's(such as openGL) optimize out unused attributes, this can create problems with how data is passed to the shader, so a user defined format needs to be correctly setup, but will be ignored with api's that don't strip out any input data(such as DirectX). 
		 \note this must be specified before any pipeline uses the shader object, otherwise the pipeline will have incorrect offset's.
	*/
	LWShader &SetInputMap(uint32_t Count, LWShaderInput *InputMap);

	/*!< \brief creates an user defined resource map list, each entry is expected to be a const char *Name. */
	LWShader &SetResourceMap(uint32_t Count, ...);

	/*!< \brief the resource map for the shader.  This map is user defined, and is used for ordering pipeline resource to bind to specific slots, shader stages that share the same name are bound to the same slot, slot order is based on shader stage order(i.e: vertex shader slots first->geometry stage->pixel stage). */
	LWShader &SetResourceMap(uint32_t Count, uint32_t *NameHashs);

	/*!< \brief creates an user defined block map list(for cbuffer's/uniform buffer block's).  Each entry is expected to be a const char *Name. */
	LWShader &SetBlockMap(uint32_t Count, ...);

	/*!< \brief The block map for the shader. This map is user defined, and is used for ordering pipeline blocks to bind to specific slots, shader stages that share the same name are bound to the same slot, slot order is based on shader stage order(i.e: vertex shader slots first->geometry stage->pixel stage). */
	LWShader &SetBlockMap(uint32_t Count, uint32_t *NameHashs);

	/*!< \brief returns the shader type. */
	uint32_t GetShaderType(void) const;

	/*!< \brief returns the underlying user defined input map table. */
	const LWShaderInput *GetInputMap(void) const;

	/*!< \brief returns the underlying user defined resource map table. */
	const LWShaderResource *GetResourceMap(void) const;

	/*!< \brief returns the underlying user defined block map table. */
	const LWShaderResource *GetBlockMap(void) const;

	/*!< \brief returns the number of user defined input's. */
	uint32_t GetInputCount(void) const;

	/*!< \brief returns the number of user defined resource map. */
	uint32_t GetResourceMapCount(void) const;

	/*!< \brief returns the number of user defined block map. */
	uint32_t GetBlockMapCount(void) const;

	/*!< \brief returns the unique shader hash. */
	uint32_t GetShaderHash(void) const;

	/*!< \brief default shader constructor. */
	LWShader(uint32_t ShaderHash, uint32_t ShaderType);
private:
	LWShaderInput m_InputMap[MaxInputs];
	LWShaderResource m_ResourceMap[MaxInputs];
	LWShaderResource m_BlockMap[MaxInputs];

	uint32_t m_InputCount = 0;
	uint32_t m_ResourceCount = 0;
	uint32_t m_BlockCount = 0;
	uint32_t m_ShaderHash = 0;
	uint32_t m_Type;
};

/*! \cond */

template<class Type>
class LWShaderCon : public LWShader {
public:
	Type GetContext(void) const {
		return m_Context;
	}

	Type &GetContext(void) {
		return m_Context;
	}
	
	LWShaderCon(Type Context, uint32_t ShaderHash, uint32_t ShaderType) : LWShader(ShaderHash, ShaderType), m_Context(Context) {}

private:
	Type m_Context;
};

/*!< \endcond */

#endif