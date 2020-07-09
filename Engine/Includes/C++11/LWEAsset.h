#ifndef LWEASSET_H
#define LWEASSET_H
#include <LWCore/LWTypes.h>
#include <LWCore/LWText.h>
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

	LWTexture *AsTexture(void);

	LWFont *AsFont(void);

	LWShader *AsShader(void);

	LWEVideoPlayer *AsVideoPlayer(void);

	LWAudioStream *AsAudioStream(void);

	LWVideoBuffer *AsVideoBuffer(void);

	LWPipeline *AsPipeline(void);

	void *GetAsset(void);

	const char *GetAssetPath(void);

	uint32_t GetType(void);

	LWEAsset(uint32_t Type, void *Asset, const char *AssetPath);

	LWEAsset();
private:
	char m_AssetPath[256];
	uint32_t m_Type;
	void *m_Asset;
};

class LWEAssetManager {
public:
	enum {
		MaxAssets = 256
	};
	static bool XMLParser(LWEXMLNode *N, void *UserData, LWEXML *XML);

	static bool XMLParseFont(LWEXMLNode *N, LWEAssetManager *AM);

	static bool XMLParseTexture(LWEXMLNode *N, LWEAssetManager *AM);

	static bool XMLParseShader(LWEXMLNode *N, LWEAssetManager *AM);

	static bool XMLParseVideoBuffer(LWEXMLNode *N, LWEAssetManager *AM);

	static bool XMLParseShaderBuilder(LWEXMLNode *N, LWEAssetManager *AM);

	static bool XMLParseAudioStream(LWEXMLNode *N, LWEAssetManager *AM);

	static bool XMLParseVideo(LWEXMLNode *N, LWEAssetManager *AM);
	
	static bool XMLParsePipeline(LWEXMLNode *N, LWEAssetManager *AM);

	LWEAsset *GetAsset(const LWText &Name);

	template<class Type>
	Type *GetAsset(const LWText &Name) {
		LWEAsset *A = GetAsset(Name);
		if (!A) return nullptr;
		if (typeid(Type) == typeid(LWTexture)) return (A->GetType() == LWEAsset::Texture) ? (Type*)A->GetAsset() : nullptr;
		else if (typeid(Type) == typeid(LWFont)) return (A->GetType() == LWEAsset::Font) ? (Type*)A->GetAsset() : nullptr;
		else if (typeid(Type) == typeid(LWShader)) return (A->GetType() == LWEAsset::Shader) ? (Type*)A->GetAsset() : nullptr;
		else if (typeid(Type) == typeid(LWPipeline)) return (A->GetType() == LWEAsset::Pipeline) ? (Type*)A->GetAsset() : nullptr;
		else if (typeid(Type) == typeid(LWAudioStream)) return (A->GetType() == LWEAsset::AudioStream) ? (Type*)A->GetAsset() : nullptr;
		else if (typeid(Type) == typeid(LWEVideoPlayer)) return (A->GetType() == LWEAsset::Video) ? (Type*)A->GetAsset() : nullptr;
		else if (typeid(Type) == typeid(LWVideoBuffer)) return (A->GetType() == LWEAsset::VideoBuffer) ? (Type*)A->GetAsset() : nullptr;
		return nullptr;
	}

	bool InsertAsset(const LWText &Name, void *Asset, uint32_t AssetType, const char *AssetPath);

	bool InsertAssetReference(const LWText &Name, const LWText &RefName);

	LWVideoDriver *GetDriver(void);

	LWELocalization *GetLocalization(void);

	LWAllocator *GetAllocator(void);

	LWEAsset *GetAsset(uint32_t i);

	uint32_t GetAssetCount(void);

	LWEAssetManager(LWVideoDriver *Driver, LWELocalization *Localization, LWAllocator &Allocator);

	~LWEAssetManager();
private:
	LWEAsset m_AssetTable[MaxAssets];
	std::map<uint32_t, LWEAsset*> m_AssetMap;
	LWELocalization *m_Localization;
	LWVideoDriver *m_Driver;
	LWAllocator *m_Allocator;
	uint32_t m_AssetCount;
};

#endif

