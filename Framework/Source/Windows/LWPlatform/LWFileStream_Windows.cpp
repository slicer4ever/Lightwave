#include "LWPlatform/LWFileStream.h"
#include "LWPlatform/LWPlatform.h"
#include <cstdarg>
#include <algorithm>

uint32_t LWFileStream::GetWorkingDirectory(char8_t *Buffer, uint32_t BufferLen) {
	char8_t *B = Buffer;
	char8_t *BL = B + std::min<uint32_t>(BufferLen - 1, BufferLen);
	uint32_t o = GetCurrentDirectory((uint32_t)(uintptr_t)(BL-B), (char*)Buffer);
	B = std::min<char8_t*>(BL, B + o);
	if (B + 1 < BL) *B++ = '/'; //Add slash to Directory path.
	if (BufferLen) *B = '\0';
	return o+2;
}

uint32_t LWFileStream::GetFolderPath(uint32_t FolderID, char8_t *Buffer, uint32_t BufferLen) {
	KNOWNFOLDERID rFID;
	if (FolderID == App) return snprintf((char8_t*)Buffer, BufferLen, "Content");
	else if (FolderID == Fonts) rFID = FOLDERID_Fonts;
	else if (FolderID == User) rFID = FOLDERID_RoamingAppData;
	else if (FolderID == Game) rFID = FOLDERID_ProgramData;
	else return 0;
	PWSTR Folder = nullptr;
	HRESULT Res = SHGetKnownFolderPath(rFID, 0, nullptr, &Folder);
	if (Res != S_OK) return false;
	size_t Len = 0;
	if (wcstombs_s(&Len, (char*)Buffer, BufferLen, Folder, BufferLen)) return 0;
	return (uint32_t)Len;
}