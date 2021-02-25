#include "LWVideo/LWVideoDrivers/LWVideoDriver_OpenGL4_5.h"
#include "LWPlatform/LWWindow.h"
#include "LWCore/LWVector.h"
#include "LWCore/LWMatrix.h"
#include "LWVideo/LWPipeline.h"
#include "LWVideo/LWImage.h"
#include "LWVideo/LWFrameBuffer.h"
#include <iostream>

LWVideoDriver &LWVideoDriver_OpenGL4_5::ViewPort(const LWVector4i &Viewport) {
	m_Viewport = Viewport;
	glViewport(Viewport.x, Viewport.y, Viewport.z, Viewport.w);
	return *this;
}

LWShader *LWVideoDriver_OpenGL4_5::CreateShader(uint32_t ShaderType, const LWUTF8Iterator &Source, LWAllocator &Allocator, char *CompiledBuffer, char8_t *ErrorBuffer, uint32_t &CompiledBufferLen, uint32_t ErrorBufferLen) {
	GLenum GShaderTypes[] = { GL_VERTEX_SHADER, GL_GEOMETRY_SHADER, GL_FRAGMENT_SHADER, GL_COMPUTE_SHADER };
	int32_t CompileResult = 0;
	int32_t LinkResult = 0;
	int32_t Len = Source.RawDistance(Source.NextEnd());
	const char *Src = Source();
	uint32_t ShaderID = glCreateShader(GShaderTypes[ShaderType]);
	if (!ShaderID) return nullptr;

	glShaderSource(ShaderID, 1, &Src, &Len);
	glCompileShader(ShaderID);
	glGetShaderiv(ShaderID, GL_COMPILE_STATUS, &CompileResult);
	if (!CompileResult) {
		if (ErrorBuffer) {
			*ErrorBuffer = '\0';
			glGetShaderInfoLog(ShaderID, ErrorBufferLen, &Len, ErrorBuffer);
			PerformErrorAnalysis(Source, ErrorBuffer, ErrorBufferLen);
		}
		glDeleteShader(ShaderID);
		return nullptr;
	}
	uint32_t ProgramID = glCreateProgram();
	if (!ProgramID) {
		glDeleteShader(ShaderID);
		return nullptr;
	}
	glProgramParameteri(ProgramID, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, CompiledBuffer?GL_TRUE:GL_FALSE);
	glProgramParameteri(ProgramID, GL_PROGRAM_SEPARABLE, GL_TRUE);
	glAttachShader(ProgramID, ShaderID);
	glLinkProgram(ProgramID);
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &LinkResult);
	glDetachShader(ProgramID, ShaderID);
	glDeleteShader(ShaderID);

	if (!LinkResult) {
		glGetProgramInfoLog(ProgramID, ErrorBufferLen, &Len, ErrorBuffer);
		glDeleteProgram(ProgramID);
		PerformErrorAnalysis(Source, ErrorBuffer, ErrorBufferLen);
		return nullptr;
	}
	if (CompiledBuffer) {
		int32_t Length = CompiledBufferLen - sizeof(GLenum);
		glGetProgramBinary(ProgramID, Length, &Len, (GLenum*)CompiledBuffer, CompiledBuffer + sizeof(GLenum));
		CompiledBufferLen = Len + sizeof(GLenum);
	}
	return Allocator.Create<LWOpenGL4_5Shader>(ProgramID, Source.Hash(), ShaderType);
}

LWShader *LWVideoDriver_OpenGL4_5::CreateShaderCompiled(uint32_t ShaderType, const char *CompiledCode, uint32_t CompiledCodeLen, LWAllocator &Allocator, char8_t *ErrorBuffer, uint32_t ErrorBufferLen) {
	GLenum GShaderTypes[] = { GL_VERTEX_SHADER, GL_GEOMETRY_SHADER, GL_FRAGMENT_SHADER, GL_COMPUTE_SHADER };
	int32_t LinkResult = 0;
	int32_t Len = 0;
	uint32_t ProgramID = glCreateProgram();
	if (!ProgramID) return nullptr;
	GLenum Format = *(GLenum*)CompiledCode;
	glProgramParameteri(ProgramID, GL_PROGRAM_SEPARABLE, GL_TRUE);
	glProgramBinary(ProgramID, Format, CompiledCode + sizeof(GLenum), CompiledCodeLen - sizeof(GLenum));
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &LinkResult);
	if (!LinkResult) {
		glGetProgramInfoLog(ProgramID, ErrorBufferLen, &Len, ErrorBuffer);
		glDeleteProgram(ProgramID);
		PerformErrorAnalysis(CompiledCode, ErrorBuffer, ErrorBufferLen);
		return nullptr;
	}
	return Allocator.Create<LWOpenGL4_5Shader>(ProgramID, LWCrypto::HashFNV1A((const uint8_t*)CompiledCode, CompiledCodeLen), ShaderType);
}

LWPipeline *LWVideoDriver_OpenGL4_5::CreatePipeline(LWShader **Stages, uint64_t Flag, LWAllocator &Allocator) {
	LWOpenGL4_5PipelineContext Context;
	LWOpenGL4_5Pipeline *P = Allocator.Create<LWOpenGL4_5Pipeline>(Context, Stages, nullptr, nullptr, nullptr, 0, 0, 0, Flag&~LWPipeline::InternalPipeline);
	if(P) UpdatePipelineStages(P);
	return P;
}

LWVideoDriver &LWVideoDriver_OpenGL4_5::ClonePipeline(LWPipeline *Target, LWPipeline *Source) {
	LWVideoDriver::ClonePipeline(Target, Source);
	LWOpenGL4_5PipelineContext &SourceCon = ((LWOpenGL4_5Pipeline*)Source)->GetContext();
	LWOpenGL4_5PipelineContext &TargetCon = ((LWOpenGL4_5Pipeline*)Target)->GetContext();
	TargetCon = SourceCon;
	return *this;
}

LWPipeline *LWVideoDriver_OpenGL4_5::CreatePipeline(LWPipeline *Source, LWAllocator &Allocator){
	const int32_t AttributePropCnt = 3;
	const uint32_t AttributeProps[AttributePropCnt] = { GL_TYPE, GL_ARRAY_SIZE, GL_LOCATION };
	int32_t AttributeValues[AttributePropCnt];
	char NameBuffer[1024];
	LWShader *StageList[LWPipeline::StageCount] = { Source->GetShaderStage(0), Source->GetShaderStage(1), Source->GetShaderStage(2) };
	LWShaderResource Blocks[LWShader::MaxBlocks];
	LWShaderResource Resources[LWShader::MaxResources];
	LWShaderInput Inputs[LWShader::MaxInputs];
	LWOpenGL4_5PipelineContext Context;
	uint32_t BlockCount = 0;
	uint32_t ResourceCount = 0;
	uint32_t InputCount = 0;
	int32_t NameLen = 0;
	int32_t Len = 0;
	int32_t NextTexIdx = 0;

	struct AttributeMap {
		uint32_t AttributeType; //The glVertexAttribPointer underlying type(GL_FLOAT, GL_INT, etc)
		uint32_t RowMultiplier; //Multiples the input count for matrix attribute types based on number of matrix rows.
	};

	auto MapAttributeType = [](uint32_t Type) -> AttributeMap {
		if (Type == GL_FLOAT) return { LWShaderInput::Float, 1 };
		else if (Type == GL_FLOAT_VEC2) return { LWShaderInput::Vec2, 1 };
		else if (Type == GL_FLOAT_VEC3) return { LWShaderInput::Vec3, 1 };
		else if (Type == GL_FLOAT_VEC4) return { LWShaderInput::Vec4, 1 };
		else if (Type == GL_INT) return { LWShaderInput::Int, 1 };
		else if (Type == GL_INT_VEC2) return { LWShaderInput::iVec2, 1 };
		else if (Type == GL_INT_VEC3) return { LWShaderInput::iVec3, 1 };
		else if (Type == GL_INT_VEC4) return { LWShaderInput::iVec4, 1 };
		else if (Type == GL_UNSIGNED_INT) return { LWShaderInput::UInt, 1 };
		else if (Type == GL_UNSIGNED_INT_VEC2) return { LWShaderInput::uVec2, 1 };
		else if (Type == GL_UNSIGNED_INT_VEC3) return { LWShaderInput::uVec3, 1 };
		else if (Type == GL_UNSIGNED_INT_VEC4) return { LWShaderInput::uVec4, 1 };
		else if (Type == GL_DOUBLE) return { LWShaderInput::Double, 1 };
		else if (Type == GL_DOUBLE_VEC2) return { LWShaderInput::dVec2, 1 };
		else if (Type == GL_DOUBLE_VEC3) return { LWShaderInput::dVec3 , 1 };
		else if (Type == GL_DOUBLE_VEC4) return { LWShaderInput::dVec4, 1 };
		else if (Type == GL_FLOAT_MAT2) return { LWShaderInput::Vec2, 2 };
		else if (Type == GL_FLOAT_MAT3) return { LWShaderInput::Vec3, 3 };
		else if (Type == GL_FLOAT_MAT4) return { LWShaderInput::Vec4,4 };
		else if (Type == GL_FLOAT_MAT2x3) return { LWShaderInput::Vec2, 3 };
		else if (Type == GL_FLOAT_MAT2x4) return { LWShaderInput::Vec2, 4 };
		else if (Type == GL_FLOAT_MAT3x2) return { LWShaderInput::Vec3, 2 };
		else if (Type == GL_FLOAT_MAT3x4) return { LWShaderInput::Vec3, 4 };
		else if (Type == GL_FLOAT_MAT4x2) return { LWShaderInput::Vec4, 2 };
		else if (Type == GL_FLOAT_MAT4x3) return { LWShaderInput::Vec4, 3 };
		else if (Type == GL_DOUBLE_MAT2) return { LWShaderInput::dVec2, 2 };
		else if (Type == GL_DOUBLE_MAT3) return { LWShaderInput::dVec3, 3 };
		else if (Type == GL_DOUBLE_MAT4) return { LWShaderInput::dVec4,4 };
		else if (Type == GL_DOUBLE_MAT2x3) return { LWShaderInput::dVec2, 3 };
		else if (Type == GL_DOUBLE_MAT2x4) return { LWShaderInput::dVec2, 4 };
		else if (Type == GL_DOUBLE_MAT3x2) return { LWShaderInput::dVec3, 2 };
		else if (Type == GL_DOUBLE_MAT3x4) return { LWShaderInput::dVec3, 4 };
		else if (Type == GL_DOUBLE_MAT4x2) return { LWShaderInput::dVec4, 2 };
		else if (Type == GL_DOUBLE_MAT4x3) return { LWShaderInput::dVec4, 3 };
		fmt::print("Unknown type: {}\n", Type);
		return { LWShaderInput::Float, 1 };
	};

	auto InsertList = [](const LWUTF8Iterator &Name, uint32_t BindIdx, uint32_t Type, uint32_t Length, LWShaderResource *List, uint32_t &Cnt) ->int32_t {
		uint32_t Hash = Name.Hash();
		for (uint32_t i = 0; i < Cnt; i++) {
			if (List[i].m_NameHash == Hash) {
				List[i].m_StageBindings = i;
				return (int32_t)i;
			}
		}
		LWShaderResource &L = List[Cnt];
		L = LWShaderResource(Hash, Type, Length, BindIdx);
		Cnt++;
		return (int32_t)(Cnt - 1);
	};

	auto ReflectShader = [&InsertList, &Blocks, &Resources, &BlockCount, &ResourceCount, &NextTexIdx](int32_t ProgramID) {
		const int32_t UniformPropCnt = 3;
		const int32_t BlockPropCnt = 2;
		const int32_t MaxProps = 32;
		const uint32_t UniformProps[UniformPropCnt] = { GL_TYPE, GL_ARRAY_SIZE, GL_LOCATION };
		const uint32_t BlockProps[BlockPropCnt] = { GL_BUFFER_BINDING, GL_BUFFER_DATA_SIZE };
		char NameBuffer[1024];
		int32_t PropValues[MaxProps];
		int32_t UniformCnt = 0;
		int32_t UniformBlockCnt = 0;
		int32_t ShaderStorageCnt = 0;
		int32_t Len = 0;
		glGetProgramInterfaceiv(ProgramID, GL_UNIFORM, GL_ACTIVE_RESOURCES, &UniformCnt);
		glGetProgramInterfaceiv(ProgramID, GL_UNIFORM_BLOCK, GL_ACTIVE_RESOURCES, &UniformBlockCnt);
		glGetProgramInterfaceiv(ProgramID, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &ShaderStorageCnt);

		for (int32_t i = 0; i < UniformCnt; i++) {
			glGetProgramResourceName(ProgramID, GL_UNIFORM, i, sizeof(NameBuffer), nullptr, NameBuffer);
			glGetProgramResourceiv(ProgramID, GL_UNIFORM, i, UniformPropCnt, UniformProps, sizeof(PropValues), &Len, PropValues);

			int32_t Type = PropValues[0];
			int32_t Length = PropValues[1];
			int32_t glLoc = PropValues[2];
			uint32_t Loc = NextTexIdx;
			uint32_t pTexIdx = NextTexIdx;
			if (Type == GL_SAMPLER_1D || Type == GL_SAMPLER_2D || Type == GL_SAMPLER_3D ||
				Type == GL_SAMPLER_CUBE || Type == GL_SAMPLER_1D_SHADOW || Type == GL_SAMPLER_2D_SHADOW ||
				Type == GL_SAMPLER_1D_ARRAY || Type == GL_SAMPLER_2D_ARRAY || Type == GL_SAMPLER_1D_ARRAY_SHADOW ||
				Type == GL_SAMPLER_2D_ARRAY_SHADOW || Type == GL_SAMPLER_CUBE_SHADOW ||
				Type == GL_SAMPLER_CUBE_MAP_ARRAY || Type == GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW ||
				Type == GL_SAMPLER_2D_MULTISAMPLE || Type == GL_SAMPLER_2D_MULTISAMPLE_ARRAY) {
				Loc = InsertList(NameBuffer, Loc, LWPipeline::Texture, Length, Resources, ResourceCount);
				glProgramUniform1i(ProgramID, glLoc, Loc);
				if (Loc == pTexIdx) NextTexIdx++;
			} else if (Type == GL_IMAGE_1D || Type == GL_IMAGE_2D || Type == GL_IMAGE_3D || Type == GL_IMAGE_CUBE || Type == GL_IMAGE_BUFFER) {
				Loc = InsertList(NameBuffer, Loc, LWPipeline::Image, Length, Resources, ResourceCount);
				glProgramUniform1i(ProgramID, glLoc, Loc);
				if (Loc == pTexIdx) NextTexIdx++;
			}
		}

		for (int32_t i = 0; i < UniformBlockCnt; i++) {
			glGetProgramResourceName(ProgramID, GL_UNIFORM_BLOCK, i, sizeof(NameBuffer), nullptr, NameBuffer);
			glGetProgramResourceiv(ProgramID, GL_UNIFORM_BLOCK, i, BlockPropCnt, BlockProps, sizeof(PropValues), &Len, PropValues);
			int32_t Loc = PropValues[0];
			int32_t Size = PropValues[1];
			int32_t BlockIdx = InsertList(NameBuffer, BlockCount, LWPipeline::UniformBlock, Size, Blocks, BlockCount);
			glUniformBlockBinding(ProgramID, i, BlockIdx);
		}

		for (int32_t i = 0; i < ShaderStorageCnt; i++) {
			glGetProgramResourceName(ProgramID, GL_SHADER_STORAGE_BLOCK, i, sizeof(NameBuffer), nullptr, NameBuffer);
			glGetProgramResourceiv(ProgramID, GL_SHADER_STORAGE_BLOCK, i, BlockPropCnt, BlockProps, sizeof(PropValues), &Len, PropValues);
			int32_t Loc = PropValues[0];
			int32_t Size = PropValues[1];
			InsertList(NameBuffer, Loc, LWPipeline::ImageBuffer, Size, Resources, ResourceCount);
		}
	};
	glGenProgramPipelines(1, &Context.m_ProgramID);
	if (!Context.m_ProgramID) return nullptr;
	glGenVertexArrays(1, &Context.m_VAOID);
	if (!Context.m_VAOID) return nullptr;
	glBindVertexArray(Context.m_VAOID);
	if (Source->isComputePipeline()) {
		uint32_t ComputeID = ((LWOpenGL4_5Shader*)StageList[LWPipeline::Compute])->GetContext();
		glUseProgramStages(Context.m_ProgramID, GL_COMPUTE_SHADER_BIT, ComputeID);
		ReflectShader(ComputeID);
		return Allocator.Create<LWOpenGL4_5Pipeline>(Context, StageList, Blocks, Resources, nullptr, BlockCount, ResourceCount, 0, LWPipeline::InternalPipeline | LWPipeline::ComputePipeline);
	}

	if (StageList[LWPipeline::Vertex]) {
		uint32_t VertexID = ((LWOpenGL4_5Shader*)StageList[LWPipeline::Vertex])->GetContext();
		glUseProgramStages(Context.m_ProgramID, GL_VERTEX_SHADER_BIT, VertexID);
		int32_t AttributeCnt = 0;
	
		glGetProgramInterfaceiv(VertexID, GL_PROGRAM_INPUT, GL_ACTIVE_RESOURCES, &AttributeCnt);
		for (int32_t i = 0; i < AttributeCnt; i++) {
			int32_t NameLen = 0;
			glGetProgramResourceiv(VertexID, GL_PROGRAM_INPUT, i, AttributePropCnt, AttributeProps, sizeof(AttributeValues), &Len, AttributeValues);
			glGetProgramResourceName(VertexID, GL_PROGRAM_INPUT, i, sizeof(NameBuffer), &NameLen, NameBuffer);
			if(AttributeValues[2]==-1) continue;
			AttributeMap Map = MapAttributeType(AttributeValues[0]);
			for (uint32_t n = 0; n < Map.RowMultiplier*AttributeValues[1]; n++) {
				LWShaderInput &In = Inputs[InputCount];
				In = LWShaderInput(NameBuffer, Map.AttributeType, 1);
				In.m_BindIndex = AttributeValues[2];
				glEnableVertexAttribArray(In.m_BindIndex);
				InputCount++;
			}
		}
		ReflectShader(VertexID);
	}

	if(StageList[LWPipeline::Geometry]){
		uint32_t GeomID = ((LWOpenGL4_5Shader*)StageList[LWPipeline::Geometry])->GetContext();
		glUseProgramStages(Context.m_ProgramID, GL_GEOMETRY_SHADER_BIT, GeomID);
		ReflectShader(GeomID);
	}

	if (StageList[LWPipeline::Pixel]) {
		uint32_t PixelID = ((LWOpenGL4_5Shader*)StageList[LWPipeline::Pixel])->GetContext();
		glUseProgramStages(Context.m_ProgramID, GL_FRAGMENT_SHADER_BIT, PixelID);
		ReflectShader(PixelID);
	}

	return Allocator.Create<LWOpenGL4_5Pipeline>(Context, StageList, Blocks, Resources, Inputs, BlockCount, ResourceCount, InputCount, LWPipeline::InternalPipeline);
}

