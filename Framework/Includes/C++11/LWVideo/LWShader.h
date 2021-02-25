#ifndef LWSHADER_H
#define LWSHADER_H
#include "LWCore/LWTypes.h"
#include "LWVideo/LWTypes.h"
#include "LWCore/LWUnicode.h"
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
	LWShaderInput(const LWUTF8Iterator &Name, uint32_t Type, uint32_t Length);

	/*!< \brief input for vertex shaders.  offset will be automatically generated when created. */
	LWShaderInput(uint32_t NameHash, uint32_t Type, uint32_t Length);

	/*!< \brief default constructor. */
	LWShaderInput() = default;

	uint32_t m_BindIndex = 0; /*!< \brief underlying bind location for the input. This is mostly useful for openGL implementations.  */
	uint32_t m_NameHash = 0; /*!< \brief hashed name when looking up input. */
	uint32_t m_Type = 0; /*!< \brief underlying input type. */
	uint32_t m_Offset = 0; /*!< \brief offset in interleaved array for data. */
	uint32_t m_Length = 1; /*!< \brief the length of the interleaved of data. */

};

/*!< \brief shader resource/block's. */
struct LWShaderResource {
	static const uint32_t BindingBitCount = 5; /*!< \brief how many bits each stage has for binding's. */
	LWBitField32(Type, 4, 0);
	LWBitField32(Length, 16, TypeBitsOffset + 4);
	LWBitField32(VertexBinding, BindingBitCount, 0);
	LWBitField32(GeometryBinding, BindingBitCount, VertexBindingBitsOffset + BindingBitCount);
	LWBitField32(PixelBinding, BindingBitCount, GeometryBindingBitsOffset + BindingBitCount);
	LWBitField32(ComputeBinding, BindingBitCount, PixelBindingBitsOffset + BindingBitCount);

	static const uint32_t VertexStage = 0x10000000; /*!< \brief flag indicating the resource is apart of the vertex stage. */
	static const uint32_t GeometryStage = 0x20000000; /*!< \brief flag indicating the resource is apart of the geometry stage. */
	static const uint32_t PixelStage = 0x40000000; /*!< \brief flag indicating the resource is apart of the pixel stage. */
	static const uint32_t ComputeStage = 0x80000000; /*!< \brief flag indicating the resource is apart of the compute stage. */

	void *m_Resource = nullptr; /*!< \brief actual resource object(LWTexture/LWVideoBuffer. */
	uint32_t m_NameHash = 0; /*!< \brief hashed version of the resource/block's name. */
	uint32_t m_Offset = 0; /*!< \brief offset of the resource if applicable. */
	uint32_t m_Flag = 0; /*!< \brief flag which contains type, length, and staging information for the resource. */
	uint32_t m_StageBindings = 0; /*!< \brief binding index for resource in the shader per stage. these values are used internally by the pipeline, and shouldn't be modified by the application. */

	/*!< \brief set's the stage bind by the stageID(Vertex/Compute=0, Geometry=1, Pixel=2). automatically raises the relevant stage's flag. */
	LWShaderResource &SetStageBinding(uint32_t StageID, uint32_t Idx);

	/*!< \brief set's the stage bind index for the vertex stage. automatically raises the vertex stage's flag. */
	LWShaderResource &SetVertexStageBinding(uint32_t Idx);

	/*!< \brief set's the stage bind index for the compute stage. automatically raises the compute stage's flag. */
	LWShaderResource &SetComputeStageBinding(uint32_t Idx);

	/*!< \brief set's the stage bind index for the pixel stage. automatically raises the pixel stage's flag. */
	LWShaderResource &SetPixelStageBinding(uint32_t Idx);

	/*!< \brief set's the stage bind index for the geometry stage. automatically raises the geometry stage's flag. */
	LWShaderResource &SetGeometryStageBinding(uint32_t Idx);

	/*!< \brief returns the type id for the resource(possible type id's found in LWPipeline). */
	uint32_t GetTypeID(void) const;

	/*!< \brief returns the size of the resource if applicable type. */
	uint32_t GetLength(void) const;

	/*!< \brief returns the binding index for vertex stage. */
	uint32_t GetVertexStageBinding(void) const;

	/*!< \brief returns the binding index for the specified stageID(Vertex=0, Geometry=1, Pixel=2, Compute=3) */
	uint32_t GetStageBinding(uint32_t StageID) const;

	/*!< \brief returns the binding index for the compute stage. */
	uint32_t GetComputeStageBinding(void) const;

	/*!< \brief returns the binding index for pixel stage. */
	uint32_t GetPixelStageBinding(void) const;

	/*!< \brief returns the binding index for geometry stage. */
	uint32_t GetGeometryStageBinding(void) const;

	/*!< \brief returns true if the stageID's flag is raised. */
	bool HasStage(uint32_t StageID) const;

	/*!< \brief returns true if the vertex stage flag is raised. */
	bool hasVertexStage(void) const;

	/*!< \brief returns true if the compute stage flag is raised. */
	bool hasComputeStage(void) const;

	/*!< \brief returns true if the pixel stage flag is raised. */
	bool hasPixelStage(void) const;

