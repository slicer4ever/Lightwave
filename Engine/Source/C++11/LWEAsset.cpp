#include "LWEAsset.h"
#include <LWCore/LWUnicode.h>
#include <LWVideo/LWVideoDriver.h>
#include <LWVideo/LWFont.h>
#include <LWVideo/LWTexture.h>
#include <LWVideo/LWShader.h>
#include <LWVideo/LWImage.h>
#include <LWPlatform/LWFileStream.h>
#include <LWAudio/LWAudioStream.h>
#include <LWEVideoPlayer.h>
#include "LWEUIManager.h"
#include "LWELocalization.h"
#include "LWEXML.h"
#include <iostream>
#include <cstdlib>

#pragma region LWEAsset
//LWEAsset
void *LWEAsset::GetAsset(void) {
	return m_Asset;
}

uint32_t LWEAsset::GetType(void) const {
	return m_Type;
}

LWUTF8Iterator LWEAsset::GetAssetPath(void) const{
	return m_AssetPath;
}

void LWEAsset::Release(LWVideoDriver *Driver) {
	if (!m_Asset) return;
	if (m_Type == Font) LWAllocator::Destroy(As<LWFont>());
	else if (m_Type == Texture) Driver->DestroyTexture(As<LWTexture>());
	else if (m_Type == Shader) Driver->DestroyShader(As<LWShader>());
	else if (m_Type == Pipeline) Driver->DestroyPipeline(As<LWPipeline>());
	else if (m_Type == VideoBuffer) Driver->DestroyVideoBuffer(As<LWVideoBuffer>());
	else if (m_Type == Video) LWAllocator::Destroy(As<LWEVideoPlayer>());
	else if (m_Type == AudioStream) LWAllocator::Destroy(As<LWAudioStream>());
	m_Asset = nullptr;
	return;
}

LWEAsset::LWEAsset(uint32_t Type, void *Asset, const LWUTF8Iterator &AssetPath) : m_Type(Type), m_Asset(Asset) {
	AssetPath.Copy(m_AssetPath, sizeof(m_AssetPath));
}

LWEAsset::LWEAsset() {}

#pragma endregion

#pragma region LWEAssetManager
bool LWEAssetManager::XMLParser(LWEXMLNode *N, void *UserData, LWEXML *XML) {
	LWEAssetManager *AM = (LWEAssetManager*)UserData;
	for (LWEXMLNode *C = XML->NextNode(nullptr, N); C; C = XML->NextNode(C, N, true)) {
		uint32_t i = C->GetName().CompareList("Texture", "Font", "Shader", "Video", "AudioStream", "Pipeline", "VideoBuffer", "ShaderBuilder", "Reference");
		bool Loaded = false;
		if (i == LWEAsset::Texture) Loaded = XMLParseTexture(C, AM);
		else if (i == LWEAsset::Font) Loaded = XMLParseFont(C, AM);
		else if (i == LWEAsset::Shader) Loaded = XMLParseShader(C, AM);
		else if (i == LWEAsset::Video) Loaded = XMLParseVideo(C, AM);
		else if (i == LWEAsset::AudioStream) Loaded = XMLParseAudioStream(C, AM);
		else if (i == LWEAsset::Pipeline) Loaded = XMLParsePipeline(C, AM);
		else if (i == LWEAsset::VideoBuffer) Loaded = XMLParseVideoBuffer(C, AM);
		else if (i == 7) Loaded = XMLParseShaderBuilder(C, AM);
		else if (i == 8) {
			LWEXMLAttribute *NameAttr = C->FindAttribute("Name");
			LWEXMLAttribute *RefAttr = C->FindAttribute("Ref");
			if (NameAttr && RefAttr) Loaded = AM->InsertAssetReference(NameAttr->GetValue(), RefAttr->GetValue());
		}
		if (!Loaded) {
			fmt::print("Error unable to load asset: '{}'\n", C->m_Name);
			//return true;
		}
	}
	return true;
}

bool LWEAssetManager::XMLParseFont(LWEXMLNode *N, LWEAssetManager *AM) {
	char SBuffer[1024 * 32];
	const uint32_t MaxRanges = 32;
	LWUTF8Iterator GFirstListIters[MaxRanges];
	LWUTF8Iterator GLengthListIters[MaxRanges];
	LWAllocator &Alloc = AM->GetAllocator();
	uint32_t GFirstList[MaxRanges];
	uint32_t GLengthList[MaxRanges];
	uint32_t GlyphCount = 0;
	uint32_t Size = 48;
	LWEXMLAttribute *PathAttr = N->FindAttribute("Path");
	LWEXMLAttribute *NameAttr = N->FindAttribute("Name");
	LWEXMLAttribute *SizeAttr = N->FindAttribute("Size");
	LWEXMLAttribute *GlyphFirstAttr = N->FindAttribute("GlyphFirst");
	LWEXMLAttribute *GlyphLengthAttr = N->FindAttribute("GlyphLength");
	LWEXMLAttribute *ErrorGlyphAttr = N->FindAttribute("ErrorGlyph");
	LWELocalization *Localize = AM->GetLocalization();
	if (!PathAttr || !NameAttr) return false;
	if (SizeAttr) Size = atoi(SizeAttr->m_Value);
	if (GlyphFirstAttr && GlyphLengthAttr) {
		uint32_t FirstCnt = GlyphFirstAttr->GetValue().SplitToken(GFirstListIters, MaxRanges, '|');
		uint32_t LengthCnt = GlyphLengthAttr->GetValue().SplitToken(GLengthListIters, MaxRanges, '|');
		GlyphCount = std::min<uint32_t>(FirstCnt, LengthCnt);
		for(uint32_t i=0;i< GlyphCount;i++) {
			GFirstList[i] = atoi((const char*)GFirstListIters[i]());
			GLengthList[i] = atoi((const char*)GLengthListIters[i]());
		}
	}
	LWFont *F = nullptr;
	LWFileStream FontFile;
	LWUTF8Iterator Path = PathAttr->GetValue();
	if (Localize && Localize->ParseLocalization(SBuffer, sizeof(SBuffer), Path)) Path = LWUTF8Iterator(SBuffer);
	uint32_t ExtType = LWFileStream::IsExtensions(Path, "ttf", "fnt", "arfont");
	if (!LWFileStream::OpenStream(FontFile, Path, LWFileStream::ReadMode | ((ExtType == 0 || ExtType==2) ? LWFileStream::BinaryMode : 0), Alloc)) {
		fmt::print("Error opening font file: '{}'\n", Path);
		return false;
	}
	if (ExtType == 0) F = LWFont::LoadFontTTF(&FontFile, AM->GetDriver(), Size, GlyphCount, GFirstList, GLengthList, Alloc);
	else if (ExtType == 1) F = LWFont::LoadFontFNT(&FontFile, AM->GetDriver(), Alloc);
	else if (ExtType == 2) F = LWFont::LoadFontAR(&FontFile, AM->GetDriver(), Alloc);
	if (!F) {
		fmt::print("Error creating font file.\n");
		return false;
	}
	if (!AM->InsertAsset(NameAttr->m_Value, F, LWEAsset::Font, Path)) {
		fmt::print("Error inserting asset: '{}'\n", NameAttr->GetValue());
		LWAllocator::Destroy(F);
		return false;
	}
	if (ErrorGlyphAttr) F->SetErrorGlyph(atoi(ErrorGlyphAttr->m_Value));
	for (LWEXMLNode *C = N->m_FirstChild; C; C = C->m_Next) {
		uint32_t n = C->GetName().CompareList("GlyphName");
		if (n == 0) {
			LWEXMLAttribute *GNameAttr = C->FindAttribute("Name");
			LWEXMLAttribute *GCodeAttr = C->FindAttribute("Code");
			if (!GNameAttr || !GCodeAttr) continue;
			F->InsertGlyphName(GNameAttr->m_Value, atoi(GCodeAttr->m_Value));
		}
	}
	return true;
}