LWTexture *LWVideoDriver_OpenGL4_5::CreateTexture1D(uint32_t TextureState, uint32_t PackType, uint32_t Size, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator) {
	//PackTypes:                         RGBA8,            RGBA8U,           RGBA16,            RGBA16,            RGBA32,          RGBA32U,         RGBA32F,    RG8,              RG8U,             RG16,              RG16U,             RG32,            RG32U,           RG32F,    R8,               R8U,              R16,               R16U,              R32,             R32U,            R32F,     Depth16, Depth24,     Depth32,              Depth24Stencil8,      DXT1, DXT2, DXT3, DXT4, DXT5, DXT6, DXT7
	const int32_t GInternalFormats[] = { GL_RGBA8,         GL_RGBA8,         GL_RGBA16,         GL_RGBA16,         GL_RGBA32I,      GL_RGBA32UI,     GL_RGBA32F, GL_RG8,           GL_RG8,           GL_RG16,           GL_RG16,           GL_RG32I,        GL_RG32UI,       GL_RG32F, GL_R8,            GL_R8,            GL_R16,            GL_R16,            GL_R32I,         GL_R32UI,        GL_R32F,  0,       0,           0,                    0,                    0,    0,    0,    0,    0,    0,    0 };
	const int32_t GFormats[]         = { GL_RGBA,          GL_RGBA,          GL_RGBA,           GL_RGBA,           GL_RGBA,         GL_RGBA,         GL_RGBA,    GL_RG,            GL_RG,            GL_RG,             GL_RG,             GL_RG,           GL_RG,           GL_RG,    GL_RED,           GL_RED,           GL_RED,            GL_RED,            GL_RED,          GL_RED,          GL_RED,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_STENCIL };
	const int32_t GType[]            = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT,   GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_FLOAT,             GL_FLOAT,             GL_FLOAT,             GL_UNSIGNED_INT_24_8 };
	if (GInternalFormats[PackType] == 0) return nullptr;

	uint32_t VideoID = 0;
	uint32_t Type = 0;
	bool MakeMipmaps = (TextureState&LWTexture::MakeMipmaps) != 0;
	bool Compressed = LWImage::CompressedType(PackType);
	if (MakeMipmaps) MipmapCnt = Compressed ? 0 : LWImage::MipmapCount(Size);
	TextureState = (TextureState&~LWTexture::MakeMipmaps);
	glGenTextures(1, &VideoID);
	glBindTexture(GL_TEXTURE_1D, VideoID);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAX_LEVEL, MipmapCnt);
	glTexStorage1D(GL_TEXTURE_1D, MipmapCnt + 1, GInternalFormats[PackType], Size);
	if (!Texels) return Allocator.Create<LWOpenGL4_5Texture>(VideoID, TextureState, PackType, MipmapCnt, LWVector3i(Size, 0, 0), LWTexture::Texture1D);
	uint32_t Mips = MakeMipmaps ? 0 : MipmapCnt;
	uint32_t t = 0;
	for (uint32_t i = 0; i <= Mips; i++, t++) {
		uint32_t S = LWImage::MipmapSize1D(Size, i);
		glTexSubImage1D(GL_TEXTURE_1D, i, 0, S, GFormats[PackType], GType[PackType], Texels[t]);
	}
	//if (MakeMipmaps && !Compressed) glGenerateMipmap(GL_TEXTURE_1D);
	return Allocator.Create<LWOpenGL4_5Texture>(VideoID, TextureState, PackType, MipmapCnt, LWVector3i(Size, 0, 0), LWTexture::Texture1D);
}

LWTexture *LWVideoDriver_OpenGL4_5::CreateTexture2D(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator) {
	//PackTypes:                         RGBA8,            RGBA8U,           RGBA16,            RGBA16,            RGBA32,          RGBA32U,         RGBA32F,    RG8,              RG8U,             RG16,              RG16U,             RG32,            RG32U,           RG32F,    R8,               R8U,              R16,               R16U,              R32,             R32U,            R32F,     Depth16,              Depth24,              Depth32,               Depth24Stencil8,     DXT1,                            DXT2,                             DXT3                              DXT4 DXT5                              DXT6                                     DXT7
	const int32_t GInternalFormats[] = { GL_RGBA8,         GL_RGBA8,         GL_RGBA16,         GL_RGBA16,         GL_RGBA32I,      GL_RGBA32UI,     GL_RGBA32F, GL_RG8,           GL_RG8,           GL_RG16,           GL_RG16,           GL_RG32I,        GL_RG32UI,       GL_RG32F, GL_R8,            GL_R8,            GL_R16,            GL_R16,            GL_R32I,         GL_R32UI,        GL_R32F,  GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32F, GL_DEPTH24_STENCIL8, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0,   GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT };
	const int32_t GFormats[]         = { GL_RGBA,          GL_RGBA,          GL_RGBA,           GL_RGBA,           GL_RGBA,         GL_RGBA,         GL_RGBA,    GL_RG,            GL_RG,            GL_RG,             GL_RG,             GL_RG,           GL_RG,           GL_RG,    GL_RED,           GL_RED,           GL_RED,            GL_RED,            GL_RED,          GL_RED,          GL_RED,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,    GL_DEPTH_STENCIL };
	const int32_t GType[]            = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT,   GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_FLOAT,             GL_FLOAT,             GL_FLOAT,              GL_UNSIGNED_INT_24_8 };
	if (GInternalFormats[PackType] == 0) return nullptr;

	uint32_t VideoID = 0;
	uint32_t Type = 0;
	if (TextureState&LWTexture::RenderBuffer) {
		glGenRenderbuffers(1, &VideoID);
		glBindRenderbuffer(GL_RENDERBUFFER, VideoID);
		glRenderbufferStorage(GL_RENDERBUFFER, GInternalFormats[PackType], Size.x, Size.y);
		return Allocator.Create<LWOpenGL4_5Texture>(VideoID, (TextureState&~(LWTexture::MakeMipmaps)), PackType, 0, LWVector3i(Size, 0), LWTexture::Texture2D);
	}
	bool MakeMipmaps = (TextureState&LWTexture::MakeMipmaps) != 0;
	bool Compressed = LWImage::CompressedType(PackType);
	if (MakeMipmaps) MipmapCnt = Compressed ? 0 : LWImage::MipmapCount(Size);
	TextureState = (TextureState&~LWTexture::MakeMipmaps);
	glGenTextures(1, &VideoID);
	glBindTexture(GL_TEXTURE_2D, VideoID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, MipmapCnt);
	glTexStorage2D(GL_TEXTURE_2D, MipmapCnt+1, GInternalFormats[PackType], Size.x, Size.y);
	if (!Texels) return Allocator.Create<LWOpenGL4_5Texture>(VideoID, TextureState, PackType, MipmapCnt, LWVector3i(Size, 0), LWTexture::Texture2D);
	uint32_t Mips = MakeMipmaps ? 0 : MipmapCnt;
	uint32_t t = 0;
	for (uint32_t i = 0; i <= Mips; i++, t++) {
		LWVector2i S = LWImage::MipmapSize2D(Size, i);
		if (Compressed) glCompressedTexSubImage2D(GL_TEXTURE_2D, i, 0, 0, S.x, S.y, GInternalFormats[PackType], LWImage::GetLength2D(S, PackType), Texels[t]);
		else glTexSubImage2D(GL_TEXTURE_2D, i, 0, 0, S.x, S.y, GFormats[PackType], GType[PackType], Texels[t]);
	}
	//if (MakeMipmaps && !Compressed) glGenerateMipmap(GL_TEXTURE_2D);
	return Allocator.Create<LWOpenGL4_5Texture>(VideoID, TextureState, PackType, MipmapCnt, LWVector3i(Size, 0), LWTexture::Texture2D);
}

