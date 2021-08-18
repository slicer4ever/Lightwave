#include "LWVideo/LWVideoDrivers/LWVideoDriver_OpenGL2_1.h"
#include "LWPlatform/LWPlatform.h"
#include "LWPlatform/LWWindow.h"
#include "LWCore/LWVector.h"
#include "LWCore/LWMath.h"
#include "LWVideo/LWImage.h"
#include "LWVideo/LWFrameBuffer.h"
#include "LWVideo/LWPipeline.h"
#include "LWVideo/LWVideoBuffer.h"
#include <iostream>

LWVideoDriver &LWVideoDriver_OpenGL2_1::ViewPort(const LWVector4i &Viewport) {
	m_Viewport = Viewport;
	glViewport(Viewport.x, Viewport.y, Viewport.z, Viewport.w);
	return *this;
}

LWShader *LWVideoDriver_OpenGL2_1::CreateShader(uint32_t ShaderType, const LWUTF8Iterator &Source, LWAllocator &Allocator, char *CompiledBuffer, char8_t *ErrorBuffer, uint32_t &CompiledBufferLen, uint32_t ErrorBufferLen) {
	CompiledBufferLen = 0;
	return CreateShaderCompiled(ShaderType, (const char*)Source(), Source.RawDistance(Source.NextEnd()), Allocator, ErrorBuffer, ErrorBufferLen);
}

LWShader *LWVideoDriver_OpenGL2_1::CreateShaderCompiled(uint32_t ShaderType, const char *CompiledCode, uint32_t CompiledCodeLen, LWAllocator &Allocator, char8_t *ErrorBuffer, uint32_t ErrorBufferLen) {
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
	return Allocator.Create<LWOpenGL2_1Shader>(ShaderID, LWCrypto::HashFNV1A((const uint8_t*)CompiledCode, CompiledCodeLen), ShaderType);
}

LWTexture *LWVideoDriver_OpenGL2_1::CreateTexture1D(uint32_t TextureState, uint32_t PackType, uint32_t Size, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator) {
	//PackTypes:                         SRGBA,            RGBA8,            RGBA8S,  RGBA16,            RGBA16S,  RGBA32,          RGBA32S, RGBA32F,    RG8,              RG8S,    RG16,              RG16S,    RG32,            RG32S,  RG32F,    R8,               R8S,          R16,               R16S,         R32,             R32S,   R32F,     Depth16,              Depth24,              Depth32,               Depth24Stencil8,    BC1,                              BC1_SRGB,                                BC2,                              BC2_SRGB,                                BC3,                              BC3_SRGB,                                BC4, BC5, BC6, DXT7,                              DXT7_SRGB
	const int32_t GInternalFormats[] = { GL_SRGB8_ALPHA8,  GL_RGBA,          GL_RGBA, GL_RGBA,           GL_RGBA,  0,               0,       0,          0,                0,       0,                 0,        0,               0,      0,        GL_LUMINANCE,     GL_LUMINANCE, GL_LUMINANCE,      GL_LUMINANCE, 0,               0,      0,        0,                    0,                    0,                     0,                  GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT,  GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT,  GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT,  0,   0,   0,   GL_COMPRESSED_RGBA_BPTC_UNORM_EXT, GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_EXT };
	const int32_t GFormats[]         = { GL_RGBA,          GL_RGBA,          GL_RGBA, GL_RGBA,           GL_RGBA,  GL_RGBA,         GL_RGBA, GL_RGBA,    GL_RG,            GL_RG,   GL_RG,             GL_RG,    GL_RG,           GL_RG,  GL_RG,    GL_RED,           GL_RED,       GL_RED,            GL_RED,       GL_RED,          GL_RED, GL_RED,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_STENCIL };
	const int32_t GType[]            = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT, GL_UNSIGNED_INT, GL_INT,  GL_FLOAT,   GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT, GL_UNSIGNED_INT, GL_INT, GL_FLOAT, GL_UNSIGNED_BYTE, GL_BYTE,      GL_UNSIGNED_SHORT, GL_SHORT,     GL_UNSIGNED_INT, GL_INT, GL_FLOAT, GL_FLOAT,             GL_FLOAT,             GL_FLOAT,             GL_UNSIGNED_INT_24_8 };
	if (GInternalFormats[PackType] == 0) return nullptr;

	uint32_t VideoID = 0;
	uint32_t Type = 0;	
	bool MakeMipmaps = (TextureState&LWTexture::MakeMipmaps) != 0;
	bool Compressed = LWImage::CompressedType(PackType);
	if (MakeMipmaps) MipmapCnt = 0;
	TextureState = (TextureState&~LWTexture::MakeMipmaps);

	glGenTextures(1, &VideoID);
	glBindTexture(GL_TEXTURE_1D, VideoID);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAX_LEVEL, MipmapCnt);
	glTexImage1D(GL_TEXTURE_1D, 0, GInternalFormats[PackType], Size, 0, GFormats[PackType], GType[PackType], Texels ? Texels[0] : nullptr);
	for (uint32_t i = 1; i <= MipmapCnt; i++) {
		uint32_t MipSize = LWImage::MipmapSize1D(Size, i);
		glTexImage1D(GL_TEXTURE_1D, i, GInternalFormats[PackType], MipSize, 0, GFormats[PackType], GType[PackType], Texels[i]);
	}
	return Allocator.Create<LWOpenGL2_1Texture>(VideoID, TextureState, PackType, MipmapCnt, LWVector3i(Size, 0, 0), LWTexture::Texture1D);
}

LWTexture *LWVideoDriver_OpenGL2_1::CreateTexture2D(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator) {
	//PackTypes:                         SRGBA,            RGBA8,            RGBA8S,  RGBA16,            RGBA16S,  RGBA32,          RGBA32S, RGBA32F,    RG8,              RG8S,    RG16,              RG16S,    RG32,            RG32S,  RG32F,    R8,               R8S,          R16,               R16S,         R32,             R32S,   R32F,     Depth16,              Depth24,              Depth32,               Depth24Stencil8,    BC1,                              BC1_SRGB,                                BC2,                              BC2_SRGB,                                BC3,                              BC3_SRGB,                                BC4, BC5, BC6, DXT7,                              DXT7_SRGB
	const int32_t GInternalFormats[] = { GL_SRGB8_ALPHA8,  GL_RGBA,          GL_RGBA, GL_RGBA,           GL_RGBA,  0,               0,       0,          0,                0,       0,                 0,        0,               0,      0,        GL_LUMINANCE,     GL_LUMINANCE, GL_LUMINANCE,      GL_LUMINANCE, 0,               0,      0,        0,                    0,                    0,                     0,                  GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT,  GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT,  GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT,  0,   0,   0,   GL_COMPRESSED_RGBA_BPTC_UNORM_EXT, GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_EXT };
	const int32_t GFormats[]         = { GL_RGBA,          GL_RGBA,          GL_RGBA, GL_RGBA,           GL_RGBA,  GL_RGBA,         GL_RGBA, GL_RGBA,    GL_RG,            GL_RG,   GL_RG,             GL_RG,    GL_RG,           GL_RG,  GL_RG,    GL_RED,           GL_RED,       GL_RED,            GL_RED,       GL_RED,          GL_RED, GL_RED,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_STENCIL };
	const int32_t GType[]            = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT, GL_UNSIGNED_INT, GL_INT,  GL_FLOAT,   GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT, GL_UNSIGNED_INT, GL_INT, GL_FLOAT, GL_UNSIGNED_BYTE, GL_BYTE,      GL_UNSIGNED_SHORT, GL_SHORT,     GL_UNSIGNED_INT, GL_INT, GL_FLOAT, GL_FLOAT,             GL_FLOAT,             GL_FLOAT,             GL_UNSIGNED_INT_24_8 };
	if (GInternalFormats[PackType] == 0) return nullptr; //Unsupported format!

	uint32_t VideoID = 0;
	uint32_t Type = 0;
	if (TextureState&LWTexture::RenderBuffer) {
		glGenRenderbuffers(1, &VideoID);
		glBindRenderbuffer(GL_RENDERBUFFER, VideoID);
		glRenderbufferStorage(GL_RENDERBUFFER, GInternalFormats[PackType], Size.x, Size.y);
		return Allocator.Create<LWOpenGL2_1Texture>(VideoID, TextureState&~LWTexture::MakeMipmaps, PackType, 0, LWVector3i(Size, 0), LWTexture::Texture2D);
	}
	bool MakeMipmaps = (TextureState&LWTexture::MakeMipmaps) != 0;
	bool Compressed = LWImage::CompressedType(PackType);
	if (MakeMipmaps) MipmapCnt = 0;
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
	return Allocator.Create<LWOpenGL2_1Texture>(VideoID, TextureState, PackType, MipmapCnt, LWVector3i(Size, 0), LWTexture::Texture2D);
}

