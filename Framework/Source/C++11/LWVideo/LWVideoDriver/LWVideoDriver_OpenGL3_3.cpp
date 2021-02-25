#include "LWVideo/LWVideoDrivers/LWVideoDriver_OpenGL3_3.h"
#include "LWPlatform/LWWindow.h"
#include "LWCore/LWVector.h"
#include "LWCore/LWMatrix.h"
#include "LWVideo/LWImage.h"
#include "LWVideo/LWFrameBuffer.h"
#include "LWVideo/LWPipeline.h"
#include <iostream>

LWVideoDriver &LWVideoDriver_OpenGL3_3::ViewPort(const LWVector4i &Viewport){
	m_Viewport = Viewport;
	glViewport(Viewport.x, Viewport.y, Viewport.z, Viewport.w);
	return *this;
}

LWShader *LWVideoDriver_OpenGL3_3::CreateShader(uint32_t ShaderType, const LWUTF8Iterator &Source, LWAllocator &Allocator, char *CompiledBuffer, char8_t *ErrorBuffer, uint32_t &CompiledBufferLen, uint32_t ErrorBufferLen) {
	CompiledBufferLen = 0;
	return CreateShaderCompiled(ShaderType, (const char*)Source(), Source.RawDistance(Source.NextEnd()), Allocator, ErrorBuffer, ErrorBufferLen);
}

LWShader *LWVideoDriver_OpenGL3_3::CreateShaderCompiled(uint32_t ShaderType, const char *CompiledCode, uint32_t CompiledCodeLen, LWAllocator &Allocator, char8_t *ErrorBuffer, uint32_t ErrorBufferLen) {
	GLenum GShaderTypes[] = { GL_VERTEX_SHADER, GL_GEOMETRY_SHADER, GL_FRAGMENT_SHADER,  GL_COMPUTE_SHADER };
	uint32_t ShaderID = glCreateShader(GShaderTypes[ShaderType]);
	int32_t CompileResult = 0;
	glShaderSource(ShaderID, 1, &CompiledCode, nullptr);
	glCompileShader(ShaderID);
	glGetShaderiv(ShaderID, GL_COMPILE_STATUS, &CompileResult);
	if (!CompileResult) {
		if (ErrorBuffer) {
			*ErrorBuffer = '\0';
			int32_t Len = 0;
			glGetShaderInfoLog(ShaderID, ErrorBufferLen, &Len, ErrorBuffer);
			PerformErrorAnalysis(CompiledCode, ErrorBuffer, ErrorBufferLen);
		}
		glDeleteShader(ShaderID);
		return nullptr;
	}
	return Allocator.Create<LWOpenGL3_3Shader>(ShaderID, LWCrypto::HashFNV1A((const uint8_t*)CompiledCode, CompiledCodeLen), ShaderType);
}

LWPipeline *LWVideoDriver_OpenGL3_3::CreatePipeline(LWShader **Stages, uint64_t Flags, LWAllocator &Allocator) {
	LWOpenGL3_3PipelineContext Context;
	LWPipeline *P = Allocator.Create<LWOpenGL3_3Pipeline>(Context, Stages, nullptr, nullptr, nullptr, 0, 0, 0, Flags&~LWPipeline::InternalPipeline);
	if (P) UpdatePipelineStages(P);
	return P;
}

LWPipeline *LWVideoDriver_OpenGL3_3::CreatePipeline(LWPipeline *Source, LWAllocator &Allocator) {
	char ErrorBuffer[1024];
	char NameBuffer[1024];
	if (Source->isComputePipeline()) return nullptr;
	LWShader *Stages[LWPipeline::StageCount] = { Source->GetShaderStage(0), Source->GetShaderStage(1), Source->GetShaderStage(2) };
	LWShaderResource Blocks[LWShader::MaxBlocks];
	LWShaderResource Resources[LWShader::MaxResources];
	LWShaderInput Inputs[LWShader::MaxInputs];
	LWOpenGL3_3PipelineContext Context;
	uint32_t ActiveProgramID = m_ActivePipeline ? ((LWOpenGL3_3Pipeline*)m_ActivePipeline)->GetContext().m_ProgramID : 0;
	int32_t BlockCount = 0;
	int32_t ResourceCount = 0;
	int32_t NameLen = 0;
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
		else if (Type == GL_UNSIGNED_INT_VEC3) return { LWShaderInput::uVec3,  1 };
		else if (Type == GL_UNSIGNED_INT_VEC4) return { LWShaderInput::uVec4, 1 };
		else if (Type == GL_FLOAT_MAT2) return { LWShaderInput::Vec2, 2 };
		else if (Type == GL_FLOAT_MAT3) return { LWShaderInput::Vec3, 3 };
		else if (Type == GL_FLOAT_MAT4) return { LWShaderInput::Vec4, 4 };
		else if (Type == GL_FLOAT_MAT2x3) return { LWShaderInput::Vec2, 3 };
		else if (Type == GL_FLOAT_MAT2x4) return { LWShaderInput::Vec2, 4 };
		else if (Type == GL_FLOAT_MAT3x2) return { LWShaderInput::Vec3, 2 };
		else if (Type == GL_FLOAT_MAT3x4) return { LWShaderInput::Vec3, 4 };
		else if (Type == GL_FLOAT_MAT4x2) return { LWShaderInput::Vec4, 2 };
		else if (Type == GL_FLOAT_MAT4x3) return { LWShaderInput::Vec4, 3 };
		fmt::print("Unknown type: {}\n", Type);
		return { LWShaderInput::Float, 1 };
	};

	Context.m_ProgramID = glCreateProgram();
	if (!Context.m_ProgramID) return nullptr;
	glGenVertexArrays(1, &Context.m_VAOID);
	if (!Context.m_VAOID) return nullptr;
	glBindVertexArray(Context.m_VAOID);
	auto VS = (LWOpenGL3_3Shader*)Stages[LWPipeline::Vertex];
	auto GS = (LWOpenGL3_3Shader*)Stages[LWPipeline::Geometry];
	auto PS = (LWOpenGL3_3Shader*)Stages[LWPipeline::Pixel];
	if (VS) glAttachShader(Context.m_ProgramID, VS->GetContext());
	if (GS) glAttachShader(Context.m_ProgramID, GS->GetContext());
	if (PS) glAttachShader(Context.m_ProgramID, PS->GetContext());
	int32_t LinkStatus = 0;
	int32_t Len = 0;
	glLinkProgram(Context.m_ProgramID);
	glGetProgramiv(Context.m_ProgramID, GL_LINK_STATUS, &LinkStatus);
	if (!LinkStatus) {
		*ErrorBuffer = '\0';
		glGetProgramInfoLog(Context.m_ProgramID, sizeof(ErrorBuffer), &Len, ErrorBuffer);
		glDeleteProgram(Context.m_ProgramID);
		fmt::print("Error in pipeline:\n{}\n", ErrorBuffer);
		return nullptr;
	}
	glUseProgram(Context.m_ProgramID);
	int32_t UniformCount = 0;
	int32_t AttributeCount = 0;
	int32_t NextTexID = 0;
	uint32_t InputCount = 0;
	glGetProgramiv(Context.m_ProgramID, GL_ACTIVE_ATTRIBUTES, &AttributeCount);
	glGetProgramiv(Context.m_ProgramID, GL_ACTIVE_UNIFORM_BLOCKS, &BlockCount);
	glGetProgramiv(Context.m_ProgramID, GL_ACTIVE_UNIFORMS, &UniformCount);
	for (int32_t i = 0; i < AttributeCount; i++) {
		uint32_t Type = 0;
		int32_t Length = 0;
		glGetActiveAttrib(Context.m_ProgramID, i, sizeof(NameBuffer), &NameLen, &Length, &Type, NameBuffer);
		AttributeMap Attr = MapAttributeType(Type);
		for (uint32_t n = 0; n < Length*Attr.RowMultiplier; n++) {
			LWShaderInput &In = Inputs[InputCount];
			In = LWShaderInput((const char8_t*)NameBuffer, Attr.AttributeType, 1);
			In.m_BindIndex = glGetAttribLocation(Context.m_ProgramID, NameBuffer);
			glEnableVertexAttribArray(In.m_BindIndex);
			InputCount++;
		}
	}
	for (int32_t i = 0; i < BlockCount; i++) {
		glGetActiveUniformBlockName(Context.m_ProgramID, i, sizeof(NameBuffer), &NameLen, NameBuffer);
		Blocks[i] = LWShaderResource(LWUTF8I(NameBuffer).Hash(), LWPipeline::UniformBlock, 0, i);
		glUniformBlockBinding(Context.m_ProgramID, i, i);

	}
	for (int32_t i = 0; i < UniformCount; i++) {
		LWShaderResource &R = Resources[ResourceCount];
		uint32_t Type = 0;
		int32_t Length = 0;
		glGetActiveUniform(Context.m_ProgramID, i, sizeof(NameBuffer), &NameLen, &Length, &Type, NameBuffer);
		LWUTF8Iterator NIter = LWUTF8Iterator(NameBuffer);
		uint32_t NameHash = NIter.Hash();
		if (Type == GL_SAMPLER_1D || Type == GL_SAMPLER_2D || Type == GL_SAMPLER_3D ||
			Type == GL_SAMPLER_CUBE || Type == GL_SAMPLER_1D_SHADOW || Type == GL_SAMPLER_2D_SHADOW || Type == GL_SAMPLER_CUBE_SHADOW ||
			Type == GL_SAMPLER_2D_MULTISAMPLE || Type==GL_SAMPLER_2D_MULTISAMPLE_ARRAY) {
			R = LWShaderResource(NameHash, LWPipeline::Texture, Length, NextTexID);
			glUniform1i(ResourceCount, NextTexID);
			NextTexID++;
			ResourceCount++;
		}
	}
	glUseProgram(ActiveProgramID);
	return Allocator.Create<LWOpenGL3_3Pipeline>(Context, Stages, Blocks, Resources, Inputs, BlockCount, ResourceCount, InputCount, LWPipeline::InternalPipeline);
}