bool LWEAssetManager::XMLParseTexture(LWEXMLNode *N, LWEAssetManager *AM) {
	const uint32_t MaxFlagIters = 64;
	char SBuffer[1024 * 32];
	LWUTF8Iterator FlagIterList[MaxFlagIters];
	const uint32_t FlagValues[] = { LWTexture::MinNearestMipmapNearest, LWTexture::MinLinearMipmapNearest, LWTexture::MinNearestMipmapLinear, LWTexture::MinLinearMipmapLinear, LWTexture::MinNearest, LWTexture::MagNearest, LWTexture::WrapSClampToEdge, LWTexture::WrapTClampToEdge, LWTexture::WrapRClampToEdge, LWTexture::CompareNone, LWTexture::MinLinear, LWTexture::MagLinear, LWTexture::WrapSClampToBorder, LWTexture::WrapSMirroredRepeat, LWTexture::WrapSRepeat, LWTexture::WrapTClampToBorder, LWTexture::WrapTMirroredRepeat, LWTexture::WrapTRepeat, LWTexture::WrapRClampToBorder, LWTexture::WrapRMirroredRepeat, LWTexture::WrapRRepeat, LWTexture::RenderTarget, LWTexture::RenderBuffer };
	const char8_t FlagNames[][32] = { "MinNearestMipmapNearest",          "MinLinearMipmapNearest",          "MinNearestMipmapLinear",          "MinLinearMipmapLinear",          "MinNearest",          "MagNearest",          "WrapSClampToEdge",          "WrapTClampToEdge",          "WrapRClampToEdge",          "CompareNone",          "MinLinear",          "MagLinear",          "WrapSClampToBorder",          "WrapSMirroredRepeat",          "WrapSRepeat",          "WrapTClampToBorder",          "WrapTMirroredRepeat",          "WrapTRepeat",          "WrapRClampToBorder",          "WrapRMirroredRepeat",          "WrapRRepeat",          "RenderTarget",          "RenderBuffer" };
	const uint32_t TotalFlags = sizeof(FlagValues) / sizeof(uint32_t);
	LWVideoDriver *Driver = AM->GetDriver();
	LWAllocator &Alloc = AM->GetAllocator();

	LWELocalization *Localize = AM->GetLocalization();
	LWEXMLAttribute *PathAttr = N->FindAttribute("Path");
	LWEXMLAttribute *NameAttr = N->FindAttribute("Name");
	LWEXMLAttribute *StateAttr = N->FindAttribute("State");
	if (!PathAttr || !NameAttr) return false;
	LWUTF8Iterator Path = PathAttr->GetValue();
	if (Localize && Localize->ParseLocalization(SBuffer, sizeof(SBuffer), Path)) Path = LWUTF8Iterator(SBuffer);
	LWImage Image;
	if (!LWImage::LoadImage(Image, Path, AM->GetAllocator())) {
		fmt::print("Error loading image: '{}'\n", Path);
		return false;
	}
	uint32_t TextureState = 0;
	if (StateAttr) {
		uint32_t StateCnt = std::min<uint32_t>(StateAttr->GetValue().SplitToken(FlagIterList, MaxFlagIters, '|'), MaxFlagIters);
		for (uint32_t i = 0; i < StateCnt; i++) {
			uint32_t FlagID = FlagIterList[i].AdvanceWord(true).CompareLista(TotalFlags, FlagNames);
			if (FlagID == -1) fmt::print("Encountered invalid flag: '{}' for '{}'\n", FlagIterList[i], NameAttr->GetValue());
			else TextureState |= FlagValues[FlagID];
		}
	}
	LWTexture *Tex = Driver->CreateTexture(TextureState, Image, Alloc);
	if (!AM->InsertAsset(NameAttr->m_Value, Tex, LWEAsset::Texture, Path)) {
		fmt::print("Error inserting asset: '{}'\n", NameAttr->GetValue());
		Driver->DestroyTexture(Tex);
		return false;
	}
	return true;
}

