#include "LWEXML.h"
#include <LWCore/LWAllocator.h>
#include <LWPlatform/LWFileStream.h>
#include <LWCore/LWLogger.h>
#include <iostream>
#include <functional>
#include <cstdarg>

//LWEXMLAttribute:

LWEXMLAttribute &LWEXMLAttribute::SetName(const LWUTF8Iterator &Name) {
	Name.Copy(m_Name, sizeof(m_Name));
	m_NameHash = LWUTF8Iterator(m_Name).Hash();
	return *this;
}

LWEXMLAttribute &LWEXMLAttribute::SetValue(const LWUTF8Iterator &Value) {
	Value.Copy(m_Value, sizeof(m_Value));
	return *this;
}

LWUTF8Iterator LWEXMLAttribute::GetName(void) const {
	return LWUTF8Iterator(m_Name);
}

LWUTF8Iterator LWEXMLAttribute::GetValue(void) const {
	return LWUTF8Iterator(m_Value);
}

LWEXMLAttribute::LWEXMLAttribute(const LWUTF8Iterator &Name, const LWUTF8Iterator &Value) {
	SetName(Name);
	SetValue(Value);
}

//LWEXMLNode:
bool LWEXMLNode::PushAttribute(const LWEXMLAttribute &Attr) {
	if (m_AttributeCount >= MaxAttributes) return false;
	m_Attributes[m_AttributeCount++] = Attr;
	return true;
}

bool LWEXMLNode::RemoveAttribute(uint32_t i) {
	if (i >= m_AttributeCount) return false;
	std::copy(m_Attributes + i + 1, m_Attributes + m_AttributeCount, m_Attributes + i);
	m_AttributeCount--;
	return true;
}

bool LWEXMLNode::RemoveAttribute(LWEXMLAttribute *Attr) {
	uint32_t i = (uint32_t)((Attr - m_Attributes) / sizeof(LWEXMLAttribute));
	return RemoveAttribute(i);
}

LWUTF8Iterator LWEXMLNode::GetName(void) const {
	return { m_Name };
}

LWUTF8Iterator LWEXMLNode::GetText(void) const {
	return { m_Text };
}

LWEXMLNode &LWEXMLNode::SetName(const LWUTF8Iterator &Name) {
	Name.Copy(m_Name, sizeof(m_Name));
	m_NameHash = Name.Hash();
	return *this;
}

LWEXMLNode &LWEXMLNode::SetText(const LWUTF8Iterator &Text) {
	Text.Copy(m_Text, sizeof(m_Text));
	return *this;
}

LWEXMLAttribute *LWEXMLNode::FindAttribute(const LWUTF8Iterator &Name) {
	return FindAttribute(Name.Hash());
}

LWEXMLAttribute *LWEXMLNode::FindAttribute(uint32_t NameHash) {
	for (uint32_t i = 0; i < m_AttributeCount; i++) {
		if (NameHash == m_Attributes[i].m_NameHash) return &m_Attributes[i];
	}
	return nullptr;
}

LWEXMLNode::LWEXMLNode(const LWUTF8Iterator &Name) {
	SetName(Name);
}

//LWEXMLParser
LWEXMLParser::LWEXMLParser(const LWUTF8Iterator &Name, LWEXMLParseCallback Callback, void *UserData) : m_Callback(Callback), m_UserData(UserData), m_NameHash(Name.Hash()) {
	Name.Copy(m_Name, sizeof(m_Name));
}

//LWEXML
bool LWEXML::LoadFile(LWEXML &XML, LWAllocator &Allocator, const LWUTF8Iterator &Path, bool StripFormatting, LWEXMLNode *Parent, LWEXMLNode *Prev, LWFileStream *ExistingStream) {
	LWFileStream Stream;
	if (!LWFileStream::OpenStream(Stream, Path, LWFileStream::BinaryMode | LWFileStream::ReadMode, Allocator, ExistingStream)) return false;
	uint32_t Len = Stream.Length() + 1;
	char8_t *B = Allocator.Allocate<char8_t>(Len);
	if (Stream.ReadText(B, Len) != Len) {
		LWAllocator::Destroy(B);
		return false;
	}
	bool Res = ParseBuffer(XML, Allocator, B, StripFormatting, Parent, Prev);
	LWAllocator::Destroy(B);
	return Res;
}

