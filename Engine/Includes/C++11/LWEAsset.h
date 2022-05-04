#ifndef LWEASSET_H
#define LWEASSET_H
#include <LWCore/LWTypes.h>
#include <LWCore/LWUnicode.h>
#include <LWVideo/LWTypes.h>
#include <LWAudio/LWTypes.h>
#include <LWVideo/LWFont.h>
#include <LWVideo/LWTexture.h>
#include <LWVideo/LWShader.h>
#include <LWVideo/LWPipeline.h>
#include <LWAudio/LWAudioStream.h>
#include <LWEVideoPlayer.h>
#include <map>
#include <typeinfo>
#include "LWETypes.h"
#include "LWEXML.h"

static const uint32_t LWEShaderCount = 18;
extern const char8_t *LWEShaderSources[LWEShaderCount]; //Defined in LWEAsset.cpp
static const char8_t LWEShaderNames[LWEShaderCount][32] = {
	"FontVertex",  "UIVertex", "PP_Vertex", "FontColor", "FontMSDF", "UITexture", "UIColor", "UIYUVTexture", "PP_Texture",
	"Structures", "Utilitys", "LightUtilitys", "GausianBlurVertex", "GausianBlurPixel", "DepthVertex", "PBRVertex", "PBRPixel", "PP_PBRComposite"
};

//Mapped input's for built-in types(last one is blank to denote limiter):
static const char8_t *LWEShaderInputNames[] = { "UIVertex", "TextureVertex", "PBRStaticVertex", "PBRSkeletonVertex", "PBRIndexStaticVertex", "PBRIndexSkeletonVertex", "StaticDepthVertex", "SkeletonDepthVertex", "IndexStaticDepthVertex", "IndexSkeletonDepthVertex" };
static const uint32_t LWEShaderInputCount = sizeof(LWEShaderInputNames) / sizeof(char8_t*);

static const LWShaderInput LWEShaderInputMapping[LWEShaderInputCount][7] = {
	{ LWShaderInput("Position", LWShaderInput::Vec4, 1), LWShaderInput("Color", LWShaderInput::Vec4, 1), LWShaderInput("TexCoord", LWShaderInput::Vec4, 1) }, //UIVertex
	{ LWShaderInput("Position", LWShaderInput::Vec4, 1), LWShaderInput("TexCoord", LWShaderInput::Vec4, 1) }, //TextureVertex
	{ LWShaderInput("v.Position", LWShaderInput::Vec4, 1), LWShaderInput("v.TexCoord", LWShaderInput::Vec4, 1), LWShaderInput("v.Tangent", LWShaderInput::Vec4, 1), LWShaderInput("v.Normal", LWShaderInput::Vec4, 1) }, //PBRStaticVertex
	{ LWShaderInput("v.Position", LWShaderInput::Vec4, 1), LWShaderInput("v.BoneWeight", LWShaderInput::Vec4, 1), LWShaderInput("v.BoneIndices", LWShaderInput::iVec4, 1), LWShaderInput("v.TexCoord", LWShaderInput::Vec4, 1), LWShaderInput("v.Tangent", LWShaderInput::Vec4, 1), LWShaderInput("v.Normal", LWShaderInput::Vec4, 1) }, //PBRSkeletonVertex
	{ LWShaderInput("v.Position", LWShaderInput::Vec4, 1), LWShaderInput("v.TexCoord", LWShaderInput::Vec4, 1), LWShaderInput("v.Tangent", LWShaderInput::Vec4, 1), LWShaderInput("v.Normal", LWShaderInput::Vec4, 1), LWShaderInput("v.ModelIndex", LWShaderInput::UInt, 1, 1) }, //PBRIndexStaticVertex
	{ LWShaderInput("v.Position", LWShaderInput::Vec4, 1), LWShaderInput("v.BoneWeight", LWShaderInput::Vec4, 1), LWShaderInput("v.BoneIndices", LWShaderInput::iVec4, 1), LWShaderInput("v.TexCoord", LWShaderInput::Vec4, 1), LWShaderInput("v.Tangent", LWShaderInput::Vec4, 1), LWShaderInput("v.Normal", LWShaderInput::Vec4, 1), LWShaderInput("v.ModelIndex", LWShaderInput::UInt, 1, 1) }, //PBRIndexSkeletonVertex
	{ LWShaderInput("v.Position", LWShaderInput::Vec4, 1) }, //StaticDepthVertex
	{ LWShaderInput("v.Position", LWShaderInput::Vec4, 1), LWShaderInput("v.BoneWeight", LWShaderInput::Vec4, 1), LWShaderInput("v.BoneIndices", LWShaderInput::iVec4, 1) }, //SkeletonDepthVertex
	{ LWShaderInput("v.Position", LWShaderInput::Vec4, 1), LWShaderInput("v.ModelIndex", LWShaderInput::UInt, 1, 1) }, //StaticDepthVertex
	{ LWShaderInput("v.Position", LWShaderInput::Vec4, 1), LWShaderInput("v.BoneWeight", LWShaderInput::Vec4, 1), LWShaderInput("v.BoneIndices", LWShaderInput::iVec4, 1),  LWShaderInput("v.ModelIndex", LWShaderInput::UInt, 1, 1) } //SkeletonDepthVertex
};
static const uint32_t LWEShaderInputMapCount[LWEShaderInputCount] = { 3, 2, 4, 6, 5, 7, 1, 3, 2, 4 };