bool LWEAssetManager::XMLParsePipeline(LWEXMLNode *N, LWEAssetManager *AM) {
	const uint32_t MaxFlagIters = 64;
	const char8_t *FlagNames[] = { "DepthTest", "Blending", "NoDepth", "NoColorR", "NoColorG", "NoColorB", "NoColorA", "NoColor", "ClipPlane0", "ClipPlane1", "ClipPlane2", "StencilTest", "DepthBias" };
	const uint64_t FlagValues[] = { LWPipeline::DEPTH_TEST, LWPipeline::BLENDING, LWPipeline::No_Depth, LWPipeline::No_ColorR, LWPipeline::No_ColorG, LWPipeline::No_ColorB, LWPipeline::No_ColorA, LWPipeline::No_Color, LWPipeline::CLIPPLANE0, LWPipeline::CLIPPLANE1, LWPipeline::CLIPPLANE2, LWPipeline::STENCIL_TEST, LWPipeline::DEPTH_BIAS };
	const char8_t *FillNames[] = { "Solid", "Wireframe" };
	const uint64_t FillValues[] = { LWPipeline::SOLID, LWPipeline::WIREFRAME };
	const char8_t *CullNames[] = { "None", "CCW", "CW" };
	const uint64_t CullValues[] = { LWPipeline::CULL_NONE, LWPipeline::CULL_CCW, LWPipeline::CULL_CW };
	const char8_t *BlendNames[] = { "Zero", "One", "SrcColor", "DstColor", "SrcAlpha", "DstAlpha", "OneMinusSrcColor", "OneMinusDstColor", "OneMinusSrcAlpha", "OneMinusDstAlpha" };
	const uint64_t BlendValues[] = { LWPipeline::BLEND_ZERO, LWPipeline::BLEND_ONE, LWPipeline::BLEND_SRC_COLOR, LWPipeline::BLEND_DST_COLOR, LWPipeline::BLEND_SRC_ALPHA, LWPipeline::BLEND_DST_ALPHA, LWPipeline::BLEND_ONE_MINUS_SRC_COLOR, LWPipeline::BLEND_ONE_MINUS_DST_COLOR, LWPipeline::BLEND_ONE_MINUS_SRC_ALPHA, LWPipeline::BLEND_ONE_MINUS_DST_ALPHA };
	const char8_t *CompareNames[] = { "Always", "Never", "Less", "Greater", "LessEqual", "GreaterEqual" };
	const uint64_t CompareValues[] = { LWPipeline::ALWAYS, LWPipeline::NEVER, LWPipeline::LESS, LWPipeline::GREATER, LWPipeline::LESS_EQL, LWPipeline::GREATER_EQL };
	const char8_t *StencilOpNames[] = { "Keep", "Zero", "Replace", "Increase", "Decrease", "IncreaseWrap", "DecreaseWrap", "Invert" };
	const uint64_t StencilOpValues[] = { LWPipeline::STENCIL_OP_KEEP, LWPipeline::STENCIL_OP_ZERO, LWPipeline::STENCIL_OP_REPLACE, LWPipeline::STENCIL_OP_INCR, LWPipeline::STENCIL_OP_DECR, LWPipeline::STENCIL_OP_INCR_WRAP, LWPipeline::STENCIL_OP_DECR_WRAP, LWPipeline::STENCIL_OP_INVERT };
	const uint32_t FlagCount = sizeof(FlagValues) / sizeof(uint64_t);
	LWUTF8Iterator FlagIterList[MaxFlagIters];

	auto ParseBlockBinding = [](LWEXMLNode *N, LWPipeline *P, LWEAssetManager *AM) {
		LWEXMLAttribute *NameAttr = N->FindAttribute("Name");
		LWEXMLAttribute *OffsetAttr = N->FindAttribute("Offset");
		LWEXMLAttribute *SlotAttr = N->FindAttribute("Slot");
		LWEXMLAttribute *SlotNameAttr = N->FindAttribute("SlotName");
		if (!NameAttr || (!SlotAttr && !SlotNameAttr)) {
			fmt::print("Block '{}' does not have required parameters.\n", N->GetName());
			return;
		}
		LWVideoBuffer *B = AM->GetAsset<LWVideoBuffer>(NameAttr->GetValue());
		if (!B) {
			fmt::print("Error block '{}' could not find buffer: '{}'\n", N->GetName(), NameAttr->GetValue());
			return;
		}
		uint32_t SlotIdx = -1;
		if (SlotAttr) SlotIdx = atoi(SlotAttr->m_Value);
		else if (SlotNameAttr) SlotIdx = P->FindBlock(SlotNameAttr->m_Value);
		if (SlotIdx == -1) {
			fmt::print("Error block '{}' slot '{}' not found.\n", N->GetName(), SlotAttr ? SlotAttr->GetValue() : SlotNameAttr->GetValue());
			return;
		}
		uint32_t Offset = OffsetAttr ? atoi(OffsetAttr->m_Value) : 0;
		P->SetUniformBlock(SlotIdx, B, Offset);
		return;
	};

	auto ParseResourceBinding = [](LWEXMLNode *N, LWPipeline *P, LWEAssetManager *AM) {
		LWEXMLAttribute *NameAttr = N->FindAttribute("Name");
		LWEXMLAttribute *OffsetAttr = N->FindAttribute("Offset");
		LWEXMLAttribute *SlotAttr = N->FindAttribute("Slot");
		LWEXMLAttribute *SlotNameAttr = N->FindAttribute("SlotName");
		if (!NameAttr || (!SlotAttr && !SlotNameAttr)) {
			fmt::print("Block '{}' does not have required parameters.\n", N->GetName());
			return;
		}
		LWVideoBuffer *B = AM->GetAsset<LWVideoBuffer>(NameAttr->m_Value);
		LWTexture *T = AM->GetAsset<LWTexture>(NameAttr->m_Value);
		if (!B && !T) {
			fmt::print("Error block '{}' could not find buffer or texture: '{}'\n", N->GetName(), NameAttr->GetValue());
			return;
		}
		uint32_t SlotIdx = -1;
		if (SlotAttr) SlotIdx = atoi(SlotAttr->m_Value);
		else if (SlotNameAttr) SlotIdx = P->FindResource(SlotNameAttr->m_Value);
		if (SlotIdx == -1) {
			fmt::print("Error block '{}' resource slot '{}' not found.\n", N->GetName(), SlotAttr ? SlotAttr->GetValue() : SlotNameAttr->GetValue());
			return;
		}
		uint32_t Offset = OffsetAttr ? atoi(OffsetAttr->m_Value) : 0;
		if (B) P->SetResource(SlotIdx, B, Offset);
		else P->SetResource(SlotIdx, T);
		return;
	};
	LWVideoDriver *Driver = AM->GetDriver();
	LWAllocator &Alloc = AM->GetAllocator();

	LWEXMLAttribute *NameAttr = N->FindAttribute("Name");
	LWEXMLAttribute *VertexAttr = N->FindAttribute("Vertex");
	LWEXMLAttribute *PixelAttr = N->FindAttribute("Pixel");
	LWEXMLAttribute *GeometryAttr = N->FindAttribute("Geometry");
	LWEXMLAttribute *ComputeAttr = N->FindAttribute("Compute");
	LWEXMLAttribute *FlagAttr = N->FindAttribute("Flags");
	LWEXMLAttribute *FillModeAttr = N->FindAttribute("FillMode");
	LWEXMLAttribute *CullModeAttr = N->FindAttribute("CullMode");
	LWEXMLAttribute *DepthCompareAttr = N->FindAttribute("DepthCompare");
	LWEXMLAttribute *SourceBlendModeAttr = N->FindAttribute("SourceBlendMode");
	LWEXMLAttribute *DestBlendModeAttr = N->FindAttribute("DestBlendMode");
	LWEXMLAttribute *StencilCompareAttr = N->FindAttribute("StencilCompare");
	LWEXMLAttribute *StencilRefValueAttr = N->FindAttribute("StencilValue");
	LWEXMLAttribute *StencilReadMaskValueAttr = N->FindAttribute("StencilReadMask");
	LWEXMLAttribute *StencilWriteMaskValueAttr = N->FindAttribute("StencilWriteMask");
	LWEXMLAttribute *StencilOpFailAttr = N->FindAttribute("StencilOpFail");
	LWEXMLAttribute *StencilOpDFailAttr = N->FindAttribute("StencilOpDepthFail");
	LWEXMLAttribute *StencilOpPassAttr = N->FindAttribute("StencilOpPass");
	LWEXMLAttribute *DepthBiasAttr = N->FindAttribute("DepthBias");
	LWEXMLAttribute *DepthSlopedBiasAttr = N->FindAttribute("DepthSlopedBias");
	if (!NameAttr){
		fmt::print("Error pipeline has no name attribute.\n");
		return false;
	}
	if (!VertexAttr && !ComputeAttr) {
		fmt::print("Pipeline '{}' must have either a vertex shader, or compute shader.\n", NameAttr->GetValue());
		return false;
	}
	uint64_t Flags = 0;

	LWShader *VS = VertexAttr ? AM->GetAsset<LWShader>(VertexAttr->GetValue()) : nullptr;
	LWShader *PS = PixelAttr ? AM->GetAsset<LWShader>(PixelAttr->GetValue()) : nullptr;
	LWShader *GS = GeometryAttr ? AM->GetAsset<LWShader>(GeometryAttr->GetValue()) : nullptr;
	LWShader *CS = ComputeAttr ? AM->GetAsset<LWShader>(ComputeAttr->GetValue()) : nullptr;
	if (VertexAttr && !VS) {
		fmt::print("Could not find vertex shader: '{}'\n", VertexAttr->GetValue());
		return false;
	}
	if (PixelAttr && !PS) {
		fmt::print("Could not find pixel shader: '{}'\n", PixelAttr->GetValue());
		return false;
	}
	if (GeometryAttr && !GS) {
		fmt::print("Could not find geometry shader: '{}'\n", GeometryAttr->GetValue());
		return false;
	}
	if (ComputeAttr && !CS) {
		fmt::print("Could not find computer shader: '{}'\n", ComputeAttr->GetValue());
		return false;
	}
	if (FlagAttr) {
		uint32_t FlagCnt = std::min<uint32_t>(FlagAttr->GetValue().SplitToken(FlagIterList, MaxFlagIters, '|'), MaxFlagIters);
		for (uint32_t i = 0; i < FlagCnt; i++) {
			uint32_t n = FlagIterList[i].AdvanceWord(true).CompareLista(FlagCount, FlagNames);
			if (n == -1) {
				fmt::print("Invalid flag encountered: {}: '{}' for: '{}'\n", i, FlagIterList[i], NameAttr->GetValue());
			} else Flags |= FlagValues[n];
		}
	}
	if (FillModeAttr) {
		uint32_t n = FillModeAttr->GetValue().CompareLista(2, FillNames);
		if (n == -1) {
			fmt::print("Unknown fill mode: '{}'\n", FillModeAttr->GetValue());
		} else Flags |= (FillValues[n] << LWPipeline::FILL_MODE_BITOFFSET);
	}
	if (CullModeAttr) {
		uint32_t n = CullModeAttr->GetValue().CompareLista(3, CullNames);
		if (n == -1) {
			fmt::print("Unknown cull mode: '{}'\n", CullModeAttr->GetValue());
		} else Flags |= (CullValues[n] << LWPipeline::CULL_BITOFFSET);
	}
	if (DepthCompareAttr) {
		uint32_t n = DepthCompareAttr->GetValue().CompareLista(6, CompareNames);
		if (n == -1) {
			fmt::print("Unknown depth compare mode: '{}'\n", DepthCompareAttr->GetValue());
		} else Flags |= (CompareValues[n] << LWPipeline::DEPTH_COMPARE_BITOFFSET);
	}
	if (SourceBlendModeAttr) {
		uint32_t n = SourceBlendModeAttr->GetValue().CompareLista(10, BlendNames);
		if (n == -1) {
			fmt::print("Unknown source blend mode: '{}'\n", SourceBlendModeAttr->GetValue());
		} else Flags |= (BlendValues[n] << LWPipeline::BLEND_SRC_BITOFFSET);
	}
	if (DestBlendModeAttr) {
		uint32_t n = DestBlendModeAttr->GetValue().CompareLista(10, BlendNames);
		if (n == -1) {
			fmt::print("Unknown dest blend mode: '{}'\n", DestBlendModeAttr->GetValue());
		} else Flags |= (BlendValues[n] << LWPipeline::BLEND_DST_BITOFFSET);
	}
	if (StencilCompareAttr) {
		uint32_t n = StencilCompareAttr->GetValue().CompareLista(6, CompareNames);
		if (n == -1) {
			fmt::print("Unknown stencil compare mode: '{}'\n", StencilCompareAttr->GetValue());
		} else Flags |= (CompareValues[n] << LWPipeline::STENCIL_COMPARE_BITOFFSET);
	}
	if (StencilOpFailAttr) {
		uint32_t n = StencilOpFailAttr->GetValue().CompareLista(8, StencilOpNames);
		if (n == -1) {
			fmt::print("Unknown stencil op fail: '{}'\n", StencilOpFailAttr->GetValue());
		} else Flags |= (StencilOpValues[n] << LWPipeline::STENCIL_OP_SFAIL_BITOFFSET);
	}
	if (StencilOpDFailAttr) {
		uint32_t n = StencilOpDFailAttr->GetValue().CompareLista(8, StencilOpNames);
		if (n == -1) {
			fmt::print("Unknown stencil op depth fail: '{}'\n", StencilOpDFailAttr->GetValue());
		} else Flags |= (StencilOpValues[n] << LWPipeline::STENCIL_OP_DFAIL_BITOFFSET);
	}
	if (StencilOpPassAttr) {
		uint32_t n = StencilOpPassAttr->GetValue().CompareLista(8, StencilOpNames);
		if (n == -1) {
			fmt::print("Unknown stencil op pass: '{}'\n", StencilOpPassAttr->GetValue());
		} else Flags |= (StencilOpValues[n] << LWPipeline::STENCIL_OP_PASS_BITOFFSET);
	}
	if (StencilRefValueAttr) {
		uint64_t Value = (uint64_t)atoi(StencilRefValueAttr->m_Value);
		Value = std::max<uint64_t>(Value, 255);
		Flags |= (Value << LWPipeline::STENCIL_REF_VALUE_BITOFFSET);
	}
	if (StencilReadMaskValueAttr) {
		uint64_t Value = (uint64_t)atoi(StencilReadMaskValueAttr->m_Value);
		Value = std::max<uint64_t>(Value, 255);
		Flags |= (Value << LWPipeline::STENCIL_READMASK_BITOFFSET);
	}
	if (StencilWriteMaskValueAttr) {
		uint64_t Value = (uint64_t)atoi(StencilWriteMaskValueAttr->m_Value);
		Value = std::max<uint64_t>(Value, 255);
		Flags |= (Value << LWPipeline::STENCIL_WRITEMASK_BITOFFSET);
	}
	LWPipeline *P = nullptr;
	if (VS) P = Driver->CreatePipeline(VS, GS, PS, Flags, Alloc);
	else if (CS) P = Driver->CreatePipeline(CS, Alloc);
	if (!P) {
		fmt::print("Failed to create pipeline for: '{}'\n", NameAttr->GetValue());
		return false;
	}
	
	P->SetDepthBias((P->GetFlag()&LWPipeline::DEPTH_BIAS), DepthBiasAttr ? (float)atof(DepthBiasAttr->m_Value) : 0.0f, DepthSlopedBiasAttr ? (float)atof(DepthSlopedBiasAttr->m_Value) : 0.0f);
	for (LWEXMLNode *C = N->m_FirstChild; C; C = C->m_Next) {
		uint32_t i = C->GetName().CompareList("Resource", "Block");
		if (i == 0) ParseResourceBinding(C, P, AM);
		else if (i == 1) ParseBlockBinding(C, P, AM);
		else{
			fmt::print("Error pipeline '{}' has unknown child node: '{}'\n", NameAttr->GetValue(), C->GetName());
		}
	}

	if (!AM->InsertAsset(NameAttr->GetValue(), P, LWEAsset::Pipeline, "")) {
		fmt::print("Error inserting asset: '{}'\n", NameAttr->GetValue());
		Driver->DestroyPipeline(P);
		return false;
	}
	return true;
}