bool LWEXML::LoadFile(LWEXML &XML, LWAllocator &Allocator, const LWUTF8Iterator &Path, bool StripFormatting, LWFileStream *ExistingStream) {
	LWFileStream Stream;
	if (!LWFileStream::OpenStream(Stream, Path, LWFileStream::BinaryMode | LWFileStream::ReadMode, Allocator, ExistingStream)) return false;
	uint32_t Len = Stream.Length() + 1;
	char8_t *B = Allocator.Allocate<char8_t>(Len);
	if (Stream.ReadText(B, Len) != Len) {
		LWAllocator::Destroy(B);
		return false;
	}
	bool Res = ParseBuffer(XML, Allocator, B, StripFormatting);
	LWAllocator::Destroy(B);
	return Res;

}

bool LWEXML::ParseBuffer(LWEXML &XML, LWAllocator &Allocator, const LWUTF8Iterator &Source, bool StripFormatting, LWEXMLNode *Parent, LWEXMLNode *Prev) {
	//bool RawData = false;
	uint32_t QuoteCounter = 0;
	LWEXMLNode *ActiveNode = Parent;
	LWEXMLNode *ChildNode = Prev;
	auto CopyAndStrip = [](LWEXMLNode *Target, const LWUTF8Iterator &First, const LWUTF8Iterator &End, bool StripFormatting) {
		if (!Target) return;
		LWUTF8Iterator F = First;
		LWUTF8Iterator E = End;
		if (StripFormatting) {
			F.AdvanceWord(true);
			(End - 1).rAdvanceWord(true);
		}
		Target->SetText(LWUTF8Iterator(F, E));
		return;
	};

	//Comments look like: <!-- comment here --> 
	//CData looks like: <![CDATA[data here]]> 
	LWUTF8Iterator P = Source;
	for(LWUTF8Iterator C = P; !C.AtEnd(); ++C) {
		if(*C=='<') {
			CopyAndStrip(ActiveNode, P, C, StripFormatting);
			LWUTF8Iterator F = C;
			C = (C + 1).AdvanceWord(true);
			if (C.Compare("!--", 3)) { //Node is start of comment:
				C.AdvanceSubString("-->"); //Skip to end of comment.
				C += 2;
				P = C + 1;
				continue;
			} else if (C.Compare("![CDATA[", 8)) { //Node is start of CDATA:
				C += 8;
				P = C;
				C.AdvanceSubString("]]>", false);
				CopyAndStrip(ActiveNode, P, C, StripFormatting);
				C += 2;
				P = C + 1;
				continue;
			}else if(*C=='/') { //is termination node for our active node:
				++C;
				if(!LWLogCriticalIf<256>(ActiveNode, "Line {}: found termination node with no active node.", LWUTF8I::CountLines(LWUTF8I(Source, F)))) return false;
				if(!LWLogCriticalIf<256>(C.isSubString(ActiveNode->GetName()), "Line {}: Error found incorrect termination name, active: '{}' Discovered: '{}'", LWUTF8I::CountLines(LWUTF8I(Source, F)), ActiveNode->GetName(), LWUTF8I(C, C.NextTokens(u8"> ")))) return false;

				C.AdvanceToken('>');
				P = C + 1;
				ChildNode = ActiveNode;
				ActiveNode = ActiveNode->m_Parent;
				continue;
			}
			//Now process this node!
			P = C;
			QuoteCounter = 0;
			ActiveNode = XML.GetInsertedNodeAfter(ActiveNode, ChildNode, Allocator);
			ChildNode = nullptr;
			if(!LWLogCriticalIf(ActiveNode, "exceeded number of XML nodes supported by this implementation.")) return false;

			LWUTF8Iterator E = C.NextTokens("> ");
			if(!LWLogCriticalIf<256>(!E.AtEnd(), "Line {}: Error could not find closing > token.", LWUTF8I::CountLines(LWUTF8I(Source, F)))) return false;

			ActiveNode->SetName(LWUTF8Iterator(C, E));
			for(C = E.NextWord(true);;C.AdvanceWord(true)) {
				if (*C == '/') {
					if(!LWLogCriticalIf<256>(ActiveNode, "Line {}: Error encountered second / before > token.", LWUTF8I::CountLines(LWUTF8I(Source, F)))) return false;

					ChildNode = ActiveNode;
					ActiveNode = ActiveNode->m_Parent;
					++C;
					continue;
				} else if (*C == '>') break;
				LWEXMLAttribute Attr;
				E = C.NextTokens(" =");
				Attr.SetName(LWUTF8Iterator(C, E));
				C = E.NextWord(true);
				if (*C == '=') {
					++C;
					C.AdvanceWord(true);
					if(!LWLogCriticalIf<256>(*C=='\"', "Line {}: Error Expected \" token, instead found: '{:c}'.", LWUTF8I::CountLines(LWUTF8I(Source, F)), *C)) return false;

					++C;
					E = C.NextToken('\"', false);
					if(!LWLogCriticalIf<256>(!E.AtEnd(), "Line {}: Error did not find matching \" token.", LWUTF8I::CountLines(LWUTF8I(Source, F)))) return false;

					Attr.SetValue(LWUTF8Iterator(C, E));
					C = E + 1;
				}
				LWLogCriticalIf<256>(ActiveNode->PushAttribute(Attr), "Line {}: Node '{}' has exceeded the amount of attributes this implementation supports.", LWUTF8I::CountLines(LWUTF8I(Source, F)), ActiveNode->GetName());
			}
			P = C+1;
		}
	}
	return true;
}

