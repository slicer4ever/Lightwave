#ifndef LWELOCALIZATION_H
#define LWELOCALIZATION_H
#include <LWCore/LWTypes.h>
#include <unordered_map>
#include "LWETypes.h"

class LWELocalization {
public:
	enum {
		en_us=0,
		Count
	};

	static bool XMLParser(LWEXMLNode *Node, void *UserData, LWEXML *X);

	/*! \brief Localization replaces <xyz> with the localized string of xyz it exists.
		\return size of buffer(including null) to contain the parsed string.
	*/
	uint32_t ParseLocalization(char8_t *Buffer, uint32_t BufferSize, const LWUTF8Iterator &Source);

	LWELocalization &SetActiveLocalization(uint32_t LocalizationID);

	bool PushString(const LWUTF8Iterator &StringName, const LWUTF8Iterator &String, uint32_t LocalizationID);

	LWUTF8Iterator Find(const LWUTF8Iterator &StringName);

	LWUTF8Iterator Find(uint32_t StringNameHash);

	LWAllocator &GetAllocator(void);

	uint32_t GetActiveLocalization(void);

	LWELocalization(LWAllocator &Allocator);

	~LWELocalization();
private:
	LWAllocator &m_Allocator;
	std::unordered_map<uint32_t, char8_t*> m_StringMap[LWELocalization::Count];
	uint32_t m_ActiveLocalization;
};


#endif
