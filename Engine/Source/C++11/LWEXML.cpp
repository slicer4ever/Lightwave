#include "LWEXML.h"
#include <LWCore/LWText.h>
#include <LWCore/LWAllocator.h>
#include <LWPlatform/LWFileStream.h>
#include <iostream>
#include <functional>
#include <cstdarg>

bool LWEXMLNode::PushAttribute(const char *Name, const char *Value) {
	if (m_AttributeCount >= MaxAttributes) return false;
	strncpy(m_Attributes[m_AttributeCount].m_Name, Name, sizeof(m_Attributes[m_AttributeCount].m_Name));
	strncpy(m_Attributes[m_AttributeCount].m_Value, Value, sizeof(m_Attributes[m_AttributeCount].m_Value));
	m_AttributeCount++;
	return true;
}

bool LWEXMLNode::PushAttributef(const char *Name, const char *ValueFmt, ...) {
	char ValueBuffer[LWEXMLMAXVALUELEN];
	va_list lst;
	va_start(lst, ValueFmt);
	vsnprintf(ValueBuffer, sizeof(ValueBuffer), ValueFmt, lst);
	va_end(lst);
	return PushAttribute(Name, ValueBuffer);
}

bool LWEXMLNode::RemoveAttribute(uint32_t i) {
	if (i >= m_AttributeCount) return false;
	std::copy(m_Attributes + i + 1, m_Attributes + m_AttributeCount, m_Attributes + i);
	m_AttributeCount--;
	return true;
}

bool LWEXMLNode::RemoveAttribute(LWXMLAttribute *Attr) {
	uint32_t i = 0;
	for (; i < m_AttributeCount; i++) if (Attr == &m_Attributes[i]) break;
	return RemoveAttribute(i);
}

LWEXMLNode &LWEXMLNode::SetName(const char *Name) {
	strncpy(m_Name, Name, sizeof(m_Name));
	return *this;
}

LWEXMLNode &LWEXMLNode::SetText(const char *Text) {
	strncpy(m_Text, Text, sizeof(m_Text));
	return *this;
}

LWEXMLNode &LWEXMLNode::SetTextf(const char *TextFmt, ...) {
	char TextBuffer[LWEXMLMAXTEXTLEN];
	va_list lst;
	va_start(lst, TextFmt);
	vsnprintf(TextBuffer, sizeof(TextBuffer), TextFmt, lst);
	va_end(lst);
	return SetText(TextBuffer);
}

LWXMLAttribute *LWEXMLNode::FindAttribute(const LWText &Name) {
	uint32_t NameHash = Name.GetHash();
	for (uint32_t i = 0; i < m_AttributeCount; i++) {
		LWXMLAttribute *A = m_Attributes + i;
		if (NameHash == LWText::MakeHash(A->m_Name)) return A;
	}
	return nullptr;
}

bool LWEXML::LoadFile(LWEXML &XML, LWAllocator &Allocator, const LWText &Path, bool StripFormatting, LWEXMLNode *Parent, LWEXMLNode *Prev, LWFileStream *ExistingStream) {
	LWFileStream Stream;
	if (!LWFileStream::OpenStream(Stream, Path, LWFileStream::BinaryMode | LWFileStream::ReadMode, Allocator, ExistingStream)) return false;
	uint32_t Len = Stream.Length() + 1;
	char *B = Allocator.AllocateArray<char>(Len);
	Stream.ReadText(B, Len);
	bool Res = ParseBuffer(XML, Allocator, B, StripFormatting, Parent, Prev);
	LWAllocator::Destroy(B);
	return Res;
}