LWVideoDriver &LWVideoDriver_OpenGL3_3::ClonePipeline(LWPipeline *Target, LWPipeline *Source) {
	LWVideoDriver::ClonePipeline(Target, Source);
	LWOpenGL3_3PipelineContext &SContext = ((LWOpenGL3_3Pipeline*)Source)->GetContext();
	LWOpenGL3_3PipelineContext &TContext = ((LWOpenGL3_3Pipeline*)Target)->GetContext();
	TContext = SContext;
	return *this;
}

LWTexture *LWVideoDriver_OpenGL3_3::CreateTexture1D(uint32_t TextureState, uint32_t PackType, uint32_t Size, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator) {
	//PackTypes:                         RGBA8,            RGBA8U,           RGBA16,            RGBA16,            RGBA32,          RGBA32U,         RGBA32F,    RG8,              RG8U,             RG16,              RG16U,             RG32,            RG32U,           RG32F,    R8,               R8U,              R16,               R16U,              R32,             R32U,            R32F,     Depth16,              Depth24,              Depth32,              Depth24Stencil8, DXT1, DXT2, DXT3, DXT4, DXT5, DXT6, DXT7
	const int32_t GInternalFormats[] = { GL_RGBA8,         GL_RGBA8,         GL_RGBA16,         GL_RGBA16,         GL_RGBA32I,      GL_RGBA32UI,     GL_RGBA32F, GL_RG8,           GL_RG8,           GL_RG16,           GL_RG16,           GL_RG32I,        GL_RG32UI,       GL_RG32F, GL_R8,            GL_R8,            GL_R16,            GL_R16,            GL_R32I,         GL_R32UI,        GL_R32F,  0,                    0,                    0,                    0,               0,    0,    0,    0,    0,    0,    0 };
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
	uint32_t Mips = MakeMipmaps ? 0 : MipmapCnt;
	uint32_t t = 0;
	for (uint32_t i = 0; i <= Mips; i++, t++) {
		uint32_t S = LWImage::MipmapSize1D(Size, i);
		glTexImage1D(GL_TEXTURE_1D, i, GInternalFormats[PackType], S, 0, GFormats[PackType], GType[PackType], Texels ? Texels[t] : nullptr);
	}
	//if (MakeMipmaps) glGenerateMipmap(GL_TEXTURE_1D);
	return Allocator.Create<LWOpenGL3_3Texture>(VideoID, TextureState, PackType, MipmapCnt, LWVector3i(Size, 0, 0), LWTexture::Texture1D);
}