bool LWEAssetManager::XMLParseShader(LWEXMLNode *N, LWEAssetManager *AM) {
	const char *DriverNames[] = LWVIDEODRIVER_NAMES;
	const char *TypeNames[] = { "Vertex", "Pixel", "Geometry", "Compute" };
	const char *CompiledNames[] = { "lwcv", "lwcp", "lwcg", "lwcc" };
	const uint32_t MaxFileBufferLen = 64 * 1024;
	const uint32_t MaxBufferLen = 1024;
	const uint32_t MaxModules = 5;
	const uint32_t MaxNameLen = 32;
	const uint32_t MaxPathLen = 256;
	const uint32_t MaxDefines = 32;
	const uint32_t MaxErrorBufferLen = 4 * 1024;
	const uint32_t TotalDriverCnt = 10;
	LWUTF8Iterator::C_View<MaxBufferLen> Buffer;
	LWUTF8Iterator::C_View<MaxBufferLen> CBuffer;
	LWUTF8Iterator AssetPath;
	
	char CompiledBuffer[MaxFileBufferLen];
	char ErrorBuffer[MaxErrorBufferLen]="";
	LWUTF8Iterator DefineIterList[MaxDefines];

	uint32_t CompiledLen = 0;
	uint32_t DefineCount = 0;
	LWVideoDriver *Driver = AM->GetDriver();
	uint32_t DriverID = Driver->GetDriverID();
	LWAllocator &Allocator = AM->GetAllocator();
	LWShader *Res = nullptr;

	auto ParseInputMap = [](LWEXMLNode *N, LWShader *S, LWEAssetManager *AM) {
		//                                                       Float,      Int,        UInt,       Double,     Vec2,       Vec3,       Vec4,       uVec2,      uVec3,      uVec4,      iVec2,      iVec3,      iVec4,      dVec2,      dVec3,      dVec4
		const uint32_t TypeNameHashs[LWShaderInput::Count] = { 0x4c816225, 0xf87415fe, 0xe939eb21, 0x8e464c28, 0x2c3c5815, 0x2b3c5682, 0x263c4ea3, 0x1a199b30, 0x1b199cc3, 0x2019a4a2, 0x5d3f3cc4, 0x5e3f3e57, 0x5b3f399e, 0xbfb0ee5f, 0xbeb0eccc, 0xbdb0eb39 };
		LWShaderInput Inputs[LWShader::MaxInputs];
		uint32_t InputCount = 0;
		for (uint32_t i = 0; i < N->m_AttributeCount; i++) {
			char8_t TypeBuffer[256]="";
			LWEXMLAttribute &A = N->m_Attributes[i];
			uint32_t NameHash = A.GetName().Hash();
			uint32_t Length = 1;
			sscanf(A.m_Value, "%[^[][%d]", TypeBuffer, &Length);
			uint32_t TypeHash = LWUTF8Iterator(TypeBuffer).Hash();
			uint32_t T = 0;
			for (; T < LWShaderInput::Count && TypeNameHashs[T] != TypeHash; T++) {}
			if (T >= LWShaderInput::Count) {
				fmt::print("Error unknown type: '{}' for input: '{}'\n", A.GetValue(), A.GetName());
				continue;
			}
			if (Length > 1) {
				for (uint32_t n = 0; n < Length; n++) Inputs[InputCount++] = LWShaderInput(LWUTF8Iterator::C_View<256>("{}[{}]", A.GetName(), n), T, 1);
			} else {
				Inputs[InputCount++] = LWShaderInput(A.m_Name, T, Length);
			}
		}
		S->SetInputMap(InputCount, Inputs);
	};

	auto ParseResourceMap = [](LWEXMLNode *N, LWShader *S, LWEAssetManager *AM) {
		uint32_t ResourceNameHashs[LWShader::MaxResources];
		uint32_t Count = 0;
		for (uint32_t i = 0; i < N->m_AttributeCount; i++) {
			ResourceNameHashs[Count++] = N->m_Attributes[i].GetName().Hash();
		}
		S->SetResourceMap(Count, ResourceNameHashs);
		return;
	};

	auto ParseBlockMap = [](LWEXMLNode *N, LWShader *S, LWEAssetManager *AM) {
		uint32_t BlockNameHashs[LWShader::MaxBlocks];
		uint32_t Count = 0;
		for (uint32_t i = 0; i < N->m_AttributeCount; i++) {
			BlockNameHashs[Count++] = N->m_Attributes[i].GetName().Hash();
		}
		S->SetBlockMap(Count, BlockNameHashs);
		return;
	};

	std::function<LWUTF8Iterator(const LWUTF8Iterator &, uint32_t &, uint64_t &, LWAllocator &, LWFileStream *)> ParseSourceFunc;

	auto ParsePath = [&ParseSourceFunc](const LWUTF8Iterator &Path, uint32_t &Length, uint64_t &ModifiedTime, LWAllocator &Allocator, LWFileStream *ExistingStream)->LWUTF8Iterator {
		LWFileStream Stream;
		if (!LWFileStream::OpenStream(Stream, Path, LWFileStream::ReadMode | LWFileStream::BinaryMode, Allocator, ExistingStream)) {
			fmt::print("Error opening file: '{}'\n", Path);
			return LWUTF8Iterator();
		}
		ModifiedTime = std::max<uint64_t>(ModifiedTime, Stream.GetModifiedTime());
		Length = Stream.Length()+1;
		char8_t *Buffer = Allocator.Allocate<char8_t>(Length);
		Stream.ReadText(Buffer, Length);
		LWUTF8Iterator Res = ParseSourceFunc(Buffer, Length, ModifiedTime, Allocator, &Stream);
		if (Res() != Buffer) LWAllocator::Destroy(Buffer);
		return Res;
	};

	auto ParseSource = [&ParsePath](const LWUTF8Iterator &Source, uint32_t &Length, uint64_t &ModifiedTime, LWAllocator &Allocator, LWFileStream *ExistingStream)->LWUTF8Iterator {
		LWUTF8Iterator P = Source;
		LWUTF8Iterator C = Source;
		for(;!C.AtEnd();++C) {
			if(*C=='#') {
				if(C.Compare("#include", 8)) {
					LWUTF8Iterator N = C.NextLine();
					LWUTF8Iterator Line = LWUTF8Iterator(C+8, N);
					LWUTF8Iterator OpenToken = Line.NextTokens("<\"", false);
					if(OpenToken.AtEnd()) continue;
					LWUTF8Iterator CloseToken = OpenToken.NextToken(*C == '<' ? '>' : '\"');
					if(CloseToken.AtEnd()) continue;
					LWUTF8Iterator Path = LWUTF8Iterator(OpenToken + 1, CloseToken);
					uint32_t Len = 0;
					LWUTF8Iterator SubSource = ParsePath(Path, Len, ModifiedTime, Allocator, ExistingStream);
					if(!SubSource.isInitialized()) continue;
					uint32_t NewLen = (Len + Length - C.RawDistance(--N)) - 1; //-1 because both Len, and Length include a null character.
					char8_t *Buf = Allocator.Allocate<char8_t>(NewLen);
					uint32_t o = P.Copy(Buf, NewLen, C) - 1;
					o += SubSource.Copy(Buf + o, NewLen - o) - 1;
					o += N.Copy(Buf + o, NewLen - o);
					if (o != NewLen) {
						fmt::print("Something went wrong copying, Expected: {} Got: {}\n", NewLen, o);
					}
					LWAllocator::Destroy(SubSource());
					if (P != Source) LWAllocator::Destroy(P());
					C = LWUTF8Iterator(Buf + C.RawIndex(), NewLen - C.RawIndex());
					P = LWUTF8Iterator(Buf, NewLen);
					Length = NewLen;
				}
			}
		}
		return P;
	};
	ParseSourceFunc = ParseSource;

	LWEXMLAttribute *TypeAttr = N->FindAttribute("Type");
	LWEXMLAttribute *PathAttr = N->FindAttribute("Path");
	LWEXMLAttribute *CompiledPathAttr = N->FindAttribute("CompiledPath");
	LWEXMLAttribute *NameAttr = N->FindAttribute("Name");
	LWEXMLAttribute *AutoCompileAttr = N->FindAttribute("AutoCompile");
	LWELocalization *Localize = AM->GetLocalization();
	if ((!PathAttr && !CompiledPathAttr) || !TypeAttr || !NameAttr) {
		fmt::print("Shader Parser missing required parameters.\n");
		return false;
	}

	for (uint32_t i = 0; i < N->m_AttributeCount && DefineCount<MaxDefines; i++) {
		LWEXMLAttribute *Attr = &N->m_Attributes[i];
		if(Attr==TypeAttr || Attr==PathAttr || Attr==CompiledPathAttr || Attr==NameAttr || Attr==AutoCompileAttr) continue;
		DefineIterList[DefineCount++] = Attr->GetName();
		DefineIterList[DefineCount++] = Attr->GetValue();
	}

	LWUTF8Iterator Path = PathAttr ? PathAttr->GetValue() : LWUTF8Iterator();
	LWUTF8Iterator CPath = CompiledPathAttr ? CompiledPathAttr->GetValue() : LWUTF8Iterator();
	if (Localize) {
		if (PathAttr && Localize->ParseLocalization(Buffer.m_Data, sizeof(Buffer.m_Data), Path)) Path = Buffer;
		if (CompiledPathAttr && Localize->ParseLocalization(CBuffer.m_Data, sizeof(CBuffer.m_Data), CPath)) CPath = CBuffer;
	}
	uint32_t Type = TypeAttr->GetValue().CompareList("Vertex", "Pixel", "Geometry", "Compute");
	if (Type == -1) {
		fmt::print("Unknown type: '{}' for shader: '{}'\n", TypeAttr->m_Value, NameAttr->m_Value);
		return false;
	}
	if (CompiledPathAttr) {
		CBuffer = LWUTF8Iterator::C_View<MaxBufferLen>("{}.{}{}", CPath, CompiledNames[Type], DriverNames[DriverID]);
		CPath = CBuffer;
	}
	uint32_t n = Path.CompareList("FontVertex", "FontColor", "FontMSDF", "UIVertex", "UITexture", "UIColor", "UIYUVTexture");
	LWUTF8Iterator Source;
	LWUTF8Iterator DelSource;
	uint64_t ModifiedTime = 0;

	if (n == -1) {
		LWFileStream CStream;
		//Open both streams and if Path source is newer than the compiled path, then recompile the source.
		if (CompiledPathAttr) {
			if (!LWFileStream::OpenStream(CStream, CPath, LWFileStream::ReadMode | LWFileStream::BinaryMode, Allocator)) {
				if (!PathAttr) {
					fmt::print("Could not open compiled shader at: '{}' for: '{}'\n", CPath, NameAttr->GetValue());
					return false;
				}
				CompiledPathAttr = nullptr;
			}
		}
		if(PathAttr) {
			uint32_t Len = 0;
			Source = DelSource = ParsePath(Path, Len, ModifiedTime, Allocator, nullptr);
			if (Source.isInitialized()) {
				if (ModifiedTime > CStream.GetModifiedTime()) CompiledPathAttr = nullptr; //Source is newer than compiled.
			}
		}
		if (CompiledPathAttr) {
			AssetPath = CPath;
			CompiledLen = CStream.Read(CompiledBuffer, sizeof(CompiledBuffer));
			Res = Driver->CreateShaderCompiled(Type, CompiledBuffer, CompiledLen, Allocator, ErrorBuffer, sizeof(ErrorBuffer));
		} else if (PathAttr) {
			AssetPath = Path;
			if (AutoCompileAttr) {
				CompiledLen = sizeof(CompiledBuffer);
				Res = Driver->ParseShader(Type, Source, Allocator, DefineCount, DefineIterList, CompiledBuffer, ErrorBuffer, CompiledLen, sizeof(ErrorBuffer));
				if (CompiledLen && CPath.isInitialized()) {
					CStream.Finished();
					if (LWFileStream::OpenStream(CStream, CBuffer, LWFileStream::WriteMode | LWFileStream::BinaryMode, Allocator)) {
						CStream.Write(CompiledBuffer, CompiledLen);
						CStream.Finished();
					}
				}
			} else Res = Driver->ParseShader(Type, Source, Allocator, DefineCount, DefineIterList, nullptr, ErrorBuffer, CompiledLen, sizeof(ErrorBuffer));
		}else {
			fmt::print("No valid path for shader: '{}'\n", NameAttr->GetValue());
			return false;
		}
	} else {
		AssetPath = Path;
		if (n == 0) Res = Driver->ParseShader(Type, LWFont::GetVertexShaderSource(), Allocator, DefineCount, DefineIterList, nullptr, ErrorBuffer, CompiledLen, sizeof(ErrorBuffer));
		else if (n == 1) Res = Driver->ParseShader(Type, LWFont::GetPixelColorShaderSource(), Allocator, DefineCount, DefineIterList, nullptr, ErrorBuffer, CompiledLen, sizeof(ErrorBuffer));
		else if (n == 2) Res = Driver->ParseShader(Type, LWFont::GetPixelMSDFShaderSource(), Allocator, DefineCount, DefineIterList, nullptr, ErrorBuffer, CompiledLen, sizeof(ErrorBuffer));
		else if (n == 3) Res = Driver->ParseShader(Type, LWEUIManager::GetVertexShaderSource(), Allocator, DefineCount, DefineIterList, nullptr, ErrorBuffer, CompiledLen, sizeof(ErrorBuffer));
		else if (n == 4) Res = Driver->ParseShader(Type, LWEUIManager::GetTextureShaderSource(), Allocator, DefineCount, DefineIterList, nullptr, ErrorBuffer, CompiledLen, sizeof(ErrorBuffer));
		else if (n == 5) Res = Driver->ParseShader(Type, LWEUIManager::GetColorShaderSource(), Allocator, DefineCount, DefineIterList, nullptr, ErrorBuffer, CompiledLen, sizeof(ErrorBuffer));
		else if (n == 6) Res = Driver->ParseShader(Type, LWEUIManager::GetYUVTextureShaderSource(), Allocator, DefineCount, DefineIterList, nullptr, ErrorBuffer, CompiledLen, sizeof(ErrorBuffer));
		if (Res) Res->SetInputMapList("Position", LWShaderInput::Vec4, 1, "Color", LWShaderInput::Vec4, 1, "TexCoord", LWShaderInput::Vec4, 1);
	}
	LWAllocator::Destroy(DelSource());
	if (!Res) {
		fmt::print("Error creating shader '{}': {}\n", NameAttr->GetValue(), ErrorBuffer);
		return false;
	}
	for (LWEXMLNode *C = N->m_FirstChild; C; C = C->m_Next) {
		uint32_t i = C->GetName().CompareList("InputMap", "ResourceMap", "BlockMap");
		if (i == 0) ParseInputMap(C, Res, AM);
		else if (i == 1) ParseResourceMap(C, Res, AM);
		else if (i == 2) ParseBlockMap(C, Res, AM);
	}
	if (!AM->InsertAsset(NameAttr->m_Value, Res, LWEAsset::Shader, AssetPath)) {
		fmt::print("Error inserting asset: '{}'\n", NameAttr->GetValue());
		Driver->DestroyShader(Res);
		return false;
	}
	return true;
}