LWTexture *LWVideoDriver_OpenGL2_1::CreateTexture3D(uint32_t TextureState, uint32_t PackType, const LWVector3i &Size, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator) {
	//PackTypes:                         SRGBA,            RGBA8,            RGBA8S,  RGBA16,            RGBA16S,  RGBA32,          RGBA32S, RGBA32F,    RG8,              RG8S,    RG16,              RG16S,    RG32,            RG32S,  RG32F,    R8,               R8S,          R16,               R16S,         R32,             R32S,   R32F,     Depth16,              Depth24,              Depth32,               Depth24Stencil8,    BC1,                              BC1_SRGB,                                BC2,                              BC2_SRGB,                                BC3,                              BC3_SRGB,                                BC4, BC5, BC6, DXT7,                              DXT7_SRGB
	const int32_t GInternalFormats[] = { GL_SRGB8_ALPHA8,  GL_RGBA,          GL_RGBA, GL_RGBA,           GL_RGBA,  0,               0,       0,          0,                0,       0,                 0,        0,               0,      0,        GL_LUMINANCE,     GL_LUMINANCE, GL_LUMINANCE,      GL_LUMINANCE, 0,               0,      0,        0,                    0,                    0,                     0,                  GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT,  GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT,  GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT,  0,   0,   0,   GL_COMPRESSED_RGBA_BPTC_UNORM_EXT, GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_EXT };
	const int32_t GFormats[]         = { GL_RGBA,          GL_RGBA,          GL_RGBA, GL_RGBA,           GL_RGBA,  GL_RGBA,         GL_RGBA, GL_RGBA,    GL_RG,            GL_RG,   GL_RG,             GL_RG,    GL_RG,           GL_RG,  GL_RG,    GL_RED,           GL_RED,       GL_RED,            GL_RED,       GL_RED,          GL_RED, GL_RED,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_STENCIL };
	const int32_t GType[]            = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT, GL_UNSIGNED_INT, GL_INT,  GL_FLOAT,   GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT, GL_UNSIGNED_INT, GL_INT, GL_FLOAT, GL_UNSIGNED_BYTE, GL_BYTE,      GL_UNSIGNED_SHORT, GL_SHORT,     GL_UNSIGNED_INT, GL_INT, GL_FLOAT, GL_FLOAT,             GL_FLOAT,             GL_FLOAT,             GL_UNSIGNED_INT_24_8 };
	if (GInternalFormats[PackType] == 0) return nullptr;

	uint32_t VideoID = 0;
	uint32_t Type = 0;
	bool MakeMipmaps = (TextureState&LWTexture::MakeMipmaps) != 0;
	bool Compressed = LWImage::CompressedType(PackType);
	if (MakeMipmaps) MipmapCnt = 0;
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
	return Allocator.Create<LWOpenGL2_1Texture>(VideoID, TextureState, PackType, MipmapCnt, Size, LWTexture::Texture3D);
}

LWTexture *LWVideoDriver_OpenGL2_1::CreateTextureCubeMap(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator) {
	//PackTypes:                         SRGBA,            RGBA8,            RGBA8S,  RGBA16,            RGBA16S,  RGBA32,          RGBA32S, RGBA32F,    RG8,              RG8S,    RG16,              RG16S,    RG32,            RG32S,  RG32F,    R8,               R8S,          R16,               R16S,         R32,             R32S,   R32F,     Depth16,              Depth24,              Depth32,               Depth24Stencil8,    BC1,                              BC1_SRGB,                                BC2,                              BC2_SRGB,                                BC3,                              BC3_SRGB,                                BC4, BC5, BC6, DXT7,                              DXT7_SRGB
	const int32_t GInternalFormats[] = { GL_SRGB8_ALPHA8,  GL_RGBA,          GL_RGBA, GL_RGBA,           GL_RGBA,  0,               0,       0,          0,                0,       0,                 0,        0,               0,      0,        GL_LUMINANCE,     GL_LUMINANCE, GL_LUMINANCE,      GL_LUMINANCE, 0,               0,      0,        0,                    0,                    0,                     0,                  GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT,  GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT,  GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT,  0,   0,   0,   GL_COMPRESSED_RGBA_BPTC_UNORM_EXT, GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_EXT };
	const int32_t GFormats[]         = { GL_RGBA,          GL_RGBA,          GL_RGBA, GL_RGBA,           GL_RGBA,  GL_RGBA,         GL_RGBA, GL_RGBA,    GL_RG,            GL_RG,   GL_RG,             GL_RG,    GL_RG,           GL_RG,  GL_RG,    GL_RED,           GL_RED,       GL_RED,            GL_RED,       GL_RED,          GL_RED, GL_RED,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_STENCIL };
	const int32_t GType[]            = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT, GL_UNSIGNED_INT, GL_INT,  GL_FLOAT,   GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT, GL_UNSIGNED_INT, GL_INT, GL_FLOAT, GL_UNSIGNED_BYTE, GL_BYTE,      GL_UNSIGNED_SHORT, GL_SHORT,     GL_UNSIGNED_INT, GL_INT, GL_FLOAT, GL_FLOAT,             GL_FLOAT,             GL_FLOAT,             GL_UNSIGNED_INT_24_8 };
	if (GInternalFormats[PackType] == 0) return nullptr;

	uint32_t VideoID = 0;
	uint32_t Type = 0;
	bool MakeMipmaps = (TextureState&LWTexture::MakeMipmaps) != 0;
	bool Compressed = LWImage::CompressedType(PackType);
	if (MakeMipmaps) MipmapCnt = 0;
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
	return Allocator.Create<LWOpenGL2_1Texture>(VideoID, TextureState, PackType, MipmapCnt, LWVector3i(Size, 0), LWTexture::TextureCubeMap);
}

LWTexture *LWVideoDriver_OpenGL2_1::CreateTexture2DMS(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint32_t Samples, LWAllocator &Allocator) {
	return nullptr;
}

LWTexture *LWVideoDriver_OpenGL2_1::CreateTexture1DArray(uint32_t TextureState, uint32_t PackType, uint32_t Size, uint32_t Layers, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator) {
	return nullptr;
}

LWTexture *LWVideoDriver_OpenGL2_1::CreateTexture2DArray(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint32_t Layers, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator) {
	return nullptr;
}

LWTexture *LWVideoDriver_OpenGL2_1::CreateTextureCubeArray(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint32_t Layers, uint8_t **Texels, uint32_t MapmapCnt, LWAllocator &Allocator) {
	return nullptr;
}

LWTexture *LWVideoDriver_OpenGL2_1::CreateTexture2DMSArray(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint32_t Samples, uint32_t Layers, LWAllocator &Allocator) {
	return nullptr;
}

LWVideoBuffer *LWVideoDriver_OpenGL2_1::CreateVideoBuffer(uint32_t Type, uint32_t UsageFlag, uint32_t TypeSize, uint32_t Length, LWAllocator &Allocator, const uint8_t *Buffer) {
	LWOpenGL2_1VideoBufferContext Context;
	if (Type==LWVideoBuffer::Vertex || Type==LWVideoBuffer::Index16 || Type==LWVideoBuffer::Index32){
		int32_t GTypes[] = { GL_ARRAY_BUFFER, 0, GL_ELEMENT_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER };
		//                 Static, WriteDiscard, WriteNoOverlap, Readable
		int32_t GUsage = GL_DYNAMIC_DRAW;
		uint32_t UsageID = UsageFlag & LWVideoBuffer::UsageFlag;
		if (UsageID == LWVideoBuffer::Static) GUsage = GL_STATIC_DRAW;

		glGenBuffers(1, &Context.m_VideoID);
		glBindBuffer(GTypes[Type], Context.m_VideoID);
		glBufferData(GTypes[Type],TypeSize*Length, Buffer, GUsage);
		return Allocator.Create<LWOpenGL2_1Buffer>(Buffer, &Allocator, TypeSize, Length, UsageFlag | Type, Context);
	} else {
		Context.m_Buffer = Allocator.Allocate<uint8_t>(Length*TypeSize);
		if (Buffer) std::copy(Buffer, Buffer + (TypeSize*Length), Context.m_Buffer);
		return Allocator.Create<LWOpenGL2_1Buffer>(Buffer, &Allocator, TypeSize, Length, UsageFlag | LWVideoBuffer::LocalCopy | LWVideoBuffer::Dirty | Type, Context);
	}
}

