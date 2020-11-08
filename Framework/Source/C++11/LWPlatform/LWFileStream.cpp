#include "LWPlatform/LWFileStream.h"
#include "LWCore/LWAllocators/LWAllocator_Default.h"
#include "LWPlatform/LWPlatform.h"
#include "LWPlatform/LWDirectory.h"
#include "LWCore/LWUnicode.h"
#include <cstdarg>
#include <cstdio>
#include <algorithm>
#include <iostream>

void LWFileStream::SplitPath(const LWUTF8Iterator &FilePath, LWUTF8Iterator &DirPath, LWUTF8Iterator &FileName, LWUTF8Iterator &Extension) {
	LWUTF8Iterator C = FilePath;
	LWUTF8Iterator NameStart;
	LWUTF8Iterator ExtStart;
	for (; !C.AtEnd(); ++C) {
		uint32_t cp = *C;
		if (cp == '\\' || cp == '/') NameStart = C+1;
		else if (cp=='.') ExtStart = C;
	}
	if (NameStart.isInitialized()) DirPath = LWUTF8Iterator(FilePath, NameStart);
	else NameStart = FilePath;
	if (ExtStart.isInitialized()) {
		FileName = LWUTF8Iterator(NameStart, ExtStart);
		Extension = LWUTF8Iterator(ExtStart + 1, C);
	} else FileName = LWUTF8Iterator(NameStart, C);
	return;
}

void LWFileStream::SplitPath(const LWUTF8Iterator &FilePath, LWUTF8Iterator &DirPath, LWUTF8Iterator &FileName) {
	LWUTF8Iterator C = FilePath;
	LWUTF8Iterator NameStart;
	for (; !C.AtEnd(); ++C) {
		uint32_t cp = *C;
		if (cp == '\\' || cp == '/') NameStart = C+1;
	}
	if (NameStart.isInitialized()) {
		DirPath = LWUTF8Iterator(FilePath, NameStart);
		FileName = LWUTF8Iterator(NameStart, C);
	} else FileName = LWUTF8Iterator(FilePath, C);
	return;
}

uint32_t LWFileStream::MakeRelativePath(const LWUTF8Iterator &From, const LWUTF8Iterator &To, char8_t *Buffer, uint32_t BufferLen){
	LWUTF8Iterator F = From;
	LWUTF8Iterator T = To;
	char8_t *B = Buffer;
	char8_t *BL = B + std::min<uint32_t>(BufferLen - 1, BufferLen);
	uint32_t DCount = 0;
	uint32_t o = 0;
	LWUTF8Iterator LastTDir = T;
	LWUTF8Iterator LastFDir = F;
	//find shared directory:
	for (; !F.AtEnd() && !T.AtEnd(); ++F, ++T) {
		bool isFDir = *F == '\\' || *F == '/';
		bool isTDir = *T == '\\' || *T == '/';
		if (isFDir) LastFDir = F+1;
		if (isTDir) LastTDir = T+1;
		if (*F != *T && !(isFDir && isTDir)) break;
	}
	F = LastFDir;
	T = LastTDir;
	//now find the number of directory that from must go up before reaching the shared directory:
	for (; !F.AtEnd(); ++F) {
		if (*F == '\\' || *F == '/') {
			if (B + 3 < BL) {
				*B++ = '.'; *B++ = '.'; *B++ = '/';
			}
			o += 3;
		}
	}

	//Now append remaining Directors in T.
	for(;!T.AtEnd();++T){
		uint32_t r = LWUTF8Iterator::CodePointUnitSize(*T);
		if (B + r < BL) B += LWUTF8Iterator::EncodeCodePoint(B, (uint32_t)(uintptr_t)(BL-B), *T);
		o += r;
	}
	if (BufferLen) *B = '\0';
	return o + 1;
}

bool LWFileStream::PathIsAbsolute(const LWUTF8Iterator &FilePath){
	return *FilePath == '\\' || *FilePath == '/' || *(FilePath + 1) == ':';
}

bool LWFileStream::IsExtension(const LWUTF8Iterator &FilePath, const LWUTF8Iterator &Extension) {
	LWUTF8Iterator Dir, Name, Ext;
	SplitPath(FilePath, Dir, Name, Ext);
	if (!Ext.isInitialized()) return false;
	return Ext.Compare(Extension);
}