bool LWEAssetManager::XMLParseShaderBuilder(LWEXMLNode *N, LWEAssetManager *AM) {
	const char *TypeNames[] = { "Vertex", "Pixel", "Geometry", "Compute" };
	const uint32_t MaxDefines = 32;
	LWUTF8Iterator DefineIterList[MaxDefines];
	LWShaderInput InputList[LWShader::MaxInputs];
	uint32_t ResourceList[LWShader::MaxResources];
	uint32_t BlockList[LWShader::MaxBlocks];
	uint32_t InputCount = 0;
	uint32_t ResoruceCount = 0;
	uint32_t BlockCount = 0;
	LWVideoDriver *Driver = AM->GetDriver();
	LWAllocator &Alloc = AM->GetAllocator();

	LWEXMLAttribute *PathAttr = N->FindAttribute("Path");
	if (!PathAttr) {
		fmt::print("Error '{}': no Path attribute specified.\n", N->GetName());
		return false;
	}

	auto BuildDefaultInputMap = [](uint32_t Cnt, LWShaderInput *InputList, ...)->uint32_t{
		va_list lst;
		va_start(lst, InputList);
		for (uint32_t i = 0; i < Cnt; i++) {
			const char *Name = va_arg(lst, const char*);
			uint32_t Type = va_arg(lst, uint32_t);
			uint32_t Length = va_arg(lst, uint32_t);
			InputList[i] = LWShaderInput(Name, Type, Length);
		}
		va_end(lst);
		return Cnt;
	};

	auto ParseInputMap = [](LWEXMLNode *N, LWShaderInput *InputList, LWEAssetManager *AM)->uint32_t {
		//                                                       Float,      Int,        UInt,       Double,     Vec2,       Vec3,       Vec4,       uVec2,      uVec3,      uVec4,      iVec2,      iVec3,      iVec4,      dVec2,      dVec3,      dVec4
		const uint32_t TypeNameHashs[LWShaderInput::Count] = { 0x4c816225, 0xf87415fe, 0xe939eb21, 0x8e464c28, 0x2c3c5815, 0x2b3c5682, 0x263c4ea3, 0x1a199b30, 0x1b199cc3, 0x2019a4a2, 0x5d3f3cc4, 0x5e3f3e57, 0x5b3f399e, 0xbfb0ee5f, 0xbeb0eccc, 0xbdb0eb39 };
		uint32_t Cnt = 0;
		for (uint32_t i = 0; i < N->m_AttributeCount; i++) {
			char8_t TypeBuffer[256];
			LWEXMLAttribute &A = N->m_Attributes[i];
			uint32_t NameHash = A.GetName().Hash();
			uint32_t Length = 1;
			sscanf(A.m_Value, "%[^[][%d]", TypeBuffer, &Length);
			uint32_t TypeHash = LWUTF8Iterator(TypeBuffer).Hash();
			uint32_t T = 0;
			for (; T < LWShaderInput::Count && TypeNameHashs[T] != TypeHash; T++) {}
			if (T >= LWShaderInput::Count) {
				fmt::print("Error unknown type '{}' for input: '{}'\n", A.GetValue(), A.GetName());
				continue;
			}
			if (Length > 1) {
				for (uint32_t n = 0; n < Length; n++) {
					InputList[Cnt++] = LWShaderInput(LWUTF8Iterator::C_View<256>("{}[{}]", A.m_Name, n), T, 1);
				}
			} else {
				InputList[Cnt++] = LWShaderInput(A.GetName(), T, Length);
			}
		}
		return Cnt;
	};

	auto ParseResourceMap = [](LWEXMLNode *N, uint32_t *HashNameList, LWEAssetManager *AM)->uint32_t {
		uint32_t Count = 0;
		for (uint32_t i = 0; i < N->m_AttributeCount; i++) {
			HashNameList[Count++] = N->m_Attributes[i].GetName().Hash();
		}
		return Count;
	};

	auto ParseBlockMap = [](LWEXMLNode *N, uint32_t *BlockNameList, LWEAssetManager *AM)->uint32_t {
		uint32_t Count = 0;
		for (uint32_t i = 0; i < N->m_AttributeCount; i++) {
			BlockNameList[Count++] = N->m_Attributes[i].GetName().Hash();
		}
		return Count;
	};

	auto ParseShader = [&DefineIterList, &MaxDefines, &ParseInputMap, &ParseResourceMap, &ParseBlockMap, &InputList, &InputCount, &ResourceList, &ResoruceCount, &BlockList, &BlockCount, &Driver, &Alloc](LWEXMLNode *N, const LWUTF8Iterator &Source, const LWUTF8Iterator &SourcePath, uint64_t SourcesModifiedTime, LWEAssetManager *AM) ->bool {
		const uint32_t MaxBufferLength = 1024 * 64;//64KB.
		const uint32_t MaxNameLen = 256;
		const char *CompiledNames[] = { "lwcv", "lwcp", "lwcg", "lwcc" };
		const char *DriverNames[] = LWVIDEODRIVER_NAMES;
		char CompiledBuffer[MaxBufferLength];
		char ErrorBuffer[MaxBufferLength] = "";
		LWUTF8Iterator::C_View<MaxNameLen> CBuffer;
		LWUTF8Iterator AssetPath = SourcePath;
		LWShaderInput SInputList[LWShader::MaxInputs];
		uint32_t SResourceList[LWShader::MaxResources];
		uint32_t SBlockList[LWShader::MaxBlocks];
		LWEXMLAttribute *TypeAttr = N->FindAttribute("Type");
		LWEXMLAttribute *CompiledPathAttr = N->FindAttribute("CompiledPath");
		LWEXMLAttribute *NameAttr = N->FindAttribute("Name");
		LWEXMLAttribute *AutoCompileAttr = N->FindAttribute("AutoCompile");
		LWELocalization *Localize = AM->GetLocalization();
		uint32_t DefineCount = 0;
		uint32_t CompiledLen = 0;
		if (!TypeAttr || !NameAttr) {
			fmt::print("Shader is missing required parameters.\n");
			return false;
		}
		for (uint32_t i = 0; i < N->m_AttributeCount; i++) {
			LWEXMLAttribute *Attr = &N->m_Attributes[i];
			if (Attr == TypeAttr || Attr == CompiledPathAttr || Attr == NameAttr || Attr == AutoCompileAttr) continue;
			DefineIterList[DefineCount++] = Attr->GetName();
			DefineIterList[DefineCount++] = Attr->GetValue();
		}
		uint32_t Type = TypeAttr->GetValue().CompareList("Vertex", "Pixel", "Geometry", "Compute");
		if (Type == -1) {
			fmt::print("Unknown type: '{}' for shader: '{}'\n", TypeAttr->GetValue(), NameAttr->GetValue());
			return false;
		}
		
		if (CompiledPathAttr && SourcesModifiedTime) {
			CBuffer = LWUTF8I::Fmt<MaxNameLen>("{}.{}{}", CompiledPathAttr->GetValue(), CompiledNames[Type], DriverNames[AM->GetDriver()->GetDriverID()]);
			LWFileStream Stream;
			if (LWFileStream::OpenStream(Stream, CBuffer, LWFileStream::ReadMode | LWFileStream::BinaryMode, Alloc)) {
				if (Stream.GetModifiedTime() > SourcesModifiedTime) {
					CompiledLen = Stream.Read(CompiledBuffer, sizeof(CompiledBuffer));
				}
			}
		}
		LWShader *Res = nullptr;
		if (CompiledLen) {
			AssetPath = CBuffer;
			Res = Driver->CreateShaderCompiled(Type, CompiledBuffer, CompiledLen, AM->GetAllocator(), ErrorBuffer, sizeof(ErrorBuffer));
		} else {
			if(AutoCompileAttr && CompiledPathAttr && SourcesModifiedTime){
				CompiledLen = sizeof(CompiledBuffer);
				Res = Driver->ParseShader(Type, Source, Alloc, DefineCount, DefineIterList, CompiledBuffer, ErrorBuffer, CompiledLen, sizeof(ErrorBuffer));
				if (Res) {
					LWFileStream Stream;
					if (!LWFileStream::OpenStream(Stream, CBuffer, LWFileStream::WriteMode | LWFileStream::BinaryMode, AM->GetAllocator())) {
						fmt::print("Error can't open file for writing: '{}'\n", CBuffer);
					} else {
						Stream.Write(CompiledBuffer, CompiledLen);
					}
				}
			} else Res = Driver->ParseShader(Type, Source, AM->GetAllocator(), DefineCount, DefineIterList, nullptr, ErrorBuffer, CompiledLen, sizeof(ErrorBuffer));
		}
		if (!Res) {
			fmt::print("Error compiling shader '{}':\n{}\n", NameAttr->GetValue(), ErrorBuffer);
			return false;
		}
		Res->SetInputMap(InputCount, InputList);
		Res->SetBlockMap(BlockCount, BlockList);
		Res->SetResourceMap(ResoruceCount, ResourceList);
		for (LWEXMLNode *C = N->m_FirstChild; C; C = C->m_Next) {
			uint32_t i = C->GetName().CompareList("InputMap", "ResourceMap", "BlockMap");
			if (i == 0) {
				uint32_t Count = ParseInputMap(C, SInputList, AM);
				Res->SetInputMap(Count, SInputList);
			} else if (i == 1) {
				uint32_t Count = ParseResourceMap(C, SResourceList, AM);
				Res->SetResourceMap(Count, SResourceList);
			} else if (i == 2) {
				uint32_t Count = ParseBlockMap(C, SBlockList, AM);
				Res->SetBlockMap(Count, SBlockList);
			}
		}
		if (!AM->InsertAsset(NameAttr->m_Value, Res, LWEAsset::Shader, AssetPath)) {
			fmt::print("Error inserting asset: '{}'\n", NameAttr->GetValue());
			Driver->DestroyShader(Res);
			return false;
		}
		return true;
	};
	
	std::function<LWUTF8Iterator(const LWUTF8Iterator &, uint32_t &, uint64_t &, LWFileStream *)> ParseSourceFunc;

	auto ParsePath = [&ParseSourceFunc, &Alloc](const LWUTF8Iterator &Path, uint32_t &Length, uint64_t &ModifiedTime, LWFileStream *ExistingStream)->LWUTF8Iterator {
		LWFileStream Stream;
		if (!LWFileStream::OpenStream(Stream, Path, LWFileStream::ReadMode | LWFileStream::BinaryMode, Alloc, ExistingStream)) {
			fmt::print("Error opening file: '{}'\n", Path);
			return nullptr;
		}
		ModifiedTime = std::max<uint64_t>(ModifiedTime, Stream.GetModifiedTime());
		Length = Stream.Length()+1;
		char8_t *Buffer = Alloc.Allocate<char8_t>(Length);
		if (Stream.ReadText(Buffer, Length) != Length) {
			fmt::print("Did not read entire buffer.\n");
		}
		LWUTF8Iterator Res = ParseSourceFunc(Buffer, Length, ModifiedTime, &Stream);
		if (Res() != Buffer) LWAllocator::Destroy(Buffer);
		return Res;
	};

	auto ParseSource = [&ParsePath, &Alloc](const LWUTF8Iterator &Source, uint32_t &Length, uint64_t &ModifiedTime, LWFileStream *ExistingStream)->LWUTF8Iterator {
		LWUTF8Iterator P = Source;
		LWUTF8Iterator C = Source;
		for(; !C.AtEnd(); ++C) {
			if(*C=='#') {
				if (C.Compare("#include", 8)) {
					LWUTF8Iterator N = C.NextLine();
					LWUTF8Iterator Line = LWUTF8Iterator(C + 8, N);
					LWUTF8Iterator OpenToken = Line.NextTokens("<\"", false);
					if (OpenToken.AtEnd()) continue;
					LWUTF8Iterator CloseToken = OpenToken.NextToken(*C == '<' ? '>' : '\"');
					if (CloseToken.AtEnd()) continue;
					LWUTF8Iterator Path = LWUTF8Iterator(OpenToken + 1, CloseToken);
					uint32_t Len = 0;
					LWUTF8Iterator SubSource = ParsePath(Path, Len, ModifiedTime, ExistingStream);
					if (!SubSource.isInitialized()) continue;
					uint32_t NewLen = (Len + Length - (C.RawDistance(--N) + 1)); //-1 because Len, +Length both sizes account for a null character.
					char8_t *Buf = Alloc.Allocate<char8_t>(NewLen);
					uint32_t o = P.Copy(Buf, NewLen, C) - 1;
					o += SubSource.Copy(Buf + o, NewLen - o) - 1;
					o += N.Copy(Buf + o, NewLen - o);
					if (o != NewLen) {
						fmt::print("Something went wrong copying, Expected: {} Got: {}\n", NewLen, o);
					}
					LWAllocator::Destroy(SubSource());
					if (P != Source) LWAllocator::Destroy(P());
					C = LWUTF8Iterator(Buf + C.RawIndex(), NewLen - C.RawIndex());
					P = LWUTF8Iterator(Buf, NewLen);
					Length = NewLen;
				}
			}
		}
		return P;
	};
	ParseSourceFunc = ParseSource;

	uint64_t ModifiedTime = 0;
	LWUTF8Iterator Source, DelSource;
	uint32_t n = PathAttr->GetValue().CompareList("FontVertex", "FontColor", "FontMSDF", "UIVertex", "UITexture", "UIColor", "UIYUVTexture");
	if (n == -1) {
		uint32_t Len = 0;
		Source = DelSource = ParsePath(PathAttr->GetValue(), Len, ModifiedTime, nullptr);
		if (!Source.isInitialized()) return false;
	} else {
		if (n == 0) Source = LWFont::GetVertexShaderSource();
		else if (n == 1) Source = LWFont::GetPixelColorShaderSource();
		else if (n == 2) Source = LWFont::GetPixelMSDFShaderSource();
		else if (n == 3) Source = LWEUIManager::GetVertexShaderSource();
		else if (n == 4) Source = LWEUIManager::GetTextureShaderSource();
		else if (n == 5) Source = LWEUIManager::GetColorShaderSource();
		else if (n == 6) Source = LWEUIManager::GetYUVTextureShaderSource();
		InputCount = BuildDefaultInputMap(3, InputList, "Position", LWShaderInput::Vec4, 1, "Color", LWShaderInput::Vec4, 1, "TexCoord", LWShaderInput::Vec4, 1);
	}
	for (LWEXMLNode *C = N->m_FirstChild; C; C = C->m_Next) {
		uint32_t i = C->GetName().CompareList("Shader", "InputMap", "ResourceMap", "BlockMap");
		if (i == 0) ParseShader(C, Source, PathAttr->GetValue(), ModifiedTime, AM);
		else if (i == 1) InputCount = ParseInputMap(C, InputList, AM);
		else if (i == 2) ResoruceCount = ParseResourceMap(C, ResourceList, AM);
		else if (i == 3) BlockCount = ParseBlockMap(C, BlockList, AM);
	}
	LWAllocator::Destroy(DelSource());
	return true;
}

