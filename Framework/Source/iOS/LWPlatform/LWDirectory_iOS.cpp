#include "LWPlatform/LWDirectory.h"
#include "LWPlatform/LWFileStream.h"
#include <cstdarg>
#include <algorithm>
#include <cstring>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <iostream>

bool LWDirectory::DirExists(const LWUTF8Iterator &DirectoryPath, const LWFileStream *ExistingStream){
	char8_t Buffer[256];
	uint32_t Len = ParsePath(DirectoryPath, Buffer, sizeof(Buffer), ExistingStream);
	if (!Len || Len > sizeof(Buffer)) return false;
	DIR *D = opendir(Buffer);
	if (!D) return false;
	closedir(D);
	return true;
}

bool LWDirectory::OpenDir(LWDirectory &DirObject, const LWUTF8Iterator &DirectoryPath, LWAllocator &Allocator, const LWFileStream *ExistingStream){
	char8_t Buffer[256]= "";
	char8_t FileBuffer[256] = "";
	uint32_t Count = 0;
	uint32_t Len = ParsePath(DirectoryPath, Buffer, sizeof(Buffer), ExistingStream);
	if (!Len || Len > sizeof(Buffer)) return false;

	DIR *D = opendir(Buffer);
	if (!D) return false;
	DirObject = LWDirectory(Buffer, Allocator);
	struct dirent *f = nullptr;
	struct stat s;
	for (; (f = readdir(D))!=nullptr; ){
		const char *FName = f->d_name;
		snprintf(FileBuffer, sizeof(FileBuffer), "%s%s", Buffer, FName);
		if(stat(FileBuffer, &s)==-1) continue;
		bool isDirectory = S_ISDIR(s.st_mode) != 0;
		bool isHidden = *FName == '.' ? Hidden : 0;
		bool isReadable = access(FileBuffer, R_OK) != 0;
		bool isWriteable = access(FileBuffer, W_OK) != 0;
		DirObject.PushFile(LWFile((const char8_t*)FName, (uint64_t)s.st_size, (isDirectory ? Directory : 0) | (isHidden ? Hidden : 0) | (isReadable ? CanRead : 0) | (isWriteable ? CanWrite : 0)));
	}
	closedir(D);
	return true;
}

bool LWDirectory::CreateDir(const LWUTF8Iterator &DirectoryPath, bool isParentDir) {
	int Error = EEXIST;
	if (mkdir(*DirectoryPath.c_str<256>(), S_IRWXU | S_IRWXG | S_IRWXG) != 0) Error = errno;
	//Certain errors are acceptable to get, if directory already exists, or if access is denied, but this is a parent directory it's possible for this to be triggered even if the child directory exists and we are allowed to write into that directory.)
	return Error == EEXIST || Error == EISDIR || (Error == EACCES && isParentDir); 
}


