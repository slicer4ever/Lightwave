#include "LWEAsset.h"
#include <LWCore/LWText.h>
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

LWTexture *LWEAsset::AsTexture(void) {
	return (LWTexture*)m_Asset;
}

LWFont *LWEAsset::AsFont(void) {
	return (LWFont*)m_Asset;
}

LWShader *LWEAsset::AsShader(void) {
	return (LWShader*)m_Asset;
}

LWAudioStream *LWEAsset::AsAudioStream(void) {
	return (LWAudioStream*)m_Asset;
}

LWEVideoPlayer *LWEAsset::AsVideoPlayer(void) {
	return (LWEVideoPlayer*)m_Asset;
}

LWPipeline *LWEAsset::AsPipeline(void) {
	return (LWPipeline*)m_Asset;
}

LWVideoBuffer *LWEAsset::AsVideoBuffer(void) {
	return (LWVideoBuffer*)m_Asset;
}

void *LWEAsset::GetAsset(void) {
	return m_Asset;
}

uint32_t LWEAsset::GetType(void) {
	return m_Type;
}

const char *LWEAsset::GetAssetPath(void){
	return m_AssetPath;
}

LWEAsset::LWEAsset(uint32_t Type, void *Asset, const char *AssetPath) : m_Type(Type), m_Asset(Asset) {
	m_AssetPath[0] = '\0';
	strncat(m_AssetPath, AssetPath, sizeof(m_AssetPath));
}

LWEAsset::LWEAsset() {}

#pragma endregion

#pragma region LWEAssetManager
bool LWEAssetManager::XMLParser(LWEXMLNode *N, void *UserData, LWEXML *XML) {
	LWEAssetManager *AM = (LWEAssetManager*)UserData;
	for (LWEXMLNode *C = XML->NextNode(nullptr, N); C; C = XML->NextNode(C, N, true)) {
		uint32_t i = LWText::CompareMultiple(C->m_Name, 9, "Texture", "Font", "Shader", "Video", "AudioStream", "Pipeline", "VideoBuffer", "ShaderBuilder", "Reference");
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
			LWXMLAttribute *NameAttr = C->FindAttribute("Name");
			LWXMLAttribute *RefAttr = C->FindAttribute("Ref");
			if (NameAttr && RefAttr) Loaded = AM->InsertAssetReference(NameAttr->m_Value, RefAttr->m_Value);
		}
		if (!Loaded) {
			std::cout << "Error unable to load asset: '" << C->m_Name << "'" << std::endl;
			//return true;
		}
	}
	return true;
}

bool LWEAssetManager::XMLParseFont(LWEXMLNode *N, LWEAssetManager *AM) {
	char SBuffer[1024 * 32];
	uint32_t GlpyhCount = 0;
	const uint32_t MaxRanges = 8;
	uint32_t GlyphFirst[MaxRanges];
	uint32_t GlyphLens[MaxRanges];
	uint32_t Size = 48;
	LWXMLAttribute *PathAttr = N->FindAttribute("Path");
	LWXMLAttribute *NameAttr = N->FindAttribute("Name");
	LWXMLAttribute *SizeAttr = N->FindAttribute("Size");
	LWXMLAttribute *GlyphFirstAttr = N->FindAttribute("GlyphFirst");
	LWXMLAttribute *GlyphLengthAttr = N->FindAttribute("GlyphLength");
	LWXMLAttribute *ErrorGlyphAttr = N->FindAttribute("ErrorGlyph");
	LWELocalization *Localize = AM->GetLocalization();
	if (!PathAttr || !NameAttr) return false;
	if (SizeAttr) Size = atoi(SizeAttr->m_Value);
	if (GlyphFirstAttr && GlyphLengthAttr) {
		for (const char *GF = GlyphFirstAttr->m_Value, *GL = GlyphLengthAttr->m_Value; *GF && *GL;) {
			GlyphFirst[GlpyhCount] = atoi(GF);
			GlyphLens[GlpyhCount] = atoi(GL);
			GlpyhCount++;
			GF = LWText::FirstToken(GF, '|');
			GL = LWText::FirstToken(GL, '|');
			GF = GF ? GF + 1 : GF;
			GL = GL ? GL + 1 : GL;
			if (!GF || !GL) break;
		}
	}
	LWFont *F = nullptr;
	LWFileStream FontFile;
	const char *PathValue = PathAttr->m_Value;
	if (Localize) PathValue = Localize->ParseLocalization(SBuffer, sizeof(SBuffer), PathAttr->m_Value);
	uint32_t ExtType = LWFileStream::IsExtensions(PathValue, 3, "ttf", "fnt", "arfont");
	if (!LWFileStream::OpenStream(FontFile, PathValue, LWFileStream::ReadMode | ((ExtType == 0 || ExtType==2) ? LWFileStream::BinaryMode : 0), *AM->GetAllocator())) {
		std::cout << "Error opening font file: '" << PathValue << "'" << std::endl;
		return false;
	}
	if (ExtType == 0) F = LWFont::LoadFontTTF(&FontFile, AM->GetDriver(), Size, GlpyhCount, GlyphFirst, GlyphLens, *AM->GetAllocator());
	else if (ExtType == 1) F = LWFont::LoadFontFNT(&FontFile, AM->GetDriver(), *AM->GetAllocator());
	else if (ExtType == 2) F = LWFont::LoadFontAR(&FontFile, AM->GetDriver(), *AM->GetAllocator());
	if (!F) {
		std::cout << "Error creating font file!" << std::endl;
		return false;
	}
	if (!AM->InsertAsset(NameAttr->m_Value, F, LWEAsset::Font, PathValue)) {
		std::cout << "Error inserting asset: '" << NameAttr->m_Value << "'" << std::endl;
		LWAllocator::Destroy(F);
		return false;
	}
	if (ErrorGlyphAttr) F->SetErrorGlyph(atoi(ErrorGlyphAttr->m_Value));
	for (LWEXMLNode *C = N->m_FirstChild; C; C = C->m_Next) {
		uint32_t n = LWText::CompareMultiple(C->m_Name, 1, "GlyphName");
		if (n == 0) {
			LWXMLAttribute *GNameAttr = C->FindAttribute("Name");
			LWXMLAttribute *GCodeAttr = C->FindAttribute("Code");
			if (!GNameAttr || !GCodeAttr) continue;
			F->InsertGlyphName(GNameAttr->m_Value, atoi(GCodeAttr->m_Value));
		}
	}

	return true;
}

