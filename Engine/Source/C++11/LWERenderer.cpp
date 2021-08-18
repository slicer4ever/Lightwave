#include "LWERenderer.h"
#include <LWPlatform/LWWindow.h>
#include <LWCore/LWMatrix.h>
#include <LWCore/LWUnicodeIterator.h>
#include <LWCore/LWTimer.h>
#include <LWVideo/LWImage.h>
#include "LWELogger.h"
#include "LWERenderPasses/LWEGeometryPass.h"
#include "LWERenderPasses/LWEShadowMapPass.h"
#include "LWERenderPasses/LWEPPPass.h"
#include "LWERenderPasses/LWEGausianBlurPass.h"
#include "LWERenderPasses/LWEUIPass.h"

//LWERenderer:
void LWERenderer::GenerateDefaultXMLPasses(std::unordered_map<uint32_t, LWEPassXMLCreateFunc> &PassMap) {
	PassMap.insert({ LWUTF8I("UIPass").Hash(), &LWEUIPass::ParseXML });
	PassMap.insert({ LWUTF8I("PPPass").Hash(), &LWEPPPass::ParseXML });
	PassMap.insert({ LWUTF8I("GeometryPass").Hash(), &LWEGeometryPass::ParseXML });
	PassMap.insert({ LWUTF8I("GausianBlurPass").Hash(), &LWEGausianBlurPass::ParseXML });
	PassMap.insert({ LWUTF8I("ShadowMapPass").Hash(), &LWEShadowMapPass::ParseXML });
	return;
}

bool LWERenderer::ParseXMLSizeAttribute(LWEXMLAttribute *SizeAttribute, LWVector2i &StaticSize, LWVector2f &DynamicSize, uint32_t &NamedDynamic) {
	uint32_t Offset = 0;
	if (sscanf(SizeAttribute->GetValue().c_str(), " %d | %d", &StaticSize.x, &StaticSize.y) == 2) return true;
	if (sscanf_s(SizeAttribute->GetValue().c_str(), " %f %% | %f %% |%n", &DynamicSize.x, &DynamicSize.y, &Offset) == 2) {
		DynamicSize *= LWVector2f(0.01f, 0.01f);
		if(Offset>0) NamedDynamic = LWUTF8I(SizeAttribute->GetValue().c_str() + Offset).NextWord(true).Hash();
		return true;
	}
	return false;
}

bool LWERenderer::ParseXMLFrameBuffer(LWEXMLNode *Node) {
	LWEXMLAttribute *NameAttr = Node->FindAttribute("Name");
	LWEXMLAttribute *SizeAttr = Node->FindAttribute("Size");
	if (!NameAttr) {
		LWELogCritical<256>("Framebuffer is missing Name attribute.");
		return true;
	}
	if (!SizeAttr) {
		LWELogCritical<256>("Framebuffer {} is missing Size attribute.", NameAttr->GetValue());
		return true;
	}

	auto ParseAttachment = [](LWEXMLAttribute *Attr, LWERenderFramebufferTexture &Attachment, uint32_t Samples) {
		const uint32_t StateList[]     = { LWTexture::MinNearest,   LWTexture::MagNearest,   LWTexture::WrapSClampToEdge,   LWTexture::WrapTClampToEdge,   LWTexture::WrapRClampToEdge,   LWTexture::CompareNone,   LWTexture::DepthRead,  LWTexture::MinLinear,   LWTexture::MinNearestMipmapNearest,  LWTexture::MinLinearMipmapNearest,   LWTexture::MinNearestMipmapLinear,   LWTexture::MinLinearMipmapLinear,  LWTexture::MagLinear,   LWTexture::WrapSClampToBorder,   LWTexture::WrapSMirroredRepeat,   LWTexture::WrapSRepeat,   LWTexture::WrapTClampToBorder,   LWTexture::WrapTMirroredRepeat,   LWTexture::WrapTRepeat,   LWTexture::WrapRClampToBorder,   LWTexture::WrapRMirroredRepeat,   LWTexture::WrapRRepeat,   LWTexture::CompareModeRefTexture,   LWTexture::CompareNever,   LWTexture::CompareAlways,   LWTexture::CompareLess,   LWTexture::CompareEqual,   LWTexture::CompareLessEqual,   LWTexture::CompareGreater,  LWTexture::CompareGreaterEqual,   LWTexture::CompareNotEqual,   LWTexture::StencilRead,   LWTexture::Anisotropy_None,   LWTexture::Anisotropy_2x,   LWTexture::Anisotropy_4x,   LWTexture::Anisotropy_8x,   LWTexture::Anisotropy_16x,   LWTexture::RenderTarget,   LWTexture::RenderBuffer,   LWTexture::MakeMipmaps };
		const uint32_t StateCount = sizeof(StateList) / sizeof(uint32_t);
		const uint32_t PackTypeCount = LWImage::DEPTH24STENCIL8 + 1;
		const char8_t StateNames[StateCount][32] = { "MinNearest", "MagNearest", "WrapSClampToEdge", "WrapTClampToEdge", "WrapRClampToEdge", "CompareNone", "DepthRead", "MinLinear", "MinNearestMipmapNearest", "MinLinearMipmapNearest", "MinNearestMipmapLinear", "MinLinearMipmapLinear", "MagLinear", "WrapSClampToBorder", "WrapSMirroredRepeat", "WrapSRepeat", "WrapTClampToBorder", "WrapTMirroredRepeat", "WrapTRepeat", "WrapRClampToBorder", "WrapRMirroredRepeat", "WrapRRepeat", "CompareModeRefTexture", "CompareNever", "CompareAlways", "CompareLess", "CompareEqual", "CompareLessEqual", "CompareGreate", "CompareGreaterEqual", "CompareNotEqual", "StencilRead", "Anisotropy_None", "Anisotropy_2x", "Anisotropy_4x", "Anisotropy_8x", "Anisotropy_16x", "RenderTarget", "RenderBuffer", "MakeMipmaps" };
		const char8_t PackNames[PackTypeCount][16] = { "SRGBA", "RGBA8", "RGBA8S", "RGBA16", "RGBA16S", "RGBA32", "RGBA32S", "RGBA32F", "RG8", "RG8S", "RG16", "RG16S", "RG32", "RG32S", "RG32F", "R8", "R8S", "R16", "R16S", "R32", "R32S", "R32F", "DEPTH16", "DEPTH24", "DEPTH32", "DEPTH24STENCIL8" };
		LWUTF8Iterator StateIterList[StateCount];
		LWUTF8Iterator SplitList[4];

		uint32_t SplitCnt = Attr->GetValue().NextWord(true).SplitToken(SplitList, 4, ':');
		uint32_t PackType = SplitList[0].NextWord(true).CompareLista(PackTypeCount, PackNames);
		if (PackType != -1) {
			uint32_t State = 0;
			if (SplitCnt >= 2) {
				uint32_t IterCnt = SplitList[1].NextWord(true).SplitToken(StateIterList, StateCount, '|');
				for (uint32_t i = 0; i < IterCnt; i++) {
					uint32_t n = StateIterList[i].NextWord(true).CompareLista(StateCount, StateNames);
					if (n == -1) {
						LWELogWarn<256>("unknown state: '{}'", StateIterList[i]);
					} else State |= StateList[n];
				}
			}
			if ((State & (LWTexture::RenderTarget | LWTexture::RenderBuffer)) == 0) State |= LWTexture::RenderTarget;
			Attachment = LWERenderFramebufferTexture(LWERenderTextureProps(State, PackType, Samples > 0 ? LWTexture::Texture2DMS : LWTexture::Texture2D, 1, Samples));
		}else{
			if (SplitCnt == 1) {//Single named texture assumed:
				//First check if we're a pack
				Attachment = LWERenderFramebufferTexture(SplitList[0]);
			} else if (SplitCnt == 2) { //Possibly texture, framebuffer, or created texture:
				//now figure out if we are a framebuffer, or this is a layer:
				uint32_t AttachPnt = SplitList[1].NextWord(true).CompareList("Color", "Color1", "Color2", "Color3", "Color4", "Color5", "Depth");
				if (AttachPnt == -1) Attachment = LWERenderFramebufferTexture(SplitList[0], 0, (uint32_t)atoi(SplitList[1].NextWord(true).c_str()));
				else Attachment = LWERenderFramebufferTexture(SplitList[0], 0, AttachPnt, 0, true);
			} else {//TextureName:Layer:Mipmap:Face
				uint32_t Face = 0;
				if (SplitCnt >= 4) {
					Face = SplitList[3].NextWord(true).CompareList("-X", "+X", "-Y", "+Y", "-Z", "+Z");
					if (Face == -1) {
						Face = 0;
						if (!SplitList[3].AtEnd()) LWELogWarn<256>("unknown face specified: '{}'", SplitList[3]);
					}
				}
				Attachment = LWERenderFramebufferTexture(SplitList[0], (uint32_t)atoi(SplitList[2].NextWord(true).c_str()), (uint32_t)atoi(SplitList[1].NextWord(true).c_str()), Face);
			}
		}
		return;
	};
	LWEXMLAttribute *SamplesAttr = Node->FindAttribute("Samples");
	LWEXMLAttribute *ColorAttr = Node->FindAttribute("Color");
	LWEXMLAttribute *Color1Attr = Node->FindAttribute("Color1");
	LWEXMLAttribute *Color2Attr = Node->FindAttribute("Color2");
	LWEXMLAttribute *Color3Attr = Node->FindAttribute("Color3");
	LWEXMLAttribute *Color4Attr = Node->FindAttribute("Color4");
	LWEXMLAttribute *Color5Attr = Node->FindAttribute("Color5");
	LWEXMLAttribute *DepthAttr = Node->FindAttribute("Depth");
	uint32_t Samples = SamplesAttr ? (uint32_t)atoi(SamplesAttr->GetValue().c_str()) : 0;
	LWERenderPendingFrameBuffer FB;
	
	ParseXMLSizeAttribute(SizeAttr, FB.m_StaticSize, FB.m_DynamicSize, FB.m_NamedDynamic);
	if (ColorAttr) ParseAttachment(ColorAttr, FB.m_Textures[0], Samples);
	if (Color1Attr) ParseAttachment(Color1Attr, FB.m_Textures[1], Samples);
	if (Color2Attr) ParseAttachment(Color2Attr, FB.m_Textures[2], Samples);
	if (Color3Attr) ParseAttachment(Color3Attr, FB.m_Textures[3], Samples);
	if (Color4Attr) ParseAttachment(Color4Attr, FB.m_Textures[4], Samples);
	if (Color5Attr) ParseAttachment(Color5Attr, FB.m_Textures[5], Samples);
	if (DepthAttr) ParseAttachment(DepthAttr, FB.m_Textures[6], Samples);
	return PushPendingResource(LWERenderPendingResource(0, NameAttr->GetValue(), 0, FB))!=0;
}

bool LWERenderer::ParseXMLTexture(LWEXMLNode *Node) {
	const uint32_t StateList[] = { LWTexture::MinNearest,   LWTexture::MagNearest,   LWTexture::WrapSClampToEdge,   LWTexture::WrapTClampToEdge,   LWTexture::WrapRClampToEdge,   LWTexture::CompareNone,   LWTexture::DepthRead,  LWTexture::MinLinear,   LWTexture::MinNearestMipmapNearest,  LWTexture::MinLinearMipmapNearest,   LWTexture::MinNearestMipmapLinear,   LWTexture::MinLinearMipmapLinear,  LWTexture::MagLinear,   LWTexture::WrapSClampToBorder,   LWTexture::WrapSMirroredRepeat,   LWTexture::WrapSRepeat,   LWTexture::WrapTClampToBorder,   LWTexture::WrapTMirroredRepeat,   LWTexture::WrapTRepeat,   LWTexture::WrapRClampToBorder,   LWTexture::WrapRMirroredRepeat,   LWTexture::WrapRRepeat,   LWTexture::CompareModeRefTexture,   LWTexture::CompareNever,   LWTexture::CompareAlways,   LWTexture::CompareLess,   LWTexture::CompareEqual,   LWTexture::CompareLessEqual,   LWTexture::CompareGreater,  LWTexture::CompareGreaterEqual,   LWTexture::CompareNotEqual,   LWTexture::StencilRead,   LWTexture::Anisotropy_None,   LWTexture::Anisotropy_2x,   LWTexture::Anisotropy_4x,   LWTexture::Anisotropy_8x,   LWTexture::Anisotropy_16x,   LWTexture::RenderTarget,   LWTexture::RenderBuffer,   LWTexture::MakeMipmaps };
	const uint32_t StateCount = sizeof(StateList) / sizeof(uint32_t);
	const char8_t StateNames[StateCount][32] = { "MinNearest", "MagNearest", "WrapSClampToEdge", "WrapTClampToEdge", "WrapRClampToEdge", "CompareNone", "DepthRead", "MinLinear", "MinNearestMipmapNearest", "MinLinearMipmapNearest", "MinNearestMipmapLinear", "MinLinearMipmapLinear", "MagLinear", "WrapSClampToBorder", "WrapSMirroredRepeat", "WrapSRepeat", "WrapTClampToBorder", "WrapTMirroredRepeat", "WrapTRepeat", "WrapRClampToBorder", "WrapRMirroredRepeat", "WrapRRepeat", "CompareModeRefTexture", "CompareNever", "CompareAlways", "CompareLess", "CompareEqual", "CompareLessEqual", "CompareGreate", "CompareGreaterEqual", "CompareNotEqual", "StencilRead", "Anisotropy_None", "Anisotropy_2x", "Anisotropy_4x", "Anisotropy_8x", "Anisotropy_16x", "RenderTarget", "RenderBuffer", "MakeMipmaps" };
	LWUTF8Iterator StateIterList[StateCount];

	uint32_t TexType = Node->GetName().CompareList("Texture1D", "Texture2D", "Texture3D", "TextureCubemap", "Texture1DArray", "Texture2DArray", "TextureCubeArray", "Texture2DMS", "Texture2DMSArray");
	if (TexType == -1) return false;

	LWEXMLAttribute *NameAttr = Node->FindAttribute("Name");
	LWEXMLAttribute *SizeAttr = Node->FindAttribute("Size");
	LWEXMLAttribute *PackTypeAttr = Node->FindAttribute("PackType");
	if (!NameAttr) {
		LWELogCritical<256>("{} is missing Name attribute.", Node->GetName());
		return true;
	}
	if (!SizeAttr) {
		LWELogCritical<256>("{} {} is missing Size attribute.", Node->GetName(), NameAttr->GetName());
		return true;
	}
	if (!PackTypeAttr) {
		LWELogCritical<256>("{} {} is missing PackType attribute.", Node->GetName(), NameAttr->GetName());
		return true;
	}
	LWEXMLAttribute *TextureStateAttr = Node->FindAttribute("TextureState");
	LWEXMLAttribute *LayersAttr = Node->FindAttribute("Layers");
	LWEXMLAttribute *SamplesAttr = Node->FindAttribute("Samples");

	uint32_t PackType = PackTypeAttr->GetValue().NextWord(true).CompareList("SRGBA", "RGBA8", "RGBA8S", "RGBA16", "RGBA16S", "RGBA32", "RGBA32S", "RGBA32F", "RG8", "RG8S", "RG16", "RG16S", "RG32", "RG32S", "RG32F", "R8", "R8S", "R16", "R16S", "R32", "R32S", "R32F", "DEPTH16", "DEPTH24", "DEPTH32", "DEPTH24STENCIL8");
	uint32_t TexState = 0;
	uint32_t Layers = LayersAttr ? (uint32_t)atoi(LayersAttr->GetValue().c_str()) : 0;
	uint32_t Samples = SamplesAttr ? (uint32_t)atoi(SamplesAttr->GetValue().c_str()) : 4;
	if ((TexType == LWTexture::Texture2DMS || TexType == LWTexture::Texture2DMSArray) && Samples == 0) {
		LWELogWarn<256>("{} is a multi-sampled texture with samples set to 0.", Node->GetName());
	}
	if (PackType == -1) {
		LWELogCritical<256>("{} has unknown PackType: {}", NameAttr->GetValue(), PackTypeAttr->GetValue());
		return true;
	}
	if (TextureStateAttr) {
		uint32_t IterCnt = TextureStateAttr->GetValue().NextWord(true).SplitToken(StateIterList, StateCount, '|');
		for (uint32_t i = 0; i < IterCnt; i++) {
			uint32_t n = StateIterList[i].NextWord(true).CompareLista(StateCount, StateNames);
			if (n == -1) {
				LWELogCritical<256>("unknown state: '{}'", StateIterList[i]);
			} else TexState |= StateList[n];
		}
	}
	LWERenderPendingTexture PT = LWERenderPendingTexture(LWVector2i(), LWERenderTextureProps(TexState, PackType, TexType, Layers, Samples));
	ParseXMLSizeAttribute(SizeAttr, PT.m_StaticSize, PT.m_DynamicSize, PT.m_NamedDynamic);
	PushPendingResource(LWERenderPendingResource(0, NameAttr->GetValue(), 0, PT));
	return true;
}

