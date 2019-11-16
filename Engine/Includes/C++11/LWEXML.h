#ifndef LWEXML_H
#define LWEXML_H
#include <LWCore/LWText.h>
#include <functional>
#include "LWETypes.h"

#define LWEXMLMAXNAMELEN 32
#define LWEXMLMAXVALUELEN 256
#define LWEXMLMAXTEXTLEN 1024

struct LWXMLAttribute {
	char m_Name[LWEXMLMAXNAMELEN];
	char m_Value[LWEXMLMAXVALUELEN];
};

struct LWEXMLNode {
	enum {
		MaxAttributes = 32
	};
	LWXMLAttribute m_Attributes[MaxAttributes];
	char m_Text[LWEXMLMAXTEXTLEN];
	char m_Name[LWEXMLMAXNAMELEN];
	uint32_t m_AttributeCount;
	LWEXMLNode *m_Parent;
	LWEXMLNode *m_Next;
	LWEXMLNode *m_FirstChild;
	LWEXMLNode *m_LastChild;

	bool PushAttribute(const char *Name, const char *Value);

	bool PushAttributef(const char *Name, const char *ValueFmt, ...);

	LWEXMLNode &SetName(const char *Name);

	LWEXMLNode &SetText(const char *Text);

	LWEXMLNode &SetTextf(const char *TextFmt, ...);

	LWXMLAttribute *FindAttribute(const LWText &Name);
};

struct LWEXMLParser {
	char m_Name[LWEXMLMAXNAMELEN];
	std::function<bool(LWEXMLNode *, void *, LWEXML *)> m_Callback;
	void *m_UserData;
};

class LWEXML {
public:
	enum {
		NodePoolSize = 256,
		MaxParsers = 32
	};
	static bool ParseBuffer(LWEXML &XML, LWAllocator &Allocator, const char *Buffer, bool StripFormatting, LWEXMLNode *Parent, LWEXMLNode *Prev);

	static bool ParseBuffer(LWEXML &XML, LWAllocator &Allocator, const char *Buffer, bool StripFormatting);

	static uint32_t ConstructBuffer(LWEXML &XML, char *Buffer, uint32_t BufferLen, bool Format);

	LWEXMLNode *NextNode(LWEXMLNode *Current, bool SkipChildren=false);

	LWEXMLNode *NextNode(LWEXMLNode *Current, LWEXMLNode *Top, bool SkipChildren = false);

	LWEXMLNode *NextNodeWithName(LWEXMLNode *Current, const LWText &Name, bool SkipChildren =false);

	template<class T, class Y>
	LWEXML &PushMethodParser(const LWText &XMLNodeName, T Method, Y Object, void *UserData) {
		return PushParser(XMLNodeName, std::bind(Method, Object, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), UserData);
	}

	LWEXML &PushParser(const LWText &XMLNodeName, std::function<bool(LWEXMLNode*, void*, LWEXML*)> Callback, void *UserData);

	LWEXML &Process(void);

	LWEXMLNode *GetInsertedNodeAfter(LWEXMLNode *Parent, LWEXMLNode *Prev, LWAllocator &Allocator);

	LWEXMLNode *GetFirstNode(void);

	LWEXMLNode *GetLastNode(void);

	LWEXML();

	~LWEXML();

private:
	LWEXMLNode **m_NodePool;
	LWEXMLParser m_Parsers[MaxParsers];
	uint32_t m_NodeCount;
	uint32_t m_ParserCount;
	LWEXMLNode *m_FirstNode;
	LWEXMLNode *m_LastNode;
};

#endif