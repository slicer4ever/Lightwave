#include "LWVideo/LWVideoDriver.h"
#include "LWVideo/LWVideoDrivers/LWVideoDriver_DirectX11_1.h"
#include "LWVideo/LWVideoDrivers/LWVideoDriver_OpenGL4_5.h"
#include "LWVideo/LWVideoDrivers/LWVideoDriver_OpenGL3_3.h"
#include "LWVideo/LWVideoDrivers/LWVideoDriver_OpenGL2_1.h"
#include "LWVideo/LWVideoDrivers/LWVideoDriver_OpenGLES2.h"
#include "LWVideo/LWVideoDrivers/LWVideoDriver_Vulkan.h"
#include "LWVideo/LWMesh.h"
#include "LWVideo/LWFrameBuffer.h"
#include "LWVideo/LWPipeline.h"
#include "LWVideo/LWImage.h"
#include "LWPlatform/LWWindow.h"
#include "LWPlatform/LWFileStream.h"
#include "LWCore/LWMatrix.h"
#include <cstring>
#include <iostream>
#include <functional>
#include <cstdarg>

uint32_t LWVideoDriver::FindModule(const char *ShaderCode, const char *Environment, const char *ModuleName, uint32_t DefinedCount, const char **DefinedList, char *ModuleBuffer, uint32_t ModuleBufferLen) {
	const uint32_t MaxDefineTable = 256;
	const uint32_t WordBufferCount = 32;
	const uint32_t WordBufferLength = 256;
	const uint32_t MaxDefines = 32;
	const uint32_t MaxDefineValueLen = 256;
	char LineBuffer[1024];
	uint32_t DefineNameHash[MaxDefines];
	char DefineValues[MaxDefines][MaxDefineValueLen];
	char ModuleMap[WordBufferCount][WordBufferLength];
	char WordBuffer[WordBufferCount][WordBufferLength];
	char *WordList[WordBufferCount];
	char *ModuleList[WordBufferCount];
	DefinedCount = std::min<uint32_t>(DefinedCount, MaxDefines);
	for (uint32_t i = 0; i < WordBufferCount; i++) {
		WordList[i] = WordBuffer[i];
		ModuleList[i] = ModuleMap[i];
	}
	for (uint32_t i = 0; i < DefinedCount; i++) {
		char NameBuffer[MaxDefineValueLen];
		*DefineValues[i] = '\0';
		sscanf(DefinedList[i], "%[^:]:%s", NameBuffer, DefineValues[i]);
		DefineNameHash[i] = LWText::MakeHash(NameBuffer);
	}

	uint32_t NameHash = LWText::MakeHash(ModuleName);
	uint32_t EnvironmentHash = LWText::MakeHash(Environment);
	uint32_t Longest = 0;
	const char *P = ShaderCode;
	char *L = LineBuffer;
	bool InTargetModule = false;
	//This is a terrible name.
	bool WriteInDefineTable[MaxDefineTable];
	WriteInDefineTable[0] = true;
	uint32_t DefineTablePosition = 1;
	uint32_t o = 0;
	for (const char *C = ShaderCode;*C; C++) {
		if (*C == '#') {
			if (LWText::Compare(C, "#module", 7)) {
				C = LWText::CopyToTokens(C + 7, LineBuffer, sizeof(LineBuffer), "\n\r\0");
				if (*C) C += LWText::UTF8ByteSize(C);
				uint32_t SplitCnt = LWText::SplitWords(LineBuffer, WordList, WordBufferLength, WordBufferCount, Longest);
				InTargetModule = false;
				if (SplitCnt < 1) continue;
				uint32_t ModuleCnt = LWText::SplitToken(WordList[0], ModuleList, WordBufferLength, WordBufferCount, Longest, '|');
				bool isTargetModule = false;
				for (uint32_t i = 0; i < ModuleCnt && !isTargetModule; i++) isTargetModule = LWText::MakeHash(ModuleList[i]) == NameHash;
				if (!isTargetModule) continue;
				for (uint32_t i = 1; i < SplitCnt && !InTargetModule; i++) InTargetModule = EnvironmentHash == LWText::MakeHash(WordList[i]);
			}else if(LWText::Compare(C, "#define", 7)){
				C = LWText::CopyToTokens(C + 7, LineBuffer, sizeof(LineBuffer), "\n\r\0");
				if (*C) C += LWText::UTF8ByteSize(C);
				uint32_t SplitCnt = LWText::SplitWords(LineBuffer, WordList, WordBufferLength, WordBufferCount, Longest);
				if(SplitCnt<1 || DefinedCount>=MaxDefines) continue;
				if (WriteInDefineTable[DefineTablePosition - 1]) {
					DefineNameHash[DefinedCount] = LWText::MakeHash(WordList[0]);
					*DefineValues[DefinedCount] = '\0';
					uint32_t r = 0;
					for (uint32_t i = 1; i < SplitCnt; i++) {
						if (i == 1) r += snprintf(DefineValues[DefinedCount] + r, WordBufferLength - r, "%s", WordList[i]);
						else r += snprintf(DefineValues[DefinedCount] + r, WordBufferLength - r, " %s", WordList[i]);
					}
					DefinedCount++;
				}
			} else if (LWText::Compare(C, "#ifdef", 6)) {
				C = LWText::CopyToTokens(C + 6, LineBuffer, sizeof(LineBuffer), "\n\r\0");
				if (*C) C += LWText::UTF8ByteSize(C);
				uint32_t SplitCnt = LWText::SplitWords(LineBuffer, WordList, WordBufferLength, WordBufferCount, Longest);
				if(SplitCnt<1) continue;

				bool isDefined = false;
				uint32_t Hash = LWText::MakeHash(WordList[0]);
				for (uint32_t i = 0; i < DefinedCount && !isDefined; i++) isDefined = DefineNameHash[i] == Hash;
				WriteInDefineTable[DefineTablePosition] = isDefined && WriteInDefineTable[DefineTablePosition - 1];
				DefineTablePosition++;
			} else if (LWText::Compare(C, "#ifndef", 7)) {
				C = LWText::CopyToTokens(C + 7, LineBuffer, sizeof(LineBuffer), "\n\r\0");
				if (*C) C += LWText::UTF8ByteSize(C);
				uint32_t SplitCnt = LWText::SplitWords(LineBuffer, WordList, WordBufferLength, WordBufferCount, Longest);
				if (SplitCnt < 1) continue;
				bool isDefined = false;
				uint32_t Hash = LWText::MakeHash(WordList[0]);
				for (uint32_t i = 0; i < DefinedCount && !isDefined; i++) isDefined = DefineNameHash[i] == Hash;
				WriteInDefineTable[DefineTablePosition] = !isDefined && WriteInDefineTable[DefineTablePosition - 1];
				DefineTablePosition++;
			} else if (LWText::Compare(C, "#endif", 6)) {
				C = LWText::CopyToTokens(C + 6, LineBuffer, sizeof(LineBuffer), "\n\r\0");
				if (*C) C += LWText::UTF8ByteSize(C);
				DefineTablePosition = DefineTablePosition == 1 ? 1 : DefineTablePosition - 1;
			} else if (LWText::Compare(C, "#else", 5)) {
				C = LWText::CopyToTokens(C + 5, LineBuffer, sizeof(LineBuffer), "\n\r\0");
				if (*C) C += LWText::UTF8ByteSize(C);
				if (DefineTablePosition > 1) {
					WriteInDefineTable[DefineTablePosition - 1] = WriteInDefineTable[DefineTablePosition - 2] && !WriteInDefineTable[DefineTablePosition - 1];
				}
			} else if (LWText::Compare(C, "#elifn", 6)) {
				C = LWText::CopyToTokens(C + 6, LineBuffer, sizeof(LineBuffer), "\n\r\0");
				if (*C) C += LWText::UTF8ByteSize(C);
				uint32_t SplitCnt = LWText::SplitWords(LineBuffer, WordList, WordBufferLength, WordBufferCount, Longest);
				if (SplitCnt < 1) continue;

				bool isDefined = false;
				uint32_t Hash = LWText::MakeHash(WordList[0]);
				for (uint32_t i = 0; i < DefinedCount && !isDefined; i++) isDefined = DefineNameHash[i] == Hash;
				if (DefineTablePosition > 1) {
					WriteInDefineTable[DefineTablePosition - 1] = WriteInDefineTable[DefineTablePosition - 2] && !isDefined;
				}
			} else if (LWText::Compare(C, "#elif", 5)) {
				C = LWText::CopyToTokens(C + 5, LineBuffer, sizeof(LineBuffer), "\n\r\0");
				if (*C) C += LWText::UTF8ByteSize(C);
				uint32_t SplitCnt = LWText::SplitWords(LineBuffer, WordList, WordBufferLength, WordBufferCount, Longest);
				if (SplitCnt < 1) continue;

				bool isDefined = false;
				uint32_t Hash = LWText::MakeHash(WordList[0]);
				for (uint32_t i = 0; i < DefinedCount && !isDefined; i++) isDefined = DefineNameHash[i] == Hash;
				if (DefineTablePosition > 1) {
					WriteInDefineTable[DefineTablePosition - 1] = WriteInDefineTable[DefineTablePosition - 2] && isDefined;
				}
			} else {
				Longest = 0;
				sscanf(C, "#%s%n", LineBuffer, &Longest);
				uint32_t Hash = LWText::MakeHash(LineBuffer);
				uint32_t i = 0;
				for (; i < DefinedCount && DefineNameHash[i] != Hash; i++) {}
				if (i < DefinedCount) {
					C += (Longest - 1);
					if (InTargetModule && WriteInDefineTable[DefineTablePosition - 1]) {
						uint32_t ValueLen = (uint32_t)strlen(DefineValues[i]);
						if (o + ValueLen < ModuleBufferLen) {
							std::copy(DefineValues[i], DefineValues[i] + ValueLen, ModuleBuffer + o);
							o += ValueLen;
						}
					}
					continue;
				}
			}
		}
		if (InTargetModule && WriteInDefineTable[DefineTablePosition-1]) {
			if (o < ModuleBufferLen) {
				ModuleBuffer[o++] = *C;
			}
		}
	}
	if (ModuleBufferLen) {
		if (o == ModuleBufferLen) o--;
		ModuleBuffer[o] = '\0';
	}
	return o;
}

