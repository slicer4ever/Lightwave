#include "LWVideo/LWVideoDrivers/LWVideoDriver_OpenGLES2.h"
#include "LWCore/LWAllocator.h"
#include "LWPlatform/LWWindow.h"
#include "LWVideo/LWTexture.h"
#include "LWVideo/LWImage.h"
#include "LWVideo/LWFrameBuffer.h"
#include <iostream>


LWVideoDriver &LWVideoDriver_OpenGLES2::ViewPort(const LWVector4i &Viewport) {
	m_Viewport = Viewport;
	glViewport(Viewport.x, Viewport.y, Viewport.z, Viewport.w);
	return *this;
}

LWShader *LWVideoDriver_OpenGLES2::CreateShader(const char *VertexModule, const char *GeometryModule, const char *PixelModule, LWAllocator &Allocator, char *ErrorBuffer, uint32_t ErrorBufferLen) {

	const uint32_t BufferMaxLength = 1024 * 16; //32 Kilobytes.
	char VertexBuffer[BufferMaxLength];
	char PixelBuffer[BufferMaxLength];
	LWShaderBlock Blocks[LWSHADER_MAXBLOCKS];
	LWShaderVariable Resources[LWSHADER_MAXBLOCKVARIABLES];
	uint32_t UniformBlockCount = 0;
	uint32_t ResourceCount = 0;

	
	if (!VertexModule || !PixelModule) return nullptr;

	if (!ParseModule(VertexModule, LWShader::Vertex, LWShader::VertexSlotOffset, VertexBuffer, sizeof(VertexBuffer), Blocks + LWSHADER_BLOCKINPUT, nullptr, Blocks + LWSHADER_BLOCKUNIFORM, LWSHADER_MAXBLOCKS - LWSHADER_BLOCKUNIFORM, Resources, LWSHADER_MAXBLOCKVARIABLES, UniformBlockCount, ResourceCount, Allocator)) return nullptr;
	if (!ParseModule(PixelModule, LWShader::Pixel, LWShader::PixelSlotOffset, PixelBuffer, sizeof(PixelBuffer), nullptr, Blocks + LWSHADER_BLOCKOUTPUT, Blocks + LWSHADER_BLOCKUNIFORM, LWSHADER_MAXBLOCKS - LWSHADER_BLOCKUNIFORM, Resources, LWSHADER_MAXBLOCKVARIABLES, UniformBlockCount, ResourceCount, Allocator)) return nullptr;
	//Meta-data loaded, and proper source codes generated, let's generate our shader!

	const char *Modules[3] = { VertexBuffer, nullptr, PixelBuffer };
	int32_t VideoID = glCreateProgram();
	uint32_t ModuleIDs[3] = { glCreateShader(GL_VERTEX_SHADER), 0, glCreateShader(GL_FRAGMENT_SHADER) };
	for (uint32_t i = 0; i < 3; i++) {
		if (!ModuleIDs[i]) continue;
		glShaderSource(ModuleIDs[i], 1, Modules + i, nullptr);
		glCompileShader(ModuleIDs[i]);
		glAttachShader(VideoID, ModuleIDs[i]);

	}

	glLinkProgram(VideoID);
	glUseProgram(VideoID);
	int32_t LinkStatus = 0;
	int32_t ErrorBufLoc = 0;
	uint32_t NextTex = 0;
	uint32_t NextBind = 0;
	glGetProgramiv(VideoID, GL_LINK_STATUS, &LinkStatus);
	if (!LinkStatus) {
		int32_t Len = 0;
		for (int32_t i = 0; i < 3; i++) {
			if (!ModuleIDs[i]) continue;
			Len = 0;
			glGetShaderInfoLog(ModuleIDs[i], ErrorBufferLen - ErrorBufLoc, &Len, ErrorBuffer + ErrorBufLoc);
			ErrorBufLoc += Len;
			glDeleteShader(ModuleIDs[i]);
		}
		Len = 0;
		glGetProgramInfoLog(VideoID, ErrorBufferLen - ErrorBufLoc, &Len, ErrorBuffer + ErrorBufLoc);
		ErrorBufLoc += Len;
		glDeleteProgram(VideoID);
		return nullptr;
	}

	for (uint32_t i = 0; i < 3; i++) {
		if (!ModuleIDs[i]) continue;
		glDeleteShader(ModuleIDs[i]);
	}
	for (uint32_t i = LWSHADER_BLOCKINPUT; i < LWSHADER_BLOCKUNIFORM + UniformBlockCount; i++) {
		for (uint32_t d = 0; d < Blocks[i].m_VariableCount; d++) {
			int32_t Location = 0;
			if (i == LWSHADER_BLOCKINPUT) Location = glGetAttribLocation(VideoID, Blocks[i].m_Variables[d].m_Name); //Construct vertex layout!
			else Location = glGetUniformLocation(VideoID, Blocks[i].m_Variables[d].m_Name);
			Blocks[i].m_Variables[d].m_VideoContext = (void*)(uintptr_t)Location;
		}
	}
	for (uint32_t i = 0; i < ResourceCount; i++) {
		int32_t Location = glGetUniformLocation(VideoID, Resources[i].m_Name);
		if (Location != -1) {
			glUniform1i(Location, NextTex);
			Resources[i].m_VideoContext = (void *)(uintptr_t)(NextTex++);
		} else Resources[i].m_VideoContext = (void*)-1;
	}
	/*
	std::cout << "Uniform Count: " << UniformBlockCount << std::endl;
	for (uint32_t i = 0; i < UniformBlockCount + 2; i++) {
		if (i == LWSHADER_BLOCKINPUT) std::cout << "Input Block: " << std::endl;
		else if (i == LWSHADER_BLOCKOUTPUT) std::cout << "output Block: " << std::endl;
		else std::cout << "Uniform Block " << (i - 2) << ": " << std::endl;
		LWShaderBlock *B = Blocks + i;
		std::cout << "Name: '" << B->m_Name << "' Tag: '" << B->m_Tag << "' HintID: " << B->HintID() << " Length: " << B->Length() << " Is Real Buffer: " << (((B->m_Flag&LWShader::Buffer) != 0) ? "Yes" : "No") << " Location: " << (uintptr_t)B->m_VideoContext;
		if (B->m_Flag&LWShader::Vertex) std::cout << " Vertex Slot: " << (((uintptr_t)B->m_VideoContext >> LWShader::VertexSlotOffset)&LWShader::VertexSlotBits) << std::endl;
		if (B->m_Flag&LWShader::Pixel) std::cout << " Pixel Slot: " << (((uintptr_t)B->m_VideoContext >> LWShader::PixelSlotOffset)&LWShader::PixelSlotBits) << std::endl;

		std::cout << std::endl;
		for (uint32_t k = 0; k < B->m_VariableCount; k++) {
			LWShaderVariable *V = B->m_Variables + k;
			std::cout << "Variable " << k << ": '" << V->m_Name << "' Tag: '" << V->m_Tag << "' Hint: " << V->HintID() << " Type: " << V->TypeID() << " Length: " << V->Length() << " Offset: " << (uintptr_t)V->m_Offset << " Location: " << (uintptr_t)V->m_VideoContext << std::endl;
		}
	}
	std::cout << "Texture Count: " << TextureCount << std::endl;
	for (uint32_t i = 0; i < TextureCount; i++) {
		LWShaderVariable *V = Textures + i;
		std::cout << "Texture " << i << ": '" << V->m_Name << "' Tag: '" << V->m_Tag << "' Hint: " << V->HintID() << " Type: " << V->TypeID() << " Length: " << V->Length() << " Offset: " << (uintptr_t)V->m_Offset << " Location: " << (uintptr_t)V->m_VideoContext << std::endl;
	}*/

	return Allocator.Create<LWShaderCon<uint32_t>>(VideoID, Blocks, Resources, UniformBlockCount + LWSHADER_BLOCKUNIFORM, ResourceCount);

}

