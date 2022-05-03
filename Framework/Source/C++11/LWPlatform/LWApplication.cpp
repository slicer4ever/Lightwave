#include "LWPlatform/LWPlatform.h"
#include "LWCore/LWUnicode.h"

int32_t LWDecodeCommandLineArguments(const LWUTF8Iterator &CommandLine, LWUTF8Iterator *IterList, uint32_t IterListCount) {
	LWUTF8Iterator C = CommandLine;
	uint32_t o = 0;
	for (C.AdvanceWord(true); !C.AtEnd();) {
		uint32_t Token = *C;
		if (Token == '\"' || Token == '\'') ++C;
		else Token = ' ';
		LWUTF8Iterator P = C.NextToken(Token, true);
		if (o++ < IterListCount) IterList[o - 1] = LWUTF8Iterator(C, P);
		C = ++P.AdvanceWord(true);
	}
	return o;
}