bool LWERenderer::ParseXMLVideoBuffer(LWEXMLNode *Node) {
	uint32_t BufferType = Node->GetName().CompareList("VertexBuffer", "UniformBuffer", "Index16Buffer", "Index32Buffer", "ImageBuffer", "IndirectBuffer");
	if (BufferType == -1) return false;
	LWEXMLAttribute *NameAttr = Node->FindAttribute("Name");
	LWEXMLAttribute *TypeSizeAttr = Node->FindAttribute("TypeSize");
	LWEXMLAttribute *CountAttr = Node->FindAttribute("Count");
	if (!NameAttr) {
		LWELogCritical<256>("{} is missing Name attribute.", Node->GetName());
		return true;
	}
	if (!TypeSizeAttr) {
		LWELogCritical<256>("{} is missing TypeSize attribute.", NameAttr->GetValue());
		return true;
	}
	if (!CountAttr) {
		LWELogCritical<256>("{} is missing Count attribute.", NameAttr->GetValue());
		return true;
	}
	LWEXMLAttribute *UsageAttr = Node->FindAttribute("Usage");

	uint32_t TypeSize = (uint32_t)atoi(TypeSizeAttr->GetValue().c_str());
	if (!TypeSize) {
		LWELogCritical<256>("{} has invalid '{}' TypeSize.", NameAttr->GetValue(), TypeSizeAttr->GetValue());
		return true;
	}
	uint32_t Count = (uint32_t)atoi(CountAttr->GetValue().c_str());
	if (!Count) {
		LWELogCritical<256>("{} has invalid '{}' Count.", NameAttr->GetValue(), CountAttr->GetValue());
		return true;
	}

	uint32_t UsageType = LWVideoBuffer::Static;
	if (UsageAttr) UsageType = UsageAttr->GetValue().NextWord(true).CompareList("PersistentMapped", "Static", "WriteDiscardable", "WriteNoOverlap", "Readable", "GPUResource");
	if (UsageType == -1) {
		LWELogCritical<256>("{} has invalid '{}' Usage type.", NameAttr->GetValue(), UsageAttr->GetValue());
		return true;
	}
	PushPendingResource(LWERenderPendingResource(0, NameAttr->GetValue(), 0, LWERenderPendingBuffer(nullptr, BufferType, UsageType, TypeSize, Count)));
	return true;
}

bool LWERenderer::ParseXMLNamedDynamicScalar(LWEXMLNode *Node) {
	LWEXMLAttribute *NameAttr = Node->FindAttribute("Name");
	if (!NameAttr) {
		LWELogCritical<256>("{} must have a Name attribute.", Node->GetName());
		return true;
	}
	LWEXMLAttribute *WidthAttr = Node->FindAttribute("Width");
	LWEXMLAttribute *HeightAttr = Node->FindAttribute("Height");
	LWEXMLAttribute *FORCEAttr = Node->FindAttribute("FORCE");
	float Width = WidthAttr ? (float)atof(WidthAttr->GetValue().c_str()) : 0.0f;
	float Height = HeightAttr ? (float)atof(HeightAttr->GetValue().c_str()) : 0.0f;
	auto Iter = m_NamedDynamicMap.find(NameAttr->GetValue().Hash());
	if (Iter != m_NamedDynamicMap.end()) {
		if (!FORCEAttr) return true;
		(*Iter).second = { LWVector2f(Width, Height), 0 };
		return true;
	}
	SetNamedDynamicSize(NameAttr->GetValue(), LWVector2f(Width, Height));
	return true;
}

bool LWERenderer::ParseXMLBlockGeometry(LWEXMLNode *Node) {
	LWEXMLAttribute *NameAttr = Node->FindAttribute("Name");
	LWEXMLAttribute *TypeAttr = Node->FindAttribute("Type");
	if (!NameAttr) {
		LWELogCritical<256>("{} must have a Name attribute.", Node->GetName());
		return true;
	}

	auto ParseAttributeMap = [](LWEXMLNode *N, LWShaderInput *Attributes)->uint32_t {
		//                                                       Float,      Int,        UInt,       Double,     Vec2,       Vec3,       Vec4,       uVec2,      uVec3,      uVec4,      iVec2,      iVec3,      iVec4,      dVec2,      dVec3,      dVec4
		const uint32_t TypeNameHashs[LWShaderInput::Count] = { 0x4c816225, 0xf87415fe, 0xe939eb21, 0x8e464c28, 0x2c3c5815, 0x2b3c5682, 0x263c4ea3, 0x1a199b30, 0x1b199cc3, 0x2019a4a2, 0x5d3f3cc4, 0x5e3f3e57, 0x5b3f399e, 0xbfb0ee5f, 0xbeb0eccc, 0xbdb0eb39 };
		uint32_t AttributeCount = 0;
		for (uint32_t i = 0; i < N->m_AttributeCount; i++) {
			LWEXMLAttribute &A = N->m_Attributes[i];
			LWUTF8Iterator TypeIter = A.GetValue();
			LWUTF8Iterator BracketIter = TypeIter.NextToken('[');
			
			uint32_t NameHash = A.GetName().Hash();
			uint32_t Length = BracketIter.AtEnd() ? 1 : atoi(BracketIter.c_str() + 1);
			TypeIter = LWUTF8Iterator(TypeIter,BracketIter);

			uint32_t TypeHash = TypeIter.Hash();
			uint32_t T = 0;
			for (; T < LWShaderInput::Count && TypeNameHashs[T] != TypeHash; T++) {}
			if (T >= LWShaderInput::Count) {
				LWELogCritical<256>("unknown type: '{}' for Attribute: '{}'", A.GetValue(), A.GetName());
				continue;
			}
			Attributes[AttributeCount++] = LWShaderInput(A.m_Name, T, Length);
		}
		return AttributeCount;
	};
	const LWShaderInput KnownTypePositionMap[2][LWShader::MaxInputs] = {
		{ LWShaderInput("Position", LWShaderInput::Vec4, 1) }, //StaticVertex
		{ LWShaderInput("Position", LWShaderInput::Vec4, 1), LWShaderInput("BoneWeights", LWShaderInput::Vec4, 1), LWShaderInput("BoneIndices", LWShaderInput::iVec4, 1) } //SkeletonVertex
	};
	const LWShaderInput KnownTypeAttributeMap[2][LWShader::MaxInputs] = {
		{LWShaderInput("TexCoord", LWShaderInput::Vec4, 1), LWShaderInput("Tangent", LWShaderInput::Vec4, 1), LWShaderInput("Normal", LWShaderInput::Vec4, 1) }, //StaticVertex
		{LWShaderInput("TexCoord", LWShaderInput::Vec4, 1), LWShaderInput("Tangent", LWShaderInput::Vec4, 1), LWShaderInput("Normal", LWShaderInput::Vec4, 1) }, //SkeletonVertex
	};
	const std::pair<uint32_t, uint32_t> KnownTypeCounts[2] = {
		{1, 3}, //StaticVertex
		{3, 3}, //SkeletonVertex
	};


	LWShaderInput AttributeLayout[LWShader::MaxInputs];
	LWShaderInput PositionLayout[LWShader::MaxInputs];

	LWEXMLAttribute *VerticesPerBlockAttr = Node->FindAttribute("VerticesPerBlock");
	LWEXMLAttribute *MaxVerticeBlocksAttr = Node->FindAttribute("MaxVerticeBlocks");
	LWEXMLAttribute *IndicesPerBlockAttr = Node->FindAttribute("IndicesPerBlock");
	LWEXMLAttribute *MaxIndiceBlocksAttr = Node->FindAttribute("MaxIndiceBlocks");
	LWEXMLAttribute *BuildPrimitivesAttr = Node->FindAttribute("BuildPrimitives");
	LWEXMLAttribute *LocalAttr = Node->FindAttribute("Local");
	LWEXMLAttribute *DebugAttr = Node->FindAttribute("Debug");
	uint32_t VerticesPerBlock = VerticesPerBlockAttr ? (uint32_t)atoi(VerticesPerBlockAttr->GetValue().c_str()) : LWERendererBlockGeometry::DefaultVerticesPerBlock;
	uint32_t MaxVerticeBlocks = MaxVerticeBlocksAttr ? (uint32_t)atoi(MaxVerticeBlocksAttr->GetValue().c_str()) : LWERendererBlockGeometry::DefaultMaxVerticeBlocks;
	uint32_t IndicesPerBlock = IndicesPerBlockAttr ? (uint32_t)atoi(IndicesPerBlockAttr->GetValue().c_str()) : LWERendererBlockGeometry::DefaultIndicesPerBlock;
	uint32_t MaxIndiceBlocks = MaxIndiceBlocksAttr ? (uint32_t)atoi(MaxIndiceBlocksAttr->GetValue().c_str()) : LWERendererBlockGeometry::DefaultMaxIndiceBlocks;
	uint32_t AttributeLayoutCount = 0;
	uint32_t PositionLayoutCount = 0;
	if (TypeAttr) {
		uint32_t TypeID = TypeAttr->GetValue().CompareList("StaticVertex", "SkeletonVertex");
		if (TypeID == -1) {
			LWELogCritical<256>("block {} has unknown type: {}", NameAttr->GetValue(), TypeAttr->GetValue());
			return true;
		}
		PositionLayoutCount = KnownTypeCounts[TypeID].first;
		AttributeLayoutCount = KnownTypeCounts[TypeID].second;
		std::copy(KnownTypePositionMap[TypeID], KnownTypePositionMap[TypeID] + PositionLayoutCount, PositionLayout);
		std::copy(KnownTypeAttributeMap[TypeID], KnownTypeAttributeMap[TypeID] + AttributeLayoutCount, AttributeLayout);
	}
	

	for (LWEXMLNode *C = Node->m_FirstChild; C; C = C->m_Next) {
		uint32_t cID = C->GetName().CompareList("PositionMap", "AttributeMap");
		if (cID == 0) PositionLayoutCount = ParseAttributeMap(C, PositionLayout);
		else if (cID == 1) AttributeLayoutCount = ParseAttributeMap(C, AttributeLayout);
		else LWELogWarn<256>("Unknown node: '{}'", C->GetName());
	}
	if (!PositionLayoutCount) PositionLayout[PositionLayoutCount++] = LWShaderInput("Position", LWShaderInput::Vec4, 1);

	LWERendererBlockGeometry *Geom = CreateBlockedGeometry(NameAttr->GetValue(), PositionLayout, PositionLayoutCount, AttributeLayout, AttributeLayoutCount, LocalAttr != nullptr, VerticesPerBlock, MaxVerticeBlocks, IndicesPerBlock, MaxIndiceBlocks);
	if (!Geom) {
		LWELogCritical<256>("{} could not be created.", NameAttr->GetValue());
		return true;
	}
	if (BuildPrimitivesAttr) Geom->BuildAllPrimitives(this);
	if (DebugAttr) m_DebugGeometry = Geom;
	return true;
}

bool LWERenderer::ParseXML(LWEXMLNode *Node, void *UserData, LWEXML*) {
	auto PassMap = (std::unordered_map<uint32_t, LWEPassXMLCreateFunc>*)UserData;

	for (LWEXMLNode *C = Node->m_FirstChild; C; C = C->m_Next) {
		uint32_t cType = C->GetName().CompareList("FrameBuffer", "DynamicScalar", "BlockGeometry");
		bool ProcessedResource = false;
		if (cType == 0) ProcessedResource = ParseXMLFrameBuffer(C);
		else if (cType == 1) ProcessedResource = ParseXMLNamedDynamicScalar(C);
		else if (cType == 2) ProcessedResource = ParseXMLBlockGeometry(C);
		if (!ProcessedResource) ProcessedResource = ParseXMLTexture(C);
		if (!ProcessedResource) ProcessedResource = ParseXMLVideoBuffer(C);
		if (ProcessedResource) ProcessPendingResources(-1); //we want to upload immediately as we'll potentially be referencing it by pass's:
		else {
			auto Iter = PassMap->find(C->GetName().Hash());
			if (Iter == PassMap->end()) {
				LWELogCritical<256>("Unknown render pass '{}'", C->GetName());
			} else {
				LWEPass *P = (*Iter).second(C, nullptr, this, m_AssetManager, m_Allocator);
				if (!P) {
					LWELogCritical<256>("Failed to create pass '{}'", C->GetName());
				} else {
					assert(m_PassList.size() < LWEMaxPasses);
					m_PassList.push_back(P);
				}
			}
		}
	}
	return true;
}

LWERenderFrame *LWERenderer::BeginFrame(void) {
	if (m_WriteFrame - m_ReadFrame >= FrameBuffer - 1) return nullptr;
	LWERenderFrame &F = m_Frames[m_WriteFrame % FrameBuffer];
	F.Initialize(m_WriteFrame);
	for (auto &&P : m_PassList) P->InitializeFrame(F);
	return &F;
}

void LWERenderer::EndFrame(void) {
	m_WriteFrame++;
	return;
}

bool LWERenderer::SizeChanged(void) {
	constexpr float e = std::numeric_limits<float>::epsilon();
	LWWindow *Window = m_Driver->GetWindow();
	LWVector2f WndSize = Window->GetSizef();
	if (WndSize.x <= e || WndSize.y <= e) return false;
	for (auto &&T : m_DynamicTextureMap) {
		if(T.m_NamedDynamic!=LWUTF8I::EmptyHash) continue;
		auto Iter = m_TextureMap.find(T.m_ID);
		if (Iter == m_TextureMap.end() || !(*Iter).second) continue; //Texture has already been destroyed(without removing dynamic?)
		LWTexture *oldTex = (*Iter).second;
		(*Iter).second = CreateTexture((T.m_DynamicSize * WndSize).CastTo<int32_t>(), T.m_Propertys);
		m_Driver->DestroyTexture(oldTex);
	}
	for (auto &&FB : m_DynamicFramebufferMap) {
		if(FB.m_NamedDynamic != LWUTF8I::EmptyHash) continue;
		auto Iter = m_FramebufferMap.find(FB.m_ID);
		if (Iter == m_FramebufferMap.end() || !(*Iter).second) continue; //Framebuffer has already been destroyed(without removing dynamic?)
		LWFrameBuffer *oldFB = (*Iter).second;
		(*Iter).second = CreateFramebuffer((FB.m_DynamicSize * WndSize).CastTo<int32_t>(), FB.m_Textures);
		DestroyFrameBuffer(oldFB);
	}
	for (auto &&P : m_PassList) P->WindowSizeChanged(m_Driver, this, Window, m_Allocator);
	m_SizeChanged = false;
	return true;
}

void LWERenderer::NamedDynamicChanged(void) {
	uint32_t HighestID = m_NamedDynamicID;
	for (auto &&T : m_DynamicTextureMap) {
		if (T.m_NamedDynamic == LWUTF8I::EmptyHash) continue;
		auto nDynamic = m_NamedDynamicMap.find(T.m_NamedDynamic);
		if(nDynamic==m_NamedDynamicMap.end()) continue;
		LWERendererDynamicScalar &Scalar = (*nDynamic).second;
		if(Scalar.m_DynamicID<=m_NamedDynamicID) continue;
		HighestID = std::max<uint32_t>(HighestID, Scalar.m_DynamicID);
		auto Iter = m_TextureMap.find(T.m_ID);
		if (Iter == m_TextureMap.end() || !(*Iter).second) continue; //Texture has already been destroyed(without removing dynamic?)
		LWTexture *oldTex = (*Iter).second;
		(*Iter).second = CreateTexture((T.m_DynamicSize * Scalar.m_Size).CastTo<int32_t>(), T.m_Propertys);
		m_Driver->DestroyTexture(oldTex);
	}
	for (auto &&FB : m_DynamicFramebufferMap) {
		if (FB.m_NamedDynamic == LWUTF8I::EmptyHash) continue;
		auto nDynamic = m_NamedDynamicMap.find(FB.m_NamedDynamic);
		if (nDynamic == m_NamedDynamicMap.end()) continue;
		LWERendererDynamicScalar &Scalar = (*nDynamic).second;
		if (Scalar.m_DynamicID <= m_NamedDynamicID) continue;
		HighestID = std::max<uint32_t>(HighestID, Scalar.m_DynamicID);

		auto Iter = m_FramebufferMap.find(FB.m_ID);
		if (Iter == m_FramebufferMap.end() || !(*Iter).second) continue; //Framebuffer has already been destroyed(without removing dynamic?)
		LWFrameBuffer *oldFB = (*Iter).second;
		(*Iter).second = CreateFramebuffer((FB.m_DynamicSize * Scalar.m_Size).CastTo<int32_t>(), FB.m_Textures);
		DestroyFrameBuffer(oldFB);
	}
	m_NamedDynamicID = HighestID+1;
	m_NamedDynamicChanged = false;
	return;
}