bool LWEXML::LoadFile(LWEXML &XML, LWAllocator &Allocator, const LWText &Path, bool StripFormatting, LWFileStream *ExistingStream) {
	LWFileStream Stream;
	if (!LWFileStream::OpenStream(Stream, Path, LWFileStream::BinaryMode | LWFileStream::ReadMode, Allocator, ExistingStream)) return false;
	uint32_t Len = Stream.Length() + 1;
	char *B = Allocator.AllocateArray<char>(Len);
	Stream.ReadText(B, Len);
	bool Res = ParseBuffer(XML, Allocator, B, StripFormatting);
	LWAllocator::Destroy(B);
	return Res;

}

bool LWEXML::ParseBuffer(LWEXML &XML, LWAllocator &Allocator, const char *Buffer, bool StripFormatting, LWEXMLNode *Parent, LWEXMLNode *Prev) {
	char Buf[1024];
	const char *P = Buffer;
	bool RawData = false;
	uint32_t QuoteCounter = 0;
	LWEXMLNode *ActiveNode = Parent;
	LWEXMLNode *ChildNode = Prev;
	auto CopyTextToNode = [](LWEXMLNode *Target, char *Buffer, uint32_t BufferLen, const char *P, const char *E, bool StripFormatting) {
		if (!Target) return;
		if (P == E) return;
		if (StripFormatting) {
			for (; P != E; P++) {
				if (*P != ' ' && *P != '\t' && *P!='\n') break;
			}
			const char *NE = E;
			for (const char *NP = P; NP != NE; NP++) {
				if (*NP != ' ' || *NP != '\t' || *NP != '\n') E = NP+1;
			}
		}
		uint32_t PLen = (uint32_t)(uintptr_t)(E - P);
		uint32_t Len = (uint32_t)strlen(Buffer);
		if (Len + PLen >= BufferLen) PLen = BufferLen - Len - 1;
		memcpy(Buffer + Len, P, PLen);
		Buffer[Len + PLen] = '\0';
		return;
	};

	auto CalculateLine = [](const char *Buffer, const char *Pos)->uint32_t {
		uint32_t Line = 1;
		for (const char *c = Buffer; c != Pos; c++) if (*c == '\n') Line++;
		return Line;
	};

	//Comments look like: <!-- comment here --> 
	//CData looks like: <![CDATA[data here]]> 
	for (const char *C = P; *C; C++) {
		if (*C == '<') {
			const char *F = C;
			//Process open text from P to C first.
			CopyTextToNode(ActiveNode, ActiveNode->m_Text, sizeof(ActiveNode->m_Text), P, C, StripFormatting);
			C = LWText::NextWord(C + 1, true);
			if (LWText::Compare(C, "!--", 3)) { //Node is start of comment.
				//Skip to end of comment!
				for (C += 3; *C; C++) {
					if (LWText::Compare(C, "-->", 3)) {
						C += 2;
						P = C+1;
						break;
					}
				}
				continue;
			}

			if (LWText::Compare(C, "![CDATA[", 8)) { //Node is start of CDATA.
				C += 8;
				P = C;
				for (; *C; C++) {
					if (LWText::Compare(C, "]]>", 3)) {
						CopyTextToNode(ActiveNode, ActiveNode->m_Text, sizeof(ActiveNode->m_Text), P, C, StripFormatting);
						C += 3;
						P = C;
					}
				}
				continue;
			}
			//Check if this is a termination node for our active node!
			if (LWText::Compare(C, "/", 1)) {
				C += 1;
				//Check that the name matches our active node!
				if (!ActiveNode) {
					std::cout << "Line " << CalculateLine(Buffer, F) << ": Error found termination node with no active node." << std::endl;
					return false;
				} else {
					uint32_t Len = (uint32_t)strlen(ActiveNode->m_Name);
					if (!LWText::Compare(C, ActiveNode->m_Name, Len)) {
						LWText::CopyToTokens(C, Buf, (uint32_t)sizeof(Buf), "> ");
						std::cout << "Line " << CalculateLine(Buffer, F) << ": Error found incorrect termination name, active: '" << ActiveNode->m_Name << "' Discovered: '" << Buf << "'" << std::endl;
						return false;
					}
					C = LWText::FirstToken(C + Len, '>');
					P = C + 1;
					ChildNode = ActiveNode;
					ActiveNode = ActiveNode->m_Parent;
					continue;
				}
			}
			//Now process this node!
			P = C;
			QuoteCounter = 0;
			ActiveNode = XML.GetInsertedNodeAfter(ActiveNode, ChildNode, Allocator);
			ChildNode = nullptr;
			if (!ActiveNode) {
				std::cout << "Error exceeded number of XML nodes supported by this implementation." << std::endl;
				return false;
			} 
			C = LWText::CopyToTokens(LWText::NextWord(C, true), ActiveNode->m_Name, sizeof(ActiveNode->m_Name), "> ");
			for (;; C++) {
				C = LWText::NextWord(C, true);
				if (*C == '/') {
					ChildNode = ActiveNode;
					ActiveNode = ActiveNode->m_Parent;
					continue;
				}
				if (*C == '>') break;
				LWXMLAttribute *Attr = &ActiveNode->m_Attributes[ActiveNode->m_AttributeCount];
				Attr->m_Value[0] = '\0';
				C = LWText::CopyToTokens(C, Attr->m_Name, (uint32_t)sizeof(Attr->m_Name), " =");
				C = LWText::NextWord(C, true);
				if (*C != '=') {
					C--;
					ActiveNode->m_AttributeCount++;
					continue;
				}
				C = LWText::NextWord(C+1, true);
				if (*C == '\"') C = LWText::CopyToTokens(C + 1, Attr->m_Value, (uint32_t)sizeof(Attr->m_Value), "\"");
				else {
					std::cout << "Line " << CalculateLine(Buffer, F) << ": Error invalid token found: '" << *C << "' line: " << std::endl;
					return false;
				}
				ActiveNode->m_AttributeCount++;
			}
			P = C+1;
		}
	}
	return true;
}