LWShader *LWVideoDriver_OpenGLES2::CreateShader(const char *ShaderCode, LWAllocator &Allocator, char *ErrorBuffer, uint32_t ErrorBufferLen) {
	const uint32_t BufferMaxLength = 1024 * 16; //32 Kilobytes.
	const uint32_t NameBufferMaxLength = 256;
	const uint32_t MaxModules = 4; //Support a maximum of 3 modules in the shader.
	char Buffer[BufferMaxLength * MaxModules];
	char NameBuffers[NameBufferMaxLength * MaxModules];
	char *MB[MaxModules]; //ModuleBuffer
	char *MN[MaxModules]; //ModuleName
	char *VB = nullptr;
	char *PB = nullptr;
	
	for (uint32_t i = 0; i < MaxModules; i++) {
		MB[i] = Buffer + BufferMaxLength*i;
		MN[i] = NameBuffers + NameBufferMaxLength * i;
	}
	uint32_t ModuleCount = FindModules(ShaderCode, "OpenGLES2", MN, MB, NameBufferMaxLength, BufferMaxLength, MaxModules, nullptr, nullptr);
	for (uint32_t i = 0; i < ModuleCount; i++) {
		if (!strcmp(MN[i], "Vertex")) VB = MB[i];
		else if (!strcmp(MN[i], "Pixel")) PB = MB[i];
	}
	return CreateShader(VB, nullptr, PB, Allocator, ErrorBuffer, ErrorBufferLen);
}

LWShader *LWVideoDriver_OpenGLES2::CreateComputeShader(const char *ComputeModule, LWAllocator &Allocator, char *ErrorBuffer, uint32_t ErrorBufferLen) {
	return nullptr;
}

LWShader *LWVideoDriver_OpenGLES2::CreateComputeShaderl(const char *ShaderCode, LWAllocator &Allocator, char *ErrorBuffer, uint32_t ErrrorBufferLen) {
	return nullptr;
}

LWTexture *LWVideoDriver_OpenGLES2::CreateTexture1D(uint32_t TextureState, uint32_t PackType, uint32_t Size, uint8_t **Texels, uint32_t MipLevelCnt, LWAllocator &Allocator) {
	return nullptr;
}