LWTexture *LWVideoDriver_OpenGL4_5::CreateTexture3D(uint32_t TextureState, uint32_t PackType, const LWVector3i &Size, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator) {
	//PackTypes:                         RGBA8,            RGBA8U,           RGBA16,            RGBA16,            RGBA32,          RGBA32U,         RGBA32F,    RG8,              RG8U,             RG16,              RG16U,             RG32,            RG32U,           RG32F,    R8,               R8U,              R16,               R16U,              R32,             R32U,            R32F,     Depth16,              Depth24,              Depth32,               Depth24Stencil8,     DXT1, DXT2, DXT3, DXT4, DXT5, DXT6, DXT7
	const int32_t GInternalFormats[] = { GL_RGBA8,         GL_RGBA8,         GL_RGBA16,         GL_RGBA16,         GL_RGBA32I,      GL_RGBA32UI,     GL_RGBA32F, GL_RG8,           GL_RG8,           GL_RG16,           GL_RG16,           GL_RG32I,        GL_RG32UI,       GL_RG32F, GL_R8,            GL_R8,            GL_R16,            GL_R16,            GL_R32I,         GL_R32UI,        GL_R32F,  GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32F, GL_DEPTH24_STENCIL8, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0,   GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT };
	const int32_t GFormats[]         = { GL_RGBA,          GL_RGBA,          GL_RGBA,           GL_RGBA,           GL_RGBA,         GL_RGBA,         GL_RGBA,    GL_RG,            GL_RG,            GL_RG,             GL_RG,             GL_RG,           GL_RG,           GL_RG,    GL_RED,           GL_RED,           GL_RED,            GL_RED,            GL_RED,          GL_RED,          GL_RED,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,    GL_DEPTH_STENCIL, };
	const int32_t GType[]            = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT,   GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_FLOAT,             GL_FLOAT,             GL_FLOAT,              GL_UNSIGNED_INT_24_8 };
	if (GInternalFormats[PackType] == 0) return nullptr;

	uint32_t VideoID = 0;
	uint32_t Type = 0;
	bool MakeMipmaps = (TextureState&LWTexture::MakeMipmaps) != 0;
	bool Compressed = LWImage::CompressedType(PackType);
	if (MakeMipmaps) MipmapCnt = Compressed ? 0 : LWImage::MipmapCount(Size);
	TextureState = (TextureState&~LWTexture::MakeMipmaps);
	glGenTextures(1, &VideoID);
	glBindTexture(GL_TEXTURE_3D, VideoID);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, MipmapCnt);
	glTexStorage3D(GL_TEXTURE_3D, MipmapCnt + 1, GInternalFormats[PackType], Size.x, Size.y, Size.z);
	if (!Texels) return Allocator.Create<LWOpenGL4_5Texture>(VideoID, TextureState, PackType, MipmapCnt, Size, LWTexture::Texture3D);
	uint32_t Mips = MakeMipmaps ? 0 : MipmapCnt;
	uint32_t t = 0;
	for (uint32_t i = 0; i <= Mips; i++, t++) {
		LWVector3i S = LWImage::MipmapSize3D(Size, i);
		if (Compressed) glCompressedTexSubImage3D(GL_TEXTURE_3D, i, 0, 0, 0, S.x, S.y, S.z, GInternalFormats[PackType], LWImage::GetLength3D(S, PackType), Texels[t]);
		else glTexSubImage3D(GL_TEXTURE_3D, i, 0, 0, 0, S.x, S.y, S.z, GFormats[PackType], GType[PackType], Texels[t]);
	}
	//if (MakeMipmaps && !Compressed) glGenerateMipmap(GL_TEXTURE_3D);
	return Allocator.Create<LWOpenGL4_5Texture>(VideoID, TextureState, PackType, MipmapCnt, Size, LWTexture::Texture3D);
}

LWTexture *LWVideoDriver_OpenGL4_5::CreateTextureCubeMap(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator) {
	//PackTypes:                         RGBA8,            RGBA8U,           RGBA16,            RGBA16,            RGBA32,          RGBA32U,         RGBA32F,    RG8,              RG8U,             RG16,              RG16U,             RG32,            RG32U,           RG32F,    R8,               R8U,              R16,               R16U,              R32,             R32U,            R32F,     Depth16,              Depth24,              Depth32,               Depth24Stencil8,     DXT1,                            DXT2,                             DXT3                              DXT4 DXT5                              DXT6                                     DXT7
	const int32_t GInternalFormats[] = { GL_RGBA8,         GL_RGBA8,         GL_RGBA16,         GL_RGBA16,         GL_RGBA32I,      GL_RGBA32UI,     GL_RGBA32F, GL_RG8,           GL_RG8,           GL_RG16,           GL_RG16,           GL_RG32I,        GL_RG32UI,       GL_RG32F, GL_R8,            GL_R8,            GL_R16,            GL_R16,            GL_R32I,         GL_R32UI,        GL_R32F,  GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32F, GL_DEPTH24_STENCIL8, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0,   GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT };
	const int32_t GFormats[]         = { GL_RGBA,          GL_RGBA,          GL_RGBA,           GL_RGBA,           GL_RGBA,         GL_RGBA,         GL_RGBA,    GL_RG,            GL_RG,            GL_RG,             GL_RG,             GL_RG,           GL_RG,           GL_RG,    GL_RED,           GL_RED,           GL_RED,            GL_RED,            GL_RED,          GL_RED,          GL_RED,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,    GL_DEPTH_STENCIL };
	const int32_t GType[]            = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT,   GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_FLOAT,             GL_FLOAT,             GL_FLOAT,              GL_UNSIGNED_INT_24_8 };
	if (GInternalFormats[PackType] == 0) return nullptr;

	uint32_t VideoID = 0;
	uint32_t Type = 0;
	bool MakeMipmaps = (TextureState&LWTexture::MakeMipmaps) != 0;
	bool Compressed = LWImage::CompressedType(PackType);
	if (MakeMipmaps) MipmapCnt = Compressed ? 0 : LWImage::MipmapCount(Size);
	TextureState = (TextureState&~LWTexture::MakeMipmaps);
	glGenTextures(1, &VideoID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, VideoID);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, MipmapCnt);
	glTexStorage2D(GL_TEXTURE_CUBE_MAP, MipmapCnt + 1, GInternalFormats[PackType], Size.x, Size.y);
	if (!Texels) return Allocator.Create<LWOpenGL4_5Texture>(VideoID, TextureState, PackType, MipmapCnt, LWVector3i(Size, 0), LWTexture::TextureCubeMap);
	uint32_t Mips = MakeMipmaps ? 0 : MipmapCnt;
	uint32_t t = 0;
	for (uint32_t l = 0; l < 6; l++) {
		uint32_t GTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + l;
		for (uint32_t i = 0; i <= Mips; i++, t++) {
			LWVector2i S = LWImage::MipmapSize2D(Size, i);
			if (Compressed) glCompressedTexSubImage2D(GTarget, i, 0, 0, S.x, S.y, GInternalFormats[PackType], LWImage::GetLength2D(S, PackType), Texels[t]);
			else glTexSubImage2D(GTarget, i, 0, 0, S.x, S.y, GFormats[PackType], GType[PackType], Texels[t]);
		}
	}
	//if (MakeMipmaps && !Compressed) glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	return Allocator.Create<LWOpenGL4_5Texture>(VideoID, TextureState, PackType, MipmapCnt, LWVector3i(Size, 0), LWTexture::TextureCubeMap);
}

LWTexture *LWVideoDriver_OpenGL4_5::CreateTexture2DMS(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint32_t Samples, LWAllocator &Allocator){
	//PackTypes:                         RGBA8,            RGBA8U,           RGBA16,            RGBA16,            RGBA32,          RGBA32U,         RGBA32F,    RG8,              RG8U,             RG16,              RG16U,             RG32,            RG32U,           RG32F,    R8,               R8U,              R16,               R16U,              R32,             R32U,            R32F,     Depth16,              Depth24,              Depth32,               Depth24Stencil8,     DXT1,                            DXT2,                             DXT3                              DXT4 DXT5                              DXT6                                     DXT7
	const int32_t GInternalFormats[] = { GL_RGBA8,         GL_RGBA8,         GL_RGBA16,         GL_RGBA16,         GL_RGBA32I,      GL_RGBA32UI,     GL_RGBA32F, GL_RG8,           GL_RG8,           GL_RG16,           GL_RG16,           GL_RG32I,        GL_RG32UI,       GL_RG32F, GL_R8,            GL_R8,            GL_R16,            GL_R16,            GL_R32I,         GL_R32UI,        GL_R32F,  GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32F, GL_DEPTH24_STENCIL8, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0,   GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT };
	const int32_t GFormats[] = { GL_RGBA,          GL_RGBA,          GL_RGBA,           GL_RGBA,           GL_RGBA,         GL_RGBA,         GL_RGBA,    GL_RG,            GL_RG,            GL_RG,             GL_RG,             GL_RG,           GL_RG,           GL_RG,    GL_RED,           GL_RED,           GL_RED,            GL_RED,            GL_RED,          GL_RED,          GL_RED,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,    GL_DEPTH_STENCIL };
	const int32_t GType[] = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT,   GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_FLOAT,             GL_FLOAT,             GL_FLOAT,              GL_UNSIGNED_INT_24_8 };
	if (GInternalFormats[PackType] == 0) return nullptr;

	uint32_t VideoID = 0;
	uint32_t Type = 0;
	TextureState = (TextureState & ~(LWTexture::RenderBuffer | LWTexture::MakeMipmaps));
	bool Compressed = LWImage::CompressedType(PackType);
	TextureState = (TextureState & ~LWTexture::MakeMipmaps);
	glGenTextures(1, &VideoID);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, VideoID);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, Samples, GInternalFormats[PackType], Size.x, Size.y, true);
	return Allocator.Create<LWOpenGL4_5Texture>(VideoID, TextureState, PackType, Samples, LWVector3i(Size, 0), LWTexture::Texture2DMS);
}

LWTexture *LWVideoDriver_OpenGL4_5::CreateTexture1DArray(uint32_t TextureState, uint32_t PackType, uint32_t Size, uint32_t Layers, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator) {
	//PackTypes:                         RGBA8,            RGBA8U,           RGBA16,            RGBA16,            RGBA32,          RGBA32U,         RGBA32F,    RG8,              RG8U,             RG16,              RG16U,             RG32,            RG32U,           RG32F,    R8,               R8U,              R16,               R16U,              R32,             R32U,            R32F,     Depth16,              Depth24,              Depth32,              Depth24Stencil8,     DXT1,                            DXT2,                             DXT3                              DXT4 DXT5                              DXT6                                     DXT7
	const int32_t GInternalFormats[] = { GL_RGBA8,         GL_RGBA8,         GL_RGBA16,         GL_RGBA16,         GL_RGBA32I,      GL_RGBA32UI,     GL_RGBA32F, GL_RG8,           GL_RG8,           GL_RG16,           GL_RG16,           GL_RG32I,        GL_RG32UI,       GL_RG32F, GL_R8,            GL_R8,            GL_R16,            GL_R16,            GL_R32I,         GL_R32UI,        GL_R32F,  GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32F, GL_DEPTH24_STENCIL8, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0,   GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT };
	const int32_t GFormats[]         = { GL_RGBA,          GL_RGBA,          GL_RGBA,           GL_RGBA,           GL_RGBA,         GL_RGBA,         GL_RGBA,    GL_RG,            GL_RG,            GL_RG,             GL_RG,             GL_RG,           GL_RG,           GL_RG,    GL_RED,           GL_RED,           GL_RED,            GL_RED,            GL_RED,          GL_RED,          GL_RED,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,    GL_DEPTH_STENCIL };
	const int32_t GType[]            = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT,   GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_FLOAT,             GL_FLOAT,             GL_FLOAT,              GL_UNSIGNED_INT_24_8 };
	if (GInternalFormats[PackType] == 0) return nullptr;

	uint32_t VideoID = 0;
	uint32_t Type = 0;
	bool MakeMipmaps = (TextureState&LWTexture::MakeMipmaps) != 0;
	bool Compressed = LWImage::CompressedType(PackType);
	if (MakeMipmaps) MipmapCnt = Compressed ? 0 : LWImage::MipmapCount(Size);
	TextureState = (TextureState&~LWTexture::MakeMipmaps);
	glGenTextures(1, &VideoID);
	glBindTexture(GL_TEXTURE_1D_ARRAY, VideoID);
	glTexParameteri(GL_TEXTURE_1D_ARRAY, GL_TEXTURE_MAX_LEVEL, MipmapCnt);
	glTexStorage2D(GL_TEXTURE_1D_ARRAY, MipmapCnt + 1, GInternalFormats[PackType], Size, Layers);
	if (!Texels) return Allocator.Create<LWOpenGL4_5Texture>(VideoID, TextureState, PackType, MipmapCnt, LWVector3i(Size, Layers, 0), LWTexture::Texture1DArray);
	uint32_t Mips = MakeMipmaps ? 0 : MipmapCnt;
	uint32_t t = 0;
	for (uint32_t l = 0; l < Layers; l++) {
		for (uint32_t i = 0; i <= Mips; i++, t++) {
			uint32_t S = LWImage::MipmapSize1D(Size, i);
			glTexSubImage2D(GL_TEXTURE_1D_ARRAY, i, 0, l, S, 1, GFormats[PackType], GType[PackType], Texels[t]);
		}
	}
	//if (MakeMipmaps && !Compressed) glGenerateMipmap(GL_TEXTURE_1D_ARRAY);
	return Allocator.Create<LWOpenGL4_5Texture>(VideoID, TextureState, PackType, MipmapCnt, LWVector3i(Size, Layers, 0), LWTexture::Texture1DArray);
}

