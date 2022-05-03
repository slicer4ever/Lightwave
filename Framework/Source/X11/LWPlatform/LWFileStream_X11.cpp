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
	uint32_t DirLen = pathconf(".", _PC_PATH_MAX);
	char8_t *B = Buffer;
	char8_t *BL = Buffer + std::min<uint32_t>(BufferLen - 1, BufferLen);
	if (DirLen < BufferLen) {
		getcwd(Buffer, BufferLen);
		B = std::min<char8_t*>(B + DirLen, BL);
	}
	if (B + 1 < BL) *B++ = '/';
	if (BufferLen) *B = '\0';
	return DirLen + 1;
}

uint32_t LWFileStream::GetFolderPath(uint32_t FolderID, char8_t *Buffer, uint32_t BufferLen){
	static struct passwd *pw = nullptr;
	if(!pw) pw = getpwuid(getuid());
	
	if (FolderID == Fonts) return snprintf((char*)Buffer, BufferLen, "/usr/share/fonts"); //Default assumed font path.
	else if (FolderID == User) return snprintf((char*)Buffer, BufferLen, "%s", pw->pw_dir); //Default assumed user path.
	else if (FolderID == Game) return snprintf((char*)Buffer, BufferLen, "/var/lib"); //Default assumed game storage path.
	else if (FolderID == App) return snprintf((char*)Buffer, BufferLen, "Content"); //Default local content game path.
	return 0;
}