uint32_t LWFileStream::ConcatExtension(const LWUTF8Iterator &FilePath, const LWUTF8Iterator &Extension, char8_t *Buffer, uint32_t BufferLen){
	LWUTF8Iterator ExtPart;
	LWUTF8Iterator C = FilePath;
	char8_t *B = Buffer;
	char8_t *BL = Buffer + std::min<uint32_t>(BufferLen - 1, BufferLen);
	uint32_t o = 0;
	for (; !C.AtEnd(); ++C) {
		uint32_t r = LWUTF8Iterator::CodePointUnitSize(*C);
		if (B + r < BL) B += LWUTF8Iterator::EncodeCodePoint(B, (uint32_t)(uintptr_t)(BL - B), *C);
		if (*C == '.') ExtPart = C;
		o += r;
	}
	if (!ExtPart.isInitialized() || !Extension.Compare(ExtPart)) {
		C = Extension;
		for (; !C.AtEnd(); ++C) {
			uint32_t r = LWUTF8Iterator::CodePointUnitSize(*C);
			if (B + r < BL) B += LWUTF8Iterator::EncodeCodePoint(B, (uint32_t)(uintptr_t)(BL - B), *C);
			o += r;
		}
	}
	if (BufferLen) *B = '\0';
	return o + 1;
}

uint32_t LWFileStream::MakeAbsolutePath(const LWUTF8Iterator &FilePath, char8_t *Buffer, uint32_t BufferLen) {
	const uint32_t MaxParentDirs = 64;
	char8_t Temp[512];
	char8_t *pDirs[MaxParentDirs];
	uint32_t DirLen = GetWorkingDirectory(Temp, sizeof(Temp)) - 1;
	uint32_t Len = ParsePath(FilePath, Temp + DirLen, sizeof(Temp) - DirLen);
	if (!Len) return 0;
	if (PathIsAbsolute(Temp + DirLen)) std::copy(Temp + DirLen, Temp + DirLen + Len, Temp);
	char8_t *B = Buffer;
	char8_t *BL = B + std::min<uint32_t>(BufferLen - 1, BufferLen);
	uint32_t o = 0;
	uint32_t p = 0;
	//Clean up any ../ and ./ in path.
	for (char8_t *T = Temp; *T; ++T, ++o) {
		if (*T == '/' || *T == '\\') {
			if(p<MaxParentDirs) pDirs[p++] = B;
		} else if (*T == '.') {
			if (*(T + 1) == '/' || *(T + 1) == '\\') T += 2;
			if (*(T + 1) == '.' && (*(T + 2) == '/' || *(T + 2) == '\\')) {
				if (p > 1) B = pDirs[--p - 1];
				T += 2;
			}
		}
		if (B < BL) *B++ = *T;
	}
	if (BufferLen) *B = '\0';
	return o+1;
}

uint32_t LWFileStream::ParsePath(const LWUTF8Iterator &FilePath, char8_t *Buffer, uint32_t BufferLen, const LWFileStream *ExistingStream){
	char8_t FolderPath[512]="";
	LWUTF8Iterator DirPath, NamePath;
	LWUTF8Iterator C = FilePath.NextToken(':', false);
	if (!C.AtEnd()) {
		LWUTF8Iterator FolderIter = LWUTF8Iterator(FilePath, C);
		uint32_t FolderID = FolderIter.CompareList("Game", "Fonts", "App", "User");
		if (FolderID != -1) {
			++C;
			if (!GetFolderPath(FolderID, FolderPath, sizeof(FolderPath))) return false;
			return snprintf(Buffer, BufferLen, "%s/%s", FolderPath, *C.c_str<512>()) + 1;
		} else C = FilePath;
	} else C = FilePath;
	if(ExistingStream && !PathIsAbsolute(C)) {
		SplitPath(ExistingStream->GetFilePath().begin(), DirPath, NamePath);
		if (DirPath.isInitialized()) return snprintf((char*)Buffer, BufferLen, "%s/%s", *DirPath.c_str<512>(), *C.c_str<512>()) + 1;
	}

	return FilePath.Copy(Buffer, BufferLen);
}

bool LWFileStream::DelFile(const LWUTF8Iterator &Filepath) {
	auto Res = remove(*Filepath.c_str<256>());
	if (Res) fmt::print("Error removing: {}\n", errno);
	return Res == 0;
}

