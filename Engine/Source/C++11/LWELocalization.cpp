#include "LWELocalization.h"
#include <LWEXML.h>
#include <LWCore/LWText.h>
#include <LWCore/LWAllocator.h>
#include <LWPlatform/LWFileStream.h>
#include <iostream>

bool LWELocalization::XMLParser(LWEXMLNode *Node, void *UserData, LWEXML *X) {
	char Buffer[1024*16]; //max of 16kb.
	LWELocalization *Local = (LWELocalization*)UserData;
	LWXMLAttribute *SrcAttr = Node->FindAttribute("Source");
	LWAllocator &Alloc = Local->GetAllocator();
	LWFileStream Stream;
	auto ParseStringNode = [&Buffer](LWEXMLNode *Node, LWELocalization *Local) {
		LWXMLAttribute *NameAttr = Node->FindAttribute("Name");
		if (!NameAttr) return;
		for (LWEXMLNode *C = Node->m_FirstChild; C; C = C->m_Next) {
			uint32_t Idx = LWText::CompareMultiple(C->m_Name, 1, "enus");
			if (Idx == 0xFFFFFFFF) continue;
			LWXMLAttribute *ValueAttr = C->FindAttribute("Value");
			if (!ValueAttr) continue;
			//Parse for \n's to turn into line breaks.
			char *B = Buffer;
			char *BL = Buffer + sizeof(Buffer) - 1;
			for (char *V = ValueAttr->m_Value; *V && B!=BL; V++, B++) {
				if (*V == '\\') {
					if (*(V + 1) == 'n') {
						V++;
						*B = '\n';
					} else *B = *V;
				} else *B = *V;
			}
			if (B == BL) B--;
			*B = '\0';
			if (!Local->PushString(NameAttr->m_Value, Buffer, Idx)) {
				std::cout << "Failed to insert string: '" << NameAttr->m_Value << "' into: " << Idx << std::endl;
			}
		}
		return;
	};

	if (SrcAttr) {
		if (!LWFileStream::OpenStream(Stream, SrcAttr->m_Value, LWFileStream::ReadMode | LWFileStream::BinaryMode, Alloc)) {
			std::cout << "Failed to open: '" << SrcAttr->m_Value << "'" << std::endl;
		} else {
			Stream.ReadText(Buffer, sizeof(Buffer));
			Stream.Finished();
			LWEXML *X = Alloc.Allocate<LWEXML>();
			X->PushParser("Localization", LWELocalization::XMLParser, Local);
			X->ParseBuffer(*X, Alloc, Buffer, true);
			X->Process();
			LWAllocator::Destroy(X);
		}
	}
	for (LWEXMLNode *C = Node->m_FirstChild; C; C = C->m_Next) {
		uint32_t Idx = LWText::CompareMultiple(C->m_Name, 1, "String");
		if (Idx == 0xFFFFFFFF) continue;
		if (Idx == 0) ParseStringNode(C, Local);
	}
	return true;
}

LWELocalization &LWELocalization::SetActiveLocalization(uint32_t LocalizationID) {
	m_ActiveLocalization = LocalizationID;
	return *this;
}

char *LWELocalization::ParseLocalization(char *Buffer, uint32_t BufferSize, const char *Source) {
	char *B = Buffer;
	char *BL = B + BufferSize;
	for (const char *S = Source; *S && B!=BL; ++S) {
		if (*S == '\\' && *(S + 1) == '<') {
			S++;
			*B++ = *S;
		} else if (*S == '<') {
			const char *E = S + 1;
			for (; *E && *E != '>'; ++E) {};

			if (*E == '>') {
				uint32_t Len = (uint32_t)(uintptr_t)((E) - (S + 1));
				uint32_t Hash = LWText::MakeHashb(S + 1, Len);
				auto Iter = m_StringMap[m_ActiveLocalization].find(Hash);
				if (Iter == m_StringMap[m_ActiveLocalization].end()) {
					*B++ = *S;
				} else {
					char *l = Iter->second;
					for (; B != BL && *l;) *B++ = *l++;
					S = E;
				}
			} else *B++ = *S;
		} else *B++ = *S;
	}
	if (B == BL) B--;
	*B = '\0';
	return Buffer;
}

bool LWELocalization::PushString(const LWText &StringName, const LWText &String, uint32_t LocalizationID) {
	uint32_t Hash = StringName.GetHash();
	auto Iter = m_StringMap[LocalizationID].find(Hash);
	if (Iter != m_StringMap[LocalizationID].end()) {
		std::cout << "Localization hash collision: '" << StringName.GetCharacters() << "' with: " << Iter->second << std::endl;
		return false;
	}
	uint32_t Len = (uint32_t)strlen((const char*)String.GetCharacters()) + 1;
	char *Mem = m_Allocator.AllocateArray<char>(Len);
	memcpy(Mem, String.GetCharacters(), sizeof(char)*Len);
	auto Res = m_StringMap[LocalizationID].emplace(std::pair<uint32_t, char*>(Hash, Mem));
	if (!Res.second) {
		std::cout << "Error inserting: '" << StringName.GetCharacters() << "'" << std::endl;
		return false;
	}
	return true;
}

const char *LWELocalization::LookupString(const LWText &StringName) {
	uint32_t Hash = StringName.GetHash();
	auto Iter = m_StringMap[m_ActiveLocalization].find(Hash);
	if (Iter == m_StringMap[m_ActiveLocalization].end()) {
		std::cout << "Error String: '" << (const char*)StringName.GetCharacters() << "' not found." << std::endl;
		return nullptr;
	}
	return Iter->second;
}

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