uint32_t LWVideoDriver::PerformErrorAnalysis(const char *Source, char *ErrorBuffer, uint32_t ErrorBufferLength) {
	auto FindLine = [](const char *Source, int32_t Line)->const char* {
		const char *S = Source;
		for (; *S && Line; S++) {
			if (*S == '\n') {
				if (*(S + 1) == '\r') S++;
				Line--;
			}
		}
		return S;
	};

	auto CopyStringBackwards = [](char *Dest, const char *Src, uint32_t DestLength)->uint32_t {
		uint32_t SrcLen = (uint32_t)(strlen(Src)+1);
		char *DestLast = Dest + DestLength;
		char *D = Dest + SrcLen;
		for (const char *S = Src + SrcLen; S >= Src; S--, D--) {
			if (D < DestLast) *D = *S;
		}
		if (DestLast) *(DestLast - 1) = '\0';
		return SrcLen;
	};

	auto InsertLine = [&FindLine, &CopyStringBackwards](const char *Source, int32_t Line, char *ErrorBuffer, uint32_t ErrorBufferLength)->char* {
		const char *Pos = FindLine(Source, Line<=1?0:Line-1);
		const char *Next = FindLine(Pos, 3);
		char *BufferLast = ErrorBuffer + ErrorBufferLength;
		uint32_t Length = (uint32_t)(uintptr_t)(Next - Pos);
		//std::cout << "Line: '" << Pos << "'" << std::endl;
		if (ErrorBufferLength < Length) return ErrorBuffer;
		char *B = ErrorBuffer;
		CopyStringBackwards(ErrorBuffer + Length, ErrorBuffer, (ErrorBufferLength - Length));
		for (const char *P = Pos; P != Next && B != BufferLast; B++, P++) *B = *P;
		return B;
	};

	auto ParseFormat = [](const char *Line, int32_t &LineVal, int32_t LineOffset, const char *Fmt, ...)->bool {
		va_list lst;
		va_start(lst, Fmt);
		uint32_t r = vsscanf(Line, Fmt, lst);
		va_end(lst);
		LineVal += LineOffset;
		return r == 2;
	};

	if (!ErrorBuffer) return 0;
	char *BufferLast = ErrorBuffer + ErrorBufferLength;
	char *P = ErrorBuffer;
	char *C = ErrorBuffer;
	for (; *C; C++) {
		if (*C == '\n') {
			C++;
			int32_t Column = 0;
			int32_t Line = 0;
			bool Found = false;
			if (ParseFormat(P, Line, -1, "ERROR: %d:%d", &Column, &Line)) Found = true;
			else if (ParseFormat(P, Line, -1, "WARNING: %d:%d", &Column, &Line)) Found = true;
			else if (ParseFormat(P, Line, 0, "%d(%d)", &Column, &Line)) Found = true;
			else if (ParseFormat(P, Line, 0, "%*[^(](%d,%d", &Line, &Column)) Found = true;
			P = C;
			if (Found) P = C = InsertLine(Source, Line, C, ErrorBufferLength - (uint32_t)(uintptr_t)(C - ErrorBuffer));
		}
	}
	*(BufferLast - 1) = '\0';
	return (uint32_t)(uintptr_t)(C - ErrorBuffer);
}


