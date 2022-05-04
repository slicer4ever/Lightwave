#ifndef LWEXML_H
#define LWEXML_H
#include <functional>
#include "LWETypes.h"
#include <LWCore/LWUnicode.h>

#define LWEXMLMAXNAMELEN 32
#define LWEXMLMAXVALUELEN 256
#define LWEXMLMAXTEXTLEN 1024

/*!< \brief Callback for xml parsing various nodes.  return true if the parser also parses the children nodes, otherwise return false to let the xml process parse children. */
typedef std::function<bool(LWEXMLNode*, void*, LWEXML*)> LWEXMLParseCallback;

struct LWEXMLAttribute {
	char8_t m_Name[LWEXMLMAXNAMELEN]={};
	char8_t m_Value[LWEXMLMAXVALUELEN]={};
	uint32_t m_NameHash = LWCrypto::FNV1AHash;

	LWEXMLAttribute &SetName(const LWUTF8Iterator &Name);

	LWEXMLAttribute &SetValue(const LWUTF8Iterator &Value);

	LWUTF8Iterator GetName(void) const;

	LWUTF8Iterator GetValue(void) const;

	LWEXMLAttribute(const LWUTF8Iterator &Name, const LWUTF8Iterator &Value);

	LWEXMLAttribute() = default;
};

struct LWEXMLNode {
	enum {
		MaxAttributes = 32
	};
	char8_t m_Text[LWEXMLMAXTEXTLEN]={};
	char8_t m_Name[LWEXMLMAXNAMELEN]={};
	LWEXMLAttribute m_Attributes[MaxAttributes];
	LWEXMLNode *m_Parent = nullptr;
	LWEXMLNode *m_Next = nullptr;
	LWEXMLNode *m_FirstChild = nullptr;
	LWEXMLNode *m_LastChild = nullptr;
	uint32_t m_AttributeCount = 0;
	uint32_t m_NameHash = LWCrypto::FNV1AHash;

	bool PushAttribute(const LWEXMLAttribute &Attr);

	bool RemoveAttribute(uint32_t i);

	bool RemoveAttribute(LWEXMLAttribute *Attr);

	LWUTF8Iterator GetName(void) const;

	LWUTF8Iterator GetText(void) const;

	LWEXMLNode &SetName(const LWUTF8Iterator &Name);

	LWEXMLNode &SetText(const LWUTF8Iterator &Text);

	LWEXMLAttribute *FindAttribute(const LWUTF8Iterator &Name);

	LWEXMLAttribute *FindAttribute(uint32_t NameHash);

	LWEXMLNode(const LWUTF8Iterator &Name);

	LWEXMLNode() = default;
};

struct LWEXMLParser {
	char m_Name[LWEXMLMAXNAMELEN]={};
	LWEXMLParseCallback m_Callback = nullptr;
	void *m_UserData = nullptr;
	uint32_t m_NameHash = LWCrypto::FNV1AHash;

	LWEXMLParser(const LWUTF8Iterator &Name, LWEXMLParseCallback Callback, void *UserData);

	LWEXMLParser() = default;
};

class LWEXML {
public:
	enum {
		NodePoolSize = 256,
		MaxParsers = 32
	};

	static bool LoadFile(LWEXML &XML, LWAllocator &Allocator, const LWUTF8Iterator &Path, bool StripFormatting, LWEXMLNode *Parent, LWEXMLNode *Prev, LWFileStream *ExistingStream = nullptr);

	static bool LoadFile(LWEXML &XML, LWAllocator &Allocator, const LWUTF8Iterator &Path, bool StripFormatting, LWFileStream *ExistingStream = nullptr);

	static bool ParseBuffer(LWEXML &XML, LWAllocator &Allocator, const LWUTF8Iterator &Source, bool StripFormatting, LWEXMLNode *Parent, LWEXMLNode *Prev);

	static bool ParseBuffer(LWEXML &XML, LWAllocator &Allocator, const LWUTF8Iterator &Source, bool StripFormatting);

	static uint32_t ConstructBuffer(LWEXML &XML, char8_t *Buffer, uint32_t BufferLen, bool Format);

	LWEXMLNode *NextNode(LWEXMLNode *Current, bool SkipChildren=false);

	LWEXMLNode *NextNode(LWEXMLNode *Current, LWEXMLNode *Top, bool SkipChildren = false);

	LWEXMLNode *NextNodeWithName(LWEXMLNode *Current, const LWUTF8Iterator &Name, bool SkipChildren =false);

	template<class Method, class Obj>
	LWEXML &PushMethodParser(const LWUTF8Iterator &XMLNodeName, Method CB, Obj *O, void *UserData) {
		return PushParser(XMLNodeName, std::bind(CB, O, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), UserData);
	}

	LWEXML &PushParser(const LWUTF8Iterator &XMLNodeName, std::function<bool(LWEXMLNode*, void*, LWEXML*)> Callback, void *UserData);

	LWEXML &Process(void);

	LWEXMLNode *GetInsertedNodeAfter(LWEXMLNode *Parent, LWEXMLNode *Prev, LWAllocator &Allocator);

	LWEXMLNode *GetFirstNode(void);

	LWEXMLNode *GetLastNode(void);

	LWEXML() = default;

	~LWEXML();

private:
	LWEXMLParser m_Parsers[MaxParsers];
	LWEXMLNode **m_NodePool = nullptr;
	uint32_t m_NodeCount = 0;
	uint32_t m_ParserCount = 0;
	LWEXMLNode *m_FirstNode = nullptr;
	LWEXMLNode *m_LastNode = nullptr;
};

#endif