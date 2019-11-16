#ifndef LWELOCALIZATION_H
#define LWELOCALIZATION_H
#include <LWCore/LWTypes.h>
#include <LWCore/LWText.h>
#include <unordered_map>
#include "LWETypes.h"

class LWELocalization {
public:
	enum {
		en_us=0,
		Count
	};

	static bool XMLParser(LWEXMLNode *Node, void *UserData, LWEXML *X);

	//Localization replaces [xyz] with the localized string if it exists.
	char *ParseLocalization(char *Buffer, uint32_t BufferSize, const char *Source);

	LWELocalization &SetActiveLocalization(uint32_t LocalizationID);

	bool PushString(const LWText &StringName, const LWText &String, uint32_t LocalizationID);

	const char *LookupString(const LWText &StringName);

	LWAllocator &GetAllocator(void);

	uint32_t GetActiveLocalization(void);

	LWELocalization(LWAllocator &Allocator);

	~LWELocalization();
private:
	LWAllocator &m_Allocator;
	std::unordered_map<uint32_t, char*> m_StringMap[LWELocalization::Count];
	uint32_t m_ActiveLocalization;
};


#endif
