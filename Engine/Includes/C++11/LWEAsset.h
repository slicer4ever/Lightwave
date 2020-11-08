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
	char8_t m_AssetPath[256]="";
	uint32_t m_Type = 0;
	void *m_Asset = nullptr;
};

class LWEAssetManager {
public:
	enum {
		AssetPoolSize = 256
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

	LWVideoDriver *GetDriver(void);

	LWELocalization *GetLocalization(void);

	LWAllocator &GetAllocator(void);

	LWEAsset *GetAsset(uint32_t i);

	uint32_t GetAssetCount(void);

	LWEAssetManager(LWVideoDriver *Driver, LWELocalization *Localization, LWAllocator &Allocator);

	~LWEAssetManager();
private:

	LWEAsset **m_AssetPools = nullptr;
	std::map<uint32_t, LWEAsset*> m_AssetMap;
	LWAllocator &m_Allocator;
	LWELocalization *m_Localization = nullptr;
	LWVideoDriver *m_Driver = nullptr;
	uint32_t m_AssetCount = 0;
	uint32_t m_PoolCount = 0;
};

#endif

