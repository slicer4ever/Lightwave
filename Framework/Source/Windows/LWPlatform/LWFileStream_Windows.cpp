#include "LWPlatform/LWFileStream.h"
#include "LWPlatform/LWPlatform.h"
#include "LWCore/LWText.h"
#include <cstdarg>
#include <algorithm>

uint32_t LWFileStream::MakeAbsolutePath(const LWText &FilePath, char *Buffer, uint32_t BufferLen){
	const uint32_t LocalBufferSize = 256;
	char LocalBuffer[LocalBufferSize];
	char LocalBufferB[LocalBufferSize];
	char ParsedBuffer[LocalBufferSize];
	char *LastC = LocalBufferB + LocalBufferSize;
	if (!LWFileStream::ParsePath(FilePath, ParsedBuffer, LocalBufferSize)) return 0;
	uint32_t ParseLen = (uint32_t)strlen(ParsedBuffer) + 1;
	if (LWFileStream::PathIsAbsolute(ParsedBuffer)){
		std::memcpy(LocalBuffer, ParsedBuffer, std::min<uint32_t>(LocalBufferSize, ParseLen));
	}else{
		if (!GetCurrentDirectory(LocalBufferSize, LocalBuffer)) return 0;
		uint32_t Len = (uint32_t)strlen(LocalBuffer);
		if (Len != LocalBufferSize) LocalBuffer[Len++] = '\\';
		if (Len != LocalBufferSize) LocalBuffer[Len] = '\0';
		std::memcpy(LocalBuffer + Len, ParsedBuffer, std::min<uint32_t>(LocalBufferSize - Len, ParseLen));
	}
	LocalBuffer[LocalBufferSize - 1] = '\0';
	//Clean up our absolute path!
	char *Last = LocalBuffer;
	char *B = LocalBufferB;
	for (char *L = LocalBuffer; *L && B != LastC; L++) {
		if (*L == '/' || *L == '\\') {
			*L = '\0';
			if (!strcmp(Last, "."))	B -= 2;
			else if (!strcmp(Last, "..")) {
				B -= 4;
				//back pedal until we get to the directory we are going up!
				for (; B != LocalBufferB && (*B != '\\' || *B != '/'); B--);
			}
			Last = L + 1;
			*L = '/';
		}
		*(B++) = *L;
	}
	if(B!=LastC) *(B++) = '\0';
	LocalBufferB[LocalBufferSize - 1] = '\0';
	uint32_t Len = (uint32_t)(B - LocalBufferB);
	if (Buffer){
		std::memcpy(Buffer, LocalBufferB, std::min<uint32_t>(Len, BufferLen));
		Buffer[BufferLen - 1] = '\0';
	}
	return Len;
}

bool LWFileStream::GetFolderPath(uint32_t FolderID, char *Buffer, uint32_t BufferLen) {
	KNOWNFOLDERID rFID;
	if (FolderID==App){
		snprintf(Buffer, BufferLen, "Content");
		return true;
	}else if (FolderID == Fonts) rFID = FOLDERID_Fonts;
	else if (FolderID == User) rFID = FOLDERID_RoamingAppData;
	else if (FolderID == Game) rFID = FOLDERID_ProgramData;
	else return false;
	PWSTR Folder = nullptr;
	HRESULT Res = SHGetKnownFolderPath(rFID, 0, nullptr, &Folder);
	if (Res != S_OK) return false;
	size_t Len = 0;
	wcstombs_s(&Len, Buffer, BufferLen, Folder, BufferLen);
	return true;
}