bool LWEXML::ParseBuffer(LWEXML &XML, LWAllocator &Allocator, const char *Buffer, bool StripFormatting) {
	LWEXMLNode *Parent = nullptr;
	LWEXMLNode *Prev = XML.GetLastNode();
	return ParseBuffer(XML, Allocator, Buffer, StripFormatting, Parent, Prev);
}

uint32_t LWEXML::ConstructBuffer(LWEXML &XML, char *Buffer, uint32_t BufferLen, bool Format) {
	
	auto TabTo = [](uint32_t Depth, char *Buffer, uint32_t BufferLen, bool Format)->uint32_t {
		uint32_t o = 0;
		if (!Format) return o;
		for (uint32_t d = 0; d < Depth; d++) o += snprintf(Buffer + o, BufferLen - o, " ");
		return o;
	};
	
	std::function<uint32_t(LWEXMLNode*, uint32_t, char *, uint32_t, bool, bool)> RecursiveOutput;
	RecursiveOutput = [&RecursiveOutput, &TabTo](LWEXMLNode *Current, uint32_t Depth, char *Buffer, uint32_t BufferLen, bool First, bool Format)->uint32_t {
		for (uint32_t i = 0; i < Depth; i++) std::cout << "-";
		std::cout << Current->m_Name << " Text: '" << Current->m_Text << "'" << std::endl;
		uint32_t o = 0;
		if (!First && Format) o += snprintf(Buffer + o, BufferLen - o, "\n");
		o += TabTo(Depth, Buffer + o, BufferLen - o, Format);
		o += snprintf(Buffer + o, BufferLen - o, "<%s", Current->m_Name);
		for (uint32_t i = 0; i < Current->m_AttributeCount; i++) {
			LWXMLAttribute *A = Current->m_Attributes + i;
			o += snprintf(Buffer + o, BufferLen - o, " %s", A->m_Name);
			if (*A->m_Value) o += snprintf(Buffer + o, BufferLen - o, "=\"%s\"", A->m_Value);
		}
		if (!Current->m_FirstChild && !*Current->m_Text) {
			o += snprintf(Buffer + o, BufferLen - o, " />");
			return o;
		}
		o += snprintf(Buffer + o, BufferLen - o, " >");
		if (*Current->m_Text) {
			if(Format) o += snprintf(Buffer + o, BufferLen - o, "\n");
			o += TabTo(Depth + 1, Buffer + o, BufferLen - o, Format);
			o += snprintf(Buffer + o, BufferLen - o, "%s", Current->m_Text);
		}
		for (LWEXMLNode *C = Current->m_FirstChild; C; C = C->m_Next) {
			o += RecursiveOutput(C, Depth + 1, Buffer + o, BufferLen - o, false, Format);
		}
		if (Format) o += snprintf(Buffer + o, BufferLen - o, "\n");
		o += TabTo(Depth, Buffer + o, BufferLen - o, Format);
		o += snprintf(Buffer + o, BufferLen - o, "</%s>", Current->m_Name);

		return o;
	};

	uint32_t o = 0;
	Buffer[0] = '\0';
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

LWEXMLNode *LWEXML::NextNodeWithName(LWEXMLNode *Current, const LWText &Name, bool SkipChildren) {
	for (LWEXMLNode *N = NextNode(Current, SkipChildren); N; N = NextNode(N, SkipChildren)) {
		if (LWText::Compare((char*)Name.GetCharacters(), N->m_Name)) return N;
	}
	return nullptr;
}

LWEXML &LWEXML::PushParser(const LWText &XMLNodeName, std::function<bool(LWEXMLNode*, void*, LWEXML*)> Callback, void *UserData) {
	if (m_ParserCount >= MaxParsers) return *this;
	snprintf(m_Parsers[m_ParserCount].m_Name, sizeof(m_Parsers[m_ParserCount].m_Name), "%s", (char*)XMLNodeName.GetCharacters());
	m_Parsers[m_ParserCount].m_Callback = Callback;
	m_Parsers[m_ParserCount++].m_UserData = UserData;
	return *this;
}

LWEXML &LWEXML::Process(void) {
	for (LWEXMLNode *C = NextNode(nullptr); C;) {
		bool Processed = false;
		for (uint32_t i = 0; i < m_ParserCount; i++) {
			if (LWText::Compare(C->m_Name, m_Parsers[i].m_Name)) {
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
		LWEXMLNode *NodePool = Allocator.AllocateArray<LWEXMLNode>(NodePoolSize);
		LWEXMLNode **NewPool = Allocator.AllocateArray<LWEXMLNode*>(TargetPool + 1);
		memcpy(NewPool, m_NodePool, sizeof(LWEXMLNode*)*(TargetPool));
		NewPool[TargetPool] = NodePool;
		LWEXMLNode **OldPool = m_NodePool;
		m_NodePool = NewPool;
		LWAllocator::Destroy(OldPool);
	}
	LWEXMLNode *NextNode = m_NodePool[TargetPool] + TargetIdx;
	m_NodeCount++;
	NextNode->m_Parent = nullptr;
	NextNode->m_FirstChild = nullptr;
	NextNode->m_LastChild = nullptr;
	NextNode->m_Text[0] = '\0';
	NextNode->m_Name[0] = '\0';
	NextNode->m_AttributeCount = 0;
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

LWEXML::LWEXML() : m_NodePool(nullptr), m_NodeCount(0), m_ParserCount(0), m_FirstNode(nullptr), m_LastNode(nullptr) {}

LWEXML::~LWEXML() {
	uint32_t PoolCount = (m_NodeCount+NodePoolSize-1) / NodePoolSize;
	for (uint32_t i = 0; i < PoolCount; i++) LWAllocator::Destroy(m_NodePool[i]);
	LWAllocator::Destroy(m_NodePool);
}