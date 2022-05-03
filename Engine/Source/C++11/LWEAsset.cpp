#include "LWEAsset.h"
#include <LWCore/LWUnicode.h>
#include <LWCore/LWLogger.h>
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

const char8_t *LWEShaderSources[LWEShaderCount] = {
	LWShaderSources[LWShaderFontVertex],
#include "LWEShaders/LWEUIVertex.lwvs"
#include "LWEShaders/LWEPP_Vertex.lwvs"
	LWShaderSources[LWShaderFontColor],
	LWShaderSources[LWShaderFontMSDF],
#include "LWEShaders/LWEUITexture.lwps"
#include "LWEShaders/LWEUIColor.lwps"
#include "LWEShaders/LWEUIYUVTexture.lwps"
#include "LWEShaders/LWEPP_Texture.lwps"
#include "LWEShaders/LWEStructures.lws"
#include "LWEShaders/LWEUtilitys.lws"
#include "LWEShaders/LWELightUtilitys.lws"
#include "LWEShaders/LWEGausianBlurVertex.lwvs"
#include "LWEShaders/LWEGausianBlurPixel.lwps"
#include "LWEShaders/LWEDepthVertex.lwvs"
#include "LWEShaders/LWEPBRVertex.lwvs"
#include "LWEShaders/LWEPBRPixel.lwps"
#include "LWEShaders/LWEPP_PBRComposite.lwps"
};

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
	LWEXMLAttribute *CompiledShaderCacheAttr = N->FindAttribute("CompiledShaderCache");
	if (CompiledShaderCacheAttr) AM->SetCompiledShaderCache(CompiledShaderCacheAttr->GetValue());
	for (LWEXMLNode *C = XML->NextNode(nullptr, N); C; C = XML->NextNode(C, N, true)) {
		uint32_t i = C->GetName().CompareList("Texture", "Font", "Shader", "Video", "AudioStream", "Pipeline", "VideoBuffer", "ShaderBuilder", "PipelineBuilder", "Reference");
		bool Loaded = false;
		if (i == LWEAsset::Texture) Loaded = XMLParseTexture(C, AM);
		else if (i == LWEAsset::Font) Loaded = XMLParseFont(C, AM);
		else if (i == LWEAsset::Shader) Loaded = XMLParseShader(C, AM);
		else if (i == LWEAsset::Video) Loaded = XMLParseVideo(C, AM);
		else if (i == LWEAsset::AudioStream) Loaded = XMLParseAudioStream(C, AM);
		else if (i == LWEAsset::Pipeline) Loaded = XMLParsePipeline(C, AM);
		else if (i == LWEAsset::VideoBuffer) Loaded = XMLParseVideoBuffer(C, AM);
		else if (i == 7) Loaded = XMLParseShaderBuilder(C, AM);
		else if (i == 8) Loaded = XMLParsePipelineBuilder(C, AM);
		else if (i == 9) {
			LWEXMLAttribute *NameAttr = C->FindAttribute("Name");
			LWEXMLAttribute *RefAttr = C->FindAttribute("Ref");
			if (NameAttr && RefAttr) Loaded = AM->InsertAssetReference(NameAttr->GetValue(), RefAttr->GetValue());
		}
		if (!Loaded) {
			LWEXMLAttribute *NameAttr = C->FindAttribute("Name");
			LWLogCritical<256>("Failed to load asset: '{}'", NameAttr ? NameAttr->GetValue() : C->m_Name);
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

	if(!LWLogCriticalIf<256>(LWFileStream::OpenStream(FontFile, Path, LWFileStream::ReadMode | ((ExtType == 0 || ExtType==2) ? LWFileStream::BinaryMode : 0), Alloc), "Failed to open font file: '{}'", Path)) return false; 

	if (ExtType == 0) F = LWFont::LoadFontTTF(&FontFile, AM->GetDriver(), Size, GlyphCount, GFirstList, GLengthList, Alloc);
	else if (ExtType == 1) F = LWFont::LoadFontFNT(&FontFile, AM->GetDriver(), Alloc);
	else if (ExtType == 2) F = LWFont::LoadFontAR(&FontFile, AM->GetDriver(), Alloc);

	if(!LWLogCriticalIf<256>(F, "Failed to create font file '{}", Path)) return false;
	if(!LWLogCriticalIf<256>(AM->InsertAsset(NameAttr->m_Value, F, LWEAsset::Font, Path), "Failed to insert asset: '{}'", NameAttr->GetValue())) {
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
	const uint32_t FlagValues[] = { LWTexture::MinNearestMipmapNearest, LWTexture::MinLinearMipmapNearest, LWTexture::MinNearestMipmapLinear, LWTexture::MinLinearMipmapLinear, LWTexture::MinNearest, LWTexture::MagNearest, LWTexture::WrapSClampToEdge, LWTexture::WrapTClampToEdge, LWTexture::WrapRClampToEdge, LWTexture::CompareNone, LWTexture::MinLinear, LWTexture::MagLinear, LWTexture::WrapSClampToBorder, LWTexture::WrapSMirroredRepeat, LWTexture::WrapSRepeat, LWTexture::WrapTClampToBorder, LWTexture::WrapTMirroredRepeat, LWTexture::WrapTRepeat, LWTexture::WrapRClampToBorder, LWTexture::WrapRMirroredRepeat, LWTexture::WrapRRepeat, LWTexture::RenderTarget, LWTexture::RenderBuffer, LWTexture::Anisotropy_2x, LWTexture::Anisotropy_4x, LWTexture::Anisotropy_8x, LWTexture::Anisotropy_16x };
	const char8_t FlagNames[][32] = { "MinNearestMipmapNearest",          "MinLinearMipmapNearest",          "MinNearestMipmapLinear",          "MinLinearMipmapLinear",          "MinNearest",          "MagNearest",          "WrapSClampToEdge",          "WrapTClampToEdge",          "WrapRClampToEdge",          "CompareNone",          "MinLinear",          "MagLinear",          "WrapSClampToBorder",          "WrapSMirroredRepeat",          "WrapSRepeat",          "WrapTClampToBorder",          "WrapTMirroredRepeat",          "WrapTRepeat",          "WrapRClampToBorder",          "WrapRMirroredRepeat",          "WrapRRepeat",          "RenderTarget",          "RenderBuffer",        "Anisotropy 2x",          "Anisotropy 4x",          "Anisotropy 8x",          "Anisotropy 16x" };
	const uint32_t TotalFlags = sizeof(FlagValues) / sizeof(uint32_t);
	LWVideoDriver *Driver = AM->GetDriver();
	LWAllocator &Alloc = AM->GetAllocator();

	LWELocalization *Localize = AM->GetLocalization();
	LWEXMLAttribute *PathAttr = N->FindAttribute("Path");
	LWEXMLAttribute *NameAttr = N->FindAttribute("Name");
	LWEXMLAttribute *StateAttr = N->FindAttribute("State");
	LWEXMLAttribute *LinearAttr = N->FindAttribute("Linear");
	if (!PathAttr || !NameAttr) return false;
	LWUTF8Iterator Path = PathAttr->GetValue();
	if (Localize && Localize->ParseLocalization(SBuffer, sizeof(SBuffer), Path)) Path = LWUTF8Iterator(SBuffer);
	LWImage Image;
	if(!LWLogCriticalIf<256>(LWImage::LoadImage(Image, Path, AM->GetAllocator()), "Failed to load image: '{}'", Path)) return false;
	uint32_t TextureState = 0;
	if (StateAttr) {
		uint32_t StateCnt = std::min<uint32_t>(StateAttr->GetValue().SplitToken(FlagIterList, MaxFlagIters, '|'), MaxFlagIters);
		for (uint32_t i = 0; i < StateCnt; i++) {
			uint32_t FlagID = FlagIterList[i].AdvanceWord(true).CompareLista(TotalFlags, FlagNames);
			if(LWLogWarnIf<256>(FlagID!=-1, "Encountered invalid flag: '{}' for '{}'", FlagIterList[i], NameAttr->GetValue())) TextureState |= FlagValues[FlagID];
		}
	}
	Image.SetSRGBA(LinearAttr == nullptr);
	LWTexture *Tex = Driver->CreateTexture(TextureState, Image, Alloc);
	if(!LWLogCriticalIf<256>(AM->InsertAsset(NameAttr->GetValue(), Tex, LWEAsset::Texture, Path), "Failed to insert asset: '{}'", NameAttr->GetValue())) {
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
		if(!LWLogCriticalIf<256>(NameAttr, "Block '{}' does not have 'Name' Attribute.", N->GetName())) return;
		if(!LWLogCriticalIf<256>(SlotAttr || SlotNameAttr, "Block '{}' does not have either 'Slot' or 'SlotName' attribute.", N->GetName())) return;

		LWVideoBuffer *B = AM->GetAsset<LWVideoBuffer>(NameAttr->GetValue());
		if(!LWLogCriticalIf<256>(B, "Block '{}' could not find buffer: '{}'", N->GetName(), NameAttr->GetValue())) return;

		uint32_t SlotIdx = -1;
		if (SlotAttr) SlotIdx = atoi(SlotAttr->m_Value);
		else if (SlotNameAttr) SlotIdx = P->FindBlock(SlotNameAttr->m_Value);
		if(!LWLogCriticalIf<256>(SlotIdx!=-1, "Block '{}' slot '{}' could not be found.", N->GetName(), SlotAttr ? SlotAttr->GetValue() : SlotNameAttr->GetValue())) return;

		uint32_t Offset = OffsetAttr ? atoi(OffsetAttr->m_Value) : 0;
		P->SetUniformBlock(SlotIdx, B, Offset);
		return;
	};

	auto ParseResourceBinding = [](LWEXMLNode *N, LWPipeline *P, LWEAssetManager *AM) {
		LWEXMLAttribute *NameAttr = N->FindAttribute("Name");
		LWEXMLAttribute *OffsetAttr = N->FindAttribute("Offset");
		LWEXMLAttribute *SlotAttr = N->FindAttribute("Slot");
		LWEXMLAttribute *SlotNameAttr = N->FindAttribute("SlotName");
		if (!LWLogCriticalIf<256>(NameAttr, "Block '{}' does not have 'Name' Attribute.", N->GetName())) return;
		if (!LWLogCriticalIf<256>(SlotAttr || SlotNameAttr, "Block '{}' does not have either 'Slot' or 'SlotName' attribute.", N->GetName())) return;

		LWVideoBuffer *B = AM->GetAsset<LWVideoBuffer>(NameAttr->m_Value);
		LWTexture *T = AM->GetAsset<LWTexture>(NameAttr->m_Value);
		if (!LWLogCriticalIf<256>(B || T, "Block '{}' could not find buffer or texture: '{}'", N->GetName(), NameAttr->GetValue())) return;

		uint32_t SlotIdx = -1;
		if (SlotAttr) SlotIdx = atoi(SlotAttr->m_Value);
		else if (SlotNameAttr) SlotIdx = P->FindResource(SlotNameAttr->m_Value);
		if (!LWLogCriticalIf<256>(SlotIdx != -1, "Block '{}' resource slot '{}' could not be found.", N->GetName(), SlotAttr ? SlotAttr->GetValue() : SlotNameAttr->GetValue())) return;

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

	if(!LWLogCriticalIf(NameAttr, "Pipeline has no name attribute.")) return false;
	if(!LWLogCriticalIf<256>(VertexAttr || ComputeAttr, "Pipeline '{}' must have either a vertex shader, or compute shader.", NameAttr->GetValue())) return false;

	uint64_t Flags = 0;

	LWShader *VS = VertexAttr ? AM->GetAsset<LWShader>(VertexAttr->GetValue()) : nullptr;
	LWShader *PS = PixelAttr ? AM->GetAsset<LWShader>(PixelAttr->GetValue()) : nullptr;
	LWShader *GS = GeometryAttr ? AM->GetAsset<LWShader>(GeometryAttr->GetValue()) : nullptr;
	LWShader *CS = ComputeAttr ? AM->GetAsset<LWShader>(ComputeAttr->GetValue()) : nullptr;
	if(!LWLogCriticalIf<256>(!VertexAttr || VS, "Pipeline '{}' could not find vertex shader: '{}'", NameAttr->GetValue(), VertexAttr->GetValue())) return false;
	if(!LWLogCriticalIf<256>(!PixelAttr || PS, "Pipeline '{}' could not find pixel shader: '{}'", NameAttr->GetValue(), PixelAttr->GetValue())) return false;
	if(!LWLogCriticalIf<256>(!GeometryAttr || GS, "Pipeline {} could not find geometry shader: '{}'", NameAttr->GetValue(), PixelAttr->GetValue())) return false;
	if(!LWLogCriticalIf<256>(!ComputeAttr || CS, "Pipeline '{}' could not find computer shader: '{}'", NameAttr->GetValue(), ComputeAttr->GetValue())) return false;

	if (FlagAttr) {
		uint32_t FlagCnt = std::min<uint32_t>(FlagAttr->GetValue().SplitToken(FlagIterList, MaxFlagIters, '|'), MaxFlagIters);
		for (uint32_t i = 0; i < FlagCnt; i++) {
			uint32_t n = FlagIterList[i].AdvanceWord(true).CompareLista(FlagCount, FlagNames);
			if(LWLogWarnIf<256>(n!=-1, "Pipeline '{}' encountered unknown flag: '{}'", NameAttr->GetValue(), FlagIterList[i])) Flags |=FlagValues[n];
		}
	}
	if (FillModeAttr) {
		uint32_t n = FillModeAttr->GetValue().CompareLista(2, FillNames);
		if(LWLogWarnIf<256>(n!=-1, "Pipeline '{}' encountered unknown fill mode: '{}'", NameAttr->GetValue(), FillModeAttr->GetValue())) Flags |= (FillValues[n]<<LWPipeline::FILL_MODE_BITOFFSET);
	}
	if (CullModeAttr) {
		uint32_t n = CullModeAttr->GetValue().CompareLista(3, CullNames);
		if(LWLogWarnIf<256>(n!=-1, "Pipeline '{}' encountered unknown cull mode: '{}'", NameAttr->GetValue(), CullModeAttr->GetValue())) Flags |= (CullValues[n]<<LWPipeline::CULL_BITOFFSET);
	}
	if (DepthCompareAttr) {
		uint32_t n = DepthCompareAttr->GetValue().CompareLista(6, CompareNames);
		if(LWLogWarnIf<256>(n!=-1, "Pipeline '{}' encountered unknown depth compare mode: '{}'", NameAttr->GetValue(), DepthCompareAttr->GetValue())) Flags |= (CompareValues[n] << LWPipeline::DEPTH_COMPARE_BITOFFSET);
	}
	if (SourceBlendModeAttr) {
		uint32_t n = SourceBlendModeAttr->GetValue().CompareLista(10, BlendNames);
		if(LWLogWarnIf<256>(n!=-1, "Pipeline '{}' encountered unknown source blend mode: '{}'", NameAttr->GetValue(), SourceBlendModeAttr->GetValue())) Flags |= (BlendValues[n]<<LWPipeline::BLEND_SRC_BITOFFSET);
	}
	if (DestBlendModeAttr) {
		uint32_t n = DestBlendModeAttr->GetValue().CompareLista(10, BlendNames);
		if(LWLogWarnIf<256>(n!=-1, "Pipeline '{}' encountered unknown dest blend mode: '{}'", NameAttr->GetValue(), DestBlendModeAttr->GetValue())) Flags |= (BlendValues[n] << LWPipeline::BLEND_DST_BITOFFSET);
	}
	if (StencilCompareAttr) {
		uint32_t n = StencilCompareAttr->GetValue().CompareLista(6, CompareNames);
		if(LWLogWarnIf<256>(n!=-1, "Pipeline '{}' encountered unknown stencil compare mode: '{}'", NameAttr->GetValue(), StencilCompareAttr->GetValue())) Flags |= (CompareValues[n]<<LWPipeline::STENCIL_COMPARE_BITOFFSET);
	}
	if (StencilOpFailAttr) {
		uint32_t n = StencilOpFailAttr->GetValue().CompareLista(8, StencilOpNames);
		if(LWLogWarnIf<256>(n!=-1, "Pipeline '{}' encountered unknown stencil op fail mode: '{}'", NameAttr->GetValue(), StencilOpFailAttr->GetValue())) Flags |= (StencilOpValues[n] << LWPipeline::STENCIL_OP_SFAIL_BITOFFSET);
	}
	if (StencilOpDFailAttr) {
		uint32_t n = StencilOpDFailAttr->GetValue().CompareLista(8, StencilOpNames);
		if(LWLogWarnIf<256>(n!=-1, "Pipeline '{}' encountered unknown stencil op depth fail mode: '{}'", NameAttr->GetValue(), StencilOpDFailAttr->GetValue())) Flags |= (StencilOpValues[n] << LWPipeline::STENCIL_OP_DFAIL_BITOFFSET);
	}
	if (StencilOpPassAttr) {
		uint32_t n = StencilOpPassAttr->GetValue().CompareLista(8, StencilOpNames);
		if(LWLogWarnIf<256>(n!=-1, "Pipeline '{}' encountered unknown stencil op pass mode: '{}'", NameAttr->GetValue(), StencilOpPassAttr->GetValue())) Flags |= (StencilOpValues[n] << LWPipeline::STENCIL_OP_PASS_BITOFFSET);
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
	if(!LWLogCriticalIf<256>(P, "Failed to create pipeline for: '{}'", NameAttr->GetValue())) return false;
	
	P->SetDepthBias((P->GetFlag()&LWPipeline::DEPTH_BIAS), DepthBiasAttr ? (float)atof(DepthBiasAttr->m_Value) : 0.0f, DepthSlopedBiasAttr ? (float)atof(DepthSlopedBiasAttr->m_Value) : 0.0f);
	for (LWEXMLNode *C = N->m_FirstChild; C; C = C->m_Next) {
		uint32_t i = C->GetName().CompareList("Resource", "Block");
		if (i == 0) ParseResourceBinding(C, P, AM);
		else if (i == 1) ParseBlockBinding(C, P, AM);
		else LWLogCritical<256>("Pipeline '{}' has unknown child node: '{}'", NameAttr->GetValue(), C->GetName());
	}
	if(!LWLogCriticalIf<256>(AM->InsertAsset(NameAttr->GetValue(), P, LWEAsset::Pipeline, ""), "Failed to insert asset: '{}'", NameAttr->GetValue())) {
		Driver->DestroyPipeline(P);
		return false;
	}
	return true;
}

bool LWEAssetManager::XMLParsePipelineBuilder(LWEXMLNode *N, LWEAssetManager *AM) {
	LWVideoDriver *Driver = AM->GetDriver();
	LWAllocator &Alloc = AM->GetAllocator();

	auto ParseBlockBinding = [](LWEXMLNode *N, LWPipeline *P, LWEAssetManager *AM) {
		LWEXMLAttribute *NameAttr = N->FindAttribute("Name");
		LWEXMLAttribute *OffsetAttr = N->FindAttribute("Offset");
		LWEXMLAttribute *SlotAttr = N->FindAttribute("Slot");
		LWEXMLAttribute *SlotNameAttr = N->FindAttribute("SlotName");
		if (!LWLogCriticalIf<256>(NameAttr, "Block '{}' does not have 'Name' Attribute.", N->GetName())) return;
		if (!LWLogCriticalIf<256>(SlotAttr || SlotNameAttr, "Block '{}' does not have either 'Slot' or 'SlotName' attribute.", N->GetName())) return;

		LWVideoBuffer *B = AM->GetAsset<LWVideoBuffer>(NameAttr->GetValue());
		if (!LWLogCriticalIf<256>(B, "Block '{}' could not find buffer: '{}'", N->GetName(), NameAttr->GetValue())) return;

		uint32_t SlotIdx = -1;
		if (SlotAttr) SlotIdx = atoi(SlotAttr->m_Value);
		else if (SlotNameAttr) SlotIdx = P->FindBlock(SlotNameAttr->m_Value);
		if (!LWLogCriticalIf<256>(SlotIdx != -1, "Block '{}' slot '{}' could not be found.", N->GetName(), SlotAttr ? SlotAttr->GetValue() : SlotNameAttr->GetValue())) return;

		uint32_t Offset = OffsetAttr ? atoi(OffsetAttr->m_Value) : 0;
		P->SetUniformBlock(SlotIdx, B, Offset);
		return;
	};

	auto ParseResourceBinding = [](LWEXMLNode *N, LWPipeline *P, LWEAssetManager *AM) {
		LWEXMLAttribute *NameAttr = N->FindAttribute("Name");
		LWEXMLAttribute *OffsetAttr = N->FindAttribute("Offset");
		LWEXMLAttribute *SlotAttr = N->FindAttribute("Slot");
		LWEXMLAttribute *SlotNameAttr = N->FindAttribute("SlotName");
		if (!LWLogCriticalIf<256>(NameAttr, "Block '{}' does not have 'Name' Attribute.", N->GetName())) return;
		if (!LWLogCriticalIf<256>(SlotAttr || SlotNameAttr, "Block '{}' does not have either 'Slot' or 'SlotName' attribute.", N->GetName())) return;

		LWVideoBuffer *B = AM->GetAsset<LWVideoBuffer>(NameAttr->m_Value);
		LWTexture *T = AM->GetAsset<LWTexture>(NameAttr->m_Value);
		if (!LWLogCriticalIf<256>(B || T, "Block '{}' could not find buffer or texture: '{}'", N->GetName(), NameAttr->GetValue())) return;

		uint32_t SlotIdx = -1;
		if (SlotAttr) SlotIdx = atoi(SlotAttr->m_Value);
		else if (SlotNameAttr) SlotIdx = P->FindResource(SlotNameAttr->m_Value);
		if (!LWLogCriticalIf<256>(SlotIdx != -1, "Block '{}' resource slot '{}' could not be found.", N->GetName(), SlotAttr ? SlotAttr->GetValue() : SlotNameAttr->GetValue())) return;

		uint32_t Offset = OffsetAttr ? atoi(OffsetAttr->m_Value) : 0;
		if (B) P->SetResource(SlotIdx, B, Offset);
		else P->SetResource(SlotIdx, T);
		return;
	};

	auto ParseRasterState = [](LWEXMLNode *N, uint64_t DefaultFlags, float &DepthBias, float &DepthSlopeBias) -> uint64_t {
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


		if (FlagAttr) {
			uint32_t FlagCnt = std::min<uint32_t>(FlagAttr->GetValue().SplitToken(FlagIterList, MaxFlagIters, '|'), MaxFlagIters);
			for (uint32_t i = 0; i < FlagCnt; i++) {
				uint32_t n = FlagIterList[i].AdvanceWord(true).CompareLista(FlagCount, FlagNames);
				if(LWLogWarnIf<256>(n!=-1, "PipelineBuilder encountered unknown flag: '{}'", FlagIterList[i])) DefaultFlags|=FlagValues[n];
			}
		}
		if (FillModeAttr) {
			uint32_t n = FillModeAttr->GetValue().CompareLista(2, FillNames);
			if (LWLogWarnIf<256>(n != -1, "PipelineBuilder encountered unknown fill mode: '{}'", FillModeAttr->GetValue())) DefaultFlags |= (FillValues[n] << LWPipeline::FILL_MODE_BITOFFSET);
		}
		if (CullModeAttr) {
			uint32_t n = CullModeAttr->GetValue().CompareLista(3, CullNames);
			if (LWLogWarnIf<256>(n != -1, "PipelineBuilder encountered unknown cull mode: '{}'", CullModeAttr->GetValue())) DefaultFlags |= (CullValues[n] << LWPipeline::CULL_BITOFFSET);
		}
		if (DepthCompareAttr) {
			uint32_t n = DepthCompareAttr->GetValue().CompareLista(6, CompareNames);
			if (LWLogWarnIf<256>(n != -1, "PipelineBuilder encountered unknown depth compare mode: '{}'", DepthCompareAttr->GetValue())) DefaultFlags |= (CompareValues[n] << LWPipeline::DEPTH_COMPARE_BITOFFSET);
		}
		if (SourceBlendModeAttr) {
			uint32_t n = SourceBlendModeAttr->GetValue().CompareLista(10, BlendNames);
			if (LWLogWarnIf<256>(n != -1, "PipelineBuilder encountered unknown source blend mode: '{}'", SourceBlendModeAttr->GetValue())) DefaultFlags |= (BlendValues[n] << LWPipeline::BLEND_SRC_BITOFFSET);
		}
		if (DestBlendModeAttr) {
			uint32_t n = DestBlendModeAttr->GetValue().CompareLista(10, BlendNames);
			if (LWLogWarnIf<256>(n != -1, "PipelineBuilder encountered unknown dest blend mode: '{}'", DestBlendModeAttr->GetValue())) DefaultFlags |= (BlendValues[n] << LWPipeline::BLEND_DST_BITOFFSET);
		}
		if (StencilCompareAttr) {
			uint32_t n = StencilCompareAttr->GetValue().CompareLista(6, CompareNames);
			if (LWLogWarnIf<256>(n != -1, "PipelineBuilder encountered unknown stencil compare mode: '{}'", StencilCompareAttr->GetValue())) DefaultFlags |= (CompareValues[n] << LWPipeline::STENCIL_COMPARE_BITOFFSET);
		}
		if (StencilOpFailAttr) {
			uint32_t n = StencilOpFailAttr->GetValue().CompareLista(8, StencilOpNames);
			if (LWLogWarnIf<256>(n != -1, "PipelineBuilder encountered unknown stencil op fail mode: '{}'", StencilOpFailAttr->GetValue())) DefaultFlags |= (StencilOpValues[n] << LWPipeline::STENCIL_OP_SFAIL_BITOFFSET);
		}
		if (StencilOpDFailAttr) {
			uint32_t n = StencilOpDFailAttr->GetValue().CompareLista(8, StencilOpNames);
			if (LWLogWarnIf<256>(n != -1, "PipelineBuilder encountered unknown stencil op depth fail mode: '{}'", StencilOpDFailAttr->GetValue())) DefaultFlags |= (StencilOpValues[n] << LWPipeline::STENCIL_OP_DFAIL_BITOFFSET);
		}
		if (StencilOpPassAttr) {
			uint32_t n = StencilOpPassAttr->GetValue().CompareLista(8, StencilOpNames);
			if (LWLogWarnIf<256>(n != -1, "PipelineBuilder encountered unknown stencil op pass mode: '{}'", StencilOpPassAttr->GetValue())) DefaultFlags |= (StencilOpValues[n] << LWPipeline::STENCIL_OP_PASS_BITOFFSET);
		}
		if (StencilRefValueAttr) {
			uint64_t Value = (uint64_t)atoi(StencilRefValueAttr->m_Value);
			Value = std::max<uint64_t>(Value, 255);
			DefaultFlags = (DefaultFlags&~LWPipeline::STENCIL_REF_VALUE_BITS) | (Value << LWPipeline::STENCIL_REF_VALUE_BITOFFSET);
		}
		if (StencilReadMaskValueAttr) {
			uint64_t Value = (uint64_t)atoi(StencilReadMaskValueAttr->m_Value);
			Value = std::max<uint64_t>(Value, 255);
			DefaultFlags = (DefaultFlags & ~LWPipeline::STENCIL_READMASK_BITS) | (Value << LWPipeline::STENCIL_READMASK_BITOFFSET);
		}
		if (StencilWriteMaskValueAttr) {
			uint64_t Value = (uint64_t)atoi(StencilWriteMaskValueAttr->m_Value);
			Value = std::max<uint64_t>(Value, 255);
			DefaultFlags = (DefaultFlags & ~LWPipeline::STENCIL_WRITEMASK_BITS) | (Value << LWPipeline::STENCIL_WRITEMASK_BITOFFSET);
		}
		if (DepthBiasAttr) DepthBias = (float)atof(DepthBiasAttr->GetValue().c_str());
		if (DepthSlopedBiasAttr) DepthSlopeBias = (float)atof(DepthSlopedBiasAttr->GetValue().c_str());
		return DefaultFlags;
	};

	auto ParsePipeline = [&Driver, &Alloc, &ParseRasterState, &ParseBlockBinding, &ParseResourceBinding](LWEXMLNode *N, LWEAssetManager *AM, uint64_t DefaultFlags,float DepthBias, float DepthSlopeBias, LWEXMLNode *BlockNode, LWEXMLNode *ResourceNode)->bool {
		LWEXMLAttribute *NameAttr = N->FindAttribute("Name");
		LWEXMLAttribute *VertexAttr = N->FindAttribute("Vertex");
		LWEXMLAttribute *ComputeAttr = N->FindAttribute("Compute");
		LWEXMLAttribute *GeometryAttr = N->FindAttribute("Geometry");
		LWEXMLAttribute *PixelAttr = N->FindAttribute("Pixel");
		if(!LWLogCriticalIf<256>(NameAttr, "Pipeline missing 'Name' attribute.")) return false;

		LWShader *VS = VertexAttr ? AM->GetAsset<LWShader>(VertexAttr->GetValue()) : nullptr;
		LWShader *PS = PixelAttr ? AM->GetAsset<LWShader>(PixelAttr->GetValue()) : nullptr;
		LWShader *GS = GeometryAttr ? AM->GetAsset<LWShader>(GeometryAttr->GetValue()) : nullptr;
		LWShader *CS = ComputeAttr ? AM->GetAsset<LWShader>(ComputeAttr->GetValue()) : nullptr;
		if (!LWLogCriticalIf<256>(!VertexAttr || VS, "Pipeline '{}' could not find vertex shader: '{}'", NameAttr->GetValue(), VertexAttr->GetValue())) return false;
		if (!LWLogCriticalIf<256>(!PixelAttr || PS, "Pipeline '{}' could not find pixel shader: '{}'", NameAttr->GetValue(), PixelAttr->GetValue())) return false;
		if (!LWLogCriticalIf<256>(!GeometryAttr || GS, "Pipeline {} could not find geometry shader: '{}'", NameAttr->GetValue(), PixelAttr->GetValue())) return false;
		if (!LWLogCriticalIf<256>(!ComputeAttr || CS, "Pipeline '{}' could not find computer shader: '{}'", NameAttr->GetValue(), ComputeAttr->GetValue())) return false;

		DefaultFlags = ParseRasterState(N, DefaultFlags, DepthBias, DepthSlopeBias);

		LWPipeline *P = nullptr;
		if (VS) P = Driver->CreatePipeline(VS, GS, PS, DefaultFlags, Alloc);
		else if (CS) P = Driver->CreatePipeline(CS, Alloc);
		if (!LWLogCriticalIf<256>(P, "Failed to create pipeline for: '{}'", NameAttr->GetValue())) return false;

		if (BlockNode) ParseBlockBinding(BlockNode, P, AM);
		if (ResourceNode) ParseResourceBinding(ResourceNode, P, AM);
		P->SetDepthBias((DefaultFlags & LWPipeline::DEPTH_BIAS), DepthBias, DepthSlopeBias);
		for (LWEXMLNode *C = N->m_FirstChild; C; C = C->m_Next) {
			uint32_t i = C->GetName().CompareList("Resource", "Block");
			if (i == 0) ParseBlockBinding(C, P, AM);
			else if (i == 1) ParseResourceBinding(C, P, AM);
		}

		if (!LWLogCriticalIf<256>(AM->InsertAsset(NameAttr->GetValue(), P, LWEAsset::Pipeline, ""), "Failed to insert asset: '{}'", NameAttr->GetValue())) {
			Driver->DestroyPipeline(P);
			return false;
		}
		return true;
	};

	float DefDepthBias = 0.0f;
	float DefDepthSlopeBias = 0.0f;
	uint64_t DefaultRaster = ParseRasterState(N, 0, DefDepthBias, DefDepthSlopeBias);

	LWEXMLNode *BlockNode = nullptr;
	LWEXMLNode *ResourceNode = nullptr;
	for (LWEXMLNode *C = N->m_FirstChild; C; C = C->m_Next) {
		uint32_t i = C->GetName().CompareList("Pipeline", "Resource", "Block");
		if (i == 0) ParsePipeline(C, AM, DefaultRaster, DefDepthBias, DefDepthSlopeBias, BlockNode, ResourceNode);
		else if (i == 1) BlockNode = C;
		else if (i == 2) ResourceNode = C;
		else {
			LWLogCritical<256>("PipelineBuilder has unknown child node: '{}'", C->GetName());
		}		
	}
	return true;
}

bool LWEAssetManager::XMLParseShader(LWEXMLNode *N, LWEAssetManager *AM) {
	const char *DriverNames[] = LWVIDEODRIVER_NAMES;
	const char *TypeNames[] = { "Vertex", "Geometry", "Pixel", "Compute" };
	const char *CompiledNames[] = { "lwcv", "lwcg", "lwcp", "lwcc" };
	const uint32_t MaxFileBufferLen = 256 * 1024;
	const uint32_t MaxBufferLen = 1024;
	const uint32_t MaxModules = 5;
	const uint32_t MaxNameLen = 32;
	const uint32_t MaxPathLen = 256;
	const uint32_t MaxDefines = 32;
	const uint32_t MaxErrorBufferLen = 4 * 1024;
	const uint32_t TotalDriverCnt = 10;
	LWUTF8C_View<MaxBufferLen> Buffer;
	LWUTF8C_View<MaxBufferLen> CBuffer;
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
			LWEXMLAttribute &A = N->m_Attributes[i];
			LWUTF8Iterator TypeIter = A.GetValue();
			LWUTF8Iterator BracketIter = TypeIter.NextToken('[');
			LWUTF8Iterator InstanceFreqIter = TypeIter.NextToken(':');

			uint32_t NameHash = A.GetName().Hash();
			uint32_t Length = BracketIter.AtEnd()?1:atoi(BracketIter.c_str()+1);
			uint32_t InstanceFreq = InstanceFreqIter.AtEnd() ? 0 : atoi(InstanceFreqIter.c_str() + 1);
			TypeIter = LWUTF8Iterator(TypeIter, BracketIter.AtEnd() ? InstanceFreqIter : BracketIter);
			
			uint32_t TypeHash = TypeIter.Hash();
			uint32_t T = 0;
			for (; T < LWShaderInput::Count && TypeNameHashs[T] != TypeHash; T++) {}
			if(!LWLogCriticalIf<256>(T<LWShaderInput::Count, "Unknown type: '{}' for input: '{}", A.GetValue(), A.GetName())) continue;

			Inputs[InputCount++] = LWShaderInput(A.m_Name, T, Length, InstanceFreq);
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

	std::function<LWUTF8Iterator(const LWUTF8Iterator &, uint32_t &, LWAllocator &, LWFileStream *)> ParseSourceFunc;

	auto ParsePath = [&ParseSourceFunc](const LWUTF8Iterator &Path, uint32_t &Length, LWAllocator &Allocator, LWFileStream *ExistingStream)->LWUTF8Iterator {
		uint32_t n = Path.CompareLista(LWEShaderCount, LWEShaderNames);
		char8_t *Buffer = nullptr;
		LWFileStream Stream;
		if (n != -1) {
			Length = (uint32_t)strlen(LWEShaderSources[n]) + 1;
			Buffer = Allocator.Allocate<char8_t>(Length);
			std::copy(LWEShaderSources[n], LWEShaderSources[n] + Length, Buffer);
		} else {
			if(!LWLogCriticalIf<256>(LWFileStream::OpenStream(Stream, Path, LWFileStream::ReadMode | LWFileStream::BinaryMode, Allocator, ExistingStream), "Failed to open file: '{}'", Path)) return nullptr;

			Length = Stream.Length() + 1;
			Buffer = Allocator.Allocate<char8_t>(Length);
			if(!LWLogCriticalIf<256>(Stream.ReadText(Buffer, Length)==Length, "Failed to read entire buffer for '{}'", Path)) {
				LWAllocator::Destroy(Buffer);
				return nullptr;
			}
		}
		LWUTF8Iterator Res = ParseSourceFunc(Buffer, Length, Allocator, n == -1 ? &Stream : ExistingStream);
		if (Res() != Buffer) LWAllocator::Destroy(Buffer);
		return Res;
	};

	auto ParseSource = [&ParsePath](const LWUTF8Iterator &Source, uint32_t &Length, LWAllocator &Allocator, LWFileStream *ExistingStream)->LWUTF8Iterator {
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
					LWUTF8Iterator SubSource = ParsePath(Path, Len, Allocator, ExistingStream);
					if(!SubSource.isInitialized()) continue;
					uint32_t NewLen = (Len + Length - C.RawDistance(--N)) - 1; //-1 because both Len, and Length include a null character.
					char8_t *Buf = Allocator.Allocate<char8_t>(NewLen);
					uint32_t o = P.Copy(Buf, NewLen, C) - 1;
					o += SubSource.Copy(Buf + o, NewLen - o) - 1;
					o += N.Copy(Buf + o, NewLen - o);
					LWLogWarnIf<256>(o==NewLen, "Something went wrong copying, Expected: {} Got: {}", NewLen, o);

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
	LWEXMLAttribute *InputTypeAttr = N->FindAttribute("InputType");
	LWEXMLAttribute *ForceCompile = N->FindAttribute("ForceCompile");
	LWEXMLAttribute *NameAttr = N->FindAttribute("Name");
	LWELocalization *Localize = AM->GetLocalization();
	if(!LWLogCriticalIf(NameAttr, "Shader node missing 'Name' attribute.")) return false;
	if(!LWLogCriticalIf<256>(TypeAttr, "Shader '{}' missing 'Type' attribute.", NameAttr->GetValue())) return false;


	for (uint32_t i = 0; i < N->m_AttributeCount && DefineCount<MaxDefines; i++) {
		LWEXMLAttribute *Attr = &N->m_Attributes[i];
		if(Attr==TypeAttr || Attr==PathAttr || Attr==NameAttr || Attr==ForceCompile || Attr==InputTypeAttr) continue;
		DefineIterList[DefineCount++] = Attr->GetName();
		DefineIterList[DefineCount++] = Attr->GetValue();
	}
	LWUTF8Iterator CompileCache = AM->GetCompiledShaderCache();
	LWUTF8Iterator Path = PathAttr ? PathAttr->GetValue() : LWUTF8Iterator();
	if (Localize) {
		if (PathAttr && Localize->ParseLocalization(Buffer.m_Data, sizeof(Buffer.m_Data), Path)) Path = Buffer;
	}
	uint32_t Type = TypeAttr->GetValue().CompareList("Vertex", "Geometry", "Pixel", "Compute");
	if(!LWLogCriticalIf<256>(Type!=-1, "Shader '{}' has unknown type: '{}'", NameAttr->GetValue(), TypeAttr->GetValue())) return false;

	auto CPath = LWUTF8I::Fmt<MaxPathLen>("{}/{}.{}{}", CompileCache, NameAttr->GetValue(), CompiledNames[Type], DriverNames[DriverID]);
	if (!CompileCache.AtEnd() && !ForceCompile) {
		LWFileStream CStream;
		if (!LWFileStream::OpenStream(CStream, CPath, LWFileStream::ReadMode | LWFileStream::BinaryMode, Allocator)) {
			if(!LWLogCriticalIf<256>(PathAttr, "Shader '{}': Could not open compiled shader at: '{}'", NameAttr->GetValue(), CPath)) return false;
			CompiledLen = 0;
		} else {
			CompiledLen = CStream.Read(CompiledBuffer, sizeof(CompiledBuffer));
			Res = Driver->CreateShaderCompiled(Type, CompiledBuffer, CompiledLen, Allocator, ErrorBuffer, sizeof(ErrorBuffer));
			if(!LWLogCriticalIf<256>(Res, "Shader '{}' Failed to load pre-compiled shader, Error: '{}' falling back to full compilation.", NameAttr->GetValue(), ErrorBuffer)) CompiledLen = 0;
		}
	}
	AssetPath = Path;
	if(!CompiledLen && PathAttr){
		uint32_t Len = 0;
		LWUTF8Iterator Source = ParsePath(Path, Len, Allocator, nullptr);
		CompiledLen = sizeof(CompiledBuffer);
		Res = Driver->ParseShader(Type, Source, Allocator, DefineCount, DefineIterList, CompiledBuffer, ErrorBuffer, CompiledLen, sizeof(ErrorBuffer));
		if(LWLogWarnIf<256>(CompiledLen<=sizeof(CompiledBuffer), "Shader '{}': Compiled buffer is not large enough to hold shader.", NameAttr->GetValue())) {
			if(Res && !CompileCache.AtEnd()) { //Write compiled shader to shader cache:
				LWFileStream CStream;
				if(LWLogWarnIf<256>(LWFileStream::OpenStream(CStream, CPath, LWFileStream::WriteMode|LWFileStream::BinaryMode, Allocator), "Shader '{}' Could not open '{}' for caching compiled shader.", NameAttr->GetValue(), CPath)) {
					LWLogWarnIf<256>(CStream.Write(CompiledBuffer, CompiledLen)==CompiledLen, "Shader '{}' Writing cache'd shader to '{}' failed.", NameAttr->GetValue(), CPath);
				}
			}
		}
		LWAllocator::Destroy(Source());
	}
	if(!LWLogCriticalIf<1024>(Res, "Shader '{}' Failed to compile: {}", NameAttr->GetValue(), ErrorBuffer)) return false;
	if (InputTypeAttr) {
		uint32_t InputType = InputTypeAttr->GetValue().CompareLista(LWEShaderInputCount, LWEShaderInputNames);
		if(!LWLogCriticalIf<256>(InputType!=-1, "Shader '{}' Has unknown unput type: '{}'", NameAttr->GetValue(), InputTypeAttr->GetValue())) {
			Driver->DestroyShader(Res);
			return false;
		}
		Res->SetInputMap(LWEShaderInputMapCount[InputType], LWEShaderInputMapping[InputType]);
	}
	for (LWEXMLNode *C = N->m_FirstChild; C; C = C->m_Next) {
		uint32_t i = C->GetName().CompareList("InputMap", "ResourceMap", "BlockMap");
		if (i == 0) ParseInputMap(C, Res, AM);
		else if (i == 1) ParseResourceMap(C, Res, AM);
		else if (i == 2) ParseBlockMap(C, Res, AM);
	}
	if(!LWLogCriticalIf<256>(AM->InsertAsset(NameAttr->m_Value, Res, LWEAsset::Shader, AssetPath), "Failed to insert asset: '{}'", NameAttr->GetValue())) {
		Driver->DestroyShader(Res);
		return false;
	}
	return true;
}

bool LWEAssetManager::XMLParseShaderBuilder(LWEXMLNode *N, LWEAssetManager *AM) {
	const char *TypeNames[] = { "Vertex", "Geometry", "Pixel", "Compute" };
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
	if(!LWLogCriticalIf<256>(PathAttr, "{}: missing 'Path' attribute.", N->GetName())) return false;

	auto ParseInputMap = [](LWEXMLNode *N, LWShaderInput *InputList, LWEAssetManager *AM)->uint32_t {
		//                                                       Float,      Int,        UInt,       Double,     Vec2,       Vec3,       Vec4,       uVec2,      uVec3,      uVec4,      iVec2,      iVec3,      iVec4,      dVec2,      dVec3,      dVec4
		const uint32_t TypeNameHashs[LWShaderInput::Count] = { 0x4c816225, 0xf87415fe, 0xe939eb21, 0x8e464c28, 0x2c3c5815, 0x2b3c5682, 0x263c4ea3, 0x1a199b30, 0x1b199cc3, 0x2019a4a2, 0x5d3f3cc4, 0x5e3f3e57, 0x5b3f399e, 0xbfb0ee5f, 0xbeb0eccc, 0xbdb0eb39 };
		uint32_t Cnt = 0;
		for (uint32_t i = 0; i < N->m_AttributeCount; i++) {
			LWEXMLAttribute &A = N->m_Attributes[i];
			LWUTF8Iterator TypeIter = A.GetValue();
			LWUTF8Iterator BracketIter = TypeIter.NextToken('[');
			LWUTF8Iterator InstanceFreqIter = TypeIter.NextToken(':');

			uint32_t NameHash = A.GetName().Hash();
			uint32_t Length = BracketIter.AtEnd() ? 1 : atoi(BracketIter.c_str() + 1);
			uint32_t InstanceFreq = InstanceFreqIter.AtEnd() ? 0 : atoi(InstanceFreqIter.c_str() + 1);

			TypeIter = LWUTF8Iterator(TypeIter, BracketIter.AtEnd() ? InstanceFreqIter : BracketIter);
			uint32_t TypeHash = TypeIter.Hash();
			uint32_t T = 0;
			for (; T < LWShaderInput::Count && TypeNameHashs[T] != TypeHash; T++) {}
			if(!LWLogWarnIf<256>(T<LWShaderInput::Count, "Unknown type '{}' for input '{}'", A.GetValue(), A.GetName())) continue;

			InputList[Cnt++] = LWShaderInput(A.GetName(), T, Length, InstanceFreq);
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

	auto ParseShader = [&DefineIterList, &MaxDefines, &ParseInputMap, &ParseResourceMap, &ParseBlockMap, &InputList, &InputCount, &ResourceList, &ResoruceCount, &BlockList, &BlockCount, &Driver, &Alloc](LWEXMLNode *N, const LWUTF8Iterator &Source, const LWUTF8Iterator &SourcePath, LWEAssetManager *AM) ->bool {
		const uint32_t MaxBufferLength = 1024 * 256;//256KB.
		const uint32_t MaxNameLen = 256;
		const uint32_t MaxPathLen = 256;
		const char *CompiledNames[] = { "lwcv", "lwcg", "lwcp", "lwcc" };
		const char *DriverNames[] = LWVIDEODRIVER_NAMES;
		char CompiledBuffer[MaxBufferLength];
		char ErrorBuffer[MaxBufferLength] = "";
		LWUTF8C_View<MaxNameLen> CBuffer;
		LWUTF8Iterator AssetPath = SourcePath;
		LWShaderInput SInputList[LWShader::MaxInputs];
		uint32_t SResourceList[LWShader::MaxResources];
		uint32_t SBlockList[LWShader::MaxBlocks];

		LWEXMLAttribute *TypeAttr = N->FindAttribute("Type");
		LWEXMLAttribute *InputTypeAttr = N->FindAttribute("InputType");
		LWEXMLAttribute *ForceCompile = N->FindAttribute("ForceCompile");
		LWEXMLAttribute *NameAttr = N->FindAttribute("Name");
		LWELocalization *Localize = AM->GetLocalization();
		uint32_t DefineCount = 0;
		uint32_t CompiledLen = 0;
		if(!LWLogCriticalIf(NameAttr, "Shader is missing 'Name' attribute.")) return false;
		if(!LWLogCriticalIf<256>(TypeAttr, "Shader '{}': is missing 'Type' attribute.", NameAttr->GetValue())) return false;

		for (uint32_t i = 0; i < N->m_AttributeCount; i++) {
			LWEXMLAttribute *Attr = &N->m_Attributes[i];
			if (Attr == TypeAttr || Attr == InputTypeAttr  || Attr == NameAttr || Attr == ForceCompile) continue;
			DefineIterList[DefineCount++] = Attr->GetName();
			DefineIterList[DefineCount++] = Attr->GetValue();
		}
		uint32_t Type = TypeAttr->GetValue().CompareList("Vertex", "Geometry", "Pixel", "Compute");
		if(!LWLogCriticalIf<256>(Type!=-1, "Shader '{}': Has unknown type: '{}'", NameAttr->GetValue(), TypeAttr->GetValue())) return false;

		LWUTF8Iterator CompileCache = AM->GetCompiledShaderCache();
		LWShader *Res = nullptr;
		auto CPath = LWUTF8I::Fmt<MaxPathLen>("{}/{}.{}{}", CompileCache, NameAttr->GetValue(), CompiledNames[Type], DriverNames[Driver->GetDriverID()]);
		if (!CompileCache.AtEnd() && !ForceCompile) {
			LWFileStream CStream;
			if (!LWFileStream::OpenStream(CStream, CPath, LWFileStream::ReadMode | LWFileStream::BinaryMode, Alloc)) {
				CompiledLen = 0;
			} else {
				CompiledLen = CStream.Read(CompiledBuffer, sizeof(CompiledBuffer));
				Res = Driver->CreateShaderCompiled(Type, CompiledBuffer, CompiledLen, Alloc, ErrorBuffer, sizeof(ErrorBuffer));
				if(!LWLogCriticalIf<1024>(Res, "Shader '{}': Failed to load cached shader, Error: '{}', Falling back to full compilation.", NameAttr->GetValue(), ErrorBuffer)) CompiledLen = 0;
			}
		}
		if (!CompiledLen) {
			uint32_t Len = 0;
			CompiledLen = sizeof(CompiledBuffer);
			Res = Driver->ParseShader(Type, Source, Alloc, DefineCount, DefineIterList, CompiledBuffer, ErrorBuffer, CompiledLen, sizeof(ErrorBuffer));
			if(LWLogCriticalIf<256>(CompiledLen<=sizeof(CompiledBuffer), "Shader '{}': Compiled shader is larger than cache buffer.", NameAttr->GetValue())) {
				if(Res && !CompileCache.AtEnd()) {
					LWFileStream CStream;
					if (LWLogWarnIf<256>(LWFileStream::OpenStream(CStream, CPath, LWFileStream::WriteMode | LWFileStream::BinaryMode, Alloc), "Shader '{}': Failed to open file '{}' for caching shader.", NameAttr->GetValue(), CPath)) {
						LWLogWarnIf<256>(CStream.Write(CompiledBuffer, CompiledLen) == CompiledLen, "Shader '{}': Failed to write cache'd shader to '{}'", NameAttr->GetValue(), CPath);
					}
				}
			}
		}
		if(!LWLogCriticalIf<1024>(Res, "Shader '{}' Failed to compile:\n{}", NameAttr->GetValue(), ErrorBuffer)) return false;

		if (InputTypeAttr) {
			uint32_t InputType = InputTypeAttr->GetValue().CompareLista(LWEShaderInputCount, LWEShaderInputNames);
			if(!LWLogCriticalIf<256>(InputType!=-1, "Shader '{}': Has unknown input type: '{}'", NameAttr->GetValue(), InputTypeAttr->GetValue())) {
				Driver->DestroyShader(Res);
				return false;
			}
			Res->SetInputMap(LWEShaderInputMapCount[InputType], LWEShaderInputMapping[InputType]);
		}else Res->SetInputMap(InputCount, InputList);
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
		if(!LWLogCriticalIf<256>(AM->InsertAsset(NameAttr->m_Value, Res, LWEAsset::Shader, AssetPath), "Failed to insert asset '{}'", NameAttr->GetValue())) {
			Driver->DestroyShader(Res);
			return false;
		}
		return true;
	};
	
	std::function<LWUTF8Iterator(const LWUTF8Iterator &, uint32_t &, LWFileStream *)> ParseSourceFunc;

	auto ParsePath = [&ParseSourceFunc, &Alloc](const LWUTF8Iterator &Path, uint32_t &Length, LWFileStream *ExistingStream)->LWUTF8Iterator {
		uint32_t n = Path.CompareLista(LWEShaderCount, LWEShaderNames);
		char8_t *Buffer = nullptr;
		LWFileStream Stream;
		if (n != -1) {
			Length = (uint32_t)strlen(LWEShaderSources[n]) + 1;
			Buffer = Alloc.Allocate<char8_t>(Length);
			std::copy(LWEShaderSources[n], LWEShaderSources[n] + Length, Buffer);
		} else {
			if(!LWLogCriticalIf<256>(LWFileStream::OpenStream(Stream, Path, LWFileStream::ReadMode | LWFileStream::BinaryMode, Alloc, ExistingStream), "Failed to open path: '{}'", Path)) return nullptr;
			Length = Stream.Length() + 1;
			Buffer = Alloc.Allocate<char8_t>(Length);
			if(!LWLogCriticalIf<256>(Stream.ReadText(Buffer, Length)==Length, "Failed to read '{}'", Path)) {
				LWAllocator::Destroy(Buffer);
				return nullptr;
			}
		}
		LWUTF8Iterator Res = ParseSourceFunc(Buffer, Length, n==-1?&Stream:ExistingStream);
		if (Res() != Buffer) LWAllocator::Destroy(Buffer);
		return Res;
	};

	auto ParseSource = [&ParsePath, &Alloc](const LWUTF8Iterator &Source, uint32_t &Length, LWFileStream *ExistingStream)->LWUTF8Iterator {
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
					LWUTF8Iterator SubSource = ParsePath(Path, Len, ExistingStream);
					if (!SubSource.isInitialized()) continue;
					uint32_t NewLen = (Len + Length - (C.RawDistance(--N) + 1)); //-1 because Len, +Length both sizes account for a null character.
					char8_t *Buf = Alloc.Allocate<char8_t>(NewLen);
					uint32_t o = P.Copy(Buf, NewLen, C) - 1;
					o += SubSource.Copy(Buf + o, NewLen - o) - 1;
					o += N.Copy(Buf + o, NewLen - o);
					LWLogCriticalIf<256>(o==NewLen, "Something went wrong copying, Expected: {} Got: {}", NewLen, o);
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

	uint32_t Len = 0;
	LWUTF8Iterator Source = ParsePath(PathAttr->GetValue(), Len, nullptr);
	if (!Source.isInitialized()) return false;

	for (LWEXMLNode *C = N->m_FirstChild; C; C = C->m_Next) {
		uint32_t i = C->GetName().CompareList("Shader", "InputMap", "ResourceMap", "BlockMap");
		if (i == 0) ParseShader(C, Source, PathAttr->GetValue(), AM);
		else if (i == 1) InputCount = ParseInputMap(C, InputList, AM);
		else if (i == 2) ResoruceCount = ParseResourceMap(C, ResourceList, AM);
		else if (i == 3) BlockCount = ParseBlockMap(C, BlockList, AM);
	}
	LWAllocator::Destroy(Source());
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
	if (!LWLogCriticalIf(NameAttr, "VideoBuffer: Missing 'Name' attribute.")) return false;
	if (!LWLogCriticalIf<256>(TypeAttr, "VideoBuffer {}: Missing 'Type' attribute.", NameAttr->GetValue())) return false;
	if (!LWLogCriticalIf<256>(UsageFlagsAttr, "VideoBuffer {}: Missing 'Usage' attribute.", NameAttr->GetValue())) return false;
	if (!LWLogCriticalIf<256>(TypeSizeAttr, "VideoBuffer {}: Missing 'TypeSize' attribute.", NameAttr->GetValue())) return false;
	if (!LWLogCriticalIf<256>(LengthAttr, "VideoBuffer {}: Missing 'Length' attribute.", NameAttr->GetValue())) return false;

	uint32_t TypeID = TypeAttr->GetValue().CompareList("Vertex", "Uniform", "Index16", "Index32", "ImageBuffer", "Indirect");
	if (!LWLogCriticalIf<256>(TypeID!=-1, "VideoBuffer {}: has unknown type: '{}'", NameAttr->GetValue(), TypeAttr->GetValue())) return false;

	uint32_t Length = atoi(LengthAttr->m_Value);
	uint32_t TypeSize = atoi(TypeSizeAttr->m_Value);
	if (PaddedAttr) TypeSize = Driver->GetUniformBlockPaddedSize(TypeSize);
	if(!LWLogCriticalIf<256>(Length, "VideoBuffer {}: Has 0 length.", NameAttr->GetValue())) return false;
	if(!LWLogCriticalIf<256>(TypeSize, "VideoBuffer {}: Has 0 type size.", NameAttr->GetValue())) return false;

	uint32_t UsageSplitCnt = std::min<uint32_t>(UsageFlagsAttr->GetValue().SplitToken(UsageIterList, MaxSplitLength, '|'), MaxSplitLength);
	for (uint32_t i = 0; i < UsageSplitCnt; i++) {
		uint32_t n = UsageIterList[i].AdvanceWord(true).CompareList("PersistentMapped", "Static", "WriteDiscardable", "WriteNoOverlap", "Readable", "GPUResource", "LocalCopy");
		if(LWLogWarnIf<256>(n!=-1, "Videobuffer {}: Has unknown usage flag: '{}'", NameAttr->GetValue(), UsageIterList[i])) UsageFlags |= UsageFlagList[n];
	}
	uint8_t *Data = nullptr;
	if (DataPathAttr) {
		LWFileStream Stream;
		if(LWLogCriticalIf<256>(LWFileStream::OpenStream(Stream, DataPathAttr->GetValue(), LWFileStream::ReadMode | LWFileStream::BinaryMode, Alloc), "VideoBuffer {}: Failed to open data file: '{}'", NameAttr->GetValue(), DataPathAttr->GetValue())) {
			uint32_t DataLen = TypeSize*Length;
			if(LWLogCriticalIf<256>(Stream.Length()>=DataLen, "VideoBuffer {}: Data file is not large enough to fill up video buffer as expected {} - {}", Stream.Length(), DataLen)) {
				Data = Alloc.Allocate<uint8_t>(DataLen);
				if(!LWLogCriticalIf<256>(Stream.Read(Data, DataLen)==DataLen, "VideoBuffer {}: Failed to read data file '{}'", NameAttr->GetValue(), DataPathAttr->GetValue())) {
					Data = LWAllocator::Destroy(Data);
				}
			}
		}
	}

	LWVideoBuffer *Res = Driver->CreateVideoBuffer(TypeID, UsageFlags, TypeSize, Length, Alloc, Data);
	LWAllocator::Destroy(Data);
	if(!LWLogCriticalIf<256>(Res, "VideoBuffer {}: Failed to create buffer.", NameAttr->GetValue())) return false;
	if(!LWLogCriticalIf<256>(AM->InsertAsset(NameAttr->m_Value, Res, LWEAsset::VideoBuffer, ""), "Failed to insert asset: '{}'", NameAttr->GetValue())) {
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
	if(!LWLogCriticalIf(NameAttr, "AudioStream: Does not have 'Name' attribute.")) return false;
	if(!LWLogCriticalIf<256>(PathAttr, "AudioStream {}: Does not have 'Path' attribute.", NameAttr->GetValue())) return false;

	uint32_t Flag = 0;
	
	const uint32_t FlagValues[] = { LWAudioStream::Decompressed };
	const char8_t FlagNames[][32] = { "Decompressed" };
	const uint32_t FlagsCount = sizeof(FlagValues) / sizeof(uint32_t);
	if (FlagAttr) {
		uint32_t FlagIterCnt = std::min<uint32_t>(FlagAttr->GetValue().SplitToken(FlagIterList, MaxIterList, '|'), MaxIterList);
		for (uint32_t i = 0; i < FlagIterCnt; i++) {
			uint32_t n = FlagIterList[i].AdvanceWord(true).CompareLista(FlagsCount, FlagNames);
			if(LWLogWarnIf<256>(n!=-1, "AudioStream {}: Encountered unknown flag: '{}'", NameAttr->GetValue(), FlagIterList[i])) Flag |= FlagValues[n];
		}
	}
	LWUTF8Iterator Path = PathAttr->GetValue();
	if (Localize && Localize->ParseLocalization(SBuffer, sizeof(SBuffer), Path)) Path = LWUTF8Iterator(SBuffer);

	LWAudioStream *Stream = LWAudioStream::Create(Path, Flag, AM->GetAllocator());
	if(!LWLogCriticalIf<256>(Stream, "AudioStream {}: Could not load file '{}'", NameAttr->GetValue(), Path)) return false;
	if(!LWLogCriticalIf<256>(AM->InsertAsset(NameAttr->m_Value, Stream, LWEAsset::AudioStream, Path), "Failed to insert asset: '{}'", NameAttr->GetValue())) {
		LWAllocator::Destroy(Stream);
		return false;
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
	if (!LWLogCriticalIf(NameAttr, "Video: Does not have 'Name' attribute.")) return false;
	if (!LWLogCriticalIf<256>(PathAttr, "Video {}: Does not have 'Path' attribute.", NameAttr->GetValue())) return false;

	uint32_t Flag = 0;
	uint32_t LoopCount = 0;
	float PlaybackSpeed = 1.0f;

	const uint32_t FlagValues[] = { LWEVideoPlayer::Playing };
	const char8_t FlagNames[][32] = { "Playing" };
	const uint32_t FlagCount = sizeof(FlagValues) / sizeof(uint32_t);

	if (FlagAttr) {
		uint32_t FlagCnt = std::min<uint32_t>(FlagAttr->GetValue().SplitToken(FlagIterList, MaxFlagIters, '|'), MaxFlagIters);
		for(uint32_t i=0;i<FlagCnt;i++){
			uint32_t n = FlagIterList[i].AdvanceWord(true).CompareLista(FlagCount, FlagNames);
			if(LWLogWarnIf<256>(n!=-1, "Video {}: Encountered unknown flag: '{}'", NameAttr->GetValue(), FlagIterList[i])) Flag |= FlagValues[n];
		}
	}
	if (LoopCntAttr) LoopCount = atoi(LoopCntAttr->m_Value);
	if (PlaybackSpeedAttr) PlaybackSpeed = (float)atof(PlaybackSpeedAttr->m_Value);
	LWUTF8Iterator Path = PathAttr->GetValue();
	if (Localize && Localize->ParseLocalization(SBuffer, sizeof(SBuffer), Path)) Path = SBuffer;
	LWEVideoPlayer *Video = AM->GetAllocator().Create<LWEVideoPlayer>();
	if(!LWLogCriticalIf<256>(
		LWEVideoPlayer::OpenVideo(*Video, AM->GetDriver(), Path, (Flag&LWEVideoPlayer::Playing) != 0, LoopCount, nullptr, nullptr, PlaybackSpeed, AM->GetAllocator()),
		"Video {}: Could not load file '{}'", NameAttr->GetValue(), Path)) {
		LWAllocator::Destroy(Video);
		return false;
	}
	if(!LWLogCriticalIf<256>(AM->InsertAsset(NameAttr->m_Value, Video, LWEAsset::Video, Path), "Failed to insert asset: '{}'", NameAttr->GetValue())) {
		LWAllocator::Destroy(Video);
		return false;
	}
	return true;
}

LWEAsset *LWEAssetManager::GetAsset(const LWUTF8Iterator &Name) {
	auto Iter = m_AssetMap.find(Name.Hash());
	if(!LWLogCriticalIf<256>(Iter!=m_AssetMap.end(), "Could not find asset '{}'", Name)) return nullptr;
	return Iter->second;
}

LWEAssetManager &LWEAssetManager::SetCompiledShaderCache(const LWUTF8Iterator &Path) {
	Path.Copy(m_CompiledShaderCache, sizeof(m_CompiledShaderCache));
	return *this;
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
	if(LWLogCriticalIf<256>(Ret.second, "Asset name collision: '{}'", Name)) m_AssetCount++;
	return Ret.second;
}

bool LWEAssetManager::InsertAsset(const LWUTF8Iterator &Name, void *Asset, uint32_t AssetType, const LWUTF8Iterator &AssetPath) {
	return InsertAsset(Name, LWEAsset(AssetType, Asset, AssetPath));
}

bool LWEAssetManager::InsertAssetReference(const LWUTF8Iterator &Name, const LWUTF8Iterator &RefName) {
	LWEAsset *A = GetAsset(RefName);
	if(!LWLogCriticalIf<256>(A, "Could not find asset '{}' to make reference: '{}'", RefName, Name)) return false;
	auto Ret = m_AssetMap.emplace(Name.Hash(), A);
	return Ret.second;
}

bool LWEAssetManager::HasAsset(const LWUTF8Iterator &Name) {
	return m_AssetMap.find(Name.Hash()) != m_AssetMap.end();
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

LWUTF8Iterator LWEAssetManager::GetCompiledShaderCache(void) {
	return LWUTF8Iterator(m_CompiledShaderCache);
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