LWTexture *LWVideoDriver_OpenGLES2::CreateTexture2D(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint8_t **Texels, uint32_t MipLevelCnt, LWAllocator &Allocator) {
	//PackTypes:                   RGBA8,            RGBA8U,           RGBA16,                    RGBA16,            RGBA32, RGBA32U, RGBA32F, RG8, RG8U, RG16, RG16U, RG32, RG32U, RG32F, R8,                 R8U,                 R16,                       R16U,                      R32, R32U, R32F, Depth16, Depth24, Depth32, Depth24Stencil8, DXT1                             DXT2
	int32_t GInternalFormats[] = { GL_RGBA,          GL_RGBA,          GL_RGBA,                   GL_RGBA,           0,      0,       0,       0,   0,    0,    0,     0,    0,     0,     GL_LUMINANCE,       GL_LUMINANCE,        GL_LUMINANCE,              GL_LUMINANCE,              0,   0,    0,    0,       0,       0,       0,               GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0,    GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT };
	int32_t GFormats[]         = { GL_RGBA,          GL_RGBA,          GL_RGBA,                   GL_RGBA,           0,      0,       0,       0,   0,    0,    0,     0,    0,     0,     GL_LUMINANCE,       GL_LUMINANCE,        GL_LUMINANCE,              GL_LUMINANCE,              0,   0,    0,    0,       0,       0,       0 };
	int32_t GType[]            = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT_4_4_4_4, GL_UNSIGNED_SHORT, 0,      0,       0,       0,   0,    0,    0,     0,    0,     0,     GL_UNSIGNED_BYTE,   GL_UNSIGNED_BYTE,    GL_UNSIGNED_SHORT_4_4_4_4, GL_UNSIGNED_SHORT_4_4_4_4, 0,   0,    0,    0,       0,       0,       0 };
	if (GInternalFormats[PackType] == 0) return nullptr; //Unsupported format!
	uint32_t VideoID = 0;
	uint32_t Type = 0;
	
	if (TextureState&LWTexture::RenderBuffer) {
		glGenRenderbuffers(1, &VideoID);
		glBindRenderbuffer(GL_RENDERBUFFER, VideoID);
		glRenderbufferStorage(GL_RENDERBUFFER, GInternalFormats[PackType], Size.x, Size.y);
		TextureState &= ~(LWTexture::HasMipmaps | LWTexture::MakeMipmaps);
	} else {
		glGenTextures(1, &VideoID);
		glBindTexture(GL_TEXTURE_2D, VideoID);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, MipLevelCnt + 1);
		if (LWImage::CompressedType(PackType)) {
			glCompressedTexImage2D(GL_TEXTURE_2D, 0, GInternalFormats[PackType], Size.x, Size.y, 0, LWImage::GetLength2D(Size, PackType), Texels ? Texels[0] : nullptr);
			for (uint32_t i = 1; i <= MipLevelCnt; i++) {
				LWVector2i MipSize = LWImage::MipmapSize2D(Size, i);
				glCompressedTexImage2D(GL_TEXTURE_2D, i, GInternalFormats[PackType], MipSize.x, MipSize.y, 0, LWImage::GetLength2D(MipSize, PackType), Texels[i]);
			}
			TextureState = (TextureState&~LWTexture::MakeMipmaps) | LWTexture::HasMipmaps;
		} else {
			glTexImage2D(GL_TEXTURE_2D, 0, GInternalFormats[PackType], Size.x, Size.y, 0, GFormats[PackType], GType[PackType], Texels ? Texels[0] : nullptr);
			if (TextureState&LWTexture::MakeMipmaps) {
				glGenerateMipmap(GL_TEXTURE_2D);
				TextureState = (TextureState | LWTexture::HasMipmaps)&(~LWTexture::MakeMipmaps);
			} else {
				for (uint32_t i = 1; i <= MipLevelCnt; i++) {
					LWVector2i MipSize = LWImage::MipmapSize2D(Size, i);
					glTexImage2D(GL_TEXTURE_2D, i, GInternalFormats[PackType], MipSize.x, MipSize.y, 0, GFormats[PackType], GType[PackType], Texels[i]);
				}
				TextureState |= MipLevelCnt > 0 ? LWTexture::HasMipmaps : 0;
			}
		}
	}
	return Allocator.Create<LWTextureCon<uint32_t>>(VideoID, TextureState, PackType, LWVector3i(Size, 0), LWTexture::Texture2D);

}

LWTexture *LWVideoDriver_OpenGLES2::CreateTexture3D(uint32_t TextureState, uint32_t PackType, const LWVector3i &Size, uint8_t **Texels, uint32_t MipLevelCnt, LWAllocator &Allocator) {
	return nullptr;
}

LWTexture *LWVideoDriver_OpenGLES2::CreateTextureCubeMap(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint8_t **Texels, uint32_t MipLevelCnt, LWAllocator &Allocator) {
	return nullptr;
}

LWTexture *LWVideoDriver_OpenGLES2::CreateTexture1DArray(uint32_t TextureState, uint32_t PackType, uint32_t Size, uint32_t Layers, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator) {
	return nullptr;
}

LWTexture *LWVideoDriver_OpenGLES2::CreateTexture2DArray(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint32_t Layers, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator) {
	return nullptr;
}

LWTexture *LWVideoDriver_OpenGLES2::CreateTextureCubeMapArray(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint32_t Layers, uint8_t **Texels, uint32_t MapmapCnt, LWAllocator &Allocator) {
	return nullptr;
}