void LWERenderer::ApplyFrame(LWERenderFrame &Frame) {
	LWVideoBuffer *IndirectBufferList[LWEMaxGeometryBuckets];
	LWVideoBuffer *IDBufferList[LWEMaxGeometryBuckets];
	for(auto &&P : m_PassList) P->PreFinalizeFrame(Frame, IndirectBufferList, IDBufferList, this, m_Driver);

	Frame.Finalize(this, m_Time, m_FrameGlobalDataBuffer, m_FrameModelDataBuffer, m_FrameBoneDataBuffer, m_FrameLightDataBuffer, IndirectBufferList, IDBufferList, m_GeometryRendableList, m_GeometryRendableCount);

	for (auto &&P : m_PassList) P->PostFinalizeFrame(Frame, this, m_Driver);
	return;
}

void LWERenderer::Render(float dTime, uint32_t SwapInterval, uint64_t lCurrentTime, uint64_t PendingResourceMaxPerFrameTime) {
	m_SizeChanged = m_SizeChanged || m_Driver->GetWindow()->SizeUpdated();
	m_Time += dTime;
	if (!m_Driver->Update()) return;
	if (m_SizeChanged) {
		if (!SizeChanged()) return;
	}
	if (m_NamedDynamicChanged) NamedDynamicChanged();

	if (m_ReadFrame != m_WriteFrame) {
		m_ApplyFrameMetric.RecordTimeStart();
		ApplyFrame(m_Frames[(m_ReadFrame++) % FrameBuffer]);
		m_ApplyFrameMetric.RecordTime();
	}
	m_PendingFrameMetric.RecordTimeStart();
	ProcessPendingResources(PendingResourceMaxPerFrameTime);
	m_PendingFrameMetric.RecordTime();
	if (!m_ReadFrame) return;
	m_RenderFrameMetric.RecordTimeStart();
	LWERenderFrame &F = m_Frames[(m_ReadFrame - 1) % FrameBuffer];
	uint32_t SubPassIndex = 0;
	for (auto &&P : m_PassList) {
		if(P->isDisabled()) continue;
		SubPassIndex += P->RenderPass(F, m_GeometryRendableList, m_GeometryRendableCount, this, m_Driver, SubPassIndex);
	}
	m_Driver->Present(SwapInterval);
	m_RenderFrameMetric.RecordTime();
	return;
}

void LWERenderer::ProcessPendingResources(uint64_t MaxPerFrameTime) {
	LWVector2f WndSize = m_Driver->GetWindow()->GetSizef();

	auto ProcessBufferResource = [this](LWERenderPendingResource &Resource, LWERenderPendingBuffer &PBuffer) {
		uint32_t ID = (uint32_t)Resource.m_ID;
		auto Iter = m_BufferMap.find(ID);
		LWVideoBuffer *CurrBuf = Iter == m_BufferMap.end() ? nullptr : (*Iter).second;
		LWVideoBuffer *NewBuf = Resource.isWriteOverlap() ? CurrBuf : nullptr;
		if (Resource.isDestroyResource()) { //Note: Named buffer's can not be easily removed, so they will stay even if their reference has become invalid.
			if (Iter != m_BufferMap.end()) (*Iter).second = nullptr;
			if (CurrBuf) m_Driver->DestroyVideoBuffer(CurrBuf);
			return;
		}
		if (PBuffer.m_Count) {
			if (!NewBuf) NewBuf = m_Driver->CreateVideoBuffer(PBuffer.m_BufferType, PBuffer.m_BufferUsage, PBuffer.m_TypeSize, PBuffer.m_Count, m_Allocator, (uint8_t*)PBuffer.m_Data);
			else m_Driver->UpdateVideoBuffer(NewBuf, PBuffer.m_Data, PBuffer.m_TypeSize * PBuffer.m_Count, PBuffer.m_Offset);
		}

		if (Resource.m_NameHash != LWUTF8I::EmptyHash) m_NamedBufferMap.insert_or_assign(Resource.m_NameHash, ID);
		if (Iter == m_BufferMap.end()) {
			m_BufferMap.emplace(ID, NewBuf);
		} else (*Iter).second = NewBuf;

		if (CurrBuf && CurrBuf != NewBuf) m_Driver->DestroyVideoBuffer(CurrBuf);
		return;
	};

	auto ProcessTextureResource = [this, &WndSize](LWERenderPendingResource &Resource, LWERenderPendingTexture &PTexture) {
		uint32_t ID = (uint32_t)Resource.m_ID;
		auto Iter = m_TextureMap.find(ID);
		auto dIter = std::find_if(m_DynamicTextureMap.begin(), m_DynamicTextureMap.end(), [&ID](const LWERendererDynamicTexture &T) { return T.m_ID == ID; });
		LWTexture *CurrTex = Iter == m_TextureMap.end() ? nullptr : (*Iter).second;
		LWTexture *NewTex = nullptr;
		if (Resource.isDestroyResource()) {
			if (Iter != m_TextureMap.end()) (*Iter).second = nullptr;
			if (dIter != m_DynamicTextureMap.end()) m_DynamicTextureMap.erase(dIter);
			if (CurrTex) m_Driver->DestroyTexture(CurrTex);
			return;
		}
		if (PTexture.m_Image) NewTex = m_Driver->CreateTexture(PTexture.m_TexProps.m_TextureState, *PTexture.m_Image, m_Allocator);
		else {
			LWVector2i Size = PTexture.m_StaticSize;
			if (PTexture.isDynamicSized()) {
				LWVector2f Scalar = WndSize;
				if (PTexture.m_NamedDynamic != LWUTF8I::EmptyHash) {
					const LWERendererDynamicScalar *nDynamic = FindDynamicScalar(PTexture.m_NamedDynamic);
					if (nDynamic) Scalar = nDynamic->m_Size;
				}
				Size = (Scalar * PTexture.m_DynamicSize).CastTo<int32_t>();
			}
			NewTex = CreateTexture(Size, PTexture.m_TexProps);
		}
		if (Resource.m_NameHash != LWUTF8I::EmptyHash) {
			m_NamedTextureMap.insert_or_assign(Resource.m_NameHash, ID);
		}
		if (PTexture.isDynamicSized()) {
			if (dIter != m_DynamicTextureMap.end()) *dIter = { ID, PTexture.m_DynamicSize, PTexture.m_NamedDynamic, PTexture.m_TexProps };
			else m_DynamicTextureMap.push_back({ ID, PTexture.m_DynamicSize, PTexture.m_NamedDynamic, PTexture.m_TexProps });
		}
		if (Iter == m_TextureMap.end()) {
			m_TextureMap.emplace(ID, NewTex);
		} else (*Iter).second = NewTex;

		if (CurrTex && CurrTex != NewTex) m_Driver->DestroyTexture(CurrTex);
		return;
	};

	auto ProcessFramebufferResource = [this, &WndSize](LWERenderPendingResource &Resource, LWERenderPendingFrameBuffer &PFrameBuffer) {
		uint32_t ID = (uint32_t)Resource.m_ID;
		auto Iter = m_FramebufferMap.find(ID);
		auto dIter = std::find_if(m_DynamicFramebufferMap.begin(), m_DynamicFramebufferMap.end(), [&ID](const LWERendererDynamicFramebuffer &FB) { return FB.m_ID == ID; });
		LWFrameBuffer *CurrFB = Iter == m_FramebufferMap.end() ? nullptr : (*Iter).second;
		LWFrameBuffer *NewFB = nullptr;
		
		if (Resource.isDestroyResource()) {
			if (Iter != m_FramebufferMap.end()) (*Iter).second = nullptr;
			if (dIter != m_DynamicFramebufferMap.end()) m_DynamicFramebufferMap.erase(dIter);
			if (CurrFB) DestroyFrameBuffer(CurrFB);
			return;
		}

		LWVector2i Size = PFrameBuffer.m_StaticSize;
		if (PFrameBuffer.isDynamicSized()) {
			LWVector2f Scalar = WndSize;
			if (PFrameBuffer.m_NamedDynamic != LWUTF8I::EmptyHash) {
				const LWERendererDynamicScalar *nDynamic = FindDynamicScalar(PFrameBuffer.m_NamedDynamic);
				if (nDynamic) Scalar = nDynamic->m_Size;
			}
			Size = (Scalar * PFrameBuffer.m_DynamicSize).CastTo<int32_t>();
		}
		NewFB = CreateFramebuffer(Size, PFrameBuffer.m_Textures);

		if (Resource.m_NameHash != LWUTF8I::EmptyHash) {
			auto Res = m_NamedFramebufferMap.insert_or_assign(Resource.m_NameHash, ID);
		}
		if (PFrameBuffer.isDynamicSized()) {
			LWERendererDynamicFramebuffer dFrameBuffer = { ID, PFrameBuffer.m_DynamicSize, PFrameBuffer.m_NamedDynamic, {PFrameBuffer.m_Textures[0], PFrameBuffer.m_Textures[1], PFrameBuffer.m_Textures[2], PFrameBuffer.m_Textures[3], PFrameBuffer.m_Textures[4], PFrameBuffer.m_Textures[5], PFrameBuffer.m_Textures[6]} };
			if (dIter != m_DynamicFramebufferMap.end()) *dIter = dFrameBuffer;
			else m_DynamicFramebufferMap.push_back(dFrameBuffer);
		}
		if (Iter == m_FramebufferMap.end()) {
			m_FramebufferMap.emplace(ID, NewFB);
		} else (*Iter).second = NewFB;

		if (CurrFB && CurrFB != NewFB) DestroyFrameBuffer(CurrFB);
		return;
	};

	auto ProcessBlockGeometryResource = [this](LWERenderPendingResource &Resource, LWERendererPendingBlockGeometry &PGeometry) {
		auto Iter = m_NamedBlockGeometryMap.find(Resource.m_NameHash);
		if (Iter == m_NamedBlockGeometryMap.end()) {
			LWELogCritical<256>("could not find named block geometry with hash {:#x}", Resource.m_NameHash);
			return;
		}
		LWERendererBlockGeometry *BGeom = (*Iter).second;
		if (Resource.isDestroyResource()) {
			BGeom->Free(Resource.m_ID); 
			BGeom->UploadTo(Resource.m_ID, nullptr, nullptr, nullptr, 0, 0, m_Driver);
			return;
		}
		BGeom->UploadTo(Resource.m_ID, PGeometry.m_VertexPosition, PGeometry.m_VertexAttributes, PGeometry.m_Indices, PGeometry.m_VerticeCount, PGeometry.m_IndiceCount, m_Driver);
		return;
	};

	uint64_t Start = LWTimer::GetCurrent();
	LWERenderPendingResource Resource;
	while (m_PendingResources.Pop(Resource)) {
		uint32_t lType = Resource.GetType();
		if (lType == LWERenderPendingResource::tBuffer) ProcessBufferResource(Resource, Resource.m_Buffer);
		else if (lType == LWERenderPendingResource::tTexture) ProcessTextureResource(Resource, Resource.m_Texture);
		else if (lType == LWERenderPendingResource::tFrameBuffer) ProcessFramebufferResource(Resource, Resource.m_FrameBuffer);
		else if (lType == LWERenderPendingResource::tBlockGeometry) ProcessBlockGeometryResource(Resource, Resource.m_BlockGeometry);
		Resource.Finished();
		if (LWTimer::ToMilliSecond(LWTimer::GetCurrent() - Start) >= MaxPerFrameTime) break;
	}
	return;
}

LWERendererBlockGeometry *LWERenderer::CreateBlockedGeometry(const LWUTF8I &Name, LWShaderInput *PositionLayout, uint32_t PositionLayoutCount, LWShaderInput *AttributeLayout, uint32_t AttributeLayoutCount, bool LocalCopy, uint32_t VerticesPerBlock, uint32_t MaxVerticeBlocks, uint32_t IndicesPerBlock, uint32_t MaxIndiceBlocks) {
	LWERendererBlockGeometry *Geom = m_Allocator.Create<LWERendererBlockGeometry>(m_Driver, m_Allocator, Name.Hash(), PositionLayout, PositionLayoutCount, AttributeLayout, AttributeLayoutCount, LocalCopy, VerticesPerBlock, MaxVerticeBlocks, IndicesPerBlock, MaxIndiceBlocks);
	auto Res = m_NamedBlockGeometryMap.insert({ Name.Hash(), Geom });
	if (!Res.second) {
		LWELogCritical<256>("Geometry block with Name '{}' already exists(or hash collision occurred.)", Name);
		Geom->Release(m_Driver);
		LWAllocator::Destroy(Geom);
		return nullptr;
	}
	return Geom;
}

LWERendererBlockGeometry *LWERenderer::CreateBlockedGeometry(uint32_t NameHash, LWShaderInput *PositionLayout, uint32_t PositionLayoutCount, LWShaderInput *AttributeLayout, uint32_t AttributeLayoutCount, bool LocalCopy, uint32_t VerticesPerBlock, uint32_t MaxVerticeBlocks, uint32_t IndicesPerBlock, uint32_t MaxIndiceBlocks) {
	LWERendererBlockGeometry *Geom = m_Allocator.Create<LWERendererBlockGeometry>(m_Driver, m_Allocator, NameHash, PositionLayout, PositionLayoutCount, AttributeLayout, AttributeLayoutCount, LocalCopy, VerticesPerBlock, MaxVerticeBlocks, IndicesPerBlock, MaxIndiceBlocks);
	auto Res = m_NamedBlockGeometryMap.insert({ NameHash, Geom });
	if (!Res.second) {
		LWELogCritical<256>("Geometry block with namehash {:#x} already exists(or hash collision occurred.)", NameHash);
		Geom->Release(m_Driver);
		LWAllocator::Destroy(Geom);
		return nullptr;
	}
	return Geom;
}

LWERenderer &LWERenderer::SetAssetManager(LWEAssetManager *AssetManager) {
	m_AssetManager = AssetManager;
	return *this;
}

LWERenderer &LWERenderer::SetDebugGeometry(LWERendererBlockGeometry *DebugGeometry) {
	m_DebugGeometry = DebugGeometry;
	return *this;
}

void LWERenderer::SetNamedDynamicSize(const LWUTF8Iterator &Name, const LWVector2f &Size) {
	return SetNamedDynamicSize(Name.Hash(), Size);
}

void LWERenderer::SetNamedDynamicSize(uint32_t NameHash, const LWVector2f &Size) {
	auto Iter = m_NamedDynamicMap.find(NameHash);
	LWERendererDynamicScalar Scalar = { Size, 0 };
	if (Iter == m_NamedDynamicMap.end()) {
		m_NamedDynamicMap.emplace(NameHash, Scalar);
	} else {
		Scalar.m_DynamicID = m_NamedDynamicID + 1;
		(*Iter).second = Scalar;
		m_NamedDynamicChanged = true;
	}
	return;
}

uint32_t LWERenderer::NextTextureID(void) {
	return m_NextTextureID++;
}

uint32_t LWERenderer::NextVideoBufferID(void) {
	return m_NextBufferID++;
}

uint32_t LWERenderer::NextFramebufferID(void) {
	return m_NextFramebufferID++;
}

uint32_t LWERenderer::PushPendingResource(const LWERenderPendingResource &Resource) {
	LWERenderPendingResource Res = Resource; //Create copy to modify ID.
	uint32_t rType = Res.GetType();
	if (rType != LWERenderPendingResource::tBlockGeometry) {
		Res.m_ID = Res.m_ID ? Res.m_ID : (rType == LWERenderPendingResource::tBuffer ? NextVideoBufferID() : (rType == LWERenderPendingResource::tTexture ? NextTextureID() : NextFramebufferID()));
		return (uint32_t)(m_PendingResources.Push(Res) ? Res.m_ID : 0);
	} else return m_PendingResources.Push(Res) ? 1 : 0;
}

LWFrameBuffer *LWERenderer::GetFrameBuffer(uint32_t ID) {
	auto Iter = m_FramebufferMap.find(ID);
	return Iter == m_FramebufferMap.end() ? nullptr : (*Iter).second;
}

LWVideoBuffer *LWERenderer::GetVideoBuffer(uint32_t ID) {
	auto Iter = m_BufferMap.find(ID);
	return Iter == m_BufferMap.end() ? nullptr : (*Iter).second;
}

LWTexture *LWERenderer::GetTexture(uint32_t ID) {
	auto Iter = m_TextureMap.find(ID);
	return Iter == m_TextureMap.end() ? nullptr : (*Iter).second;
}

LWEPass *LWERenderer::GetPass(uint32_t Idx) {
	return m_PassList[Idx];
}

uint32_t LWERenderer::FindNamedFrameBuffer(const LWUTF8Iterator &Name, bool Verbose) {
	auto Iter = m_NamedFramebufferMap.find(Name.Hash());
	if (Iter == m_NamedFramebufferMap.end()) {
		if (Verbose) LWELogCritical<256>("Could not find framebuffer with name: {}", Name);
		return 0;
	}
	return (*Iter).second;
}

uint32_t LWERenderer::FindNamedFrameBuffer(uint32_t NameHash, bool Verbose) {
	auto Iter = m_NamedFramebufferMap.find(NameHash);
	if (Iter == m_NamedFramebufferMap.end()) {
		if (Verbose) LWELogCritical<256>("Could not find framebuffer with namehash: {#x}", NameHash);
		return 0;
	}
	return (*Iter).second;
}