LWPipeline *LWVideoDriver_OpenGL2_1::CreatePipeline(LWPipeline *Source, LWAllocator &Allocator) {
	char ErrorBuffer[1024];
	char NameBuffer[1024];
	if (Source->isComputePipeline()) return nullptr;
	LWShader *Stages[LWPipeline::StageCount] = { Source->GetShaderStage(0), Source->GetShaderStage(1), Source->GetShaderStage(2) };
	LWShaderResource Blocks[LWShader::MaxBlocks];
	LWShaderResource Resources[LWShader::MaxResources];
	LWShaderInput Inputs[LWShader::MaxInputs];
	LWOpenGL2_1PipelineContext Context;
	uint32_t ActiveProgramID = m_ActivePipeline ? ((LWOpenGL2_1Pipeline*)m_ActivePipeline)->GetContext().m_ProgramID : 0;
	int32_t BlockCount = 0;
	int32_t ResourceCount = 0;
	int32_t InputCount = 0;
	int32_t NameLen = 0;
	int32_t NextTex = 0;

	struct AttributeMap {
		uint32_t AttributeType; //The glVertexAttribPointer underlying type(GL_FLOAT, GL_INT, etc)
		uint32_t RowMultiplier; //Multiples the input count for matrix attribute types based on number of matrix rows.
	};

	auto MapAttributeType = [](uint32_t Type)->AttributeMap {
		if (Type == GL_FLOAT) return { LWShaderInput::Float, 1 };
		else if (Type == GL_FLOAT_VEC2) return { LWShaderInput::Vec2, 1 };
		else if (Type == GL_FLOAT_VEC3) return { LWShaderInput::Vec3, 1 };
		else if (Type == GL_FLOAT_VEC4) return { LWShaderInput::Vec4, 1 };
		else if (Type == GL_FLOAT_MAT2) return { LWShaderInput::Vec2, 2 };
		else if (Type == GL_FLOAT_MAT3) return { LWShaderInput::Vec3, 3 };
		else if (Type == GL_FLOAT_MAT4) return { LWShaderInput::Vec4, 4 };
		else if (Type == GL_FLOAT_MAT2x3) return { LWShaderInput::Vec2, 3 };
		else if (Type == GL_FLOAT_MAT2x4) return { LWShaderInput::Vec2, 4 };
		else if (Type == GL_FLOAT_MAT3x2) return { LWShaderInput::Vec3, 2 };
		else if (Type == GL_FLOAT_MAT3x4) return { LWShaderInput::Vec3, 4 };
		else if (Type == GL_FLOAT_MAT4x2) return { LWShaderInput::Vec4, 2 };
		else if (Type == GL_FLOAT_MAT4x3) return { LWShaderInput::Vec4, 3 };
		return { LWShaderInput::Float, 1 };
	};

	auto GetUniformSize = [](uint32_t Type)->uint32_t {
		if (Type == GL_FLOAT) return 4;
		else if (Type == GL_FLOAT_VEC2) return 8;
		else if (Type == GL_FLOAT_VEC3) return 12;
		else if (Type == GL_FLOAT_VEC4) return 16;
		else if (Type == GL_INT) return 4;
		else if (Type == GL_INT_VEC2) return 8;
		else if (Type == GL_INT_VEC3) return 12;
		else if (Type == GL_INT_VEC4) return 16;
		else if (Type == GL_BOOL) return 4;
		else if (Type == GL_BOOL_VEC2) return 8;
		else if (Type == GL_BOOL_VEC3) return 12;
		else if (Type == GL_BOOL_VEC4) return 16;
		else if (Type == GL_FLOAT_MAT2) return 16;
		else if (Type == GL_FLOAT_MAT3) return 36;
		else if (Type == GL_FLOAT_MAT4) return 64;
		else if (Type == GL_FLOAT_MAT2x3) return 24;
		else if (Type == GL_FLOAT_MAT2x4) return 32;
		else if (Type == GL_FLOAT_MAT3x2) return 32;
		else if (Type == GL_FLOAT_MAT3x4) return 48;
		else if (Type == GL_FLOAT_MAT4x2) return 32;
		else if (Type == GL_FLOAT_MAT4x3) return 48;
		return 4;
	};

	auto ParseBlockAttribute = [&Blocks, &BlockCount, &Context, &GetUniformSize](const LWUTF8Iterator &NameIter, int32_t Type, int32_t Size, uint32_t Index)->bool {
		LWUTF8Iterator BName = LWUTF8Iterator(NameIter, NameIter.NextToken('.', false));
		if (BName.AtEnd()) return false;
		uint32_t NameHash = BName.Hash();
		int32_t i = 0;
		for (; i < BlockCount; i++) {
			if (Blocks[i].m_NameHash == NameHash) break;
		}
		LWOpenGL2_1Block &B = Context.m_Blocks[i];
		if (i >= BlockCount) {
			Blocks[i].m_NameHash = NameHash;
			Blocks[i].m_StageBindings = i;
			B.m_UniformCount = 0;
			B.m_Size = 0;
			BlockCount++;
		}
		LWOpenGL2_1BlockUniforms &U = B.m_Uniforms[B.m_UniformCount];
		U.m_Index = Index;
		uint32_t PackedSize = GetUniformSize(Type);
		int32_t RawSize = PackedSize*Size;
		int32_t PadRemaining = ((B.m_Size / 16) + 1) * 16 - B.m_Size;
		if (RawSize > PadRemaining && PadRemaining<16) B.m_Size += PadRemaining;
		U.m_Offset = B.m_Size;
		U.m_Size = Size;
		U.m_Type = Type;
		B.m_Size += RawSize;
		B.m_UniformCount++;
		return true;
	};

	Context.m_ProgramID = glCreateProgram();
	auto VS = (LWOpenGL2_1Shader*)Stages[LWPipeline::Vertex];
	auto GS = (LWOpenGL2_1Shader*)Stages[LWPipeline::Geometry];
	auto PS = (LWOpenGL2_1Shader*)Stages[LWPipeline::Pixel];
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
		fmt::print("Error in pipeline: {}\n", ErrorBuffer);
		return nullptr;
	}
	glUseProgram(Context.m_ProgramID);
	int32_t UniformCount = 0;
	int32_t AttributeCount = 0;
	glGetProgramiv(Context.m_ProgramID, GL_ACTIVE_ATTRIBUTES, &AttributeCount);
	glGetProgramiv(Context.m_ProgramID, GL_ACTIVE_UNIFORMS, &UniformCount);
	for (int32_t i = 0; i < AttributeCount; i++) {
		uint32_t Type = 0;
		int32_t Length = 0;
		glGetActiveAttrib(Context.m_ProgramID, i, sizeof(NameBuffer), &NameLen, &Length, &Type, NameBuffer);
		LWUTF8I Name = LWUTF8I(NameBuffer);
		if (Name.Compare("gl_", 3)) continue; //Built-in types are skipped.
		AttributeMap Attr = MapAttributeType(Type);
		LWShaderInput &In = Inputs[InputCount];
		LWUTF8I CleanName;
		Name.SplitTokenList(&CleanName, 1, "["); //Clean's up the name to remove any bracket identifier's.
		In = LWShaderInput(CleanName, Attr.AttributeType, Attr.RowMultiplier * Length);
		In.SetBindIndex(glGetAttribLocation(Context.m_ProgramID, NameBuffer));
		InputCount++;
	}
	for (int32_t i = 0; i < UniformCount; i++) {
		LWShaderResource &R = Resources[ResourceCount];
		uint32_t Type = 0;
		int32_t Length = 0;
		glGetActiveUniform(Context.m_ProgramID, i, sizeof(NameBuffer), &NameLen, &Length, &Type, NameBuffer);
		LWUTF8Iterator Name = LWUTF8Iterator(NameBuffer);
		uint32_t NameHash = Name.Hash();
		if (Type == GL_SAMPLER_1D || Type == GL_SAMPLER_2D || Type == GL_SAMPLER_3D ||
			Type == GL_SAMPLER_CUBE || Type == GL_SAMPLER_1D_SHADOW || Type == GL_SAMPLER_2D_SHADOW || Type == GL_SAMPLER_CUBE_SHADOW) {
			R = LWShaderResource(NameHash, LWPipeline::Texture, Length, NextTex);
			glUniform1i(i, NextTex);
			ResourceCount++;
			NextTex++;
		} else if(!ParseBlockAttribute(NameBuffer, Type, Length, glGetUniformLocation(Context.m_ProgramID, NameBuffer))){
			fmt::print("Error uniform not accounted for(likely declared outside struct): '{}'\n", NameBuffer);
		}
	}
	glUseProgram(ActiveProgramID);
	return Allocator.Create<LWOpenGL2_1Pipeline>(Context, Stages, Blocks, Resources, Inputs, BlockCount, ResourceCount, InputCount, LWPipeline::InternalPipeline);
}

LWVideoDriver &LWVideoDriver_OpenGL2_1::ClonePipeline(LWPipeline *Target, LWPipeline *Source) {
	LWVideoDriver::ClonePipeline(Target, Source);
	LWOpenGL2_1PipelineContext &SContext = ((LWOpenGL2_1Pipeline*)Source)->GetContext();
	LWOpenGL2_1PipelineContext &TContext = ((LWOpenGL2_1Pipeline*)Target)->GetContext();
	uint32_t BlockCnt = Source->GetBlockCount();
	for (uint32_t i = 0; i < BlockCnt; i++) {
		LWOpenGL2_1Block &SB = SContext.m_Blocks[i];
		LWOpenGL2_1Block &TB = TContext.m_Blocks[i];
		std::copy(SB.m_Uniforms, SB.m_Uniforms + SB.m_UniformCount, TB.m_Uniforms);
		TB.m_UniformCount = SB.m_UniformCount;
		TB.m_Size = SB.m_Size;
	}
	TContext.m_ProgramID = SContext.m_ProgramID;
	TContext = SContext;
	return *this;
}

LWPipeline *LWVideoDriver_OpenGL2_1::CreatePipeline(LWShader **Stages, uint64_t Flags, LWAllocator &Allocator){
	LWOpenGL2_1PipelineContext Context;
	LWOpenGL2_1Pipeline *P = Allocator.Create<LWOpenGL2_1Pipeline>(Context, Stages, nullptr, nullptr, nullptr, 0, 0, 0, Flags&~LWPipeline::InternalPipeline);
	if (P) UpdatePipelineStages(P);
	return P;
}

LWFrameBuffer *LWVideoDriver_OpenGL2_1::CreateFrameBuffer(const LWVector2i &Size, LWAllocator &Allocator) {
	LWOpenGL2_1FrameBufferContext Con;
	glGenFramebuffersEXT(1, &Con.m_FBOID);
	return Allocator.Create<LWOpenGL2_1FrameBuffer>(Con, Size);
}