LWVideoBuffer *LWVideoDriver_OpenGLES2::CreateVideoBuffer(uint32_t Type, uint32_t UsageFlag, uint32_t TypeSize, uint32_t Length, LWAllocator &Allocator, const uint8_t *Buffer) {

	if (Type == LWVideoBuffer::Uniform) {
		return Allocator.Create<LWVideoBufferCon<uint32_t>>(Buffer, &Allocator, TypeSize, Length, UsageFlag | LWVideoBuffer::LocalCopy | LWVideoBuffer::Updated | Type, 0);
	} else {
		int32_t GTypes[] = { GL_ARRAY_BUFFER, 0, GL_ELEMENT_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, 0 };
		uint32_t UsageID = UsageFlag & LWVideoBuffer::UsageFlag;
		//                 Static, WriteDiscard, WriteNoOverlap, Readable
		int32_t GUsage = GL_DYNAMIC_DRAW;
		if (UsageID == LWVideoBuffer::Static) GUsage = GL_STATIC_DRAW;
		
		uint32_t VideoID = 0;
		glGenBuffers(1, &VideoID);
		glBindBuffer(GTypes[Type], VideoID);
		glBufferData(GTypes[Type], TypeSize*Length, Buffer, GUsage);
		return Allocator.Create<LWVideoBufferCon<uint32_t>>(Buffer, &Allocator, TypeSize, Length, UsageFlag | Type, VideoID);
	}
}

LWVideoState *LWVideoDriver_OpenGLES2::CreateVideoState(uint32_t VideoState, uint32_t SourceBlend, uint32_t DestBlend, uint32_t DepthCompareFunc, LWAllocator &Allocator) {
	VideoState |= (SourceBlend << LWVideoState::BLEND_SRC_BITOFFSET) | (DestBlend << LWVideoState::BLEND_DST_BITOFFSET) | (DepthCompareFunc << LWVideoState::DEPTH_COMPARE_BITOFFSET);

	return Allocator.Create<LWVideoState>(VideoState);
}

LWFrameBuffer *LWVideoDriver_OpenGLES2::CreateFrameBuffer(const LWVector2i &Size, LWAllocator &Allocator) {
	uint32_t VideoID = 0;
	glGenFramebuffers(1, &VideoID);
	return Allocator.Create<LWFrameBufferCon<uint32_t>>(VideoID, Size);
}

bool LWVideoDriver_OpenGLES2::UpdateTexture(LWTexture *Texture) {
	uint32_t VideoID = ((LWTextureCon<uint32_t>*)Texture)->GetContext();

	int32_t GTypes[] = { 0, GL_TEXTURE_2D, 0, GL_TEXTURE_CUBE_MAP };
	int32_t MinMagFilters[] = { GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR };
	int32_t WrapFilters[] = { GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER, GL_MIRRORED_REPEAT, GL_REPEAT };
	int32_t CompareFuncs[] = { GL_NONE, GL_COMPARE_R_TO_TEXTURE };
	int32_t CompareModes[] = { GL_NEVER, GL_ALWAYS, GL_LESS, GL_EQUAL, GL_LEQUAL, GL_GREATER, GL_GEQUAL, GL_NOTEQUAL };
	int32_t DepthReadMode[] = { GL_DEPTH_COMPONENT, GL_STENCIL_COMPONENTS }; //Possible bug, GL_STENCIL_COMPONENTS instead of GL_STENCIL_COMPONENT (note: S) is only defined.

	int32_t Type = GTypes[Texture->GetType()];
	glBindTexture(Type, VideoID);
	if (Texture->GetTextureState()&LWTexture::Dirty) {
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
		glTexParameteri(Type, GL_TEXTURE_WRAP_R, WrapFilters[WrapR]);
		glTexParameteri(Type, GL_TEXTURE_COMPARE_FUNC, CompareFuncs[CFunc]);
		glTexParameteri(Type, GL_TEXTURE_COMPARE_MODE, CompareModes[CMode]);
		glTexParameteri(Type, GL_DEPTH_STENCIL_TEXTURE_MODE, DepthReadMode[DRMode]);
	}
	return true;
}

bool LWVideoDriver_OpenGLES2::UpdateTexture1D(LWTexture *Texture, uint32_t MipmapLevel, void *Texels, uint32_t Position, uint32_t Size) {
	return false;
}

bool LWVideoDriver_OpenGLES2::UpdateTexture2D(LWTexture *Texture, uint32_t MipmapLevel, void *Texels, const LWVector2i &Position, const LWVector2i &Size) {
	UpdateTexture(Texture);
	//PackTypes:                   RGBA8,            RGBA8U,           RGBA16,                    RGBA16,            RGBA32, RGBA32U, RGBA32F, RG8, RG8U, RG16, RG16U, RG32, RG32U, RG32F, R8,                 R8U,                 R16,                       R16U,                      R32, R32U, R32F, Depth16, Depth24, Depth32, Depth24Stencil8
	int32_t GFormats[]         = { GL_RGBA,          GL_RGBA,          GL_RGBA,                   GL_RGBA,           0,      0,       0,       0,   0,    0,    0,     0,    0,     0,     GL_LUMINANCE,       GL_LUMINANCE,        GL_LUMINANCE,              GL_LUMINANCE,              0,   0,    0,    0,       0,       0,       0 };
	int32_t GType[]            = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT_4_4_4_4, GL_UNSIGNED_SHORT, 0,      0,       0,       0,   0,    0,    0,     0,    0,     0,     GL_UNSIGNED_BYTE,   GL_UNSIGNED_BYTE,    GL_UNSIGNED_SHORT_4_4_4_4, GL_UNSIGNED_SHORT_4_4_4_4, 0,   0,    0,    0,       0,       0,       0 };
	if (Texels) glTexSubImage2D(GL_TEXTURE_2D, MipmapLevel, Position.x, Position.y, Size.x, Size.y, GFormats[Texture->GetPackType()], GType[Texture->GetPackType()], Texels);
	return true;
}

