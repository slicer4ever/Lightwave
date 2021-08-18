#include "LWELocalization.h"
#include <LWEXML.h>
#include <LWCore/LWUnicode.h>
#include <LWEJson.h>
#include <LWCore/LWAllocator.h>
#include <LWPlatform/LWFileStream.h>
#include <iostream>
#include "LWELogger.h"

bool LWELocalization::XMLParser(LWEXMLNode *Node, void *UserData, LWEXML *X) {
	char8_t Buffer[1024*16]; //max of 16kb.
	LWELocalization *Local = (LWELocalization*)UserData;
	LWEXMLAttribute *SrcAttr = Node->FindAttribute("Source");
	LWAllocator &Alloc = Local->GetAllocator();
	LWFileStream Stream;
	auto ParseStringNode = [&Buffer](LWEXMLNode *Node, LWELocalization *Local) {
		LWEXMLAttribute *NameAttr = Node->FindAttribute("Name");
		if (!NameAttr) return;
		for (LWEXMLNode *C = Node->m_FirstChild; C; C = C->m_Next) {
			uint32_t Idx = C->GetName().CompareList("enus");
			if(Idx==-1) continue;
			LWEXMLAttribute *ValueAttr = C->FindAttribute("Value");
			if (!ValueAttr) continue;
			//Parse Unescape string as if it were a json string.
			LWEJson::UnEscapeString(ValueAttr->GetValue(), Buffer, sizeof(Buffer));
			if (!Local->PushString(NameAttr->GetValue(), Buffer, Idx)) {
				LWELogCritical<256>("Failed to insert string: '{}' into: {}", NameAttr->GetValue(), Idx);
			}
		}
		return;
	};

	if (SrcAttr) {
		if (!LWFileStream::OpenStream(Stream, SrcAttr->m_Value, LWFileStream::ReadMode | LWFileStream::BinaryMode, Alloc)) {
			LWELogCritical<256>("Failed to open: '{}'", SrcAttr->GetValue());
		} else {
			Stream.ReadText(Buffer, sizeof(Buffer));
			Stream.Finished();
			LWEXML X;
			X.PushParser("Localization", LWELocalization::XMLParser, Local);
			if (!X.ParseBuffer(X, Alloc, Buffer, true)) {
				LWELogCritical<256>("Failed to parse: '{}'", SrcAttr->GetValue());
			} else X.Process();
		}
	}
	for (LWEXMLNode *C = Node->m_FirstChild; C; C = C->m_Next) {
		uint32_t i = C->GetName().CompareList("String");
		if (i == 0) ParseStringNode(C, Local);
		else LWELogCritical<256>("Unknown node: {}", C->GetName());
	}
	return true;
}

LWELocalization &LWELocalization::SetActiveLocalization(uint32_t LocalizationID) {
	m_ActiveLocalization = LocalizationID;
	return *this;
}

uint32_t LWELocalization::ParseLocalization(char8_t *Buffer, uint32_t BufferSize, const LWUTF8Iterator &Source) {
	char8_t *B = Buffer;
	char8_t *BL = B + std::min<uint32_t>(BufferSize - 1, BufferSize);
	uint32_t o = 0;
	for (LWUTF8Iterator C = Source; !C.AtEnd(); ++C) {
		if (*C == '\\') {
			//Escape string.
			if (*(C + 1) == '<') {
				++C;
				if (B != BL) *B++ = '<';
				o++;
				continue;
			}
		} else if (*C == '<') {
			LWUTF8Iterator T = C.NextToken('>');
			if (*T == '>') {
				LWUTF8Iterator S = Find(LWUTF8Iterator(C + 1, T));
				if (!S.isInitialized()) {
					if (B != BL) *B++ = '<';
					o++;
					continue;
				} else {
					uint32_t r = S.Copy(B, (uint32_t)(uintptr_t)(BL - B)) - 1;
					B = std::min<char8_t*>(B + r, BL);
					o += r;
					C = T;
					continue;
				}
			}
		}
		uint32_t r = LWUTF8Iterator::EncodeCodePoint(B, (uint32_t)(uintptr_t)(BL - B), *C);
		B = std::min<char8_t*>(B + r, BL);
		o += r;
	}
	if (BufferSize) *B = '\0';
	return o + 1;
}

bool LWELocalization::PushString(const LWUTF8Iterator &StringName, const LWUTF8Iterator &String, uint32_t LocalizationID) {
	uint32_t Hash = StringName.Hash();
	auto Iter = m_StringMap[LocalizationID].find(Hash);
	if (Iter != m_StringMap[LocalizationID].end()) {
		LWELogCritical<256>("Localization hash collision: '{}'", StringName);
		return false;
	}
	uint32_t Len = String.RawDistance(String.NextEnd());
	char8_t *Mem = m_Allocator.Allocate<char8_t>(Len);
	if (String.Copy(Mem, Len) != Len) {
		LWELogCritical<256>("copying string.");
		return false;
	}
	auto Res = m_StringMap[LocalizationID].emplace(Hash, Mem);
	if (!Res.second) {
		LWELogCritical<256>("inserting: '{}'", StringName);
		LWAllocator::Destroy(Mem);
	}
	return Res.second;
}

LWUTF8Iterator LWELocalization::Find(const LWUTF8Iterator &StringName) {
	uint32_t Hash = StringName.Hash();
	auto Iter = m_StringMap[m_ActiveLocalization].find(Hash);
	if (Iter == m_StringMap[m_ActiveLocalization].end()) {
		LWELogCritical<256>("string '{}' not found.", StringName);
		return LWUTF8Iterator();
	}
	return LWUTF8Iterator(Iter->second);
}

LWUTF8Iterator LWELocalization::Find(uint32_t StringNameHash) {
	auto Iter = m_StringMap[m_ActiveLocalization].find(StringNameHash);
	if (Iter == m_StringMap[m_ActiveLocalization].end()) {
		LWELogCritical<256>("string hash '{:#x}' not found.", StringNameHash);
		return LWUTF8Iterator();
	}
	return LWUTF8Iterator(Iter->second);
};

LWAllocator &LWELocalization::GetAllocator(void) {
	return m_Allocator;
}

uint32_t LWELocalization::GetActiveLocalization(void) {
	return m_ActiveLocalization;
}

LWELocalization::LWELocalization(LWAllocator &Allocator) : m_Allocator(Allocator), m_ActiveLocalization(en_us) {}

LWELocalization::~LWELocalization() {
	for (uint32_t i = 0; i < LWELocalization::Count; i++) {
		for (auto &&Iter : m_StringMap[i]) {
			LWAllocator::Destroy(Iter.second);
		}
	}
}