LWVideoDriver *LWVideoDriver::MakeVideoDriver(LWWindow *Window, uint32_t Type) {
	LWVideoDriver *Driver = nullptr;
	if ((Type&DirectX12) && !Driver) {
		LWMatrix4_UseDXOrtho = true;
		Driver = LWVideoDriver_DirectX12::MakeVideoDriver(Window, Type);
	}
	if ((Type&Vulkan) && !Driver) {
		LWMatrix4_UseDXOrtho = true;
		Driver = LWVideoDriver_Vulkan::MakeVideoDriver(Window, Type);
	}
	if ((Type&Metal) && !Driver) {
		LWMatrix4_UseDXOrtho = false;
		Driver = LWVideoDriver_Metal::MakeVideoDriver(Window, Type);
	}
	if ((Type&DirectX11_1) && !Driver) {
		LWMatrix4_UseDXOrtho = true;
		Driver = LWVideoDriver_DirectX11_1::MakeVideoDriver(Window, Type);
	}
	if ((Type&OpenGL4_5) && !Driver) {
		LWMatrix4_UseDXOrtho = false;
		Driver = LWVideoDriver_OpenGL4_5::MakeVideoDriver(Window, Type);
	}
	if ((Type&OpenGLES3) && !Driver) {
		LWMatrix4_UseDXOrtho = false;
		Driver = LWVideoDriver_OpenGLES3::MakeVideoDriver(Window, Type);
	}
	if ((Type&OpenGL3_3) && !Driver) {
		LWMatrix4_UseDXOrtho = false;
		Driver = LWVideoDriver_OpenGL3_3::MakeVideoDriver(Window, Type);
	}
	if ((Type&DirectX9C) && !Driver) {
		LWMatrix4_UseDXOrtho = true;
		Driver = LWVideoDriver_DirectX9C::MakeVideoDriver(Window, Type);
	}
	if ((Type&OpenGL2_1) && !Driver) {
		LWMatrix4_UseDXOrtho = false;
		Driver = LWVideoDriver_OpenGL2_1::MakeVideoDriver(Window, Type);
	}
	if ((Type&OpenGLES2) && !Driver) {
		LWMatrix4_UseDXOrtho = false;
		Driver = LWVideoDriver_OpenGLES2::MakeVideoDriver(Window, Type);
	}
	return Driver;
}