bool LWVideoDriver_OpenGLES2::UpdateTexture3D(LWTexture *Texture, uint32_t MipmapLevel, void *Texels, const LWVector3i &Position, const LWVector3i &Size) {
	return false;
}

bool LWVideoDriver_OpenGLES2::UpdateTextureCubeMap(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Face, void *Texels, const LWVector2i &Position, const LWVector2i &Size) {
	return false;
}

bool LWVideoDriver_OpenGLES2::UpdateTexture1DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, void *Texels, uint32_t Position, uint32_t Size) {
	return false;
}

bool LWVideoDriver_OpenGLES2::UpdateTexture2DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, void *Texels, const LWVector2i &Position, const LWVector2i &Size) {
	return false;
}

bool LWVideoDriver_OpenGLES2::UpdateTextureCubeMapArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint32_t Face, void *Texels, const LWVector2i &Position, const LWVector2i &Size) {
	return false;
}

bool LWVideoDriver_OpenGLES2::UpdateVideoBuffer(LWVideoBuffer *VideoBuffer, const uint8_t *Buffer, uint32_t Length) {
	LWVideoBufferCon<uint32_t> *Buf = (LWVideoBufferCon<uint32_t>*)VideoBuffer;
	uint32_t Type = Buf->GetType();
	int32_t GTypes[] = { GL_ARRAY_BUFFER, 0, GL_ELEMENT_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, 0 };
	
	if (Type != LWVideoBuffer::Uniform) {
		glBindBuffer(GTypes[Type], Buf->GetContext());
		glBufferSubData(GTypes[Type], 0, Length, Buffer);
	} else {
		std::copy(Buffer, Buffer + Length, Buf->GetLocalBuffer());
	}
	return true;
}

bool LWVideoDriver_OpenGLES2::DownloadTexture1D(LWTexture *Texture, uint32_t MipmapLevel, uint8_t *Buffer) {
	return false;
}

bool LWVideoDriver_OpenGLES2::DownloadTexture2D(LWTexture *Texture, uint32_t MipmapLevel, uint8_t *Buffer) {
	return false;
}

bool LWVideoDriver_OpenGLES2::DownloadTexture3D(LWTexture *Texture, uint32_t MipmapLevel, uint8_t *Buffer) {
	return false;
}

bool LWVideoDriver_OpenGLES2::DownloadTextureCubeMap(LWTexture *Texture, uint32_t Face, uint32_t MipmapLevel, uint8_t *Buffer) {
	return false;
}

bool LWVideoDriver_OpenGLES2::DownloadTexture1DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint8_t *Buffer) {
	return false;
}

bool LWVideoDriver_OpenGLES2::DownloadTexture2DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint8_t *Buffer) {
	return false;
}

bool LWVideoDriver_OpenGLES2::DownloadTextureCubeMapArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint32_t Face, uint8_t *Buffer) {
	return false;
}

bool LWVideoDriver_OpenGLES2::DownloadVideoBuffer(LWVideoBuffer *VBuffer, uint8_t *Buffer, uint32_t Offset, uint32_t Length) {
	return false;
}

LWVideoDriver &LWVideoDriver_OpenGLES2::DestroyVideoState(LWVideoState *State) {
	LWAllocator::Destroy(State);
	return *this;
}

LWVideoDriver &LWVideoDriver_OpenGLES2::DestroyVideoBuffer(LWVideoBuffer *Buffer) {
	LWVideoBufferCon<uint32_t> *Buf = (LWVideoBufferCon<uint32_t>*)Buffer;
	uint32_t Type = Buf->GetType();
	if (Type != LWVideoBuffer::Uniform) {
		uint32_t VideoID = Buf->GetContext();
		glDeleteBuffers(1, &VideoID);
	}
	LWAllocator::Destroy(Buf);
	return *this;
}

LWVideoDriver &LWVideoDriver_OpenGLES2::DestroyShader(LWShader *Shader) {
	LWShaderCon<uint32_t> *S = (LWShaderCon<uint32_t>*)Shader;
	uint32_t VideoID = S->GetContext();
	glDeleteShader(S->GetContext());
	LWAllocator::Destroy(S);
	return *this;
}

LWVideoDriver &LWVideoDriver_OpenGLES2::DestroyTexture(LWTexture *Texture) {
	uint32_t VideoID = ((LWTextureCon<uint32_t>*)Texture)->GetContext();
	glDeleteTextures(1, &VideoID);
	LWAllocator::Destroy(Texture);
	return *this;
}

LWVideoDriver &LWVideoDriver_OpenGLES2::DestroyFrameBuffer(LWFrameBuffer *FrameBuffer) {
	LWFrameBufferCon<uint32_t> *FB = (LWFrameBufferCon<uint32_t>*)FrameBuffer;
	uint32_t VideoID = FB->GetContext();
	glDeleteFramebuffers(1, &VideoID);
	LWAllocator::Destroy(FB);
	return *this;
}

