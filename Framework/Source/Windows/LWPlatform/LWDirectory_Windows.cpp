#include "LWPlatform/LWDirectory.h"
#include "LWPlatform/LWFileStream.h"
#include <cstdarg>
#include <algorithm>
#include <cstring>

bool LWDirectory::DirExists(const LWUTF8Iterator &DirectoryPath, const LWFileStream *ExistingStream){
	char Buffer[512];
	uint32_t Len = ParsePath(DirectoryPath, Buffer, sizeof(Buffer), ExistingStream);
	if (!Len || Len > sizeof(Buffer)-1) return false;
	//Have to add a * to path for windows.
	Buffer[Len-1] = '*';
	Buffer[Len++] = '\0';

	WIN32_FIND_DATA FileData;
	HANDLE H = FindFirstFileA(Buffer, &FileData);
	if (H == INVALID_HANDLE_VALUE) return false;
	FindClose(H);
	return true;
}

bool LWDirectory::OpenDir(LWDirectory &DirObject, const LWUTF8Iterator &DirectoryPath, LWAllocator &Allocator, const LWFileStream *ExistingStream){
	char8_t Buffer[512];
	uint32_t Len = ParsePath(DirectoryPath, Buffer, sizeof(Buffer), ExistingStream);
	if (!Len || Len > sizeof(Buffer)-1) return false;
	Buffer[Len - 1] = '*';
	Buffer[Len++] = '\0';
	WIN32_FIND_DATA FileData;
	HANDLE H = FindFirstFileA(Buffer, &FileData);
	if (H == INVALID_HANDLE_VALUE) return false;
	DirObject = LWDirectory(Buffer, Allocator);
	uint32_t Count = 0;
	for (; FindNextFile(H, &FileData); ){
		bool isDirectory = (FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
		bool isHidden = (FileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0;
		bool isWriteable = (FileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) == 0;
		DirObject.PushFile(LWFile((char8_t*)FileData.cFileName, (uint64_t)FileData.nFileSizeHigh << 32 | (uint64_t)FileData.nFileSizeLow, (isDirectory ? Directory : 0) | (isHidden ? Hidden : 0) | (isWriteable ? CanWrite : 0) | CanRead));
	}
	return true;
	
}

bool LWDirectory::CreateDir(const LWUTF8Iterator &DirectoryPath, bool isParentDir){
	int Error = ERROR_ALREADY_EXISTS;
	if (!CreateDirectory(*DirectoryPath.c_str<256>(), nullptr)) Error = GetLastError();
	return Error == ERROR_ALREADY_EXISTS || (isParentDir && Error == ERROR_ACCESS_DENIED);
}