uint32_t LWERenderer::FindNamedVideoBuffer(const LWUTF8Iterator &Name, bool Verbose) {
	auto Iter = m_NamedBufferMap.find(Name.Hash());
	if (Iter == m_NamedBufferMap.end()) {
		if (Verbose) LWELogCritical<256>("Could not find buffer with name: {}", Name);
		return 0;
	}
	return (*Iter).second;
}

uint32_t LWERenderer::FindNamedVideoBuffer(uint32_t NameHash, bool Verbose) {
	auto Iter = m_NamedBufferMap.find(NameHash);
	if (Iter == m_NamedBufferMap.end()) {
		if (Verbose) LWELogCritical<256>("Could not find buffer with namehash: {:#x}", NameHash);
		return 0;
	}
	return (*Iter).second;
}

uint32_t LWERenderer::FindNamedTexture(const LWUTF8Iterator &Name, bool Verbose) {
	auto Iter = m_NamedTextureMap.find(Name.Hash());
	if (Iter == m_NamedTextureMap.end()) {
		if(Verbose) LWELogCritical<256>("Could not find texture with name: {}", Name);
		return 0;
	}
	return (*Iter).second;
}

uint32_t LWERenderer::FindNamedTexture(uint32_t NameHash, bool Verbose) {
	auto Iter = m_NamedTextureMap.find(NameHash);
	if (Iter == m_NamedTextureMap.end()) {
		if(Verbose) LWELogCritical<256>("Could not find named texture with hash: {:#x}", NameHash);
		return 0;
	}
	return (*Iter).second;
}

LWERendererBlockGeometry *LWERenderer::FindNamedBlockGeometryMap(const LWUTF8Iterator &Name, bool Verbose) {
	auto Iter = m_NamedBlockGeometryMap.find(Name.Hash());
	if (Iter == m_NamedBlockGeometryMap.end()) {
		if (Verbose) LWELogCritical<256>("Could not find BlockGeometryMap: '{}'", Name);
		return nullptr;
	}
	return (*Iter).second;
}

LWERendererBlockGeometry *LWERenderer::FindNamedBlockGeometryMap(uint32_t NameHash, bool Verbose) {
	auto Iter = m_NamedBlockGeometryMap.find(NameHash);
	if (Iter == m_NamedBlockGeometryMap.end()) {
		if (Verbose) LWELogCritical<256>("Could not find BlockGeometryMap: {:#x}", NameHash);
		return nullptr;
	}
	return (*Iter).second;
}

LWEPass *LWERenderer::FindNamedPass(const LWUTF8Iterator &Name, bool Verbose) {
	uint32_t Hash = Name.Hash();
	for (auto &&P : m_PassList) {
		if (P->GetNameHash() == Hash) return P;
	}
	if(Verbose) LWELogCritical<256>("Could not find named with: {}", Name);
	return nullptr;
}

LWEPass *LWERenderer::FindNamedPass(uint32_t NameHash, bool Verbose) {
	for (auto &&P : m_PassList) {
		if (P->GetNameHash() == NameHash) return P;
	}
	if (Verbose) LWELogCritical<256>("Could not find named pass with hash: {:#x}", NameHash);
	return nullptr;
}

const LWERendererDynamicScalar *LWERenderer::FindDynamicScalar(const LWUTF8Iterator &Name, bool Verbose) const {
	auto Iter = m_NamedDynamicMap.find(Name.Hash());
	if (Iter == m_NamedDynamicMap.end()) {
		if (Verbose) LWELogCritical<256>("Could not find named dynamic scalar: {}", Name);
		return nullptr;
	}
	return &(*Iter).second;
}

const LWERendererDynamicScalar *LWERenderer::FindDynamicScalar(uint32_t NameHash, bool Verbose) const {
	auto Iter = m_NamedDynamicMap.find(NameHash);
	if (Iter == m_NamedDynamicMap.end()) {
		if (Verbose) LWELogCritical<256>("Could not find named dynamic scalar: {}", NameHash);
		return nullptr;
	}
	return &(*Iter).second;
}

bool LWERenderer::FinalizePasses(void) {
	LWEShaderPassData PassData[LWEMaxGeometryBuckets*32];
	uint32_t PassCount = (uint32_t)m_PassList.size();
	uint32_t PassDataCount = 0;
	for (uint32_t i = 0; i < FrameBuffer; ++i) new (&m_Frames[i]) LWERenderFrame(m_Driver, (uint32_t)m_PassList.size(), m_DebugGeometry, m_Allocator);
	for (uint32_t i = 0; i < PassCount; ++i) {
		m_PassList[i]->SetPassID(i);
		PassDataCount += m_PassList[i]->InitializePass(m_Driver, this, m_AssetManager, PassData+PassDataCount, m_Allocator);
		for (uint32_t f = 0; f < FrameBuffer; f++) m_PassList[i]->CreateFrame(m_Frames[f], m_Driver, m_AssetManager, m_Allocator);
	}

	uint8_t *PaddedPassData = m_Allocator.Allocate<uint8_t>(m_Driver->GetUniformPaddedLength<LWEShaderPassData>(PassDataCount));
	for (uint32_t i = 0; i < PassDataCount; i++) *m_Driver->GetUniformPaddedAt<LWEShaderPassData>(i, PaddedPassData) = PassData[i];
	m_FramePassDataBuffer = m_Driver->CreatePaddedVideoBuffer<LWEShaderPassData>(LWVideoBuffer::Uniform, LWVideoBuffer::Static, PassDataCount, m_Allocator, PaddedPassData);
	LWAllocator::Destroy(PaddedPassData);
	return true;
}

LWEAssetManager *LWERenderer::GetAssetManager(void) {
	return m_AssetManager;
}

LWVideoBuffer *LWERenderer::GetPPScreenVertices(void) {
	return m_PPScreenVertices;
}

LWVideoBuffer *LWERenderer::GetFrameGlobalDataBuffer(void) {
	return m_FrameGlobalDataBuffer;
}

LWVideoBuffer *LWERenderer::GetFramePassDataBuffer(void) {
	return m_FramePassDataBuffer;
}

LWVideoBuffer *LWERenderer::GetFrameModelDataBuffer(void) {
	return m_FrameModelDataBuffer;
}

LWVideoBuffer *LWERenderer::GetFrameBoneDataBuffer(void) {
	return m_FrameBoneDataBuffer;
}

LWVideoBuffer *LWERenderer::GetFrameLightDataBuffer(void) {
	return m_FrameLightDataBuffer;
}

const LWELoggerTimeMetrics &LWERenderer::GetApplyFrameMetric(void) const {
	return m_ApplyFrameMetric;
}

const LWELoggerTimeMetrics &LWERenderer::GetPendingFrameMetric(void) const {
	return m_PendingFrameMetric;
}

const LWELoggerTimeMetrics &LWERenderer::GetRenderFrameMetric(void) const {
	return m_RenderFrameMetric;
}

LWVideoDriver *LWERenderer::GetVideoDriver(void) {
	return m_Driver;
}

LWERendererBlockGeometry *LWERenderer::GetDebugGeometry(void) {
	return m_DebugGeometry;
}

LWAllocator &LWERenderer::GetAllocator(void) {
	return m_Allocator;
}

uint32_t LWERenderer::GetPassCount(void) const {
	return (uint32_t)m_PassList.size();
}

float LWERenderer::GetTime(void) const {
	return m_Time;
}

LWTexture *LWERenderer::CreateTexture(const LWVector2i &TexSize, LWERenderTextureProps &Props) {
	if (Props.m_PackType == -1) return nullptr;
	switch (Props.m_TextureType) {
	case LWTexture::Texture1D: return m_Driver->CreateTexture1D(Props.m_TextureState, Props.m_PackType, TexSize.x, nullptr, 0, m_Allocator);
	case LWTexture::Texture2D: return m_Driver->CreateTexture2D(Props.m_TextureState, Props.m_PackType, TexSize, nullptr, 0, m_Allocator);
	case LWTexture::Texture3D: return m_Driver->CreateTexture3D(Props.m_TextureState, Props.m_PackType, LWVector3i(TexSize, Props.m_Layers), nullptr, 0, m_Allocator);
	case LWTexture::TextureCubeMap: return m_Driver->CreateTextureCubeMap(Props.m_TextureState, Props.m_PackType, TexSize, nullptr, 0, m_Allocator);
	case LWTexture::Texture1DArray: return m_Driver->CreateTexture1DArray(Props.m_TextureState, Props.m_PackType, TexSize.x, Props.m_Layers, nullptr, 0, m_Allocator);
	case LWTexture::Texture2DArray: return m_Driver->CreateTexture2DArray(Props.m_TextureState, Props.m_PackType, TexSize, Props.m_Layers, nullptr, 0, m_Allocator);
	case LWTexture::TextureCubeMapArray: return m_Driver->CreateTextureCubeArray(Props.m_TextureState, Props.m_PackType, TexSize, Props.m_Layers, nullptr, 0, m_Allocator);
	case LWTexture::Texture2DMS: return m_Driver->CreateTexture2DMS(Props.m_TextureState, Props.m_PackType, TexSize, Props.m_Samples, m_Allocator);
	case LWTexture::Texture2DMSArray: return m_Driver->CreateTexture2DMSArray(Props.m_TextureState, Props.m_PackType, TexSize, Props.m_Samples, Props.m_Layers, m_Allocator);
	}
	LWELogCritical<256>("Unknown texture type {}", Props.m_TextureType);
	return nullptr;
}

LWFrameBuffer *LWERenderer::CreateFramebuffer(const LWVector2i &FramebufferSize, LWERenderFramebufferTexture TextureList[LWFrameBuffer::Count]) {
	LWFrameBuffer *FB = m_Driver->CreateFrameBuffer(FramebufferSize, m_Allocator);
	for (uint32_t i = 0; i < LWFrameBuffer::Count; i++) {
		LWERenderFramebufferTexture &FTex = TextureList[i];
		if (FTex.m_NameHash != LWUTF8I::EmptyHash) {
			LWFrameBufferAttachment Attachment;
			if (FTex.isFramebufferName()) {
				uint32_t TargetID = FindNamedFrameBuffer(FTex.m_NameHash);
				if (!TargetID) continue;
				LWFrameBuffer *TargetFB = GetFrameBuffer(TargetID);
				if (!TargetFB) continue;
				Attachment = TargetFB->GetAttachment(FTex.GetNamedLayer());
			} else {
				uint32_t TargetID = FindNamedTexture(FTex.m_NameHash);
				if (!TargetID) continue;
				Attachment = { GetTexture(TargetID), FTex.GetNamedLayer(), FTex.GetNamedMipmap(), FTex.GetNamedFace(), nullptr };
			}
			if(!Attachment.m_Source) continue;
			LWVector2i SourceSize = Attachment.m_Source->Get2DSize();
			if (Attachment.m_Mipmap > 0) SourceSize = LWImage::MipmapSize2D(SourceSize, Attachment.m_Mipmap - 1);
			if(SourceSize!=FramebufferSize){
				LWELogCritical<256>("Attachment size({}) is not the same as framebuffer({}).", SourceSize, FramebufferSize);
				continue;
			}
			FB->SetCubeAttachment(i, Attachment.m_Source, Attachment.m_Face, Attachment.m_Layer, Attachment.m_Mipmap, (void*)(uintptr_t)FBAttachmentIDBit);
		} else {
			LWTexture *Tex = CreateTexture(FramebufferSize, FTex.m_TexProps);
			FB->SetAttachment(i, Tex);
		}
	}
	return FB;
}

LWFrameBuffer *LWERenderer::DestroyFrameBuffer(LWFrameBuffer *FB) {
	for (uint32_t i = 0; i < LWFrameBuffer::Count; i++) {
		LWFrameBufferAttachment &Attach = FB->GetAttachment(i);
		if (!Attach.m_Source) continue;
		if (Attach.m_UserData != nullptr) continue; //this was a named attachment.
		m_Driver->DestroyTexture(Attach.m_Source);
	}
	m_Driver->DestroyFrameBuffer(FB);
	return nullptr;
}


LWERenderer::LWERenderer(LWVideoDriver *Driver, LWEAssetManager *AssetManager, LWAllocator &Allocator) : m_Allocator(Allocator), m_Driver(Driver), m_AssetManager(AssetManager) {
	LWVertexTexture TL = LWVertexTexture(LWVector4f(-1.0f, 1.0f, 0.0f, 1.0f), LWVector4f(0.0f, 1.0f, 0.0f, 1.0f));
	LWVertexTexture BL = LWVertexTexture(LWVector4f(-1.0f,-1.0f, 0.0f, 1.0f), LWVector4f(0.0f, 0.0f, 0.0f, 0.0f));
	LWVertexTexture TR = LWVertexTexture(LWVector4f( 1.0f, 1.0f, 0.0f, 1.0f), LWVector4f(1.0f, 1.0f, 1.0f, 1.0f));
	LWVertexTexture BR = LWVertexTexture(LWVector4f( 1.0f,-1.0f, 0.0f, 1.0f), LWVector4f(1.0f, 0.0f, 1.0f, 0.0f));
	LWVertexTexture PPVertices[6] = { TL, BL, BR, BR, TR, TL };
	m_PPScreenVertices = m_Driver->CreateVideoBuffer<LWVertexTexture>(LWVideoBuffer::Vertex, LWVideoBuffer::Static, 6, Allocator, PPVertices);
	m_FrameGlobalDataBuffer = m_Driver->CreateVideoBuffer<LWEShaderGlobalData>(LWVideoBuffer::Uniform, LWVideoBuffer::WriteDiscardable, 1, m_Allocator, nullptr);
	m_FrameModelDataBuffer = m_Driver->CreateVideoBuffer<LWEGeometryModelData>(LWVideoBuffer::ImageBuffer, LWVideoBuffer::WriteDiscardable, LWEMaxBucketSize, m_Allocator, nullptr);
	m_FrameBoneDataBuffer = m_Driver->CreateVideoBuffer<LWMatrix4f>(LWVideoBuffer::ImageBuffer, LWVideoBuffer::WriteDiscardable, LWEMaxBoneCount, m_Allocator, nullptr);
	m_FrameLightDataBuffer = m_Driver->CreateVideoBuffer<LWEShaderLightData>(LWVideoBuffer::ImageBuffer, LWVideoBuffer::WriteDiscardable, LWEMaxLights, m_Allocator, nullptr);
}

LWERenderer::~LWERenderer() {
	for (auto &&P : m_PassList) {
		for (uint32_t i = 0; i < FrameBuffer; i++) P->ReleaseFrame(m_Frames[i], m_Driver);
		P->DestroyPass(m_Driver);
	}
	LWERenderPendingResource Resource;
	while (m_PendingResources.Pop(Resource)) Resource.Finished();
	for (auto &&B : m_BufferMap) {
		if (B.second) m_Driver->DestroyVideoBuffer(B.second);
	}
	for (auto &&T : m_TextureMap) {
		if (T.second) m_Driver->DestroyTexture(T.second);
	}
	for (auto &&FB : m_FramebufferMap) {
		if (FB.second) DestroyFrameBuffer(FB.second);
	}
	for (auto &&BG : m_NamedBlockGeometryMap) {
		if (BG.second) {
			BG.second->Release(m_Driver);
			LWAllocator::Destroy(BG.second);
		}
	}
	if (m_PPScreenVertices) m_Driver->DestroyVideoBuffer(m_PPScreenVertices);
	if (m_FrameGlobalDataBuffer) m_Driver->DestroyVideoBuffer(m_FrameGlobalDataBuffer);
	if (m_FramePassDataBuffer) m_Driver->DestroyVideoBuffer(m_FramePassDataBuffer);
	if (m_FrameModelDataBuffer) m_Driver->DestroyVideoBuffer(m_FrameModelDataBuffer);
	if (m_FrameBoneDataBuffer) m_Driver->DestroyVideoBuffer(m_FrameBoneDataBuffer);
	if (m_FrameLightDataBuffer) m_Driver->DestroyVideoBuffer(m_FrameLightDataBuffer);
}

//LWERendererBlockAllocator:
uint32_t LWERendererBlockAllocator::Allocate(uint32_t Blocks) {
	if (!Blocks) return m_TotalBlocks;
	assert(Blocks <= m_TotalBlocks);

	auto Iter = std::find_if(m_FreeList.begin(), m_FreeList.end(), [&Blocks](const auto &F) {return F.m_BlockCount >= Blocks; });
	if (Iter == m_FreeList.end()) return -1;
	LWERendererFreeBlock &Blk = *Iter;
	uint32_t ID = Blk.m_BlockID;
	Blk.m_BlockID += Blocks;
	Blk.m_BlockCount -= Blocks;
	if (!Blk.m_BlockCount) m_FreeList.erase(Iter);
	m_AllocatedMap.insert_or_assign(ID, Blocks);
	return ID;
}

