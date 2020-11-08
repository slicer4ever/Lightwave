#include "LWPlatform/LWFileStream.h"
#include "LWPlatform/LWPlatform.h"
#include "LWCore/LWText.h"
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <pwd.h>
#include <algorithm>

uint32_t LWFileStream::GetWorkingDirectory(char8_t *Buffer, uint32_t BufferLen) {
	static const char *DirPath = nullptr;
	static uint32_t DirLen = 0;
	if (!DirPath) {
		DirPath = [[[NSBudle mainBundle]bundlePath] UTF8String];
		DirLen = (uint32_t)strlen(DirPath);
	}
	char8_t *B = Buffer;
	char8_t *BL = Buffer + std::min<uint32_t>(BufferLen - 1, BufferLen);
	uint32_t CopyLen = std::min<uint32_t>(DirLen, (uint32_t)(uintptr_t)(BL - B));
	std::copy(DirPath, DirPath + CopyLen, B);
	B += CopyLen;
	if (B + 1 < BL) *B++ = '/';
	if (BufferLen) *B = '\0';
	return DirLen + 2;
}


uint32_t LWFileStream::GetFolderPath(uint32_t FolderID, char8_t *Buffer, uint32_t BufferLen) {
	static struct passwd *pw = nullptr;
	static const char *AppPath = nullptr;
	if (!pw) {
		pw = getpwuid(getuid());
		AppPath = [[[NSBundle mainBundle] bundlePath] UTF8String];
	}
	

	if (FolderID == Fonts) return snprintf((char*)Buffer, BufferLen, "/Library/Fonts"); //Default assumed font path.
	else if (FolderID == Game) return snprintf((char*)Buffer, BufferLen, "/Library/Application/Support");
	else if (FolderID == User) return snprintf((char*)Buffer, BufferLen, "%s", pw->pw_dir); //Default assumed app data path.
	else if (FolderID == App) return snprintf((char*)Buffer, BufferLen, "%s/Contents/Resources/Content", AppPath); //Default assumed program data path.
	return 0;
}