bool LWVideoDriver_OpenGL2_1::UpdateTexture(LWTexture *Texture) {
	uint32_t VideoID = ((LWOpenGL2_1Texture*)Texture)->GetContext();
	int32_t GTypes[] = { GL_TEXTURE_1D, GL_TEXTURE_2D, GL_TEXTURE_3D, GL_TEXTURE_CUBE_MAP };
	int32_t MinMagFilters[] = { GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR };
	int32_t WrapFilters[] = { GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER, GL_MIRRORED_REPEAT, GL_REPEAT };
	int32_t CompareFuncs[] = { GL_NONE, GL_COMPARE_R_TO_TEXTURE };
	int32_t CompareModes[] = { GL_NEVER, GL_ALWAYS, GL_LESS, GL_EQUAL, GL_LEQUAL, GL_GREATER, GL_GEQUAL, GL_NOTEQUAL };
	int32_t DepthReadMode[] = { GL_DEPTH_COMPONENT, GL_STENCIL_COMPONENTS }; //Possible bug, GL_STENCIL_COMPONENTS instead of GL_STENCIL_COMPONENT (note: S) is only defined.

	int32_t Type = GTypes[Texture->GetType()];
	glBindTexture(Type, VideoID);
	if(Texture->isDirty()){
		uint32_t State = Texture->GetTextureState();
		uint32_t MinFilter = (State&LWTexture::MinFilterFlag) >> LWTexture::MinFilterBitOffset;
		uint32_t MagFilter = (State&LWTexture::MagFilterFlag) >> LWTexture::MagFilterBitOffset;
		uint32_t WrapS = (State&LWTexture::WrapSFilterFlag) >> LWTexture::WrapSFilterBitOffset;
		uint32_t WrapT = (State&LWTexture::WrapTFilterFlag) >> LWTexture::WrapTFilterBitOffset;
		uint32_t WrapR = (State&LWTexture::WrapRFilterFlag) >> LWTexture::WrapRFilterBitOffset;
		uint32_t CFunc = (State&LWTexture::CompareFuncFlag) >> LWTexture::CompareFuncBitOffset;
		uint32_t CMode = (State&LWTexture::CompareModeFlag) >> LWTexture::CompareModeBitOffset;
		uint32_t DRMode = (State&LWTexture::DepthReadFlag) >> LWTexture::DepthReadBitOffset;

		glTexParameteri(Type, GL_TEXTURE_MIN_FILTER, MinMagFilters[MinFilter]);
		glTexParameteri(Type, GL_TEXTURE_MAG_FILTER, MinMagFilters[MagFilter]);
		glTexParameteri(Type, GL_TEXTURE_WRAP_S, WrapFilters[WrapS]);

		glTexParameteri(Type, GL_TEXTURE_WRAP_T, WrapFilters[WrapT]);
		Texture->ClearDirty();
	}

	return true;
}

bool LWVideoDriver_OpenGL2_1::UpdateTexture1D(LWTexture *Texture, uint32_t MipmapLevel, void *Texels, uint32_t Position, uint32_t Size) {
	//PackTypes:                         SRGBA,            RGBA8,            RGBA8S,  RGBA16,            RGBA16S,  RGBA32,          RGBA32S, RGBA32F,    RG8,              RG8S,    RG16,              RG16S,    RG32,            RG32S,  RG32F,    R8,               R8S,          R16,               R16S,         R32,             R32S,   R32F,     Depth16,              Depth24,              Depth32,               Depth24Stencil8,    BC1,                              BC1_SRGB,                                BC2,                              BC2_SRGB,                                BC3,                              BC3_SRGB,                                BC4, BC5, BC6, DXT7,                              DXT7_SRGB
	const int32_t GInternalFormats[] = { GL_SRGB8_ALPHA8,  GL_RGBA,          GL_RGBA, GL_RGBA,           GL_RGBA,  0,               0,       0,          0,                0,       0,                 0,        0,               0,      0,        GL_LUMINANCE,     GL_LUMINANCE, GL_LUMINANCE,      GL_LUMINANCE, 0,               0,      0,        0,                    0,                    0,                     0,                  GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT,  GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT,  GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT,  0,   0,   0,   GL_COMPRESSED_RGBA_BPTC_UNORM_EXT, GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_EXT };
	const int32_t GFormats[]         = { GL_RGBA,          GL_RGBA,          GL_RGBA, GL_RGBA,           GL_RGBA,  GL_RGBA,         GL_RGBA, GL_RGBA,    GL_RG,            GL_RG,   GL_RG,             GL_RG,    GL_RG,           GL_RG,  GL_RG,    GL_RED,           GL_RED,       GL_RED,            GL_RED,       GL_RED,          GL_RED, GL_RED,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_STENCIL };
	const int32_t GType[]            = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT, GL_UNSIGNED_INT, GL_INT,  GL_FLOAT,   GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT, GL_UNSIGNED_INT, GL_INT, GL_FLOAT, GL_UNSIGNED_BYTE, GL_BYTE,      GL_UNSIGNED_SHORT, GL_SHORT,     GL_UNSIGNED_INT, GL_INT, GL_FLOAT, GL_FLOAT,             GL_FLOAT,             GL_FLOAT,             GL_UNSIGNED_INT_24_8 };
	if (!UpdateTexture(Texture)) return false;
	if (Texels) {
		uint32_t PackType = Texture->GetPackType();
		glTexSubImage1D(GL_TEXTURE_1D, MipmapLevel, Position, Size, GFormats[PackType], GType[PackType], Texels);
	}
	return true;
}

bool LWVideoDriver_OpenGL2_1::UpdateTexture2D(LWTexture *Texture, uint32_t MipmapLevel, void *Texels, const LWVector2i &Position, const LWVector2i &Size) {
	//PackTypes:                         SRGBA,            RGBA8,            RGBA8S,  RGBA16,            RGBA16S,  RGBA32,          RGBA32S, RGBA32F,    RG8,              RG8S,    RG16,              RG16S,    RG32,            RG32S,  RG32F,    R8,               R8S,          R16,               R16S,         R32,             R32S,   R32F,     Depth16,              Depth24,              Depth32,               Depth24Stencil8,    BC1,                              BC1_SRGB,                                BC2,                              BC2_SRGB,                                BC3,                              BC3_SRGB,                                BC4, BC5, BC6, DXT7,                              DXT7_SRGB
	const int32_t GInternalFormats[] = { GL_SRGB8_ALPHA8,  GL_RGBA,          GL_RGBA, GL_RGBA,           GL_RGBA,  0,               0,       0,          0,                0,       0,                 0,        0,               0,      0,        GL_LUMINANCE,     GL_LUMINANCE, GL_LUMINANCE,      GL_LUMINANCE, 0,               0,      0,        0,                    0,                    0,                     0,                  GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT,  GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT,  GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT,  0,   0,   0,   GL_COMPRESSED_RGBA_BPTC_UNORM_EXT, GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_EXT };
	const int32_t GFormats[]         = { GL_RGBA,          GL_RGBA,          GL_RGBA, GL_RGBA,           GL_RGBA,  GL_RGBA,         GL_RGBA, GL_RGBA,    GL_RG,            GL_RG,   GL_RG,             GL_RG,    GL_RG,           GL_RG,  GL_RG,    GL_RED,           GL_RED,       GL_RED,            GL_RED,       GL_RED,          GL_RED, GL_RED,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_STENCIL };
	const int32_t GType[]            = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT, GL_UNSIGNED_INT, GL_INT,  GL_FLOAT,   GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT, GL_UNSIGNED_INT, GL_INT, GL_FLOAT, GL_UNSIGNED_BYTE, GL_BYTE,      GL_UNSIGNED_SHORT, GL_SHORT,     GL_UNSIGNED_INT, GL_INT, GL_FLOAT, GL_FLOAT,             GL_FLOAT,             GL_FLOAT,             GL_UNSIGNED_INT_24_8 };
	if (!UpdateTexture(Texture)) return false;
	if (Texels) {
		uint32_t PackType = Texture->GetPackType();
		if (LWImage::CompressedType(PackType)) {
			glCompressedTexSubImage2D(GL_TEXTURE_2D, MipmapLevel, Position.x, Position.y, Size.x, Size.y, GInternalFormats[PackType], LWImage::GetLength2D(Size, PackType), Texels);
		} else {
			glTexSubImage2D(GL_TEXTURE_2D, MipmapLevel, Position.x, Position.y, Size.x, Size.y, GFormats[Texture->GetPackType()], GType[Texture->GetPackType()], Texels);
		}
	}
	return true;
}

bool LWVideoDriver_OpenGL2_1::UpdateTexture3D(LWTexture *Texture, uint32_t MipmapLevel, void *Texels, const LWVector3i &Position, const LWVector3i &Size) {
	//PackTypes:                         SRGBA,            RGBA8,            RGBA8S,  RGBA16,            RGBA16S,  RGBA32,          RGBA32S, RGBA32F,    RG8,              RG8S,    RG16,              RG16S,    RG32,            RG32S,  RG32F,    R8,               R8S,          R16,               R16S,         R32,             R32S,   R32F,     Depth16,              Depth24,              Depth32,               Depth24Stencil8,    BC1,                              BC1_SRGB,                                BC2,                              BC2_SRGB,                                BC3,                              BC3_SRGB,                                BC4, BC5, BC6, DXT7,                              DXT7_SRGB
	const int32_t GInternalFormats[] = { GL_SRGB8_ALPHA8,  GL_RGBA,          GL_RGBA, GL_RGBA,           GL_RGBA,  0,               0,       0,          0,                0,       0,                 0,        0,               0,      0,        GL_LUMINANCE,     GL_LUMINANCE, GL_LUMINANCE,      GL_LUMINANCE, 0,               0,      0,        0,                    0,                    0,                     0,                  GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT,  GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT,  GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT,  0,   0,   0,   GL_COMPRESSED_RGBA_BPTC_UNORM_EXT, GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_EXT };
	const int32_t GFormats[]         = { GL_RGBA,          GL_RGBA,          GL_RGBA, GL_RGBA,           GL_RGBA,  GL_RGBA,         GL_RGBA, GL_RGBA,    GL_RG,            GL_RG,   GL_RG,             GL_RG,    GL_RG,           GL_RG,  GL_RG,    GL_RED,           GL_RED,       GL_RED,            GL_RED,       GL_RED,          GL_RED, GL_RED,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_STENCIL };
	const int32_t GType[]            = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT, GL_UNSIGNED_INT, GL_INT,  GL_FLOAT,   GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT, GL_UNSIGNED_INT, GL_INT, GL_FLOAT, GL_UNSIGNED_BYTE, GL_BYTE,      GL_UNSIGNED_SHORT, GL_SHORT,     GL_UNSIGNED_INT, GL_INT, GL_FLOAT, GL_FLOAT,             GL_FLOAT,             GL_FLOAT,             GL_UNSIGNED_INT_24_8 };
	if (!UpdateTexture(Texture)) return false;
	if (Texels) {
		uint32_t PackType = Texture->GetPackType();
		if (LWImage::CompressedType(PackType)) {
			glCompressedTexSubImage3D(GL_TEXTURE_3D, MipmapLevel, Position.x, Position.y, Position.z, Size.x, Size.y, Size.z, GInternalFormats[PackType], LWImage::GetLength3D(Size, PackType), Texels);
		} else {
			glTexSubImage3D(GL_TEXTURE_3D, MipmapLevel, Position.x, Position.y, Position.z, Size.x, Size.y, Size.z, GFormats[PackType], GType[PackType], Texels);
		}
	}
	return false;
}