void LWERendererBlockAllocator::Free(uint32_t BlockID) {
	auto AllocIter = m_AllocatedMap.find(BlockID);
	if (AllocIter == m_AllocatedMap.end()) return;
	uint32_t Blocks = (*AllocIter).second;
	assert(BlockID + Blocks <= m_TotalBlocks);
	auto Iter = std::lower_bound(m_FreeList.begin(), m_FreeList.end(), BlockID, [](const auto &F, uint32_t BID) { return F.m_BlockID < BID; });
	LWERendererFreeBlock *CurrBlock = nullptr;
	if (Iter != m_FreeList.begin()) {
		LWERendererFreeBlock &Prev = *(Iter - 1);
		if (Prev.m_BlockID + Prev.m_BlockCount == BlockID) {
			Prev.m_BlockCount += Blocks;
			CurrBlock = &Prev;
		}
	}
	if (Iter != m_FreeList.end()) {
		LWERendererFreeBlock &Next = *Iter;
		if (BlockID + Blocks == Next.m_BlockID) {
			if (CurrBlock) {
				CurrBlock->m_BlockCount += Next.m_BlockCount;
				m_FreeList.erase(Iter);
			} else {
				Next.m_BlockID = BlockID;
				Next.m_BlockCount += Blocks;
				CurrBlock = &Next;
			}
		}
	}
	if (!CurrBlock) m_FreeList.insert(Iter, { BlockID, Blocks });
	m_AllocatedMap.erase(BlockID);
	return;
}

LWERendererBlockAllocator::LWERendererBlockAllocator(uint32_t TotalBlocks) : m_TotalBlocks(TotalBlocks) {
	m_FreeList.push_back({ 0, TotalBlocks });
}

//LWERendererBlockGeometry:
uint64_t LWERendererBlockGeometry::Allocate(uint8_t *VertexPosition, uint8_t *VertexAttributes, uint32_t *Indices, uint32_t VerticeCount, uint32_t IndiceCount, LWERenderer *Renderer, uint32_t Flags) {
	uint64_t ID = Allocate(VerticeCount, IndiceCount);
	if (ID == NullID) return ID;
	if (!Renderer->PushPendingResource(LWERenderPendingResource(ID, m_NameHash, Flags, LWERendererPendingBlockGeometry(VertexPosition, VertexAttributes, Indices, VerticeCount, IndiceCount)))) {
		Free(ID);
		return NullID;
	}
	return ID;
}

bool LWERendererBlockGeometry::SubmitLocal(uint64_t ID, uint32_t VerticeCount, uint32_t IndiceCount, LWERenderer *Renderer) {
	uint32_t VertID = ((ID&VerticeBits)>>VerticeBitOffset) * m_VerticesPerBlock;
	uint32_t IndID = ((ID&IndiceBits)>>IndiceBitOffset) * m_IndicesPerBlock;
	uint8_t *VP = GetLocalVerticePositionsAt(VertID);
	uint8_t *VA = GetLocalVerticeAttributesAt(VertID);
	uint32_t *I = GetLocalIndicesAt(IndID);
	return Renderer->PushPendingResource(LWERenderPendingResource(ID, m_NameHash, LWERenderPendingResource::NoDiscard, LWERendererPendingBlockGeometry(VP, VA, I, VerticeCount, IndiceCount)));
}

uint64_t LWERendererBlockGeometry::Allocate(uint32_t VerticeCount, uint32_t IndiceCount) {
	m_AllocateLock.lock();
	const uint32_t PositionSize = GetPositionTypeSize();
	const uint32_t AttributeSize = GetAttributeTypeSize();

	uint32_t VerticeBlocks = (VerticeCount + m_VerticesPerBlock - 1) / m_VerticesPerBlock;
	uint32_t IndiceBlocks = (IndiceCount + m_IndicesPerBlock - 1) / m_IndicesPerBlock;

	uint32_t VerticeBlockID = m_VerticeBlocks.Allocate(VerticeBlocks);
	if (VerticeBlockID == -1) {
		LWELogCritical<256>("no space remains for vertices in geometry block.");
		m_AllocateLock.unlock();
		return NullID;
	}
	uint32_t IndiceBlockID = m_IndiceBlocks.Allocate(IndiceBlocks);
	if (IndiceBlockID == -1) {
		LWELogCritical<256>("no space remains for indices in indice block.");
		m_VerticeBlocks.Free(VerticeBlockID);
		m_AllocateLock.unlock();
		return NullID;
	}
	assert(VerticeBlockID <= VerticeBits && IndiceBlockID <= VerticeBits);
	m_AllocateLock.unlock();
	return (uint64_t)VerticeBlockID | ((uint64_t)IndiceBlockID << IndiceBitOffset);
}

uint64_t LWERendererBlockGeometry::Free(uint64_t ID) {
	uint32_t VerticeID = (uint32_t)((ID & VerticeBits) >> VerticeBitOffset);
	uint32_t IndicesID = (uint32_t)((ID & IndiceBits) >> IndiceBitOffset);
	m_AllocateLock.lock();
	m_VerticeBlocks.Free(VerticeID);
	m_IndiceBlocks.Free(IndicesID);
	m_AllocateLock.unlock();
	return NullID;
}

uint64_t LWERendererBlockGeometry::DelayFree(uint64_t ID, LWERenderer *Renderer) {
	if (!Renderer->PushPendingResource(LWERenderPendingResource(ID, m_NameHash, LWERenderPendingResource::DestroyResource, LWERendererPendingBlockGeometry()))) return ID;
	return NullID;
}

LWIndirectIndice LWERendererBlockGeometry::MakeDrawCall(uint64_t ID, uint32_t InstanceOffset, uint32_t Offset, uint32_t Count) {
	assert(ID != NullID);
	uint32_t VertID = (uint32_t)((ID & VerticeBits) >> VerticeBitOffset);
	uint32_t IndiceID = (uint32_t)((ID & IndiceBits) >> IndiceBitOffset);
	if (!Count) {
		auto Iter = m_CountMap.find(ID);
		Count = Iter == m_CountMap.end() ? Count : (m_IndiceBuffer ? (*Iter).second.second : (*Iter).second.first);
		Offset = 0;
	}
	return { Count, 1, Offset + IndiceID * m_IndicesPerBlock, (int32_t)(VertID * m_VerticesPerBlock), InstanceOffset };
}

bool LWERendererBlockGeometry::UploadTo(uint64_t ID, uint8_t *VertexPosition, uint8_t *VertexAttributes, uint32_t *Indices, uint32_t VerticeCount, uint32_t IndiceCount, LWVideoDriver *Driver) {
	const uint32_t PositionSize = GetPositionTypeSize();
	const uint32_t AttributeSize = GetAttributeTypeSize();
	uint32_t VertID = (uint32_t)((ID & VerticeBits) >> VerticeBitOffset);
	uint32_t IndiceID = (uint32_t)((ID & IndiceBits) >> IndiceBitOffset);
	if (m_VertexPositionBuffer) Driver->UpdateVideoBuffer(m_VertexPositionBuffer, VertexPosition, PositionSize * VerticeCount, VertID * m_VerticesPerBlock * PositionSize);
	if (m_VertexAttributesBuffer) Driver->UpdateVideoBuffer(m_VertexAttributesBuffer, VertexAttributes, AttributeSize * VerticeCount, VertID * m_VerticesPerBlock * AttributeSize);
	if (m_IndiceBuffer) Driver->UpdateVideoBuffer(m_IndiceBuffer, (uint8_t*)Indices, sizeof(uint32_t) * IndiceCount, IndiceID * m_IndicesPerBlock * sizeof(uint32_t));
	std::pair<uint32_t, uint32_t> p = { VerticeCount, IndiceCount };
	m_CountMap.insert_or_assign(ID, p);
	return true;
}

uint32_t LWERendererBlockGeometry::BuildInputStreams(LWPipeline *Pipeline, LWPipelineInputStream StreamBuffer[LWShader::MaxInputs], LWVideoBuffer *ModelIDBuffer) {
	uint32_t InputCnt = Pipeline->GetInputCount();
	uint32_t PosCnt = std::min<uint32_t>(InputCnt, m_PositionCount);
	uint32_t AttrCnt = InputCnt - PosCnt;
	for (uint32_t i = 0; i < PosCnt; i++) {
		LWShaderInput &In = Pipeline->GetInput(i);
		StreamBuffer[i] = LWPipelineInputStream(m_VertexPositionBuffer, m_PositionLayout[i].GetOffset(), m_PositionTypeSize);
	}
	for (uint32_t i = 0; i < AttrCnt; i++) {
		LWShaderInput &In = Pipeline->GetInput(PosCnt + i);
		if (In.GetInstanceFrequency() == 0) {
			if (i < m_AttributeCount) StreamBuffer[i + PosCnt] = LWPipelineInputStream(m_VertexAttributesBuffer, m_AttributeLayout[i].GetOffset(), m_AttributeTypeSize);
			else StreamBuffer[i + PosCnt] = LWPipelineInputStream(nullptr, 0, 0);
		} else StreamBuffer[i + PosCnt] = LWPipelineInputStream(ModelIDBuffer, 0, sizeof(uint32_t));
	}
	return InputCnt;
}

void LWERendererBlockGeometry::Release(LWVideoDriver *Driver) {
	if (m_VertexPositionBuffer) Driver->DestroyVideoBuffer(m_VertexPositionBuffer);
	if (m_VertexAttributesBuffer) Driver->DestroyVideoBuffer(m_VertexAttributesBuffer);
	if (m_IndiceBuffer) Driver->DestroyVideoBuffer(m_IndiceBuffer);
	return;
}

LWERendererBlockGeometry &LWERendererBlockGeometry::BuildAllPrimitives(LWERenderer *Renderer) {
	GetBoxPrimitive(Renderer);
	GetSpherePrimitive(Renderer);
	GetConePrimtive(Renderer);
	GetCapsulePrimitive(Renderer);
	GetCylinderPrimitive(Renderer);
	GetDomePrimitive(Renderer);
	GetPlanePrimitive(Renderer);
	return *this;
}

LWShaderInput &LWERendererBlockGeometry::GetAttribute(uint32_t Idx) {
	return m_AttributeLayout[Idx];
}

LWShaderInput &LWERendererBlockGeometry::GetPositionAttribute(uint32_t Idx) {
	return m_PositionLayout[Idx];
}


uint8_t *LWERendererBlockGeometry::GetLocalVerticePositionsAt(uint32_t Index) {
	uint8_t *Buffer = m_VertexPositionBuffer->GetLocalBuffer();
	return Buffer ? (Buffer + m_PositionTypeSize * Index) : nullptr;
}

uint8_t *LWERendererBlockGeometry::GetLocalVerticeAttributesAt(uint32_t Index) {
	uint8_t *Buffer = m_VertexAttributesBuffer->GetLocalBuffer();
	return Buffer ? (Buffer + m_AttributeTypeSize * Index) : nullptr;
}

uint32_t *LWERendererBlockGeometry::GetLocalIndicesAt(uint32_t Index) {
	uint32_t *Buffer = (uint32_t*)m_IndiceBuffer->GetLocalBuffer();
	return Buffer ? (Buffer + Index) : nullptr;
}

uint32_t LWERendererBlockGeometry::GetAttributeCount(void) const {
	return m_AttributeCount;
}

uint32_t LWERendererBlockGeometry::GetPositionAttributeCount(void) const {
	return m_PositionCount;
}

uint32_t LWERendererBlockGeometry::FindAttribute(const LWUTF8Iterator &Name, bool Verbose) const {
	uint32_t NameHash = Name.Hash();
	for (uint32_t i = 0; i < m_AttributeCount; i++) {
		if (NameHash == m_AttributeLayout[i].m_NameHash) return i;
	}
	if (Verbose) LWELogEvent<256>("Could not find attribute with name: '{}'", Name);
	return -1;
}

uint32_t LWERendererBlockGeometry::FindAttribute(uint32_t NameHash, bool Verbose) const {
	for (uint32_t i = 0; i < m_AttributeCount; i++) {
		if (NameHash == m_AttributeLayout[i].m_NameHash) return i;
	}
	if (Verbose) LWELogEvent<256>("Could not find attribute with namehash: '{:#x}'", NameHash);
	return -1;
}


uint32_t LWERendererBlockGeometry::FindPositionAttribute(const LWUTF8Iterator &Name, bool Verbose) const {
	uint32_t NameHash = Name.Hash();
	for (uint32_t i = 0; i < m_PositionCount; i++) {
		if (NameHash == m_PositionLayout[i].m_NameHash) return i;
	}
	if (Verbose) LWELogEvent<256>("Could not find position attribute with name: '{}'", Name);
	return -1;
}

uint32_t LWERendererBlockGeometry::FindPositionAttribute(uint32_t NameHash, bool Verbose) const {
	for (uint32_t i = 0; i < m_PositionCount; i++) {
		if (NameHash == m_PositionLayout[i].m_NameHash) return i;
	}
	if (Verbose) LWELogEvent<256>("Could not find position attribute with namehash: '{:#x}'", NameHash);
	return -1;
}

uint32_t LWERendererBlockGeometry::GetVerticeTypeSize(void) const {
	return m_PositionTypeSize+m_AttributeTypeSize;
}

uint32_t LWERendererBlockGeometry::GetPositionTypeSize(void) const {
	return m_PositionTypeSize;
}

uint32_t LWERendererBlockGeometry::GetAttributeTypeSize(void) const {
	return m_AttributeTypeSize;
}

uint32_t LWERendererBlockGeometry::GetNameHash(void) const {
	return m_NameHash;
}

uint64_t LWERendererBlockGeometry::UploadPrimitive(const LWEStaticVertice *VerticePos, const LWEVerticePBR *VerticeAttr, uint32_t VerticeCount, uint32_t *IndiceList, uint32_t IndiceCount, LWERenderer *Renderer) {
	LWAllocator &Allocator = Renderer->GetAllocator();
	bool bHasIndices = m_IndiceBuffer != nullptr;
	uint32_t TexCoordID = FindAttribute("TexCoord", false);
	uint32_t TangentID = FindAttribute("Tangent", false);
	uint32_t NormalID = FindAttribute("Normal", false);
	uint32_t TexCoordOffset = TexCoordID!=-1?m_AttributeLayout[TexCoordID].GetOffset():-1;
	uint32_t TangentOffset = TangentID!=-1?m_AttributeLayout[TangentID].GetOffset():-1;
	uint32_t NormalOffset = NormalID!=-1?m_AttributeLayout[NormalID].GetOffset():-1;
	uint32_t AttributeTypeSize = GetAttributeTypeSize();
	uint32_t PositionTypeSize = GetPositionTypeSize();

	auto WriteVertice = [&TexCoordOffset, &TangentOffset, &NormalOffset, &AttributeTypeSize, &PositionTypeSize](const LWEStaticVertice &VP, const LWEVerticePBR &VA, uint8_t *VertexPosition, uint8_t *VertexAttributes, uint32_t Index) {
		VertexPosition += Index * PositionTypeSize;
		VertexAttributes += Index * AttributeTypeSize;

		*(LWVector4f*)VertexPosition = VP.m_Position;
		if (TexCoordOffset != -1) *(LWVector4f*)(VertexAttributes + TexCoordOffset) = VA.m_TexCoord;
		if (TangentOffset != -1) *(LWVector4f*)(VertexAttributes + TangentOffset) = VA.m_Tangent;
		if (NormalOffset != -1) *(LWVector4f*)(VertexAttributes + NormalOffset) = VA.m_Normal;
	};
	if (!bHasIndices && IndiceCount) VerticeCount = IndiceCount;

	uint32_t *Indices = nullptr;
	uint8_t *VertexPosition = Allocator.Allocate<uint8_t>(PositionTypeSize*VerticeCount);
	uint8_t *VertexAttributes = Allocator.Allocate<uint8_t>(AttributeTypeSize*VerticeCount);
	std::fill(VertexPosition, VertexPosition + (PositionTypeSize * VerticeCount), (uint8_t)0);
	std::fill(VertexAttributes, VertexAttributes + (AttributeTypeSize * VerticeCount), (uint8_t)0);
	
	if (bHasIndices) {
		Indices = Allocator.Allocate<uint32_t>(IndiceCount ? IndiceCount : VerticeCount);
		if (IndiceCount) std::copy(IndiceList, IndiceList + IndiceCount, Indices);
		else {
			IndiceCount = VerticeCount;
			for (uint32_t i = 0; i < IndiceCount; i++) Indices[i] = i;
		}
		for (uint32_t i = 0; i < VerticeCount; i++) WriteVertice(VerticePos[i], VerticeAttr[i], VertexPosition, VertexAttributes, i);
	} else {
		if (IndiceCount) {
			for (uint32_t i = 0; i < IndiceCount; i++) WriteVertice(VerticePos[IndiceList[i]], VerticeAttr[IndiceList[i]], VertexPosition, VertexAttributes, i);
			IndiceCount = 0;
		} else {
			for (uint32_t i = 0; i < VerticeCount; i++) WriteVertice(VerticePos[i], VerticeAttr[i], VertexPosition, VertexAttributes, i);
		}
	}
	uint64_t ID = Allocate(VertexPosition, VertexAttributes, Indices, VerticeCount, IndiceCount, Renderer);
	if (ID == NullID) {
		LWAllocator::Destroy(VertexPosition);
		LWAllocator::Destroy(VertexAttributes);
		LWAllocator::Destroy(Indices);
	}
	return ID;
}