LWTexture *LWVideoDriver_OpenGL3_3::CreateTexture2D(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator){
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
		return Allocator.Create<LWOpenGL3_3Texture>(VideoID, TextureState&~(LWTexture::MakeMipmaps), PackType, 0, LWVector3i(Size, 0), LWTexture::Texture2D);
	}
	bool MakeMipmaps = (TextureState&LWTexture::MakeMipmaps) != 0;
	bool Compressed = LWImage::CompressedType(PackType);
	if (MakeMipmaps) MipmapCnt = Compressed ? 0 : LWImage::MipmapCount(Size);
	TextureState = (TextureState&~LWTexture::MakeMipmaps);
	glGenTextures(1, &VideoID);
	glBindTexture(GL_TEXTURE_2D, VideoID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, MipmapCnt);
	uint32_t Mips = MakeMipmaps ? 0 : MipmapCnt;
	uint32_t t = 0;
	for (uint32_t i = 0; i <= Mips; i++, t++) {
		LWVector2i S = LWImage::MipmapSize2D(Size, i);
		if (Compressed) glCompressedTexImage2D(GL_TEXTURE_2D, i, GInternalFormats[PackType], S.x, S.y, 0, LWImage::GetLength2D(S, PackType), Texels ? Texels[t] : nullptr);
		else glTexImage2D(GL_TEXTURE_2D, i, GInternalFormats[PackType], S.x, S.y, 0, GFormats[PackType], GType[PackType], Texels ? Texels[t] : nullptr);
	}
	//if (MakeMipmaps) glGenerateMipmap(GL_TEXTURE_2D);
	return Allocator.Create<LWOpenGL3_3Texture>(VideoID, TextureState, PackType, MipmapCnt, LWVector3i(Size, 0), LWTexture::Texture2D);
}

LWTexture *LWVideoDriver_OpenGL3_3::CreateTexture3D(uint32_t TextureState, uint32_t PackType, const LWVector3i &Size, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator){
	//PackTypes:                         RGBA8,            RGBA8U,           RGBA16,            RGBA16,            RGBA32,          RGBA32U,         RGBA32F,    RG8,              RG8U,             RG16,              RG16U,             RG32,            RG32U,           RG32F,    R8,               R8U,              R16,               R16U,              R32,             R32U,            R32F,     Depth16,              Depth24,              Depth32,               Depth24Stencil8,     DXT1, DXT2, DXT3, DXT4, DXT5, DXT6, DXT7
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
	glBindTexture(GL_TEXTURE_3D, VideoID);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, MipmapCnt);
	uint32_t Mips = MakeMipmaps ? 0 : MipmapCnt;
	uint32_t t = 0;
	for (uint32_t i = 0; i <= Mips; i++, t++) {
		LWVector3i S = LWImage::MipmapSize3D(Size, i);
		if (Compressed) glCompressedTexImage3D(GL_TEXTURE_3D, i, GInternalFormats[PackType], Size.x, Size.y, Size.z, 0, LWImage::GetLength3D(Size, PackType), Texels ? Texels[t] : nullptr);
		else glTexImage3D(GL_TEXTURE_3D, i, GInternalFormats[PackType], Size.x, Size.y, Size.z, 0, GFormats[PackType], GType[PackType], Texels ? Texels[t] : nullptr);
	}
	//if (MakeMipmaps) glGenerateMipmap(GL_TEXTURE_3D);
	return Allocator.Create<LWOpenGL3_3Texture>(VideoID, TextureState, PackType, MipmapCnt, Size, LWTexture::Texture3D);
}

LWTexture *LWVideoDriver_OpenGL3_3::CreateTextureCubeMap(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator){
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
	uint32_t Mips = MakeMipmaps ? 0 : MipmapCnt;
	uint32_t t = 0;
	for (uint32_t d = 0; d < 6; d++) {
		uint32_t GTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + d;
		for (uint32_t i = 0; i <= Mips; i++, t++) {
			LWVector2i S = LWImage::MipmapSize2D(Size, i);
			if (Compressed) glCompressedTexImage2D(GTarget, i, GInternalFormats[PackType], S.x, S.y, 0, LWImage::GetLength2D(S, PackType), Texels ? Texels[t] : nullptr);
			else glTexImage2D(GTarget, i, GInternalFormats[PackType], S.x, S.y, 0, GFormats[PackType], GType[PackType], Texels ? Texels[t] : nullptr);
		}
	}
	//if (MakeMipmaps) glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	return Allocator.Create<LWOpenGL3_3Texture>(VideoID, TextureState, PackType, MipmapCnt, LWVector3i(Size, 0), LWTexture::TextureCubeMap);
}

LWTexture *LWVideoDriver_OpenGL3_3::CreateTexture2DMS(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint32_t Samples, LWAllocator &Allocator){
	//PackTypes:                         RGBA8,            RGBA8U,           RGBA16,            RGBA16,            RGBA32,          RGBA32U,         RGBA32F,    RG8,              RG8U,             RG16,              RG16U,             RG32,            RG32U,           RG32F,    R8,               R8U,              R16,               R16U,              R32,             R32U,            R32F,     Depth16,              Depth24,              Depth32,               Depth24Stencil8,     DXT1,                            DXT2,                             DXT3                              DXT4 DXT5                              DXT6                                     DXT7
	const int32_t GInternalFormats[] = { GL_RGBA8,         GL_RGBA8,         GL_RGBA16,         GL_RGBA16,         GL_RGBA32I,      GL_RGBA32UI,     GL_RGBA32F, GL_RG8,           GL_RG8,           GL_RG16,           GL_RG16,           GL_RG32I,        GL_RG32UI,       GL_RG32F, GL_R8,            GL_R8,            GL_R16,            GL_R16,            GL_R32I,         GL_R32UI,        GL_R32F,  GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32F, GL_DEPTH24_STENCIL8, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0,   GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT };
	const int32_t GFormats[] = { GL_RGBA,          GL_RGBA,          GL_RGBA,           GL_RGBA,           GL_RGBA,         GL_RGBA,         GL_RGBA,    GL_RG,            GL_RG,            GL_RG,             GL_RG,             GL_RG,           GL_RG,           GL_RG,    GL_RED,           GL_RED,           GL_RED,            GL_RED,            GL_RED,          GL_RED,          GL_RED,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,    GL_DEPTH_STENCIL };
	const int32_t GType[] = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT,   GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_FLOAT,             GL_FLOAT,             GL_FLOAT,              GL_UNSIGNED_INT_24_8 };
	if (GInternalFormats[PackType] == 0) return nullptr;

	uint32_t VideoID = 0;
	uint32_t Type = 0;
	TextureState = (TextureState & ~(LWTexture::MakeMipmaps|LWTexture::RenderBuffer));
	bool MakeMipmaps = (TextureState & LWTexture::MakeMipmaps) != 0;
	bool Compressed = LWImage::CompressedType(PackType);
	glGenTextures(1, &VideoID);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, VideoID);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, Samples, GInternalFormats[PackType], Size.x, Size.y, true);
	return Allocator.Create<LWOpenGL3_3Texture>(VideoID, TextureState, PackType, Samples, LWVector3i(Size, 0), LWTexture::Texture2DMS);
}


LWTexture *LWVideoDriver_OpenGL3_3::CreateTexture1DArray(uint32_t TextureState, uint32_t PackType, uint32_t Size, uint32_t Layers, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator) {
	return nullptr;
}

LWTexture *LWVideoDriver_OpenGL3_3::CreateTexture2DArray(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint32_t Layers, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator) {
	return nullptr;
}