class LWEAsset {
public:
	enum {
		Texture,
		Font,
		Shader,
		Video,
		AudioStream,
		Pipeline,
		VideoBuffer
	};

	template<class Type>
	Type *As(void) {
		return (Type*)m_Asset;
	}

	void *GetAsset(void);

	LWUTF8Iterator GetAssetPath(void) const;

	uint32_t GetType(void) const;

	void Release(LWVideoDriver *Driver);

	LWEAsset(uint32_t Type, void *Asset, const LWUTF8Iterator &AssetPath);

	LWEAsset();

private:
	char8_t m_AssetPath[256]={};
	uint32_t m_Type = 0;
	void *m_Asset = nullptr;
};

class LWEAssetManager {
public:
	enum {
		AssetPoolSize = 256
	};
	/*!< \brief parse's children Texture, Shader, VideoBuffer, ShaderBuilder, AudioStream, Video, Pipeline, PipelineBuilder nodes.
	*	Attribute: CompiledShaderCache, if included any shader that doesn't include the ForceCompile attribute will be compiled for future iterations on platforms that support compiling shaders. */
	static bool XMLParser(LWEXMLNode *N, void *UserData, LWEXML *XML);

	static bool XMLParseFont(LWEXMLNode *N, LWEAssetManager *AM);
	/*!< \brief add LINEAR no valued-attribute to not have the image by SRGBA formated. */
	static bool XMLParseTexture(LWEXMLNode *N, LWEAssetManager *AM);

	/*<! \brief Parse's an XML Shader node
		 \note Special Path parameters that are equal to the above LWEShaderNames will translate to that shader source.  equally doing #include <LWEShaderName> will include that bit of shader source into a file.
	*/
	static bool XMLParseShader(LWEXMLNode *N, LWEAssetManager *AM);

	static bool XMLParseVideoBuffer(LWEXMLNode *N, LWEAssetManager *AM);

	static bool XMLParseShaderBuilder(LWEXMLNode *N, LWEAssetManager *AM);

	static bool XMLParseAudioStream(LWEXMLNode *N, LWEAssetManager *AM);

	static bool XMLParseVideo(LWEXMLNode *N, LWEAssetManager *AM);
	
	static bool XMLParsePipeline(LWEXMLNode *N, LWEAssetManager *AM);

	static bool XMLParsePipelineBuilder(LWEXMLNode *N, LWEAssetManager *AM);

	LWEAssetManager &SetCompiledShaderCache(const LWUTF8Iterator &Path);

	LWEAsset *GetAsset(const LWUTF8Iterator &Name);

	template<class Type>
	Type *GetAsset(const LWUTF8Iterator &Name) {
		LWEAsset *A = GetAsset(Name);
		if (!A) return nullptr;
		return A->As<Type>();
	}

	bool InsertAsset(const LWUTF8Iterator &Name, const LWEAsset &A);

	bool InsertAsset(const LWUTF8Iterator &Name, void *Asset, uint32_t AssetType, const LWUTF8Iterator &AssetPath);

	bool InsertAssetReference(const LWUTF8Iterator &Name, const LWUTF8Iterator &RefName);

	bool HasAsset(const LWUTF8Iterator &Name);

	LWVideoDriver *GetDriver(void);

	LWELocalization *GetLocalization(void);

	LWAllocator &GetAllocator(void);

	LWEAsset *GetAsset(uint32_t i);

	LWUTF8Iterator GetCompiledShaderCache(void);

	uint32_t GetAssetCount(void);

	LWEAssetManager(LWVideoDriver *Driver, LWELocalization *Localization, LWAllocator &Allocator);

	~LWEAssetManager();
private:
	char8_t m_CompiledShaderCache[256] = {};
	LWEAsset **m_AssetPools = nullptr;
	std::map<uint32_t, LWEAsset*> m_AssetMap;
	LWAllocator &m_Allocator;
	LWELocalization *m_Localization = nullptr;
	LWVideoDriver *m_Driver = nullptr;
	uint32_t m_AssetCount = 0;
	uint32_t m_PoolCount = 0;
};

#endif

