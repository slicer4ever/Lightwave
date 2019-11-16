#include "LWPlatform/LWFileStream.h"
#include "LWCore/LWAllocators/LWAllocator_Default.h"
#include "LWPlatform/LWPlatform.h"
#include "LWCore/LWText.h"
#include <cstdarg>
#include <cstdio>
#include <algorithm>
#include <iostream>

uint32_t LWFileStream::MakeDirectoryPath(const LWText &FilePath, char *Buffer, uint32_t BufferLen){
	uint32_t l = 0;
	const char *F = (const char*)FilePath.GetCharacters();
	char *B = Buffer;
	for (const char *C = F; *C; C++){
		if (Buffer && (uint32_t)(C - F) < BufferLen) *B++ = *C;
		if (*C == '\\' || *C == '/') l = (uint32_t)(C - F) + 1;
	}
	if (Buffer && l < BufferLen) *(Buffer + l) = '\0';
	return l + 1;
}

uint32_t LWFileStream::MakeRelativePath(const LWText &From, const LWText &To, char *Buffer, uint32_t BufferLen){
	const char *F = (const char*)From.GetCharacters();
	const char *T = (const char*)To.GetCharacters();
	uint32_t DCount = 0;
	//find shared directory:
	for (; *F && *T; F++, T++){
		if (!((*F == '\\' || *F == '/') && (*T == '\\' || *T == '/')) && *F != *T) break;
	}
	//now find the number of directory that from must go up before reaching the shared directory:
	for (; *F; F++){
		if (*F == '\\' || *F == '/') DCount++;
	}
	uint32_t Len = 0;
	for (; DCount; DCount--){
		if (Buffer && Len + 3 < BufferLen){
			*Buffer++ = '.';
			*Buffer++ = '.';
			*Buffer++ = '/';
		}
		Len += 3;
	}
	for (; *T; T++, Len++){
		if (Buffer && Len + 1 < BufferLen) *Buffer++ = *T;
	}
	if (Buffer && Len + 1 < BufferLen) *Buffer = '\0';
	return Len + 1;
}

uint32_t LWFileStream::MakeFileName(const LWText &FilePath, char *Buffer, uint32_t BufferLen){
    const char *F = (const char*)FilePath.GetCharacters();
    const char *P = F;
    for(;*F;F++) if(*F=='\\' || *F=='/' || *F==':') P=F+1;
    uint32_t Len = (uint32_t)(F-P);
    if(Buffer){
        char *Last = Buffer+BufferLen;
        char *C = Buffer;
        for(;*P && C!=Last;) *C++=*P++;
        if(C!=Last) *C=*P;
        Buffer[BufferLen-1]='\0';
    }
    return Len+1;
}

bool LWFileStream::PathIsAbsolute(const LWText &FilePath){
	const char *F = (const char*)FilePath.GetCharacters();
	return *F == '\\' || *F == '/' || *(F + 1) == ':';
}

uint32_t LWFileStream::GetExtension(const LWText &FilePath, char *Buffer, uint32_t BufferLen){
	const char *C = (const char*)FilePath.GetCharacters();
	const char *E = nullptr;
	uint32_t Len = 0;
	for (; *C; C++) if (*C == '.') E = C + 1;
	if (E){
		for (; *E; E++, Len++){
			if (Buffer && Len + 1 < BufferLen) *Buffer++ = *E;
		}
	}
	if (Buffer && Len + 1 < BufferLen) *Buffer = '\0';
	return Len;
}

bool LWFileStream::IsExtension(const LWText &FilePath, const char *Extension){
	const char *C = (const char*)FilePath.GetCharacters();
	const char *E = nullptr;
	for (; *C; C++) if (*C == '.') E = C + 1;
	if (!E) return false;
	for (; *Extension; E++, Extension++) if (tolower(*E) != tolower(*Extension)) return false;
	return true;
}