LWTexture *LWVideoDriver_OpenGL4_5::CreateTexture2DArray(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint32_t Layers, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator) {
	//PackTypes:                         RGBA8,            RGBA8U,           RGBA16,            RGBA16,            RGBA32,          RGBA32U,         RGBA32F,    RG8,              RG8U,             RG16,              RG16U,             RG32,            RG32U,           RG32F,    R8,               R8U,              R16,               R16U,              R32,             R32U,            R32F,     Depth16,              Depth24,              Depth32,              Depth24Stencil8,     DXT1,                            DXT2,                             DXT3                              DXT4 DXT5                              DXT6                                     DXT7
	const int32_t GInternalFormats[] = { GL_RGBA8,         GL_RGBA8,         GL_RGBA16,         GL_RGBA16,         GL_RGBA32I,      GL_RGBA32UI,     GL_RGBA32F, GL_RG8,           GL_RG8,           GL_RG16,           GL_RG16,           GL_RG32I,        GL_RG32UI,       GL_RG32F, GL_R8,            GL_R8,            GL_R16,            GL_R16,            GL_R32I,         GL_R32UI,        GL_R32F,  GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32F, GL_DEPTH24_STENCIL8, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0,   GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT };
	const int32_t GFormats[]         = { GL_RGBA,          GL_RGBA,          GL_RGBA,           GL_RGBA,           GL_RGBA,         GL_RGBA,         GL_RGBA,    GL_RG,            GL_RG,            GL_RG,             GL_RG,             GL_RG,           GL_RG,           GL_RG,    GL_RED,           GL_RED,           GL_RED,            GL_RED,            GL_RED,          GL_RED,          GL_RED,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_STENCIL };
	const int32_t GType[]            = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT,   GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_FLOAT,             GL_FLOAT,             GL_FLOAT,             GL_UNSIGNED_INT_24_8 };
	if (GInternalFormats[PackType] == 0) return nullptr;

	uint32_t VideoID = 0;
	uint32_t Type = 0;
	bool MakeMipmaps = (TextureState&LWTexture::MakeMipmaps) != 0;
	bool Compressed = LWImage::CompressedType(PackType);
	if (MakeMipmaps) MipmapCnt = Compressed ? 0 : LWImage::MipmapCount(Size);
	TextureState = (TextureState&~LWTexture::MakeMipmaps);
	glGenTextures(1, &VideoID);
	glBindTexture(GL_TEXTURE_2D_ARRAY, VideoID);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, MipmapCnt);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, MipmapCnt + 1, GInternalFormats[PackType], Size.x, Size.y, Layers);
	if (!Texels) return Allocator.Create<LWOpenGL4_5Texture>(VideoID, TextureState, PackType, MipmapCnt, LWVector3i(Size, Layers), LWTexture::Texture2DArray);
	uint32_t Mips = MakeMipmaps ? 0 : MipmapCnt;
	uint32_t t = 0;
	for (uint32_t l = 0; l < Layers; l++) {
		for (uint32_t i = 0; i <= Mips; i++, t++) {
			LWVector2i S = LWImage::MipmapSize2D(Size, i);
			if (Compressed) glCompressedTexSubImage3D(GL_TEXTURE_2D_ARRAY, i, 0, 0, l, S.x, S.y, 1, GInternalFormats[PackType], LWImage::GetLength2D(S, PackType), Texels[t]);
			else glTexSubImage3D(GL_TEXTURE_2D_ARRAY, i, 0, 0, l, S.x, S.y, 1, GFormats[PackType], GType[PackType], Texels[t]);
		}
	}
	//if (MakeMipmaps && !Compressed) glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
	return Allocator.Create<LWOpenGL4_5Texture>(VideoID, TextureState, PackType, MipmapCnt, LWVector3i(Size, Layers), LWTexture::Texture2DArray);
}

LWTexture *LWVideoDriver_OpenGL4_5::CreateTextureCubeArray(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint32_t Layers, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator) {
	//PackTypes:                         RGBA8,            RGBA8U,           RGBA16,            RGBA16,            RGBA32,          RGBA32U,         RGBA32F,    RG8,              RG8U,             RG16,              RG16U,             RG32,            RG32U,           RG32F,    R8,               R8U,              R16,               R16U,              R32,             R32U,            R32F,     Depth16,              Depth24,              Depth32,               Depth24Stencil8,     DXT1,                            DXT2,                             DXT3                              DXT4 DXT5                              DXT6                                     DXT7
	const int32_t GInternalFormats[] = { GL_RGBA8,         GL_RGBA8,         GL_RGBA16,         GL_RGBA16,         GL_RGBA32I,      GL_RGBA32UI,     GL_RGBA32F, GL_RG8,           GL_RG8,           GL_RG16,           GL_RG16,           GL_RG32I,        GL_RG32UI,       GL_RG32F, GL_R8,            GL_R8,            GL_R16,            GL_R16,            GL_R32I,         GL_R32UI,        GL_R32F,  GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32F, GL_DEPTH24_STENCIL8, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0,   GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT };
	const int32_t GFormats[]         = { GL_RGBA,          GL_RGBA,          GL_RGBA,           GL_RGBA,           GL_RGBA,         GL_RGBA,         GL_RGBA,    GL_RG,            GL_RG,            GL_RG,             GL_RG,             GL_RG,           GL_RG,           GL_RG,    GL_RED,           GL_RED,           GL_RED,            GL_RED,            GL_RED,          GL_RED,          GL_RED,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,    GL_DEPTH_STENCIL };
	const int32_t GType[]            = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT,   GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_FLOAT,             GL_FLOAT,             GL_FLOAT,              GL_UNSIGNED_INT_24_8 };
	if (GInternalFormats[PackType] == 0) return nullptr;

	uint32_t VideoID = 0;
	uint32_t Type = 0;
	bool MakeMipmaps = (TextureState&LWTexture::MakeMipmaps) != 0;
	bool Compressed = LWImage::CompressedType(PackType);
	if (MakeMipmaps) MipmapCnt = Compressed ? 0 : LWImage::MipmapCount(Size);
	TextureState = (TextureState&~LWTexture::MakeMipmaps);
	glGenTextures(1, &VideoID);
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, VideoID);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAX_LEVEL, MipmapCnt);
	glTexStorage3D(GL_TEXTURE_CUBE_MAP_ARRAY, MipmapCnt + 1, GInternalFormats[PackType], Size.x, Size.y, Layers*6);
	if (!Texels) return Allocator.Create<LWOpenGL4_5Texture>(VideoID, TextureState, PackType, MipmapCnt, LWVector3i(Size, Layers), LWTexture::TextureCubeMapArray);
	uint32_t Mips = MakeMipmaps ? 0 : MipmapCnt;
	uint32_t t = 0;
	for (uint32_t l = 0; l < Layers*6; l++) {
		for (uint32_t i = 0; i <= Mips; i++, t++) {
			LWVector2i S = LWImage::MipmapSize2D(Size, i);
			if (Compressed) glCompressedTexSubImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, i, 0, 0, l, S.x, S.y, 1, GInternalFormats[PackType], LWImage::GetLength2D(S, PackType), Texels[t]);
			else glTexSubImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, i, 0, 0, l, S.x, S.y, 1, GFormats[PackType], GType[PackType], Texels[t]);
		}
	}
	//if (MakeMipmaps && !Compressed) glGenerateMipmap(GL_TEXTURE_CUBE_MAP_ARRAY);
	return Allocator.Create<LWOpenGL4_5Texture>(VideoID, TextureState, PackType, MipmapCnt, LWVector3i(Size, Layers), LWTexture::TextureCubeMapArray);
}

LWTexture *LWVideoDriver_OpenGL4_5::CreateTexture2DMSArray(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint32_t Samples, uint32_t Layers, LWAllocator &Allocator) {
	//PackTypes:                         RGBA8,            RGBA8U,           RGBA16,            RGBA16,            RGBA32,          RGBA32U,         RGBA32F,    RG8,              RG8U,             RG16,              RG16U,             RG32,            RG32U,           RG32F,    R8,               R8U,              R16,               R16U,              R32,             R32U,            R32F,     Depth16,              Depth24,              Depth32,               Depth24Stencil8,     DXT1,                            DXT2,                             DXT3                              DXT4 DXT5                              DXT6                                     DXT7
	const int32_t GInternalFormats[] = { GL_RGBA8,         GL_RGBA8,         GL_RGBA16,         GL_RGBA16,         GL_RGBA32I,      GL_RGBA32UI,     GL_RGBA32F, GL_RG8,           GL_RG8,           GL_RG16,           GL_RG16,           GL_RG32I,        GL_RG32UI,       GL_RG32F, GL_R8,            GL_R8,            GL_R16,            GL_R16,            GL_R32I,         GL_R32UI,        GL_R32F,  GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32F, GL_DEPTH24_STENCIL8, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0,   GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT };
	const int32_t GFormats[] = { GL_RGBA,          GL_RGBA,          GL_RGBA,           GL_RGBA,           GL_RGBA,         GL_RGBA,         GL_RGBA,    GL_RG,            GL_RG,            GL_RG,             GL_RG,             GL_RG,           GL_RG,           GL_RG,    GL_RED,           GL_RED,           GL_RED,            GL_RED,            GL_RED,          GL_RED,          GL_RED,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,    GL_DEPTH_STENCIL };
	const int32_t GType[] = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT,   GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_FLOAT,             GL_FLOAT,             GL_FLOAT,              GL_UNSIGNED_INT_24_8 };
	if (GInternalFormats[PackType] == 0) return nullptr;

	uint32_t VideoID = 0;
	uint32_t Type = 0;
	TextureState = (TextureState & ~(LWTexture::RenderBuffer | LWTexture::MakeMipmaps));
	bool Compressed = LWImage::CompressedType(PackType);
	TextureState = (TextureState & ~LWTexture::MakeMipmaps);
	glGenTextures(1, &VideoID);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, VideoID);
	glTexImage3DMultisample(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, Samples, GInternalFormats[PackType], Size.x, Size.y, Layers, true);
	return Allocator.Create<LWOpenGL4_5Texture>(VideoID, TextureState, PackType, Samples, LWVector3i(Size, Layers), LWTexture::Texture2DMSArray);
}

LWVideoBuffer *LWVideoDriver_OpenGL4_5::CreateVideoBuffer(uint32_t Type, uint32_t UsageFlag, uint32_t TypeSize, uint32_t Length, LWAllocator &Allocator, const uint8_t *Buffer) {
	int32_t GTypes[] = { GL_ARRAY_BUFFER, GL_UNIFORM_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_SHADER_STORAGE_BUFFER};
	//                 Static, WriteDiscard, WriteNoOverlap, Readable
	int32_t GUsage = GL_DYNAMIC_DRAW;
	uint32_t UsageID = UsageFlag & LWVideoBuffer::UsageFlag;
	if (UsageID == LWVideoBuffer::Static) GUsage = GL_STATIC_DRAW;
	else if (UsageID == LWVideoBuffer::Readable) GUsage = GL_DYNAMIC_READ;
	uint32_t VideoID = 0;

	glGenBuffers(1, &VideoID);
	glBindBuffer(GTypes[Type], VideoID);
	glBufferData(GTypes[Type], Length*TypeSize, Buffer, GUsage);
	
	return Allocator.Create<LWOpenGL4_5Buffer>(Buffer, &Allocator, TypeSize, Length, UsageFlag | Type, VideoID);
}

LWFrameBuffer *LWVideoDriver_OpenGL4_5::CreateFrameBuffer(const LWVector2i &Size, LWAllocator &Allocator) {
	LWOpenGL4_5FrameBufferContext Con;
	glGenFramebuffers(1, &Con.m_FBOID);
	return Allocator.Create<LWOpenGL4_5FrameBuffer>(Con, Size);
}