bool LWEAssetManager::XMLParseTexture(LWEXMLNode *N, LWEAssetManager *AM) {
	char SBuffer[1024 * 32];
	LWELocalization *Localize = AM->GetLocalization();
	LWXMLAttribute *PathAttr = N->FindAttribute("Path");
	LWXMLAttribute *NameAttr = N->FindAttribute("Name");
	LWXMLAttribute *StateAttr = N->FindAttribute("State");
	if (!PathAttr || !NameAttr) return false;
	const uint32_t FlagValues[] = { LWTexture::MinNearestMipmapNearest, LWTexture::MinLinearMipmapNearest, LWTexture::MinNearestMipmapLinear, LWTexture::MinLinearMipmapLinear, LWTexture::MinNearest, LWTexture::MagNearest, LWTexture::WrapSClampToEdge, LWTexture::WrapTClampToEdge, LWTexture::WrapRClampToEdge, LWTexture::CompareNone, LWTexture::MinLinear, LWTexture::MagLinear, LWTexture::WrapSClampToBorder, LWTexture::WrapSMirroredRepeat, LWTexture::WrapSRepeat, LWTexture::WrapTClampToBorder, LWTexture::WrapTMirroredRepeat, LWTexture::WrapTRepeat, LWTexture::WrapRClampToBorder, LWTexture::WrapRMirroredRepeat, LWTexture::WrapRRepeat, LWTexture::RenderTarget, LWTexture::RenderBuffer };
	const char FlagNames[][32]  = { "MinNearestMipmapNearest",          "MinLinearMipmapNearest",          "MinNearestMipmapLinear",          "MinLinearMipmapLinear",          "MinNearest",          "MagNearest",          "WrapSClampToEdge",          "WrapTClampToEdge",          "WrapRClampToEdge",          "CompareNone",          "MinLinear",          "MagLinear",          "WrapSClampToBorder",          "WrapSMirroredRepeat",          "WrapSRepeat",          "WrapTClampToBorder",          "WrapTMirroredRepeat",          "WrapTRepeat",          "WrapRClampToBorder",          "WrapRMirroredRepeat",          "WrapRRepeat",          "RenderTarget",          "RenderBuffer" };
	const uint32_t TotalValues = sizeof(FlagValues) / sizeof(uint32_t);
	const char *PathValue = PathAttr->m_Value;
	if (Localize) PathValue = Localize->ParseLocalization(SBuffer, sizeof(SBuffer), PathAttr->m_Value);
	LWImage Image;
	if (!LWImage::LoadImage(Image, PathValue, *AM->GetAllocator())) {
		std::cout << "Error loading image: '" << PathValue << "'" << std::endl;
		return false;
	}
	uint32_t TextureState = 0;
	if (StateAttr) {
		for (char *C = StateAttr->m_Value; *C;C++) {
			uint32_t i = 0;
			for (; i < TotalValues; i++) {
				if (LWText::Compare(C, FlagNames[i], (uint32_t)strlen(FlagNames[i]))) break;
			}
			if (i >= TotalValues) {
				std::cout << "Encountered invalid flag: '" << C << "' for: '" << NameAttr->m_Value << "'" << std::endl;
			} else {
				TextureState |= FlagValues[i];
			}

			C = LWText::FirstToken(C, '|');
			if (!C) break;
		}
	}
	LWTexture *Tex = AM->GetDriver()->CreateTexture(TextureState, Image, *AM->GetAllocator());
	if (!AM->InsertAsset(NameAttr->m_Value, Tex, LWEAsset::Texture, PathValue)) {
		std::cout << "Error inserting asset: '" << NameAttr->m_Value << "'" << std::endl;
		AM->GetDriver()->DestroyTexture(Tex);
		return false;
	}
	return true;
}