bool LWVideoDriver_OpenGL2_1::UpdateTextureCubeMap(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Face, void *Texels, const LWVector2i &Position, const LWVector2i &Size) {
	//PackTypes:                         SRGBA,            RGBA8,            RGBA8S,  RGBA16,            RGBA16S,  RGBA32,          RGBA32S, RGBA32F,    RG8,              RG8S,    RG16,              RG16S,    RG32,            RG32S,  RG32F,    R8,               R8S,          R16,               R16S,         R32,             R32S,   R32F,     Depth16,              Depth24,              Depth32,               Depth24Stencil8,    BC1,                              BC1_SRGB,                                BC2,                              BC2_SRGB,                                BC3,                              BC3_SRGB,                                BC4, BC5, BC6, DXT7,                              DXT7_SRGB
	const int32_t GInternalFormats[] = { GL_SRGB8_ALPHA8,  GL_RGBA,          GL_RGBA, GL_RGBA,           GL_RGBA,  0,               0,       0,          0,                0,       0,                 0,        0,               0,      0,        GL_LUMINANCE,     GL_LUMINANCE, GL_LUMINANCE,      GL_LUMINANCE, 0,               0,      0,        0,                    0,                    0,                     0,                  GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT,  GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT,  GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT,  0,   0,   0,   GL_COMPRESSED_RGBA_BPTC_UNORM_EXT, GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_EXT };
	const int32_t GFormats[]         = { GL_RGBA,          GL_RGBA,          GL_RGBA, GL_RGBA,           GL_RGBA,  GL_RGBA,         GL_RGBA, GL_RGBA,    GL_RG,            GL_RG,   GL_RG,             GL_RG,    GL_RG,           GL_RG,  GL_RG,    GL_RED,           GL_RED,       GL_RED,            GL_RED,       GL_RED,          GL_RED, GL_RED,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_STENCIL };
	const int32_t GType[]            = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT, GL_UNSIGNED_INT, GL_INT,  GL_FLOAT,   GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT, GL_UNSIGNED_INT, GL_INT, GL_FLOAT, GL_UNSIGNED_BYTE, GL_BYTE,      GL_UNSIGNED_SHORT, GL_SHORT,     GL_UNSIGNED_INT, GL_INT, GL_FLOAT, GL_FLOAT,             GL_FLOAT,             GL_FLOAT,             GL_UNSIGNED_INT_24_8 };
	if (!UpdateTexture(Texture)) return false;
	if (Texels) {
		uint32_t PackType = Texture->GetPackType();
		if (LWImage::CompressedType(PackType)) {
			glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+Face, MipmapLevel, Position.x, Position.y, Size.x, Size.y, GInternalFormats[PackType], LWImage::GetLength2D(Size, PackType), Texels);
		} else {
			glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + Face, MipmapLevel, Position.x, Position.y, Size.x, Size.y, GFormats[Texture->GetPackType()], GType[Texture->GetPackType()], Texels);
		}
	}
	return false;
}

bool LWVideoDriver_OpenGL2_1::UpdateTexture1DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, void *Texels, uint32_t Position, uint32_t Size) {
	return false;
}

bool LWVideoDriver_OpenGL2_1::UpdateTexture2DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, void *Texels, const LWVector2i &Position, const LWVector2i &Size) {
	return false;
}

bool LWVideoDriver_OpenGL2_1::UpdateTextureCubeArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint32_t Face, void *Texels, const LWVector2i &Position, const LWVector2i &Size) {
	return false;
}

bool LWVideoDriver_OpenGL2_1::UpdateVideoBuffer(LWVideoBuffer *VideoBuffer, const uint8_t *Buffer, uint32_t Length, uint32_t Offset) {
	if (!Length) return true;
	const int32_t GTypes[] = { GL_ARRAY_BUFFER, 0, GL_ELEMENT_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, 0, 0 };
	auto Buf = (LWOpenGL2_1Buffer*)VideoBuffer;
	LWOpenGL2_1VideoBufferContext &Context = Buf->GetContext();
	int32_t GType = GTypes[Buf->GetType()];
	if (GType) {
		glBindBuffer(GType, Context.m_VideoID);
		glBufferSubData(GType, Offset, Length, Buffer);
	} else {
		std::copy(Buffer, Buffer + Length, Context.m_Buffer+Offset);
	}
	return true;
}

void *LWVideoDriver_OpenGL2_1::MapVideoBuffer(LWVideoBuffer *VideoBuffer, uint32_t Length, uint32_t Offset) {
	const int32_t GTypes[] = { GL_ARRAY_BUFFER, 0, GL_ELEMENT_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, 0, 0 };
	LWOpenGL2_1Buffer *VB = (LWOpenGL2_1Buffer*)VideoBuffer;
	LWOpenGL2_1VideoBufferContext &Context = VB->GetContext();
	int32_t GType = GTypes[VB->GetType()];
	if (GType) {
		glBindBuffer(GType, Context.m_VideoID);
		return (void*)((uint8_t*)glMapBuffer(GType, GL_WRITE_ONLY) + Offset);
	}
	return Context.m_Buffer + Offset;
}

bool LWVideoDriver_OpenGL2_1::UnmapVideoBuffer(LWVideoBuffer *VideoBuffer) {
	const int32_t GTypes[] = { GL_ARRAY_BUFFER, 0, GL_ELEMENT_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, 0, 0 };
	LWOpenGL2_1Buffer *VB = (LWOpenGL2_1Buffer*)VideoBuffer;
	LWOpenGL2_1VideoBufferContext &Context = VB->GetContext();
	int32_t GType = GTypes[VB->GetType()];
	if (GType) {
		glBindBuffer(GType, Context.m_VideoID);
		glUnmapBuffer(GType);
	}
	return true;
}

bool LWVideoDriver_OpenGL2_1::DownloadTexture1D(LWTexture *Texture, uint32_t MipmapLevel, uint8_t *Buffer) {
	//PackTypes:                         SRGBA,            RGBA8,            RGBA8S,  RGBA16,            RGBA16S,  RGBA32,          RGBA32S, RGBA32F,    RG8,              RG8S,    RG16,              RG16S,    RG32,            RG32S,  RG32F,    R8,               R8S,          R16,               R16S,         R32,             R32S,   R32F,     Depth16,              Depth24,              Depth32,               Depth24Stencil8,    BC1,                              BC1_SRGB,                                BC2,                              BC2_SRGB,                                BC3,                              BC3_SRGB,                                BC4, BC5, BC6, DXT7,                              DXT7_SRGB
	const int32_t GInternalFormats[] = { GL_SRGB8_ALPHA8,  GL_RGBA,          GL_RGBA, GL_RGBA,           GL_RGBA,  0,               0,       0,          0,                0,       0,                 0,        0,               0,      0,        GL_LUMINANCE,     GL_LUMINANCE, GL_LUMINANCE,      GL_LUMINANCE, 0,               0,      0,        0,                    0,                    0,                     0,                  GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT,  GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT,  GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT,  0,   0,   0,   GL_COMPRESSED_RGBA_BPTC_UNORM_EXT, GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_EXT };
	const int32_t GFormats[]         = { GL_RGBA,          GL_RGBA,          GL_RGBA, GL_RGBA,           GL_RGBA,  GL_RGBA,         GL_RGBA, GL_RGBA,    GL_RG,            GL_RG,   GL_RG,             GL_RG,    GL_RG,           GL_RG,  GL_RG,    GL_RED,           GL_RED,       GL_RED,            GL_RED,       GL_RED,          GL_RED, GL_RED,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_STENCIL };
	const int32_t GType[]            = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT, GL_UNSIGNED_INT, GL_INT,  GL_FLOAT,   GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT, GL_UNSIGNED_INT, GL_INT, GL_FLOAT, GL_UNSIGNED_BYTE, GL_BYTE,      GL_UNSIGNED_SHORT, GL_SHORT,     GL_UNSIGNED_INT, GL_INT, GL_FLOAT, GL_FLOAT,             GL_FLOAT,             GL_FLOAT,             GL_UNSIGNED_INT_24_8 };
	if (!UpdateTexture(Texture)) return false;
	uint32_t PackType = Texture->GetPackType();
	glGetTexImage(GL_TEXTURE_1D, MipmapLevel, GFormats[PackType], GType[PackType], Buffer);
	return true;
}