LWVideoDriver &LWVideoDriver_OpenGLES2::Clear(uint32_t ClearFlag, uint32_t Color, float Depth, uint8_t Stencil) {
	uint32_t Flag = ((ClearFlag&LWVideoDriver::Color) != 0 ? GL_COLOR_BUFFER_BIT : 0) | ((ClearFlag&LWVideoDriver::Depth) != 0 ? GL_DEPTH_BUFFER_BIT : 0) | ((ClearFlag&LWVideoDriver::Stencil) != 0 ? GL_STENCIL_BUFFER_BIT : 0);
	const float i = 1.0f / 255.0f;
	glClearColor((float)(Color >> 24)*i, (float)((Color >> 16) & 0xFF)*i, (float)((Color >> 8) & 0xFF)*i, (float)((Color)& 0xFF)*i);
	glClearDepthf(Depth);
	glClearStencil((int32_t)Stencil);

	if (m_NextFrameBuffer != m_ActiveFrameBuffer || (m_ActiveFrameBuffer && m_ActiveFrameBuffer->GetFlag()&LWFrameBuffer::Dirty)) {
		m_ActiveFrameBuffer = m_NextFrameBuffer;
		LWFrameBufferCon<uint32_t> *FB = (LWFrameBufferCon<uint32_t>*)m_ActiveFrameBuffer;
		if (!m_ActiveFrameBuffer) glBindFramebuffer(GL_FRAMEBUFFER, m_DefaultFrameBuffer);
		else {
			GLenum Attachments[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, GL_DEPTH_ATTACHMENT };
			glBindFramebuffer(GL_FRAMEBUFFER, FB->GetContext());
			for (uint32_t i = 0; i < LWFrameBuffer::Count; i++) {
				LWTextureCon<uint32_t> *Tex = (LWTextureCon<uint32_t>*)m_ActiveFrameBuffer->GetAttachment(LWFrameBuffer::Color0 + i);
				if (!Tex) continue;
				if (Tex->GetTextureState()&LWTexture::RenderBuffer) glFramebufferRenderbuffer(GL_FRAMEBUFFER, Attachments[i], GL_RENDERBUFFER, Tex->GetContext());
				else if (Tex->GetType() == LWTexture::Texture2D) glFramebufferTexture2D(GL_FRAMEBUFFER, Attachments[i], GL_TEXTURE_2D, Tex->GetContext(), 0);
			}

			m_ActiveFrameBuffer->ClearDirtyFlag();
		}
	}

	glClear(Flag);
	return *this;
}