bool LWVideoDriver_OpenGL4_5::UpdateTexture(LWTexture *Texture) {
	const int32_t GTypes[] = { GL_TEXTURE_1D, GL_TEXTURE_2D, GL_TEXTURE_3D, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_1D_ARRAY, GL_TEXTURE_2D_ARRAY, GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_2D_MULTISAMPLE_ARRAY };
	const int32_t MinMagFilters[] = { GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR };
	const int32_t WrapFilters[] = { GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER, GL_MIRRORED_REPEAT, GL_REPEAT };
	const int32_t CompareModes[] = { GL_NONE, GL_COMPARE_REF_TO_TEXTURE };
	const int32_t CompareFuncs[] = { GL_NEVER, GL_ALWAYS, GL_LESS, GL_EQUAL, GL_LEQUAL, GL_GREATER, GL_GEQUAL, GL_NOTEQUAL };
	const int32_t DepthReadMode[] = { GL_DEPTH_COMPONENT, GL_STENCIL_COMPONENTS }; //Possible bug, GL_STENCIL_COMPONENTS instead of GL_STENCIL_COMPONENT (note: S) is only defined.
	const float AnisotropyValues[] = { 1.0f, 2.0f, 4.0f, 8.0f, 16.0f }; //Anisotropy

	uint32_t VideoID = ((LWOpenGL4_5Texture*)Texture)->GetContext();
	int32_t Type = GTypes[Texture->GetType()];
	glBindTexture(Type, VideoID);
	if (!Texture->isDirty()) return true;
	uint32_t State = Texture->GetTextureState();
	bool MakeMipmaps = (State & LWTexture::MakeMipmaps) != 0;
	bool MultiSampled = Texture->isMultiSampled();
	if (!MultiSampled) { //Sampler states not supported by multisamplers.
		uint32_t MinFilter = (State&LWTexture::MinFilterFlag) >> LWTexture::MinFilterBitOffset;
		uint32_t MagFilter = (State&LWTexture::MagFilterFlag) >> LWTexture::MagFilterBitOffset;
		uint32_t WrapS = (State&LWTexture::WrapSFilterFlag) >> LWTexture::WrapSFilterBitOffset;
		uint32_t WrapT = (State&LWTexture::WrapTFilterFlag) >> LWTexture::WrapTFilterBitOffset;
		uint32_t WrapR = (State&LWTexture::WrapRFilterFlag) >> LWTexture::WrapRFilterBitOffset;
		uint32_t CFunc = (State&LWTexture::CompareFuncFlag) >> LWTexture::CompareFuncBitOffset;
		uint32_t CMode = (State&LWTexture::CompareModeFlag) >> LWTexture::CompareModeBitOffset;
		uint32_t DRMode = (State&LWTexture::DepthReadFlag) >> LWTexture::DepthReadBitOffset;
		uint32_t Anisotropy = (State & LWTexture::AnisotropyFlag) >> LWTexture::AnisotropyBitOffset;
		
		glTexParameteri(Type, GL_TEXTURE_MIN_FILTER, MinMagFilters[MinFilter]);
		glTexParameteri(Type, GL_TEXTURE_MAG_FILTER, MinMagFilters[MagFilter]);
		
		glTexParameteri(Type, GL_TEXTURE_WRAP_S, WrapFilters[WrapS]);
		glTexParameteri(Type, GL_TEXTURE_WRAP_T, WrapFilters[WrapT]);
		glTexParameteri(Type, GL_TEXTURE_WRAP_R, WrapFilters[WrapR]);
		glTexParameteri(Type, GL_TEXTURE_COMPARE_FUNC, CompareFuncs[CFunc]);
		glTexParameteri(Type, GL_TEXTURE_COMPARE_MODE, CompareModes[CMode]);
		glTexParameteri(Type, GL_DEPTH_STENCIL_TEXTURE_MODE, DepthReadMode[DRMode]);
		
		glTexParameterf(Type, GL_TEXTURE_MAX_ANISOTROPY_EXT, AnisotropyValues[Anisotropy]);
		if (MakeMipmaps) glGenerateMipmap(Type);
	}
	Texture->ClearDirty();
	
	return true;
}

bool LWVideoDriver_OpenGL4_5::UpdateTexture1D(LWTexture *Texture, uint32_t MipmapLevel, void *Texels, uint32_t Position, uint32_t Size) {
	//PackTypes:                   RGBA8,            RGBA8U,           RGBA16,            RGBA16,            RGBA32,          RGBA32U,         RGBA32F,    RG8,              RG8U,             RG16,              RG16U,             RG32,            RG32U,           RG32F,    R8,               R8U,              R16,               R16U,              R32,             R32U,            R32F,     Depth16,              Depth24,              Depth32,              Depth24Stencil8
	const int32_t GInternalFormats[] = { GL_RGBA8,         GL_RGBA8,         GL_RGBA16,         GL_RGBA16,         GL_RGBA32I,      GL_RGBA32UI,     GL_RGBA32F, GL_RG8,           GL_RG8,           GL_RG16,           GL_RG16,           GL_RG32I,        GL_RG32UI,       GL_RG32F, GL_R8,            GL_R8,            GL_R16,            GL_R16,            GL_R32I,         GL_R32UI,        GL_R32F,  0,       0,       0,       0,               0,    0,    0,    0,    0,    0,    0 };
	const int32_t GFormats[] = { GL_RGBA,          GL_RGBA,          GL_RGBA,           GL_RGBA,           GL_RGBA,         GL_RGBA,         GL_RGBA,    GL_RG,            GL_RG,            GL_RG,             GL_RG,             GL_RG,           GL_RG,           GL_RG,    GL_RED,           GL_RED,           GL_RED,            GL_RED,            GL_RED,          GL_RED,          GL_RED,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_STENCIL };
	const int32_t GType[] = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT,   GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_FLOAT,             GL_FLOAT,             GL_FLOAT,             GL_UNSIGNED_INT_24_8 };
	if (!UpdateTexture(Texture)) return false;
	if (Texels) {
		uint32_t PackType = Texture->GetPackType();
		glTexSubImage1D(GL_TEXTURE_1D, MipmapLevel, Position, Size, GFormats[PackType], GType[PackType], Texels);
	}
	return true;
}

bool LWVideoDriver_OpenGL4_5::UpdateTexture2D(LWTexture *Texture, uint32_t MipmapLevel, void *Texels, const LWVector2i &Position, const LWVector2i &Size) {
	//PackTypes:                   RGBA8,            RGBA8U,           RGBA16,            RGBA16,            RGBA32,          RGBA32U,         RGBA32F,    RG8,              RG8U,             RG16,              RG16U,             RG32,            RG32U,           RG32F,    R8,               R8U,              R16,               R16U,              R32,             R32U,            R32F,     Depth16,              Depth24,              Depth32,              Depth24Stencil8
	const int32_t GInternalFormats[] = { GL_RGBA8,         GL_RGBA8,         GL_RGBA16,         GL_RGBA16,         GL_RGBA32I,      GL_RGBA32UI,     GL_RGBA32F, GL_RG8,           GL_RG8,           GL_RG16,           GL_RG16,           GL_RG32I,        GL_RG32UI,       GL_RG32F, GL_R8,            GL_R8,            GL_R16,            GL_R16,            GL_R32I,         GL_R32UI,        GL_R32F,  GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32, GL_DEPTH24_STENCIL8, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0,   GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT };
	const int32_t GFormats[]         = { GL_RGBA,          GL_RGBA,          GL_RGBA,           GL_RGBA,           GL_RGBA,         GL_RGBA,         GL_RGBA,    GL_RG,            GL_RG,            GL_RG,             GL_RG,             GL_RG,           GL_RG,           GL_RG,    GL_RED,           GL_RED,           GL_RED,            GL_RED,            GL_RED,          GL_RED,          GL_RED,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_STENCIL };
	const int32_t GType[]            = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT,   GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_FLOAT,             GL_FLOAT,             GL_FLOAT,             GL_UNSIGNED_INT_24_8 };
	if (!UpdateTexture(Texture)) return false;

	if (Texels) {
		uint32_t PackType = Texture->GetPackType();
		if (LWImage::CompressedType(PackType)) {
			glCompressedTexSubImage2D(GL_TEXTURE_2D, MipmapLevel, Position.x, Position.y, Size.x, Size.y, GFormats[PackType], LWImage::GetLength2D(Size, PackType), Texels);
		} else {
			glTexSubImage2D(GL_TEXTURE_2D, MipmapLevel, Position.x, Position.y, Size.x, Size.y, GFormats[PackType], GType[PackType], Texels);
		}
	}
	return true;
}

bool LWVideoDriver_OpenGL4_5::UpdateTexture3D(LWTexture *Texture, uint32_t MipmapLevel, void *Texels, const LWVector3i &Position, const LWVector3i &Size) {
	//PackTypes:           RGBA8,            RGBA8U,           RGBA16,            RGBA16,            RGBA32,          RGBA32U,         RGBA32F,    RG8,              RG8U,             RG16,              RG16U,             RG32,            RG32U,           RG32F,    R8,               R8U,              R16,               R16U,              R32,             R32U,            R32F,     Depth16,              Depth24,              Depth32,              Depth24Stencil8
	const int32_t GInternalFormats[] = { GL_RGBA8,         GL_RGBA8,         GL_RGBA16,         GL_RGBA16,         GL_RGBA32I,      GL_RGBA32UI,     GL_RGBA32F, GL_RG8,           GL_RG8,           GL_RG16,           GL_RG16,           GL_RG32I,        GL_RG32UI,       GL_RG32F, GL_R8,            GL_R8,            GL_R16,            GL_R16,            GL_R32I,         GL_R32UI,        GL_R32F,  GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32, GL_DEPTH24_STENCIL8, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0,   GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT };
	const int32_t GFormats[] = { GL_RGBA,          GL_RGBA,          GL_RGBA,           GL_RGBA,           GL_RGBA,         GL_RGBA,         GL_RGBA,    GL_RG,            GL_RG,            GL_RG,             GL_RG,             GL_RG,           GL_RG,           GL_RG,    GL_RED,           GL_RED,           GL_RED,            GL_RED,            GL_RED,          GL_RED,          GL_RED,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_STENCIL };
	const int32_t GType[] = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT,   GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_FLOAT,             GL_FLOAT,             GL_FLOAT,             GL_UNSIGNED_INT_24_8 };
	if (!UpdateTexture(Texture)) return false;

	if (Texels) {
		uint32_t PackType = Texture->GetPackType();
		if (LWImage::CompressedType(PackType)) {
			glCompressedTexSubImage3D(GL_TEXTURE_3D, MipmapLevel, Position.x, Position.y, Position.z, Size.x, Size.y, Size.z, GInternalFormats[PackType], LWImage::GetLength3D(Size, PackType), Texels);
		} else {
			glTexSubImage3D(GL_TEXTURE_3D, MipmapLevel, Position.x, Position.y, Position.z, Size.x, Size.y, Size.z, GFormats[PackType], GType[PackType], Texels);
		}
	}
	return true;
}

bool LWVideoDriver_OpenGL4_5::UpdateTextureCubeMap(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Face, void *Texels, const LWVector2i &Position, const LWVector2i &Size) {
	//PackTypes:                   RGBA8,            RGBA8U,           RGBA16,            RGBA16,            RGBA32,          RGBA32U,         RGBA32F,    RG8,              RG8U,             RG16,              RG16U,             RG32,            RG32U,           RG32F,    R8,               R8U,              R16,               R16U,              R32,             R32U,            R32F,     Depth16,              Depth24,              Depth32,              Depth24Stencil8
	const int32_t GInternalFormats[] = { GL_RGBA8,         GL_RGBA8,         GL_RGBA16,         GL_RGBA16,         GL_RGBA32I,      GL_RGBA32UI,     GL_RGBA32F, GL_RG8,           GL_RG8,           GL_RG16,           GL_RG16,           GL_RG32I,        GL_RG32UI,       GL_RG32F, GL_R8,            GL_R8,            GL_R16,            GL_R16,            GL_R32I,         GL_R32UI,        GL_R32F,  GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32, GL_DEPTH24_STENCIL8, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0,   GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT };
	const int32_t GFormats[] = { GL_RGBA,          GL_RGBA,          GL_RGBA,           GL_RGBA,           GL_RGBA,         GL_RGBA,         GL_RGBA,    GL_RG,            GL_RG,            GL_RG,             GL_RG,             GL_RG,           GL_RG,           GL_RG,    GL_RED,           GL_RED,           GL_RED,            GL_RED,            GL_RED,          GL_RED,          GL_RED,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_STENCIL };
	const int32_t GType[] = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT,   GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_FLOAT,             GL_FLOAT,             GL_FLOAT,             GL_UNSIGNED_INT_24_8 };
	if (!UpdateTexture(Texture)) return false;
	if (Texels) {
		uint32_t PackType = Texture->GetPackType();
		if (LWImage::CompressedType(PackType)) {
			glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + Face, MipmapLevel, Position.x, Position.y, Size.x, Size.y, GInternalFormats[PackType], LWImage::GetLength2D(Size, PackType), Texels);
		} else {
			glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + Face, MipmapLevel, Position.x, Position.y, Size.x, Size.y, GFormats[PackType], GType[PackType], Texels);
		}
	}
	return true;
}

bool LWVideoDriver_OpenGL4_5::UpdateTexture1DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, void *Texels, uint32_t Position, uint32_t Size) {
	//PackTypes:                   RGBA8,            RGBA8U,           RGBA16,            RGBA16,            RGBA32,          RGBA32U,         RGBA32F,    RG8,              RG8U,             RG16,              RG16U,             RG32,            RG32U,           RG32F,    R8,               R8U,              R16,               R16U,              R32,             R32U,            R32F,     Depth16,              Depth24,              Depth32,              Depth24Stencil8
	const int32_t GInternalFormats[] = { GL_RGBA8,         GL_RGBA8,         GL_RGBA16,         GL_RGBA16,         GL_RGBA32I,      GL_RGBA32UI,     GL_RGBA32F, GL_RG8,           GL_RG8,           GL_RG16,           GL_RG16,           GL_RG32I,        GL_RG32UI,       GL_RG32F, GL_R8,            GL_R8,            GL_R16,            GL_R16,            GL_R32I,         GL_R32UI,        GL_R32F,  GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32, GL_DEPTH24_STENCIL8, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0,   GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT };
	const int32_t GFormats[] = { GL_RGBA,          GL_RGBA,          GL_RGBA,           GL_RGBA,           GL_RGBA,         GL_RGBA,         GL_RGBA,    GL_RG,            GL_RG,            GL_RG,             GL_RG,             GL_RG,           GL_RG,           GL_RG,    GL_RED,           GL_RED,           GL_RED,            GL_RED,            GL_RED,          GL_RED,          GL_RED,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_STENCIL };
	const int32_t GType[] = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT,   GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_FLOAT,             GL_FLOAT,             GL_FLOAT,             GL_UNSIGNED_INT_24_8 };
	if (!UpdateTexture(Texture)) return false;
	if (Texels) {
		uint32_t PackType = Texture->GetPackType();
		glTexSubImage2D(GL_TEXTURE_1D_ARRAY, MipmapLevel, Position, Layer, Size, 1, GFormats[PackType], GType[PackType], Texels);
	}
	return true;
}