uint64_t LWERendererBlockGeometry::GetBoxPrimitive(LWERenderer *Renderer) {
	if (m_BoxPrimitive != NullID || !Renderer) return m_BoxPrimitive;
	LWEStaticVertice VertPos[24];
	LWEVerticePBR VertAttr[24];
	uint32_t Idxs[36] = {
		0, 2, 1, 1, 2, 3, //Back
		4, 5, 6, 5, 7, 6, //Front
		8, 9,10, 9,11,10, //Left
	   12,14,13,13,14,15, //Right
	   16,17,18,17,19,18, //Top
	   20,22,21,21,22,23 //Btm
	};
	//Back
	VertPos[0] = { LWVector4f(-1.0f, -1.0f, -1.0f, 1.0f) }; VertAttr[0] = { LWVector4f(1.0f, 1.0f, 0.0f, 0.0f), LWVector4f(0.0f, 1.0f, 0.0f, 1.0f), LWVector4f(0.0f, 0.0f, -1.0f, 0.0f) };
	VertPos[1] = { LWVector4f(1.0f, -1.0f, -1.0f, 1.0f) };  VertAttr[1] = { LWVector4f(0.0f, 1.0f, 0.0f, 0.0f), LWVector4f(0.0f, 1.0f, 0.0f, 1.0f), LWVector4f(0.0f, 0.0f, -1.0f, 0.0f) };
	VertPos[2] = { LWVector4f(-1.0f,  1.0f, -1.0f, 1.0f) }; VertAttr[2] = { LWVector4f(1.0f, 0.0f, 0.0f, 0.0f), LWVector4f(0.0f, 1.0f, 0.0f, 1.0f), LWVector4f(0.0f, 0.0f, -1.0f, 0.0f) };
	VertPos[3] = { LWVector4f(1.0f,  1.0f, -1.0f, 1.0f) };  VertAttr[3] = { LWVector4f(0.0f, 0.0f, 0.0f, 0.0f), LWVector4f(0.0f, 1.0f, 0.0f, 1.0f), LWVector4f(0.0f, 0.0f, -1.0f, 0.0f) };

	//Front
	VertPos[4] = { LWVector4f(-1.0f, -1.0f,  1.0f, 1.0f) };  VertAttr[4] = { LWVector4f(0.0f, 1.0f, 0.0f, 0.0f), LWVector4f(0.0f, 1.0f, 0.0f, 1.0f), LWVector4f(0.0f, 0.0f, 1.0f, 0.0f) };
	VertPos[5] = { LWVector4f(1.0f, -1.0f,  1.0f, 1.0f) };   VertAttr[5] = { LWVector4f(1.0f, 1.0f, 0.0f, 0.0f), LWVector4f(0.0f, 1.0f, 0.0f, 1.0f), LWVector4f(0.0f, 0.0f, 1.0f, 0.0f) };
	VertPos[6] = { LWVector4f(-1.0f,  1.0f,  1.0f, 1.0f) };  VertAttr[6] = { LWVector4f(0.0f, 0.0f, 0.0f, 0.0f), LWVector4f(0.0f, 1.0f, 0.0f, 1.0f), LWVector4f(0.0f, 0.0f, 1.0f, 0.0f) };
	VertPos[7] = { LWVector4f(1.0f,  1.0f,  1.0f, 1.0f) };   VertAttr[7] = { LWVector4f(1.0f, 0.0f, 0.0f, 0.0f), LWVector4f(0.0f, 1.0f, 0.0f, 1.0f), LWVector4f(0.0f, 0.0f, 1.0f, 0.0f) };

	//Left
	VertPos[8] = { LWVector4f(-1.0f, -1.0f, -1.0f, 1.0f) };   VertAttr[8] = { LWVector4f(0.0f, 1.0f, 0.0f, 0.0f), LWVector4f(0.0f, 1.0f, 0.0f, 1.0f), LWVector4f(-1.0f, 0.0f, 0.0f, 0.0f) };
	VertPos[9] = { LWVector4f(-1.0f, -1.0f,  1.0f, 1.0f) };   VertAttr[9] = { LWVector4f(1.0f, 1.0f, 0.0f, 0.0f), LWVector4f(0.0f, 1.0f, 0.0f, 1.0f), LWVector4f(-1.0f, 0.0f, 0.0f, 0.0f) };
	VertPos[10] = { LWVector4f(-1.0f,  1.0f, -1.0f, 1.0f) };  VertAttr[10] = { LWVector4f(0.0f, 0.0f, 0.0f, 0.0f), LWVector4f(0.0f, 1.0f, 0.0f, 1.0f), LWVector4f(-1.0f, 0.0f, 0.0f, 0.0f) };
	VertPos[11] = { LWVector4f(-1.0f,  1.0f,  1.0f, 1.0f) };  VertAttr[11] = { LWVector4f(1.0f, 0.0f, 0.0f, 0.0f), LWVector4f(0.0f, 1.0f, 0.0f, 1.0f), LWVector4f(-1.0f, 0.0f, 0.0f, 0.0f) };

	//Right
	VertPos[12] = { LWVector4f(1.0f, -1.0f, -1.0f, 1.0f) };  VertAttr[12] = { LWVector4f(1.0f, 1.0f, 0.0f, 0.0f), LWVector4f(0.0f, 1.0f, 0.0f, 1.0f), LWVector4f(1.0f, 0.0f, 0.0f, 0.0f) };
	VertPos[13] = { LWVector4f(1.0f, -1.0f,  1.0f, 1.0f) };  VertAttr[13] = { LWVector4f(0.0f, 1.0f, 0.0f, 0.0f), LWVector4f(0.0f, 1.0f, 0.0f, 1.0f), LWVector4f(1.0f, 0.0f, 0.0f, 0.0f) };
	VertPos[14] = { LWVector4f(1.0f,  1.0f, -1.0f, 1.0f) };  VertAttr[14] = { LWVector4f(1.0f, 0.0f, 0.0f, 0.0f), LWVector4f(0.0f, 1.0f, 0.0f, 1.0f), LWVector4f(1.0f, 0.0f, 0.0f, 0.0f) };
	VertPos[15] = { LWVector4f(1.0f,  1.0f,  1.0f, 1.0f) };  VertAttr[15] = { LWVector4f(0.0f, 0.0f, 0.0f, 0.0f), LWVector4f(0.0f, 1.0f, 0.0f, 1.0f), LWVector4f(1.0f, 0.0f, 0.0f, 0.0f) };

	//Top
	VertPos[16] = { LWVector4f(-1.0f,  1.0f, -1.0f, 1.0f) };  VertAttr[16] = { LWVector4f(0.0f, 0.0f, 0.0f, 0.0f), LWVector4f(1.0f, 0.0f, 0.0f, 1.0f), LWVector4f(0.0f, 1.0f, 0.0f, 0.0f) };
	VertPos[17] = { LWVector4f(-1.0f,  1.0f,  1.0f, 1.0f) };  VertAttr[17] = { LWVector4f(0.0f, 1.0f, 0.0f, 0.0f), LWVector4f(1.0f, 0.0f, 0.0f, 1.0f), LWVector4f(0.0f, 1.0f, 0.0f, 0.0f) };
	VertPos[18] = { LWVector4f(1.0f,  1.0f, -1.0f, 1.0f) };   VertAttr[18] = { LWVector4f(1.0f, 0.0f, 0.0f, 0.0f), LWVector4f(1.0f, 0.0f, 0.0f, 1.0f), LWVector4f(0.0f, 1.0f, 0.0f, 0.0f) };
	VertPos[19] = { LWVector4f(1.0f,  1.0f,  1.0f, 1.0f) };   VertAttr[19] = { LWVector4f(1.0f, 1.0f, 0.0f, 0.0f), LWVector4f(1.0f, 0.0f, 0.0f, 1.0f), LWVector4f(0.0f, 1.0f, 0.0f, 0.0f) };

	//Bottom
	VertPos[20] = { LWVector4f(-1.0f, -1.0f, -1.0f, 1.0f) };  VertAttr[20] = { LWVector4f(0.0f, 1.0f, 0.0f, 0.0f), LWVector4f(1.0f, 0.0f, 0.0f, 1.0f), LWVector4f(0.0f,-1.0f, 0.0f, 0.0f) };
	VertPos[21] = { LWVector4f(-1.0f, -1.0f,  1.0f, 1.0f) };  VertAttr[21] = { LWVector4f(0.0f, 0.0f, 0.0f, 0.0f), LWVector4f(1.0f, 0.0f, 0.0f, 1.0f), LWVector4f(0.0f,-1.0f, 0.0f, 0.0f) };
	VertPos[22] = { LWVector4f(1.0f, -1.0f, -1.0f, 1.0f) };   VertAttr[22] = { LWVector4f(1.0f, 1.0f, 0.0f, 0.0f), LWVector4f(1.0f, 0.0f, 0.0f, 1.0f), LWVector4f(0.0f,-1.0f, 0.0f, 0.0f) };
	VertPos[23] = { LWVector4f(1.0f, -1.0f,  1.0f, 1.0f) };   VertAttr[23] = { LWVector4f(1.0f, 0.0f, 0.0f, 0.0f), LWVector4f(1.0f, 0.0f, 0.0f, 1.0f), LWVector4f(0.0f,-1.0f, 0.0f, 0.0f) };

	m_BoxPrimitive = UploadPrimitive(VertPos, VertAttr, 24, Idxs, 36, Renderer);
	return m_BoxPrimitive;
}

uint64_t LWERendererBlockGeometry::GetSpherePrimitive(LWERenderer *Renderer) {
	if (m_SpherePrimitive != NullID || !Renderer) return m_SpherePrimitive;
	//Sphere
	const uint32_t verticalSteps = 20;
	const uint32_t horizontalSteps = 20;
	const uint32_t TotalVertices = verticalSteps * horizontalSteps * 6;
	LWEStaticVertice VertPos[TotalVertices];
	LWEVerticePBR VertAttr[TotalVertices];

	uint32_t o = 0;

	float uStep = 1.0f / (horizontalSteps + 1);
	float vStep = 1.0f / verticalSteps;
	float v = LW_PI_2;
	for (uint32_t y = 1; y <= verticalSteps; y++) {
		float nv = LW_PI_2 + LW_PI / (float)verticalSteps * (float)y;
		float h = 0.0f;
		for (uint32_t x = 1; x <= horizontalSteps; x++) {
			float nh = LW_2PI / (float)horizontalSteps * (float)x;
			LWVector3f TopLeftPnt = LWVector3f(cosf(h) * cosf(v), sinf(v), sinf(h) * cosf(v));
			LWVector3f BtmLeftPnt = LWVector3f(cosf(h) * cosf(nv), sinf(nv), sinf(h) * cosf(nv));
			LWVector3f TopRightPnt = LWVector3f(cosf(nh) * cosf(v), sinf(v), sinf(nh) * cosf(v));
			LWVector3f BtmRightPnt = LWVector3f(cosf(nh) * cosf(nv), sinf(nv), sinf(nh) * cosf(nv));

			LWVector2f TCTopLeft = LWVector2f((x - 1) * uStep, (y - 1) * vStep);
			LWVector2f TCBtmLeft = LWVector2f((x - 1) * uStep, y * vStep);
			LWVector2f TCTopRight = LWVector2f(x * uStep, (y - 1) * vStep);
			LWVector2f TCBtmRight = LWVector2f(x * uStep, y * vStep);

			LWVector3f TLUp = LWVector3f();
			LWVector3f TLRight = LWVector3f();
			LWVector3f BLUp = LWVector3f();
			LWVector3f BLRight = LWVector3f();
			LWVector3f TRUp = LWVector3f();
			LWVector3f TRRight = LWVector3f();
			LWVector3f BRUp = LWVector3f();
			LWVector3f BRRight = LWVector3f();

			TopLeftPnt.Othogonal(TLRight, TLUp);
			BtmLeftPnt.Othogonal(BLRight, BLUp);
			TopRightPnt.Othogonal(TRRight, TRUp);
			BtmRightPnt.Othogonal(BRRight, BRUp);

			VertPos[o] = { LWVector4f(TopLeftPnt, 1.0f) };  VertAttr[o++] = { LWVector4f(TCTopLeft, 0.0f, 0.0f), LWVector4f(TLUp, 1.0f), LWVector4f(TopLeftPnt, 0.0f) };
			VertPos[o] = { LWVector4f(TopRightPnt, 1.0f) };  VertAttr[o++] = { LWVector4f(TCTopRight, 0.0f, 0.0f), LWVector4f(TRUp, 1.0f), LWVector4f(TopRightPnt, 0.0f) };
			VertPos[o] = { LWVector4f(BtmRightPnt, 1.0f) };  VertAttr[o++] = { LWVector4f(TCBtmRight, 0.0f, 0.0f), LWVector4f(BRUp, 1.0f), LWVector4f(BtmRightPnt, 0.0f) };
			VertPos[o] = { LWVector4f(TopLeftPnt, 1.0f) };  VertAttr[o++] = { LWVector4f(TCTopLeft, 0.0f, 0.0f), LWVector4f(TLUp, 1.0f), LWVector4f(TopLeftPnt, 0.0f) };
			VertPos[o] = { LWVector4f(BtmRightPnt, 1.0f) };  VertAttr[o++] = { LWVector4f(TCBtmRight, 0.0f, 0.0f), LWVector4f(BRUp, 1.0f), LWVector4f(BtmRightPnt, 0.0f) };
			VertPos[o] = { LWVector4f(BtmLeftPnt, 1.0f) };  VertAttr[o++] = { LWVector4f(TCBtmLeft, 0.0f, 0.0f), LWVector4f(BLUp, 1.0f), LWVector4f(BtmLeftPnt, 0.0f) };
			h = nh;
		}
		v = nv;
	}
	m_SpherePrimitive = UploadPrimitive(VertPos, VertAttr, TotalVertices, nullptr, 0, Renderer);
	return m_SpherePrimitive;
}

uint64_t LWERendererBlockGeometry::GetConePrimtive(LWERenderer *Renderer) {
	if (m_ConePrimitive != NullID || !Renderer) return m_ConePrimitive;
	const uint32_t ConeRadiCnt = 30;
	LWEStaticVertice VertPos[ConeRadiCnt + 2];
	LWEVerticePBR VertAttr[ConeRadiCnt + 2];
	uint32_t Idxs[ConeRadiCnt * 6];

	VertPos[0] = { LWVector4f(0.0f, 0.0f, 0.0f, 1.0f) }; VertAttr[0] = { LWVector4f(0.5f, 0.5f, 0.0f, 0.0f), LWVector4f(1.0f, 0.0f, 0.0f, 1.0f), LWVector4f(0.0f, 0.0f,-1.0f, 0.0f) };
	VertPos[1] = { LWVector4f(0.0f, 0.0f, 1.0f, 1.0f) }; VertAttr[1] = { LWVector4f(0.5f, 0.5f, 0.0f, 0.0f), LWVector4f(1.0f, 0.0f, 0.0f, 1.0f), LWVector4f(0.0f, 0.0f, 1.0f, 0.0f) };
	for (uint32_t n = 0; n < ConeRadiCnt; n++) {
		float T = LW_2PI / ConeRadiCnt * n;
		LWVector2f D = LWVector2f::MakeTheta(T);
		VertPos[n + 2] = { LWVector4f(D.x, D.y, 1.0f, 1.0f) }; VertAttr[n+2] = { LWVector4f(D * 0.5f + 0.5f, 0.0f, 0.0f), LWVector4f(0.0f, 0.0f, -1.0f, 1.0f), LWVector4f(D.x, D.y, 0.0f, 0.0f) };

		uint32_t a = n * 3;
		uint32_t b = (n * 3) + (ConeRadiCnt * 3);
		Idxs[a + 0] = 0;
		Idxs[a + 1] = ((n + 1) % ConeRadiCnt) + 2;
		Idxs[a + 2] = (n)+2;

		Idxs[b + 0] = 1;
		Idxs[b + 1] = (n)+2;
		Idxs[b + 2] = ((n + 1) % ConeRadiCnt) + 2;
	}
	m_ConePrimitive = UploadPrimitive(VertPos, VertAttr, ConeRadiCnt + 2, Idxs, ConeRadiCnt * 6, Renderer);
	return m_ConePrimitive;
}