uint32_t LWVideoDriver::MakeRasterStateHash(uint64_t RasterFlags, float Bias, float ScaleBias) {
	uint32_t Buffer[4];
	*((uint64_t*)Buffer) = RasterFlags;
	*((float*)&Buffer[2]) = Bias;
	*((float*)&Buffer[3]) = ScaleBias;
	return LWText::MakeHashb((const char*)Buffer, 16);
}


bool LWVideoDriver::DestroyVideoDriver(LWVideoDriver *Driver) {
	if (!Driver) return true;
	uint32_t Type = Driver->GetDriverType();
	Driver->ClearPipelines();
	if (Type == DirectX12) return LWVideoDriver_DirectX12::DestroyVideoContext((LWVideoDriver_DirectX12*)Driver);
	else if (Type == Vulkan) return LWVideoDriver_Vulkan::DestroyVideoContext((LWVideoDriver_Vulkan*)Driver);
	else if (Type == Metal) return LWVideoDriver_Metal::DestroyVideoContext((LWVideoDriver_Metal*)Driver);
	else if (Type == DirectX11_1) return LWVideoDriver_DirectX11_1::DestroyVideoContext((LWVideoDriver_DirectX11_1*)Driver);
	else if (Type == OpenGL4_5) return LWVideoDriver_OpenGL4_5::DestroyVideoContext((LWVideoDriver_OpenGL4_5*)Driver);
	else if (Type == OpenGLES3) return LWVideoDriver_OpenGLES3::DestroyVideoContext((LWVideoDriver_OpenGLES3*)Driver);
	else if (Type == OpenGL3_3) return LWVideoDriver_OpenGL3_3::DestroyVideoContext((LWVideoDriver_OpenGL3_3*)Driver);
	else if (Type == DirectX9C) return LWVideoDriver_DirectX9C::DestroyVideoContext((LWVideoDriver_DirectX9C*)Driver);
	else if (Type == OpenGL2_1) return LWVideoDriver_OpenGL2_1::DestroyVideoContext((LWVideoDriver_OpenGL2_1*)Driver);
	else if (Type == OpenGLES2) return LWVideoDriver_OpenGLES2::DestroyVideoContext((LWVideoDriver_OpenGLES2*)Driver);
	return false;
}

bool LWVideoDriver::UpdatePipelineStages(LWPipeline *Pipeline) {
	uint64_t Flag = Pipeline->GetFlag();
	if ((Flag&LWPipeline::DirtyStages) == 0) return false;
	uint32_t PipelineHash = Pipeline->GetPipelineHash();
	LWPipeline *InternalPipeline = nullptr;
	auto Res = m_PipelineMap.find(PipelineHash);
	if (Res == m_PipelineMap.end()) {
		InternalPipeline = CreatePipeline(Pipeline, *LWAllocator::GetAllocator(Pipeline));
		if (!InternalPipeline) return false;
		InternalPipeline->BuildMappings();
		m_PipelineMap.emplace(PipelineHash, InternalPipeline);
	} else InternalPipeline = Res->second;
	ClonePipeline(Pipeline, InternalPipeline);
	Pipeline->SetFlag(Flag&~LWPipeline::DirtyStages);
	return true;
}

bool LWVideoDriver::SetFrameBuffer(LWFrameBuffer *Buffer) {
	bool Dirty = Buffer ? Buffer->isDirty() : false;
	if (Buffer == m_ActiveFrameBuffer) {
		if (!Dirty) return false;
	}
	m_ActiveFrameBuffer = Buffer;
	if (m_ActiveFrameBuffer) m_ActiveFrameBuffer->ClearDirty();
	return true;
}

bool LWVideoDriver::SetPipeline(LWPipeline *Pipeline, LWVideoBuffer *Vertices, LWVideoBuffer *Indices, uint32_t VerticeStride, uint32_t Offset){
	bool Dirty = Pipeline->isDirty();
	uint64_t Flag = Pipeline->GetFlag();
	Dirty = UpdatePipelineStages(Pipeline) || Dirty;
	if (!Pipeline->isComputePipeline()) SetRasterState(Flag&LWPipeline::RasterFlags, Pipeline->GetBias(), Pipeline->GetSlopedBias());
	if (Pipeline == m_ActivePipeline) {
		if (!Dirty) return false;
	}
	m_ActivePipeline = Pipeline;
	m_ActivePipeline->ClearDirty();
	return true;
}

bool LWVideoDriver::SetRasterState(uint64_t Flags, float Bias, float SlopedScaleBias) {
	if (m_ActiveRasterFlags == Flags && m_ActiveBias==Bias && m_ActiveSlopedBias==SlopedScaleBias) return false;
	m_ActiveRasterFlags = Flags;
	return true;
}

LWShader *LWVideoDriver::LoadShader(uint32_t ShaderType, const char *Path, LWAllocator &Allocator, uint32_t DefinedCount, const char **DefinedList, char *CompiledBuffer, char *ErrorBuffer, uint32_t *CompiledBufferLen, uint32_t ErrorBufferLen, LWFileStream *ExistingStream) {
	const uint32_t BufferSize = 32 * 1024;
	char Buffer[BufferSize];
	LWFileStream Stream;
	if (!LWFileStream::OpenStream(Stream, Path, LWFileStream::ReadMode | LWFileStream::BinaryMode, Allocator, ExistingStream)) {
		std::cout << "Error could not open file: '" << Path << "'" << std::endl;
		return nullptr;
	}
	Stream.ReadText(Buffer, BufferSize);
	return ParseShader(ShaderType, Buffer, Allocator, DefinedCount, DefinedList, CompiledBuffer, ErrorBuffer, CompiledBufferLen, ErrorBufferLen);
}