bool LWEXML::ParseBuffer(LWEXML &XML, LWAllocator &Allocator, const LWUTF8Iterator &Buffer, bool StripFormatting) {
	LWEXMLNode *Parent = nullptr;
	LWEXMLNode *Prev = XML.GetLastNode();
	return ParseBuffer(XML, Allocator, Buffer, StripFormatting, Parent, Prev);
}

uint32_t LWEXML::ConstructBuffer(LWEXML &XML, char8_t *Buffer, uint32_t BufferLen, bool Format) {
	
	auto TabTo = [](uint32_t Depth, char8_t *Buffer, uint32_t BufferLen, bool Format)->uint32_t {
		uint32_t o = 0;
		if (!Format) return o;
		o += LWUTF8I::Fmt_ns(Buffer, BufferLen, o, "{: >{}}", "", Depth);
		return o;
	};
	
	std::function<uint32_t(LWEXMLNode*, uint32_t, char8_t *, uint32_t, bool, bool)> RecursiveOutput;
	RecursiveOutput = [&RecursiveOutput, &TabTo](LWEXMLNode *Current, uint32_t Depth, char *Buffer, uint32_t BufferLen, bool First, bool Format)->uint32_t {
		uint32_t o = 0;
		if (!First && Format) o += LWUTF8I::Fmt_ns(Buffer, BufferLen, o, "\n");
		o += TabTo(Depth, Buffer + o, BufferLen - o, Format);
		o += LWUTF8I::Fmt_ns(Buffer, BufferLen, o, "<{}", Current->m_Name);
		for (uint32_t i = 0; i < Current->m_AttributeCount; i++) {
			LWEXMLAttribute &A = Current->m_Attributes[i];
			o += LWUTF8I::Fmt_ns(Buffer, BufferLen, o, " {}", A.m_Name);
			if (!A.GetValue().AtEnd()) o += LWUTF8I::Fmt_ns(Buffer, BufferLen, o, "=\"{}\"", A.m_Value);
		}
		if (!Current->m_FirstChild && !*Current->m_Text) {
			o += LWUTF8I::Fmt_ns(Buffer, BufferLen, o, " />");
			return o;
		}
		o += LWUTF8I::Fmt_ns(Buffer, BufferLen, o, " >");
		if (*Current->m_Text) {
			if(Format) o += LWUTF8I::Fmt_ns(Buffer, BufferLen, o, "\n");
			o += TabTo(Depth + 1, Buffer + o, BufferLen - o, Format);
			o += LWUTF8I::Fmt_ns(Buffer, BufferLen, o, "{}", Current->m_Text);
		}
		for (LWEXMLNode *C = Current->m_FirstChild; C; C = C->m_Next) {
			o += RecursiveOutput(C, Depth + 1, Buffer + o, BufferLen - o, false, Format);
		}
		if (Format) o += LWUTF8I::Fmt_ns(Buffer, BufferLen, o, "\n");
		o += TabTo(Depth, Buffer + o, BufferLen - o, Format);
		o += LWUTF8I::Fmt_ns(Buffer, BufferLen, o, "</{}>", Current->m_Name);

		return o;
	};

	uint32_t o = 0;
	if(Buffer) *Buffer = '\0';
	for (LWEXMLNode *C = XML.GetFirstNode(); C; C=C->m_Next){
		o += RecursiveOutput(C, 0, Buffer + o, BufferLen - o, C == XML.GetFirstNode(), Format);
	}
	return o;
}