uint32_t LWFileStream::IsExtensions(const LWText &FilePath, uint32_t ExtCount, ...){
	const char *C = (const char*)FilePath.GetCharacters();
	const char *E = nullptr;
	for (; *C; C++) if (*C == '.') E = C + 1;
	if (!E) return 0xFFFFFFFF;
	va_list lst;
	va_start(lst, ExtCount);
	for (uint32_t i = 0; i < ExtCount; i++){
		const char *Ext = va_arg(lst, const char*);
		bool Is = true;
		for (const char *CE = E; *Ext && Is; CE++, Ext++) Is = tolower(*CE) == tolower(*Ext);
		if (Is){
			va_end(lst);
			return i;
		}
	}
	va_end(lst);
	return 0xFFFFFFFF;
}

uint32_t LWFileStream::ConcatExtension(const LWText &FilePath, const LWText &Extension, char *Buffer, uint32_t BufferLen){
	const char *C = (const char*)FilePath.GetCharacters();
	const char *E = nullptr;
	uint32_t Len = 0;
	for (; *C; C++, Len++){
		if (Buffer && Len + 1 < BufferLen) *Buffer++ = *C;
		if (*C == '.') E = C + 1;
	}
	if (E){
		const char *Ext = (const char*)Extension.GetCharacters();
		bool Is = true;
		for (; *Ext && Is; E++) Is = tolower(*E) == tolower(*(Ext++));
		if (Is) return Len + 1;
	}
	if (Buffer && Len + 1 < BufferLen) *Buffer++ = '.';
	Len++;
	for (C = (const char*)Extension.GetCharacters(); *C; C++, Len++){
		if (Buffer && Len + 1 < BufferLen) *Buffer++ = *C;
	}
	if (Buffer && Len + 1 < BufferLen) *Buffer = '\0';
	Len++;
	return Len;
}

bool LWFileStream::ParsePath(const LWText &FilePath, char *Buffer, uint32_t BufferLen, const LWFileStream *ExistingStream){
	char FolderPath[512] = "";
	const char *Path = (const char*)FilePath.GetCharacters();
	const char *PathRes = LWText::CopyToTokens(Path, nullptr, 0, ":");
	if (*PathRes==':') {
		if (!std::strncmp("Game:", Path, 5)) {
			Path = Path + 5;
			if (!GetFolderPath(Game, FolderPath, sizeof(FolderPath))) return false;
		} else if (!std::strncmp("Fonts:", Path, 6)) {
			Path = Path + 6;
			if (!GetFolderPath(Fonts, FolderPath, sizeof(FolderPath))) return false;
		} else if (!std::strncmp("App:", Path, 4)) {
			Path = Path + 4;
			if (!GetFolderPath(App, FolderPath, sizeof(FolderPath))) return false;
		} else if (!std::strncmp("User:", Path, 5)) {
			Path = Path + 5;
			if (!GetFolderPath(User, FolderPath, sizeof(FolderPath))) return false;
		}
	}else if(ExistingStream && std::strncmp(Path, "/", 1) && std::strncmp(Path, "\\", 1)){
		snprintf(Buffer, BufferLen, "%s%s", (const char*)ExistingStream->GetDirectoryPath().GetCharacters(), Path);
		return true;
	}

	if (*FolderPath) snprintf(Buffer, BufferLen, "%s/%s", FolderPath, Path);
	else snprintf(Buffer, BufferLen, "%s", Path);
	return true;
}

bool LWFileStream::OpenStream(LWFileStream &Result, const LWText &FilePath, uint32_t Flag, LWAllocator &Allocator, const LWFileStream *ExistingStream){
	return OpenStream(Result, (const char*)FilePath.GetCharacters(), Flag, Allocator, ExistingStream);
}