LWShader *LWVideoDriver::ParseShader(uint32_t ShaderType, const char *Source, LWAllocator &Allocator, uint32_t DefinedCount, const char **DefinedList, char *CompiledBuffer, char *ErrorBuffer, uint32_t *CompiledBufferLen, uint32_t ErrorBufferLen) {
	const uint32_t BufferSize = 16 * 1024;
	char Buffer[BufferSize];
	char *DriverNames[] = LWVIDEODRIVER_NAMES;
	char *ModuleNames[] = { "Vertex", "Pixel", "Geometry", "Compute" };
	uint32_t DriverID = GetDriverID();
	uint32_t Len = FindModule(Source, DriverNames[DriverID], ModuleNames[ShaderType], DefinedCount, DefinedList, Buffer, BufferSize);
	if (!Len) return nullptr;
	return CreateShader(ShaderType, Buffer, Allocator, CompiledBuffer, ErrorBuffer, CompiledBufferLen, ErrorBufferLen);
}

LWPipeline *LWVideoDriver::CreatePipeline(LWShader *VertexShader, LWShader *GeomShader, LWShader *PixelShader, uint64_t Flags, LWAllocator &Allocator) {
	LWShader *Stages[LWPipeline::StageCount] = { VertexShader, GeomShader, PixelShader };
	return CreatePipeline(Stages, Flags, Allocator);
}

LWPipeline *LWVideoDriver::CreatePipeline(LWShader *ComputeShader, LWAllocator &Allocator) {
	LWShader *Stages[LWPipeline::StageCount] = { ComputeShader, nullptr, nullptr };
	return CreatePipeline(Stages, LWPipeline::ComputePipeline, Allocator);
}

LWPipeline *LWVideoDriver::CreatePipeline(LWShader *VertexShader, LWShader *GeomShader, LWShader *PixelShader, uint64_t Flags, uint64_t DepthCompareFunc, uint64_t CullMode, uint64_t FillMode, LWAllocator &Allocator) {
	return CreatePipeline(VertexShader, GeomShader, PixelShader, Flags | (DepthCompareFunc << LWPipeline::DEPTH_COMPARE_BITOFFSET) | (CullMode << LWPipeline::CULL_BITOFFSET) | (FillMode << LWPipeline::FILL_MODE_BITOFFSET), Allocator);
}

LWPipeline *LWVideoDriver::CreatePipeline(LWShader *VertexShader, LWShader *GeomShader, LWShader *PixelShader, uint64_t Flags, uint64_t DepthCompareFunc, uint64_t CullMode, uint64_t FillMode, uint64_t SrcBlendMode, uint64_t DstBlendMode, LWAllocator &Allocator) {
	return CreatePipeline(VertexShader, GeomShader, PixelShader, Flags | (DepthCompareFunc << LWPipeline::DEPTH_COMPARE_BITOFFSET) | (CullMode << LWPipeline::CULL_BITOFFSET) | (FillMode << LWPipeline::FILL_MODE_BITOFFSET) | (SrcBlendMode << LWPipeline::BLEND_SRC_BITOFFSET) | (DstBlendMode << LWPipeline::BLEND_DST_BITOFFSET), Allocator);
}

LWPipeline *LWVideoDriver::CreatePipeline(LWShader *VertexShader, LWShader *GeomShader, LWShader *PixelShader, uint64_t Flags, uint64_t DepthCompareFunc, uint64_t CullMode, uint64_t FillMode, uint64_t StencilCompareFunc, uint64_t StencilFailOp, uint64_t StencilDepthFailOp, uint64_t StencilPassOp, LWAllocator &Allocator) {
	return CreatePipeline(VertexShader, GeomShader, PixelShader, Flags | (DepthCompareFunc << LWPipeline::DEPTH_COMPARE_BITOFFSET) | (CullMode << LWPipeline::CULL_BITOFFSET) | (FillMode << LWPipeline::FILL_MODE_BITOFFSET) | (StencilCompareFunc << LWPipeline::STENCIL_COMPARE_BITOFFSET) | (StencilFailOp << LWPipeline::STENCIL_OP_SFAIL_BITOFFSET) | (StencilDepthFailOp << LWPipeline::STENCIL_OP_DFAIL_BITOFFSET) | (StencilPassOp << LWPipeline::STENCIL_OP_PASS_BITOFFSET) | LWPipeline::STENCIL_READMASK_BITS | LWPipeline::STENCIL_WRITEMASK_BITS, Allocator);
}