LWTexture *LWVideoDriver_OpenGL3_3::CreateTextureCubeArray(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint32_t Layers, uint8_t **Texels, uint32_t MapmapCnt, LWAllocator &Allocator) {
	return nullptr;
}

LWTexture *LWVideoDriver_OpenGL3_3::CreateTexture2DMSArray(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint32_t Samples, uint32_t Layers, LWAllocator &Allocator) {
	return nullptr;
}

LWVideoBuffer *LWVideoDriver_OpenGL3_3::CreateVideoBuffer(uint32_t Type, uint32_t UsageFlag, uint32_t TypeSize, uint32_t Length, LWAllocator &Allocator, const uint8_t *Buffer){
	int32_t GTypes[] = {GL_ARRAY_BUFFER, GL_UNIFORM_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_TEXTURE_BUFFER};
	
	//Static, WriteDiscard, WriteNoOverlap, Readable, GPUResource
	uint32_t UsageID = UsageFlag & LWVideoBuffer::UsageFlag;

	int32_t GUsage = GL_DYNAMIC_DRAW;
	if (UsageID == LWVideoBuffer::Static) GUsage = GL_STATIC_DRAW;
	else if (UsageID == LWVideoBuffer::Readable) GUsage = GL_DYNAMIC_READ;

	uint32_t VideoID = 0;
	glGenBuffers(1, &VideoID);
	glBindBuffer(GTypes[Type], VideoID);
	glBufferData(GTypes[Type], TypeSize*Length, Buffer, GUsage);
	return Allocator.Create<LWOpenGL3_3Buffer>(Buffer, &Allocator, TypeSize, Length, UsageFlag|Type, VideoID);
}

LWFrameBuffer *LWVideoDriver_OpenGL3_3::CreateFrameBuffer(const LWVector2i &Size, LWAllocator &Allocator) {
	LWOpenGL3_3FrameBufferContext Con;
	glGenFramebuffers(1, &Con.m_FBOID);
	return Allocator.Create<LWOpenGL3_3FrameBuffer>(Con, Size);
}

bool LWVideoDriver_OpenGL3_3::UpdateTexture(LWTexture *Texture){
	uint32_t VideoID = ((LWOpenGL3_3Texture*)Texture)->GetContext();

	int32_t GTypes[] = { GL_TEXTURE_1D, GL_TEXTURE_2D, GL_TEXTURE_3D, GL_TEXTURE_CUBE_MAP, 0, 0, 0, GL_TEXTURE_2D_MULTISAMPLE, 0 };
	int32_t MinMagFilters[] = { GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR };
	int32_t WrapFilters[] = { GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER, GL_MIRRORED_REPEAT, GL_REPEAT };
	int32_t CompareModes[] = { GL_NONE, GL_COMPARE_REF_TO_TEXTURE };
	int32_t CompareFuncs[] = { GL_NEVER, GL_ALWAYS, GL_LESS, GL_EQUAL, GL_LEQUAL, GL_GREATER, GL_GEQUAL, GL_NOTEQUAL };
	int32_t DepthReadMode[] = { GL_DEPTH_COMPONENT, GL_STENCIL_COMPONENTS }; //Possible bug, GL_STENCIL_COMPONENTS instead of GL_STENCIL_COMPONENT (note: S) is only defined.
	float AnisotropyValues[] = { 1.0f, 2.0f, 4.0f, 8.0f, 16.0f }; //Anisotropy

	int32_t Type = GTypes[Texture->GetType()];
	if (!Type) return false;

	glBindTexture(Type, VideoID);
	if(Texture->isDirty()){
		bool MultiSampled = Texture->isMultiSampled();
		uint32_t State = Texture->GetTextureState();
		uint32_t MinFilter = (State&LWTexture::MinFilterFlag) >> LWTexture::MinFilterBitOffset;
		uint32_t MagFilter = (State&LWTexture::MagFilterFlag) >> LWTexture::MagFilterBitOffset;
		uint32_t WrapS = (State&LWTexture::WrapSFilterFlag) >> LWTexture::WrapSFilterBitOffset;
		uint32_t WrapT = (State&LWTexture::WrapTFilterFlag) >> LWTexture::WrapTFilterBitOffset;
		uint32_t WrapR = (State&LWTexture::WrapRFilterFlag) >> LWTexture::WrapRFilterBitOffset;
		uint32_t CFunc = (State&LWTexture::CompareFuncFlag) >> LWTexture::CompareFuncBitOffset;
		uint32_t CMode = (State&LWTexture::CompareModeFlag) >> LWTexture::CompareModeBitOffset;
		uint32_t DRMode = (State&LWTexture::DepthReadFlag) >> LWTexture::DepthReadBitOffset;
		uint32_t Anisotropy = (State & LWTexture::AnisotropyFlag) >> LWTexture::AnisotropyBitOffset;

		if (!MultiSampled) { //Sampler states not supported by multi-sampled textures
			glTexParameteri(Type, GL_TEXTURE_MIN_FILTER, MinMagFilters[MinFilter]);
			glTexParameteri(Type, GL_TEXTURE_MAG_FILTER, MinMagFilters[MagFilter]);
			glTexParameteri(Type, GL_TEXTURE_WRAP_S, WrapFilters[WrapS]);
			glTexParameteri(Type, GL_TEXTURE_WRAP_T, WrapFilters[WrapT]);
			glTexParameteri(Type, GL_TEXTURE_WRAP_R, WrapFilters[WrapR]);
			glTexParameteri(Type, GL_TEXTURE_COMPARE_FUNC, CompareFuncs[CFunc]);
			glTexParameteri(Type, GL_TEXTURE_COMPARE_MODE, CompareModes[CMode]);
			glTexParameteri(Type, GL_DEPTH_STENCIL_TEXTURE_MODE, DepthReadMode[DRMode]);
			glTexParameterf(Type, GL_TEXTURE_MAX_ANISOTROPY_EXT, AnisotropyValues[Anisotropy]);
		}

		Texture->ClearDirty();
	}
	return true;
}