bool LWVideoDriver_OpenGL2_1::DownloadTexture2D(LWTexture *Texture, uint32_t MipmapLevel, uint8_t *Buffer) {
	//PackTypes:                         SRGBA,            RGBA8,            RGBA8S,  RGBA16,            RGBA16S,  RGBA32,          RGBA32S, RGBA32F,    RG8,              RG8S,    RG16,              RG16S,    RG32,            RG32S,  RG32F,    R8,               R8S,          R16,               R16S,         R32,             R32S,   R32F,     Depth16,              Depth24,              Depth32,               Depth24Stencil8,    BC1,                              BC1_SRGB,                                BC2,                              BC2_SRGB,                                BC3,                              BC3_SRGB,                                BC4, BC5, BC6, DXT7,                              DXT7_SRGB
	const int32_t GInternalFormats[] = { GL_SRGB8_ALPHA8,  GL_RGBA,          GL_RGBA, GL_RGBA,           GL_RGBA,  0,               0,       0,          0,                0,       0,                 0,        0,               0,      0,        GL_LUMINANCE,     GL_LUMINANCE, GL_LUMINANCE,      GL_LUMINANCE, 0,               0,      0,        0,                    0,                    0,                     0,                  GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT,  GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT,  GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT,  0,   0,   0,   GL_COMPRESSED_RGBA_BPTC_UNORM_EXT, GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_EXT };
	const int32_t GFormats[]         = { GL_RGBA,          GL_RGBA,          GL_RGBA, GL_RGBA,           GL_RGBA,  GL_RGBA,         GL_RGBA, GL_RGBA,    GL_RG,            GL_RG,   GL_RG,             GL_RG,    GL_RG,           GL_RG,  GL_RG,    GL_RED,           GL_RED,       GL_RED,            GL_RED,       GL_RED,          GL_RED, GL_RED,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_STENCIL };
	const int32_t GType[]            = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT, GL_UNSIGNED_INT, GL_INT,  GL_FLOAT,   GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT, GL_UNSIGNED_INT, GL_INT, GL_FLOAT, GL_UNSIGNED_BYTE, GL_BYTE,      GL_UNSIGNED_SHORT, GL_SHORT,     GL_UNSIGNED_INT, GL_INT, GL_FLOAT, GL_FLOAT,             GL_FLOAT,             GL_FLOAT,             GL_UNSIGNED_INT_24_8 };
	if (!UpdateTexture(Texture)) return false;
	uint32_t PackType = Texture->GetPackType();
	if (LWImage::CompressedType(PackType)) {
		glGetCompressedTexImage(GL_TEXTURE_2D, MipmapLevel, Buffer);
	} else {
		glGetTexImage(GL_TEXTURE_1D, MipmapLevel, GFormats[PackType], GType[PackType], Buffer);
	}
	return true;
}

bool LWVideoDriver_OpenGL2_1::DownloadTexture3D(LWTexture *Texture, uint32_t MipmapLevel, uint8_t *Buffer) {
	//PackTypes:                         SRGBA,            RGBA8,            RGBA8S,  RGBA16,            RGBA16S,  RGBA32,          RGBA32S, RGBA32F,    RG8,              RG8S,    RG16,              RG16S,    RG32,            RG32S,  RG32F,    R8,               R8S,          R16,               R16S,         R32,             R32S,   R32F,     Depth16,              Depth24,              Depth32,               Depth24Stencil8,    BC1,                              BC1_SRGB,                                BC2,                              BC2_SRGB,                                BC3,                              BC3_SRGB,                                BC4, BC5, BC6, DXT7,                              DXT7_SRGB
	const int32_t GInternalFormats[] = { GL_SRGB8_ALPHA8,  GL_RGBA,          GL_RGBA, GL_RGBA,           GL_RGBA,  0,               0,       0,          0,                0,       0,                 0,        0,               0,      0,        GL_LUMINANCE,     GL_LUMINANCE, GL_LUMINANCE,      GL_LUMINANCE, 0,               0,      0,        0,                    0,                    0,                     0,                  GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT,  GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT,  GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT,  0,   0,   0,   GL_COMPRESSED_RGBA_BPTC_UNORM_EXT, GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_EXT };
	const int32_t GFormats[]         = { GL_RGBA,          GL_RGBA,          GL_RGBA, GL_RGBA,           GL_RGBA,  GL_RGBA,         GL_RGBA, GL_RGBA,    GL_RG,            GL_RG,   GL_RG,             GL_RG,    GL_RG,           GL_RG,  GL_RG,    GL_RED,           GL_RED,       GL_RED,            GL_RED,       GL_RED,          GL_RED, GL_RED,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_STENCIL };
	const int32_t GType[]            = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT, GL_UNSIGNED_INT, GL_INT,  GL_FLOAT,   GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT, GL_UNSIGNED_INT, GL_INT, GL_FLOAT, GL_UNSIGNED_BYTE, GL_BYTE,      GL_UNSIGNED_SHORT, GL_SHORT,     GL_UNSIGNED_INT, GL_INT, GL_FLOAT, GL_FLOAT,             GL_FLOAT,             GL_FLOAT,             GL_UNSIGNED_INT_24_8 };
	if (!UpdateTexture(Texture)) return false;
	uint32_t PackType = Texture->GetPackType();
	if (LWImage::CompressedType(PackType)) {
		glGetCompressedTexImage(GL_TEXTURE_3D, MipmapLevel, Buffer);
	} else {
		glGetTexImage(GL_TEXTURE_3D, MipmapLevel, GFormats[PackType], GType[PackType], Buffer);
	}
	return true;
}

bool LWVideoDriver_OpenGL2_1::DownloadTextureCubeMap(LWTexture *Texture, uint32_t Face, uint32_t MipmapLevel, uint8_t *Buffer) {
	//PackTypes:                         SRGBA,            RGBA8,            RGBA8S,  RGBA16,            RGBA16S,  RGBA32,          RGBA32S, RGBA32F,    RG8,              RG8S,    RG16,              RG16S,    RG32,            RG32S,  RG32F,    R8,               R8S,          R16,               R16S,         R32,             R32S,   R32F,     Depth16,              Depth24,              Depth32,               Depth24Stencil8,    BC1,                              BC1_SRGB,                                BC2,                              BC2_SRGB,                                BC3,                              BC3_SRGB,                                BC4, BC5, BC6, DXT7,                              DXT7_SRGB
	const int32_t GInternalFormats[] = { GL_SRGB8_ALPHA8,  GL_RGBA,          GL_RGBA, GL_RGBA,           GL_RGBA,  0,               0,       0,          0,                0,       0,                 0,        0,               0,      0,        GL_LUMINANCE,     GL_LUMINANCE, GL_LUMINANCE,      GL_LUMINANCE, 0,               0,      0,        0,                    0,                    0,                     0,                  GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT,  GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT,  GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT,  0,   0,   0,   GL_COMPRESSED_RGBA_BPTC_UNORM_EXT, GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_EXT };
	const int32_t GFormats[]         = { GL_RGBA,          GL_RGBA,          GL_RGBA, GL_RGBA,           GL_RGBA,  GL_RGBA,         GL_RGBA, GL_RGBA,    GL_RG,            GL_RG,   GL_RG,             GL_RG,    GL_RG,           GL_RG,  GL_RG,    GL_RED,           GL_RED,       GL_RED,            GL_RED,       GL_RED,          GL_RED, GL_RED,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_COMPONENT,   GL_DEPTH_STENCIL };
	const int32_t GType[]            = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT, GL_UNSIGNED_INT, GL_INT,  GL_FLOAT,   GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT, GL_UNSIGNED_INT, GL_INT, GL_FLOAT, GL_UNSIGNED_BYTE, GL_BYTE,      GL_UNSIGNED_SHORT, GL_SHORT,     GL_UNSIGNED_INT, GL_INT, GL_FLOAT, GL_FLOAT,             GL_FLOAT,             GL_FLOAT,             GL_UNSIGNED_INT_24_8 };
	if (!UpdateTexture(Texture)) return false;
	uint32_t PackType = Texture->GetPackType();
	if (LWImage::CompressedType(PackType)) {
		glGetCompressedTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + Face, MipmapLevel, Buffer);
	} else {
		glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + Face, MipmapLevel, GFormats[PackType], GType[PackType], Buffer);
	}
	return true;
}

bool LWVideoDriver_OpenGL2_1::DownloadTexture1DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint8_t *Buffer) {
	return false;
}

bool LWVideoDriver_OpenGL2_1::DownloadTexture2DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint8_t *Buffer) {
	return false;
}

bool LWVideoDriver_OpenGL2_1::DownloadTextureCubeArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint32_t Face, uint8_t *Buffer) {
	return false;
}

bool LWVideoDriver_OpenGL2_1::DownloadVideoBuffer(LWVideoBuffer *VBuffer, uint8_t *Buffer, uint32_t Offset, uint32_t Length) {
	auto Buf = (LWOpenGL2_1Buffer*)VBuffer;
	LWOpenGL2_1VideoBufferContext &Context = Buf->GetContext();
	uint32_t Type = Buf->GetType();
	int32_t GTypes[] = { GL_ARRAY_BUFFER, 0, GL_ELEMENT_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER };

	if (Type == LWVideoBuffer::Vertex || Type==LWVideoBuffer::Index16 || Type==LWVideoBuffer::Index32) {
		glBindBuffer(GTypes[Type], Context.m_VideoID);
		glGetBufferSubData(GTypes[Type], Offset, Length, Buffer);
	} else {
		uint8_t *VBuf = Context.m_Buffer + Offset;
		std::copy(VBuf, VBuf + Length, Buffer);
	}

	return true;
}