bool LWVideoDriver_OpenGL4_5::UpdateTexture2DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, void *Texels, const LWVector2i &Position, const LWVector2i &Size) {
	//PackTypes:                   RGBA8,            RGBA8U,           RGBA16,            RGBA16,            RGBA32,          RGBA32U,         RGBA32F,    RG8,              RG8U,             RG16,              RG16U,             RG32,            RG32U,           RG32F,    R8,               R8U,              R16,               R16U,              R32,             R32U,            R32F,     Depth16,              Depth24,              Depth32,              Depth24Stencil8
	const int32_t GInternalFormats[] = { GL_RGBA8,         GL_RGBA8,         GL_RGBA16,         GL_RGBA16,         GL_RGBA32I,      GL_RGBA32UI,     GL_RGBA32F, GL_RG8,           GL_RG8,           GL_RG16,           GL_RG16,           GL_RG32I,        GL_RG32UI,       GL_RG32F, GL_R8,            GL_R8,            GL_R16,            GL_R16,            GL_R32I,         GL_R32UI,        GL_R32F,  GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32, GL_DEPTH24_STENCIL8, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0,   GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT };
	const int32_t GFormats[] = { GL_RGBA,          GL_RGBA,          GL_RGBA,           GL_RGBA,           GL_RGBA,         GL_RGBA,         GL_RGBA,    GL_RG,            GL_RG,            GL_RG,             GL_RG,             GL_RG,           GL_RG,           GL_RG,    GL_RED,           GL_RED,           GL_RED,            GL_RED,            GL_RED,          GL_RED,          GL_RED,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_STENCIL };
	const int32_t GType[] = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT,   GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_FLOAT,             GL_FLOAT,             GL_FLOAT,             GL_UNSIGNED_INT_24_8 };
	if (!UpdateTexture(Texture)) return false;
	if (Texels) {
		uint32_t PackType = Texture->GetPackType();
		if (LWImage::CompressedType(PackType)) {
			glCompressedTexSubImage3D(GL_TEXTURE_2D_ARRAY, MipmapLevel, Position.x, Position.y, Layer, Size.x, Size.y, 1, GInternalFormats[PackType], LWImage::GetLength2D(Size, PackType), Texels);
		} else {
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY, MipmapLevel, Position.x, Position.y, Layer, Size.x, Size.y, 1, GFormats[PackType], GType[PackType], Texels);
		}
	}
	return true;
}

bool LWVideoDriver_OpenGL4_5::UpdateTextureCubeArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint32_t Face, void *Texels, const LWVector2i &Position, const LWVector2i &Size) {
	//PackTypes:                   RGBA8,            RGBA8U,           RGBA16,            RGBA16,            RGBA32,          RGBA32U,         RGBA32F,    RG8,              RG8U,             RG16,              RG16U,             RG32,            RG32U,           RG32F,    R8,               R8U,              R16,               R16U,              R32,             R32U,            R32F,     Depth16,              Depth24,              Depth32,              Depth24Stencil8
	const int32_t GInternalFormats[] = { GL_RGBA8,         GL_RGBA8,         GL_RGBA16,         GL_RGBA16,         GL_RGBA32I,      GL_RGBA32UI,     GL_RGBA32F, GL_RG8,           GL_RG8,           GL_RG16,           GL_RG16,           GL_RG32I,        GL_RG32UI,       GL_RG32F, GL_R8,            GL_R8,            GL_R16,            GL_R16,            GL_R32I,         GL_R32UI,        GL_R32F,  GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32, GL_DEPTH24_STENCIL8, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0,   GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT };
	const int32_t GFormats[] = { GL_RGBA,          GL_RGBA,          GL_RGBA,           GL_RGBA,           GL_RGBA,         GL_RGBA,         GL_RGBA,    GL_RG,            GL_RG,            GL_RG,             GL_RG,             GL_RG,           GL_RG,           GL_RG,    GL_RED,           GL_RED,           GL_RED,            GL_RED,            GL_RED,          GL_RED,          GL_RED,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_STENCIL };
	const int32_t GType[] = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT,   GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_FLOAT,             GL_FLOAT,             GL_FLOAT,             GL_UNSIGNED_INT_24_8 };
	if (!UpdateTexture(Texture)) return false;
	if (Texels) {
		uint32_t PackType = Texture->GetPackType();
		if (LWImage::CompressedType(PackType)) {
			glCompressedTexSubImage3D(GL_TEXTURE_2D_ARRAY, MipmapLevel, Position.x, Position.y, Layer*6+Face, Size.x, Size.y, 1, GInternalFormats[PackType], LWImage::GetLength2D(Size, PackType), Texels);
		} else {
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY, MipmapLevel, Position.x, Position.y, Layer*6+Face, Size.x, Size.y, 1, GFormats[PackType], GType[PackType], Texels);
		}
	}
	return true;
}

bool LWVideoDriver_OpenGL4_5::UpdateVideoBuffer(LWVideoBuffer *VideoBuffer, const uint8_t *Buffer, uint32_t Length) {
	if (!Length) return true;
	LWOpenGL4_5Buffer *VB = (LWOpenGL4_5Buffer*)VideoBuffer;
	int32_t GTypes[] = { GL_ARRAY_BUFFER, GL_UNIFORM_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_SHADER_STORAGE_BUFFER};
	
	int32_t Type = GTypes[VB->GetType()];

	glBindBuffer(Type, VB->GetContext());
	uint8_t *B = (uint8_t*)glMapBuffer(Type, GL_WRITE_ONLY);
	if (!B) return false;
	std::copy(Buffer, Buffer + Length, B);
	glUnmapBuffer(Type);
	return true;
}

bool LWVideoDriver_OpenGL4_5::DownloadTexture1D(LWTexture *Texture, uint32_t MipmapLevel, uint8_t *Buffer) {
	const int32_t GInternalFormats[] = { GL_RGBA8,         GL_RGBA8,         GL_RGBA16,         GL_RGBA16,         GL_RGBA32I,      GL_RGBA32UI,     GL_RGBA32F, GL_RG8,           GL_RG8,           GL_RG16,           GL_RG16,           GL_RG32I,        GL_RG32UI,       GL_RG32F, GL_R8,            GL_R8,            GL_R16,            GL_R16,            GL_R32I,         GL_R32UI,        GL_R32F,  0,       0,       0,       0,               0,    0,    0,    0,    0,    0,    0 };
	const int32_t GFormats[] = { GL_RGBA,          GL_RGBA,          GL_RGBA,           GL_RGBA,           GL_RGBA,         GL_RGBA,         GL_RGBA,    GL_RG,            GL_RG,            GL_RG,             GL_RG,             GL_RG,           GL_RG,           GL_RG,    GL_RED,           GL_RED,           GL_RED,            GL_RED,            GL_RED,          GL_RED,          GL_RED,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_STENCIL };
	const int32_t GType[] = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT,   GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_FLOAT,             GL_FLOAT,             GL_FLOAT,             GL_UNSIGNED_INT_24_8 };
	if (!UpdateTexture(Texture)) return false;
	uint32_t PackType = Texture->GetPackType();
	glGetTexImage(GL_TEXTURE_1D, MipmapLevel, GFormats[PackType], GType[PackType], Buffer);
	return true;
}

bool LWVideoDriver_OpenGL4_5::DownloadTexture2D(LWTexture *Texture, uint32_t MipmapLevel, uint8_t *Buffer) {
	const int32_t GInternalFormats[] = { GL_RGBA8,         GL_RGBA8,         GL_RGBA16,         GL_RGBA16,         GL_RGBA32I,      GL_RGBA32UI,     GL_RGBA32F, GL_RG8,           GL_RG8,           GL_RG16,           GL_RG16,           GL_RG32I,        GL_RG32UI,       GL_RG32F, GL_R8,            GL_R8,            GL_R16,            GL_R16,            GL_R32I,         GL_R32UI,        GL_R32F,  GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32, GL_DEPTH24_STENCIL8, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0,   GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT };
	const int32_t GFormats[] = { GL_RGBA,          GL_RGBA,          GL_RGBA,           GL_RGBA,           GL_RGBA,         GL_RGBA,         GL_RGBA,    GL_RG,            GL_RG,            GL_RG,             GL_RG,             GL_RG,           GL_RG,           GL_RG,    GL_RED,           GL_RED,           GL_RED,            GL_RED,            GL_RED,          GL_RED,          GL_RED,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_STENCIL };
	const int32_t GType[] = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT,   GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_FLOAT,             GL_FLOAT,             GL_FLOAT,             GL_UNSIGNED_INT_24_8 };
	if (!UpdateTexture(Texture)) return false;
	uint32_t PackType = Texture->GetPackType();
	if (LWImage::CompressedType(PackType)) {
		glGetCompressedTexImage(GL_TEXTURE_2D, MipmapLevel, Buffer);
	} else {
		glGetTexImage(GL_TEXTURE_2D, MipmapLevel, GFormats[PackType], GType[PackType], Buffer);
	}
	return true;
}

bool LWVideoDriver_OpenGL4_5::DownloadTexture3D(LWTexture *Texture, uint32_t MipmapLevel, uint8_t *Buffer) {
	const int32_t GInternalFormats[] = { GL_RGBA8,         GL_RGBA8,         GL_RGBA16,         GL_RGBA16,         GL_RGBA32I,      GL_RGBA32UI,     GL_RGBA32F, GL_RG8,           GL_RG8,           GL_RG16,           GL_RG16,           GL_RG32I,        GL_RG32UI,       GL_RG32F, GL_R8,            GL_R8,            GL_R16,            GL_R16,            GL_R32I,         GL_R32UI,        GL_R32F,  GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32, GL_DEPTH24_STENCIL8, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0,   GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT };
	const int32_t GFormats[] = { GL_RGBA,          GL_RGBA,          GL_RGBA,           GL_RGBA,           GL_RGBA,         GL_RGBA,         GL_RGBA,    GL_RG,            GL_RG,            GL_RG,             GL_RG,             GL_RG,           GL_RG,           GL_RG,    GL_RED,           GL_RED,           GL_RED,            GL_RED,            GL_RED,          GL_RED,          GL_RED,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_STENCIL };
	const int32_t GType[] = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT,   GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_FLOAT,             GL_FLOAT,             GL_FLOAT,             GL_UNSIGNED_INT_24_8 };
	if (!UpdateTexture(Texture)) return false;
	uint32_t PackType = Texture->GetPackType();
	if (LWImage::CompressedType(PackType)) {
		glGetCompressedTexImage(GL_TEXTURE_3D, MipmapLevel, Buffer);
	} else {
		glGetTexImage(GL_TEXTURE_3D, MipmapLevel, GFormats[PackType], GType[PackType], Buffer);
	}
	return true;
}

bool LWVideoDriver_OpenGL4_5::DownloadTextureCubeMap(LWTexture *Texture, uint32_t Face, uint32_t MipmapLevel, uint8_t *Buffer) {
	const int32_t GInternalFormats[] = { GL_RGBA8,         GL_RGBA8,         GL_RGBA16,         GL_RGBA16,         GL_RGBA32I,      GL_RGBA32UI,     GL_RGBA32F, GL_RG8,           GL_RG8,           GL_RG16,           GL_RG16,           GL_RG32I,        GL_RG32UI,       GL_RG32F, GL_R8,            GL_R8,            GL_R16,            GL_R16,            GL_R32I,         GL_R32UI,        GL_R32F,  GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32, GL_DEPTH24_STENCIL8, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0,   GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT };
	const int32_t GFormats[] = { GL_RGBA,          GL_RGBA,          GL_RGBA,           GL_RGBA,           GL_RGBA,         GL_RGBA,         GL_RGBA,    GL_RG,            GL_RG,            GL_RG,             GL_RG,             GL_RG,           GL_RG,           GL_RG,    GL_RED,           GL_RED,           GL_RED,            GL_RED,            GL_RED,          GL_RED,          GL_RED,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_STENCIL };
	const int32_t GType[] = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT,   GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_FLOAT,             GL_FLOAT,             GL_FLOAT,             GL_UNSIGNED_INT_24_8 };
	if (!UpdateTexture(Texture)) return false;
	uint32_t PackType = Texture->GetPackType();
	if (LWImage::CompressedType(PackType)) {
		glGetCompressedTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + Face, MipmapLevel, Buffer);
	} else {
		glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + Face, MipmapLevel, GFormats[PackType], GType[PackType], Buffer);
	}
	return true;
}

bool LWVideoDriver_OpenGL4_5::DownloadTexture1DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint8_t *Buffer) {
	const int32_t GInternalFormats[] = { GL_RGBA8,         GL_RGBA8,         GL_RGBA16,         GL_RGBA16,         GL_RGBA32I,      GL_RGBA32UI,     GL_RGBA32F, GL_RG8,           GL_RG8,           GL_RG16,           GL_RG16,           GL_RG32I,        GL_RG32UI,       GL_RG32F, GL_R8,            GL_R8,            GL_R16,            GL_R16,            GL_R32I,         GL_R32UI,        GL_R32F,  GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32, GL_DEPTH24_STENCIL8, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0,   GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT };
	const int32_t GFormats[] = { GL_RGBA,          GL_RGBA,          GL_RGBA,           GL_RGBA,           GL_RGBA,         GL_RGBA,         GL_RGBA,    GL_RG,            GL_RG,            GL_RG,             GL_RG,             GL_RG,           GL_RG,           GL_RG,    GL_RED,           GL_RED,           GL_RED,            GL_RED,            GL_RED,          GL_RED,          GL_RED,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_STENCIL };
	const int32_t GType[] = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT,   GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_FLOAT,             GL_FLOAT,             GL_FLOAT,             GL_UNSIGNED_INT_24_8 };
	if (!UpdateTexture(Texture)) return false;
	uint32_t PackType = Texture->GetPackType();
	uint32_t S = LWImage::MipmapSize1D(Texture->Get1DSize(), MipmapLevel);
	glGetTextureSubImage(GL_TEXTURE_1D_ARRAY, MipmapLevel, 0, Layer, 0, S, 1, 1, GFormats[PackType], GType[PackType], LWImage::GetLength1D(S, PackType), Buffer);
	return true;
}