bool LWEAssetManager::XMLParseVideoBuffer(LWEXMLNode *N, LWEAssetManager *AM) {
	const uint32_t MaxSplits = 8;
	const uint32_t MaxSplitLength = 32;
	const uint32_t UsageFlagList[7] = { LWVideoBuffer::PersistentMapped, LWVideoBuffer::Static, LWVideoBuffer::WriteDiscardable, LWVideoBuffer::WriteNoOverlap,LWVideoBuffer::Readable, LWVideoBuffer::GPUResource, LWVideoBuffer::LocalCopy };
	LWUTF8Iterator UsageIterList[MaxSplitLength];

	LWEXMLAttribute *NameAttr = N->FindAttribute("Name");
	LWEXMLAttribute *TypeAttr = N->FindAttribute("Type");
	LWEXMLAttribute *UsageFlagsAttr = N->FindAttribute("Usage");
	LWEXMLAttribute *TypeSizeAttr = N->FindAttribute("TypeSize");
	LWEXMLAttribute *LengthAttr = N->FindAttribute("Length");
	LWEXMLAttribute *PaddedAttr = N->FindAttribute("Padded");
	LWEXMLAttribute *DataPathAttr = N->FindAttribute("DataPath");
	LWAllocator &Alloc = AM->GetAllocator();
	LWVideoDriver *Driver = AM->GetDriver();
	uint32_t Longest = 0;
	uint32_t UsageFlags = 0;
	if (!NameAttr || !TypeAttr || !UsageFlagsAttr || !TypeSizeAttr || !LengthAttr) {
		fmt::print("Error making video buffer: Missing required parameters.\n");
		return false;
	}
	uint32_t TypeID = TypeAttr->GetValue().CompareList("Vertex", "Uniform", "Index16", "Index32", "ImageBuffer");
	if (TypeID == -1) {
		fmt::print("Error video buffer '{}' has unknown type: '{}'\n", NameAttr->GetValue(), TypeAttr->GetValue());
		return false;
	}
	uint32_t Length = atoi(LengthAttr->m_Value);
	uint32_t TypeSize = atoi(TypeSizeAttr->m_Value);
	if (PaddedAttr) TypeSize = Driver->GetUniformBlockPaddedSize(TypeSize);
	if (!Length) {
		fmt::print("Error video buffer '{}' has 0 length.\n", NameAttr->GetValue());
		return false;
	}
	if (!TypeSize) {
		fmt::print("Error video buffer '{}' has 0 type size.\n", NameAttr->GetValue());
		return false;
	}
	uint32_t UsageSplitCnt = std::min<uint32_t>(UsageFlagsAttr->GetValue().SplitToken(UsageIterList, MaxSplitLength, '|'), MaxSplitLength);
	for (uint32_t i = 0; i < UsageSplitCnt; i++) {
		uint32_t n = UsageIterList[i].AdvanceWord(true).CompareList("PersistentMapped", "Static", "WriteDiscardable", "WriteNoOverlap", "Readable", "GPUResource", "LocalCopy");
		if (n == -1) {
			fmt::print("Unknown usage flag: '{}'\n", UsageIterList[i]);
		} else UsageFlags |= UsageFlagList[n];
	}
	uint8_t *Data = nullptr;
	if (DataPathAttr) {
		LWFileStream Stream;
		if (!LWFileStream::OpenStream(Stream, DataPathAttr->GetValue(), LWFileStream::ReadMode | LWFileStream::BinaryMode, Alloc)) {
			fmt::print("Error cannot open file: '{}'\n", DataPathAttr->GetValue());
		} else {
			Data = Alloc.Allocate<uint8_t>(Stream.Length());
			Stream.Read(Data, Stream.Length());
		}
	}

	LWVideoBuffer *Res = Driver->CreateVideoBuffer(TypeID, UsageFlags, TypeSize, Length, Alloc, Data);
	LWAllocator::Destroy(Data);
	if (!Res) {
		fmt::print("Error creating video buffer: '{}'\n", NameAttr->GetValue());
		return false;
	}
	if (!AM->InsertAsset(NameAttr->m_Value, Res, LWEAsset::VideoBuffer, "")) {
		fmt::print("Error inserting video buffer: '{}'\n", NameAttr->GetValue());
		Driver->DestroyVideoBuffer(Res);
		return false;
	}
	return true;
}

