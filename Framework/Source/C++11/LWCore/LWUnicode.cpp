#include "LWCore/LWUnicode.h"

size_t LWstrlcpy(char *Dest, const char *Src, size_t dstSize) {
	char *Last = Dest + (dstSize==0?0:dstSize-1);
	const char *sSrc = Src;
	while (Dest != Last && *Src) *Dest++ = *Src++;
	if (dstSize) *Dest = '\0';
	while (*Src) ++Src;
	return (Src - sSrc);
}

size_t LWstrlcat(char *Dest, const char *Src, size_t dstSize) {
	char *Last = Dest + (dstSize==0?0:dstSize-1);
	const char *sSrc = Src;
	const char *sDest = Dest;
	char *eDest = Dest;
	while (Dest != Last && *Dest) ++Dest;
	for (eDest = Dest; *eDest;) ++eDest;
	while (Dest != Last && *Src) *Dest++ = *Src++;
	if (dstSize) *Last = '\0';
	while (*Src) ++Src;
	return (Src - sSrc) + (eDest - sDest);
}