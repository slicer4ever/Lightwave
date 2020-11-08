#include "LWPlatform/LWDirectory.h"
#include "LWPlatform/LWFileStream.h"
#include <cstdarg>
#include <algorithm>
#include <cstring>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

bool LWDirectory::DirExists(const LWUTF8Iterator &DirectoryPath, const LWFileStream *ExistingStream) {
	auto DirExistsAsset = [](char8_t *Buffer) -> bool {
		AAssetDir *D = AAssetManager_openDir(LWAppContext.m_App->assetManager, (char*)Buffer + 1);
		if (!D) return false;
		AAssetDir_close(D);
		return true;
	};

	auto DirExistsDir = [](char8_t *Buffer)->bool {
		Dir *D = opendir((char*)Buffer);
		if (!D) return false;
		closedir(D);
		return true;
	};
	char8_t Buffer[256];
	uint32_t Len = ParsePath(DirectoryPath, Buffer, sizeof(Buffer), ExistingStream);
	if (!Len || Len > sizeof(Buffer)) return false;
	if (*Buffer == LWFileStream::AssetToken) return DirExistsAsset(Buffer);
	return DirExistsDir(Buffer);
}

bool LWDirectory::OpenDir(LWDirectory &DirObject, const LWUTF8Iterator &DirectoryPath, LWAllocator &Allocator, const LWFileStream *ExistingStream) {
	char8_t Buffer[256];
	char8_t FileBuffer[256];
	uint32_t Count = 0;
	uint32_t Len = ParsePath(DirectoryPath, Buffer, sizeof(Buffer), ExistingStream);
	if (!Len || Len > sizeof(Buffer)) return false;
	auto ProcessAssetDir = [&DirObject, &Buffer, &FileBuffer, &Count](void)->bool {
		AAssetDir *D = AAssetManager_openDir(LWAppContext.m_App->assetManager, Buffer + 1);
		if (!D) return false;
		DirObject = LWDirectory(Buffer, Allocator);
		const char *f = nullptr;
		for (; (f = AAssetManager_getNextFileName(D)) != nullptr;) {
			DirObject.PushFile(LWFile(LWUTF8Iterator((const char8_t*)f), 0, CanRead));
		}
		AAssetDir_close(D);
		return true;
	};
	auto ProcessDir = [&DirObject, &Buffer, &FileBuffer, &Count](void)->bool {
		DIR *D = opendir(Buffer);
		struct dirent *f = nullptr;
		struct stat s;
		if (!D) return false;
		DirObject = LWDirectory(Buffer, Allocator);
		for (; (f = readdir(D)) != nullptr;) {
			const char *FName = f->d_name;
			snprintf(FileBuffer, sizeof(FileBuffer), "%s%s", Buffer, FName);
			if (stat(FileBuffer, &s) == -1) continue;
			bool isDirectory = S_ISDIR(s.st_mode) != 0;
			bool isHidden = *FName == '.' ? Hidden : 0;
			bool isReadable = access(FileBuffer, R_OK) != 0;
			bool isWriteable = access(FileBuffer, W_OK) != 0;
			DirObject.PushFile(LWFile(LWUTF8Iterator((const char8_t*)f->d_name), (uint64_t)s.st_size, (isDirectory ? Directory : 0) | (isHidden ? Hidden : 0) | (isReadable ? CanRead : 0) | (isWriteable ? CanWrite : 0)));
		}
		closedir(D);
		return true;
	};

	bool Result = true;
	if (*Buffer == AssetToken) Result = ProcessAssetDir();
	else Result = ProcessDir();
	return Result;
}

bool LWDirectory::CreateDir(const LWUTF8Iterator &DirectoryPath, bool isParentDir) {
	int Error = EEXIST;
	if (mkdir(*DirectoryPath.c_str<256>(), S_IRWXU | S_IRWXG | S_IRWXG) != 0) Error = errno;
	//Certain errors are acceptable to get, if directory already exists.
	return Error == EEXIST;
}

