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
	LWBitField32(TypeBits, 4, 0); // \brief bits of flag for the type. */
	LWBitField32(BindIndexBits, 4, TypeBitsOffset+4); // \brief bits of flag for BindIndex(which means max of 16 bindable inputs at the moment.)
	LWBitField32(OffsetBits, 8, BindIndexBitsOffset+4); // \brief bits of flag for offset*4 in the interleaved array.
	LWBitField32(LengthBits, 8, OffsetBitsOffset+8); // \brief bits of flag for the number of elements in the interleaved array.
	LWBitField32(InstanceFrequencyBits, 8, LengthBitsOffset+8); // \brief bits of flag for the Instance frequency(how frequently to change per-instance) (if InstanceFrequency is not 0, then this data is expected to come from an external source).

	/*!< \brief set's the flag bits for the underlying type of the input. */
	LWShaderInput &SetType(uint32_t lType);

	/*!< \brief set's the flag bits for the underlying BindIndex of the input. */
	LWShaderInput &SetBindIndex(uint32_t lBindIndex);

	/*!< \brief set's the flag bits for the underlying Offset of the input(offset must be a multiple of 4). */
	LWShaderInput &SetOffset(uint32_t lOffset);

	/*!< \brief set's the flag bits for the underlying length of the input. */
	LWShaderInput &SetLength(uint32_t lLength);

	/*!< \brief set's frequency the input changes per-instance, 0 means each vertex, 1 each instance, 2 every-2 instances, etc. */
	LWShaderInput &SetInstanceFrequency(uint32_t lFrequency);

	/*!< \brief returns the type of the input. */
	uint32_t GetType(void) const;

	/*!< \brief returns the bind index of the input. */
	uint32_t GetBindIndex(void) const;

	/*!< \brief returns the offset of the input(this offset will be multiplied by 4 to get the correct byte offset). */
	uint32_t GetOffset(void) const;

	/*!< \brief returns the length of the input. */
	uint32_t GetLength(void) const;

	/*!< \brief returns the instance frequency for the input. */
	uint32_t GetInstanceFrequency(void) const;

	/*!< \brief input for vertex shaders, creates namehash from name. offset will be automatically generated when created. */
	LWShaderInput(const LWUTF8Iterator &Name, uint32_t Type, uint32_t Length, uint32_t InstanceFreq = 0);

	/*!< \brief input for vertex shaders.  offset will be automatically generated when created. */
	LWShaderInput(uint32_t NameHash, uint32_t Type, uint32_t Length, uint32_t InstanceFreq = 0);

	/*!< \brief default constructor. */
	LWShaderInput() = default;

	uint32_t m_NameHash = LWUTF8I::EmptyHash; /*!< \brief hashed name when looking up input. */
	uint32_t m_Flag = (1<<LengthBitsOffset); /*!< \brief flag of type, bind index, and instance divisor. */

};

/*!< \brief shader resource/block's. */
struct LWShaderResource {
	static const uint32_t BindingBitCount = 5; /*!< \brief how many bits each stage has for binding's. */
	LWBitField32(TypeBits, 4, 0);
	LWBitField32(LengthBits, 16, TypeBitsOffset + 4);
	LWBitField32(VertexBindingBits, BindingBitCount, 0);
	LWBitField32(GeometryBindingBits, BindingBitCount, VertexBindingBitsOffset + BindingBitCount);
	LWBitField32(PixelBindingBits, BindingBitCount, GeometryBindingBitsOffset + BindingBitCount);
	LWBitField32(ComputeBindingBits, BindingBitCount, PixelBindingBitsOffset + BindingBitCount);

	static const uint32_t VertexStage = 0x10000000; /*!< \brief flag indicating the resource is apart of the vertex stage. */
	static const uint32_t GeometryStage = 0x20000000; /*!< \brief flag indicating the resource is apart of the geometry stage. */
	static const uint32_t PixelStage = 0x40000000; /*!< \brief flag indicating the resource is apart of the pixel stage. */
	static const uint32_t ComputeStage = 0x80000000; /*!< \brief flag indicating the resource is apart of the compute stage. */

	LWVideoResource *m_Resource = nullptr; /*!< \brief actual resource object(LWTexture/LWVideoBuffer. */
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
class LWShader : public LWVideoResource {
public:
	static const uint32_t Vertex = 0; /*!< \brief shader is a vertex type. */
	static const uint32_t Geometry = 1; /*!< \brief shader is a vertex type. */
	static const uint32_t Pixel = 2; /*!< \brief shader is a vertex type. */
	static const uint32_t Compute =  3; /*!< \brief shader is a vertex type. */
	static const uint32_t MaxInputs = 32; /*!< \brief max input map size for shader. */
	static const uint32_t MaxResources = 64; /*!< \brief max resource's mappable by a single stage. */
	static const uint32_t MaxBlocks = 64; /*!< \brief max block's mappable by a single stage. */

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
	LWShader &SetInputMap(uint32_t Count, const LWShaderInput *InputMap);

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
	LWShader &SetResourceMap(uint32_t Count, const uint32_t *NameHashs);

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
	LWShader &SetBlockMap(uint32_t Count, const uint32_t *NameHashs);

	/*!< \brief returns the shader type. */
	uint32_t GetShaderType(void) const;

	/*!< \brief returns the underlying user defined input map table. */
	const LWShaderInput *GetInputMap(void) const;

	/*!< \brief returns the underlying user defined resource map table. */
	const LWShaderResource *GetResourceMap(void) const;

	/*!< \brief returns the underlying user defined block map table. */
	const LWShaderResource *GetBlockMap(void) const;

	/*!< \brief search's input map list for the specified name.  returns null if not found. */
	const LWShaderInput *FindInputMap(const LWUTF8Iterator &Name) const;

	/*!< \brief search's input map list for the specified name.  returns null if not found. */
	const LWShaderInput *FindInputMap(uint32_t NameHash) const;

	/*!< \brief search's the resource map list for the specified name.  returns null if not found. */
	const LWShaderResource *FindResourceMap(const LWUTF8Iterator &Name) const;

	/*!< \brief search's the resource map list for the specified namehash.  returns null if not found. */
	const LWShaderResource *FindResourceMap(uint32_t NameHash) const;

	/*!< \brief search's the block map list for the specified name block.  returns null if not found. */
	const LWShaderResource *FindBlockMap(const LWUTF8Iterator &Name) const;

	/*!< \brief search's the block map list for the specified namehash block. returns null if not found. */
	const LWShaderResource *FindBlockMap(uint32_t NameHash) const;

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