bool LWEAssetManager::XMLParseAudioStream(LWEXMLNode *N, LWEAssetManager *AM) {
	char8_t SBuffer[1024];
	const uint32_t MaxIterList = 32;
	LWEXMLAttribute *PathAttr = N->FindAttribute("Path");
	LWEXMLAttribute *NameAttr = N->FindAttribute("Name");
	LWEXMLAttribute *FlagAttr = N->FindAttribute("Flag");
	LWELocalization *Localize = AM->GetLocalization();
	LWUTF8Iterator FlagIterList[MaxIterList];
	if (!PathAttr || !NameAttr) return false;
	uint32_t Flag = 0;
	
	const uint32_t FlagValues[] = { LWAudioStream::Decompressed };
	const char FlagNames[][32] = { "Decompressed" };
	const uint32_t FlagsCount = sizeof(FlagValues) / sizeof(uint32_t);
	if (FlagAttr) {
		uint32_t FlagIterCnt = std::min<uint32_t>(FlagAttr->GetValue().SplitToken(FlagIterList, MaxIterList, '|'), MaxIterList);
		for (uint32_t i = 0; i < FlagIterCnt; i++) {
			uint32_t n = FlagIterList[i].AdvanceWord(true).CompareLista(FlagsCount, FlagNames);
			if (n == -1) {
				fmt::print("Unknown flag: '{}'\n", FlagIterList[i]);
			} else Flag |= FlagValues[i];
		}
	}
	LWUTF8Iterator Path = PathAttr->GetValue();
	if (Localize && Localize->ParseLocalization(SBuffer, sizeof(SBuffer), Path)) Path = LWUTF8Iterator(SBuffer);

	LWAudioStream *Stream = LWAudioStream::Create(Path, Flag, AM->GetAllocator());
	if (!Stream) {
		fmt::print("Audiostream: '{}' Could not be found at: '{}'\n", NameAttr->GetValue(), Path);
		return false;
	}
	if (!AM->InsertAsset(NameAttr->m_Value, Stream, LWEAsset::AudioStream, Path)) {
		fmt::print("Name collision with audio stream: '{}'\n", NameAttr->GetValue());
		LWAllocator::Destroy(Stream);
	}
	return true;
}