	/*!< \brief returns true if the geometry stage flag is raised. */
	bool hasGeometryStage(void) const;

	/*!< \brief creates resource object. */
	LWShaderResource(const LWUTF8Iterator &Name, uint32_t Type, uint32_t Length);

	/*!< \brief creates resource object. */
	LWShaderResource(uint32_t NameHash, uint32_t Type, uint32_t Length);

	/*!< \brief used internally to create a pipeline resource with the bound stage+stageBindIdx. raising the relevant stage's flag as well. */
	LWShaderResource(uint32_t NameHash, uint32_t Type, uint32_t Length, uint32_t StageID, uint32_t StageBindIdx);
	
	/*!< \brief used intenally by opengl to create a pipeline resource with the stageBindIdx.  opengl does not utilize the individual stage bindings per shader, as such StageBindings is a global property of the entire program. */
	LWShaderResource(uint32_t NameHash, uint32_t Type, uint32_t Length, uint32_t StageBindIdx); 

	/*!< \brief default constructor. */
	LWShaderResource() = default;
};

/*!< \brief LWShader is the compiled shader object which is used to create pipelines. */
class LWShader{
public:
	enum{
		Vertex = 0, /*!< \brief shader is a vertex type. */
		Geometry = 1, /*!< \brief shader is a vertex type. */
		Pixel = 2, /*!< \brief shader is a vertex type. */
		Compute =  3, /*!< \brief shader is a vertex type. */
		MaxInputs = 32, /*!< \brief max input map size for shader. */
		MaxResources = 64, /*!< \brief max resource's mappable by a single stage. */
		MaxBlocks = 64 /*!< \brief max block's mappable by a single stage. */
	};

	/*!< \brief generates offset for inputs based on the inputmap provided. returns total size of mapped input. */
	static uint32_t GenerateInputOffsets(uint32_t Count, LWShaderInput *InputMap);

	/*!< \brief base template variatable packing for creating an LWShaderInput list. */
	template<uint32_t N=0>
	LWShader &SetInputMapList(const LWUTF8Iterator &Name, uint32_t Type, uint32_t Length) {
		if (N >= MaxInputs) return *this;
		m_InputMap[N] = LWShaderInput(Name, Type, Length);
		m_InputCount = N + 1;
		GenerateInputOffsets(m_InputCount, m_InputMap);
		return *this;
	}

	/*!< \brief create's an input map list, each entry is expected to be a pair of const char * name, followed by the LWShaderInput type, followed by a integer length parameter. */
	template<uint32_t N=0, typename ...Args>
	LWShader &SetInputMapList(const LWUTF8Iterator &Name, uint32_t Type, uint32_t Length, Args... Pack) {
		if (N >= MaxInputs) return *this;
		m_InputMap[N] = LWShaderInput(Name, Type, Length);
		return SetInputMapList<N+1>(Pack...);
	}

	/*!< \brief the input map for vertex shader's, this map is user defined, as Lightwave is built around interleaved vertex formats being the norm, this means when certain graphics api's(such as openGL) optimize out unused attributes, this can create problems with how data is passed to the shader, so a user defined format needs to be correctly setup, but will be ignored with api's that don't strip out any input data(such as DirectX). 
		 \note this must be specified before any pipeline uses the shader object, otherwise the pipeline will have incorrect offset's.
	*/
	LWShader &SetInputMap(uint32_t Count, LWShaderInput *InputMap);

	/*!< \brief base tempalte varitable for resource map list. */
	template<uint32_t N=0>
	LWShader &SetResourceMapList(const LWUTF8Iterator &Name) {
		if (N >= MaxInputs) return *this;
		m_ResourceMap[N].m_NameHash = Name.Hash();
		m_ResourceCount = N + 1;
		return *this;
	}

	/*!< \brief creates an user defined resource map list from template variable arguments. */
	template<uint32_t N=0, typename ...Args>
	LWShader &SetResourceMapList(const LWUTF8Iterator &Name, Args... Pack) {
		if (N >= MaxInputs) return *this;
		m_ResourceMap[N].m_NameHash = Name.Hash();
		return SetResourceMapList<N + 1>(Pack...);
	}

	/*!< \brief the resource map for the shader.  This map is user defined, and is used for ordering pipeline resource to bind to specific slots, shader stages that share the same name are bound to the same slot, slot order is based on shader stage order(i.e: vertex shader slots first->geometry stage->pixel stage). */
	LWShader &SetResourceMap(uint32_t Count, uint32_t *NameHashs);

	/*!< \brief base block map list for variatic template. */
	template<uint32_t N=0>
	LWShader &SetBlockMapList(const LWUTF8Iterator &Name) {
		if (N >= MaxInputs) return *this;
		m_BlockMap[N].m_NameHash = Name.Hash();
		m_BlockCount = N + 1;
		return *this;
	}
	
	/*!< \brief creates an user defined block map list(for cbuffer's/uniform buffer block's). */
	template<uint32_t N=0, typename ...Args>
	LWShader &SetBlockMapList(const LWUTF8Iterator &Name, Args ...Pack) {
		if (N >= MaxInputs) return *this;
		m_BlockMap[N].m_NameHash = Name.Hash();
		return SetBlockMapList(Pack...);
	}

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