bool LWVideoDriver_OpenGL3_3::UpdateTexture1D(LWTexture *Texture, uint32_t MipmapLevel, void *Texels, uint32_t Position, uint32_t Size){
	const int32_t GInternalFormats[] = { GL_RGBA8,         GL_RGBA8,         GL_RGBA16,         GL_RGBA16,         GL_RGBA32I,      GL_RGBA32UI,     GL_RGBA32F, GL_RG8,           GL_RG8,           GL_RG16,           GL_RG16,           GL_RG32I,        GL_RG32UI,       GL_RG32F, GL_R8,            GL_R8,            GL_R16,            GL_R16,            GL_R32I,         GL_R32UI,        GL_R32F,  0,       0,       0,       0,               0,    0,    0,    0,    0,    0,    0 };
	const int32_t GFormats[]         = { GL_RGBA,          GL_RGBA,          GL_RGBA,           GL_RGBA,           GL_RGBA,         GL_RGBA,         GL_RGBA,    GL_RG,            GL_RG,            GL_RG,             GL_RG,             GL_RG,           GL_RG,           GL_RG,    GL_RED,           GL_RED,           GL_RED,            GL_RED,            GL_RED,          GL_RED,          GL_RED,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_STENCIL };
	const int32_t GType[]            = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT,   GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_FLOAT,             GL_FLOAT,             GL_FLOAT,             GL_UNSIGNED_INT_24_8 };
	if (!UpdateTexture(Texture)) return false;
	if (Texels) {
		uint32_t PackType = Texture->GetPackType();
		glTexSubImage1D(GL_TEXTURE_1D, MipmapLevel, Position, Size, GFormats[PackType], GType[PackType], Texels);
	}
	return true;
}

bool LWVideoDriver_OpenGL3_3::UpdateTexture2D(LWTexture *Texture, uint32_t MipmapLevel, void *Texels, const LWVector2i &Position, const LWVector2i &Size){
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

bool LWVideoDriver_OpenGL3_3::UpdateTexture3D(LWTexture *Texture, uint32_t MipmapLevel, void *Texels, const LWVector3i &Position, const LWVector3i &Size){
	//PackTypes:           RGBA8,            RGBA8U,           RGBA16,            RGBA16,            RGBA32,          RGBA32U,         RGBA32F,    RG8,              RG8U,             RG16,              RG16U,             RG32,            RG32U,           RG32F,    R8,               R8U,              R16,               R16U,              R32,             R32U,            R32F,     Depth16,              Depth24,              Depth32,              Depth24Stencil8
	const int32_t GInternalFormats[] = { GL_RGBA8,         GL_RGBA8,         GL_RGBA16,         GL_RGBA16,         GL_RGBA32I,      GL_RGBA32UI,     GL_RGBA32F, GL_RG8,           GL_RG8,           GL_RG16,           GL_RG16,           GL_RG32I,        GL_RG32UI,       GL_RG32F, GL_R8,            GL_R8,            GL_R16,            GL_R16,            GL_R32I,         GL_R32UI,        GL_R32F,  GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32, GL_DEPTH24_STENCIL8, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0,   GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT };
	const int32_t GFormats[]         = { GL_RGBA,          GL_RGBA,          GL_RGBA,           GL_RGBA,           GL_RGBA,         GL_RGBA,         GL_RGBA,    GL_RG,            GL_RG,            GL_RG,             GL_RG,             GL_RG,           GL_RG,           GL_RG,    GL_RED,           GL_RED,           GL_RED,            GL_RED,            GL_RED,          GL_RED,          GL_RED,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_STENCIL };
	const int32_t GType[]            = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT,   GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_FLOAT,             GL_FLOAT,             GL_FLOAT,             GL_UNSIGNED_INT_24_8 };
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

bool LWVideoDriver_OpenGL3_3::UpdateTextureCubeMap(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Face, void *Texels, const LWVector2i &Position, const LWVector2i &Size){
	//PackTypes:                   RGBA8,            RGBA8U,           RGBA16,            RGBA16,            RGBA32,          RGBA32U,         RGBA32F,    RG8,              RG8U,             RG16,              RG16U,             RG32,            RG32U,           RG32F,    R8,               R8U,              R16,               R16U,              R32,             R32U,            R32F,     Depth16,              Depth24,              Depth32,              Depth24Stencil8
	const int32_t GInternalFormats[] = { GL_RGBA8,         GL_RGBA8,         GL_RGBA16,         GL_RGBA16,         GL_RGBA32I,      GL_RGBA32UI,     GL_RGBA32F, GL_RG8,           GL_RG8,           GL_RG16,           GL_RG16,           GL_RG32I,        GL_RG32UI,       GL_RG32F, GL_R8,            GL_R8,            GL_R16,            GL_R16,            GL_R32I,         GL_R32UI,        GL_R32F,  GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32, GL_DEPTH24_STENCIL8, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0,   GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT };
	const int32_t GFormats[]         = { GL_RGBA,          GL_RGBA,          GL_RGBA,           GL_RGBA,           GL_RGBA,         GL_RGBA,         GL_RGBA,    GL_RG,            GL_RG,            GL_RG,             GL_RG,             GL_RG,           GL_RG,           GL_RG,    GL_RED,           GL_RED,           GL_RED,            GL_RED,            GL_RED,          GL_RED,          GL_RED,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_STENCIL };
	const int32_t GType[]            = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT,   GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_FLOAT,             GL_FLOAT,             GL_FLOAT,             GL_UNSIGNED_INT_24_8 };
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

bool LWVideoDriver_OpenGL3_3::UpdateTexture1DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, void *Texels, uint32_t Position, uint32_t Size) {
	return false;
}

bool LWVideoDriver_OpenGL3_3::UpdateTexture2DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, void *Texels, const LWVector2i &Position, const LWVector2i &Size) {
	return false;
}

bool LWVideoDriver_OpenGL3_3::UpdateTextureCubeArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint32_t Face, void *Texels, const LWVector2i &Position, const LWVector2i &Size) {
	return false;
}

bool LWVideoDriver_OpenGL3_3::UpdateVideoBuffer(LWVideoBuffer *VideoBuffer, const uint8_t *Buffer, uint32_t Length){
	LWOpenGL3_3Buffer *VB = (LWOpenGL3_3Buffer*)VideoBuffer;
	int32_t GTypes[] = { GL_ARRAY_BUFFER, GL_UNIFORM_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_TEXTURE_BUFFER };
	int32_t Type = GTypes[VB->GetType()];
	glBindBuffer(Type, VB->GetContext());
	uint8_t *B = (uint8_t*)glMapBuffer(Type, GL_WRITE_ONLY);
	if (!B) return false;
	std::copy(Buffer, Buffer + Length, B);
	glUnmapBuffer(Type);
	return true;
}