LWPipeline *LWVideoDriver::CreatePipeline(LWShader *VertexShader, LWShader *GeomShader, LWShader *PixelShader, uint64_t Flags, uint64_t DepthCompareFunc, uint64_t CullMode, uint64_t FillMode, uint64_t SrcBlendMode, uint64_t DstBlendMode, uint64_t StencilCompareFunc, uint64_t StencilFailOp, uint64_t StencilDepthFailOp, uint64_t StencilPassOp, LWAllocator &Allocator) {
	return CreatePipeline(VertexShader, GeomShader, PixelShader, Flags | (DepthCompareFunc << LWPipeline::DEPTH_COMPARE_BITOFFSET) | (CullMode << LWPipeline::CULL_BITOFFSET) | (FillMode << LWPipeline::FILL_MODE_BITOFFSET) | (SrcBlendMode << LWPipeline::BLEND_SRC_BITOFFSET) | (DstBlendMode << LWPipeline::BLEND_DST_BITOFFSET) | (StencilCompareFunc << LWPipeline::STENCIL_COMPARE_BITOFFSET) | (StencilFailOp << LWPipeline::STENCIL_OP_SFAIL_BITOFFSET) | (StencilDepthFailOp << LWPipeline::STENCIL_OP_DFAIL_BITOFFSET) | (StencilPassOp << LWPipeline::STENCIL_OP_PASS_BITOFFSET) | LWPipeline::STENCIL_READMASK_BITS | LWPipeline::STENCIL_WRITEMASK_BITS, Allocator);
}

LWTexture *LWVideoDriver::CreateTexture(uint32_t TextureState, LWImage &Image, LWAllocator &Allocator) {
	LWTexture *R = nullptr;
	uint32_t PackType = Image.GetPackType();
	uint32_t MipCnt = Image.GetMipmapCount();
	uint32_t ImageType = Image.GetType();
	if (ImageType == LWImage::Image1D) R = CreateTexture1D(TextureState, PackType, Image.GetSize1D(), Image.GetTexels(), Image.GetMipmapCount(), Allocator);
	else if (ImageType == LWImage::Image2D) R = CreateTexture2D(TextureState, PackType, Image.GetSize2D(), Image.GetTexels(), Image.GetMipmapCount(), Allocator);
	else if (ImageType == LWImage::Image3D) R = CreateTexture3D(TextureState, PackType, Image.GetSize3D(), Image.GetTexels(), Image.GetMipmapCount(), Allocator);
	else if (ImageType == LWImage::ImageCubeMap) R = CreateTextureCubeMap(TextureState, PackType, Image.GetSize2D(), Image.GetTexels(), Image.GetMipmapCount(), Allocator);
	return R;
}

bool LWVideoDriver::UpdateTexture1D(LWTexture *Texture, uint32_t MipmapLevel, void *Texels) {
	return UpdateTexture1D(Texture, MipmapLevel, Texels, 0, LWImage::MipmapSize1D(Texture->Get1DSize(), MipmapLevel));
}

bool LWVideoDriver::UpdateTexture2D(LWTexture *Texture, uint32_t MipmapLevel, void *Texels) {
	return UpdateTexture2D(Texture, MipmapLevel, Texels, LWVector2i(), LWImage::MipmapSize2D(Texture->Get2DSize(), MipmapLevel));
}

bool LWVideoDriver::UpdateTexture3D(LWTexture *Texture, uint32_t MipmapLevel, void *Texels) {
	return UpdateTexture3D(Texture, MipmapLevel, Texels, LWVector3i(), LWImage::MipmapSize3D(Texture->Get3DSize(), MipmapLevel));
}

bool LWVideoDriver::UpdateTextureCubeMap(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Face, void *Texels) {
	return UpdateTextureCubeMap(Texture, MipmapLevel, Face, Texels, LWVector2i(), LWImage::MipmapSize2D(Texture->Get2DSize(), MipmapLevel));
}

bool LWVideoDriver::UpdateTexture1DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, void *Texels) {
	return UpdateTexture1DArray(Texture, MipmapLevel, Layer, Texels, 0, LWImage::MipmapSize1D(Texture->Get1DSize(), MipmapLevel));
}

bool LWVideoDriver::UpdateTexture2DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, void *Texels) {
	return UpdateTexture2DArray(Texture, MipmapLevel, Layer, Texels, LWVector2i(), LWImage::MipmapSize2D(Texture->Get2DSize(), MipmapLevel));
}

bool LWVideoDriver::UpdateTextureCubeArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint32_t Face, void *Texels) {
	return UpdateTextureCubeArray(Texture, MipmapLevel, Layer, Face, Texels, LWVector2i(), LWImage::MipmapSize2D(Texture->Get2DSize(), MipmapLevel));
}

LWVideoDriver &LWVideoDriver::DrawMesh(LWPipeline *Pipeline, int32_t DrawMode, LWBaseMesh *Mesh){
	Mesh->ClearFinished();
	return DrawBuffer(Pipeline, DrawMode, Mesh->GetVertexBuffer(), Mesh->GetIndiceBuffer(), Mesh->GetRenderCount(), Mesh->GetTypeSize());
}

LWVideoDriver &LWVideoDriver::DrawInstancedMesh(LWPipeline *Pipeline, int32_t DrawMode, LWBaseMesh *Mesh, uint32_t InstanceCount) {
	Mesh->ClearFinished();
	return DrawInstancedBuffer(Pipeline, DrawMode, Mesh->GetVertexBuffer(), Mesh->GetIndiceBuffer(), Mesh->GetRenderCount(), Mesh->GetTypeSize(), InstanceCount, 0);
}

LWVideoDriver &LWVideoDriver::DrawMesh(LWPipeline *Pipeline, int32_t DrawMode, LWBaseMesh *Mesh, uint32_t Count, uint32_t Offset){
	Mesh->ClearFinished();
	return DrawBuffer(Pipeline, DrawMode, Mesh->GetVertexBuffer(), Mesh->GetIndiceBuffer(), Count, Mesh->GetTypeSize(), Offset);
}