bool LWEAssetManager::XMLParsePipeline(LWEXMLNode *N, LWEAssetManager *AM) {

	const char *FlagNames[] = { "DepthTest", "Blending", "NoDepth", "NoColorR", "NoColorG", "NoColorB", "NoColorA", "NoColor", "ClipPlane0", "ClipPlane1", "ClipPlane2", "StencilTest", "DepthBias" };
	const uint64_t FlagValues[] = { LWPipeline::DEPTH_TEST, LWPipeline::BLENDING, LWPipeline::No_Depth, LWPipeline::No_ColorR, LWPipeline::No_ColorG, LWPipeline::No_ColorB, LWPipeline::No_ColorA, LWPipeline::No_Color, LWPipeline::CLIPPLANE0, LWPipeline::CLIPPLANE1, LWPipeline::CLIPPLANE2, LWPipeline::STENCIL_TEST, LWPipeline::DEPTH_BIAS };
	const char *FillNames[] = { "Solid", "Wireframe" };
	const uint64_t FillValues[] = { LWPipeline::SOLID, LWPipeline::WIREFRAME };
	const char *CullNames[] = { "None", "CCW", "CW" };
	const uint64_t CullValues[] = { LWPipeline::CULL_NONE, LWPipeline::CULL_CCW, LWPipeline::CULL_CW };
	const char *BlendNames[] = { "Zero", "One", "SrcColor", "DstColor", "SrcAlpha", "DstAlpha", "OneMinusSrcColor", "OneMinusDstColor", "OneMinusSrcAlpha", "OneMinusDstAlpha" };
	const uint64_t BlendValues[] = { LWPipeline::BLEND_ZERO, LWPipeline::BLEND_ONE, LWPipeline::BLEND_SRC_COLOR, LWPipeline::BLEND_DST_COLOR, LWPipeline::BLEND_SRC_ALPHA, LWPipeline::BLEND_DST_ALPHA, LWPipeline::BLEND_ONE_MINUS_SRC_COLOR, LWPipeline::BLEND_ONE_MINUS_DST_COLOR, LWPipeline::BLEND_ONE_MINUS_SRC_ALPHA, LWPipeline::BLEND_ONE_MINUS_DST_ALPHA };
	const char *CompareNames[] = { "Always", "Never", "Less", "Greater", "LessEqual", "GreaterEqual" };
	const uint64_t CompareValues[] = { LWPipeline::ALWAYS, LWPipeline::NEVER, LWPipeline::LESS, LWPipeline::GREATER, LWPipeline::LESS_EQL, LWPipeline::GREATER_EQL };
	const char *StencilOpNames[] = { "Keep", "Zero", "Replace", "Increase", "Decrease", "IncreaseWrap", "DecreaseWrap", "Invert" };
	const uint64_t StencilOpValues[] = { LWPipeline::STENCIL_OP_KEEP, LWPipeline::STENCIL_OP_ZERO, LWPipeline::STENCIL_OP_REPLACE, LWPipeline::STENCIL_OP_INCR, LWPipeline::STENCIL_OP_DECR, LWPipeline::STENCIL_OP_INCR_WRAP, LWPipeline::STENCIL_OP_DECR_WRAP, LWPipeline::STENCIL_OP_INVERT };
	const uint32_t FlagCount = sizeof(FlagValues) / sizeof(uint64_t);

	auto ParseBlockBinding = [](LWEXMLNode *N, LWPipeline *P, LWEAssetManager *AM) {
		LWXMLAttribute *NameAttr = N->FindAttribute("Name");
		LWXMLAttribute *OffsetAttr = N->FindAttribute("Offset");
		LWXMLAttribute *SlotAttr = N->FindAttribute("Slot");
		LWXMLAttribute *SlotNameAttr = N->FindAttribute("SlotName");
		if (!NameAttr || (!SlotAttr && !SlotNameAttr)) {
			std::cout << "Block node does not have required parameters." << std::endl;
			return;
		}
		LWVideoBuffer *B = AM->GetAsset<LWVideoBuffer>(NameAttr->m_Value);
		if (!B) {
			std::cout << "Error block could not find buffer: '" << NameAttr->m_Value << "'" << std::endl;
			return;
		}
		uint32_t SlotIdx = -1;
		if (SlotAttr) SlotIdx = atoi(SlotAttr->m_Value);
		else if (SlotNameAttr) SlotIdx = P->FindBlock(SlotNameAttr->m_Value);
		if (SlotIdx == -1) {
			std::cout << "Error block not found." << std::endl;
		}
		uint32_t Offset = OffsetAttr ? atoi(OffsetAttr->m_Value) : 0;
		P->SetUniformBlock(SlotIdx, B, Offset);
		return;
	};

	auto ParseResourceBinding = [](LWEXMLNode *N, LWPipeline *P, LWEAssetManager *AM) {
		LWXMLAttribute *NameAttr = N->FindAttribute("Name");
		LWXMLAttribute *OffsetAttr = N->FindAttribute("Offset");
		LWXMLAttribute *SlotAttr = N->FindAttribute("Slot");
		LWXMLAttribute *SlotNameAttr = N->FindAttribute("SlotName");
		if (!NameAttr || (!SlotAttr && !SlotNameAttr)) {
			std::cout << "Block node does not have required parameters." << std::endl;
			return;
		}
		LWVideoBuffer *B = AM->GetAsset<LWVideoBuffer>(NameAttr->m_Value);
		LWTexture *T = AM->GetAsset<LWTexture>(NameAttr->m_Value);
		if (!B && !T) {
			std::cout << "Error block could not find buffer or texture: '" << NameAttr->m_Value << "'" << std::endl;
			return;
		}
		uint32_t SlotIdx = -1;
		if (SlotAttr) SlotIdx = atoi(SlotAttr->m_Value);
		else if (SlotNameAttr) SlotIdx = P->FindResource(SlotNameAttr->m_Value);
		if (SlotIdx == -1) {
			std::cout << "Error resource slot not found." << std::endl;
			return;
		}
		uint32_t Offset = OffsetAttr ? atoi(OffsetAttr->m_Value) : 0;
		if (B) P->SetResource(SlotIdx, B, Offset);
		else P->SetResource(SlotIdx, T);
		return;
	};

	LWXMLAttribute *NameAttr = N->FindAttribute("Name");
	LWXMLAttribute *VertexAttr = N->FindAttribute("Vertex");
	LWXMLAttribute *PixelAttr = N->FindAttribute("Pixel");
	LWXMLAttribute *GeometryAttr = N->FindAttribute("Geometry");
	LWXMLAttribute *ComputeAttr = N->FindAttribute("Compute");
	LWXMLAttribute *FlagAttr = N->FindAttribute("Flags");
	LWXMLAttribute *FillModeAttr = N->FindAttribute("FillMode");
	LWXMLAttribute *CullModeAttr = N->FindAttribute("CullMode");
	LWXMLAttribute *DepthCompareAttr = N->FindAttribute("DepthCompare");
	LWXMLAttribute *SourceBlendModeAttr = N->FindAttribute("SourceBlendMode");
	LWXMLAttribute *DestBlendModeAttr = N->FindAttribute("DestBlendMode");
	LWXMLAttribute *StencilCompareAttr = N->FindAttribute("StencilCompare");
	LWXMLAttribute *StencilRefValueAttr = N->FindAttribute("StencilValue");
	LWXMLAttribute *StencilReadMaskValueAttr = N->FindAttribute("StencilReadMask");
	LWXMLAttribute *StencilWriteMaskValueAttr = N->FindAttribute("StencilWriteMask");
	LWXMLAttribute *StencilOpFailAttr = N->FindAttribute("StencilOpFail");
	LWXMLAttribute *StencilOpDFailAttr = N->FindAttribute("StencilOpDepthFail");
	LWXMLAttribute *StencilOpPassAttr = N->FindAttribute("StencilOpPass");
	LWXMLAttribute *DepthBiasAttr = N->FindAttribute("DepthBias");
	LWXMLAttribute *DepthSlopedBiasAttr = N->FindAttribute("DepthSlopedBias");
	if (!NameAttr){
		std::cout << "Error pipeline has no name attribute." << std::endl;
		return false;
	}
	if (!VertexAttr && !ComputeAttr) {
		std::cout << "Pipeline '" << NameAttr->m_Value << "' must have either a vertex shader, or compute shader." << std::endl;
		return false;
	}
	uint64_t Flags = 0;

	LWShader *VS = VertexAttr ? AM->GetAsset<LWShader>(VertexAttr->m_Value) : nullptr;
	LWShader *PS = PixelAttr ? AM->GetAsset<LWShader>(PixelAttr->m_Value) : nullptr;
	LWShader *GS = GeometryAttr ? AM->GetAsset<LWShader>(GeometryAttr->m_Value) : nullptr;
	LWShader *CS = ComputeAttr ? AM->GetAsset<LWShader>(ComputeAttr->m_Value) : nullptr;
	if (VertexAttr && !VS) {
		std::cout << "Could not find vertex shader: '" << VertexAttr->m_Value << "'" << std::endl;
		return false;
	}
	if (PixelAttr && !PS) {
		std::cout << "Could not find pixel shader: '" << PixelAttr->m_Value << "'" << std::endl;
		return false;
	}
	if (GeometryAttr && !GS) {
		std::cout << "Could not find geometry shader: '" << GeometryAttr->m_Value << "'" << std::endl;
		return false;
	}
	if (ComputeAttr && !CS) {
		std::cout << "Could not find compute shader: '" << ComputeAttr->m_Value << "'" << std::endl;
		return false;
	}
	if (FlagAttr) {

		for (char *C = FlagAttr->m_Value; *C; C++) {
			uint32_t n = 0;
			for (; n < FlagCount; n++) {
				if (LWText::Compare(C, FlagNames[n], (uint32_t)strlen(FlagNames[n]))) break;
			}
			if (n >= FlagCount) {
				std::cout << "Invalid flag encountered: '" << C << "' for: '" << NameAttr->m_Value << "'" << std::endl;
			} else {
				Flags |= FlagValues[n];
			}
			C = LWText::FirstToken(C, '|');
			if (!C) break;
		}
	}
	if (FillModeAttr) {
		uint32_t n = LWText::CompareMultiplea(FillModeAttr->m_Value, 2, FillNames);
		if (n == -1) {
			std::cout << "Unknown fill mode: '" << FillModeAttr->m_Value << "'" << std::endl;
		} else Flags |= (FillValues[n] << LWPipeline::FILL_MODE_BITOFFSET);
	}
	if (CullModeAttr) {
		uint32_t n = LWText::CompareMultiplea(CullModeAttr->m_Value, 3, CullNames);
		if (n == -1) {
			std::cout << "Unknown cull mode: '" << CullModeAttr->m_Value << "'" << std::endl;
		} else Flags |= (CullValues[n] << LWPipeline::CULL_BITOFFSET);
	}
	if (DepthCompareAttr) {
		uint32_t n = LWText::CompareMultiplea(DepthCompareAttr->m_Value, 6, CompareNames);
		if (n == -1) {
			std::cout << "Unknown depth compare mode: '" << DestBlendModeAttr->m_Value << "'" << std::endl;
		} else Flags |= (CompareValues[n] << LWPipeline::DEPTH_COMPARE_BITOFFSET);
	}
	if (SourceBlendModeAttr) {
		uint32_t n = LWText::CompareMultiplea(SourceBlendModeAttr->m_Value, 10, BlendNames);
		if (n == -1) {
			std::cout << "Unknown source blend mode: '" << SourceBlendModeAttr->m_Value << "'" << std::endl;
		} else Flags |= (BlendValues[n] << LWPipeline::BLEND_SRC_BITOFFSET);
	}
	if (DestBlendModeAttr) {
		uint32_t n = LWText::CompareMultiplea(DestBlendModeAttr->m_Value, 10, BlendNames);
		if (n == -1) {
			std::cout << "Unknown dest blend mode: '" << DestBlendModeAttr->m_Value << "'" << std::endl;
		} else Flags |= (BlendValues[n] << LWPipeline::BLEND_DST_BITOFFSET);
	}
	if (StencilCompareAttr) {
		uint32_t n = LWText::CompareMultiplea(StencilCompareAttr->m_Value, 6, CompareNames);
		if (n == -1) {
			std::cout << "Unknown stencil compare mode: '" << StencilCompareAttr->m_Value << "'" << std::endl;
		} else Flags |= (CompareValues[n] << LWPipeline::STENCIL_COMPARE_BITOFFSET);
	}
	if (StencilOpFailAttr) {
		uint32_t n = LWText::CompareMultiplea(StencilOpFailAttr->m_Value, 8, StencilOpNames);
		if (n == -1) {
			std::cout << "Unknown stencil op fail: '" << StencilOpFailAttr->m_Value << "'" << std::endl;
		} else Flags |= (StencilOpValues[n] << LWPipeline::STENCIL_OP_SFAIL_BITOFFSET);
	}
	if (StencilOpDFailAttr) {
		uint32_t n = LWText::CompareMultiplea(StencilOpDFailAttr->m_Value, 8, StencilOpNames);
		if (n == -1) {
			std::cout << "Unknown stencil op depth fail: '" << StencilOpDFailAttr->m_Value << "'" << std::endl;
		} else Flags |= (StencilOpValues[n] << LWPipeline::STENCIL_OP_DFAIL_BITOFFSET);
	}
	if (StencilOpPassAttr) {
		uint32_t n = LWText::CompareMultiplea(StencilOpPassAttr->m_Value, 8, StencilOpNames);
		if (n == -1) {
			std::cout << "Unknown stencil op pass: '" << StencilOpPassAttr->m_Value << "'" << std::endl;
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
	if (VS) {
		P = AM->GetDriver()->CreatePipeline(VS, GS, PS, Flags, *AM->GetAllocator());
	} else if (CS) {
		P = AM->GetDriver()->CreatePipeline(CS, *AM->GetAllocator());
	}
	if (!P) {
		std::cout << "Failed to create pipeline for: '" << NameAttr->m_Value << "'" << std::endl;
		return false;
	}
	P->SetDepthBias((P->GetFlag()&LWPipeline::DEPTH_BIAS), DepthBiasAttr ? (float)atof(DepthBiasAttr->m_Value) : 0.0f, DepthSlopedBiasAttr ? (float)atof(DepthSlopedBiasAttr->m_Value) : 0.0f);
	for (LWEXMLNode *C = N->m_FirstChild; C; C = C->m_Next) {
		uint32_t i = LWText::CompareMultiple(C->m_Name, 2, "Resource", "Block");
		if (i == 0) ParseResourceBinding(C, P, AM);
		else if (i == 1) ParseBlockBinding(C, P, AM);
		else{
			std::cout << "Error pipeline '" << NameAttr->m_Value << "' has unknown child node: '" << C->m_Name << "'" << std::endl;
		}
	}

	if (!AM->InsertAsset(NameAttr->m_Value, P, LWEAsset::Pipeline, "")) {
		std::cout << "Error inserting asset: '" << NameAttr->m_Value << "'" << std::endl;
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
	const uint32_t MaxDefineValue = 64;
	const uint32_t MaxErrorBufferLen = 4 * 1024;
	const uint32_t TotalDriverCnt = 10;
	char AssetPath[MaxPathLen] = "";
	
	char Buffer[MaxBufferLen];
	char CBuffer[MaxBufferLen]="";
	char CompiledBuffer[MaxFileBufferLen];
	char ErrorBuffer[MaxErrorBufferLen]="";
	char DefineList[MaxDefines][MaxDefineValue];
	char *DefineMap[MaxDefines];
	for (uint32_t i = 0; i < MaxDefines; i++) DefineMap[i] = DefineList[i];

	uint32_t CompiledLen = 0;
	uint32_t DefineCount = 0;
	LWVideoDriver *Driver = AM->GetDriver();
	uint32_t DriverID = Driver->GetDriverID();
	LWAllocator &Allocator = *AM->GetAllocator();
	LWShader *Res = nullptr;

	auto ParseInputMap = [](LWEXMLNode *N, LWShader *S, LWEAssetManager *AM) {
		//                                                       Float,      Int,        UInt,       Double,     Vec2,       Vec3,       Vec4,       uVec2,      uVec3,      uVec4,      iVec2,      iVec3,      iVec4,      dVec2,      dVec3,      dVec4
		const uint32_t TypeNameHashs[LWShaderInput::Count] = { 0x4c816225, 0xf87415fe, 0xe939eb21, 0x8e464c28, 0x2c3c5815, 0x2b3c5682, 0x263c4ea3, 0x1a199b30, 0x1b199cc3, 0x2019a4a2, 0x5d3f3cc4, 0x5e3f3e57, 0x5b3f399e, 0xbfb0ee5f, 0xbeb0eccc, 0xbdb0eb39 };
		char TypeBuffer[256];
		char NameBuffer[256];
		LWShaderInput Inputs[LWShader::MaxInputs];
		uint32_t InputCount = 0;
		for (uint32_t i = 0; i < N->m_AttributeCount; i++) {
			LWXMLAttribute &A = N->m_Attributes[i];
			uint32_t NameHash = LWText::MakeHash(A.m_Name);
			uint32_t Length = 1;
			sscanf(A.m_Value, "%[^[][%d]", TypeBuffer, &Length);
			uint32_t TypeHash = LWText::MakeHash(TypeBuffer);
			uint32_t T = 0;
			for (; T < LWShaderInput::Count && TypeNameHashs[T] != TypeHash; T++) {}
			if (T >= LWShaderInput::Count) {
				std::cout << "Error unknown type: '" << A.m_Value << "' for input: '" << A.m_Name << "'" << std::endl;
				continue;
			}
			if (Length > 1) {
				for (uint32_t n = 0; n < Length; n++) {
					snprintf(NameBuffer, sizeof(NameBuffer), "%s[%d]", A.m_Name, n);
					Inputs[InputCount++] = LWShaderInput(NameBuffer, T, Length);
				}
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
			ResourceNameHashs[Count++] = LWText::MakeHash(N->m_Attributes[i].m_Name);
		}
		S->SetResourceMap(Count, ResourceNameHashs);
		return;
	};

	auto ParseBlockMap = [](LWEXMLNode *N, LWShader *S, LWEAssetManager *AM) {
		uint32_t BlockNameHashs[LWShader::MaxBlocks];
		uint32_t Count = 0;
		for (uint32_t i = 0; i < N->m_AttributeCount; i++) {
			BlockNameHashs[Count++] = LWText::MakeHash(N->m_Attributes[i].m_Name);
		}
		S->SetBlockMap(Count, BlockNameHashs);
		return;
	};

	std::function<char*(char*, uint32_t &, uint64_t &, LWAllocator &, LWFileStream *)> ParseSourceFunc;

	auto ParsePath = [&ParseSourceFunc](const char *Path, uint32_t &Length, uint64_t &ModifiedTime, LWAllocator &Allocator, LWFileStream *ExistingStream)->char* {
		LWFileStream Stream;
		if (!LWFileStream::OpenStream(Stream, Path, LWFileStream::ReadMode | LWFileStream::BinaryMode, Allocator, ExistingStream)) {
			std::cout << "Error opening file: '" << Path << "'" << std::endl;
			return nullptr;
		}
		ModifiedTime = std::max<uint64_t>(ModifiedTime, Stream.GetModifiedTime());
		Length = Stream.Length();
		char *Buffer = Allocator.AllocateArray<char>(Length + 1);
		Stream.ReadText(Buffer, Length);
		char *Res = ParseSourceFunc(Buffer, Length, ModifiedTime, Allocator, &Stream);
		if (Res != Buffer) LWAllocator::Destroy(Buffer);
		return Res;
	};

	auto ParseSource = [&ParsePath](char *Source, uint32_t &Length, uint64_t &ModifiedTime, LWAllocator &Allocator, LWFileStream *ExistingStream)->char* {
		char *P = Source;
		char *S = Source;
		char LineBuffer[1024];
		char PathBuffer[1024];
		for (; *S; S++) {
			if (*S == '#') {
				if (LWText::Compare(S, "#include", 8)) {
					const char *C = LWText::CopyToTokens(S + 8, LineBuffer, sizeof(LineBuffer), "\n\r\0");
					if (*C) C += LWText::UTF8ByteSize(C);
					uint32_t LineLen = (uint32_t)(uintptr_t)(C - S);
					const char *B = LWText::FirstToken(LineBuffer, '<');
					if (B) {
						LWText::CopyToTokens(B + 1, PathBuffer, sizeof(PathBuffer), ">");
					} else {
						B = LWText::FirstToken(LineBuffer, '\"');
						if (!B) continue;
						LWText::CopyToTokens(B + 1, PathBuffer, sizeof(PathBuffer), "\"");
					}
					uint32_t Len = 0;
					char *SubBuffer = ParsePath(PathBuffer, Len, ModifiedTime, Allocator, ExistingStream);
					if (!SubBuffer) continue;
					uint32_t Pos = (uint32_t)(uintptr_t)(S - P);
					uint32_t NewLen = (Len + Length - LineLen);
					char *Buf = Allocator.AllocateArray<char>(NewLen + 1);
					std::copy(P, P + Pos, Buf);
					std::copy(SubBuffer, SubBuffer + Len, Buf + Pos);
					std::copy(P + (Pos + LineLen), P + (Length + 1), Buf + (Pos + Len));
					LWAllocator::Destroy(SubBuffer);
					if (P != Source) LWAllocator::Destroy(P);
					P = Buf;
					S = P + (Pos + Len - 1);
					Length = NewLen;
				}
			}
		}
		return P;
	};
	ParseSourceFunc = ParseSource;

	LWXMLAttribute *TypeAttr = N->FindAttribute("Type");
	LWXMLAttribute *PathAttr = N->FindAttribute("Path");
	LWXMLAttribute *CompiledPathAttr = N->FindAttribute("CompiledPath");
	LWXMLAttribute *NameAttr = N->FindAttribute("Name");
	LWXMLAttribute *AutoCompileAttr = N->FindAttribute("AutoCompile");
	LWELocalization *Localize = AM->GetLocalization();
	if ((!PathAttr && !CompiledPathAttr) || !TypeAttr || !NameAttr) return false;

	for (uint32_t i = 0; i < N->m_AttributeCount && DefineCount<MaxDefines; i++) {
		LWXMLAttribute *Attr = &N->m_Attributes[i];
		if(Attr==TypeAttr || Attr==PathAttr || Attr==CompiledPathAttr || Attr==NameAttr || Attr==AutoCompileAttr) continue;
		if (*Attr->m_Value) {
			snprintf(DefineList[DefineCount], MaxDefineValue, "%s:%s", Attr->m_Name, Attr->m_Value);
		} else snprintf(DefineList[DefineCount], MaxDefineValue, "%s", Attr->m_Name);
		DefineCount++;
	}

	const char *Path = PathAttr ? PathAttr->m_Value : nullptr;
	const char *CPath = CompiledPathAttr ? CompiledPathAttr->m_Value : nullptr;
	if (Localize) {
		if (Path) Path = Localize->ParseLocalization(Buffer, sizeof(Buffer), Path);
		if (CPath) CPath = Localize->ParseLocalization(CBuffer, sizeof(CBuffer), CPath);
	}
	uint32_t Type = LWText::CompareMultiple(TypeAttr->m_Value, 4, "Vertex", "Pixel", "Geometry", "Compute");
	if (Type == -1) {
		std::cout << "Unknown type: '" << TypeAttr->m_Value << "' for shader: '" << NameAttr->m_Value << "'" << std::endl;
		return false;
	}
	if (CPath) {
		snprintf(CBuffer, sizeof(CBuffer), "%s.%s%s", CPath, CompiledNames[Type], DriverNames[DriverID]);
		CPath = CBuffer;
	}
	uint32_t n = LWText::CompareMultiple(Path, 7, "FontVertex", "FontColor", "FontMSDF", "UIVertex", "UITexture", "UIColor", "UIYUVTexture");
	char *Source = nullptr;
	char *DelSource = nullptr;
	uint64_t ModifiedTime = 0;

	if (n == -1) {
		LWFileStream CStream;
		//Open both streams and if Path source is newer than the compiled path, then recompile the source.
		if (CPath) {
			if (!LWFileStream::OpenStream(CStream, CPath, LWFileStream::ReadMode | LWFileStream::BinaryMode, Allocator)) {
				if (!Path) {
					std::cout << "Could not open compiled shader at: '" << CPath << "' for: '" << NameAttr->m_Value << "'" << std::endl;
					return false;
				}
				CPath = nullptr;
			}
		}
		if (Path) {
			uint32_t Len = 0;
			Source = DelSource = ParsePath(Path, Len, ModifiedTime, Allocator, nullptr);
			if (!Source) return false;
			if (ModifiedTime > CStream.GetModifiedTime()) CPath = nullptr;
		}

		if (CPath) {
			strncat(AssetPath, CPath, sizeof(AssetPath));
			CompiledLen = CStream.Read(CompiledBuffer, sizeof(CompiledBuffer));
			Res = Driver->CreateShaderCompiled(Type, CompiledBuffer, CompiledLen, Allocator, ErrorBuffer, sizeof(ErrorBuffer));
		} else if (Path) {
			strncat(AssetPath, Path, sizeof(AssetPath));
			if (AutoCompileAttr) {
				CompiledLen = sizeof(CompiledBuffer);
				Res = Driver->ParseShader(Type, Source, Allocator, DefineCount, (const char**)DefineMap, CompiledBuffer, ErrorBuffer, &CompiledLen, sizeof(ErrorBuffer));
				if (CompiledLen && *CBuffer) {
					CStream.Finished();
					if (LWFileStream::OpenStream(CStream, CBuffer, LWFileStream::WriteMode | LWFileStream::BinaryMode, Allocator)) {
						CStream.Write(CompiledBuffer, CompiledLen);
						CStream.Finished();
					}
				}
			} else Res = Driver->ParseShader(Type, Source, Allocator, DefineCount, (const char**)DefineMap, nullptr, ErrorBuffer, nullptr, sizeof(ErrorBuffer));
		} else {
			std::cout << "No valid path for shader: '" << NameAttr->m_Value << "'" << std::endl;
			return false;
		}
	} else {
		strncat(AssetPath, Path, sizeof(AssetPath));
		if (n == 0) Res = Driver->ParseShader(Type, LWFont::GetVertexShaderSource(), Allocator, DefineCount, (const char**)DefineMap, nullptr, ErrorBuffer, nullptr, sizeof(ErrorBuffer));
		else if (n == 1) Res = Driver->ParseShader(Type, LWFont::GetPixelColorShaderSource(), Allocator, DefineCount, (const char**)DefineMap, nullptr, ErrorBuffer, nullptr, sizeof(ErrorBuffer));
		else if (n == 2) Res = Driver->ParseShader(Type, LWFont::GetPixelMSDFShaderSource(), Allocator, DefineCount, (const char**)DefineMap, nullptr, ErrorBuffer, nullptr, sizeof(ErrorBuffer));
		else if (n == 3) Res = Driver->ParseShader(Type, LWEUIManager::GetVertexShaderSource(), Allocator, DefineCount, (const char**)DefineMap, nullptr, ErrorBuffer, nullptr, sizeof(ErrorBuffer));
		else if (n == 4) Res = Driver->ParseShader(Type, LWEUIManager::GetTextureShaderSource(), Allocator, DefineCount, (const char**)DefineMap, nullptr, ErrorBuffer, nullptr, sizeof(ErrorBuffer));
		else if (n == 5) Res = Driver->ParseShader(Type, LWEUIManager::GetColorShaderSource(), Allocator, DefineCount, (const char**)DefineMap, nullptr, ErrorBuffer, nullptr, sizeof(ErrorBuffer));
		else if (n == 6) Res = Driver->ParseShader(Type, LWEUIManager::GetYUVTextureShaderSource(), Allocator, DefineCount, (const char**)DefineMap, nullptr, ErrorBuffer, nullptr, sizeof(ErrorBuffer));
		if (Res) Res->SetInputMap(3, "Position", LWShaderInput::Vec4, 1, "Color", LWShaderInput::Vec4, 1, "TexCoord", LWShaderInput::Vec4, 1);
	}
	LWAllocator::Destroy(DelSource);
	if (!Res) {
		std::cout << "Error creating shader '" << NameAttr->m_Value << "':" << std::endl << ErrorBuffer << std::endl;
		return false;
	}
	for (LWEXMLNode *C = N->m_FirstChild; C; C = C->m_Next) {
		uint32_t i = LWText::CompareMultiple(C->m_Name, 3, "InputMap", "ResourceMap", "BlockMap");
		if (i == 0) ParseInputMap(C, Res, AM);
		else if (i == 1) ParseResourceMap(C, Res, AM);
		else if (i == 2) ParseBlockMap(C, Res, AM);
	}
	if (!AM->InsertAsset(NameAttr->m_Value, Res, LWEAsset::Shader, AssetPath)) {
		std::cout << "Error inserting asset: '" << NameAttr->m_Value << "'" << std::endl;
		Driver->DestroyShader(Res);
		return false;
	}
	return true;
}

bool LWEAssetManager::XMLParseShaderBuilder(LWEXMLNode *N, LWEAssetManager *AM) {
	const char *TypeNames[] = { "Vertex", "Pixel", "Geometry", "Compute" };
	const uint32_t MaxDefineValue = 256;
	const uint32_t MaxDefines = 32;
	char DefineList[MaxDefines][MaxDefineValue];
	char *DefineMap[MaxDefines];
	LWShaderInput InputList[LWShader::MaxInputs];
	uint32_t ResourceList[LWShader::MaxResources];
	uint32_t BlockList[LWShader::MaxBlocks];
	uint32_t InputCount = 0;
	uint32_t ResoruceCount = 0;
	uint32_t BlockCount = 0;
	for (uint32_t i = 0; i < MaxDefines; i++) DefineMap[i] = DefineList[i];
	LWXMLAttribute *PathAttr = N->FindAttribute("Path");
	if (!PathAttr) {
		std::cout << "Error no Path attribute specified." << std::endl;
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
		char TypeBuffer[256];
		char NameBuffer[256];
		uint32_t Cnt = 0;
		for (uint32_t i = 0; i < N->m_AttributeCount; i++) {
			LWXMLAttribute &A = N->m_Attributes[i];
			uint32_t NameHash = LWText::MakeHash(A.m_Name);
			uint32_t Length = 1;
			sscanf(A.m_Value, "%[^[][%d]", TypeBuffer, &Length);
			uint32_t TypeHash = LWText::MakeHash(TypeBuffer);
			uint32_t T = 0;
			for (; T < LWShaderInput::Count && TypeNameHashs[T] != TypeHash; T++) {}
			if (T >= LWShaderInput::Count) {
				std::cout << "Error unknown type: '" << A.m_Value << "' for input: '" << A.m_Name << "'" << std::endl;
				continue;
			}
			if (Length > 1) {
				for (uint32_t n = 0; n < Length; n++) {
					snprintf(NameBuffer, sizeof(NameBuffer), "%s[%d]", A.m_Name, n);
					InputList[Cnt++] = LWShaderInput(NameBuffer, T, Length);
				}
			} else {
				InputList[Cnt++] = LWShaderInput(A.m_Name, T, Length);
			}
		}
		return Cnt;
	};

	auto ParseResourceMap = [](LWEXMLNode *N, uint32_t *HashNameList, LWEAssetManager *AM)->uint32_t {
		uint32_t Count = 0;
		for (uint32_t i = 0; i < N->m_AttributeCount; i++) {
			HashNameList[Count++] = LWText::MakeHash(N->m_Attributes[i].m_Name);
		}
		return Count;
	};

	auto ParseBlockMap = [](LWEXMLNode *N, uint32_t *BlockNameList, LWEAssetManager *AM)->uint32_t {
		uint32_t Count = 0;
		for (uint32_t i = 0; i < N->m_AttributeCount; i++) {
			BlockNameList[Count++] = LWText::MakeHash(N->m_Attributes[i].m_Name);
		}
		return Count;
	};

	auto ParseShader = [&DefineMap, &MaxDefines, &MaxDefineValue, &ParseInputMap, &ParseResourceMap, &ParseBlockMap, &InputList, &InputCount, &ResourceList, &ResoruceCount, &BlockList, &BlockCount](LWEXMLNode *N, const char *Source, uint64_t SourcesModifiedTime, LWEAssetManager *AM) ->bool {
		const uint32_t MaxBufferLength = 1024 * 64;//32KB.
		const char *CompiledNames[] = { "lwcv", "lwcp", "lwcg", "lwcc" };
		const char *DriverNames[] = LWVIDEODRIVER_NAMES;
		char CompiledBuffer[MaxBufferLength];
		char ErrorBuffer[MaxBufferLength] = "";
		char AssetPath[256]="";
		LWShaderInput SInputList[LWShader::MaxInputs];
		uint32_t SResourceList[LWShader::MaxResources];
		uint32_t SBlockList[LWShader::MaxBlocks];
		LWXMLAttribute *TypeAttr = N->FindAttribute("Type");
		LWXMLAttribute *CompiledPathAttr = N->FindAttribute("CompiledPath");
		LWXMLAttribute *NameAttr = N->FindAttribute("Name");
		LWXMLAttribute *AutoCompileAttr = N->FindAttribute("AutoCompile");
		LWELocalization *Localize = AM->GetLocalization();
		LWVideoDriver *Driver = AM->GetDriver();
		uint32_t DefineCount = 0;
		uint32_t CompiledLen = 0;
		if (!TypeAttr || !NameAttr) return false;
		for (uint32_t i = 0; i < N->m_AttributeCount; i++) {
			LWXMLAttribute *Attr = &N->m_Attributes[i];
			if (Attr == TypeAttr || Attr == CompiledPathAttr || Attr == NameAttr || Attr == AutoCompileAttr) continue;
			if (*Attr->m_Value) {
				snprintf(DefineMap[DefineCount], MaxDefineValue, "%s:%s", Attr->m_Name, Attr->m_Value);
			} else snprintf(DefineMap[DefineCount], MaxDefineValue, "%s", Attr->m_Name);
			DefineCount++;
		}
		uint32_t Type = LWText::CompareMultiple(TypeAttr->m_Value, 4, "Vertex", "Pixel", "Geometry", "Compute");
		if (Type == -1) {
			std::cout << "Unknown type: '" << TypeAttr->m_Value << "' for shader: '" << NameAttr->m_Value << "'" << std::endl;
			return false;
		}
		
		if (CompiledPathAttr && SourcesModifiedTime) {
			snprintf(AssetPath, sizeof(AssetPath), "%s.%s%s", CompiledPathAttr->m_Value, CompiledNames[Type], DriverNames[AM->GetDriver()->GetDriverID()]);
			LWFileStream Stream;
			if (LWFileStream::OpenStream(Stream, AssetPath, LWFileStream::ReadMode | LWFileStream::BinaryMode, *AM->GetAllocator())) {
				if (Stream.GetModifiedTime() > SourcesModifiedTime) {
					CompiledLen = Stream.Read(CompiledBuffer, sizeof(CompiledBuffer));
				}
			}
		}
		LWShader *Res = nullptr;
		if (CompiledLen) {
			Res = Driver->CreateShaderCompiled(Type, CompiledBuffer, CompiledLen, *AM->GetAllocator(), ErrorBuffer, sizeof(ErrorBuffer));
		} else {
			CompiledLen = sizeof(CompiledBuffer);
			if(AutoCompileAttr && CompiledPathAttr && CompiledLen){
				Res = Driver->ParseShader(Type, Source, *AM->GetAllocator(), DefineCount, (const char**)DefineMap, CompiledBuffer, ErrorBuffer, &CompiledLen, sizeof(ErrorBuffer));
				if (Res) {
					LWFileStream Stream;
					if (!LWFileStream::OpenStream(Stream, AssetPath, LWFileStream::WriteMode | LWFileStream::BinaryMode, *AM->GetAllocator())) {
						std::cout << "Error can't open file for writing: '" << AssetPath << "'" << std::endl;
					} else {
						Stream.Write(CompiledBuffer, CompiledLen);
					}
				}
			} else Res = Driver->ParseShader(Type, Source, *AM->GetAllocator(), DefineCount, (const char**)DefineMap, nullptr, ErrorBuffer, nullptr, sizeof(ErrorBuffer));
			*AssetPath = '\0';
		}
		if (!Res) {
			std::cout << "Error compiling shader '" << NameAttr->m_Value << "':" << std::endl << ErrorBuffer << std::endl;
			return false;
		}
		Res->SetInputMap(InputCount, InputList);
		Res->SetBlockMap(BlockCount, BlockList);
		Res->SetResourceMap(ResoruceCount, ResourceList);
		for (LWEXMLNode *C = N->m_FirstChild; C; C = C->m_Next) {
			uint32_t i = LWText::CompareMultiple(C->m_Name, 3, "InputMap", "ResourceMap", "BlockMap");
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
			std::cout << "Error inserting asset: '" << NameAttr->m_Value << "'" << std::endl;
			AM->GetDriver()->DestroyShader(Res);
			return false;
		}
		return true;
	};
	
	std::function<char*(char*, uint32_t &, uint64_t &, LWAllocator &, LWFileStream *)> ParseSourceFunc;

	auto ParsePath = [&ParseSourceFunc](const char *Path, uint32_t &Length, uint64_t &ModifiedTime, LWAllocator &Allocator, LWFileStream *ExistingStream)->char* {
		LWFileStream Stream;
		if (!LWFileStream::OpenStream(Stream, Path, LWFileStream::ReadMode | LWFileStream::BinaryMode, Allocator, ExistingStream)) {
			std::cout << "Error opening file: '" << Path << "'" << std::endl;
			return nullptr;
		}
		ModifiedTime = std::max<uint64_t>(ModifiedTime, Stream.GetModifiedTime());
		Length = Stream.Length();
		char *Buffer = Allocator.AllocateArray<char>(Length + 1);
		Stream.ReadText(Buffer, Length);
		char *Res = ParseSourceFunc(Buffer, Length, ModifiedTime, Allocator, &Stream);
		if (Res != Buffer) LWAllocator::Destroy(Buffer);
		return Res;
	};

	auto ParseSource = [&ParsePath](char *Source, uint32_t &Length, uint64_t &ModifiedTime, LWAllocator &Allocator, LWFileStream *ExistingStream)->char* {
		char *P = Source;
		char *S = Source;
		char LineBuffer[1024];
		char PathBuffer[1024];
		for (; *S; S++) {
			if (*S == '#') {
				if (LWText::Compare(S, "#include", 8)) {
					const char *C = LWText::CopyToTokens(S + 8, LineBuffer, sizeof(LineBuffer), "\n\r\0");
					if (*C) C += LWText::UTF8ByteSize(C);
					uint32_t LineLen = (uint32_t)(uintptr_t)(C - S);
					const char *B = LWText::FirstToken(LineBuffer, '<');
					if (B) {
						LWText::CopyToTokens(B + 1, PathBuffer, sizeof(PathBuffer), ">");
					} else {
						B = LWText::FirstToken(LineBuffer, '\"');
						if (!B) continue;
						LWText::CopyToTokens(B + 1, PathBuffer, sizeof(PathBuffer), "\"");
					}
					uint32_t Len = 0;
					char *SubBuffer = ParsePath(PathBuffer, Len, ModifiedTime, Allocator, ExistingStream);
					if (!SubBuffer) continue;
					uint32_t Pos = (uint32_t)(uintptr_t)(S - P);
					uint32_t NewLen = (Len + Length - LineLen);
					char *Buf = Allocator.AllocateArray<char>(NewLen + 1);
					std::copy(P, P + Pos, Buf);
					std::copy(SubBuffer, SubBuffer + Len, Buf + Pos);
					std::copy(P + (Pos + LineLen), P + (Length + 1), Buf + (Pos + Len));
					LWAllocator::Destroy(SubBuffer);
					if (P != Source) LWAllocator::Destroy(P);
					P = Buf;
					S = P + (Pos + Len - 1);
					Length = NewLen;
				}
			}
		}
		return P;
	};
	ParseSourceFunc = ParseSource;

	uint64_t ModifiedTime = 0;
	const char *Source = nullptr;
	char *DelSource = nullptr;
	uint32_t n = LWText::CompareMultiple(PathAttr->m_Value, 7, "FontVertex", "FontColor", "FontMSDF", "UIVertex", "UITexture", "UIColor", "UIYUVTexture");
	if (n == -1) {
		uint32_t Len = 0;
		Source = DelSource = ParsePath(PathAttr->m_Value, Len, ModifiedTime, *AM->GetAllocator(), nullptr);
		if (!Source) return false;
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
		uint32_t i = LWText::CompareMultiple(C->m_Name, 4, "Shader", "InputMap", "ResourceMap", "BlockMap");
		if (i == 0) ParseShader(C, Source, ModifiedTime, AM);
		else if (i == 1) InputCount = ParseInputMap(C, InputList, AM);
		else if (i == 2) ResoruceCount = ParseResourceMap(C, ResourceList, AM);
		else if (i == 3) BlockCount = ParseBlockMap(C, BlockList, AM);
	}
	LWAllocator::Destroy(DelSource);
	return true;
}

bool LWEAssetManager::XMLParseVideoBuffer(LWEXMLNode *N, LWEAssetManager *AM) {
	const uint32_t MaxSplits = 8;
	const uint32_t MaxSplitLength = 32;
	const uint32_t UsageFlagList[7] = { LWVideoBuffer::PersistentMapped, LWVideoBuffer::Static, LWVideoBuffer::WriteDiscardable, LWVideoBuffer::WriteNoOverlap,LWVideoBuffer::Readable, LWVideoBuffer::GPUResource, LWVideoBuffer::LocalCopy };
	char UsageSplits[MaxSplits][MaxSplitLength];
	char *UsageMap[MaxSplits];
	for (uint32_t i = 0; i < MaxSplits; i++) UsageMap[i] = UsageSplits[i];

	LWXMLAttribute *NameAttr = N->FindAttribute("Name");
	LWXMLAttribute *TypeAttr = N->FindAttribute("Type");
	LWXMLAttribute *UsageFlagsAttr = N->FindAttribute("Usage");
	LWXMLAttribute *TypeSizeAttr = N->FindAttribute("TypeSize");
	LWXMLAttribute *LengthAttr = N->FindAttribute("Length");
	LWXMLAttribute *PaddedAttr = N->FindAttribute("Padded");
	LWXMLAttribute *DataPathAttr = N->FindAttribute("DataPath");
	uint32_t Longest = 0;
	if (!NameAttr || !TypeAttr || !UsageFlagsAttr || !TypeSizeAttr || !LengthAttr) {
		std::cout << "Error making video buffer: Missing required parameters." << std::endl;
		return false;
	}
	uint32_t TypeID = LWText::CompareMultiple(TypeAttr->m_Value, 5, "Vertex", "Uniform", "Index16", "Index32", "ImageBuffer");
	if (TypeID == -1) {
		std::cout << "Error video buffer '" << NameAttr->m_Value << "' has unknown type: '" << TypeAttr->m_Value << "'" << std::endl;
		return false;
	}
	uint32_t Length = atoi(LengthAttr->m_Value);
	uint32_t TypeSize = atoi(TypeSizeAttr->m_Value);
	if (PaddedAttr) TypeSize = AM->GetDriver()->GetUniformBlockPaddedSize(TypeSize);
	if (!Length) {
		std::cout << "Error video buffer '" << NameAttr->m_Value << "' has 0 length." << std::endl;
		return false;
	}
	if (!TypeSize) {
		std::cout << "Error video buffer '" << NameAttr->m_Value << "' has 0 type size." << std::endl;
		return false;
	}
	uint32_t UsageSplitCnt = LWText::SplitToken(UsageFlagsAttr->m_Value, (char**)UsageMap, MaxSplitLength, MaxSplits, Longest, '|');
	uint32_t UsageFlags = 0;
	for (uint32_t i = 0; i < UsageSplitCnt; i++) {
		uint32_t n = LWText::CompareMultiple(UsageMap[i], 7, "PersistentMapped", "Static", "WriteDiscardable", "WriteNoOverlap", "Readable", "GPUResource", "LocalCopy");
		if (n == -1) {
			std::cout << "Unknown usage flag: '" << UsageMap[i] << "'" << std::endl;
		} else UsageFlags |= UsageFlagList[n];
	}
	uint8_t *Data = nullptr;
	if (DataPathAttr) {
		LWFileStream Stream;
		if (!LWFileStream::OpenStream(Stream, DataPathAttr->m_Value, LWFileStream::ReadMode | LWFileStream::BinaryMode, *AM->GetAllocator())) {
			std::cout << "Error cannot open file: '" << DataPathAttr->m_Value << "'" << std::endl;
		} else {
			Data = AM->GetAllocator()->AllocateArray<uint8_t>(Stream.Length());
			Stream.Read(Data, Stream.Length());
		}
	}

	LWVideoBuffer *Res = AM->GetDriver()->CreateVideoBuffer(TypeID, UsageFlags, TypeSize, Length, *AM->GetAllocator(), Data);
	LWAllocator::Destroy(Data);
	if (!Res) {
		std::cout << "Error creating video buffer '" << NameAttr->m_Value << "'" << std::endl;
		return false;
	}
	if (!AM->InsertAsset(NameAttr->m_Value, Res, LWEAsset::VideoBuffer, "")) {
		std::cout << "Error inserting video buffer '" << NameAttr->m_Value << "'" << std::endl;
		return false;
	}
	return true;
}

bool LWEAssetManager::XMLParseAudioStream(LWEXMLNode *N, LWEAssetManager *AM) {
	char SBuffer[1024 * 32];
	LWXMLAttribute *PathAttr = N->FindAttribute("Path");
	LWXMLAttribute *NameAttr = N->FindAttribute("Name");
	LWXMLAttribute *FlagAttr = N->FindAttribute("Flag");
	LWELocalization *Localize = AM->GetLocalization();
	if (!PathAttr || !NameAttr) return false;
	uint32_t Flag = 0;
	
	const uint32_t FlagValues[] = { LWAudioStream::Decompressed };
	const char FlagNames[][32] = { "Decompressed" };
	const uint32_t TotalValues = sizeof(FlagValues) / sizeof(uint32_t);
	if (FlagAttr) {
		for (char *C = FlagAttr->m_Value; *C; C++) {
			uint32_t i = 0;
			for (; i < TotalValues; i++) {
				if (LWText::Compare(C, FlagNames[i], (uint32_t)strlen(FlagNames[i]))) break;
			}
			if (i >= TotalValues) {
				std::cout << "Encountered invalid flag: '" << C << "' for: '" << NameAttr->m_Value << "'" << std::endl;
			} else {
				Flag |= FlagValues[i];
			}
			C = LWText::FirstToken(C, '|');
			if (!C) break;
		}
	}
	const char *PathValue = PathAttr->m_Value;
	if (Localize) PathValue = Localize->ParseLocalization(SBuffer, sizeof(SBuffer), PathAttr->m_Value);
	LWAudioStream *Stream = LWAudioStream::Create(PathValue, Flag, *AM->GetAllocator());
	if (!Stream) {
		std::cout << "Audiostream: '" << NameAttr->m_Value << "' Could not be found at: '" << PathValue << "'" << std::endl;
		return false;
	}
	if (!AM->InsertAsset(NameAttr->m_Value, Stream, LWEAsset::AudioStream, PathValue)) {
		std::cout << "Name collision with audio stream: '" << NameAttr->m_Value << "'" << std::endl;
		LWAllocator::Destroy(Stream);
	}
	return true;
}

bool LWEAssetManager::XMLParseVideo(LWEXMLNode *N, LWEAssetManager *AM) {
	char SBuffer[1024 * 32];
	LWXMLAttribute *PathAttr = N->FindAttribute("Path");
	LWXMLAttribute *NameAttr = N->FindAttribute("Name");
	LWXMLAttribute *FlagAttr = N->FindAttribute("Flag");
	LWXMLAttribute *LoopCntAttr = N->FindAttribute("LoopCount");
	LWXMLAttribute *PlaybackSpeedAttr = N->FindAttribute("PlaybackSpeed");
	LWELocalization *Localize = AM->GetLocalization();
	if (!PathAttr || !NameAttr) return false;
	uint32_t Flag = 0;
	uint32_t LoopCount = 0;
	float PlaybackSpeed = 1.0f;

	const uint32_t FlagValues[] = { LWEVideoPlayer::Playing };
	const char FlagNames[][32] = { "Playing" };
	const uint32_t TotalValues = sizeof(FlagValues) / sizeof(uint32_t);
	if (FlagAttr) {
		for (char *C = FlagAttr->m_Value; *C; C++) {
			uint32_t i = 0;
			for (uint32_t i = 0; i < TotalValues; i++) {
				if (LWText::Compare(C, FlagNames[i], (uint32_t)strlen(FlagNames[i]))) break;
			}
			if (i >= TotalValues) {
				std::cout << "Encountered invalid flag: '" << C << "' for: '" << NameAttr->m_Value << "'" << std::endl;
			} else {
				Flag |= FlagValues[i];
			}

			C = LWText::FirstToken(C, '|');
			if (!C) break;
		}
	}
	if (LoopCntAttr) LoopCount = atoi(LoopCntAttr->m_Value);
	if (PlaybackSpeedAttr) PlaybackSpeed = (float)atof(PlaybackSpeedAttr->m_Value);
	const char *PathValue = PathAttr->m_Value;
	if (Localize) PathValue = Localize->ParseLocalization(SBuffer, sizeof(SBuffer), PathAttr->m_Value);
	LWEVideoPlayer *Video = AM->GetAllocator()->Allocate<LWEVideoPlayer>();
	if (!LWEVideoPlayer::OpenVideo(*Video, AM->GetDriver(), PathValue, (Flag&LWEVideoPlayer::Playing) != 0, LoopCount, nullptr, nullptr, PlaybackSpeed, *AM->GetAllocator())) {
		std::cout << "Video: '" << NameAttr->m_Value << "' Could not be found at: '" << PathValue << "'" << std::endl;
		LWAllocator::Destroy(Video);
		return false;
	}
	if (!AM->InsertAsset(NameAttr->m_Value, Video, LWEAsset::Video, PathValue)) {
		std::cout << "Name collision with audio stream: '" << NameAttr->m_Value << "'" << std::endl;
		LWAllocator::Destroy(Video);
		return false;
	}
	return true;
}

LWEAsset *LWEAssetManager::GetAsset(const LWText &Name) {
	auto Iter = m_AssetMap.find(Name.GetHash());
	if (Iter == m_AssetMap.end()) {
		std::cout << "Error: Could not find asset '" << Name << "'" << std::endl;
		return nullptr;
	}
	return Iter->second;
}

bool LWEAssetManager::InsertAsset(const LWText &Name, void *Asset, uint32_t AssetType, const char *AssetPath) {
	if (m_AssetCount >= MaxAssets) return false;
	LWEAsset *A = m_AssetTable + m_AssetCount;
	*A = LWEAsset(AssetType, Asset, AssetPath);
	auto Ret = m_AssetMap.insert(std::pair<uint32_t, LWEAsset*>(Name.GetHash(), A));
	if (Ret.second) m_AssetCount++;
	return Ret.second;
}

bool LWEAssetManager::InsertAssetReference(const LWText &Name, const LWText &RefName) {
	LWEAsset *A = GetAsset(RefName);
	if (!A) return false;
	auto Ret = m_AssetMap.insert(std::pair<uint32_t, LWEAsset*>(Name.GetHash(), A));
	return Ret.second;
}

LWVideoDriver *LWEAssetManager::GetDriver(void) {
	return m_Driver;
}

LWELocalization *LWEAssetManager::GetLocalization(void) {
	return m_Localization;
}

LWAllocator *LWEAssetManager::GetAllocator(void) {
	return m_Allocator;
}

LWEAsset *LWEAssetManager::GetAsset(uint32_t i){
	return m_AssetTable + i;
}

uint32_t LWEAssetManager::GetAssetCount(void){
	return m_AssetCount;
}

LWEAssetManager::LWEAssetManager(LWVideoDriver *Driver, LWELocalization *Localization, LWAllocator &Allocator) : m_Driver(Driver), m_Localization(nullptr), m_Allocator(&Allocator), m_AssetCount(0) {}

LWEAssetManager::~LWEAssetManager() {
	for(uint32_t i = 0;i<m_AssetCount;i++){
		LWEAsset *A = m_AssetTable+i;
		uint32_t Type = A->GetType();
		if (Type == LWEAsset::Font) LWAllocator::Destroy(A->AsFont());
		else if (Type == LWEAsset::Texture) m_Driver->DestroyTexture(A->AsTexture());
		else if (Type == LWEAsset::Shader) m_Driver->DestroyShader(A->AsShader());
		else if (Type == LWEAsset::Pipeline) m_Driver->DestroyPipeline(A->AsPipeline());
		else if (Type == LWEAsset::Video) LWAllocator::Destroy(A->AsVideoPlayer());
		else if (Type == LWEAsset::AudioStream) LWAllocator::Destroy(A->AsAudioStream());
		else if (Type == LWEAsset::VideoBuffer) m_Driver->DestroyVideoBuffer(A->AsVideoBuffer());
	}
}

#pragma endregion