bool LWVideoDriver_OpenGL3_3::DownloadTexture1D(LWTexture *Texture, uint32_t MipmapLevel, uint8_t *Buffer){
	const int32_t GInternalFormats[] = { GL_RGBA8,         GL_RGBA8,         GL_RGBA16,         GL_RGBA16,         GL_RGBA32I,      GL_RGBA32UI,     GL_RGBA32F, GL_RG8,           GL_RG8,           GL_RG16,           GL_RG16,           GL_RG32I,        GL_RG32UI,       GL_RG32F, GL_R8,            GL_R8,            GL_R16,            GL_R16,            GL_R32I,         GL_R32UI,        GL_R32F,  0,       0,       0,       0,               0,    0,    0,    0,    0,    0,    0 };
	const int32_t GFormats[]         = { GL_RGBA,          GL_RGBA,          GL_RGBA,           GL_RGBA,           GL_RGBA,         GL_RGBA,         GL_RGBA,    GL_RG,            GL_RG,            GL_RG,             GL_RG,             GL_RG,           GL_RG,           GL_RG,    GL_RED,           GL_RED,           GL_RED,            GL_RED,            GL_RED,          GL_RED,          GL_RED,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_STENCIL };
	const int32_t GType[]            = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT,   GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_FLOAT,             GL_FLOAT,             GL_FLOAT,             GL_UNSIGNED_INT_24_8 };
	if (!UpdateTexture(Texture)) return false;
	uint32_t PackType = Texture->GetPackType();
	glGetTexImage(GL_TEXTURE_1D, MipmapLevel, GFormats[PackType], GType[PackType], Buffer);
	return true;
}

bool LWVideoDriver_OpenGL3_3::DownloadTexture2D(LWTexture *Texture, uint32_t MipmapLevel, uint8_t *Buffer){
	const int32_t GInternalFormats[] = { GL_RGBA8,         GL_RGBA8,         GL_RGBA16,         GL_RGBA16,         GL_RGBA32I,      GL_RGBA32UI,     GL_RGBA32F, GL_RG8,           GL_RG8,           GL_RG16,           GL_RG16,           GL_RG32I,        GL_RG32UI,       GL_RG32F, GL_R8,            GL_R8,            GL_R16,            GL_R16,            GL_R32I,         GL_R32UI,        GL_R32F,  GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32, GL_DEPTH24_STENCIL8, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0,   GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT };
	const int32_t GFormats[]         = { GL_RGBA,          GL_RGBA,          GL_RGBA,           GL_RGBA,           GL_RGBA,         GL_RGBA,         GL_RGBA,    GL_RG,            GL_RG,            GL_RG,             GL_RG,             GL_RG,           GL_RG,           GL_RG,    GL_RED,           GL_RED,           GL_RED,            GL_RED,            GL_RED,          GL_RED,          GL_RED,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_STENCIL };
	const int32_t GType[]            = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT,   GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_FLOAT,             GL_FLOAT,             GL_FLOAT,             GL_UNSIGNED_INT_24_8 };
	if (!UpdateTexture(Texture)) return false;
	uint32_t PackType = Texture->GetPackType();
	if (LWImage::CompressedType(PackType)) {
		glGetCompressedTexImage(GL_TEXTURE_2D, MipmapLevel, Buffer);
	} else {
		glGetTexImage(GL_TEXTURE_2D, MipmapLevel, GFormats[PackType], GType[PackType], Buffer);
	}
	return true;
}

bool LWVideoDriver_OpenGL3_3::DownloadTexture3D(LWTexture *Texture, uint32_t MipmapLevel, uint8_t *Buffer){
	const int32_t GInternalFormats[] = { GL_RGBA8,         GL_RGBA8,         GL_RGBA16,         GL_RGBA16,         GL_RGBA32I,      GL_RGBA32UI,     GL_RGBA32F, GL_RG8,           GL_RG8,           GL_RG16,           GL_RG16,           GL_RG32I,        GL_RG32UI,       GL_RG32F, GL_R8,            GL_R8,            GL_R16,            GL_R16,            GL_R32I,         GL_R32UI,        GL_R32F,  GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32, GL_DEPTH24_STENCIL8, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0,   GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT };
	const int32_t GFormats[]         = { GL_RGBA,          GL_RGBA,          GL_RGBA,           GL_RGBA,           GL_RGBA,         GL_RGBA,         GL_RGBA,    GL_RG,            GL_RG,            GL_RG,             GL_RG,             GL_RG,           GL_RG,           GL_RG,    GL_RED,           GL_RED,           GL_RED,            GL_RED,            GL_RED,          GL_RED,          GL_RED,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_STENCIL };
	const int32_t GType[]            = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT,   GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_FLOAT,             GL_FLOAT,             GL_FLOAT,             GL_UNSIGNED_INT_24_8 };
	if (!UpdateTexture(Texture)) return false;
	uint32_t PackType = Texture->GetPackType();
	if (LWImage::CompressedType(PackType)) {
		glGetCompressedTexImage(GL_TEXTURE_3D, MipmapLevel, Buffer);
	} else {
		glGetTexImage(GL_TEXTURE_3D, MipmapLevel, GFormats[PackType], GType[PackType], Buffer);
	}
	return true;
}

bool LWVideoDriver_OpenGL3_3::DownloadTextureCubeMap(LWTexture *Texture, uint32_t Face, uint32_t MipmapLevel, uint8_t *Buffer){
	const int32_t GInternalFormats[] = { GL_RGBA8,         GL_RGBA8,         GL_RGBA16,         GL_RGBA16,         GL_RGBA32I,      GL_RGBA32UI,     GL_RGBA32F, GL_RG8,           GL_RG8,           GL_RG16,           GL_RG16,           GL_RG32I,        GL_RG32UI,       GL_RG32F, GL_R8,            GL_R8,            GL_R16,            GL_R16,            GL_R32I,         GL_R32UI,        GL_R32F,  GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32, GL_DEPTH24_STENCIL8, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0,   GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT };
	const int32_t GFormats[]         = { GL_RGBA,          GL_RGBA,          GL_RGBA,           GL_RGBA,           GL_RGBA,         GL_RGBA,         GL_RGBA,    GL_RG,            GL_RG,            GL_RG,             GL_RG,             GL_RG,           GL_RG,           GL_RG,    GL_RED,           GL_RED,           GL_RED,            GL_RED,            GL_RED,          GL_RED,          GL_RED,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_STENCIL };
	const int32_t GType[]            = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT,   GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_FLOAT,             GL_FLOAT,             GL_FLOAT,             GL_UNSIGNED_INT_24_8 };
	if (!UpdateTexture(Texture)) return false;
	uint32_t PackType = Texture->GetPackType();
	if (LWImage::CompressedType(PackType)) {
		glGetCompressedTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X+Face, MipmapLevel, Buffer);
	} else {
		glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + Face, MipmapLevel, GFormats[PackType], GType[PackType], Buffer);
	}
	return true;
}

bool LWVideoDriver_OpenGL3_3::DownloadTexture1DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint8_t *Buffer) {
	return false;
}

bool LWVideoDriver_OpenGL3_3::DownloadTexture2DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint8_t *Buffer) {
	return false;
}

bool LWVideoDriver_OpenGL3_3::DownloadTextureCubeArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint32_t Face, uint8_t *Buffer) {
	return false;
}