uint64_t LWERendererBlockGeometry::GetCapsulePrimitive(LWERenderer *Renderer) {
	if (m_CapsulePrimitive != NullID || !Renderer) return m_CapsulePrimitive;
	//Capsule
	const uint32_t verticalSteps = 20;
	const uint32_t horizontalSteps = 20;
	const uint32_t TotalVertices = verticalSteps * horizontalSteps * 6 + horizontalSteps * 6;
	//const uint32_t TotalVertices = horizontalSteps * 6;
	LWEStaticVertice VertPos[TotalVertices];
	LWEVerticePBR VertAttr[TotalVertices];
	uint32_t o = 0;

	float uStep = 1.0f / (horizontalSteps + 1);
	float vStep = 1.0f / verticalSteps;

	//Cylinder portion:
	float h = 0.0f;
	for (uint32_t x = 1; x <= horizontalSteps; x++) {
		float nh = LW_2PI / (float)horizontalSteps * (float)x;
		LWVector3f TopLeftPnt = LWVector3f(0.5f, cosf(h), sinf(h));
		LWVector3f BtmLeftPnt = LWVector3f(-0.5f, cosf(h), sinf(h));
		LWVector3f TopRightPnt = LWVector3f(0.5f, cosf(nh), sinf(nh));
		LWVector3f BtmRightPnt = LWVector3f(-0.5f, cosf(nh), sinf(nh));

		LWVector2f TCTopLeft = LWVector2f((x - 1) * uStep, 1.0f);
		LWVector2f TCBtmLeft = LWVector2f((x - 1) * uStep, 0.0f);
		LWVector2f TCTopRight = LWVector2f(x * uStep, 1.0f);
		LWVector2f TCBtmRight = LWVector2f(x * uStep, 0.0f);

		LWVector3f TLUp, TLRight, BLUp, BLRight, TRUp, TRRight, BRUp, BRRight;
		TopLeftPnt.Othogonal(TLUp, TLRight);
		BtmLeftPnt.Othogonal(BLUp, BLRight);
		TopRightPnt.Othogonal(TRUp, TRRight);
		BtmRightPnt.Othogonal(BRUp, BRRight);

		VertPos[o] = { LWVector4f(TopLeftPnt, 1.0f) };  VertAttr[o++] = { LWVector4f(TCTopLeft, 0.0f, 0.0f), LWVector4f(TLUp, 1.0f), LWVector4f(TopLeftPnt, 0.0f) };
		VertPos[o] = { LWVector4f(BtmRightPnt, 1.0f) };  VertAttr[o++] = { LWVector4f(TCBtmRight, 0.0f, 0.0f), LWVector4f(BRUp, 1.0f), LWVector4f(BtmRightPnt, 0.0f) };
		VertPos[o] = { LWVector4f(TopRightPnt, 1.0f) };  VertAttr[o++] = { LWVector4f(TCTopRight, 0.0f, 0.0f), LWVector4f(TRUp, 1.0f), LWVector4f(TopRightPnt, 0.0f) };
		VertPos[o] = { LWVector4f(TopLeftPnt, 1.0f) };  VertAttr[o++] = { LWVector4f(TCTopLeft, 0.0f, 0.0f), LWVector4f(TLUp, 1.0f), LWVector4f(TopLeftPnt, 0.0f) };
		VertPos[o] = { LWVector4f(BtmLeftPnt, 1.0f) };  VertAttr[o++] = { LWVector4f(TCBtmLeft, 0.0f, 0.0f), LWVector4f(BLUp, 1.0f), LWVector4f(BtmLeftPnt, 0.0f) };
		VertPos[o] = { LWVector4f(BtmRightPnt, 1.0f) };  VertAttr[o++] = { LWVector4f(TCBtmRight, 0.0f, 0.0f), LWVector4f(BRUp, 1.0f), LWVector4f(BtmRightPnt, 0.0f) };
		h = nh;
	}

	//Top+Bottom:
	float v = LW_PI_2;
	for (uint32_t y = 1; y <= verticalSteps; y++) {
		float nv = LW_PI_2 + LW_PI / (float)verticalSteps * (float)y;
		h = 0.0f;
		for (uint32_t x = 1; x <= horizontalSteps; x++) {
			float nh = LW_2PI / (float)horizontalSteps * (float)x;
			float yo = y > verticalSteps / 2 ? -0.5f : 0.5f;
			LWVector3f TopLeftPnt = LWVector3f(yo + sinf(v) * 0.5f, cosf(h) * cosf(v), sinf(h) * cosf(v));
			LWVector3f BtmLeftPnt = LWVector3f(yo + sinf(nv) * 0.5f, cosf(h) * cosf(nv), sinf(h) * cosf(nv));
			LWVector3f TopRightPnt = LWVector3f(yo + sinf(v) * 0.5f, cosf(nh) * cosf(v), sinf(nh) * cosf(v));
			LWVector3f BtmRightPnt = LWVector3f(yo + sinf(nv) * 0.5f, cosf(nh) * cosf(nv), sinf(nh) * cosf(nv));

			LWVector2f TCTopLeft = LWVector2f((x - 1) * uStep, (y - 1) * vStep);
			LWVector2f TCBtmLeft = LWVector2f((x - 1) * uStep, y * vStep);
			LWVector2f TCTopRight = LWVector2f(x * uStep, (y - 1) * vStep);
			LWVector2f TCBtmRight = LWVector2f(x * uStep, y * vStep);

			LWVector3f TLUp = LWVector3f();
			LWVector3f TLRight = LWVector3f();
			LWVector3f BLUp = LWVector3f();
			LWVector3f BLRight = LWVector3f();
			LWVector3f TRUp = LWVector3f();
			LWVector3f TRRight = LWVector3f();
			LWVector3f BRUp = LWVector3f();
			LWVector3f BRRight = LWVector3f();

			TopLeftPnt.Othogonal(TLRight, TLUp);
			BtmLeftPnt.Othogonal(BLRight, BLUp);
			TopRightPnt.Othogonal(TRRight, TRUp);
			BtmRightPnt.Othogonal(BRRight, BRUp);

			VertPos[o] = { LWVector4f(TopLeftPnt, 1.0f) };  VertAttr[o++] = { LWVector4f(TCTopLeft, 0.0f, 0.0f), LWVector4f(TLUp, 1.0f), LWVector4f(TopLeftPnt, 0.0f) };
			VertPos[o] = { LWVector4f(BtmRightPnt, 1.0f) };  VertAttr[o++] = { LWVector4f(TCBtmRight, 0.0f, 0.0f), LWVector4f(BRUp, 1.0f), LWVector4f(BtmRightPnt, 0.0f) };
			VertPos[o] = { LWVector4f(TopRightPnt, 1.0f) };  VertAttr[o++] = { LWVector4f(TCTopRight, 0.0f, 0.0f), LWVector4f(TRUp, 1.0f), LWVector4f(TopRightPnt, 0.0f) };
			VertPos[o] = { LWVector4f(TopLeftPnt, 1.0f) };  VertAttr[o++] = { LWVector4f(TCTopLeft, 0.0f, 0.0f), LWVector4f(TLUp, 1.0f), LWVector4f(TopLeftPnt, 0.0f) };
			VertPos[o] = { LWVector4f(BtmLeftPnt, 1.0f) };  VertAttr[o++] = { LWVector4f(TCBtmLeft, 0.0f, 0.0f), LWVector4f(BLUp, 1.0f), LWVector4f(BtmLeftPnt, 0.0f) };
			VertPos[o] = { LWVector4f(BtmRightPnt, 1.0f) };  VertAttr[o++] = { LWVector4f(TCBtmRight, 0.0f, 0.0f), LWVector4f(BRUp, 1.0f), LWVector4f(BtmRightPnt, 0.0f) };
			h = nh;
		}
		v = nv;
	}
	m_CapsulePrimitive = UploadPrimitive(VertPos, VertAttr, TotalVertices, nullptr, 0, Renderer);
	return m_CapsulePrimitive;
}

uint64_t LWERendererBlockGeometry::GetCylinderPrimitive(LWERenderer *Renderer) {
	if (m_CylinderPrimitive != NullID || !Renderer) return m_CylinderPrimitive;
	const uint32_t horizontalSteps = 20;
	const uint32_t TotalVertices = horizontalSteps * 6 + horizontalSteps * 6;
	
	LWEStaticVertice VertPos[TotalVertices];
	LWEVerticePBR VertAttr[TotalVertices];
	uint32_t o = 0;

	float uStep = 1.0f / (horizontalSteps + 1);

	//Cylinder portion:
	float h = 0.0f;
	for (uint32_t x = 1; x <= horizontalSteps; x++) {
		float nh = LW_2PI / (float)horizontalSteps * (float)x;
		LWVector3f TopLeftPnt = LWVector3f(1.0f, sinf(h), cosf(h));
		LWVector3f BtmLeftPnt = LWVector3f(-1.0f, sinf(h), cosf(h));
		LWVector3f TopRightPnt = LWVector3f(1.0f, sinf(nh), cosf(nh));
		LWVector3f BtmRightPnt = LWVector3f(-1.0f, sinf(nh), cosf(nh));

		LWVector2f TCTopLeft = LWVector2f((x - 1) * uStep, 1.0f);
		LWVector2f TCBtmLeft = LWVector2f((x - 1) * uStep, 0.0f);
		LWVector2f TCTopRight = LWVector2f(x * uStep, 1.0f);
		LWVector2f TCBtmRight = LWVector2f(x * uStep, 0.0f);

		LWVector3f TLUp, TLRight, BLUp, BLRight, TRUp, TRRight, BRUp, BRRight;
		LWVector3f TLNormal = LWVector3f(0.0f, TopLeftPnt.y, TopLeftPnt.z);
		LWVector3f TRNormal = LWVector3f(0.0f, TopRightPnt.y, TopRightPnt.z);
		LWVector3f BLNormal = LWVector3f(0.0f, BtmLeftPnt.y, BtmLeftPnt.z);
		LWVector3f BRNormal = LWVector3f(0.0f, BtmRightPnt.y, BtmRightPnt.z);
		TLNormal.Othogonal(TLUp, TLRight);
		BLNormal.Othogonal(BLUp, BLRight);
		TRNormal.Othogonal(TRUp, TRRight);
		BRNormal.Othogonal(BRUp, BRRight);

		VertPos[o] = { LWVector4f(TopLeftPnt, 1.0f) };  VertAttr[o++] = { LWVector4f(TCTopLeft, 0.0f, 0.0f), LWVector4f(TLUp, 1.0f), LWVector4f(TLNormal, 0.0f) };
		VertPos[o] = { LWVector4f(TopRightPnt, 1.0f) };  VertAttr[o++] = { LWVector4f(TCTopRight, 0.0f, 0.0f), LWVector4f(TRUp, 1.0f), LWVector4f(TRNormal, 0.0f) };
		VertPos[o] = { LWVector4f(BtmRightPnt, 1.0f) };  VertAttr[o++] = { LWVector4f(TCBtmRight, 0.0f, 0.0f), LWVector4f(BRUp, 1.0f), LWVector4f(BRNormal, 0.0f) };
		VertPos[o] = { LWVector4f(TopLeftPnt, 1.0f) };  VertAttr[o++] = { LWVector4f(TCTopLeft, 0.0f, 0.0f), LWVector4f(TLUp, 1.0f), LWVector4f(TLNormal, 0.0f) };
		VertPos[o] = { LWVector4f(BtmRightPnt, 1.0f) };  VertAttr[o++] = { LWVector4f(TCBtmRight, 0.0f, 0.0f), LWVector4f(BRUp, 1.0f), LWVector4f(BRNormal, 0.0f) };
		VertPos[o] = { LWVector4f(BtmLeftPnt, 1.0f) };  VertAttr[o++] = { LWVector4f(TCBtmLeft, 0.0f, 0.0f), LWVector4f(BLUp, 1.0f), LWVector4f(BLNormal, 0.0f) };

		LWVector2f TTCLeft = LWVector2f(TopLeftPnt.x, TopLeftPnt.z) * 0.5f + 0.5f;
		LWVector2f TTCRight = LWVector2f(TopRightPnt.x, TopRightPnt.z) * 0.5f + 0.5f;
		//Top:
		VertPos[o] = { LWVector4f(TopLeftPnt, 1.0f) };  VertAttr[o++] = { LWVector4f(TTCLeft, 0.0f, 0.0f),    LWVector4f(0.0f, 1.0f, 0.0f, 1.0f), LWVector4f(1.0f, 0.0f, 0.0f, 0.0f) };
		VertPos[o] = { LWVector4f(1.0f, 0.0f, 0.0f, 1.0f) };  VertAttr[o++] = { LWVector4f(0.5f, 0.5f, 0.0f, 0.0f), LWVector4f(0.0f, 1.0f, 0.0f, 1.0f), LWVector4f(1.0f, 0.0f, 0.0f, 0.0f) };
		VertPos[o] = { LWVector4f(TopRightPnt, 1.0f) };  VertAttr[o++] = { LWVector4f(TTCRight, 0.0f, 0.0f),  LWVector4f(0.0f, 1.0f, 0.0f, 1.0f), LWVector4f(1.0f, 0.0f, 0.0f, 0.0f) };

		//Btm:
		VertPos[o] = { LWVector4f(BtmLeftPnt, 1.0f) };  VertAttr[o++] = { LWVector4f(TTCLeft, 0.0f, 0.0f), LWVector4f(0.0f, 1.0f, 0.0f, 1.0f), LWVector4f(-1.0f, 0.0f, 0.0f, 0.0f) };
		VertPos[o] = { LWVector4f(BtmRightPnt, 1.0f) };  VertAttr[o++] = { LWVector4f(TTCRight, 0.0f, 0.0f), LWVector4f(0.0f, 1.0f, 0.0f, 1.0f), LWVector4f(-1.0f, 0.0f, 0.0f, 0.0f) };
		VertPos[o] = { LWVector4f(-1.0f, 0.0f, 0.0f, 1.0f) };  VertAttr[o++] = { LWVector4f(0.5f, 0.5f, 0.0f, 0.0f), LWVector4f(0.0f, 1.0f, 0.0f, 1.0f), LWVector4f(-1.0f, 0.0f, 0.0f, 0.0f) };

		h = nh;
	}
	m_CylinderPrimitive = UploadPrimitive(VertPos, VertAttr, TotalVertices, nullptr, 0, Renderer);
	return m_CylinderPrimitive;
}

uint64_t LWERendererBlockGeometry::GetDomePrimitive(LWERenderer *Renderer) {
	if (m_DomePrimitive != NullID || !Renderer) return m_DomePrimitive;
	const uint32_t verticalSteps = 10;
	const uint32_t horizontalSteps = 20;
	const uint32_t TotalVertices = verticalSteps * horizontalSteps * 6 + horizontalSteps * 3;
	LWEStaticVertice VertPos[TotalVertices];
	LWEVerticePBR VertAttr[TotalVertices];
	uint32_t o = 0;

	float uStep = 1.0f / (horizontalSteps + 1);
	float vStep = 1.0f / verticalSteps;
	float v = LW_PI_2;
	for (uint32_t y = 1; y <= verticalSteps; y++) {
		float nv = LW_PI_2 + LW_PI_2 / (float)verticalSteps * (float)y;
		float h = 0.0f;
		for (uint32_t x = 1; x <= horizontalSteps; x++) {
			float nh = LW_2PI / (float)horizontalSteps * (float)x;
			LWVector3f TopLeftPnt = LWVector3f(cosf(h) * cosf(v), sinf(h) * cosf(v), sinf(v));
			LWVector3f BtmLeftPnt = LWVector3f(cosf(h) * cosf(nv), sinf(h) * cosf(nv), sinf(nv));
			LWVector3f TopRightPnt = LWVector3f(cosf(nh) * cosf(v), sinf(nh) * cosf(v), sinf(v));
			LWVector3f BtmRightPnt = LWVector3f(cosf(nh) * cosf(nv), sinf(nh) * cosf(nv), sinf(nv));

			LWVector2f TCTopLeft = LWVector2f((x - 1) * uStep, (y - 1) * vStep);
			LWVector2f TCBtmLeft = LWVector2f((x - 1) * uStep, y * vStep);
			LWVector2f TCTopRight = LWVector2f(x * uStep, (y - 1) * vStep);
			LWVector2f TCBtmRight = LWVector2f(x * uStep, y * vStep);

			LWVector3f TLUp = LWVector3f();
			LWVector3f TLRight = LWVector3f();
			LWVector3f BLUp = LWVector3f();
			LWVector3f BLRight = LWVector3f();
			LWVector3f TRUp = LWVector3f();
			LWVector3f TRRight = LWVector3f();
			LWVector3f BRUp = LWVector3f();
			LWVector3f BRRight = LWVector3f();

			TopLeftPnt.Othogonal(TLRight, TLUp);
			BtmLeftPnt.Othogonal(BLRight, BLUp);
			TopRightPnt.Othogonal(TRRight, TRUp);
			BtmRightPnt.Othogonal(BRRight, BRUp);

			VertPos[o] = { LWVector4f(TopLeftPnt, 1.0f) };  VertAttr[o++] = { LWVector4f(TCTopLeft, 0.0f, 0.0f), LWVector4f(TLUp, 1.0f), LWVector4f(TopLeftPnt, 0.0f) };
			VertPos[o] = { LWVector4f(TopRightPnt, 1.0f) };  VertAttr[o++] = { LWVector4f(TCTopRight, 0.0f, 0.0f), LWVector4f(TRUp, 1.0f), LWVector4f(TopRightPnt, 0.0f) };
			VertPos[o] = { LWVector4f(BtmRightPnt, 1.0f) };  VertAttr[o++] = { LWVector4f(TCBtmRight, 0.0f, 0.0f), LWVector4f(BRUp, 1.0f), LWVector4f(BtmRightPnt, 0.0f) };
			VertPos[o] = { LWVector4f(TopLeftPnt, 1.0f) };  VertAttr[o++] = { LWVector4f(TCTopLeft, 0.0f, 0.0f), LWVector4f(TLUp, 1.0f), LWVector4f(TopLeftPnt, 0.0f) };
			VertPos[o] = { LWVector4f(BtmRightPnt, 1.0f) };  VertAttr[o++] = { LWVector4f(TCBtmRight, 0.0f, 0.0f), LWVector4f(BRUp, 1.0f), LWVector4f(BtmRightPnt, 0.0f) };
			VertPos[o] = { LWVector4f(BtmLeftPnt, 1.0f) };  VertAttr[o++] = { LWVector4f(TCBtmLeft, 0.0f, 0.0f), LWVector4f(BLUp, 1.0f), LWVector4f(BtmLeftPnt, 0.0f) };
			h = nh;
		}
		v = nv;
	}
	float h = 0.0f;
	for (uint32_t x = 1; x <= horizontalSteps; x++) {
		float nh = LW_2PI / (float)horizontalSteps * (float)x;
		LWVector3f PntA = LWVector3f(cosf(h), sinf(h), 0.0f);
		LWVector3f PntB = LWVector3f(cosf(nh), sinf(nh), 0.0f);
		LWVector3f PntC = LWVector3f(0.0f);
		LWVector4f Tangent = LWVector4f(1.0f, 0.0f, 0.0f, 1.0f);
		LWVector4f Normal = LWVector4f(0.0f, -1.0f, 0.0f, 0.0f);
		VertPos[o] = { LWVector4f(PntA, 1.0f) };  VertAttr[o++] = { LWVector4f((PntA.xz() + 1.0f) * 0.5f, 0.0f, 0.0f), Tangent, Normal };
		VertPos[o] = { LWVector4f(PntB, 1.0f) };  VertAttr[o++] = { LWVector4f((PntB.xz() + 1.0f) * 0.5f, 0.0f, 0.0f), Tangent, Normal };
		VertPos[o] = { LWVector4f(PntC, 1.0f) };  VertAttr[o++] = { LWVector4f((PntC.xz() + 1.0f) * 0.5f, 0.0f, 0.0f), Tangent, Normal };
		h = nh;
	}
	m_DomePrimitive = UploadPrimitive(VertPos, VertAttr, TotalVertices, nullptr, 0, Renderer);
	return m_DomePrimitive;
}