LWVideoDriver &LWVideoDriver_OpenGL2_1::DestroyPipeline(LWPipeline *Pipeline){
	LWOpenGL2_1Pipeline *P = (LWOpenGL2_1Pipeline*)Pipeline;
	if (P->isInternalPipeline()) {
		LWOpenGL2_1PipelineContext &Con = P->GetContext();
		glDeleteProgram(Con.m_ProgramID);
	}
	LWAllocator::Destroy(P);
	return *this;
}

LWVideoDriver &LWVideoDriver_OpenGL2_1::DestroyVideoBuffer(LWVideoBuffer *Buffer) {
	LWOpenGL2_1Buffer *Buf = (LWOpenGL2_1Buffer *)Buffer;
	LWOpenGL2_1VideoBufferContext &Context = Buf->GetContext();
	uint32_t Type = Buf->GetType();
	if (Type == LWVideoBuffer::Vertex || Type == LWVideoBuffer::Index16 || Type == LWVideoBuffer::Index32) {
		glDeleteBuffers(1, &Context.m_VideoID);
	} else LWAllocator::Destroy(Context.m_Buffer);
	LWAllocator::Destroy(Buf);
	return *this;
}

LWVideoDriver &LWVideoDriver_OpenGL2_1::DestroyShader(LWShader *Shader) {
	LWOpenGL2_1Shader *S = (LWOpenGL2_1Shader*)Shader;
	uint32_t VideoID = S->GetContext();
	glDeleteShader(S->GetContext());
	LWAllocator::Destroy(S);
	return *this;
}

LWVideoDriver &LWVideoDriver_OpenGL2_1::DestroyTexture(LWTexture *Texture) {
	LWOpenGL2_1Texture *T = (LWOpenGL2_1Texture*)Texture;
	uint32_t VideoID = T->GetContext();
	glDeleteTextures(1, &VideoID);
	LWAllocator::Destroy(T);
	return *this;
}

LWVideoDriver &LWVideoDriver_OpenGL2_1::DestroyFrameBuffer(LWFrameBuffer *FrameBuffer) {
	LWOpenGL2_1FrameBuffer *FB = (LWOpenGL2_1FrameBuffer*)FrameBuffer;
	auto &Con = FB->GetContext();
	glDeleteFramebuffersEXT(1, &Con.m_FBOID);
	LWAllocator::Destroy(FB);
	return *this;
}

LWVideoDriver &LWVideoDriver_OpenGL2_1::ClearColor(uint32_t Color) {
	SetFrameBuffer(m_ActiveFrameBuffer);
	LWVector4f ClearClr = LWUNPACK_COLORVEC4f(Color);
	glClearColor(ClearClr.x, ClearClr.y, ClearClr.z, ClearClr.w);
	glClear(GL_COLOR_BUFFER_BIT);
	return *this;
}

LWVideoDriver &LWVideoDriver_OpenGL2_1::ClearColor(const LWVector4f &Color) {
	SetFrameBuffer(m_ActiveFrameBuffer);
	glClearColor(Color.x, Color.y, Color.z, Color.w);
	glClear(GL_COLOR_BUFFER_BIT);
	return *this;
}

LWVideoDriver &LWVideoDriver_OpenGL2_1::ClearDepth(float Depth) {
	SetFrameBuffer(m_ActiveFrameBuffer);
	glClearDepth(Depth);
	glClear(GL_DEPTH_BUFFER_BIT);
	return *this;
}

LWVideoDriver &LWVideoDriver_OpenGL2_1::ClearStencil(uint8_t Stencil) {
	SetFrameBuffer(m_ActiveFrameBuffer);
	glClearStencil((int32_t)Stencil);
	glClear(GL_STENCIL_BUFFER_BIT);
	return *this;
}