LWVideoDriver &LWVideoDriver::DrawInstancedMesh(LWPipeline *Pipeline, int32_t DrawMode, LWBaseMesh *Mesh, uint32_t Count, uint32_t InstanceCount, uint32_t Offset) {
	Mesh->ClearFinished();
	return DrawInstancedBuffer(Pipeline, DrawMode, Mesh->GetVertexBuffer(), Mesh->GetIndiceBuffer(), Count, Mesh->GetTypeSize(), InstanceCount, Offset);
}

LWVideoDriver &LWVideoDriver::UpdateMesh(LWBaseMesh *Mesh){
	if (!Mesh->isFinished()) return *this;
	Mesh->ClearFinished();
	LWVideoBuffer *IBuffer = Mesh->GetIndiceBuffer();
	UpdateVideoBuffer(Mesh->GetVertexBuffer());
	if (IBuffer) UpdateVideoBuffer(IBuffer);
	return *this;
}

bool LWVideoDriver::UpdateVideoBuffer(LWVideoBuffer *Buffer){
	bool Dirty = Buffer->isDirty();
	if (!Dirty) return Dirty;
	bool Res = UpdateVideoBuffer(Buffer, Buffer->GetLocalBuffer(), Buffer->GetEditLength());
	Buffer->ClearDirty();
	return Res;
}

LWVideoDriver &LWVideoDriver::ViewPort(void){
	return ViewPort(LWVector4i(0, 0, m_Window->GetSize()));
}

LWVideoDriver &LWVideoDriver::ViewPort(const LWFrameBuffer *FrameBuffer) {
	return ViewPort(LWVector4i(0, 0, FrameBuffer->GetSize()));
}

uint32_t LWVideoDriver::GetUniformBlockPaddedSize(uint32_t RawSize) const {
	uint32_t n = RawSize % m_UniformBlockSize;
	return (n == 0 ? RawSize : RawSize + (m_UniformBlockSize - n));
}

uint32_t LWVideoDriver::GetUniformBlockOffset(uint32_t RawSize, uint32_t Offset) const {
	uint32_t Blocks = (RawSize+(m_UniformBlockSize-1)) / m_UniformBlockSize;
	return Blocks * Offset;
}

LWWindow *LWVideoDriver::GetWindow(void) const{
	return m_Window;
}

LWVector4i LWVideoDriver::GetViewPort(void) const{
	return m_Viewport;
}

uint32_t LWVideoDriver::GetDriverType(void) const{
	return m_DriverType;
}

uint64_t LWVideoDriver::GetRasterFlags(void) const {
	return m_ActiveRasterFlags;
}

float LWVideoDriver::GetBias(void) const {
	return m_ActiveBias;
}

float LWVideoDriver::GetSlopedBias(void) const {
	return m_ActiveSlopedBias;
}

uint32_t LWVideoDriver::GetDriverID(void) const {
	return (uint32_t)(log(m_DriverType) / log(2));
}

uint32_t LWVideoDriver::GetUniformBlockSize(void) const {
	return m_UniformBlockSize;
}

LWVideoDriver::LWVideoDriver(LWWindow *Window, uint32_t DriverType, uint32_t UniformBlockSize) : m_Window(Window), m_DriverType(DriverType), m_UniformBlockSize(UniformBlockSize) {}

//Protected functions:
LWPipeline *LWVideoDriver::CreatePipeline(LWPipeline *Source, LWAllocator &Allocator) {
	return nullptr;
}

LWVideoDriver &LWVideoDriver::ClonePipeline(LWPipeline *Target, LWPipeline *Source) {
	Target->ClonePipeline(Source);
	return *this;
}

LWVideoDriver &LWVideoDriver::ClearPipelines(void) {
	for (auto &&Iter : m_PipelineMap) DestroyPipeline(Iter.second);
	return *this;
}

/*!!! STUBBED functions for platforms that don't have the corresponding driver api. !!!*/
bool LWVideoDriver::Update(void) {
	return false;
}

LWVideoDriver &LWVideoDriver::ClearColor(uint32_t Color) {
	return *this;
}

LWVideoDriver &LWVideoDriver::ClearDepth(float Depth) {
	return *this;
}

LWVideoDriver &LWVideoDriver::ClearStencil(uint8_t Stencil) {
	return *this;
}

LWVideoDriver &LWVideoDriver::ViewPort(const LWVector4i &Viewport) {
	return *this;
}

LWVideoDriver &LWVideoDriver::Present(uint32_t SwapInterval) {
	return *this;
}

LWVideoDriver &LWVideoDriver::DrawBuffer(LWPipeline *Pipeline, int32_t DrawMode, LWVideoBuffer *InputBlock, LWVideoBuffer *IndexBuffer, uint32_t Count, uint32_t VertexStride, uint32_t Offset) {
	return *this;
}

LWVideoDriver &LWVideoDriver::DrawInstancedBuffer(LWPipeline *Pipeline, int32_t DrawMode, LWVideoBuffer *InputBlock, LWVideoBuffer *IndexBuffer, uint32_t Count, uint32_t VertexStride, uint32_t InstanceCount, uint32_t Offset) {
	return *this;
}

LWVideoDriver &LWVideoDriver::Dispatch(LWPipeline *Pipeline, const LWVector3i &GroupDimension) {
	return *this;
}

LWPipeline *LWVideoDriver::CreatePipeline(LWShader **ShaderStages, uint64_t Flags, LWAllocator &Allocator) {
	return nullptr;
}