bool LWVideoDriver_OpenGL3_3::DownloadVideoBuffer(LWVideoBuffer *VBuffer, uint8_t *Buffer, uint32_t Offset, uint32_t Length){
	LWOpenGL3_3Buffer *VB = (LWOpenGL3_3Buffer*)VBuffer;
	int32_t GTypes[] = { GL_ARRAY_BUFFER, GL_UNIFORM_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER };
	glBindBuffer(GTypes[VB->GetType()], VB->GetContext());
	glGetBufferSubData(GTypes[VB->GetType()], Offset, Length, Buffer);
	return true;
}

LWVideoDriver &LWVideoDriver_OpenGL3_3::DestroyPipeline(LWPipeline *Pipeline) {
	LWOpenGL3_3Pipeline *P = (LWOpenGL3_3Pipeline*)Pipeline;
	if (P->isInternalPipeline()) {
		LWOpenGL3_3PipelineContext &Con = P->GetContext();
		glDeleteProgram(Con.m_ProgramID);
		glDeleteVertexArrays(1, &Con.m_VAOID);
	}
	LWAllocator::Destroy(P);
	return *this;
}

LWVideoDriver &LWVideoDriver_OpenGL3_3::DestroyVideoBuffer(LWVideoBuffer *Buffer){
	LWOpenGL3_3Buffer *VB = (LWOpenGL3_3Buffer*)Buffer;
	uint32_t VideoID = VB->GetContext();
	glDeleteBuffers(1, &VideoID);
	LWAllocator::Destroy(VB);
	return *this;
}

LWVideoDriver &LWVideoDriver_OpenGL3_3::DestroyShader(LWShader *Shader){
	LWOpenGL3_3Shader *S = (LWOpenGL3_3Shader*)Shader;
	uint32_t VideoID = S->GetContext();
	glDeleteShader(S->GetContext());
	LWAllocator::Destroy(S);
	return *this;
}

LWVideoDriver &LWVideoDriver_OpenGL3_3::DestroyTexture(LWTexture *Texture){
	LWOpenGL3_3Texture *T = (LWOpenGL3_3Texture*)Texture;
	uint32_t VideoID = T->GetContext();
	glDeleteTextures(1, &VideoID);
	LWAllocator::Destroy(T);
	return *this;
}

LWVideoDriver &LWVideoDriver_OpenGL3_3::DestroyFrameBuffer(LWFrameBuffer *FrameBuffer) {
	LWOpenGL3_3FrameBuffer *FB = (LWOpenGL3_3FrameBuffer*)FrameBuffer;
	auto &Con = FB->GetContext();
	glDeleteFramebuffers(1, &Con.m_FBOID);
	LWAllocator::Destroy(FB);
	return *this;
}

LWVideoDriver &LWVideoDriver_OpenGL3_3::ClearColor(uint32_t Color) {
	SetFrameBuffer(m_ActiveFrameBuffer);
	LWVector4f ClearClr = LWUNPACK_COLORVEC4f(Color);
	for (uint32_t i = 0; i < m_ActiveDrawCount; i++) {
		glClearBufferfv(GL_COLOR, i, &ClearClr.x);
	}
	/*
	if (m_ActiveFrameBuffer) {
		for(uint32_t i=0;i<LWFrameBuffer::ColorCount;i++){
			LWFrameBufferAttachment &A = m_ActiveFrameBuffer->GetAttachment(i);
			if(!A.m_Source) continue;
			glClearBufferfv(
	} else {
		glClearColor(ClearClr.x, ClearClr.y, ClearClr.z, ClearClr.w);
		glClear(GL_COLOR_BUFFER_BIT);
	}*/
	return *this;
}

LWVideoDriver &LWVideoDriver_OpenGL3_3::ClearColor(const LWVector4f &Color) {
	SetFrameBuffer(m_ActiveFrameBuffer);
	for (uint32_t i = 0; i < m_ActiveDrawCount; i++) {
		glClearBufferfv(GL_COLOR, i, &Color.x);
	}
	//glClearColor(Color.x, Color.y, Color.z, Color.w);
	//glClear(GL_COLOR_BUFFER_BIT);
	return *this;
}

LWVideoDriver &LWVideoDriver_OpenGL3_3::ClearDepth(float Depth) {
	//SetFrameBuffer(m_ActiveFrameBuffer);
	//glClearBufferfi(GL_DEPTH, 0, Depth, 0);
	glClearDepth(Depth);
	glClear(GL_DEPTH_BUFFER_BIT);
	return *this;
}

LWVideoDriver &LWVideoDriver_OpenGL3_3::ClearStencil(uint8_t Stencil) {
	//SetFrameBuffer(m_ActiveFrameBuffer);
	//glClearBufferfi(GL_STENCIL, 0, 0.0f, (int32_t)Stencil);
	glClearStencil((int32_t)Stencil);
	glClear(GL_STENCIL_BUFFER_BIT);
	return *this;
}