LWVideoDriver &LWVideoDriver_OpenGLES2::DrawBuffer(LWShader *Shader, int32_t DrawMode, LWVideoBuffer *InputBlock, LWVideoBuffer *IndexBuffer, uint32_t Count, uint32_t VertexStride, uint32_t Offset, uint32_t InstanceCount) {

	LWShaderCon<uint32_t> *S = (LWShaderCon<uint32_t>*)Shader;
	uint32_t VideoID = S->GetContext();
	glUseProgram(VideoID);
	//Unknown, Float, Int, Vec2, Vec3, Vec4, IVec2, IVec3, IVec4, Mat2, Mat3, Mat4
	int32_t GTypes[] = { 0, GL_FLOAT, GL_INT, GL_FLOAT, GL_FLOAT, GL_FLOAT, GL_INT, GL_INT, GL_INT, GL_FLOAT, GL_FLOAT, GL_FLOAT };
	int32_t GSizes[] = { 0, 1, 1, 2, 3, 4, 2, 3, 4, 4, 12, 16 };
	//Points, LineStrip, Lines, TriangleStrip, Triangles
	int32_t GModes[] = { GL_POINTS, GL_LINE_STRIP, GL_LINES, GL_TRIANGLE_STRIP, GL_TRIANGLES };

	//Video buffer types.
	int32_t GBTypes[] = { GL_ARRAY_BUFFER, 0, GL_ELEMENT_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, 0 };

	//Texture buffer types:
	//int32_t GTTypes[] = { GL_TEXTURE_1D, GL_TEXTURE_2D, GL_TEXTURE_3D, GL_TEXTURE_CUBE_MAP };

	LWShaderBlock *InBlock = Shader->GetInputBlock();
	if (IndexBuffer) {
		LWVideoDriver::UpdateVideoBuffer(IndexBuffer);
		LWVideoBufferCon<uint32_t> *IdxB = (LWVideoBufferCon<uint32_t>*)IndexBuffer;
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IdxB->GetContext());
	}
	//Setup our inputs!
	LWVideoDriver::UpdateVideoBuffer(InputBlock);
	LWVideoBufferCon<uint32_t> *IB = (LWVideoBufferCon<uint32_t>*)InputBlock;

	glBindBuffer(GBTypes[InBlock->m_Flag&LWVideoBuffer::TypeFlag], IB->GetContext());

	for (uint32_t i = 0; i < InBlock->m_VariableCount; i++) {
		LWShaderVariable *V = InBlock->m_Variables + i;
		int32_t vVideoID = (int32_t)(intptr_t)V->m_VideoContext;
		if (vVideoID == -1) continue;
		uint32_t TypeID = V->TypeID();
		glEnableVertexAttribArray(vVideoID);
		if (TypeID == LWShader::Int || TypeID == LWShader::IVec2 || TypeID == LWShader::IVec3 || TypeID == LWShader::IVec4) {
			//glVertexAttribIPointer(vVideoID, GSizes[TypeID], GTypes[TypeID], VertexStride, V->m_Offset);
		} else {
			glVertexAttribPointer(vVideoID, GSizes[TypeID], GTypes[TypeID], false, VertexStride, V->m_Offset);
		}
	}
	
	for (uint32_t i = LWSHADER_BLOCKUNIFORM; i < Shader->GetBlockCount(); i++) {
		LWShaderBlock *UB = Shader->GetBlock(i);
		LWVideoBufferCon<uint32_t> *VB = (LWVideoBufferCon<uint32_t>*)UB->m_Buffer;
		if (!VB) continue;
		int32_t UBContext = (int32_t)(intptr_t)UB->m_VideoContext;
		LWVideoDriver::UpdateVideoBuffer(VB);
		if ((UB->m_Flag&LWShader::Dirty) != 0 || (((UB->m_Flag&LWShader::Buffer) == 0) && (VB->GetUpdateID() != UB->m_BufferID))) {
			uint8_t *BlockBuffer = VB->GetLocalBuffer() + UB->m_BufferOffset*m_UniformBlockSize;
			for (uint32_t d = 0; d < UB->m_VariableCount; d++) {
				//                       Null, Float, Int, Vec2, Vec3, Vec4, Vec2i, Vec3i, Vec4i, mat2, mat3, mat4.
				uint32_t RealSizes[] = { 0, 4,  4,  8,  12, 16, 8,  12, 16, 8,  12, 64 }; //Each row's real size.
				uint32_t Rows[] = { 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 3, 1 };
				uint32_t PackSize = 16; //Each row packs 16 bytes.

				uint8_t Buffer[64 * 1024]; //Max of 64KB of data to copy padded data, into tight packing.
				LWShaderVariable *V = UB->m_Variables + d;

				if (!BlockBuffer) continue;
				for (uint32_t n = 0; n < V->Length()*Rows[V->TypeID()]; n += Rows[V->TypeID()]) {
					for (uint32_t k = 0; k < Rows[V->TypeID()]; k++) {
						std::memcpy(Buffer + n*RealSizes[V->TypeID()] + k*RealSizes[V->TypeID()], BlockBuffer + (int32_t)(intptr_t)V->m_Offset + n*PackSize + k*PackSize, RealSizes[V->TypeID()]);
					}
				}
				int32_t VarID = (int32_t)(intptr_t)V->m_VideoContext;
				if (V->TypeID() == LWShader::Float)      glUniform1fv(VarID, V->Length(), (float*)Buffer);
				else if (V->TypeID() == LWShader::Vec2)  glUniform2fv(VarID, V->Length(), (float*)Buffer);
				else if (V->TypeID() == LWShader::Vec4)  glUniform4fv(VarID, V->Length(), (float*)Buffer);
				else if (V->TypeID() == LWShader::Int)   glUniform1iv(VarID, V->Length(), (int32_t*)Buffer);
				else if (V->TypeID() == LWShader::IVec2) glUniform2iv(VarID, V->Length(), (int32_t*)Buffer);
				else if (V->TypeID() == LWShader::IVec4) glUniform4iv(VarID, V->Length(), (int32_t*)Buffer);
				else if (V->TypeID() == LWShader::Mat4)  glUniformMatrix4fv(VarID, V->Length(), false, (float*)Buffer);
				else if (V->TypeID() == LWShader::Vec3)    glUniform3fv(VarID, V->Length(), (float*)Buffer);
				else if (V->TypeID() == LWShader::IVec3)    glUniform3iv(VarID, V->Length(), (int*)Buffer);
				else if (V->TypeID() == LWShader::Mat2)     glUniformMatrix3fv(VarID, V->Length(), false, (float*)Buffer);
				else if (V->TypeID() == LWShader::Mat3)     glUniformMatrix3fv(VarID, V->Length(), false, (float*)Buffer);
				
			}
			UB->m_BufferID = VB->GetUpdateID();
			UB->m_Flag &= ~LWShader::Dirty;
		}
	}
	
	if (m_NextFrameBuffer != m_ActiveFrameBuffer || (m_ActiveFrameBuffer && m_ActiveFrameBuffer->GetFlag()&LWFrameBuffer::Dirty)) {
		m_ActiveFrameBuffer = m_NextFrameBuffer;
		LWFrameBufferCon<uint32_t> *FB = (LWFrameBufferCon<uint32_t>*)m_ActiveFrameBuffer;
		if (!m_ActiveFrameBuffer) glBindFramebuffer(GL_FRAMEBUFFER, m_DefaultFrameBuffer);
		else {
			GLenum Attachments[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, GL_DEPTH_ATTACHMENT };
			glBindFramebuffer(GL_FRAMEBUFFER, FB->GetContext());
			for (uint32_t i = 0; i < LWFrameBuffer::Count; i++) {
				LWTextureCon<uint32_t> *Tex = (LWTextureCon<uint32_t>*)m_ActiveFrameBuffer->GetAttachment(LWFrameBuffer::Color0 + i);
				if (!Tex) continue;
				if (Tex->GetTextureState()&LWTexture::RenderBuffer) glFramebufferRenderbuffer(GL_FRAMEBUFFER, Attachments[i], GL_RENDERBUFFER, Tex->GetContext());
				else if (Tex->GetType() == LWTexture::Texture2D) glFramebufferTexture2D(GL_FRAMEBUFFER, Attachments[i], GL_TEXTURE_2D, Tex->GetContext(), 0);
			}

			m_ActiveFrameBuffer->ClearDirtyFlag();
		}
	}

	for (uint32_t i = 0; i < Shader->GetResourceCount(); i++) {
		LWShaderVariable *Rsc = Shader->GetResource(i);
		LWTextureCon<uint32_t> *Tex = (LWTextureCon<uint32_t> *)Rsc->m_Offset;
		uint32_t RscVideoID = (uint32_t)(uintptr_t)Rsc->m_VideoContext;
		if (!Tex || RscVideoID == -1) continue;
		glActiveTexture(GL_TEXTURE0 + RscVideoID);
		UpdateTexture(Tex);
	}

	if (IndexBuffer) {
		uint32_t OffsetSize = IndexBuffer->GetType() == LWVideoBuffer::Index16 ? sizeof(uint16_t) : sizeof(uint32_t);
		GLenum IndexType = IndexBuffer->GetType() == LWVideoBuffer::Index16 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
		const void *Elem = (void*)(uintptr_t)(Offset*OffsetSize);
		glDrawElements(GModes[DrawMode], Count, IndexType, Elem);
	} else glDrawArrays(GModes[DrawMode], Offset, Count);

	for (uint32_t i = 0; i < InBlock->m_VariableCount; i++) {
		const LWShaderVariable *V = InBlock->m_Variables + i;
		int32_t vVideoID = (int32_t)(intptr_t)V->m_VideoContext;
		if (vVideoID == -1) continue;;
		glDisableVertexAttribArray(vVideoID);
	}
	return *this;
}