uint64_t LWERendererBlockGeometry::GetPlanePrimitive(LWERenderer *Renderer) {
	if (m_PlanePrimitive != NullID || !Renderer) return m_PlanePrimitive;
	LWEStaticVertice VertPos[4];
	LWEVerticePBR VertAttr[4];
	VertPos[0] = { LWVector4f(-1.0f, -1.0f, 0.0f, 1.0f) };  VertAttr[0] = { LWVector4f(0.0f, 0.0f, 0.0f, 1.0f), LWVector4f(1.0f, 0.0f, 0.0f, 1.0f), LWVector4f(0.0f, 0.0f, 1.0f, 0.0f) };
	VertPos[1] = { LWVector4f( 1.0f, -1.0f, 0.0f, 1.0f) };  VertAttr[1] = { LWVector4f(1.0f, 0.0f, 0.0f, 1.0f), LWVector4f(1.0f, 0.0f, 0.0f, 1.0f), LWVector4f(0.0f, 0.0f, 1.0f, 0.0f) };
	VertPos[2] = { LWVector4f(-1.0f,  1.0f, 0.0f, 1.0f) };  VertAttr[2] = { LWVector4f(0.0f, 1.0f, 0.0f, 1.0f), LWVector4f(1.0f, 0.0f, 0.0f, 1.0f), LWVector4f(0.0f, 0.0f, 1.0f, 0.0f) };
	VertPos[3] = { LWVector4f( 1.0f,  1.0f, 0.0f, 1.0f) };  VertAttr[3] = { LWVector4f(1.0f, 1.0f, 0.0f, 1.0f), LWVector4f(1.0f, 0.0f, 0.0f, 1.0f), LWVector4f(0.0f, 0.0f, 1.0f, 0.0f) };
	uint32_t Idxs[6] = { 0,1,2, 1,3,2 };
	
	m_PlanePrimitive = UploadPrimitive(VertPos, VertAttr, 4, Idxs, 6, Renderer);
	return m_PlanePrimitive;
}

LWVideoBuffer *LWERendererBlockGeometry::GetIndiceBuffer(void) {
	return m_IndiceBuffer;
}

LWERendererBlockGeometry::LWERendererBlockGeometry(LWVideoDriver *Driver, LWAllocator &Allocator, uint32_t NameHash, LWShaderInput *PositionLayout, uint32_t PositionLayoutCount, LWShaderInput *AttributeLayout, uint32_t AttributeLayoutCount, bool LocalCopy, uint32_t VerticesPerBlock, uint32_t MaxVerticeBlocks, uint32_t IndicesPerBlock, uint32_t MaxIndiceBlocks) : m_VerticeBlocks(MaxVerticeBlocks), m_IndiceBlocks(MaxIndiceBlocks), m_VerticesPerBlock(VerticesPerBlock), m_IndicesPerBlock(IndicesPerBlock), m_NameHash(NameHash), m_AttributeCount(AttributeLayoutCount), m_PositionCount(PositionLayoutCount) {
	assert(VerticesPerBlock>0 && MaxVerticeBlocks>0 && m_PositionCount>0);
	std::copy(PositionLayout, PositionLayout + PositionLayoutCount, m_PositionLayout);
	std::copy(AttributeLayout, AttributeLayout + AttributeLayoutCount, m_AttributeLayout);
	m_PositionTypeSize = LWShader::GenerateInputOffsets(m_PositionCount, m_PositionLayout);
	m_AttributeTypeSize = LWShader::GenerateInputOffsets(m_AttributeCount, m_AttributeLayout);
	assert(m_PositionTypeSize >= sizeof(float) * 4);

	uint32_t IndicesSize = sizeof(uint32_t) * (IndicesPerBlock * MaxIndiceBlocks);
	m_VertexPositionBuffer = Driver->CreateVideoBuffer(LWVideoBuffer::Vertex, LWVideoBuffer::WriteNoOverlap | (LocalCopy ? LWVideoBuffer::LocalCopy : 0), m_PositionTypeSize, VerticesPerBlock*MaxVerticeBlocks, Allocator, nullptr);
	m_VertexAttributesBuffer = Driver->CreateVideoBuffer(LWVideoBuffer::Vertex, LWVideoBuffer::WriteNoOverlap | (LocalCopy ? LWVideoBuffer::LocalCopy : 0), m_AttributeTypeSize, VerticesPerBlock * MaxVerticeBlocks, Allocator, nullptr);
	if (IndicesSize) m_IndiceBuffer = Driver->CreateVideoBuffer<uint32_t>(LWVideoBuffer::Index32, LWVideoBuffer::WriteNoOverlap | (LocalCopy ? LWVideoBuffer::LocalCopy : 0), IndicesSize, Allocator, nullptr);
}

//Pending implementations:

//LWERendererPendingBlockGeometry
LWERendererPendingBlockGeometry::LWERendererPendingBlockGeometry(uint8_t *VertexPosition, uint8_t *VertexAttributes, uint32_t *Indices, uint32_t VerticeCount, uint32_t IndiceCount) : m_VertexPosition(VertexPosition), m_VertexAttributes(VertexAttributes), m_Indices(Indices), m_VerticeCount(VerticeCount), m_IndiceCount(IndiceCount) {}


//LWERenderPendingBuffer:
LWERenderPendingBuffer::LWERenderPendingBuffer(uint8_t *Data, uint32_t BufferType, uint32_t BufferUsage, uint32_t TypeSize, uint32_t Count, uint32_t Offset) : m_Data(Data), m_BufferType(BufferType), m_BufferUsage(BufferUsage), m_TypeSize(TypeSize), m_Count(Count), m_Offset(Offset) {}

//LWERenderTextureProps
LWERenderTextureProps::LWERenderTextureProps(uint32_t TextureState) : m_TextureState(TextureState) {}

LWERenderTextureProps::LWERenderTextureProps(uint32_t TextureState, uint32_t PackType, uint32_t TextureType, uint32_t Layers, uint32_t Samples) : m_TextureState(TextureState), m_PackType(PackType), m_TextureType(TextureType), m_Layers(Layers), m_Samples(Samples) {}


//LWERenderPendingTexture:
bool LWERenderPendingTexture::isDynamicSized(void) const {
	return m_DynamicSize.x > 0.0f;
}

LWERenderPendingTexture::LWERenderPendingTexture(LWImage *Image, uint32_t TextureState) : m_Image(Image), m_TexProps(TextureState) {}

LWERenderPendingTexture::LWERenderPendingTexture(const LWVector2i &StaticSize, const LWERenderTextureProps &TexProps) : m_StaticSize(StaticSize), m_TexProps(TexProps) {}

LWERenderPendingTexture::LWERenderPendingTexture(const LWVector2f &DynamicSize, const LWERenderTextureProps &TexProps, uint32_t NamedDynamic) : m_DynamicSize(DynamicSize), m_NamedDynamic(NamedDynamic), m_TexProps(TexProps) {}

//LWEFramebufferTexture:
uint32_t LWERenderFramebufferTexture::GetNamedMipmap(void) const {
	return LWBitFieldGet(NamedMipmap, m_NamedFlags);
}

uint32_t LWERenderFramebufferTexture::GetNamedLayer(void) const {
	return LWBitFieldGet(NamedLayer, m_NamedFlags);
}

uint32_t LWERenderFramebufferTexture::GetNamedFace(void) const {
	return LWBitFieldGet(NamedFace, m_NamedFlags);
}

bool LWERenderFramebufferTexture::isFramebufferName(void) const {
	return (m_NamedFlags & FrameBufferName) != 0;
}

LWERenderFramebufferTexture::LWERenderFramebufferTexture(const LWUTF8Iterator &Name, uint32_t Mipmap, uint32_t Layer, uint32_t Face, bool bIsFramebufferName) : m_NameHash(Name.Hash()), m_NamedFlags((Mipmap<<NamedMipmapBitsOffset) | (Layer << NamedLayerBitsOffset) | (Face << NamedFaceBitsOffset) | (bIsFramebufferName ? FrameBufferName : 0)) {}

LWERenderFramebufferTexture::LWERenderFramebufferTexture(uint32_t NameHash, uint32_t Mipmap, uint32_t Layer, uint32_t Face, bool bIsFramebufferName) : m_NameHash(NameHash), m_NamedFlags((Mipmap << NamedMipmapBitsOffset) | (Layer << NamedLayerBitsOffset) | (Face << NamedFaceBitsOffset) | (bIsFramebufferName ? FrameBufferName : 0)) {}

LWERenderFramebufferTexture::LWERenderFramebufferTexture(const LWERenderTextureProps &Props) : m_TexProps(Props) {}

//LEWRenderPendingFramebuffer:
bool LWERenderPendingFrameBuffer::isDynamicSized(void) const {
	return m_DynamicSize.x > 0.0f;
}

LWERenderPendingFrameBuffer::LWERenderPendingFrameBuffer(const LWVector2i &StaticSize) : m_StaticSize(StaticSize) {}

LWERenderPendingFrameBuffer::LWERenderPendingFrameBuffer(const LWVector2f &DynamicSize, uint32_t NamedDynamic) : m_DynamicSize(DynamicSize), m_NamedDynamic(NamedDynamic) {}


//LWERenderPendingResource:
bool LWERenderPendingResource::isNoDiscard(void) const {
	return (m_Flag & NoDiscard) != 0;
}

bool LWERenderPendingResource::isDestroyResource(void) const {
	return (m_Flag & DestroyResource) != 0;
}

bool LWERenderPendingResource::isWriteOverlap(void) const {
	return (m_Flag & WriteOverlap) != 0;
}

uint32_t LWERenderPendingResource::GetType(void) const {
	return LWBitFieldGet(Type, m_Flag);
}

void LWERenderPendingResource::Finished(void) {
	if (isNoDiscard()) return;
	uint32_t lType = GetType();
	if (lType == tBuffer) LWAllocator::Destroy(m_Buffer.m_Data);
	else if (lType == tTexture) LWAllocator::Destroy(m_Texture.m_Image);
	else if (lType == tBlockGeometry) {
		LWAllocator::Destroy(m_BlockGeometry.m_VertexPosition);
		LWAllocator::Destroy(m_BlockGeometry.m_VertexAttributes);
		LWAllocator::Destroy(m_BlockGeometry.m_Indices);
	}
};

LWERenderPendingResource::LWERenderPendingResource(uint32_t ID, uint32_t Flag, const LWERenderPendingBuffer &Buffer) : m_Buffer(Buffer), m_ID(ID), m_Flag(Flag|tBuffer) {}

LWERenderPendingResource::LWERenderPendingResource(uint32_t ID, uint32_t Flag, const LWERenderPendingTexture &Texture) : m_Texture(Texture), m_ID(ID), m_Flag(Flag|tTexture) {}

LWERenderPendingResource::LWERenderPendingResource(uint32_t ID, uint32_t Flag, const LWERenderPendingFrameBuffer &FrameBuffer) : m_FrameBuffer(FrameBuffer), m_ID(ID), m_Flag(Flag | tFrameBuffer) {}

LWERenderPendingResource::LWERenderPendingResource(uint32_t ID, const LWUTF8Iterator &Name, uint32_t Flag, const LWERenderPendingBuffer &Buffer) : m_Buffer(Buffer), m_ID(ID), m_NameHash(Name.Hash()), m_Flag(Flag|tBuffer) {}

LWERenderPendingResource::LWERenderPendingResource(uint32_t ID, const LWUTF8Iterator &Name, uint32_t Flag, const LWERenderPendingTexture &Texture) : m_Texture(Texture), m_ID(ID), m_NameHash(Name.Hash()), m_Flag(Flag|tTexture) {}

LWERenderPendingResource::LWERenderPendingResource(uint32_t ID, const LWUTF8Iterator &Name, uint32_t Flag, const LWERenderPendingFrameBuffer &FrameBuffer) : m_FrameBuffer(FrameBuffer), m_ID(ID), m_NameHash(Name.Hash()), m_Flag(Flag|tFrameBuffer) {}

LWERenderPendingResource::LWERenderPendingResource(uint64_t ID, const LWUTF8Iterator &Name, uint32_t Flag, const LWERendererPendingBlockGeometry &BlockGeom) : m_BlockGeometry(BlockGeom), m_ID(ID), m_NameHash(Name.Hash()), m_Flag(Flag | tBlockGeometry) {}

LWERenderPendingResource::LWERenderPendingResource(uint32_t ID, uint32_t NameHash, uint32_t Flag, const LWERenderPendingBuffer &Buffer) : m_Buffer(Buffer), m_ID(ID), m_NameHash(NameHash), m_Flag(Flag | tBuffer) {}

LWERenderPendingResource::LWERenderPendingResource(uint32_t ID, uint32_t NameHash, uint32_t Flag, const LWERenderPendingTexture &Texture) : m_Texture(Texture), m_ID(ID), m_NameHash(NameHash), m_Flag(Flag | tTexture) {}

LWERenderPendingResource::LWERenderPendingResource(uint32_t ID, uint32_t NameHash, uint32_t Flag, const LWERenderPendingFrameBuffer &Framebuffer) : m_FrameBuffer(Framebuffer), m_ID(ID), m_NameHash(NameHash), m_Flag(Flag | tFrameBuffer) {}

LWERenderPendingResource::LWERenderPendingResource(uint64_t ID, uint32_t NameHash, uint32_t Flag, const LWERendererPendingBlockGeometry &BlockGeom) : m_BlockGeometry(BlockGeom), m_ID(ID), m_NameHash(NameHash), m_Flag(Flag | tBlockGeometry) {}

LWERenderPendingResource::LWERenderPendingResource(uint32_t ID, const LWERenderPendingBuffer &) : m_ID(ID), m_Flag(DestroyResource | tBuffer) {}

LWERenderPendingResource::LWERenderPendingResource(uint32_t ID, const LWERenderPendingTexture &) : m_ID(ID), m_Flag(DestroyResource | tTexture) {}

LWERenderPendingResource::LWERenderPendingResource(uint32_t ID, const LWERenderPendingFrameBuffer &) : m_ID(ID), m_Flag(DestroyResource | tFrameBuffer) {}

LWERenderPendingResource::LWERenderPendingResource(uint64_t ID, uint32_t NameHash) : m_ID(ID), m_NameHash(NameHash), m_Flag(DestroyResource | tBlockGeometry) {}