bool LWFileStream::OpenStreamf(LWFileStream &Result, const uint8_t *FilePath, uint32_t Flag, LWAllocator &Allocator, const LWFileStream *ExistingStream, ...){
	char Buffer[512];
	va_list lst;
	va_start(lst, ExistingStream);
	vsnprintf(Buffer, sizeof(Buffer), (const char*)FilePath, lst);
	va_end(lst);
	return OpenStream(Result, Buffer, Flag, Allocator, ExistingStream);
}

bool LWFileStream::OpenStreamf(LWFileStream &Result, const char *FilePath, uint32_t Flag, LWAllocator &Allocator, const LWFileStream *ExistingStream, ...){
	char Buffer[512];
	va_list lst;
	va_start(lst, ExistingStream);
	vsnprintf(Buffer, sizeof(Buffer), FilePath, lst);
	va_end(lst);
	return OpenStream(Result, Buffer, Flag, Allocator, ExistingStream);
}

bool LWFileStream::Exists(const LWText &Filepath) {
	LWFileStream Stream;
	LWAllocator_Default Alloc;
	return OpenStream(Stream, Filepath, LWFileStream::BinaryMode | LWFileStream::ReadMode, Alloc);
}

bool LWFileStream::DelFile(const LWText &Filepath) {
	auto Res = remove((const char*)Filepath.GetCharacters());
	if (Res) std::cout << "Error removing: " << errno << std::endl;
	return Res == 0;
}

bool LWFileStream::MovFile(const LWText &SrcFilepath, const LWText &DstFilepath) {
	auto Res = rename((const char*)SrcFilepath.GetCharacters(), (const char*)DstFilepath.GetCharacters());
	if (Res) std::cout << "Error moving: " << errno << std::endl;
	return Res == 0;
}

LWFileStream &LWFileStream::operator =(LWFileStream &&Other){
	if (m_FileObject) fclose(m_FileObject);
	m_FileObject = Other.m_FileObject;
	m_FilePath = std::move(Other.m_FilePath);
	m_DirPath = std::move(Other.m_DirPath);
	m_CreateTime = Other.m_CreateTime;
	m_ModifiedTime = Other.m_ModifiedTime;
	m_AccessedTime = Other.m_AccessedTime;
	m_Length = Other.m_Length;
	m_Flag = Other.m_Flag;
	Other.m_FileObject = nullptr;
	return *this;
}

uint32_t LWFileStream::ReadText(char *Buffer, uint32_t BufferLen) {
	return ReadText((uint8_t*)Buffer, BufferLen);
}

uint32_t LWFileStream::ReadTextLine(char *Buffer, uint32_t BufferLen) {
	return ReadTextLine((uint8_t*)Buffer, BufferLen);
}

uint32_t LWFileStream::Read(char *Buffer, uint32_t Len) {
	return Read((uint8_t*)Buffer, Len);
}

uint32_t LWFileStream::WriteTextf(const char *Format, ...) {
	char Buffer[512];
	va_list lst;
	va_start(lst, Format);
	vsnprintf(Buffer, sizeof(Buffer), Format, lst);
	va_end(lst);
	return WriteText(LWText(Buffer));
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

const LWText &LWFileStream::GetDirectoryPath(void) const{
	return m_DirPath;
}

const LWText &LWFileStream::GetFilePath(void) const{
	return m_FilePath;
}

LWFileStream::LWFileStream() : m_FileObject(nullptr), m_CreateTime(0), m_ModifiedTime(0), m_AccessedTime(0), m_Length(0), m_Flag(0){}

LWFileStream::LWFileStream(LWFileStream &&Other) : m_FileObject(Other.m_FileObject), m_FilePath(std::move(Other.m_FilePath)), m_DirPath(std::move(Other.m_DirPath)), m_CreateTime(Other.m_CreateTime), m_ModifiedTime(Other.m_ModifiedTime), m_AccessedTime(Other.m_AccessedTime), m_Length(Other.m_Length), m_Flag(Other.m_Flag){
	Other.m_FileObject = nullptr;
}

LWFileStream::~LWFileStream() {
	Finished();
}