bool LWFileStream::MovFile(const LWUTF8Iterator &SrcFilepath, const LWUTF8Iterator &DstFilepath) {
	char8_t SrcPath[256];
	char8_t DstPath[256];
	uint32_t sLen = ParsePath(SrcFilepath, SrcPath, sizeof(SrcPath));
	uint32_t dLen = ParsePath(DstFilepath, DstPath, sizeof(DstPath));

	LWUTF8Iterator DstDir, DstName;
	SplitPath(DstPath, DstDir, DstName);
	if (DstDir.isInitialized()) {
		if (!LWDirectory::CreateDir(DstDir, nullptr)) return false;
	}
	auto Res = rename((const char*)SrcPath, (const char*)DstPath);
	if (Res) fmt::print("Error moving: {}\n", errno);
	return Res == 0;
}

bool LWFileStream::CpyFile(const LWUTF8Iterator &SrcFilePath, const LWUTF8Iterator &DstFilePath, LWAllocator &Allocator) {
	char8_t SrcPath[256];
	char8_t DstPath[256];
	LWFileStream SrcStream;
	LWFileStream DstStream;
	uint32_t sLen = ParsePath(SrcFilePath, SrcPath, sizeof(SrcPath));
	uint32_t dLen = ParsePath(DstFilePath, DstPath, sizeof(DstPath));
	LWUTF8Iterator DstDir, DstName;
	SplitPath(DstFilePath, DstDir, DstName);
	if (DstDir.isInitialized()) {
		if (!LWDirectory::CreateDir(DstPath, nullptr)) return false;
	}
	if (!LWFileStream::OpenStream(SrcStream, SrcFilePath, LWFileStream::BinaryMode | LWFileStream::ReadMode, Allocator)) {
		fmt::print("Error opening File: '{}'\n", SrcFilePath);
		return false;
	}
	if (!LWFileStream::OpenStream(DstStream, DstFilePath, LWFileStream::BinaryMode | LWFileStream::WriteMode, Allocator)) {
		fmt::print("Error opening File: '{}'\n", DstFilePath);
		return false;
	}
	char *Buffer = Allocator.AllocateA<char>(SrcStream.Length());
	if (!Buffer) return false;
	bool Result = true;
	if (SrcStream.Read(Buffer, SrcStream.Length()) != SrcStream.Length()) Result = false;
	else if (DstStream.Write(Buffer, SrcStream.Length()) != SrcStream.Length()) Result = false;
	LWAllocator::Destroy(Buffer);
	return false;
}

LWFileStream &LWFileStream::operator =(LWFileStream &&Other){
	if (m_FileObject) fclose(m_FileObject);
	m_FileObject = Other.m_FileObject;
	m_FilePath = std::move(Other.m_FilePath);
	m_CreateTime = Other.m_CreateTime;
	m_ModifiedTime = Other.m_ModifiedTime;
	m_AccessedTime = Other.m_AccessedTime;
	m_Length = Other.m_Length;
	m_Flag = Other.m_Flag;
	Other.m_FileObject = nullptr;
	return *this;
}

uint32_t LWFileStream::Read(char *Buffer, uint32_t Len) {
	return Read((uint8_t*)Buffer, Len);
}

uint32_t LWFileStream::Write(const char *Buffer, uint32_t Len) {
	if (m_Flag&AssetMode) return 0;
	return (uint32_t)Write((const uint8_t*)Buffer, Len);
}

uint32_t LWFileStream::Length(void) const{
	return m_Length;
}

uint32_t LWFileStream::GetFlag(void) const{
	return m_Flag;
}

uint64_t LWFileStream::GetCreatedTime(void) const {
	return m_CreateTime;
}

uint64_t LWFileStream::GetModifiedTime(void) const {
	return m_ModifiedTime;
}

uint64_t LWFileStream::GetAccessedTime(void) const {
	return m_AccessedTime;
}

FILE *LWFileStream::GetFileObject(void){
	return m_FileObject;
}

const LWUTF8 &LWFileStream::GetFilePath(void) const{
	return m_FilePath;
}

LWFileStream::LWFileStream() : m_FileObject(nullptr), m_CreateTime(0), m_ModifiedTime(0), m_AccessedTime(0), m_Length(0), m_Flag(0){}

LWFileStream::LWFileStream(LWFileStream &&Other) : m_FileObject(Other.m_FileObject), m_FilePath(std::move(Other.m_FilePath)), m_CreateTime(Other.m_CreateTime), m_ModifiedTime(Other.m_ModifiedTime), m_AccessedTime(Other.m_AccessedTime), m_Length(Other.m_Length), m_Flag(Other.m_Flag){
	Other.m_FileObject = nullptr;
}

LWFileStream::~LWFileStream() {
	Finished();
}