bool LWVideoDriver_OpenGL4_5::DownloadTexture2DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint8_t *Buffer) {
	const int32_t GInternalFormats[] = { GL_RGBA8,         GL_RGBA8,         GL_RGBA16,         GL_RGBA16,         GL_RGBA32I,      GL_RGBA32UI,     GL_RGBA32F, GL_RG8,           GL_RG8,           GL_RG16,           GL_RG16,           GL_RG32I,        GL_RG32UI,       GL_RG32F, GL_R8,            GL_R8,            GL_R16,            GL_R16,            GL_R32I,         GL_R32UI,        GL_R32F,  GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32, GL_DEPTH24_STENCIL8, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0,   GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT };
	const int32_t GFormats[] = { GL_RGBA,          GL_RGBA,          GL_RGBA,           GL_RGBA,           GL_RGBA,         GL_RGBA,         GL_RGBA,    GL_RG,            GL_RG,            GL_RG,             GL_RG,             GL_RG,           GL_RG,           GL_RG,    GL_RED,           GL_RED,           GL_RED,            GL_RED,            GL_RED,          GL_RED,          GL_RED,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_STENCIL };
	const int32_t GType[] = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT,   GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_FLOAT,             GL_FLOAT,             GL_FLOAT,             GL_UNSIGNED_INT_24_8 };
	if (!UpdateTexture(Texture)) return false;
	uint32_t PackType = Texture->GetPackType();
	LWVector2i S = LWImage::MipmapSize2D(Texture->Get2DSize(), MipmapLevel);
	glGetTextureSubImage(GL_TEXTURE_2D_ARRAY, MipmapLevel, 0, 0, Layer, S.x, S.y, 1, GFormats[PackType], GType[PackType], LWImage::GetLength2D(S, PackType), Buffer);
	return true;
}

bool LWVideoDriver_OpenGL4_5::DownloadTextureCubeArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint32_t Face, uint8_t *Buffer) {
	const int32_t GInternalFormats[] = { GL_RGBA8,         GL_RGBA8,         GL_RGBA16,         GL_RGBA16,         GL_RGBA32I,      GL_RGBA32UI,     GL_RGBA32F, GL_RG8,           GL_RG8,           GL_RG16,           GL_RG16,           GL_RG32I,        GL_RG32UI,       GL_RG32F, GL_R8,            GL_R8,            GL_R16,            GL_R16,            GL_R32I,         GL_R32UI,        GL_R32F,  GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32, GL_DEPTH24_STENCIL8, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0,   GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT };
	const int32_t GFormats[] = { GL_RGBA,          GL_RGBA,          GL_RGBA,           GL_RGBA,           GL_RGBA,         GL_RGBA,         GL_RGBA,    GL_RG,            GL_RG,            GL_RG,             GL_RG,             GL_RG,           GL_RG,           GL_RG,    GL_RED,           GL_RED,           GL_RED,            GL_RED,            GL_RED,          GL_RED,          GL_RED,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_STENCIL };
	const int32_t GType[] = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT,   GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_FLOAT,             GL_FLOAT,             GL_FLOAT,             GL_UNSIGNED_INT_24_8 };
	if (!UpdateTexture(Texture)) return false;
	uint32_t PackType = Texture->GetPackType();
	LWVector2i S = LWImage::MipmapSize2D(Texture->Get2DSize(), MipmapLevel);
	glGetTextureSubImage(GL_TEXTURE_2D_ARRAY, MipmapLevel, 0, 0, Layer*6+Face, S.x, S.y, 1, GFormats[PackType], GType[PackType], LWImage::GetLength2D(S, PackType), Buffer);
	return true;
}

bool LWVideoDriver_OpenGL4_5::DownloadVideoBuffer(LWVideoBuffer *VBuffer, uint8_t *Buffer, uint32_t Offset, uint32_t Length) {
	LWOpenGL4_5Buffer *VB = (LWOpenGL4_5Buffer*)VBuffer;
	int32_t GTypes[] = { GL_ARRAY_BUFFER, GL_UNIFORM_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_TEXTURE_BUFFER };
	int32_t Type = GTypes[VB->GetType()];
	glBindBuffer(Type, VB->GetContext());
	glGetBufferSubData(Type, Offset, Length, Buffer);
	return true;
}

LWVideoDriver &LWVideoDriver_OpenGL4_5::DestroyVideoBuffer(LWVideoBuffer *Buffer) {
	LWOpenGL4_5Buffer *VB = (LWOpenGL4_5Buffer*)Buffer;
	uint32_t VideoID = VB->GetContext();
	glDeleteBuffers(1, &VideoID);
	LWAllocator::Destroy(VB);
	return *this;
}

LWVideoDriver &LWVideoDriver_OpenGL4_5::DestroyPipeline(LWPipeline *Pipeline) {
	LWOpenGL4_5Pipeline *Pipe = (LWOpenGL4_5Pipeline*)Pipeline;
	if (Pipe->isInternalPipeline()) {
		LWOpenGL4_5PipelineContext &Context = Pipe->GetContext();
		glDeleteProgramPipelines(1, &Context.m_ProgramID);
		glDeleteVertexArrays(1, &Context.m_VAOID);
	}
	LWAllocator::Destroy(Pipe);
	return *this;
}

LWVideoDriver &LWVideoDriver_OpenGL4_5::DestroyShader(LWShader *Shader) {
	LWOpenGL4_5Shader *S = (LWOpenGL4_5Shader*)Shader;
	glDeleteProgram(S->GetContext());
	LWAllocator::Destroy(S);
	return *this;
}

LWVideoDriver &LWVideoDriver_OpenGL4_5::DestroyTexture(LWTexture *Texture) {
	LWOpenGL4_5Texture *T = (LWOpenGL4_5Texture*)Texture;
	uint32_t VideoID = T->GetContext();
	glDeleteTextures(1, &VideoID);
	LWAllocator::Destroy(T);
	return *this;
}

LWVideoDriver &LWVideoDriver_OpenGL4_5::DestroyFrameBuffer(LWFrameBuffer *FrameBuffer) {
	LWOpenGL4_5FrameBuffer *FB = (LWOpenGL4_5FrameBuffer*)FrameBuffer;
	auto &FBCon = FB->GetContext();
	glDeleteFramebuffers(1, &FBCon.m_FBOID);
	LWAllocator::Destroy(FB);
	return *this;
}

bool LWVideoDriver_OpenGL4_5::SetRasterState(uint64_t Flags, float Bias, float SlopedScaleBias){
	if (!LWVideoDriver::SetRasterState(Flags, Bias, SlopedScaleBias)) return false;

	const GLenum GCompareFuncs[] = { GL_ALWAYS, GL_NEVER, GL_LESS, GL_GREATER, GL_LEQUAL, GL_GEQUAL };
	const GLenum GBlendFuncs[] = { GL_ZERO, GL_ONE, GL_SRC_COLOR, GL_DST_COLOR, GL_SRC_ALPHA, GL_DST_ALPHA, GL_ONE_MINUS_SRC_COLOR, GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA };
	const GLenum GFillModes[] = { GL_FILL, GL_LINE };
	const GLenum GStencilOps[] = { GL_KEEP, GL_ZERO, GL_REPLACE, GL_INCR, GL_DECR, GL_INCR_WRAP, GL_DECR_WRAP, GL_INVERT };
	const GLenum GCullModes[] = { GL_FRONT, GL_FRONT, GL_BACK };
	auto SetState = [](bool Enable, GLenum State) {
		if (Enable) glEnable(State);
		else glDisable(State);
		return;
	};

	uint64_t SourceBlend = (Flags&LWPipeline::BLEND_SRC_BITS) >> LWPipeline::BLEND_SRC_BITOFFSET;
	uint64_t DestBlend = (Flags&LWPipeline::BLEND_DST_BITS) >> LWPipeline::BLEND_DST_BITOFFSET;
	uint64_t FillMode = (Flags&LWPipeline::FILL_MODE_BITS) >> LWPipeline::FILL_MODE_BITOFFSET;
	uint64_t CullMode = (Flags&LWPipeline::CULL_BITS) >> LWPipeline::CULL_BITOFFSET;
	uint64_t DepthCompareFunc = (Flags&LWPipeline::DEPTH_COMPARE_BITS) >> LWPipeline::DEPTH_COMPARE_BITOFFSET;
	uint64_t StencilCompareFunc = (Flags&LWPipeline::STENCIL_COMPARE_BITS) >> LWPipeline::STENCIL_COMPARE_BITOFFSET;
	uint64_t SFailOp = (Flags&LWPipeline::STENCIL_OP_SFAIL_BITS) >> LWPipeline::STENCIL_OP_SFAIL_BITOFFSET;
	uint64_t DFailOp = (Flags&LWPipeline::STENCIL_OP_DFAIL_BITS) >> LWPipeline::STENCIL_OP_DFAIL_BITOFFSET;
	uint64_t PassOp = (Flags&LWPipeline::STENCIL_OP_PASS_BITS) >> LWPipeline::STENCIL_OP_PASS_BITOFFSET;
	uint64_t StencilReadMask = (Flags&LWPipeline::STENCIL_READMASK_BITS) >> LWPipeline::STENCIL_READMASK_BITOFFSET;
	uint64_t StencilWriteMask = (Flags&LWPipeline::STENCIL_WRITEMASK_BITS) >> LWPipeline::STENCIL_WRITEMASK_BITOFFSET;
	uint64_t StencilValue = (Flags&LWPipeline::STENCIL_REF_VALUE_BITS) >> LWPipeline::STENCIL_REF_VALUE_BITOFFSET;
	bool depthBias = (Flags&LWPipeline::DEPTH_BIAS);
	bool depthEnabled = (Flags&LWPipeline::DEPTH_TEST);
	bool stencilEnabled = (Flags&LWPipeline::STENCIL_TEST);
	bool blendEnabled = (Flags&LWPipeline::BLENDING);
	bool cullEnabled = CullMode != LWPipeline::CULL_NONE;

	SetState(depthEnabled, GL_DEPTH_TEST);
	SetState(stencilEnabled, GL_STENCIL_TEST);
	SetState(blendEnabled, GL_BLEND);
	SetState(cullEnabled, GL_CULL_FACE);
	SetState(depthBias, GL_POLYGON_OFFSET_FILL);
	SetState(depthBias, GL_POLYGON_OFFSET_LINE);
	SetState(depthBias, GL_POLYGON_OFFSET_POINT);
	SetState(Flags&LWPipeline::CLIPPLANE0, GL_CLIP_PLANE0);
	SetState(Flags&LWPipeline::CLIPPLANE1, GL_CLIP_PLANE1);
	SetState(Flags&LWPipeline::CLIPPLANE2, GL_CLIP_PLANE2);
	if (depthEnabled) glDepthFunc(GCompareFuncs[DepthCompareFunc]);
	if (blendEnabled) glBlendFunc(GBlendFuncs[SourceBlend], GBlendFuncs[DestBlend]);
	if (cullEnabled) glCullFace(GCullModes[CullMode]);
	if (stencilEnabled) {
		glStencilOp(GStencilOps[SFailOp], GStencilOps[DFailOp], GStencilOps[PassOp]);
		glStencilFunc(GCompareFuncs[StencilCompareFunc], (int32_t)StencilValue, (int32_t)StencilWriteMask);
		glStencilMask((int32_t)StencilWriteMask);
	}
	if (depthBias) {
		glPolygonOffset(Bias, SlopedScaleBias);
	}
	glPolygonMode(GL_FRONT_AND_BACK, GFillModes[FillMode]);
	glDepthMask(!(Flags&LWPipeline::No_Depth));
	glColorMask(!(Flags&LWPipeline::No_ColorR), !(Flags&LWPipeline::No_ColorG), !(Flags&LWPipeline::No_ColorB), !(Flags&LWPipeline::No_ColorA));

	return true;
}