LWEXMLNode *LWEXML::NextNode(LWEXMLNode *Current, bool SkipChildren) {
	if (!Current) return m_FirstNode;
	if (Current->m_FirstChild && !SkipChildren) return Current->m_FirstChild;
	if (!Current->m_Next) {
		if (Current->m_Parent) return NextNode(Current->m_Parent, true);
	}
	return Current->m_Next;
}

LWEXMLNode *LWEXML::NextNode(LWEXMLNode *Current, LWEXMLNode *Top, bool SkipChildren) {
	if (!Current) return Top?Top->m_FirstChild:m_FirstNode;
	if (Current->m_FirstChild && !SkipChildren) return Current->m_FirstChild;
	if (!Current->m_Next) {
		if (Current->m_Parent && Current->m_Parent!=Top) return NextNode(Current->m_Parent, true);
	}
	return Current->m_Next;
}

LWEXMLNode *LWEXML::NextNodeWithName(LWEXMLNode *Current, const LWUTF8Iterator &Name, bool SkipChildren) {
	uint32_t Hash = Name.Hash();
	for (LWEXMLNode *N = NextNode(Current, SkipChildren); N; N = NextNode(N, SkipChildren)) {
		if (N->m_NameHash == Hash) return N;
	}
	return nullptr;
}

LWEXML &LWEXML::PushParser(const LWUTF8Iterator &XMLNodeName, std::function<bool(LWEXMLNode*, void*, LWEXML*)> Callback, void *UserData) {
	if (m_ParserCount >= MaxParsers) return *this;
	m_Parsers[m_ParserCount++] = LWEXMLParser(XMLNodeName, Callback, UserData);
	return *this;
}

LWEXML &LWEXML::Process(void) {
	for (LWEXMLNode *C = NextNode(nullptr); C;) {
		bool Processed = false;
		for (uint32_t i = 0; i < m_ParserCount; i++) {
			if(C->GetName().Hash()==m_Parsers[i].m_NameHash) {
				if (m_Parsers[i].m_Callback(C, m_Parsers[i].m_UserData, this)) Processed = true;
			}
		}
		C = NextNode(C, Processed);
	}
	return *this;
}

LWEXMLNode *LWEXML::GetInsertedNodeAfter(LWEXMLNode *Parent, LWEXMLNode *Prev, LWAllocator &Allocator) {
	uint32_t TargetPool = m_NodeCount / NodePoolSize;
	uint32_t TargetIdx = m_NodeCount%NodePoolSize;
	if (!TargetIdx) {
		LWEXMLNode *NodePool = Allocator.Allocate<LWEXMLNode>(NodePoolSize);
		LWEXMLNode **NewPool = Allocator.Allocate<LWEXMLNode*>(TargetPool + 1);
		memcpy(NewPool, m_NodePool, sizeof(LWEXMLNode*)*(TargetPool));
		NewPool[TargetPool] = NodePool;
		LWEXMLNode **OldPool = m_NodePool;
		m_NodePool = NewPool;
		LWAllocator::Destroy(OldPool);
	}
	LWEXMLNode *NextNode = m_NodePool[TargetPool] + TargetIdx;
	m_NodeCount++;
	if (!Prev) {
		if (!Parent) {
			NextNode->m_Next = m_FirstNode;
			m_FirstNode = NextNode;
			if (!m_LastNode) m_LastNode = NextNode;
		} else {
			NextNode->m_Parent = Parent;
			NextNode->m_Next = Parent->m_FirstChild;
			Parent->m_FirstChild = NextNode;
			if (!Parent->m_LastChild) Parent->m_LastChild = NextNode;
		}
	} else {
		NextNode->m_Parent = Prev->m_Parent;
		NextNode->m_Next = Prev->m_Next;
		Prev->m_Next = NextNode;
		if (Prev->m_Parent) {
			if (Parent->m_LastChild == Prev) Parent->m_LastChild = NextNode;
		} else {
			if (m_LastNode == Prev) m_LastNode = NextNode;
		}
	}
	return NextNode;
}

LWEXMLNode *LWEXML::GetFirstNode(void) {
	return m_FirstNode;
}

LWEXMLNode *LWEXML::GetLastNode(void) {
	return m_LastNode;
}

LWEXML::~LWEXML() {
	uint32_t PoolCount = (m_NodeCount+NodePoolSize-1) / NodePoolSize;
	for (uint32_t i = 0; i < PoolCount; i++) LWAllocator::Destroy(m_NodePool[i]);
	LWAllocator::Destroy(m_NodePool);
}