LWShader *LWVideoDriver::CreateShader(uint32_t ShaderType, const char *Source, LWAllocator &Allocator, char *CompiledBuffer, char *ErrorBuffer, uint32_t *CompiledBufferLen, uint32_t ErrorBufferLen) {
	return nullptr;
}

LWShader *LWVideoDriver::CreateShaderCompiled(uint32_t ShaderType, const char *CompiledCode, uint32_t CompiledLen, LWAllocator &Allocator, char *ErrorBuffer, uint32_t ErroBufferLen) {
	return nullptr;
}

LWTexture *LWVideoDriver::CreateTexture1D(uint32_t TextureState, uint32_t PackType, uint32_t Size, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator) {
	return nullptr;
}

LWTexture *LWVideoDriver::CreateTexture2D(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator) {
	return nullptr;
}

LWTexture *LWVideoDriver::CreateTexture3D(uint32_t TextureState, uint32_t PackType, const LWVector3i &Size, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator) {
	return nullptr;
}

LWTexture *LWVideoDriver::CreateTextureCubeMap(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator) {
	return nullptr;
}

LWTexture *LWVideoDriver::CreateTexture1DArray(uint32_t TextureState, uint32_t PackType, uint32_t Size, uint32_t Layers, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator) {
	return nullptr;
}

LWTexture *LWVideoDriver::CreateTexture2DArray(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint32_t Layers, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator) {
	return nullptr;
}

LWTexture *LWVideoDriver::CreateTextureCubeArray(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint32_t Layers, uint8_t **Texels, uint32_t MapmapCnt, LWAllocator &Allocator) {
	return nullptr;
}

LWVideoBuffer *LWVideoDriver::CreateVideoBuffer(uint32_t Type, uint32_t UsageFlag, uint32_t TypeSize, uint32_t Length, LWAllocator &Allocator, const uint8_t *Buffer) {
	return nullptr;
}

LWFrameBuffer *LWVideoDriver::CreateFrameBuffer(const LWVector2i &Size, LWAllocator &Allocator) {
	return nullptr;
}

bool LWVideoDriver::UpdateTexture(LWTexture *Texture) {
	return false;
}

bool LWVideoDriver::UpdateTexture1D(LWTexture *Texture, uint32_t MipmapLevel, void *Texels, uint32_t Position, uint32_t Size) {
	return false;
}

bool LWVideoDriver::UpdateTexture2D(LWTexture *Texture, uint32_t MipmapLevel, void *Texels, const LWVector2i &Position, const LWVector2i &Size) {
	return false;
}

bool LWVideoDriver::UpdateTexture3D(LWTexture *Texture, uint32_t MipmapLevel, void *Texels, const LWVector3i &Position, const LWVector3i &Size) {
	return false;
}

bool LWVideoDriver::UpdateTextureCubeMap(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Face, void *Texels, const LWVector2i &Position, const LWVector2i &Size) {
	return false;
}

bool LWVideoDriver::UpdateTexture1DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, void *Texels, uint32_t Position, uint32_t Size) {
	return false;
}

bool LWVideoDriver::UpdateTexture2DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, void *Texels, const LWVector2i &Position, const LWVector2i &Size) {
	return false;
}

bool LWVideoDriver::UpdateTextureCubeArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint32_t Face, void *Texels, const LWVector2i &Position, const LWVector2i &Size) {
	return false;
}

bool LWVideoDriver::UpdateVideoBuffer(LWVideoBuffer *VideoBuffer, const uint8_t *Buffer, uint32_t Length) {
	return false;
}

bool LWVideoDriver::DownloadTexture1D(LWTexture *Texture, uint32_t MipmapLevel, uint8_t *Buffer) {
	return false;
}

bool LWVideoDriver::DownloadTexture2D(LWTexture *Texture, uint32_t MipmapLevel, uint8_t *Buffer) {
	return false;
}

bool LWVideoDriver::DownloadTexture3D(LWTexture *Texture, uint32_t MipmapLevel, uint8_t *Buffer) {
	return false;
}

bool LWVideoDriver::DownloadTextureCubeMap(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Face, uint8_t *Buffer) {
	return false;
}

bool LWVideoDriver::DownloadTexture1DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint8_t *Buffer) {
	return false;
}

bool LWVideoDriver::DownloadTexture2DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint8_t *Buffer) {
	return false;
}

bool LWVideoDriver::DownloadTextureCubeArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint32_t Face, uint8_t *Buffer) {
	return false;
}

bool LWVideoDriver::DownloadVideoBuffer(LWVideoBuffer *VBuffer, uint8_t *Buffer, uint32_t Offset, uint32_t Length) {
	return false;
}

LWVideoDriver &LWVideoDriver::DestroyVideoBuffer(LWVideoBuffer *Buffer) {
	return *this;
}

LWVideoDriver &LWVideoDriver::DestroyFrameBuffer(LWFrameBuffer *FrameBuffer) {
	return *this;
}

LWVideoDriver &LWVideoDriver::DestroyShader(LWShader *Shader) {
	return *this;
}

LWVideoDriver &LWVideoDriver::DestroyPipeline(LWPipeline *Pipeline) {
	return *this;
}

LWVideoDriver &LWVideoDriver::DestroyTexture(LWTexture *Texture) {
	return *this;
}