bool LWVideoDriver_OpenGL4_5::SetPipeline(LWPipeline *Pipeline, LWVideoBuffer *VertexBuffer, LWVideoBuffer *IndiceBuffer, uint32_t VertexStride, uint32_t Offset){
	//Video buffer types.
	const int32_t GBTypes[] = { GL_ARRAY_BUFFER, GL_UNIFORM_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_SHADER_STORAGE_BUFFER };
	const int32_t GIBaseType[] = { GL_FLOAT, GL_INT, GL_UNSIGNED_INT, GL_DOUBLE, GL_FLOAT, GL_FLOAT, GL_FLOAT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_INT, GL_INT, GL_INT, GL_DOUBLE, GL_DOUBLE, GL_DOUBLE, GL_FLOAT, GL_FLOAT, GL_FLOAT, GL_DOUBLE, GL_DOUBLE, GL_DOUBLE };
	const int32_t GIComponentCnt[] = { 1,    1,      1,               1,         2,        3,        4,        2,               3,               4,               2,      3,      4,      2,         3,         4,         4,        9,        16,       4,         9,         16 };

	bool Update = LWVideoDriver::SetPipeline(Pipeline, VertexBuffer, IndiceBuffer, VertexStride, Offset);
	auto &Context = ((LWOpenGL4_5Pipeline*)Pipeline)->GetContext();
	uint32_t BlockCount = Pipeline->GetBlockCount();
	uint32_t ResourceCount = Pipeline->GetResourceCount();
	if (Update) glBindProgramPipeline(Context.m_ProgramID);
	for (uint32_t i = 0; i < BlockCount; i++) {
		LWShaderResource &Block = Pipeline->GetBlock(i);
		LWOpenGL4_5Buffer *B = (LWOpenGL4_5Buffer*)Block.m_Resource;
		if(!B) continue;
		uint32_t VideoID = B->GetContext();
		LWVideoDriver::UpdateVideoBuffer(B);
		if(!Update) continue;
		uint32_t Offset = Block.m_Offset*m_UniformBlockSize;
		glBindBufferRange(GBTypes[B->GetType()], Block.m_StageBindings, VideoID, Offset, Block.GetLength());
	}
	for (uint32_t i = 0; i < ResourceCount; i++) {
		//Image Formats:      RGBA8,    RGBA8U,      RGBA16,     RGBA16U,     RGBA32,     RGBA32U,     RGBA32F,    RG8,     RG8U,     RG16,     RG16U,     RG32,     RG32U,     RG32F,    R8,     R8U,     R16,     R16U,     R32,     R32U,     R32F,    D16,     D24, D32,     D24S8, DXT1, DXT2, DXT3, DXT4, DXT5, DXT6, DXT7
		GLenum IFormats[] = { GL_RGBA8I, GL_RGBA8UI, GL_RGBA16I, GL_RGBA16UI, GL_RGBA32I, GL_RGBA32UI, GL_RGBA32F, GL_RG8I, GL_RG8UI, GL_RG16I, GL_RG16UI, GL_RG32I, GL_RG32UI, GL_RG32F, GL_R8I, GL_R8UI, GL_R16I, GL_R16UI, GL_R32I, GL_R32UI, GL_R32F, GL_R16F, 0,   GL_R32F, 0,     0,    0,    0,    0,    0,    0,    0 };
		LWShaderResource &R = Pipeline->GetResource(i);
		LWOpenGL4_5Texture *T = (LWOpenGL4_5Texture *)R.m_Resource;
		LWOpenGL4_5Buffer *B = (LWOpenGL4_5Buffer *)R.m_Resource;
		uint32_t TypeID = R.GetTypeID();
		if (TypeID == LWPipeline::Texture) {
			if (!T) continue;
			glActiveTexture(GL_TEXTURE0 + R.m_StageBindings);
			UpdateTexture(T);
		} else if (TypeID == LWPipeline::Image) {
			if (!T) continue;
			if(Update) glBindImageTexture(R.m_StageBindings, T->GetContext(), 0, false, 0, GL_READ_WRITE, IFormats[T->GetPackType()]);
		} else if (TypeID == LWPipeline::ImageBuffer) {
			if(!B) continue;
			LWVideoDriver::UpdateVideoBuffer(B);
			if (Update) glBindBufferBase(GBTypes[B->GetType()], R.m_StageBindings, B->GetContext());
		}
	}

	if (VertexBuffer) {
		LWOpenGL4_5Buffer *VBuffer = (LWOpenGL4_5Buffer*)VertexBuffer;
		LWVideoDriver::UpdateVideoBuffer(VBuffer);
		glBindVertexArray(Context.m_VAOID);
		glBindBuffer(GL_ARRAY_BUFFER, VBuffer->GetContext());
		for (uint32_t i = 0; i < Pipeline->GetInputCount(); i++) {
			LWShaderInput &I = Pipeline->GetInput(i);
			int32_t GBaseType = GIBaseType[I.m_Type];
			int32_t GCompCnt = GIComponentCnt[I.m_Type];
			if (GBaseType == GL_INT || GBaseType == GL_UNSIGNED_INT) {
				glVertexAttribIPointer(I.m_BindIndex, GCompCnt, GBaseType, VertexStride, (void*)(uintptr_t)I.m_Offset);
			} else {
				glVertexAttribPointer(I.m_BindIndex, GCompCnt, GBaseType, false, VertexStride, (void*)(uintptr_t)I.m_Offset);
			}
		}
	}
	if (IndiceBuffer) {
		LWVideoDriver::UpdateVideoBuffer(IndiceBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ((LWOpenGL4_5Buffer*)IndiceBuffer)->GetContext());
	} else glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	return Update;
}

bool LWVideoDriver_OpenGL4_5::SetFrameBuffer(LWFrameBuffer *Buffer, bool ChangeViewport) {
	if (!LWVideoDriver::SetFrameBuffer(Buffer, ChangeViewport)) return false;
	if (!Buffer) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		m_ActiveDrawCount = 1;
	} else {
		auto &Context = ((LWOpenGL4_5FrameBuffer*)Buffer)->GetContext();
		GLenum AttachmentPnts[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, GL_DEPTH_STENCIL_ATTACHMENT };
		const GLenum GFaces[] = { GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, GL_TEXTURE_CUBE_MAP_POSITIVE_Y,GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, GL_TEXTURE_CUBE_MAP_POSITIVE_Z };
		 
		GLenum DrawBuffers[LWFrameBuffer::Count];
		m_ActiveDrawCount = 0;
		glBindFramebuffer(GL_FRAMEBUFFER, Context.m_FBOID);
		for (uint32_t i = 0; i < LWFrameBuffer::Count; i++) {
			auto &Slot = Buffer->GetAttachment(i);			
			if (!Slot.m_Source) {
				if (Context.m_Attached[i]) glFramebufferRenderbuffer(GL_FRAMEBUFFER, Context.m_Attached[i], GL_RENDERBUFFER, 0);
				Context.m_Attached[i] = 0;
				continue;
			}
			LWOpenGL4_5Texture *Tex = (LWOpenGL4_5Texture*)Slot.m_Source;
			uint32_t TexType = Tex->GetType();
			uint32_t TexID = Tex->GetContext();
			if (i == LWFrameBuffer::Depth) {
				if (!LWImage::DepthType(TexID)) AttachmentPnts[i] = GL_STENCIL_ATTACHMENT;
				if (!LWImage::StencilType(TexID)) AttachmentPnts[i] = GL_DEPTH_ATTACHMENT;
			}
			Context.m_Attached[i] = AttachmentPnts[i];
			if (Tex->GetTextureState()&LWTexture::RenderBuffer) glFramebufferRenderbuffer(GL_FRAMEBUFFER, AttachmentPnts[i], GL_RENDERBUFFER, TexID);
			else {
				if (TexType == LWTexture::Texture1D) glFramebufferTexture1D(GL_FRAMEBUFFER, AttachmentPnts[i], GL_TEXTURE_1D, TexID, Slot.m_Mipmap);
				else if (TexType == LWTexture::Texture2D) glFramebufferTexture2D(GL_FRAMEBUFFER, AttachmentPnts[i], GL_TEXTURE_2D, TexID, Slot.m_Mipmap);
				else if (TexType == LWTexture::Texture3D) glFramebufferTexture3D(GL_FRAMEBUFFER, AttachmentPnts[i], GL_TEXTURE_3D, TexID, Slot.m_Mipmap, Slot.m_Layer);
				else if (TexType == LWTexture::TextureCubeMap) glFramebufferTexture2D(GL_FRAMEBUFFER, AttachmentPnts[i], GFaces[Slot.m_Face], TexID, Slot.m_Mipmap);
				else if (TexType == LWTexture::Texture2DMS) glFramebufferTexture2D(GL_FRAMEBUFFER, AttachmentPnts[i], GL_TEXTURE_2D_MULTISAMPLE, TexID, 0);
				else if (TexType == LWTexture::Texture2DArray) glFramebufferTextureLayer(GL_FRAMEBUFFER, AttachmentPnts[i], TexID, Slot.m_Mipmap, Slot.m_Layer);
				else if (TexType == LWTexture::TextureCubeMapArray) glFramebufferTextureLayer(GL_FRAMEBUFFER, AttachmentPnts[i], TexID, Slot.m_Mipmap, Slot.m_Layer * 6 + Slot.m_Face);
				else if (TexType == LWTexture::Texture2DMSArray) glFramebufferTextureLayer(GL_FRAMEBUFFER, AttachmentPnts[i], TexID, 0, Slot.m_Layer);
			}
			if (i < LWFrameBuffer::ColorCount) DrawBuffers[m_ActiveDrawCount++] = AttachmentPnts[i];
		}
		glDrawBuffers(m_ActiveDrawCount, DrawBuffers);
	}
	return true;
}

LWVideoDriver &LWVideoDriver_OpenGL4_5::ClearColor(uint32_t Color) {
	SetFrameBuffer(m_ActiveFrameBuffer);
	LWVector4f ClearClr = LWUNPACK_COLORVEC4f(Color);
	for (uint32_t i = 0; i < m_ActiveDrawCount; i++) {
		glClearBufferfv(GL_COLOR, i, &ClearClr.x);
	}
	/*glClearColor(ClearClr.x, ClearClr.y, ClearClr.z, ClearClr.w);
	glClear(GL_COLOR_BUFFER_BIT);*/
	return *this;
}

LWVideoDriver &LWVideoDriver_OpenGL4_5::ClearColor(const LWVector4f &Color) {
	SetFrameBuffer(m_ActiveFrameBuffer);
	for (uint32_t i = 0; i < m_ActiveDrawCount; i++) {
		glClearBufferfv(GL_COLOR, i, &Color.x);
	}
	//glClearColor(Color.x, Color.y, Color.z, Color.w);
	//glClear(GL_COLOR_BUFFER_BIT);
	return *this;
}

LWVideoDriver &LWVideoDriver_OpenGL4_5::ClearDepth(float Depth) {
	//SetFrameBuffer(m_ActiveFrameBuffer);
	//glClearBufferfi(GL_DEPTH, 0, Depth, 0);
	glClearDepth(Depth);
	glClear(GL_DEPTH_BUFFER_BIT);
	return *this;
}

LWVideoDriver &LWVideoDriver_OpenGL4_5::ClearStencil(uint8_t Stencil) {
	//SetFrameBuffer(m_ActiveFrameBuffer);
	//glClearBufferfi(GL_STENCIL, 0, 0.0f, (int32_t)Stencil);
	glClearStencil((int32_t)Stencil);
	glClear(GL_STENCIL_BUFFER_BIT);
	return *this;
}

LWVideoDriver &LWVideoDriver_OpenGL4_5::DrawBuffer(LWPipeline *Pipeline, int32_t DrawMode, LWVideoBuffer *InputBlock, LWVideoBuffer *IndexBuffer, uint32_t Count, uint32_t VertexStride, uint32_t Offset) {
	const int32_t GModes[] = { GL_POINTS, GL_LINE_STRIP, GL_LINES, GL_TRIANGLE_STRIP, GL_TRIANGLES };
	SetFrameBuffer(m_ActiveFrameBuffer);
	SetPipeline(Pipeline, InputBlock, IndexBuffer, VertexStride, Offset);
	if (IndexBuffer) {
		uint32_t OffsetSize = IndexBuffer->GetType() == LWVideoBuffer::Index16 ? sizeof(uint16_t) : sizeof(uint32_t);
		GLenum IndexType = IndexBuffer->GetType() == LWVideoBuffer::Index16 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
		glDrawElements(GModes[DrawMode], Count, IndexType, (void*)(uintptr_t)(Offset*OffsetSize));
	} else {
		glDrawArrays(GModes[DrawMode], Offset, Count);
	}
	return *this;
}

LWVideoDriver &LWVideoDriver_OpenGL4_5::DrawInstancedBuffer(LWPipeline *Pipeline, int32_t DrawMode, LWVideoBuffer *InputBlock, LWVideoBuffer *IndexBuffer, uint32_t Count, uint32_t VertexStride, uint32_t InstanceCount, uint32_t Offset) {
	const int32_t GModes[] = { GL_POINTS, GL_LINE_STRIP, GL_LINES, GL_TRIANGLE_STRIP, GL_TRIANGLES };
	SetFrameBuffer(m_ActiveFrameBuffer);
	SetPipeline(Pipeline, InputBlock, IndexBuffer, VertexStride, Offset);
	if (IndexBuffer) {
		uint32_t OffsetSize = IndexBuffer->GetType() == LWVideoBuffer::Index16 ? sizeof(uint16_t) : sizeof(uint32_t);
		GLenum IndexType = IndexBuffer->GetType() == LWVideoBuffer::Index16 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
		glDrawElementsInstanced(GModes[DrawMode], Count, IndexType, (void*)(uintptr_t)(Offset*OffsetSize), InstanceCount);
	} else {
		glDrawArraysInstanced(GModes[DrawMode], Offset, Count, InstanceCount);
	}
	return *this;
}

LWVideoDriver &LWVideoDriver_OpenGL4_5::Dispatch(LWPipeline *Pipeline, const LWVector3i &GroupDimension){
	SetFrameBuffer(m_ActiveFrameBuffer);
	SetPipeline(Pipeline, nullptr, nullptr, 0, 0);
	glDispatchCompute(GroupDimension.x, GroupDimension.y, GroupDimension.z);
	return *this;
}

LWOpenGL4_5Context &LWVideoDriver_OpenGL4_5::GetContext(void) {
	return m_Context;
}

LWVideoDriver_OpenGL4_5::LWVideoDriver_OpenGL4_5(LWWindow *Window, LWOpenGL4_5Context &Context, uint32_t UniformBlockSize) : LWVideoDriver(Window, OpenGL4_5, UniformBlockSize) {
	m_Context = Context;
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}