LWVideoDriver &LWVideoDriver_OpenGLES2::Dispatch(LWShader *Shader, const LWVector3i &GroupDimension) {
	return *this;
}

LWVideoDriver &LWVideoDriver_OpenGLES2::SetVideoState(LWVideoState *State) {
	GLenum glCompareFuncs[] = { GL_ALWAYS, GL_NEVER, GL_LESS, GL_GREATER, GL_LEQUAL, GL_GEQUAL };
	GLenum glBlendFuncs[] = { GL_ZERO, GL_ONE, GL_SRC_COLOR, GL_DST_COLOR, GL_SRC_ALPHA, GL_DST_ALPHA, GL_ONE_MINUS_SRC_COLOR, GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA };


	uint32_t VState = State->GetState();
	uint32_t SrcBlendMode = (VState&LWVideoState::BLEND_SRC_BITS) >> LWVideoState::BLEND_SRC_BITOFFSET;
	uint32_t DstBlendMode = (VState&LWVideoState::BLEND_DST_BITS) >> LWVideoState::BLEND_DST_BITOFFSET;
	uint32_t DepthCompareFunc = (VState&LWVideoState::DEPTH_COMPARE_BITS) >> LWVideoState::DEPTH_COMPARE_BITOFFSET;

	auto SetState = [](bool Enable, GLenum State) {
		if (Enable) glEnable(State);
		else glDisable(State);
		return;
	};
	SetState((VState&LWVideoState::DEPTH_TEST) != 0 ? true : false, GL_DEPTH_TEST);
	SetState((VState&LWVideoState::BLENDING) != 0 ? true : false, GL_BLEND);
	SetState((VState&(LWVideoState::CULL_CCW | LWVideoState::CULL_CW)) != 0 ? true : false, GL_CULL_FACE);	
	SetState((VState&LWVideoState::CLIP0) != 0, GL_CLIP_PLANE0);
	SetState((VState&LWVideoState::CLIP1) != 0, GL_CLIP_PLANE1);
	SetState((VState&LWVideoState::CLIP2) != 0, GL_CLIP_PLANE2);

	glDepthFunc(glCompareFuncs[DepthCompareFunc]);
	glBlendFunc(glBlendFuncs[SrcBlendMode], glBlendFuncs[DstBlendMode]);
	glCullFace(((VState&LWVideoState::CULL_CCW) != 0) ? GL_FRONT : GL_BACK);
	glDepthMask((VState&LWVideoState::NoWrite_Depth) != 0);
	glColorMask((VState&LWVideoState::NoWrite_ColorR) == 0, (VState&LWVideoState::NoWrite_ColorG) == 0, (VState&LWVideoState::NoWrite_ColorB) == 0, (VState&LWVideoState::NoWrite_ColorA) == 0);
	glStencilMask((VState&LWVideoState::NoWrite_Stencil) == 0);
	return *this;
}

LWOpenGLES2Context &LWVideoDriver_OpenGLES2::GetContext(void) {
	return m_Context;
}

LWVideoDriver_OpenGLES2::LWVideoDriver_OpenGLES2(LWWindow *Window, LWOpenGLES2Context &Context, int32_t DefaultFrameBuffer, uint32_t UniformBlockSize) : LWVideoDriver(Window, LWVideoDriver::OpenGLES2, UniformBlockSize), m_Context(Context), m_DefaultFrameBuffer(DefaultFrameBuffer) {
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}