bool LWVideoDriver_OpenGL2_1::SetRasterState(uint64_t Flags, float Bias, float SlopedScaleBias){
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

bool LWVideoDriver_OpenGL2_1::SetPipeline(LWPipeline *Pipeline, LWPipelineInputStream *InputStream, LWVideoBuffer *IndiceBuffer, LWVideoBuffer *IndirectBuffer) {
	//Video buffer types.
	const int32_t GBTypes[] = { GL_ARRAY_BUFFER, 0, GL_ELEMENT_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, 0 };
	//                             Float,    Int,    Uint,            Double,    Vec2      Vec3      Vec4,     uvec2,           uvec3,           uvec4,           ivec2,  ivec3,  ivec4,  dVec2,     dVec3,     dVec4,     mat2,     mat3,     mat4,     dmat2,     dmat3,     dmat4
	const int32_t GIBaseType[] = { GL_FLOAT, GL_INT, GL_UNSIGNED_INT, GL_DOUBLE, GL_FLOAT, GL_FLOAT, GL_FLOAT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_INT, GL_INT, GL_INT, GL_DOUBLE, GL_DOUBLE, GL_DOUBLE, GL_FLOAT, GL_FLOAT, GL_FLOAT, GL_DOUBLE, GL_DOUBLE, GL_DOUBLE };
	const int32_t GIComponentCnt[] = { 1,    1,      1,               1,         2,        3,        4,        2,               3,               4,               2,      3,      4,      2,         3,         4,         4,        9,        16,       4,         9,         16 };
	const int32_t GIComponentSize[] = { sizeof(float), sizeof(int32_t), sizeof(uint32_t), sizeof(double), sizeof(float), sizeof(float), sizeof(float), sizeof(uint32_t), sizeof(uint32_t), sizeof(uint32_t), sizeof(int32_t), sizeof(int32_t), sizeof(int32_t), sizeof(double), sizeof(double), sizeof(double), sizeof(float), sizeof(float), sizeof(float), sizeof(double), sizeof(double), sizeof(double) };

	bool Update = LWVideoDriver::SetPipeline(Pipeline, InputStream, IndiceBuffer, IndirectBuffer);
	auto &Context = ((LWOpenGL2_1Pipeline*)Pipeline)->GetContext();
	uint32_t BlockCount = Pipeline->GetBlockCount();
	uint32_t ResourceCount = Pipeline->GetResourceCount();
	
	if (Update) glUseProgram(Context.m_ProgramID);

	for (uint32_t i = 0; i < BlockCount; i++) {
		LWShaderResource &R = Pipeline->GetBlock(i);
		auto B = R.m_Resource->As<LWOpenGL2_1Buffer>();
		if (!B) continue;
		
		LWOpenGL2_1VideoBufferContext &BContext = B->GetContext();
		LWVideoDriver::UpdateVideoBuffer(B);
		if (!Update) continue;
		
		LWOpenGL2_1Block &Blk = Context.m_Blocks[i];
		uint32_t Offset = R.m_Offset*m_UniformBlockSize;
		for (uint32_t n = 0; n < Blk.m_UniformCount; n++) {
			LWOpenGL2_1BlockUniforms &U = Blk.m_Uniforms[n];
			uint8_t *Data = BContext.m_Buffer + Offset + U.m_Offset;
			if (U.m_Type == GL_FLOAT) glUniform1fv(U.m_Index, U.m_Size, (float*)Data);
			else if (U.m_Type == GL_FLOAT_VEC2) glUniform2fv(U.m_Index, U.m_Size, (float*)Data);
			else if (U.m_Type == GL_FLOAT_VEC3) glUniform3fv(U.m_Index, U.m_Size, (float*)Data);
			else if (U.m_Type == GL_FLOAT_VEC4) glUniform4fv(U.m_Index, U.m_Size, (float*)Data);
			else if (U.m_Type == GL_FLOAT_MAT4) glUniformMatrix4fv(U.m_Index, U.m_Size, false, (float*)Data);
			else if (U.m_Type == GL_INT) glUniform1iv(U.m_Index, U.m_Size, (int32_t*)Data);
			else if (U.m_Type == GL_INT_VEC2) glUniform2iv(U.m_Index, U.m_Size, (int32_t*)Data);
			else if (U.m_Type == GL_INT_VEC3) glUniform3iv(U.m_Index, U.m_Size, (int32_t*)Data);
			else if (U.m_Type == GL_INT_VEC4) glUniform4iv(U.m_Index, U.m_Size, (int32_t*)Data);
			else if (U.m_Type == GL_BOOL) glUniform1iv(U.m_Index, U.m_Size, (int32_t*)Data);
			else if (U.m_Type == GL_BOOL_VEC2) glUniform1iv(U.m_Index, U.m_Size, (int32_t*)Data);
			else if (U.m_Type == GL_BOOL_VEC3) glUniform1iv(U.m_Index, U.m_Size, (int32_t*)Data);
			else if (U.m_Type == GL_BOOL_VEC4) glUniform1iv(U.m_Index, U.m_Size, (int32_t*)Data);
			else if (U.m_Type == GL_FLOAT_MAT3) glUniformMatrix3fv(U.m_Index, U.m_Size, false, (float*)Data);
			else if (U.m_Type == GL_FLOAT_MAT2) glUniformMatrix2fv(U.m_Index, U.m_Size, false, (float*)Data);
			else if (U.m_Type == GL_FLOAT_MAT2x3) glUniformMatrix2x3fv(U.m_Index, U.m_Size, false, (float*)Data);
			else if (U.m_Type == GL_FLOAT_MAT2x4) glUniformMatrix2x4fv(U.m_Index, U.m_Size, false, (float*)Data);
			else if (U.m_Type == GL_FLOAT_MAT3x2) glUniformMatrix3x2fv(U.m_Index, U.m_Size, false, (float*)Data);
			else if (U.m_Type == GL_FLOAT_MAT3x4) glUniformMatrix3x4fv(U.m_Index, U.m_Size, false, (float*)Data);
			else if (U.m_Type == GL_FLOAT_MAT4x2) glUniformMatrix4x2fv(U.m_Index, U.m_Size, false, (float*)Data);
			else if (U.m_Type == GL_FLOAT_MAT4x3) glUniformMatrix4x2fv(U.m_Index, U.m_Size, false, (float*)Data);
		}
	}
	for (uint32_t i = 0; i < ResourceCount; i++) {
		LWShaderResource &R = Pipeline->GetResource(i);
		LWOpenGL2_1Texture *T = (LWOpenGL2_1Texture *)R.m_Resource;
		uint32_t TypeID = R.GetTypeID();
		if (TypeID == LWPipeline::Texture) {
			if (!T) continue;
			glActiveTexture(GL_TEXTURE0 + R.m_StageBindings);
			UpdateTexture(T);
		}
	}

	if (InputStream) {
		//Disable previous attributes:
		for (uint32_t i = 0; i < m_Context.m_ActiveAttribs; i++) glDisableVertexAttribArray(m_Context.m_ActiveAttributeIDs[i]);
		uint32_t BoundArray = 0;
		uint32_t InputCount = Pipeline->GetInputCount();
		uint32_t AttribCnt = 0;
		for (uint32_t i = 0; i < InputCount; i++) {
			LWPipelineInputStream &Stream = InputStream[i];
			LWShaderInput &I = Pipeline->GetInput(i);
			uint32_t Len = I.GetLength();
			uint32_t BindIdx = I.GetBindIndex();
			if(!Len) continue;

			uint32_t BufferID = ((LWOpenGL2_1Buffer*)Stream.m_Buffer)->GetContext().m_VideoID;
			if (BufferID != BoundArray) {
				LWVideoDriver::UpdateVideoBuffer(Stream.m_Buffer);
				glBindBuffer(GL_ARRAY_BUFFER, BufferID);
				BoundArray = BufferID;
			}

			int32_t GBaseType = GIBaseType[I.GetType()];
			int32_t GCompCnt = GIComponentCnt[I.GetType()];
			int32_t GCompSize = GIComponentCnt[I.GetType()] * GCompCnt;
			for (uint32_t n = 0; n < Len; n++, AttribCnt++) {
				m_Context.m_ActiveAttributeIDs[AttribCnt] = BindIdx + n;
				glEnableVertexAttribArray(m_Context.m_ActiveAttributeIDs[AttribCnt]);

				if (GBaseType == GL_INT || GBaseType == GL_UNSIGNED_INT) {
					glVertexAttribIPointer(m_Context.m_ActiveAttributeIDs[AttribCnt], GCompCnt, GBaseType, Stream.m_Stride, (void*)((uintptr_t)Stream.m_Offset + GCompSize * n));
				} else {
					glVertexAttribPointer(m_Context.m_ActiveAttributeIDs[AttribCnt], GCompCnt, GBaseType, false, Stream.m_Stride, (void*)((uintptr_t)Stream.m_Offset + GCompSize * n));
				}
			}
		}
		m_Context.m_ActiveAttribs = AttribCnt;
	}
	if (IndiceBuffer) {
		LWVideoDriver::UpdateVideoBuffer(IndiceBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ((LWOpenGL2_1Buffer*)IndiceBuffer)->GetContext().m_VideoID);
	}
	if (IndirectBuffer) {
		LWVideoDriver::UpdateVideoBuffer(IndirectBuffer);
	}

	return Update;
}

bool LWVideoDriver_OpenGL2_1::SetFrameBuffer(LWFrameBuffer *Buffer, bool ChangeViewport) {
	//This is mostly for OpenGL3.2, but since OpenGL2.1 basically exists to support OpenGLES2, I kindof just leave this as is and hope for the best!
	if (!LWVideoDriver::SetFrameBuffer(Buffer, ChangeViewport)) return false;

	if (!Buffer) glBindFramebuffer(GL_FRAMEBUFFER, 0);
	else {
		auto &Context = ((LWOpenGL2_1FrameBuffer*)Buffer)->GetContext();
		const GLenum AttachmentPnts[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, GL_DEPTH_STENCIL_ATTACHMENT };
		const GLenum GFaces[] = { GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, GL_TEXTURE_CUBE_MAP_POSITIVE_Y,GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, GL_TEXTURE_CUBE_MAP_POSITIVE_Z };

		GLenum DrawBuffers[LWFrameBuffer::Count];
		uint32_t DrawCnt = 0;
		glBindFramebuffer(GL_FRAMEBUFFER, Context.m_FBOID);
		for (uint32_t i = 0; i < LWFrameBuffer::Count; i++) {
			auto &Slot = Buffer->GetAttachment(i);
			if (!Slot.m_Source) {
				if (Context.m_Attached[i] != 0) glFramebufferRenderbuffer(GL_FRAMEBUFFER, Context.m_Attached[i], GL_RENDERBUFFER, 0);
				Context.m_Attached[i] = 0;
				continue;
			}
			auto Tex = (LWOpenGL2_1Texture *)Slot.m_Source;
			uint32_t TexType = Tex->GetType();
			uint32_t TexID = Tex->GetContext();
			Context.m_Attached[i] = AttachmentPnts[i];
			if (Tex->GetTextureState()&LWTexture::RenderBuffer) glFramebufferRenderbuffer(GL_FRAMEBUFFER, AttachmentPnts[i], GL_RENDERBUFFER, TexID);
			else {
				if (TexType == LWTexture::Texture1D) glFramebufferTexture1D(GL_FRAMEBUFFER, AttachmentPnts[i], GL_TEXTURE_1D, TexID, Slot.m_Mipmap);
				else if (TexType == LWTexture::Texture2D) glFramebufferTexture2D(GL_FRAMEBUFFER, AttachmentPnts[i], GL_TEXTURE_2D, TexID, Slot.m_Mipmap);
				else if (TexType == LWTexture::Texture3D) glFramebufferTexture3D(GL_FRAMEBUFFER, AttachmentPnts[i], GL_TEXTURE_3D, TexID, Slot.m_Mipmap, Slot.m_Layer);
				else if (TexType == LWTexture::TextureCubeMap) glFramebufferTexture2D(GL_FRAMEBUFFER, AttachmentPnts[i], GFaces[Slot.m_Face], TexID, Slot.m_Mipmap);
			}
			if (i < LWFrameBuffer::ColorCount) DrawBuffers[DrawCnt++] = AttachmentPnts[i];
		}
		glDrawBuffers(DrawCnt, DrawBuffers);
	}
	return true;
}

LWVideoDriver &LWVideoDriver_OpenGL2_1::DrawBuffer(LWPipeline *Pipeline, int32_t DrawMode, LWPipelineInputStream *InputStreams, LWVideoBuffer *IndexBuffer, uint32_t Count, uint32_t Offset) {
	const int32_t GModes[] = { GL_POINTS, GL_LINE_STRIP, GL_LINES, GL_TRIANGLE_STRIP, GL_TRIANGLES };
	SetFrameBuffer(m_ActiveFrameBuffer);
	SetPipeline(Pipeline, InputStreams, IndexBuffer, nullptr);

	if (IndexBuffer) {
		LWOpenGL2_1Buffer *IBuffer = (LWOpenGL2_1Buffer*)IndexBuffer;
		uint32_t OffsetSize = IBuffer->GetType() == LWVideoBuffer::Index16 ? sizeof(uint16_t) : sizeof(uint32_t);
		GLenum IndexType = IBuffer->GetType() == LWVideoBuffer::Index16 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
		glDrawElements(GModes[DrawMode], Count, IndexType, (void*)(uintptr_t)(Offset*OffsetSize));
	} else {
		glDrawArrays(GModes[DrawMode], Offset, Count);
	}
	
	return *this;
}

LWVideoDriver &LWVideoDriver_OpenGL2_1::DrawInstancedBuffer(LWPipeline *Pipeline, int32_t DrawMode, LWPipelineInputStream *InputStreams, LWVideoBuffer *IndexBuffer, uint32_t Count, uint32_t InstanceCount, uint32_t Offset) {
	return *this;
}

LWVideoDriver &LWVideoDriver_OpenGL2_1::DrawIndirectBuffer(LWPipeline *Pipeline, int32_t DrawMode, LWPipelineInputStream *InpustStreams, LWVideoBuffer *IndexBuffer, LWVideoBuffer *IndirectBuffer, uint32_t IndirectCount, uint32_t IndirectOffset) {
	return *this;
}

LWVideoDriver &LWVideoDriver_OpenGL2_1::Dispatch(LWPipeline *Pipeline, const LWVector3i &GroupDimension) {
	return *this;
}

LWOpenGL2_1Context &LWVideoDriver_OpenGL2_1::GetContext(void) {
	return m_Context;
}

LWVideoDriver_OpenGL2_1::LWVideoDriver_OpenGL2_1(LWWindow *Window, LWOpenGL2_1Context &Context, uint32_t UniformBlockSize) : LWVideoDriver(Window, LWVideoDriver::OpenGL2_1, UniformBlockSize), m_Context(Context) {
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glEnable(GL_FRAMEBUFFER_SRGB);
}