bool LWVideoDriver_OpenGL3_3::SetRasterState(uint64_t Flags, float Bias, float SlopedScaleBias){
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

bool LWVideoDriver_OpenGL3_3::SetPipeline(LWPipeline *Pipeline, LWVideoBuffer *VertexBuffer, LWVideoBuffer *IndiceBuffer, uint32_t VertexStride, uint32_t Offset) {
	//Video buffer types.
	const int32_t GBTypes[] = { GL_ARRAY_BUFFER, GL_UNIFORM_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_SHADER_STORAGE_BUFFER };
	const int32_t GIBaseType[] = { GL_FLOAT, GL_INT, GL_UNSIGNED_INT, GL_DOUBLE, GL_FLOAT, GL_FLOAT, GL_FLOAT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_INT, GL_INT, GL_INT, GL_DOUBLE, GL_DOUBLE, GL_DOUBLE, GL_FLOAT, GL_FLOAT, GL_FLOAT, GL_DOUBLE, GL_DOUBLE, GL_DOUBLE };
	const int32_t GIComponentCnt[] = { 1,    1,      1,               1,         2,        3,        4,        2,               3,               4,               2,      3,      4,      2,         3,         4,         4,        9,        16,       4,         9,         16 };

	bool Update = LWVideoDriver::SetPipeline(Pipeline, VertexBuffer, IndiceBuffer, VertexStride, Offset);
	LWOpenGL3_3Pipeline *P = (LWOpenGL3_3Pipeline*)Pipeline;
	auto &Context = P->GetContext();
	uint32_t BlockCount = Pipeline->GetBlockCount();
	uint32_t ResourceCount = Pipeline->GetResourceCount();
	if (Update) glUseProgram(Context.m_ProgramID);
	for (uint32_t i = 0; i < BlockCount; i++) {
		LWShaderResource &R = Pipeline->GetBlock(i);
		LWOpenGL3_3Buffer *B = (LWOpenGL3_3Buffer*)R.m_Resource;
		if (!B) continue;
		uint32_t VideoID = B->GetContext();
		LWVideoDriver::UpdateVideoBuffer(B);
		if(!Update) continue;
		uint32_t Offset = R.m_Offset*m_UniformBlockSize;
		glBindBufferRange(GBTypes[B->GetType()], R.m_StageBindings, VideoID, Offset, B->GetRawLength() - Offset);
	}

	for (uint32_t i = 0; i < ResourceCount; i++) {
		//Image Formats:      RGBA8,    RGBA8U,      RGBA16,     RGBA16U,     RGBA32,     RGBA32U,     RGBA32F,    RG8,     RG8U,     RG16,     RG16U,     RG32,     RG32U,     RG32F,    R8,     R8U,     R16,     R16U,     R32,     R32U,     R32F,    D16,     D24, D32,     D24S8, DXT1, DXT2, DXT3, DXT4, DXT5, DXT6, DXT7
		GLenum IFormats[] = { GL_RGBA8I, GL_RGBA8UI, GL_RGBA16I, GL_RGBA16UI, GL_RGBA32I, GL_RGBA32UI, GL_RGBA32F, GL_RG8I, GL_RG8UI, GL_RG16I, GL_RG16UI, GL_RG32I, GL_RG32UI, GL_RG32F, GL_R8I, GL_R8UI, GL_R16I, GL_R16UI, GL_R32I, GL_R32UI, GL_R32F, GL_R16F, 0,   GL_R32F, 0,     0,    0,    0,    0,    0,    0,    0 };
		LWShaderResource &R = Pipeline->GetResource(i);
		LWOpenGL3_3Texture *T = (LWOpenGL3_3Texture *)R.m_Resource;
		LWOpenGL3_3Buffer *B = (LWOpenGL3_3Buffer *)R.m_Resource;
		uint32_t TypeID = R.GetTypeID();
		if (TypeID == LWPipeline::Texture) {
			if (!T) continue;
			glActiveTexture(GL_TEXTURE0 + R.m_StageBindings);
			UpdateTexture(T);
		} else if (TypeID == LWPipeline::ImageBuffer) {
			if (!B) continue;
			LWVideoDriver::UpdateVideoBuffer(B);
			if (Update) glBindBufferBase(GBTypes[B->GetType()], R.m_StageBindings, B->GetContext());
		}
	}
	if (VertexBuffer) {
		LWOpenGL3_3Buffer *VBuffer = (LWOpenGL3_3Buffer*)VertexBuffer;
		LWVideoDriver::UpdateVideoBuffer(VertexBuffer);
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
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ((LWOpenGL3_3Buffer*)IndiceBuffer)->GetContext());
	}
	return Update;
}

bool LWVideoDriver_OpenGL3_3::SetFrameBuffer(LWFrameBuffer *Buffer,bool ChangeViewport) {
	if (!LWVideoDriver::SetFrameBuffer(Buffer, ChangeViewport)) return false;
	if (!Buffer) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		m_ActiveDrawCount = 1;
	} else {
		auto &Context = ((LWOpenGL3_3FrameBuffer*)Buffer)->GetContext();
		GLenum AttachmentPnts[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, GL_DEPTH_STENCIL_ATTACHMENT };
		const GLenum GFaces[] = { GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, GL_TEXTURE_CUBE_MAP_POSITIVE_Y,GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, GL_TEXTURE_CUBE_MAP_POSITIVE_Z };

		GLenum DrawBuffers[LWFrameBuffer::Count];
		m_ActiveDrawCount = 0;
		glBindFramebuffer(GL_FRAMEBUFFER, Context.m_FBOID);
		for (uint32_t i = 0; i < LWFrameBuffer::Count; i++) {
			auto &Slot = Buffer->GetAttachment(i);
			if (!Slot.m_Source) {
				if (Context.m_Attached[i] != 0) glFramebufferRenderbuffer(GL_FRAMEBUFFER, Context.m_Attached[i], GL_RENDERBUFFER, 0);
				Context.m_Attached[i] = 0;
				continue;
			}
			auto Tex = (LWOpenGL3_3Texture*)Slot.m_Source;
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
			}
			if (i < LWFrameBuffer::ColorCount) DrawBuffers[m_ActiveDrawCount++] = AttachmentPnts[i];
		}
		glDrawBuffers(m_ActiveDrawCount, DrawBuffers);
	}
	return true;
}

LWVideoDriver &LWVideoDriver_OpenGL3_3::DrawBuffer(LWPipeline *Pipeline, int32_t DrawMode, LWVideoBuffer *InputBlock, LWVideoBuffer *IndexBuffer, uint32_t Count, uint32_t VertexStride, uint32_t Offset){
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

LWVideoDriver &LWVideoDriver_OpenGL3_3::DrawInstancedBuffer(LWPipeline *Pipeline, int32_t DrawMode, LWVideoBuffer *InputBlock, LWVideoBuffer *IndexBuffer, uint32_t Count, uint32_t VertexStride, uint32_t InstanceCount /* = 0 */, uint32_t Offset /* = 0 */) {
	const int32_t GModes[] = { GL_POINTS, GL_LINE_STRIP, GL_LINES, GL_TRIANGLE_STRIP, GL_TRIANGLES };
	SetFrameBuffer(m_ActiveFrameBuffer);
	SetPipeline(Pipeline, InputBlock, InputBlock, VertexStride, Offset);

	if (IndexBuffer) {
		uint32_t OffsetSize = IndexBuffer->GetType() == LWVideoBuffer::Index16 ? sizeof(uint16_t) : sizeof(uint32_t);
		GLenum IndexType = IndexBuffer->GetType() == LWVideoBuffer::Index16 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
		glDrawElementsInstanced(GModes[DrawMode], Count, IndexType, (void*)(uintptr_t)(Offset*OffsetSize), InstanceCount);
	} else {
		glDrawArraysInstanced(GModes[DrawMode], Offset, Count, InstanceCount);
	}
	return *this;
}


LWVideoDriver &LWVideoDriver_OpenGL3_3::Dispatch(LWPipeline *Pipeline, const LWVector3i &GroupDimension) {
	return *this;
}

LWOpenGL3_3Context &LWVideoDriver_OpenGL3_3::GetContext(void) {
	return m_Context;
}

LWVideoDriver_OpenGL3_3::LWVideoDriver_OpenGL3_3(LWWindow *Window, LWOpenGL3_3Context &Context, uint32_t UniformBlockSize) : LWVideoDriver(Window, OpenGL3_3, UniformBlockSize) {
	m_Context = Context;
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}