bool LWEAssetManager::XMLParseVideo(LWEXMLNode *N, LWEAssetManager *AM) {
	char SBuffer[1024 * 32];
	const uint32_t MaxFlagIters = 32;
	LWEXMLAttribute *PathAttr = N->FindAttribute("Path");
	LWEXMLAttribute *NameAttr = N->FindAttribute("Name");
	LWEXMLAttribute *FlagAttr = N->FindAttribute("Flag");
	LWEXMLAttribute *LoopCntAttr = N->FindAttribute("LoopCount");
	LWEXMLAttribute *PlaybackSpeedAttr = N->FindAttribute("PlaybackSpeed");
	LWELocalization *Localize = AM->GetLocalization();
	LWUTF8Iterator FlagIterList[MaxFlagIters];
	if (!PathAttr || !NameAttr) return false;
	uint32_t Flag = 0;
	uint32_t LoopCount = 0;
	float PlaybackSpeed = 1.0f;

	const uint32_t FlagValues[] = { LWEVideoPlayer::Playing };
	const char FlagNames[][32] = { "Playing" };
	const uint32_t FlagCount = sizeof(FlagValues) / sizeof(uint32_t);

	if (FlagAttr) {
		uint32_t FlagCnt = std::min<uint32_t>(FlagAttr->GetValue().SplitToken(FlagIterList, MaxFlagIters, '|'), MaxFlagIters);
		for(uint32_t i=0;i<FlagCnt;i++){
			uint32_t n = FlagIterList[i].AdvanceWord(true).CompareLista(FlagCount, FlagNames);
			if(n==-1){
				fmt::print("Encountered invalid flag: '{}' for '{}'\n", FlagIterList[i], NameAttr->GetValue());
			} else {
				Flag |= FlagValues[i];
			}
		}
	}
	if (LoopCntAttr) LoopCount = atoi(LoopCntAttr->m_Value);
	if (PlaybackSpeedAttr) PlaybackSpeed = (float)atof(PlaybackSpeedAttr->m_Value);
	LWUTF8Iterator Path = PathAttr->GetValue();
	if (Localize && Localize->ParseLocalization(SBuffer, sizeof(SBuffer), Path)) Path = SBuffer;
	LWEVideoPlayer *Video = AM->GetAllocator().Create<LWEVideoPlayer>();
	if (!LWEVideoPlayer::OpenVideo(*Video, AM->GetDriver(), Path, (Flag&LWEVideoPlayer::Playing) != 0, LoopCount, nullptr, nullptr, PlaybackSpeed, AM->GetAllocator())) {
		fmt::print("Video: '{}' Could not be found at: '{}'\n", NameAttr->GetValue(), Path);
		LWAllocator::Destroy(Video);
		return false;
	}
	if (!AM->InsertAsset(NameAttr->m_Value, Video, LWEAsset::Video, Path)) {
		fmt::print("Name collision with audio stream: '{}'\n", NameAttr->GetValue());
		LWAllocator::Destroy(Video);
		return false;
	}
	return true;
}

LWEAsset *LWEAssetManager::GetAsset(const LWUTF8Iterator &Name) {
	auto Iter = m_AssetMap.find(Name.Hash());
	if (Iter == m_AssetMap.end()) {
		fmt::print("Error: Could not find asset '{}'\n", Name);
		return nullptr;
	}
	return Iter->second;
}

bool LWEAssetManager::InsertAsset(const LWUTF8Iterator &Name, const LWEAsset &A) {
	LWEAsset **Pools = m_AssetPools;
	uint32_t Pool = m_AssetCount / AssetPoolSize;
	uint32_t PoolIdx = m_AssetCount % AssetPoolSize;
	if (m_PoolCount <= Pool) {
		LWEAsset **NewPools = m_Allocator.Allocate<LWEAsset*>(Pool + 1);
		std::copy(Pools, Pools + Pool, NewPools);
		NewPools[Pool] = m_Allocator.Allocate<LWEAsset>(AssetPoolSize);
		m_AssetPools = NewPools;
		m_PoolCount++;
	}
	m_AssetPools[Pool][PoolIdx] = A;
	auto Ret = m_AssetMap.emplace(Name.Hash(), &m_AssetPools[Pool][PoolIdx]);
	if (!Ret.second) {
		fmt::print("Error Asset name collision: '{}'.\n", Name);
	} else m_AssetCount++;
	return Ret.second;
}

bool LWEAssetManager::InsertAsset(const LWUTF8Iterator &Name, void *Asset, uint32_t AssetType, const LWUTF8Iterator &AssetPath) {
	return InsertAsset(Name, LWEAsset(AssetType, Asset, AssetPath));
}

bool LWEAssetManager::InsertAssetReference(const LWUTF8Iterator &Name, const LWUTF8Iterator &RefName) {
	LWEAsset *A = GetAsset(RefName);
	if (!A) {
		fmt::print("Error: Could not find asset: '{}' to make reference: '{}'\n", RefName, Name);
		return false;
	}
	auto Ret = m_AssetMap.emplace(Name.Hash(), A);
	return Ret.second;
}

LWVideoDriver *LWEAssetManager::GetDriver(void) {
	return m_Driver;
}

LWELocalization *LWEAssetManager::GetLocalization(void) {
	return m_Localization;
}

LWAllocator &LWEAssetManager::GetAllocator(void) {
	return m_Allocator;
}

LWEAsset *LWEAssetManager::GetAsset(uint32_t i){
	uint32_t Pool = i / AssetPoolSize;
	return &m_AssetPools[Pool][i%AssetPoolSize];
}

uint32_t LWEAssetManager::GetAssetCount(void){
	return m_AssetCount;
}

LWEAssetManager::LWEAssetManager(LWVideoDriver *Driver, LWELocalization *Localization, LWAllocator &Allocator) : m_Driver(Driver), m_Localization(Localization), m_Allocator(Allocator), m_AssetCount(0) {}

LWEAssetManager::~LWEAssetManager() {
	for (uint32_t i = 0; i <m_PoolCount;i++) {
		uint32_t Cnt = std::min<uint32_t>(m_AssetCount, AssetPoolSize);
		for (uint32_t n = 0; n < Cnt; n++) m_AssetPools[i][n].Release(m_Driver);
		m_AssetCount -= Cnt;
		LWAllocator::Destroy(m_AssetPools[i]);
	}
	LWAllocator::Destroy(m_AssetPools);
}

#pragma endregion